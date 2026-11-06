"""Identify coroutine-three stack overwrite after configuration programmer relocation.

This is a conditional S251 result from the declared direct-memory flash model.
It records the saved coroutine-three SPX, the relocated code bytes at its saved
return slot, and the invalid return after ResumeCoroutine3 in both images.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

POINTS = {
    "stock": 0xFDCF8E,
    "reconstructed": 0xFDCF77,
}


def run_side(source: Path, output: Path, resume3: int, uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    observation = wine_path(output / "stack-overlap.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "status-yield.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x0173d0,0x0173ef\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: status-yield continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "BK *", f"BS C:0x{resume3:06x}", "G",
                "_WDWORD(0x017410, $)",
                "_WDWORD(0x017414, _RDWORD(E:0x00116a))",
            ]
            for index in range(8):
                lines.append(f"_WBYTE(0x{0x17418 + index:06x}, _RBYTE(E:0x{0x900 + index:06x}))")
            for index in range(5):
                base = 0x17420 + 8 * index
                lines += ["T", f"_WDWORD(0x{base:06x}, $)", f"_WBYTE(0x{base + 4:06x}, SP)"]
            lines.append(f'SAVE "{observation}" 0x017410,0x017447')
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-stack-overlap.log", timeout)
    memory = read_intel_hex(output / "stack-overlap.hex")
    def dword(address: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(4)), "big")
    result = {
        "resume3_pc": dword(0x17410) & 0xFFFFFF,
        "coroutine3_saved_spx": dword(0x17414),
        "stack_bytes_0900_0907": [memory[0x17418 + i] for i in range(8)],
        "post_resume3_steps": [
            {"pc": dword(0x17420 + 8 * i) & 0xFFFFFF,
             "sp": memory[0x17424 + 8 * i]}
            for i in range(5)
        ],
    }
    if (result["resume3_pc"] != resume3
            or result["coroutine3_saved_spx"] != 0x907
            or result["stack_bytes_0900_0907"] != [0xB4, 0x55, 0x54, 0x7E, 0xA4, 0, 0xF0, 0x7E]
            or result["post_resume3_steps"][-1] != {"pc": 0x55547E, "sp": 0}):
        raise SimulatorError(f"{output.name}: stack-overlap boundary differs: {result}")
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
        raise SimulatorError("source run needs the passing current-image status yield")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / side, output / side, resume3, args.uv4.resolve(), args.timeout)
        for side, resume3 in POINTS.items()
    }
    if observations["stock"]["stack_bytes_0900_0907"] != observations["reconstructed"]["stack_bytes_0900_0907"]:
        raise SimulatorError("stock and reconstruction differ at the overwritten coroutine-three return slot")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Conditional S251 configuration-programmer relocation overwrites coroutine-three stack at RAM:000900; natural ResumeCoroutine3 returns to invalid PC 0x55547e in both images. No physical flash or reset behavior proven.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched stock and reconstructed coroutine-three return-slot overwrite")


if __name__ == "__main__":
    main()
