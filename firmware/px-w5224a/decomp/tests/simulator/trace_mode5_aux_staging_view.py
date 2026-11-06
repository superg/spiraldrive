"""Capture the S251 XDATA view at auxiliary staged-checksum entry.

The debugger maps X:0000..ffff to E:010000..01ffff for this scenario. The
X:6000..7fff part of the candidate checksum range also holds simulator trace
scratch, so a matching stored word cannot be inferred from a single staged
byte. This records that model limit without claiming a checksum result.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path


def run_side(source: Path, output: Path, uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    stage_path = wine_path(output / "stage-x.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "aux-checksum-entry.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x016000,0x01601f\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: checksum-entry insertion point changed")
            content = content.replace(anchor, anchor + f'SAVE "{stage_path}" X:0x0000,X:0xffff\n', 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-aux-stage-view.log", timeout)
    memory = read_intel_hex(output / "stage-x.hex")
    if len(memory) != 0x10000 or max(memory) - min(memory) != 0xFFFF:
        raise SimulatorError(f"{output.name}: incomplete XDATA view")
    base = min(memory)
    def byte(offset: int) -> int:
        return memory[base + offset]
    regions = ((0, 0x2000), (0x2000, 0x4000), (0x4000, 0x6000),
               (0x6000, 0x8000), (0xFF00, 0xFFFE))
    sums = {f"{start:04x}_{end - 1:04x}": sum(byte(i) for i in range(start, end)) & 0xFFFF
            for start, end in regions}
    result = {
        "xdata_hex_base": base,
        "staged_first_byte": byte(0),
        "stored_checksum_word": [byte(0xFFFE), byte(0xFFFF)],
        "scratch_pc_bytes_6000_6003": [byte(0x6000 + i) for i in range(4)],
        "region_sums": sums,
        "candidate_sum": (sum(byte(i) for i in range(0x8000))
                          + sum(byte(i) for i in range(0xFF00, 0xFFFE))) & 0xFFFF,
        "scratch_nonzero_count_6000_7fff": sum(byte(i) != 0 for i in range(0x6000, 0x8000)),
    }
    if result["staged_first_byte"] != 1 or result["stored_checksum_word"] != [0, 0]:
        raise SimulatorError(f"{output.name}: source staged input changed: {result}")
    if result["scratch_nonzero_count_6000_7fff"] < 16:
        raise SimulatorError(f"{output.name}: expected scratch overlap missing: {result}")
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
        raise SimulatorError("source run needs the passing auxiliary checksum-entry trace")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / side, output / side, args.uv4.resolve(), args.timeout)
        for side in ("stock", "reconstructed")
    }
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Diagnostic S251 XDATA staging view at auxiliary checksum entry: simulator trace scratch occupies X:6000..7fff inside the candidate checksum range. The captured debugger-view sums are not validated CPU checksum results or physical staged-image bytes.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("captured stock and reconstructed auxiliary XDATA staging views")


if __name__ == "__main__":
    main()
