"""Continue the modeled WRITE BUFFER mode-4 transfer to command idle.

The source trace supplies a real captured coroutine context. This continuation
injects four declared controller-buffer bytes and DATA OUT completion status;
it does not model physical DMA or flash.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import (
    SimulatorError,
    decode_hardware_trace,
    read_intel_hex,
    run_uv4,
    wine_path,
)

CASE_ID = "write-buffer-mode4-valid-yield-returns-scheduler"
CHECKPOINTS = {
    "stock": (0xFD2C92, 0xFDC215),
    "reconstructed": (0xFAED71, 0xFCE1B6),
}


def prepare(source: Path, destination: Path, handler: int, idle: int) -> Path:
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
            anchor = "_WBYTE(0x016049, _RBYTE(0x018047))\n"
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: DATA OUT helper-return insertion point changed")
            trace = wine_path(destination / "mode4-command.hex").replace("\\", "\\\\")
            lines = [
                anchor.rstrip("\n"),
                "BK *",
                f"BS C:0x{handler:06x}",
                "G",
                "_WDWORD(0x016050, $)",
                "_WDWORD(0x016054, DR60)",
                "_WBYTE(0x016058, _RBYTE(E:0x000c33))",
                "_WBYTE(0x016059, _RBYTE(E:0x000b18))",
                "_WBYTE(0x01605a, _RBYTE(E:0x000b19))",
                "_WBYTE(0x01605b, _RBYTE(E:0x000b1a))",
                "_WBYTE(0x01605c, _RBYTE(E:0x000c83))",
                "BK *",
                f"BS C:0x{idle:06x}",
                "G",
                "_WDWORD(0x016060, $)",
                "_WDWORD(0x016064, DR60)",
                "_WBYTE(0x016068, _RBYTE(E:0x000c32))",
                "_WBYTE(0x016069, _RBYTE(E:0x000c33))",
                "_WBYTE(0x01606a, _RBYTE(E:0x000b19))",
                "_WBYTE(0x01606b, _RBYTE(E:0x100010))",
                "_WBYTE(0x01606c, _RBYTE(E:0x100011))",
                "_WBYTE(0x01606d, _RBYTE(E:0x100012))",
                "_WBYTE(0x01606e, _RBYTE(E:0x100013))",
                "_WBYTE(0x01606f, _RBYTE(0x01804a))",
                "_WBYTE(0x016070, _RBYTE(0x018047))",
                f'SAVE "{trace}" 0x016050,0x01607f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n")
        path.write_text(content)
    (destination / "mode4-command.hex").unlink(missing_ok=True)
    return destination / "run.uvproj"


def decode(destination: Path, handler: int, idle: int) -> dict[str, object]:
    memory = read_intel_hex(destination / "mode4-command.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + index] for index in range(width)), "big")

    result = {
        "handler_return_pc": value(0x016050, 4) & 0xFFFFFF,
        "handler_return_spx": value(0x016054, 4),
        "handler_transfer_status": value(0x016058, 1),
        "handler_sense": [value(0x016059 + index, 1) for index in range(3)],
        "handler_completion_flags": value(0x01605C, 1),
        "idle_pc": value(0x016060, 4) & 0xFFFFFF,
        "idle_spx": value(0x016064, 4),
        "packet_state": value(0x016068, 1),
        "transfer_state": value(0x016069, 1),
        "sense_key": value(0x01606A, 1),
        "staged_bytes": [value(0x01606B + index, 1) for index in range(4)],
        "command_code": value(0x01606F, 1),
        "controller_cleanup": value(0x016070, 1),
    }
    if result["handler_return_pc"] != handler or result["idle_pc"] != idle:
        raise SimulatorError(f"{destination.name}: handler/idle checkpoint missing: {result}")
    if not 0x300 <= result["handler_return_spx"] <= 0x33F or result["idle_spx"] != 0x300:
        raise SimulatorError(f"{destination.name}: command stack did not unwind: {result}")
    if result["handler_transfer_status"] != 0x0D or result["handler_sense"] != [0x70, 0, 0]:
        raise SimulatorError(f"{destination.name}: handler did not report success: {result}")
    if result["handler_completion_flags"] != 0:
        raise SimulatorError(f"{destination.name}: handler completion flag remained set: {result}")
    if (result["packet_state"], result["transfer_state"], result["sense_key"]) != (1, 5, 0):
        raise SimulatorError(f"{destination.name}: packet did not return to idle: {result}")
    if result["staged_bytes"] != [0x11, 0x22, 0x33, 0x44]:
        raise SimulatorError(f"{destination.name}: staged bytes changed: {result}")
    if (result["command_code"], result["controller_cleanup"]) != (0xC0, 0x14):
        raise SimulatorError(f"{destination.name}: controller completion differs: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--cases", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source_run = args.source_run.resolve()
    source_report = json.loads((source_run / "results.json").read_text())
    if source_report.get("status") != "pass" or source_report.get("source_case") != CASE_ID:
        raise SimulatorError("source run needs the passing mode-4 DATA OUT trace")
    cases = json.loads(args.cases.read_text())["cases"]
    case = next(item for item in cases if item["id"] == CASE_ID)
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    traces: dict[str, dict[str, object]] = {}
    events: dict[str, list[dict[str, object]]] = {}
    for side, (handler, idle) in CHECKPOINTS.items():
        destination = output / side
        project = prepare(source_run / side, destination, handler, idle)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-completion.log", args.timeout)
        traces[side] = decode(destination, handler, idle)
        events[side] = decode_hardware_trace(case, destination / "hardware-trace.hex", False)
    if events["stock"] != events["reconstructed"]:
        raise SimulatorError("WRITE BUFFER hardware event order differs")
    command_writes = [
        event["value"] for event in events["stock"]
        if event["operation"] == "write" and event["space"] == "CONTROLLER"
        and event["address"] == "0x01804a"
    ]
    if command_writes != [0x80, 0xC0]:
        raise SimulatorError(f"WRITE BUFFER command-code publication differs: {command_writes}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline valid mode-4 packet through handler return and command idle with declared payload/status; physical DMA and flash untested",
        "source_trace_sha256": source_report["script_sha256"],
        "source_case_sha256": source_report["source_case_sha256"],
        "source_run_identity": source_report["source_run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "stock": traces["stock"],
        "reconstructed": traces["reconstructed"],
        "hardware_events": events["stock"],
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched mode-4 handler return, command idle, staged bytes, and ordered events")


if __name__ == "__main__":
    main()
