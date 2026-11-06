"""Trace auxiliary checksum acceptance with identical declared staging bytes.

Starts from the paired clean-scratch checksum-entry trace, replaces the one
implementation-specific candidate byte, and sets the additive checksum word.
Stops at auxiliary configuration staging, checksum rejection, or flash handoff.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

POINTS = {
    "stock": {"stage": 0xFD68BA, "sense": 0xFAFF63, "checksum_bank_bytes": [0xFD2CAA, 0xFD2CF8, 0xFD2D19]},
    "reconstructed": {"stage": 0xF8538E, "sense": 0xF95D8A, "checksum_bank_bytes": [0xF84FCD, 0xF85019, 0xF85039]},
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    normalized = wine_path(output / "normalized-stage.hex").replace("\\", "\\\\")
    trace = wine_path(output / "acceptance-checkpoint.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source path")
        if name == "run.ini":
            previous = wine_path(output / "clean-scratch-stage.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" X:0x0000,X:0xffff\n'
            if content.count(anchor) != 1:
                raise SimulatorError("clean-entry continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "_WBYTE(X:0x1a57, 0)",
                "_WBYTE(X:0xfffe, 0)", "_WBYTE(X:0xffff, 1)",
                *(f"_WBYTE(0x{address:06x}, 1)" for address in points["checksum_bank_bytes"]),
                f'SAVE "{normalized}" X:0x0000,X:0xffff',
                "BK *", f"BS C:0x{points['stage']:06x}", f"BS C:0x{points['sense']:06x}",
                "BS C:0xff4000", "BS C:0x000800", "G",
                "_WDWORD(0x01d040, $)", "_WDWORD(0x01d044, states)",
                "_WDWORD(0x01d048, DR0)", "_WDWORD(0x01d04c, DR4)",
                "_WBYTE(0x01d050, ACC)",
                "_WBYTE(0x01d051, _RBYTE(X:0x1a57))",
                "_WBYTE(0x01d052, _RBYTE(X:0xfffe))",
                "_WBYTE(0x01d053, _RBYTE(X:0xffff))",
                f'SAVE "{trace}" 0x01d040,0x01d05f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
            content = content.replace("twatch(20000)", "twatch(5000000)")
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-acceptance.log", timeout)
    memory = read_intel_hex(output / "acceptance-checkpoint.hex")
    stage = read_intel_hex(output / "normalized-stage.hex")
    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    base = min(stage)
    candidate = bytes(stage[base + i] for i in range(0x8000)) + bytes(stage[base + i] for i in range(0xFF00, 0x10000))
    result = {
        "stop_pc": value(0x1D040, 4) & 0xFFFFFF,
        "states": value(0x1D044, 4),
        "dr0": value(0x1D048, 4), "dr4": value(0x1D04C, 4), "acc": value(0x1D050, 1),
        "candidate_sha256": hashlib.sha256(candidate).hexdigest(),
        "candidate_sum": sum(candidate[:-2]) & 0xFFFF,
        "stored_word": list(candidate[-2:]),
        "staged_byte_1a57_at_stop": value(0x1D051, 1),
    }
    if result["candidate_sum"] != 1 or result["stored_word"] != [0, 1]:
        raise SimulatorError(f"{output.name}: normalized candidate wrong: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--uv4", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=90)
    args = parser.parse_args()
    source = args.source_run.resolve()
    source_report = json.loads((source / "results.json").read_text())
    if source_report.get("status") != "pass":
        raise SimulatorError("source run needs paired clean-scratch entry")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout)
                    for side, points in POINTS.items()}
    if observations["stock"]["candidate_sha256"] != observations["reconstructed"]["candidate_sha256"]:
        raise SimulatorError("declared candidate bytes differ")
    report = {
        "schema_version": 1, "status": "pass" if all(observations[side]["stop_pc"] == POINTS[side]["stage"] for side in POINTS) else "mismatch",
        "scope": "S251 auxiliary checksum acceptance on identical declared candidate bytes with debugger-only bank-0x10-to-0x01 substitution at the three checksum source reads, after relocating observation scratch. Stops before configuration staging or flash handoff. Physical staged-buffer mapping unverified.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    if report["status"] != "pass":
        raise SimulatorError(f"auxiliary checksum acceptance mismatch: {observations}")
    print(json.dumps(observations, indent=2))


if __name__ == "__main__":
    main()
