"""Check mode-5 revision rejection with an explicit S251-only data-bank relocation.

The actual 1.04 controller window is RAM:100000. S251 cannot complete a native
register-indirect read there, so this fixture changes the copy helper's bank
add from 0x10 to 0x01 in both debugger-loaded images and stages the same
bytes at X:001a/001b. Original image files are never modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4
from tests.simulator.trace_mode5_copy_entry import CASE_ID, prepare

CHECKPOINTS = {
    "stock": (0xFEE4FD, 0xFAFF63, 0xFDE0B5),
    "reconstructed": (0xFEAA52, 0xF95D8A, 0xFAE37B),
}


def run_side(source: Path, output: Path, side: str, uv4: Path, timeout: int) -> dict[str, object]:
    resume, sense_entry, bank_byte = CHECKPOINTS[side]
    project = prepare(source / side, output, resume, sense_entry, bank_byte)
    ini = output / "run.ini"
    content = ini.read_text()
    map_line = "MAP 0x010000,0x017fff READ WRITE EXEC\n"
    if content.count(map_line) != 1:
        raise SimulatorError(f"{side}: low-bank map anchor changed")
    content = content.replace(map_line, "MAP X:0x0000,X:0xffff READ WRITE\n" + map_line, 1)
    seed_line = "_WBYTE(E:0x10001b, 0x30)\n"
    if content.count(seed_line) != 1:
        raise SimulatorError(f"{side}: staged revision anchor changed")
    content = content.replace(
        seed_line,
        seed_line
        + "_WBYTE(X:0x001a, 0x30)\n"
        + "_WBYTE(X:0x001b, 0x30)\n"
        + f"_WBYTE(0x01601f, _RBYTE(C:0x{bank_byte:06x}))\n"
        + f"_WBYTE(C:0x{bank_byte:06x}, 0x01)\n",
        1,
    )
    bank_capture = f"_WBYTE(0x016014, _RBYTE(C:0x{bank_byte:06x}))\n"
    if content.count(bank_capture) != 1:
        raise SimulatorError(f"{side}: bank capture anchor changed")
    content = content.replace(
        bank_capture,
        bank_capture
        + "_WBYTE(0x016018, _RBYTE(E:0x000b44))\n"
        + "_WBYTE(0x016019, _RBYTE(E:0x000b45))\n"
        + "_WBYTE(0x01601a, _RBYTE(X:0x001a))\n"
        + "_WBYTE(0x01601b, _RBYTE(X:0x001b))\n"
        + "_WBYTE(0x01601c, ACC)\n",
        1,
    )
    content = content.replace("twatch(20000)", "twatch(500000)")
    ini.write_text(content)
    run_uv4(uv4, project, output / "uv4-revision-reject.log", timeout)
    memory = read_intel_hex(output / "mode5-copy-entry.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")

    result = {
        "checkpoint_pc": value(0x16000, 4) & 0xFFFFFF,
        "sense_key_argument": value(0x1601C, 1),
        "sense_code_argument": value(0x1600C, 4) & 0xFFFF,
        "original_bank_immediate": value(0x1601F, 1),
        "simulator_bank_immediate": value(0x16014, 1),
        "copied_revision": [value(0x16018 + i, 1) for i in range(2)],
        "staged_revision": [value(0x1601A + i, 1) for i in range(2)],
    }
    expected = {
        "checkpoint_pc": sense_entry,
        "sense_key_argument": 4,
        "sense_code_argument": 0x40F5,
        "original_bank_immediate": 0x10,
        "simulator_bank_immediate": 1,
        "copied_revision": [0x30, 0x30],
        "staged_revision": [0x30, 0x30],
    }
    if result != expected:
        raise SimulatorError(f"{side}: mode-5 revision rejection differs: {result}")
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
    matches = [item for item in source_report["results"] if item["id"] == CASE_ID]
    if len(matches) != 1 or matches[0]["status"] != "pass":
        raise SimulatorError("source run needs a passing versioned mode-5 first-yield case")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    uv4 = args.uv4.resolve()
    observations = {
        side: run_side(source / CASE_ID, output / side, side, uv4, args.timeout)
        for side in CHECKPOINTS
    }
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline mode-5 rejection reaches SetSenseData(4, 0x40f5) after copied staged revision 00, with explicit S251-only controller-window bank relocation; sense write, checksum, flash operations and activation untested",
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched mode-5 revision rejection call on stock and reconstructed images")


if __name__ == "__main__":
    main()
