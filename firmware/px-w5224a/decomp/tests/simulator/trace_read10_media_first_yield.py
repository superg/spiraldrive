"""Resume READ(10)'s initialized media coroutine through its first yield.

The source case returns the command coroutine to a captured scheduler context.
This trace selects the scheduler's actual ResumeCoroutine2 ECALL at that
checkpoint. It proves the media task's entry and first yield, including the
call frame needed for its later scheduler return.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

CASE_ID = "host-command-read10-first-media-yield-returns-scheduler"
CHECKPOINTS = {
    "stock": (0xFDCF87, 0xF90000, 0xFEE53F),
    "reconstructed": (0xFDCF6A, 0xFDE81B, 0xFEAA94),
}


def prepare(source: Path, destination: Path, resume: int, entry: int, yield2: int) -> Path:
    shutil.copytree(source, destination)
    old = wine_path(source.resolve())
    new = wine_path(destination.resolve())
    for name in ("run.ini", "run.uvproj"):
        path = destination / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            content = content.replace("twatch(30000)", "twatch(500000)")
            anchor = "G C:0xf70000\n_WBYTE(0x017003, 0)\n"
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: scheduler insertion point changed")
            trace = wine_path(destination / "read10-media-first-yield.hex").replace("\\", "\\\\")
            lines = [
                "G C:0xf70000",
                "_WBYTE(0x017003, 0)",
                "BK *",
                f"BS C:0x{entry:06x}",
                f"$ = C:0x{resume:06x}",
                "G",
                "_WDWORD(0x016000, $)",
                "_WDWORD(0x016004, DR60)",
                "_WDWORD(0x016008, _RDWORD(E:0x00115a))",
                "_WDWORD(0x01600c, _RDWORD(E:0x001166))",
                "_WDWORD(0x016010, _RDWORD(E:0x001598))",
                "BK *",
                f"BS C:0x{yield2:06x}",
                "G",
                "_WDWORD(0x016020, $)",
                "_WDWORD(0x016024, DR60)",
                "_WDWORD(0x016028, _RDWORD(E:0x001166))",
                "_WDWORD(0x01602c, _RDWORD(E:0x001598))",
                "_WBYTE(0x016030, _RBYTE(E:0x000f05))",
                "_WBYTE(0x016031, _RBYTE(E:0x000e44))",
                "_WBYTE(0x016032, _RBYTE(E:0x00127e))",
                "_WDWORD(0x016034, _RDWORD(E:0x00156a))",
                "_WDWORD(0x016038, _RDWORD(E:0x00129a))",
                f'SAVE "{trace}" 0x016000,0x01603f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    (destination / "read10-media-first-yield.hex").unlink(missing_ok=True)
    return destination / "run.uvproj"


def decode(destination: Path, entry: int, yield2: int) -> dict[str, object]:
    memory = read_intel_hex(destination / "read10-media-first-yield.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")

    result = {
        "media_entry_pc": value(0x16000, 4) & 0xFFFFFF,
        "media_entry_spx": value(0x16004, 4),
        "scheduler_spx": value(0x16008, 4),
        "coroutine2_initial_spx": value(0x1600C, 4),
        "media_entry_target": value(0x16010, 4),
        "media_first_yield_pc": value(0x16020, 4) & 0xFFFFFF,
        "media_first_yield_spx": value(0x16024, 4),
        "coroutine2_saved_spx_before_yield": value(0x16028, 4),
        "media_yield_target": value(0x1602C, 4),
        "pipeline_state": [value(0x16030 + i, 1) for i in range(3)],
        "ring_slot_count": value(0x16034, 4),
        "ring_limits": value(0x16038, 4),
    }
    if (result["media_entry_pc"], result["media_first_yield_pc"]) != (entry, yield2):
        raise SimulatorError(f"{destination.name}: media coroutine checkpoint missing: {result}")
    if (result["media_entry_spx"], result["scheduler_spx"], result["coroutine2_initial_spx"]) != (0x700, 0x0D, 0x707):
        raise SimulatorError(f"{destination.name}: media coroutine entry context differs: {result}")
    if not 0x700 <= result["media_first_yield_spx"] <= 0x73F:
        raise SimulatorError(f"{destination.name}: media coroutine stack range differs: {result}")
    if (result["media_entry_target"], result["media_yield_target"]) != (150, 150):
        raise SimulatorError(f"{destination.name}: READ(10) media target changed: {result}")
    if (result["pipeline_state"], result["ring_slot_count"], result["ring_limits"]) != ([2, 1, 1], 512, 0x01FF01FA):
        raise SimulatorError(f"{destination.name}: media setup differs: {result}")
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
    for side, (resume, entry, yield2) in CHECKPOINTS.items():
        destination = output / side
        project = prepare(source / CASE_ID / side, destination, resume, entry, yield2)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-media-first-yield.log", args.timeout)
        observations[side] = decode(destination, entry, yield2)
    for field in ("media_entry_spx", "scheduler_spx", "coroutine2_initial_spx", "media_entry_target", "coroutine2_saved_spx_before_yield", "media_yield_target", "pipeline_state", "ring_slot_count", "ring_limits"):
        if observations["stock"][field] != observations["reconstructed"][field]:
            raise SimulatorError(f"READ(10) media state differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline READ(10) packet yields to captured scheduler; selection of the real scheduler ResumeCoroutine2 ECALL enters primary media coroutine and reaches its first YieldCoroutine2 with matched target and ring setup; scheduler return from that media yield, sector transfer and command completion untested by this trace",
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched READ(10) media coroutine entry and first yield")


if __name__ == "__main__":
    main()
