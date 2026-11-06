"""Follow one READ(10) command/media wait cycle through real scheduler calls.

The source case yields from the command coroutine to a captured scheduler
context. This trace selects the scheduler's actual ResumeCoroutine2 ECALL to
skip an unrelated coroutine-one turn, then executes the scheduler naturally
through coroutine-two return, the next ResumeCoroutine0 call, batch polling,
and a second command wait yield. No sector payload is supplied.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path
from tests.simulator.trace_read10_media_first_yield import CASE_ID, prepare

CHECKPOINTS = {
    "stock": {
        "resume2_call": 0xFDCF87,
        "media_entry": 0xF90000,
        "media_yield": 0xFEE53F,
        "scheduler_return": 0xFDCF8B,
        "resume0_call": 0xFDCF7C,
        "batch_entry": 0xFC5226,
        "command_yield": 0xFEE529,
    },
    "reconstructed": {
        "resume2_call": 0xFDCF6A,
        "media_entry": 0xFDE81B,
        "media_yield": 0xFEAA94,
        "scheduler_return": 0xFDCF6E,
        "resume0_call": 0xFDCF59,
        "batch_entry": 0xFA6323,
        "command_yield": 0xFEAA7E,
    },
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    project = prepare(
        source,
        output,
        points["resume2_call"],
        points["media_entry"],
        points["media_yield"],
    )
    ini = output / "run.ini"
    content = ini.read_text()
    first_trace = wine_path(output / "read10-media-first-yield.hex").replace("\\", "\\\\")
    anchor = f'SAVE "{first_trace}" 0x016000,0x01603f\n'
    if content.count(anchor) != 1:
        raise SimulatorError(f"{ini}: media-yield continuation point changed")
    trace = wine_path(output / "read10-wait-cycle.hex").replace("\\", "\\\\")
    lines = [
        anchor.rstrip("\n"),
        "BK *",
        f"BS C:0x{points['scheduler_return']:06x}",
        "G",
        "_WDWORD(0x016040, $)",
        "_WDWORD(0x016044, DR60)",
        "_WDWORD(0x016048, _RDWORD(E:0x001166))",
        "BK *",
        f"BS C:0x{points['resume0_call']:06x}",
        "G",
        "_WDWORD(0x016050, $)",
        "_WDWORD(0x016054, DR60)",
        "BK *",
        f"BS C:0x{points['batch_entry']:06x}",
        "G",
        "_WDWORD(0x016060, $)",
        "_WDWORD(0x016064, DR60)",
        "_WDWORD(0x016068, _RDWORD(E:0x001459))",
        "_WDWORD(0x01606c, _RDWORD(E:0x001455))",
        "BK *",
        f"BS C:0x{points['command_yield']:06x}",
        "G",
        "_WDWORD(0x016070, $)",
        "_WDWORD(0x016074, DR60)",
        "_WDWORD(0x016078, _RDWORD(E:0x001459))",
        "_WDWORD(0x01607c, _RDWORD(E:0x001455))",
        "_WBYTE(0x016080, _RBYTE(E:0x001226))",
        "_WBYTE(0x016081, _RBYTE(E:0x000fae))",
        f'SAVE "{trace}" 0x016000,0x01608f',
    ]
    ini.write_text(content.replace(anchor, "\n".join(lines) + "\n", 1))
    run_uv4(uv4, project, output / "uv4-read10-wait-cycle.log", timeout)
    memory = read_intel_hex(output / "read10-wait-cycle.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")

    result = {
        "media_entry_pc": value(0x16000, 4) & 0xFFFFFF,
        "media_entry_spx": value(0x16004, 4),
        "scheduler_spx_during_media": value(0x16008, 4),
        "coroutine2_initial_spx": value(0x1600C, 4),
        "media_entry_target": value(0x16010, 4),
        "media_first_yield_pc": value(0x16020, 4) & 0xFFFFFF,
        "media_first_yield_spx": value(0x16024, 4),
        "media_yield_target": value(0x1602C, 4),
        "pipeline_state": [value(0x16030 + i, 1) for i in range(3)],
        "ring_slot_count": value(0x16034, 4),
        "ring_limits": value(0x16038, 4),
        "scheduler_return_pc": value(0x16040, 4) & 0xFFFFFF,
        "scheduler_return_spx": value(0x16044, 4),
        "coroutine2_saved_spx": value(0x16048, 4),
        "next_resume0_call_pc": value(0x16050, 4) & 0xFFFFFF,
        "next_resume0_call_spx": value(0x16054, 4),
        "batch_entry_pc": value(0x16060, 4) & 0xFFFFFF,
        "batch_entry_spx": value(0x16064, 4),
        "batch_completed_blocks": value(0x16068, 4),
        "batch_requested_blocks": value(0x1606C, 4),
        "second_command_yield_pc": value(0x16070, 4) & 0xFFFFFF,
        "second_command_yield_spx": value(0x16074, 4),
        "completed_blocks": value(0x16078, 4),
        "requested_blocks": value(0x1607C, 4),
        "completion_state": value(0x16080, 1),
        "media_error": value(0x16081, 1),
    }
    pc_fields = {
        "media_entry_pc": "media_entry",
        "media_first_yield_pc": "media_yield",
        "scheduler_return_pc": "scheduler_return",
        "next_resume0_call_pc": "resume0_call",
        "batch_entry_pc": "batch_entry",
        "second_command_yield_pc": "command_yield",
    }
    for field, point in pc_fields.items():
        if result[field] != points[point]:
            raise SimulatorError(f"{output.name}: {field} missing: {result}")
    if (result["media_entry_spx"], result["scheduler_spx_during_media"], result["coroutine2_initial_spx"]) != (0x700, 0x0D, 0x707):
        raise SimulatorError(f"{output.name}: scheduler/media entry context differs: {result}")
    for field in ("media_first_yield_spx", "coroutine2_saved_spx"):
        if not 0x700 <= result[field] <= 0x73F:
            raise SimulatorError(f"{output.name}: coroutine-two stack range differs: {result}")
    if (result["scheduler_return_spx"], result["next_resume0_call_spx"]) != (0x0A, 0x0A):
        raise SimulatorError(f"{output.name}: scheduler return context differs: {result}")
    for field in ("batch_entry_spx", "second_command_yield_spx"):
        if not 0x300 <= result[field] <= 0x33F:
            raise SimulatorError(f"{output.name}: coroutine-zero stack range differs: {result}")
    if (result["media_entry_target"], result["media_yield_target"]) != (150, 150):
        raise SimulatorError(f"{output.name}: media target changed: {result}")
    if (result["pipeline_state"], result["ring_slot_count"], result["ring_limits"]) != ([2, 1, 1], 512, 0x01FF01FA):
        raise SimulatorError(f"{output.name}: media setup differs: {result}")
    if (result["batch_completed_blocks"], result["batch_requested_blocks"], result["completed_blocks"], result["requested_blocks"], result["completion_state"], result["media_error"]) != (0, 1, 0, 1, 0, 0):
        raise SimulatorError(f"{output.name}: READ(10) wait state differs: {result}")
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
        raise SimulatorError("source run needs the passing versioned READ(10) scheduler-yield case")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {}
    for side, points in CHECKPOINTS.items():
        observations[side] = run_side(
            source / CASE_ID / side,
            output / side,
            points,
            args.uv4.resolve(),
            args.timeout,
        )
    for field in ("media_entry_spx", "scheduler_spx_during_media", "coroutine2_initial_spx", "media_entry_target", "media_yield_target", "pipeline_state", "ring_slot_count", "ring_limits", "scheduler_return_spx", "next_resume0_call_spx", "batch_completed_blocks", "batch_requested_blocks", "completed_blocks", "requested_blocks", "completion_state", "media_error"):
        if observations["stock"][field] != observations["reconstructed"][field]:
            raise SimulatorError(f"READ(10) wait-cycle field differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline READ(10) packet yields to captured scheduler; controlled selection of real ResumeCoroutine2 ECALL runs media task to first yield; natural scheduler return and next ResumeCoroutine0 enter batch service and second command wait yield with 0/1 blocks; media payload, data transfer and command completion untested",
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched READ(10) media yield, scheduler return, batch poll and second wait")


if __name__ == "__main__":
    main()
