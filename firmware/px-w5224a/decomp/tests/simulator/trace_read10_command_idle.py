"""Trace READ(10) from routed DATA IN completion to its next idle yield.

The source trace reaches the scheduler after clearing the transfer latch. This
trace selects the scheduler's real ResumeCoroutine0 ECALL and steps a fixed
number of instructions through the command return. It skips unrelated
coroutine turns, and the sector payload remains a declared simulator input.
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
        "source_steps": 71, "resume0": 0xFDCF7C,
        "engine_return": 0xFC5225, "engine_return_step": 36,
        "idle_yield": 0xFEE529, "idle_yield_step": 108,
        "scheduler": 0xFDCF80, "steps": 113,
    },
    "reconstructed": {
        "source_steps": 74, "resume0": 0xFDCF59,
        "engine_return": 0xFA65FF, "engine_return_step": 56,
        "idle_yield": 0xFEAA7E, "idle_yield_step": 132,
        "scheduler": 0xFDCF5D, "steps": 137,
    },
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path,
             timeout: int) -> dict[str, int]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "read10-data-in-interrupt.hex").replace("\\", "\\\\")
            previous_end = 0x16120 + 8 * points["source_steps"] - 1
            anchor = f'SAVE "{previous}" 0x0160d0,0x{previous_end:06x}\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: scheduler continuation point changed")
            trace = wine_path(output / "read10-command-idle.hex").replace("\\", "\\\\")
            lines = [anchor.rstrip("\n"), f"$ = C:0x{points['resume0']:06x}"]
            for index in range(points["steps"]):
                base = 0x16400 + 8 * index
                lines += [
                    "T", f"_WDWORD(0x{base:06x}, $)",
                    f"_WBYTE(0x{base + 4:06x}, _RBYTE(E:0x001462))",
                    f"_WBYTE(0x{base + 5:06x}, _RBYTE(E:0x001226))",
                    f"_WBYTE(0x{base + 6:06x}, SP)",
                ]
            lines += [
                "_WBYTE(0x016f00, _RBYTE(E:0x000c32))",
                "_WBYTE(0x016f01, _RBYTE(E:0x000c83))",
                "_WBYTE(0x016f02, _RBYTE(E:0x001226))",
                "_WBYTE(0x016f03, _RBYTE(E:0x001462))",
                "_WDWORD(0x016f04, _RDWORD(E:0x001459))",
                "_WDWORD(0x016f08, _RDWORD(E:0x001455))",
                f'SAVE "{trace}" 0x016400,0x016f0f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-command-idle.log", timeout)
    memory = read_intel_hex(output / "read10-command-idle.hex")

    def dword(address: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(4)), "big")

    pcs = [dword(0x16400 + 8 * i) & 0xFFFFFF for i in range(points["steps"])]
    engine_index = next((i for i, pc in enumerate(pcs) if pc == points["engine_return"]), None)
    result = {
        "engine_return_step": engine_index,
        "engine_return_pc": pcs[points["engine_return_step"]],
        "engine_return_state": memory[0x16400 + 8 * points["engine_return_step"] + 5],
        "idle_yield_pc": pcs[points["idle_yield_step"]],
        "scheduler_pc": pcs[-1],
        "scheduler_sp": memory[0x16400 + 8 * (points["steps"] - 1) + 6],
        "packet_state": memory[0x16F00],
        "completion_flags": memory[0x16F01],
        "command_state": memory[0x16F02],
        "transfer_latch": memory[0x16F03],
        "issued_blocks": dword(0x16F04),
        "requested_blocks": dword(0x16F08),
        "instruction_steps": points["steps"],
    }
    expected = {
        "engine_return_step": points["engine_return_step"],
        "engine_return_pc": points["engine_return"],
        "engine_return_state": 4,
        "idle_yield_pc": points["idle_yield"],
        "scheduler_pc": points["scheduler"],
        "scheduler_sp": 10,
        "packet_state": 1,
        "completion_flags": 0,
        "command_state": 4,
        "transfer_latch": 0,
        "issued_blocks": 1,
        "requested_blocks": 1,
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
        raise SimulatorError("source run needs the passing READ(10) DATA IN interrupt trace")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    results = {
        side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in CHECKPOINTS.items()
    }
    for field in ("engine_return_state", "scheduler_sp", "packet_state", "completion_flags",
                  "command_state", "transfer_latch", "issued_blocks", "requested_blocks"):
        if results["stock"][field] != results["reconstructed"][field]:
            raise SimulatorError(f"READ(10) command idle differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline READ(10) with declared descriptor, prefix-acquired state and cause-0x04 DATA IN event; controlled selection of real ResumeCoroutine0 ECALL after interrupt; fixed instruction steps reach read engine return and next idle yield with matching command state. Sector bytes, subsequent command and physical IRQ timing untested.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **results,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched READ(10) engine return and next command idle yield")


if __name__ == "__main__":
    main()
