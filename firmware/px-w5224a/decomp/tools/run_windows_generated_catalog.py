#!/usr/bin/env python3
"""Finish the Windows generated-function catalog with staged timeout retries."""

from __future__ import annotations

import argparse
from collections import Counter
import json
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def catalog_summary(results_path: Path) -> tuple[int, int, Counter[str]]:
    document = json.loads(results_path.read_text(encoding="utf-8"))
    results = document.get("results", [])
    expected = int(document.get("run_identity", {}).get("case_count", 0))
    statuses = Counter(str(result.get("status", "missing")) for result in results)
    return len(results), expected, statuses


def catalog_command(keil_root: Path, timeout: int) -> list[str]:
    firmware = ROOT / "out/windows/firmware/Release"
    simulator_output = ROOT / "out/simulator"
    return [
        sys.executable,
        str(ROOT / "tools/run_on_private_desktop.py"),
        "--watch-executable",
        "UV4.exe",
        "--watchdog-seconds",
        str(timeout + 30),
        "--",
        sys.executable,
        str(ROOT / "tests/simulator/simulator.py"),
        "differential",
        "--uv4",
        str(keil_root.parent / "UV4/UV4.exe"),
        "--base-project",
        str(keil_root / "Examples/Blinky/BLINKY.uvproj"),
        "--application",
        str(firmware / "main.abs"),
        "--keil-driver",
        str(ROOT / "tools/keil_driver.py"),
        "--keil-root",
        str(keil_root),
        "--a251",
        str(keil_root / "BIN/A251.exe"),
        "--l251",
        str(keil_root / "BIN/L251.exe"),
        "--stock",
        str(ROOT / "data/W5224A104/MONT_104.bin"),
        "--stock-application",
        str(simulator_output / "stock/STOCK_FIRMWARE.ABS"),
        "--stock-memory-application",
        str(simulator_output / "stock/FIRMWARE_MEMORY.ABS"),
        "--reconstructed-image",
        str(firmware / "MONT_reconstructed.bin"),
        "--monitor-hex",
        str(simulator_output / "images/monitor.hex"),
        "--map",
        str(firmware / "main.map"),
        "--listings",
        str(firmware),
        "--metadata",
        str(ROOT / "tests/simulator/metadata/W5224A104.json"),
        "--cases",
        str(simulator_output / "cases/auto.json"),
        "--output",
        str(simulator_output / "runs/generated"),
        "--timeout",
        str(timeout),
        "--resume",
    ]


def validate_inputs(command: list[str]) -> None:
    valued_options = {
        "--uv4",
        "--base-project",
        "--application",
        "--keil-driver",
        "--a251",
        "--l251",
        "--stock",
        "--stock-application",
        "--stock-memory-application",
        "--reconstructed-image",
        "--monitor-hex",
        "--map",
        "--metadata",
        "--cases",
    }
    missing = [
        command[index + 1]
        for index, token in enumerate(command[:-1])
        if token in valued_options and not Path(command[index + 1]).is_file()
    ]
    if missing:
        raise FileNotFoundError("missing catalog input(s): " + ", ".join(missing))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--keil-root", type=Path, default=Path("C:/Keil_v5/C251")
    )
    parser.add_argument(
        "--timeouts",
        type=int,
        nargs="+",
        default=(10, 30, 60, 120),
        help="successive per-uVision wall limits in seconds",
    )
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args(argv)
    if os.name != "nt":
        parser.error("this driver is for the native Windows Keil installation")
    if any(timeout <= 0 for timeout in args.timeouts):
        parser.error("timeouts must be positive")

    results_path = ROOT / "out/simulator/runs/generated/results.json"
    for timeout in args.timeouts:
        command = catalog_command(args.keil_root.resolve(), timeout)
        validate_inputs(command)
        if args.dry_run:
            print(subprocess.list2cmdline(command))
            continue
        print(f"catalog pass: timeout={timeout}s", flush=True)
        try:
            subprocess.run(command, cwd=ROOT, check=False)
        except KeyboardInterrupt:
            return 130
        if not results_path.is_file():
            print("catalog pass produced no results.json", file=sys.stderr)
            return 1
        completed, expected, statuses = catalog_summary(results_path)
        print(
            f"catalog status: {completed}/{expected}; "
            + ", ".join(f"{name}={count}" for name, count in sorted(statuses.items())),
            flush=True,
        )
        if statuses.get("mismatch", 0):
            print("stopping on a firmware mismatch", file=sys.stderr)
            return 2
        if completed == expected and statuses == Counter({"pass": expected}):
            return 0

    if args.dry_run:
        return 0
    print("catalog still has incomplete or error cases after all retries", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
