"""Continue the modeled mode-5 revision rejection through command idle.

The source trace has already checked the simulator-only controller-window bank
relocation, completed revision copy, and exact SetSenseData call arguments.
This continuation checks the resulting sense bytes and scheduler return.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

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
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            first_trace = wine_path(destination / "mode5-copy-entry.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{first_trace}" 0x016000,0x01601f\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: sense-entry insertion point changed")
            completion = wine_path(destination / "mode5-rejection-completion.hex").replace("\\", "\\\\")
            lines = [
                anchor.rstrip("\n"),
                "BK *",
                f"BS C:0x{handler:06x}",
                "BS C:0xff4000",
                "BS C:0x000800",
                "G",
                "_WDWORD(0x016040, $)",
                "_WDWORD(0x016044, DR60)",
                "_WBYTE(0x016048, _RBYTE(E:0x000b18))",
                "_WBYTE(0x016049, _RBYTE(E:0x000b1a))",
                "_WBYTE(0x01604a, _RBYTE(E:0x000b24))",
                "_WBYTE(0x01604b, _RBYTE(E:0x000b25))",
                "_WBYTE(0x01604c, _RBYTE(E:0x000c83))",
                "BK *",
                f"BS C:0x{idle:06x}",
                "BS C:0xff4000",
                "BS C:0x000800",
                "G",
                "_WDWORD(0x016050, $)",
                "_WDWORD(0x016054, DR60)",
                "_WBYTE(0x016058, _RBYTE(E:0x000c32))",
                "_WBYTE(0x016059, _RBYTE(E:0x000c33))",
                "_WBYTE(0x01605a, _RBYTE(E:0x000b18))",
                "_WBYTE(0x01605b, _RBYTE(E:0x000b1a))",
                "_WBYTE(0x01605c, _RBYTE(E:0x000b24))",
                "_WBYTE(0x01605d, _RBYTE(E:0x000b25))",
                f'SAVE "{completion}" 0x016040,0x01605f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    (destination / "mode5-rejection-completion.hex").unlink(missing_ok=True)
    return destination / "run.uvproj"


def decode(destination: Path, handler: int, idle: int) -> dict[str, object]:
    memory = read_intel_hex(destination / "mode5-rejection-completion.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")

    result = {
        "handler_return_pc": value(0x16040, 4) & 0xFFFFFF,
        "handler_return_spx": value(0x16044, 4),
        "handler_sense": [value(0x16048 + i, 1) for i in range(4)],
        "handler_completion_flags": value(0x1604C, 1),
        "idle_pc": value(0x16050, 4) & 0xFFFFFF,
        "idle_spx": value(0x16054, 4),
        "packet_state": value(0x16058, 1),
        "transfer_state": value(0x16059, 1),
        "idle_sense": [value(0x1605A + i, 1) for i in range(4)],
    }
    if result["handler_return_pc"] != handler or result["idle_pc"] != idle:
        raise SimulatorError(f"{destination.name}: command checkpoint missing: {result}")
    if not 0x300 <= result["handler_return_spx"] <= 0x33F or result["idle_spx"] != 0x300:
        raise SimulatorError(f"{destination.name}: command stack did not unwind: {result}")
    if result["handler_sense"] != [0x70, 4, 0x40, 0xF5] or result["idle_sense"] != result["handler_sense"]:
        raise SimulatorError(f"{destination.name}: revision sense differs: {result}")
    if result["handler_completion_flags"] != 0:
        raise SimulatorError(f"{destination.name}: completion flag remained set: {result}")
    if (result["packet_state"], result["transfer_state"]) != (1, 5):
        raise SimulatorError(f"{destination.name}: packet did not return to idle: {result}")
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
    if source_report.get("status") != "pass" or set(CHECKPOINTS) - set(source_report):
        raise SimulatorError("source run needs the passing mode-5 revision-rejection trace")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {}
    for side, (handler, idle) in CHECKPOINTS.items():
        destination = output / side
        project = prepare(source / side, destination, handler, idle)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-rejection-completion.log", args.timeout)
        observations[side] = decode(destination, handler, idle)
    for field in ("handler_sense", "handler_completion_flags", "idle_spx", "packet_state", "transfer_state", "idle_sense"):
        if observations["stock"][field] != observations["reconstructed"][field]:
            raise SimulatorError(f"mode-5 rejection field differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline mode-5 staged-revision rejection writes 70/04/40/F5 sense and reaches command idle in both images; source trace used explicit S251-only data-bank relocation; header/checksum, programming and physical flash behavior untested",
        "source_trace_sha256": source_report["script_sha256"],
        "source_case_sha256": source_report["source_case_sha256"],
        "source_run_identity": source_report["source_run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched mode-5 revision sense write, handler return, and command idle")


if __name__ == "__main__":
    main()
