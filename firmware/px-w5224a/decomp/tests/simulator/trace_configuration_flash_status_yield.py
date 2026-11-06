"""Compare coroutine-one yield after the modeled configuration flash return."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

POINTS = {
    "stock": {"yield1": 0xFEE534, "scheduler": 0xFDCF87},
    "reconstructed": {"yield1": 0xFEAA7F, "scheduler": 0xFDCF6A},
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    observation = wine_path(output / "status-yield.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "caller-return.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x017380,0x0173cf\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: caller-step continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "BK *", f"BS C:0x{points['yield1']:06x}", "G",
                "_WDWORD(0x0173d0, $)", "_WDWORD(0x0173d4, states)",
                "_WBYTE(0x0173d8, SP)", "_WBYTE(0x0173d9, _RBYTE(E:0x001009))",
                "BK *", f"BS C:0x{points['scheduler']:06x}", "G",
                "_WDWORD(0x0173dc, $)", "_WDWORD(0x0173e0, states)",
                "_WBYTE(0x0173e4, SP)",
                "_WDWORD(0x0173e8, _RDWORD(E:0x001162))",
                "_WDWORD(0x0173ec, _RDWORD(E:0x00115a))",
                f'SAVE "{observation}" 0x0173d0,0x0173ef',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-status-yield.log", timeout)
    memory = read_intel_hex(output / "status-yield.hex")
    def dword(address: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(4)), "big")
    result = {
        "yield_pc": dword(0x173d0) & 0xFFFFFF,
        "yield_states": dword(0x173d4),
        "yield_sp": memory[0x173d8],
        "ram_1009": memory[0x173d9],
        "scheduler_pc": dword(0x173dc) & 0xFFFFFF,
        "scheduler_states": dword(0x173e0),
        "scheduler_sp": memory[0x173e4],
        "coroutine1_saved_spx": dword(0x173e8),
        "scheduler_saved_spx": dword(0x173ec),
    }
    expected = {
        "yield_pc": points["yield1"], "scheduler_pc": points["scheduler"],
        "yield_sp": 3, "ram_1009": 3, "scheduler_sp": 7,
        "coroutine1_saved_spx": 0x507, "scheduler_saved_spx": 10,
    }
    if any(result[field] != value for field, value in expected.items()):
        raise SimulatorError(f"{output.name}: status yield differs: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source = args.source_run.resolve()
    source_report = json.loads((source / "results.json").read_text())
    if source_report.get("status") != "pass":
        raise SimulatorError("source run needs the passing current-image configuration caller trace")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in POINTS.items()
    }
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Current-image status-coroutine yield after declared configuration flash memory; physical flash effect and later scheduler turn unverified.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("reached status-coroutine yield and scheduler in both images")


if __name__ == "__main__":
    main()
