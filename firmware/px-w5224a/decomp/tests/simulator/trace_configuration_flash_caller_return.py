"""Compare the configuration caller return after a modeled 32-word flash batch.

The source trace has already verified the relocated programmer's final ERET.
This continuation stops at the enclosing ProgramAndVerifyDriveConfiguration
return in each image. It does not model physical flash busy timing or effects.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

RETURN_PCS = {"stock": 0xFD6663, "reconstructed": 0xF83FC2}


def run_side(source: Path, output: Path, side: str, uv4: Path, timeout: int) -> dict[str, int]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    observation = wine_path(output / "caller-return.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "return-steps.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x017000,0x01736f\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: return-step continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "BK *", f"BS C:0x{RETURN_PCS[side]:06x}", "G",
                "_WDWORD(0x017380, $)",
                "_WDWORD(0x017384, states)",
                "_WBYTE(0x017388, ACC)",
                "_WBYTE(0x017389, SP)",
                "_WBYTE(0x01738a, _RBYTE(E:0x001009))",
                "_WBYTE(0x01738b, _RBYTE(E:0x001692))",
                "_WBYTE(0x01738c, _RBYTE(E:0x0012f7))",
                "_WBYTE(0x01738d, _RBYTE(E:0x0012fc))",
                "_WBYTE(0x01738e, _RBYTE(E:0x0012fd))",
                "_WBYTE(0x01738f, _RBYTE(E:0x000fae))",
            ]
            for index in range(8):
                base = 0x17390 + 8 * index
                lines += [
                    "T", f"_WDWORD(0x{base:06x}, $)",
                    f"_WBYTE(0x{base + 4:06x}, ACC)",
                    f"_WBYTE(0x{base + 5:06x}, SP)",
                ]
            lines.append(f'SAVE "{observation}" 0x017380,0x0173cf')
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-caller-return.log", timeout)
    memory = read_intel_hex(output / "caller-return.hex")
    result = {
        "pc": int.from_bytes(bytes(memory[0x17380+i] for i in range(4)), "big") & 0xFFFFFF,
        "states": int.from_bytes(bytes(memory[0x17384+i] for i in range(4)), "big"),
        "acc": memory[0x17388], "sp": memory[0x17389],
        "ram_1009": memory[0x1738a], "ram_1692": memory[0x1738b],
        "ram_12f7": memory[0x1738c],
        "ram_12fc_12fd": (memory[0x1738d] << 8) | memory[0x1738e],
        "ram_0fae": memory[0x1738f],
        "successor_steps": [
            {
                "pc": int.from_bytes(bytes(memory[0x17390 + 8 * step + i] for i in range(4)), "big") & 0xFFFFFF,
                "acc": memory[0x17394 + 8 * step],
                "sp": memory[0x17395 + 8 * step],
            }
            for step in range(8)
        ],
    }
    if result["pc"] != RETURN_PCS[side] or result["states"] == 0 or result["successor_steps"][0]["pc"] == 0:
        raise SimulatorError(f"{side}: did not reach configuration caller return: {result}")
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
        raise SimulatorError("source run needs the passing current-image programmer-return trace")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / side, output / side, side, args.uv4.resolve(), args.timeout)
        for side in RETURN_PCS
    }
    fields = ("acc", "ram_1009", "ram_1692", "ram_12f7", "ram_12fc_12fd", "ram_0fae")
    differences = {field: [observations[side][field] for side in RETURN_PCS]
                   for field in fields if observations["stock"][field] != observations["reconstructed"][field]}
    report = {
        "schema_version": 1,
        "status": "pass" if not differences else "mismatch",
        "scope": "Current-image configuration caller return after declared direct-memory 32-word flash batch; physical flash effects and later scheduler return unverified.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "differences": differences,
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    if differences:
        raise SimulatorError(f"configuration caller return differs: {differences}")
    print("configuration caller returns with matching observed state")


if __name__ == "__main__":
    main()
