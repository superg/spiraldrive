"""Continue the declared auxiliary mode-5 staged checksum to rejection.

This paired S251 scenario seeds one staged byte and a zero stored checksum word.
It starts from the accepted auxiliary checksum-entry trace and forbids the
runtime flash loader. A passing result requires the firmware to select sense
04/40/F6 in both images. The S251 controller-window mapping remains modeled.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

POINTS = {
    "stock": {"sense": 0xFAFF63},
    "reconstructed": {"sense": 0xF95D8A},
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "aux-checksum-rejection.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "aux-checksum-entry.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x016000,0x01601f\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: checksum-entry continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "BK *", f"BS C:0x{points['sense']:06x}",
                "BS C:0xff4000", "BS C:0x000800", "G",
                "_WDWORD(0x016040, $)",
                "_WDWORD(0x016044, states)",
                "_WDWORD(0x016048, DR0)",
                "_WDWORD(0x01604c, DR4)",
                "_WDWORD(0x016050, DR60)",
                "_WBYTE(0x016054, ACC)",
                "_WBYTE(0x016055, _RBYTE(X:0x0000))",
                "_WBYTE(0x016056, _RBYTE(X:0xfffe))",
                "_WBYTE(0x016057, _RBYTE(X:0xffff))",
                f'SAVE "{trace}" 0x016040,0x01605f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
            content = content.replace("twatch(20000)", "twatch(5000000)")
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-aux-checksum-rejection.log", timeout)
    memory = read_intel_hex(output / "aux-checksum-rejection.hex")
    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    result = {
        "stop_pc": value(0x16040, 4) & 0xFFFFFF,
        "stop_states": value(0x16044, 4),
        "dr0": value(0x16048, 4),
        "sense_code_argument": value(0x1604C, 4) & 0xFFFF,
        "spx": value(0x16050, 4),
        "sense_key_argument": value(0x16054, 1),
        "staged_first_byte": value(0x16055, 1),
        "staged_checksum_word": [value(0x16056 + i, 1) for i in range(2)],
    }
    if result["stop_pc"] != points["sense"]:
        raise SimulatorError(f"{output.name}: checksum loop stopped before sense: {result}")
    if (result["sense_key_argument"], result["sense_code_argument"]) != (4, 0x40F6):
        raise SimulatorError(f"{output.name}: checksum rejection selected wrong sense: {result}")
    if result["staged_first_byte"] != 1 or result["staged_checksum_word"] != [0, 0]:
        raise SimulatorError(f"{output.name}: staged checksum input changed: {result}")
    if not 100000 < result["stop_states"] < 5000000:
        raise SimulatorError(f"{output.name}: checksum continuation has invalid progress: {result}")
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
        raise SimulatorError("source run needs the passing current-image auxiliary checksum entry")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in POINTS.items()
    }
    comparable = ("sense_code_argument", "sense_key_argument", "staged_first_byte", "staged_checksum_word")
    differences = {key: (observations["stock"][key], observations["reconstructed"][key])
                   for key in comparable if observations["stock"][key] != observations["reconstructed"][key]}
    if differences:
        raise SimulatorError(f"auxiliary checksum rejection differs: {differences}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Latest-image auxiliary mode-5 checksum rejection reaches SetSenseData(4,0x40f6) in both S251 images with declared XDATA byte zero=1, stored word=0, media state, and minimum revision. Simulator trace scratch also occupies the XDATA checksum range; physical staged-buffer checksum and command idle unverified.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "differences": differences,
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched auxiliary mode-5 checksum rejection sense call")


if __name__ == "__main__":
    main()
