"""Continue a valid WRITE BUFFER mode-4 packet through its saved coroutine context.

This offline trace checks the return from the first real coroutine-zero yield and
resumption through the DATA OUT transport, with declared payload bytes and
completion status. It does not establish WRITE BUFFER handler completion or flash.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path


CASE_ID = "write-buffer-mode4-valid-yield-returns-scheduler"
ENTRIES = {
    "stock": (0xFEE508, 0xFEE4FD, 0xFD3124, 0xFDDF0B, 0xFDDFCF, 0xFDDFF3),
    "reconstructed": (0xFEAA5D, 0xFEAA52, 0xF826CF, 0xFDB7BE, 0xFDB87C, 0xFDB89C),
}


def prepare(source: Path, destination: Path, resume: int, transfer: int, helper: int, wait: int, finish: int) -> Path:
    shutil.copytree(source, destination)
    old = wine_path(source.resolve())
    new = wine_path(destination.resolve())
    for name in ("run.ini", "run.uvproj"):
        path = destination / name
        content = path.read_text()
        content = content.replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\"))
        content = content.replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            anchor = "G C:0xf70000\n_WBYTE(0x017003, 0)\n"
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: staging-resume insertion point changed")
            trace = wine_path(destination / "mode4-resume.hex").replace("\\", "\\\\")
            lines = [
                "G C:0xf70000",
                "_WDWORD(0x016000, $)",
                "_WDWORD(0x016004, DR60)",
                "BK *",
                f"BS C:0x{transfer:06x}",
                f"$ = C:0x{resume:06x}",
                "G",
                "_WDWORD(0x016008, $)",
                "_WDWORD(0x01600c, DR60)",
                "_WBYTE(0x016010, ACC)",
                "_WDWORD(0x016014, DR0)",
                "_WDWORD(0x016018, DR4)",
                "_WBYTE(0x01601c, _RBYTE(E:0x000c33))",
                "BK *",
                f"BS C:0x{helper:06x}",
                "G",
                "_WDWORD(0x016020, $)",
                "_WDWORD(0x016024, DR60)",
                "_WBYTE(0x016028, _RBYTE(E:0x000c33))",
                "BK *",
                f"BS C:0x{wait:06x}",
                "G",
                "_WDWORD(0x016030, $)",
                "_WBYTE(0x016034, _RBYTE(0x01800d))",
                "_WBYTE(0x016035, _RBYTE(0x01800e))",
                "_WBYTE(0x016036, _RBYTE(0x018010))",
                "_WBYTE(0x016037, _RBYTE(0x018011))",
                "_WBYTE(0x016038, _RBYTE(0x018012))",
                "_WBYTE(0x016039, _RBYTE(0x018056))",
                "_WBYTE(0x01603a, _RBYTE(0x018051))",
                "_WBYTE(0x01603b, _RBYTE(0x01805e))",
                "_WBYTE(E:0x100010, 0x11)",
                "_WBYTE(E:0x100011, 0x22)",
                "_WBYTE(E:0x100012, 0x33)",
                "_WBYTE(E:0x100013, 0x44)",
                "_WBYTE(E:0x000c33, 0x0d)",
                "BK *",
                f"BS C:0x{finish:06x}",
                "G",
                "_WDWORD(0x01603c, $)",
                "_WDWORD(0x016040, DR60)",
                "_WBYTE(0x016044, _RBYTE(E:0x000c33))",
                "_WBYTE(0x016045, _RBYTE(E:0x100010))",
                "_WBYTE(0x016046, _RBYTE(E:0x100011))",
                "_WBYTE(0x016047, _RBYTE(E:0x100012))",
                "_WBYTE(0x016048, _RBYTE(E:0x100013))",
                "_WBYTE(0x016049, _RBYTE(0x018047))",
                f'SAVE "{trace}" 0x016000,0x01604f',
                "_WBYTE(0x017003, 0)",
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n")
        path.write_text(content)
    (destination / "mode4-resume.hex").unlink(missing_ok=True)
    return destination / "run.uvproj"


def decode(destination: Path, first: int, transfer: int, helper: int, wait: int, finish: int) -> dict[str, int | list[int]]:
    memory = read_intel_hex(destination / "mode4-resume.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + index] for index in range(width)), "big")

    result = {
        "first_checkpoint_pc": value(0x016000, 4) & 0xFFFFFF,
        "saved_scheduler_spx": value(0x016004, 4),
        "data_out_entry_pc": value(0x016008, 4) & 0xFFFFFF,
        "resumed_coroutine_spx": value(0x01600C, 4),
        "acc": value(0x016010, 1),
        "dr0": value(0x016014, 4),
        "dr4": value(0x016018, 4),
        "transfer_status": value(0x01601C, 1),
        "helper_entry_pc": value(0x016020, 4) & 0xFFFFFF,
        "helper_spx": value(0x016024, 4),
        "helper_transfer_status": value(0x016028, 1),
        "wait_pc": value(0x016030, 4) & 0xFFFFFF,
        "controller_length_low": value(0x016034, 1),
        "controller_length_high": value(0x016035, 1),
        "controller_destination_low": value(0x016036, 1),
        "controller_destination_mid": value(0x016037, 1),
        "controller_destination_high": value(0x016038, 1),
        "controller_trigger": value(0x016039, 1),
        "controller_argument": value(0x01603A, 1),
        "controller_mode": value(0x01603B, 1),
        "helper_return_pc": value(0x01603C, 4) & 0xFFFFFF,
        "helper_return_spx": value(0x016040, 4),
        "completion_status": value(0x016044, 1),
        "payload": [value(0x016045 + index, 1) for index in range(4)],
        "controller_cleanup": value(0x016049, 1),
    }
    if result["first_checkpoint_pc"] != first or result["data_out_entry_pc"] != transfer:
        raise SimulatorError(f"{destination.name}: unexpected scheduler/transfer PCs: {result}")
    if result["saved_scheduler_spx"] != 0xA or not 0x300 <= result["resumed_coroutine_spx"] <= 0x3FF:
        raise SimulatorError(f"{destination.name}: invalid scheduler/coroutine SPX: {result}")
    if (result["acc"], result["dr0"], result["dr4"]) != (0x3B, 4, 0x10):
        raise SimulatorError(f"{destination.name}: wrong WRITE BUFFER transfer arguments: {result}")
    if result["helper_entry_pc"] != helper or result["helper_transfer_status"] != 8:
        raise SimulatorError(f"{destination.name}: DATA OUT helper did not enter waiting state: {result}")
    if result["wait_pc"] != wait or result["helper_return_pc"] != finish:
        raise SimulatorError(f"{destination.name}: DATA OUT wait/return checkpoint missing: {result}")
    configured = tuple(result[key] for key in (
        "controller_length_low", "controller_length_high", "controller_destination_low",
        "controller_destination_mid", "controller_destination_high", "controller_trigger",
        "controller_argument", "controller_mode"))
    if configured != (4, 0, 0x10, 0, 0, 0x50, 0, 0x87):
        raise SimulatorError(f"{destination.name}: DATA OUT controller setup differs: {result}")
    if result["completion_status"] != 0x0D or result["payload"] != [0x11, 0x22, 0x33, 0x44]:
        raise SimulatorError(f"{destination.name}: modeled DATA OUT completion differs: {result}")
    if result["controller_cleanup"] != 0x14:
        raise SimulatorError(f"{destination.name}: DATA OUT cleanup missing: {result}")
    if result["transfer_status"] != 0:
        raise SimulatorError(f"{destination.name}: DATA OUT status changed before transfer entry")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source_run = args.source_run.resolve()
    source_report = json.loads((source_run / "results.json").read_text())
    matches = [item for item in source_report["results"] if item["id"] == CASE_ID]
    if len(matches) != 1 or matches[0]["status"] != "pass":
        raise SimulatorError("source run needs the passing versioned mode-4 yield case")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    traces: dict[str, dict[str, int | list[int]]] = {}
    for side, (first, resume, transfer, helper, wait, finish) in ENTRIES.items():
        destination = output / side
        project = prepare(source_run / CASE_ID / side, destination, resume, transfer, helper, wait, finish)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-resume.log", args.timeout)
        traces[side] = decode(destination, first, transfer, helper, wait, finish)
    for key in ("acc", "dr0", "dr4", "transfer_status"):
        if traces["stock"][key] != traces["reconstructed"][key]:
            raise SimulatorError(f"DATA OUT entry {key} differs across images")
    report = {
        "schema_version": 3,
        "status": "pass",
        "scope": "Offline mode-4 packet, authentic first yield, controlled ResumeCoroutine0, declared four-byte payload and completion status through DATA OUT helper return; handler completion untested",
        "source_case": CASE_ID,
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "stock": traces["stock"],
        "reconstructed": traces["reconstructed"],
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched mode-4 yield, coroutine-zero resume, and modeled DATA OUT helper return")


if __name__ == "__main__":
    main()
