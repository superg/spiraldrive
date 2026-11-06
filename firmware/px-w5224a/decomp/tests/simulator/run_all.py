#!/usr/bin/env python3
"""Configure and run the Ghidra-free Plextor simulator suite."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
METADATA = ROOT / "tests/simulator/metadata/W5224A104.json"


def run(command: list[str], *, allow_failure: bool = False) -> int:
    print("+", " ".join(command), flush=True)
    result = subprocess.run(command, cwd=ROOT)
    if result.returncode and not allow_failure:
        raise SystemExit(result.returncode)
    return result.returncode


def default_keil_root() -> Path | None:
    candidates = (
        ROOT / "tools/Keil/C251",
        Path.home() / ".wine/drive_c/Keil_v5/C251",
        Path("C:/Keil_v5/C251"),
    )
    return next((path for path in candidates if (path / "BIN/C251.EXE").is_file()), None)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build both firmware images and differentially test every catalogued function without Ghidra"
    )
    parser.add_argument("stock", type=Path, help="original Plextor W5224A 1.04 MONT image")
    parser.add_argument("--keil-root", type=Path)
    parser.add_argument(
        "--monitor",
        type=Path,
        help="optional 64-KiB RAM:ff0000 runtime-monitor capture",
    )
    parser.add_argument("--build", type=Path, default=ROOT / "out/simulator/build")
    parser.add_argument("--configure-only", action="store_true")
    args = parser.parse_args()

    stock = args.stock.resolve()
    if not stock.is_file():
        parser.error(f"stock firmware does not exist: {stock}")
    monitor = args.monitor.resolve() if args.monitor else None
    if monitor is not None and not monitor.is_file():
        parser.error(f"monitor capture does not exist: {monitor}")
    keil_root = (args.keil_root or default_keil_root())
    if keil_root is None:
        parser.error("Keil C251 was not found; pass --keil-root")
    keil_root = keil_root.resolve()

    run(
        [
            sys.executable,
            str(ROOT / "tests/simulator/simulator.py"),
            "validate-metadata",
            "--metadata",
            str(METADATA),
            "--stock",
            str(stock),
        ]
    )
    configure = [
        "cmake",
        "-S",
        str(ROOT),
        "-B",
        str(args.build.resolve()),
        f"-DFIRMWARE_REFERENCE={stock}",
        f"-DKEIL_C251_ROOT={keil_root}",
        f"-DPLEXTOR_MONITOR_CAPTURE={monitor or ''}",
        "-DPLEXTOR_GHIDRA_HOME=",
        "-DBUILD_TESTING=ON",
        "-DPLEXTOR_BUILD_FIRMWARE=ON",
    ]
    if shutil.which("ninja"):
        configure.extend(("-G", "Ninja"))
    run(configure)
    if args.configure_only:
        return 0

    build_command = [
        "cmake",
        "--build",
        str(args.build.resolve()),
        "--parallel",
        "1",
        "--target",
    ]
    run([*build_command, "emulator-smoke"])
    feasibility = run(
        [*build_command, "emulator-differential"],
        allow_failure=True,
    )
    monitor_result = (
        run([*build_command, "emulator-monitor-differential"], allow_failure=True)
        if monitor is not None
        else 0
    )
    differential = run(
        [*build_command, "emulator-generated-differential"],
        allow_failure=True,
    )
    run([*build_command, "emulator-report"])
    print(f"Reports: {ROOT / 'out/simulator/report'}", flush=True)
    return feasibility or monitor_result or differential


if __name__ == "__main__":
    sys.exit(main())
