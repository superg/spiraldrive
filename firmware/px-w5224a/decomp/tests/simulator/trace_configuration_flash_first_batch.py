"""Compare 31 configuration words under a declared direct-memory flash model.

The test stops before the final word of the 32-word batch and does not model
flash busy/toggle behavior or the physical programming aperture.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4
from tests.simulator.trace_configuration_flash_poll import CASE_ID, decode, prepare, save_line


STOPS = 95


def install_batch_trace(destination: Path) -> None:
    path = destination / "run.ini"
    content = path.read_text()
    anchor = "_WBYTE(0x016032, ACC)\n"
    if content.count(anchor) != 1:
        raise SimulatorError(f"{path}: flash-poll continuation insertion point changed")
    lines = ["MAP 0xf70000,0xf7ffff READ WRITE"]
    for index in range(STOPS):
        lines += [
            "G",
            f"_WDWORD(0x016040 + {4 * index}, $)",
            f"_WDWORD(0x016200 + {4 * index}, DR28)",
            f"_WDWORD(0x016400 + {4 * index}, DR0)",
            f"_WWORD(0x016600 + {2 * index}, WR4)",
        ]
    lines += [
        save_line(destination / "program-stops.hex", 0x016040, 0x0166BF),
        save_line(destination / "program-window.hex", 0xF7A060, 0xF7A09F),
    ]
    path.write_text(content.replace(anchor, anchor + "\n".join(lines) + "\n"))
    for name in ("poll-resume-pcs.hex", "relocated-programmer.hex", "program-stops.hex",
                 "program-window.hex", "result.hex"):
        (destination / name).unlink(missing_ok=True)


def decode_batch(destination: Path) -> dict[str, object]:
    memory = read_intel_hex(destination / "program-stops.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + offset] for offset in range(width)), "big")

    gaps = []
    records = []
    for index in range(STOPS):
        word = index // 3
        phase = index % 3
        expected_pc = (0x915, 0x91C, 0x90E)[phase]
        expected_target = 0xF7A060 + 2 * (word + (phase == 2))
        expected_source = 0x1680 + 2 * (word + (phase == 2))
        expected_remaining = 0x20 - word - (phase == 2)
        actual = (
            value(0x016040 + 4 * index, 4) & 0xFFFFFF,
            value(0x016200 + 4 * index, 4) & 0xFFFFFF,
            value(0x016400 + 4 * index, 4) & 0xFFFFFF,
            value(0x016600 + 2 * index, 2),
        )
        expected = (expected_pc, expected_target, expected_source, expected_remaining)
        if actual[0] == 0 and expected_pc == 0x90E:
            gaps.append(index)
        elif actual[0] != expected_pc:
            raise SimulatorError(f"{destination.name}: stop {index} PC {actual[0]:#x}, expected {expected_pc:#x}")
        if actual[1:] != expected[1:]:
            raise SimulatorError(f"{destination.name}: stop {index} {actual}, expected {expected}")
        records.append(actual)
    if len(gaps) > 2:
        raise SimulatorError(f"{destination.name}: too many zero debugger PC captures: {gaps}")
    window = read_intel_hex(destination / "program-window.hex")
    written = bytes(window[address] for address in range(0xF7A060, 0xF7A09E))
    near = read_intel_hex(destination / "ram-after-near.hex")
    source = bytes(near[0xF2000000 + address] for address in range(0x1680, 0x16BE))
    if written != source:
        raise SimulatorError(f"{destination.name}: direct-memory program window differs from 31 source words")
    return {
        "completed_words": 31,
        "total_words_in_batch": 32,
        "last_completed_target": 0xF7A09C,
        "next_prepared_target": 0xF7A09E,
        "last_stop_pc": records[-1][0],
        "last_source_cursor": records[-1][2],
        "last_remaining_count": records[-1][3],
        "zero_pc_capture_indices": gaps,
        "written_62_bytes_sha256": hashlib.sha256(written).hexdigest(),
        "records": records,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source_run = args.source_run.resolve()
    output = args.output.resolve()
    source_report = json.loads((source_run / "results.json").read_text())
    matches = [item for item in source_report["results"] if item["id"] == CASE_ID]
    if len(matches) != 1 or matches[0]["status"] != "pass":
        raise SimulatorError("source run needs the passing versioned scheduler case")
    output.mkdir(parents=True, exist_ok=False)
    traces = {}
    for side in ("stock", "reconstructed"):
        destination = output / side
        project = prepare(source_run / CASE_ID / side, destination)
        install_batch_trace(destination)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-trace.log", args.timeout)
        traces[side] = {"poll": decode(destination), "batch": decode_batch(destination)}
    stock = traces["stock"]["batch"]
    reconstructed = traces["reconstructed"]["batch"]
    if (stock["written_62_bytes_sha256"] != reconstructed["written_62_bytes_sha256"]
            or any(a[1:] != b[1:] for a, b in zip(stock["records"], reconstructed["records"]))):
        raise SimulatorError("stock and reconstructed configuration program records differ")
    for side in traces:
        traces[side]["batch"].pop("records")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "31 direct-memory configuration program words; flash effects, busy/toggle timing, final word and later scheduler handoff unverified",
        "source_case": CASE_ID,
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "stock": traces["stock"],
        "reconstructed": traces["reconstructed"],
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("31 matching configuration program words; final word and handoff remain open")


if __name__ == "__main__":
    main()
