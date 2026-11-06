"""Trace the final configuration word and monitor return in both images.

The flash window is ordinary S251 memory with a declared erase-poll reply.
This proves CPU control flow under that model, not a physical flash write.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4
from tests.simulator.trace_configuration_flash_first_batch import decode_batch, install_batch_trace
from tests.simulator.trace_configuration_flash_poll import CASE_ID, decode, prepare, save_line


STEPS = 55
KEY_PCS = {3: 0x926, 4: 0x929, 17: 0x963, 23: 0x97B,
           25: 0x985, 30: 0xA1F, 38: 0xA70, 52: 0xA8D}
RETURN_PCS = {
    "stock": (0xFD649F, 0xFD64B7),
    "reconstructed": (0xF800D9, 0xF83F59),
}


def install_return_steps(destination: Path) -> None:
    path = destination / "run.ini"
    content = path.read_text()
    anchor = save_line(destination / "program-stops.hex", 0x016040, 0x0166BF)
    if content.count(anchor) != 1:
        raise SimulatorError(f"{path}: first-batch trace insertion point changed")
    lines = []
    for index in range(STEPS):
        base = 0x017000 + 16 * index
        lines += [
            "T",
            f"_WDWORD(0x{base:06x}, $)",
            f"_WDWORD(0x{base + 4:06x}, DR0)",
            f"_WDWORD(0x{base + 8:06x}, DR28)",
            f"_WWORD(0x{base + 12:06x}, WR4)",
            f"_WBYTE(0x{base + 14:06x}, ACC)",
            f"_WBYTE(0x{base + 15:06x}, SP)",
        ]
    lines.append(save_line(destination / "return-steps.hex", 0x017000, 0x01736F))
    path.write_text(content.replace(anchor, "\n".join(lines) + "\n" + anchor))
    for name in ("poll-resume-pcs.hex", "relocated-programmer.hex", "program-stops.hex",
                 "program-window.hex", "return-steps.hex", "result.hex"):
        (destination / name).unlink(missing_ok=True)


def decode_return(destination: Path, side: str) -> dict[str, object]:
    memory = read_intel_hex(destination / "return-steps.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + offset] for offset in range(width)), "big")

    records = []
    for index in range(STEPS):
        base = 0x017000 + 16 * index
        records.append((value(base, 4) & 0xFFFFFF,
                        value(base + 4, 4) & 0xFFFFFF,
                        value(base + 8, 4) & 0xFFFFFF,
                        value(base + 12, 2), value(base + 14, 1), value(base + 15, 1)))
    for index, expected in KEY_PCS.items():
        if records[index][0] != expected:
            raise SimulatorError(f"{side}: step {index} PC {records[index][0]:#x}, expected {expected:#x}")
    if tuple(record[0] for record in records[53:55]) != RETURN_PCS[side]:
        raise SimulatorError(f"{side}: wrong monitor-return continuation")
    if records[23][3] != 0 or records[52][4] != 1 or records[53][4] != 1:
        raise SimulatorError(f"{side}: count or success result wrong at return")
    window = read_intel_hex(destination / "program-window.hex")
    written = bytes(window[address] for address in range(0xF7A060, 0xF7A0A0))
    near = read_intel_hex(destination / "ram-after-near.hex")
    source = bytes(near[0xF2000000 + address] for address in range(0x1680, 0x16C0))
    if written != source:
        raise SimulatorError(f"{side}: full 32-word modeled window differs from source")
    return {
        "steps": STEPS,
        "shared_relocated_pc_sequence": [record[0] for record in records[:53]],
        "return_pcs": [record[0] for record in records[53:55]],
        "success_acc_before_and_after_return": [records[52][4], records[53][4]],
        "stack_pointer_before_and_after_return": [records[52][5], records[53][5]],
        "full_64_bytes_sha256": hashlib.sha256(written).hexdigest(),
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
        install_return_steps(destination)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-trace.log", args.timeout)
        traces[side] = {"poll": decode(destination), "first_batch": decode_batch(destination),
                        "return": decode_return(destination, side)}
    stock, reconstructed = traces["stock"]["return"], traces["reconstructed"]["return"]
    if (stock["shared_relocated_pc_sequence"] != reconstructed["shared_relocated_pc_sequence"]
            or stock["full_64_bytes_sha256"] != reconstructed["full_64_bytes_sha256"]):
        raise SimulatorError("stock and reconstructed final-word or relocated return traces differ")
    for side in traces:
        traces[side]["first_batch"].pop("records")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "full 32-word configuration batch and runtime ERET under direct-memory flash model; physical busy/toggle response and next scheduler cycle unverified",
        "source_case": CASE_ID,
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "stock": traces["stock"],
        "reconstructed": traces["reconstructed"],
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("32 modeled words match; both images return from the relocated programmer")


if __name__ == "__main__":
    main()
