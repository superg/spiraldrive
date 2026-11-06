"""Continue auxiliary mode-5 checksum rejection through sense and command idle."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

POINTS = {
    "stock": {"idle": 0xFDC215},
    "reconstructed": {"idle": 0xFCE1B6},
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "aux-rejection-completion.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "aux-checksum-rejection.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x016040,0x01605f\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: checksum-rejection continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "BK *", f"BS C:0x{points['idle']:06x}",
                "BS C:0xff4000", "BS C:0x000800", "G",
                "_WDWORD(0x016060, $)",
                "_WDWORD(0x016064, states)",
                "_WDWORD(0x016068, DR60)",
                "_WBYTE(0x01606c, _RBYTE(E:0x000c32))",
                "_WBYTE(0x01606d, _RBYTE(E:0x000c33))",
                "_WBYTE(0x01606e, _RBYTE(E:0x000b18))",
                "_WBYTE(0x01606f, _RBYTE(E:0x000b1a))",
                "_WBYTE(0x016070, _RBYTE(E:0x000b24))",
                "_WBYTE(0x016071, _RBYTE(E:0x000b25))",
                f'SAVE "{trace}" 0x016040,0x01607f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-aux-rejection-completion.log", timeout)
    memory = read_intel_hex(output / "aux-rejection-completion.hex")
    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    result = {
        "sense_call_pc": value(0x16040, 4) & 0xFFFFFF,
        "sense_call_states": value(0x16044, 4),
        "idle_pc": value(0x16060, 4) & 0xFFFFFF,
        "idle_states": value(0x16064, 4),
        "idle_spx": value(0x16068, 4),
        "packet_state": value(0x1606C, 1),
        "transfer_state": value(0x1606D, 1),
        "sense": [value(0x1606E + i, 1) for i in range(4)],
    }
    if result["idle_pc"] != points["idle"]:
        raise SimulatorError(f"{output.name}: command idle checkpoint missing: {result}")
    if result["sense"] != [0x70, 4, 0x40, 0xF6]:
        raise SimulatorError(f"{output.name}: checksum rejection sense not written: {result}")
    if (result["idle_spx"], result["packet_state"], result["transfer_state"]) != (0x300, 1, 5):
        raise SimulatorError(f"{output.name}: command did not unwind to idle: {result}")
    if not 0 < result["idle_states"] - result["sense_call_states"] < 50000:
        raise SimulatorError(f"{output.name}: unbounded completion: {result}")
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
        raise SimulatorError("source run needs the passing current-image auxiliary checksum rejection")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in POINTS.items()
    }
    comparable = ("idle_spx", "packet_state", "transfer_state", "sense")
    differences = {key: (observations["stock"][key], observations["reconstructed"][key])
                   for key in comparable if observations["stock"][key] != observations["reconstructed"][key]}
    if differences:
        raise SimulatorError(f"auxiliary checksum rejection completion differs: {differences}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Latest-image auxiliary mode-5 checksum rejection writes 70/04/40/F6 sense and returns to command idle in both S251 images under declared staged data/media/revision model; no accepted programming or physical flash behavior proven.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "differences": differences,
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched auxiliary mode-5 checksum rejection through command idle")


if __name__ == "__main__":
    main()
