"""Trace a routed DATA IN completion interrupt during READ(10)'s transfer wait.

The source transfer trace has a declared descriptor. This continuation also
models completion of the media prefix phase by clearing RAM:127e at descriptor
insertion, as AcquireRawReadPrefixAtCurrentPipelineTarget does. A declared
host cause 0x04 enters the captured monitor router. The handler is stepped to
the scheduler so the trace has a fixed execution bound. Sector payload and
command idle remain outside this trace.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

CHECKPOINTS = {
    "stock": {
        "yield0": 0xFEE529, "handler": 0xFDD3BF,
        "completion": 0xFED307, "isr_exit": 0xF80097,
        "yield_resume": 0xFEE52F, "scheduler": 0xFDCF80,
        "steps": 71, "isr_exit_step": 55, "yield_resume_step": 68,
    },
    "reconstructed": {
        "yield0": 0xFEAA7E, "handler": 0xFB99F0,
        "completion": 0xF83B81, "isr_exit": 0xFEADE2,
        "yield_resume": 0xFEAA84, "scheduler": 0xFDCF5D,
        "steps": 74, "isr_exit_step": 58, "yield_resume_step": 71,
    },
}


def run_side(source: Path, output: Path, points: dict[str, int], monitor: Path,
             uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "read10-transfer-start.hex").replace("\\", "\\\\")
    observation = wine_path(output / "read10-data-in-interrupt.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            monitor_path = wine_path(monitor).replace("\\", "\\\\")
            reset = "RESET\nBK *\n"
            if content.count(reset) != 1:
                raise SimulatorError(f"{path}: monitor-load insertion point changed")
            content = content.replace(reset, reset + f'LOAD "{monitor_path}" INCREMENTAL\n', 1)
            descriptor = "_WBYTE(E:0x001292, 0)\n"
            if content.count(descriptor) != 1:
                raise SimulatorError(f"{path}: descriptor insertion point changed")
            content = content.replace(descriptor, "_WBYTE(E:0x00127e, 0)\n" + descriptor, 1)
            anchor = f'SAVE "{trace}" 0x016000,0x0160bf\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: transfer-wait continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "BK *",
                "_WBYTE(E:0x0000fb, 1)",
                "_WBYTE(E:0x0000fc, 1)",
                "_WBYTE(E:0x001157, 1)",
                "_WBYTE(E:0x00faef, 0x80)",
                "_WBYTE(X:0x805a, 4)",
                "_WBYTE(0x01805a, 4)",
                "_WBYTE(D:0xa8, 0x82)",
                "_WBYTE(D:0x88, 0x30)",
                "_WDWORD(0x0160d0, $)",
                "_WBYTE(0x0160d4, SP)",
                "BS C:0xff000b", "G",
                "_WDWORD(0x0160d8, $)",
                "BK *", f"BS C:0x{points['handler']:06x}", "G",
                "_WDWORD(0x0160e0, $)",
                "BK *", f"BS C:0x{points['completion']:06x}", "G",
                "_WDWORD(0x0160e8, $)",
                "_WBYTE(0x0160ec, _RBYTE(E:0x001462))",
                "_WBYTE(0x0160ed, _RBYTE(E:0x00127e))",
            ]
            for index in range(points["steps"]):
                base = 0x16120 + 8 * index
                lines += [
                    "T", f"_WDWORD(0x{base:06x}, $)",
                    f"_WBYTE(0x{base + 4:06x}, _RBYTE(E:0x001462))",
                    f"_WBYTE(0x{base + 5:06x}, SP)",
                ]
            end = 0x16120 + 8 * points["steps"] - 1
            lines += [
                "_WBYTE(0x0160ee, _RBYTE(E:0x00131b))",
                "_WDWORD(0x0160f0, _RDWORD(E:0x001459))",
                f'SAVE "{observation}" 0x0160d0,0x{end:06x}',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-data-in-interrupt.log", timeout)
    memory = read_intel_hex(output / "read10-data-in-interrupt.hex")

    def dword(address: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(4)), "big")

    records = []
    for index in range(points["steps"]):
        base = 0x16120 + 8 * index
        records.append((dword(base) & 0xFFFFFF, memory[base + 4], memory[base + 5]))
    result = {
        "command_yield_pc": dword(0x160D0) & 0xFFFFFF,
        "command_yield_sp": memory[0x160D4],
        "physical_vector_pc": dword(0x160D8) & 0xFFFFFF,
        "handler_pc": dword(0x160E0) & 0xFFFFFF,
        "normal_completion_pc": dword(0x160E8) & 0xFFFFFF,
        "transfer_latch_at_completion": memory[0x160EC],
        "prefix_state_at_completion": memory[0x160ED],
        "first_latch_clear_step": next((i for i, (_, latch, _) in enumerate(records) if latch == 0), None),
        "isr_exit_pc": records[points["isr_exit_step"]][0],
        "yield_resume_pc": records[points["yield_resume_step"]][0],
        "scheduler_pc": records[-1][0],
        "scheduler_sp": records[-1][2],
        "transfer_latch_at_scheduler": records[-1][1],
        "recorded_cause": memory[0x160EE],
        "issued_blocks": dword(0x160F0),
        "instruction_steps": points["steps"],
    }
    expected = {
        "command_yield_pc": points["yield0"],
        "physical_vector_pc": 0xFF000B,
        "handler_pc": points["handler"],
        "normal_completion_pc": points["completion"],
        "transfer_latch_at_completion": 1,
        "prefix_state_at_completion": 0,
        "first_latch_clear_step": 3,
        "isr_exit_pc": points["isr_exit"],
        "yield_resume_pc": points["yield_resume"],
        "scheduler_pc": points["scheduler"],
        "scheduler_sp": 10,
        "transfer_latch_at_scheduler": 0,
        "recorded_cause": 4,
        "issued_blocks": 1,
        "instruction_steps": points["steps"],
    }
    for field, value in expected.items():
        if result[field] != value:
            raise SimulatorError(f"{output.name}: {field} differs: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--monitor", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source = args.source_run.resolve()
    source_report = json.loads((source / "results.json").read_text())
    if source_report.get("status") != "pass":
        raise SimulatorError("source run needs the passing READ(10) transfer-start trace")
    monitor = args.monitor.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    results = {
        side: run_side(source / side, output / side, points, monitor, args.uv4.resolve(), args.timeout)
        for side, points in CHECKPOINTS.items()
    }
    for field in ("physical_vector_pc", "transfer_latch_at_completion", "prefix_state_at_completion",
                  "first_latch_clear_step", "scheduler_sp", "transfer_latch_at_scheduler",
                  "recorded_cause", "issued_blocks"):
        if results["stock"][field] != results["reconstructed"][field]:
            raise SimulatorError(f"READ(10) DATA IN interrupt differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline READ(10) transfer wait with declared descriptor and prefix-acquired state; declared host cause 0x04 routes through captured monitor, enters normal raw-read completion, clears transfer latch, resumes YieldCoroutine0 and reaches scheduler. Fixed instruction-step continuation after completion entry; no sector payload, command idle or physical IRQ timing proven.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "monitor_sha256": hashlib.sha256(monitor.read_bytes()).hexdigest(),
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **results,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched READ(10) routed DATA IN interrupt and scheduler return")


if __name__ == "__main__":
    main()
