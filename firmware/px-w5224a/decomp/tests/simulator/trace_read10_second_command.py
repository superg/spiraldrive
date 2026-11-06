"""Trace a second prebuffered SCSI packet after READ(10) returns idle.

The source trace reaches the command coroutine's idle scheduler yield after a
conditional READ(10) transfer. This trace declares a buffered unsupported
opcode 0xff, selects the real scheduler ResumeCoroutine0 ECALL, and follows
the command's direct sense writer through the next idle yield. Physical packet
ingress and READ(10) sector payload remain outside this trace.
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
        "resume0": 0xFDCF7C, "unsupported": 0xFAFF96,
        "idle_yield": 0xFEE529, "idle_yield_step": 84,
        "scheduler": 0xFDCF80, "steps": 89,
    },
    "reconstructed": {
        "resume0": 0xFDCF59, "unsupported": 0xF89B8B,
        "idle_yield": 0xFEAA7E, "idle_yield_step": 81,
        "scheduler": 0xFDCF5D, "steps": 86,
    },
}
EXPECTED_SENSE = [0x70, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x24, 0, 0, 0, 0, 0]


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path,
             timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "read10-command-idle.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x016400,0x016f0f\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: post-READ(10) idle checkpoint changed")
            trace = wine_path(output / "read10-second-command.hex").replace("\\", "\\\\")
            lines = [
                anchor.rstrip("\n"),
                "_WBYTE(E:0x000c32, 0x10)",  # prebuffered packet pending
                "_WBYTE(E:0x000c78, 1)",     # SCSI packet protocol
                "_WBYTE(E:0x000b0a, 0xff)",  # unsupported opcode
                f"$ = C:0x{points['resume0']:06x}",
                "BK *", f"BS C:0x{points['unsupported']:06x}", "BS C:0x000000", "G",
                "_WDWORD(0x016f20, $)",
                "_WBYTE(0x016f24, _RBYTE(E:0x000b0a))",
            ]
            for index in range(points["steps"]):
                base = 0x16200 + 8 * index
                lines += [
                    "T", f"_WDWORD(0x{base:06x}, $)",
                    f"_WBYTE(0x{base + 4:06x}, _RBYTE(E:0x000c32))",
                    f"_WBYTE(0x{base + 5:06x}, SP)",
                ]
            lines += [
                *[f"_WBYTE(0x{0x16F40 + i:06x}, _RBYTE(E:0x{0x0B18 + i:06x}))" for i in range(18)],
                "_WBYTE(0x016f52, _RBYTE(E:0x000c32))",
                "_WBYTE(0x016f53, _RBYTE(E:0x000c83))",
                f'SAVE "{trace}" 0x016200,0x016f5f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-second-command.log", timeout)
    memory = read_intel_hex(output / "read10-second-command.hex")

    def dword(address: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(4)), "big")

    pcs = [dword(0x16200 + 8 * i) & 0xFFFFFF for i in range(points["steps"])]
    result = {
        "unsupported_handler_pc": dword(0x16F20) & 0xFFFFFF,
        "received_opcode": memory[0x16F24],
        "idle_yield_pc": pcs[points["idle_yield_step"]],
        "scheduler_pc": pcs[-1],
        "scheduler_sp": memory[0x16200 + 8 * (points["steps"] - 1) + 5],
        "sense": [memory[0x16F40 + i] for i in range(18)],
        "packet_state": memory[0x16F52],
        "completion_flags": memory[0x16F53],
        "instruction_steps": points["steps"],
    }
    expected = {
        "unsupported_handler_pc": points["unsupported"],
        "received_opcode": 0xFF,
        "idle_yield_pc": points["idle_yield"],
        "scheduler_pc": points["scheduler"],
        "scheduler_sp": 10,
        "sense": EXPECTED_SENSE,
        "packet_state": 1,
        "completion_flags": 0,
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
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source = args.source_run.resolve()
    source_report = json.loads((source / "results.json").read_text())
    if source_report.get("status") != "pass":
        raise SimulatorError("source run needs the passing READ(10) command-idle trace")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    results = {
        side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in CHECKPOINTS.items()
    }
    for field in ("received_opcode", "scheduler_sp", "sense", "packet_state", "completion_flags"):
        if results["stock"][field] != results["reconstructed"][field]:
            raise SimulatorError(f"second command after READ(10) differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline conditional READ(10) command idle followed by a declared prebuffered unsupported SCSI packet; both images enter the unsupported handler, write 70/05/24/00 sense and return to the next idle yield. No sector payload or physical second-packet ingress tested.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **results,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched second unsupported packet and idle yield after READ(10)")


if __name__ == "__main__":
    main()
