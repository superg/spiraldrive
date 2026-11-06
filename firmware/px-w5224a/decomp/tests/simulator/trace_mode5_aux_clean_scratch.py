"""Relocate S251 trace scratch outside auxiliary staged-checksum bytes.

This reproduces the first-yield-to-checksum-entry path on stock and reconstructed
1.04 images. It changes only debugger observation addresses, then records the
candidate XDATA view at entry. No firmware or flash effect is patched.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

CASE_ID = "write-buffer-mode5-auxiliary-zero-length-finalize-first-yield"
POINTS = {
    "stock": {"resume": 0xFEE4FD, "checksum": 0xFD2C93},
    "reconstructed": {"resume": 0xFEAA52, "checksum": 0xF84FB4},
}
SCRATCH = re.compile(r"0x01([2467])([0-9a-f]{3})")
BANK = {"2": "a", "4": "b", "6": "d", "7": "e"}


def relocate(line: str) -> str:
    if line.startswith("MAP "):
        return line
    return SCRATCH.sub(lambda match: f"0x01{BANK[match[1]]}{match[2]}", line)


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "clean-scratch-entry.hex").replace("\\", "\\\\")
    stage = wine_path(output / "clean-scratch-stage.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source path")
        if name == "run.ini":
            content = "\n".join(relocate(line) for line in content.splitlines()) + "\n"
            map_line = "MAP 0x010000,0x017fff READ WRITE EXEC\n"
            if content.count(map_line) != 1:
                raise SimulatorError("low-bank map changed")
            content = content.replace(map_line, "MAP X:0x0000,X:0xffff READ WRITE\n" + map_line, 1)
            anchor = "G C:0xf70000\n_WBYTE(0x01e003, 0)\n"
            if content.count(anchor) != 1:
                raise SimulatorError("first-yield insertion point changed")
            lines = [
                "G C:0xf70000", "_WBYTE(0x01e003, 0)",
                "_WBYTE(0xffc03a, 0)", "_WBYTE(E:0x000fae, 1)",
                "_WBYTE(X:0x0000, 1)", "_WBYTE(X:0xfffe, 0)", "_WBYTE(X:0xffff, 0)",
                "BK *", f"BS C:0x{points['checksum']:06x}",
                "BS C:0xff4000", "BS C:0x000800",
                f"$ = C:0x{points['resume']:06x}", "G",
                "_WDWORD(0x01d000, $)", "_WDWORD(0x01d004, states)",
                "_WDWORD(0x01d008, DR0)", "_WDWORD(0x01d00c, DR4)",
                "_WBYTE(0x01d010, _RBYTE(X:0x0000))",
                "_WBYTE(0x01d011, _RBYTE(X:0xfffe))",
                "_WBYTE(0x01d012, _RBYTE(X:0xffff))",
                f'SAVE "{trace}" 0x01d000,0x01d01f',
                f'SAVE "{stage}" X:0x0000,X:0xffff',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-clean-scratch.log", timeout)
    memory = read_intel_hex(output / "clean-scratch-entry.hex")
    stage_memory = read_intel_hex(output / "clean-scratch-stage.hex")
    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    base = min(stage_memory)
    def stage_byte(offset: int) -> int:
        return stage_memory[base + offset]
    result = {
        "checksum_pc": value(0x1D000, 4) & 0xFFFFFF,
        "states": value(0x1D004, 4),
        "start_offset": value(0x1D008, 4),
        "checksum_word_offset": value(0x1D00C, 4),
        "first_byte": value(0x1D010, 1),
        "stored_word": [value(0x1D011, 1), value(0x1D012, 1)],
        "candidate_sum": (sum(stage_byte(i) for i in range(0x8000)) + sum(stage_byte(i) for i in range(0xFF00, 0xFFFE))) & 0xFFFF,
        "nonzero_6000_7fff": sum(stage_byte(i) != 0 for i in range(0x6000, 0x8000)),
        "scratch_pc_bytes_e000_e003": [stage_byte(0xE000 + i) for i in range(4)],
    }
    if result["checksum_pc"] != points["checksum"] or result["start_offset"] != 0 or result["checksum_word_offset"] != 0xFFFE:
        raise SimulatorError(f"{output.name}: wrong checksum entry: {result}")
    if result["nonzero_6000_7fff"] != 0:
        raise SimulatorError(f"{output.name}: trace scratch overlaps candidate: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--uv4", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=90)
    args = parser.parse_args()
    source = args.source_run.resolve()
    report = json.loads((source / "results.json").read_text())
    matches = [item for item in report["results"] if item["id"] == CASE_ID]
    if len(matches) != 1 or matches[0]["status"] != "pass":
        raise SimulatorError("source run lacks passing auxiliary first yield")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {side: run_side(source / CASE_ID / side, output / side, points, args.uv4.resolve(), args.timeout)
                    for side, points in POINTS.items()}
    result = {
        "schema_version": 1, "status": "pass", "scope": "Relocated S251 trace scratch outside auxiliary checksum range; checksum entry and XDATA view only, no checksum decision or hardware effect.",
        "source_run_identity": report["run_identity"],
        "source_case_sha256": matches[0]["case_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(observations, indent=2))


if __name__ == "__main__":
    main()
