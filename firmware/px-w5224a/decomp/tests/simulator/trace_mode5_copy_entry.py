"""Trace the mode-5 revision copy up to its first controller-window read.

The continuation starts from the versioned first scheduler yield. It declares
configuration revision 2 and staged ASCII revision 00, then checks the native
copy arguments and source address before any flash loader can run.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

CASE_ID = "write-buffer-mode5-zero-length-finalize-first-yield"
CHECKPOINTS = {
    "stock": (0xFEE4FD, 0xFDE0BA, 0xFDE0B5),
    "reconstructed": (0xFEAA52, 0xFAE390, 0xFAE37B),
}


def prepare(source: Path, destination: Path, resume: int, first_read: int, immediate: int) -> Path:
    shutil.copytree(source, destination)
    old = wine_path(source.resolve())
    new = wine_path(destination.resolve())
    for name in ("run.ini", "run.uvproj"):
        path = destination / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            anchor = "G C:0xf70000\n_WBYTE(0x017003, 0)\n"
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: first-yield insertion point changed")
            trace = wine_path(destination / "mode5-copy-entry.hex").replace("\\", "\\\\")
            lines = [
                "G C:0xf70000",
                "_WBYTE(0x017003, 0)",
                "_WBYTE(0xffc03b, 2)",
                "_WBYTE(E:0x10001a, 0x30)",
                "_WBYTE(E:0x10001b, 0x30)",
                "BK *",
                f"BS C:0x{first_read:06x}",
                "BS C:0xff4000",
                "BS C:0x000800",
                f"$ = C:0x{resume:06x}",
                "G",
                "_WDWORD(0x016000, $)",
                "_WDWORD(0x016004, DR60)",
                "_WDWORD(0x016008, DR0)",
                "_WDWORD(0x01600c, DR4)",
                "_WDWORD(0x016010, DR8)",
                f"_WBYTE(0x016014, _RBYTE(C:0x{immediate:06x}))",
                "_WBYTE(0x016015, _RBYTE(0xffc03b))",
                "_WBYTE(0x016016, _RBYTE(E:0x10001a))",
                "_WBYTE(0x016017, _RBYTE(E:0x10001b))",
                f'SAVE "{trace}" 0x016000,0x01601f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    (destination / "mode5-copy-entry.hex").unlink(missing_ok=True)
    return destination / "run.uvproj"


def decode(destination: Path, first_read: int, side: str) -> dict[str, object]:
    memory = read_intel_hex(destination / "mode5-copy-entry.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + index] for index in range(width)), "big")

    result = {
        "first_read_pc": value(0x016000, 4) & 0xFFFFFF,
        "spx": value(0x016004, 4),
        "dr0": value(0x016008, 4),
        "dr4": value(0x01600C, 4),
        "dr8": value(0x016010, 4),
        "bank_add_immediate": value(0x016014, 1),
        "minimum_revision": value(0x016015, 1),
        "staged_revision": [value(0x016016 + index, 1) for index in range(2)],
    }
    destination_register = result["dr0"] if side == "stock" else result["dr8"]
    source_register = result["dr8"] if side == "stock" else result["dr0"]
    if result["first_read_pc"] != first_read or not 0x300 <= result["spx"] <= 0x33F:
        raise SimulatorError(f"{side}: revision copy checkpoint missing: {result}")
    if (destination_register, source_register, result["dr4"]) != (0xB2A, 0x100000, 0x30):
        raise SimulatorError(f"{side}: controller copy address/return register differs: {result}")
    if (result["bank_add_immediate"], result["minimum_revision"], result["staged_revision"]) != (0x10, 2, [0x30, 0x30]):
        raise SimulatorError(f"{side}: revision input or bank immediate differs: {result}")
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
        raise SimulatorError("source run needs the passing versioned mode-5 first-yield case")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    traces = {}
    for side, (resume, first_read, immediate) in CHECKPOINTS.items():
        destination = output / side
        project = prepare(source_run / CASE_ID / side, destination, resume, first_read, immediate)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-copy-entry.log", args.timeout)
        traces[side] = decode(destination, first_read, side)
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline mode-5 zero-length finalization through the first revision-copy read with declared revision bytes; copy completion, rejection, programming and activation untested",
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "stock": traces["stock"],
        "reconstructed": traces["reconstructed"],
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched mode-5 first revision-copy read and physical controller-window source")


if __name__ == "__main__":
    main()
