"""Bound main-image staged checksum continuation at S251's data-bank edge."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

POINTS = {
    "stock": {"sense": 0xFAFF63, "boundary_pc": (0xFD2CB8,), "cursor_register": "dr4"},
    "reconstructed": {"sense": 0xF95D8A, "boundary_pc": (0xF84FD5, 0), "cursor_register": "dr0"},
}


def run_side(source: Path, output: Path, sense: int, uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "main-checksum-boundary.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source path")
        if name == "run.ini":
            previous = wine_path(output / "main-checksum-entry.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x016080,0x01609f\nEXIT\n'
            if content.count(anchor) != 1:
                raise SimulatorError("main checksum-entry continuation point changed")
            lines = [
                anchor.split("\nEXIT\n")[0],
                "BK *", f"BS C:0x{sense:06x}",
                "BS C:0xff4000", "BS C:0x000800", "G",
                "_WDWORD(0x0160a0, $)", "_WDWORD(0x0160a4, states)",
                "_WDWORD(0x0160a8, DR0)", "_WDWORD(0x0160ac, DR4)",
                "_WDWORD(0x0160b0, DR8)", "_WDWORD(0x0160b4, DR60)",
                "_WBYTE(0x0160b8, ACC)",
                f'SAVE "{trace}" 0x0160a0,0x0160bf',
                "EXIT", "",
            ]
            content = content.replace(anchor, "\n".join(lines), 1)
            content = content.replace("twatch(1000000)", "twatch(7000000)")
        path.write_text(content)
    (output / "main-checksum-boundary.hex").unlink(missing_ok=True)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-main-checksum-boundary.log", timeout)
    memory = read_intel_hex(output / "main-checksum-boundary.hex")
    def val(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    result = {
        "stop_pc": val(0x160A0, 4) & 0xFFFFFF,
        "states": val(0x160A4, 4),
        "dr0": val(0x160A8, 4), "dr4": val(0x160AC, 4),
        "dr8": val(0x160B0, 4), "spx": val(0x160B4, 4),
        "acc": val(0x160B8, 1),
    }
    if result["states"] < 100000:
        raise SimulatorError(f"{output.name}: insufficient checksum progress: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--uv4", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=90)
    args = parser.parse_args()
    source = args.source_run.resolve()
    source_report = json.loads((source / "results.json").read_text())
    if source_report.get("status") != "pass":
        raise SimulatorError("source run needs paired main checksum entry")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {side: run_side(source / side, output / side, points["sense"], args.uv4.resolve(), args.timeout)
                    for side, points in POINTS.items()}
    for side, points in POINTS.items():
        observed = observations[side]
        if observed["stop_pc"] not in points["boundary_pc"] or observed[points["cursor_register"]] != 0x020000:
            raise SimulatorError(f"{side}: expected shared data-bank boundary missing: {observed}")
        if not 3000000 < observed["states"] < 5000000:
            raise SimulatorError(f"{side}: unexpected boundary progress: {observed}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Bounded S251 main-image checksum continuation from paired real mode-5 entry; both images stop with source cursor RAM:020000 after more than three million states; stock stops at its read instruction, while reconstructed S251 may report that instruction or zero PC at the invalid-bank halt. This proves a shared S251 bank-boundary limit, not a checksum decision or physical controller behavior.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(observations, indent=2))


if __name__ == "__main__":
    main()
