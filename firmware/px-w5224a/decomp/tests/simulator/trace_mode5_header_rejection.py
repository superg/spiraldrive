"""Check malformed main-image header rejection through command idle.

A declared media-state transition bypasses the firmware's media quiesce wait.
The controller-window copy uses the same explicit S251-only bank relocation as
the revision-rejection trace. Both substitutions affect debugger memory only.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path
from tests.simulator.trace_mode5_copy_entry import CASE_ID, prepare

CHECKPOINTS = {
    "stock": (0xFEE4FD, 0xFD2D40, 0xFAFF63, 0xFD2C92, 0xFDC215, 0xFDE0B5),
    "reconstructed": (0xFEAA52, 0xF82726, 0xF95D8A, 0xFAED71, 0xFCE1B6, 0xFAE37B),
}


def run_side(source: Path, output: Path, side: str, uv4: Path, timeout: int) -> dict[str, object]:
    resume, header, sense, handler, idle, bank_byte = CHECKPOINTS[side]
    project = prepare(source / side, output, resume, header, bank_byte)
    ini = output / "run.ini"
    content = ini.read_text()
    map_line = "MAP 0x010000,0x017fff READ WRITE EXEC\n"
    if content.count(map_line) != 1:
        raise SimulatorError(f"{side}: low-bank map anchor changed")
    content = content.replace(map_line, "MAP X:0x0000,X:0xffff READ WRITE\n" + map_line, 1)
    seed_line = "_WBYTE(E:0x10001b, 0x30)\n"
    if content.count(seed_line) != 1:
        raise SimulatorError(f"{side}: staged revision anchor changed")
    content = content.replace(
        seed_line,
        "_WBYTE(E:0x10001b, 0x39)\n"
        "_WBYTE(X:0x001a, 0x30)\n"
        "_WBYTE(X:0x001b, 0x39)\n"
        "_WBYTE(X:0x0030, 0x00)\n"
        "_WBYTE(E:0x000fae, 0x01)\n"
        f"_WBYTE(0x01601f, _RBYTE(C:0x{bank_byte:06x}))\n"
        f"_WBYTE(C:0x{bank_byte:06x}, 0x01)\n",
        1,
    )
    first_trace = wine_path(output / "mode5-copy-entry.hex").replace("\\", "\\\\")
    anchor = f'SAVE "{first_trace}" 0x016000,0x01601f\n'
    if content.count(anchor) != 1:
        raise SimulatorError(f"{side}: header-entry insertion point changed")
    trace = wine_path(output / "mode5-header-rejection.hex").replace("\\", "\\\\")
    lines = [
        "_WBYTE(0x016014, _RBYTE(E:0x000fae))",
        "_WBYTE(0x016015, _RBYTE(X:0x001a))",
        "_WBYTE(0x016016, _RBYTE(X:0x001b))",
        "_WBYTE(0x016017, _RBYTE(X:0x0030))",
        "_WBYTE(0x016018, _RBYTE(C:0xf80030))",
        f"_WBYTE(0x016019, _RBYTE(C:0x{bank_byte:06x}))",
        anchor.rstrip("\n"),
        "BK *",
        f"BS C:0x{sense:06x}",
        "BS C:0xff4000",
        "BS C:0x000800",
        "G",
        "_WDWORD(0x016040, $)",
        "_WDWORD(0x016044, DR4)",
        "_WBYTE(0x016048, ACC)",
        "BK *",
        f"BS C:0x{handler:06x}",
        "BS C:0xff4000",
        "BS C:0x000800",
        "G",
        "_WDWORD(0x016050, $)",
        "_WDWORD(0x016054, DR60)",
        "_WBYTE(0x016058, _RBYTE(E:0x000b18))",
        "_WBYTE(0x016059, _RBYTE(E:0x000b1a))",
        "_WBYTE(0x01605a, _RBYTE(E:0x000b24))",
        "_WBYTE(0x01605b, _RBYTE(E:0x000b25))",
        "BK *",
        f"BS C:0x{idle:06x}",
        "BS C:0xff4000",
        "BS C:0x000800",
        "G",
        "_WDWORD(0x016060, $)",
        "_WDWORD(0x016064, DR60)",
        "_WBYTE(0x016068, _RBYTE(E:0x000c32))",
        "_WBYTE(0x016069, _RBYTE(E:0x000c33))",
        "_WBYTE(0x01606a, _RBYTE(E:0x000b18))",
        "_WBYTE(0x01606b, _RBYTE(E:0x000b1a))",
        "_WBYTE(0x01606c, _RBYTE(E:0x000b24))",
        "_WBYTE(0x01606d, _RBYTE(E:0x000b25))",
        f'SAVE "{trace}" 0x016000,0x01607f',
    ]
    content = content.replace(anchor, "\n".join(lines) + "\n", 1)
    content = content.replace("twatch(20000)", "twatch(500000)")
    ini.write_text(content)
    run_uv4(uv4, project, output / "uv4-header-rejection.log", timeout)
    memory = read_intel_hex(output / "mode5-header-rejection.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")

    result = {
        "header_entry_pc": value(0x16000, 4) & 0xFFFFFF,
        "declared_media_state": value(0x16014, 1),
        "staged_revision": [value(0x16015 + i, 1) for i in range(2)],
        "staged_identity_first_byte": value(0x16017, 1),
        "installed_identity_first_byte": value(0x16018, 1),
        "original_bank_immediate": value(0x1601F, 1),
        "simulator_bank_immediate": value(0x16019, 1),
        "sense_call_pc": value(0x16040, 4) & 0xFFFFFF,
        "sense_key_argument": value(0x16048, 1),
        "sense_code_argument": value(0x16044, 4) & 0xFFFF,
        "handler_return_pc": value(0x16050, 4) & 0xFFFFFF,
        "handler_return_spx": value(0x16054, 4),
        "handler_sense": [value(0x16058 + i, 1) for i in range(4)],
        "idle_pc": value(0x16060, 4) & 0xFFFFFF,
        "idle_spx": value(0x16064, 4),
        "packet_state": value(0x16068, 1),
        "transfer_state": value(0x16069, 1),
        "idle_sense": [value(0x1606A + i, 1) for i in range(4)],
    }
    if (result["header_entry_pc"], result["sense_call_pc"], result["handler_return_pc"], result["idle_pc"]) != (header, sense, handler, idle):
        raise SimulatorError(f"{side}: header-rejection checkpoint missing: {result}")
    if (result["declared_media_state"], result["staged_revision"], result["staged_identity_first_byte"], result["installed_identity_first_byte"]) != (1, [0x30, 0x39], 0, 0x50):
        raise SimulatorError(f"{side}: declared header input differs: {result}")
    if (result["original_bank_immediate"], result["simulator_bank_immediate"]) != (0x10, 1):
        raise SimulatorError(f"{side}: simulator bank substitution differs: {result}")
    if (result["sense_key_argument"], result["sense_code_argument"]) != (4, 0x40F5):
        raise SimulatorError(f"{side}: malformed header did not select expected sense: {result}")
    if result["handler_sense"] != [0x70, 4, 0x40, 0xF5] or result["idle_sense"] != result["handler_sense"]:
        raise SimulatorError(f"{side}: malformed-header sense write differs: {result}")
    if not 0x300 <= result["handler_return_spx"] <= 0x33F or result["idle_spx"] != 0x300:
        raise SimulatorError(f"{side}: command stack did not unwind: {result}")
    if (result["packet_state"], result["transfer_state"]) != (1, 5):
        raise SimulatorError(f"{side}: command did not return to idle: {result}")
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
        raise SimulatorError("source run needs a passing versioned mode-5 first-yield case")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / CASE_ID, output / side, side, args.uv4.resolve(), args.timeout)
        for side in CHECKPOINTS
    }
    for field in ("declared_media_state", "staged_revision", "staged_identity_first_byte", "installed_identity_first_byte", "original_bank_immediate", "simulator_bank_immediate", "sense_key_argument", "sense_code_argument", "handler_sense", "idle_spx", "packet_state", "transfer_state", "idle_sense"):
        if observations["stock"][field] != observations["reconstructed"][field]:
            raise SimulatorError(f"mode-5 malformed-header field differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline mode-5 passing revision 09 and malformed main-image header rejection through 70/04/40/F5 sense and command idle; declared media state 1 and S251-only source-bank relocation; checksum, programming and physical flash behavior untested",
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched mode-5 malformed-header rejection through command idle")


if __name__ == "__main__":
    main()
