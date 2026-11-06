"""Test a declared coroutine-three reinitialization after configuration programming.

The stock and reconstructed paths have already returned from the relocated
programmer and yielded coroutine one. This script applies the exact RAM effects
of InitializeCoroutine3 at the next scheduler ResumeCoroutine3 call, then
checks whether both images reach the following ResumeCoroutine0. It does not
claim the firmware naturally performs this reinitialization at that point.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

APPLICATION_SHA256 = "7a848a669a864d8ebea33bb14149e3bdbf2a4f0c5a90c57a99d73948c74ffbfa"
POINTS = {
    "stock": {"resume3": 0xFDCF8E, "idle3": 0xF84B00, "resume0": 0xFEE4FD},
    "reconstructed": {"resume3": 0xFDCF77, "idle3": 0xFCE9B7, "resume0": 0xFEAA48},
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    observation = wine_path(output / "reinitialized-coroutine3.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            previous = wine_path(output / "status-yield.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x0173d0,0x0173ef\n'
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: status-yield continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                "BK *", f"BS C:0x{points['resume3']:06x}", "G",
                "_WDWORD(0x017450, $)", "_WDWORD(0x017454, states)",
                "_WDWORD(0x017458, _RDWORD(E:0x00116a))",
                "_WDWORD(0x01745c, _RDWORD(E:0x000900))",
            ]
            for index, value in enumerate(points["idle3"].to_bytes(4, "big")):
                lines.append(f"_WBYTE(E:0x{0x900 + index:06x}, 0x{value:02x})")
            for index, value in enumerate((0, 0, 9, 7)):
                lines.append(f"_WBYTE(E:0x{0x116a + index:06x}, 0x{value:02x})")
            lines += [
                "_WDWORD(0x017460, _RDWORD(E:0x000900))",
                "_WDWORD(0x017464, _RDWORD(E:0x00116a))",
                "BK *", f"BS C:0x{points['resume0']:06x}", "G",
                "_WDWORD(0x017468, $)", "_WDWORD(0x01746c, states)",
                "_WBYTE(0x017470, SP)",
                "_WDWORD(0x017474, _RDWORD(E:0x001162))",
                "_WDWORD(0x017478, _RDWORD(E:0x00116a))",
                "_WBYTE(0x01747c, _RBYTE(E:0x001009))",
                f'SAVE "{observation}" 0x017450,0x01747f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-reinitialized-coroutine3.log", timeout)
    memory = read_intel_hex(output / "reinitialized-coroutine3.hex")
    def dword(address: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(4)), "big")
    result = {
        "resume3_pc": dword(0x17450) & 0xFFFFFF,
        "resume3_states": dword(0x17454),
        "saved_spx_before": dword(0x17458),
        "return_slot_before": dword(0x1745c),
        "return_slot_after": dword(0x17460),
        "saved_spx_after": dword(0x17464),
        "next_resume0_pc": dword(0x17468) & 0xFFFFFF,
        "next_resume0_states": dword(0x1746c),
        "next_resume0_sp": memory[0x17470],
        "coroutine1_saved_spx": dword(0x17474),
        "coroutine3_saved_spx": dword(0x17478),
        "ram_1009": memory[0x1747c],
    }
    expected = {
        "resume3_pc": points["resume3"],
        "saved_spx_before": 0x907,
        "return_slot_before": 0xB455547E,
        "return_slot_after": points["idle3"],
        "saved_spx_after": 0x907,
        "next_resume0_pc": points["resume0"],
        "next_resume0_sp": 10,
        "coroutine1_saved_spx": 0x507,
        "coroutine3_saved_spx": 0x907,
        "ram_1009": 3,
    }
    if any(result[field] != value for field, value in expected.items()):
        raise SimulatorError(f"{output.name}: declared reinitialization differs: {result}")
    if not 0 < result["next_resume0_states"] - result["resume3_states"] < 10000:
        raise SimulatorError(f"{output.name}: unbounded resume progress: {result}")
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
        raise SimulatorError("source run needs the passing current-image status yield")
    if source_report["source_run_identity"]["application_sha256"] != APPLICATION_SHA256:
        raise SimulatorError("source run application identity changed")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in POINTS.items()
    }
    comparable = ("saved_spx_before", "return_slot_before", "saved_spx_after",
                  "next_resume0_sp", "coroutine1_saved_spx", "coroutine3_saved_spx", "ram_1009")
    differences = {name: (observations["stock"][name], observations["reconstructed"][name])
                   for name in comparable if observations["stock"][name] != observations["reconstructed"][name]}
    if differences:
        raise SimulatorError(f"stock/reconstruction differ after declared coroutine-three reinitialization: {differences}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Declared InitializeCoroutine3 RAM effects after configuration programmer return restore the fourth ResumeCoroutine0 checkpoint in both S251 images; no natural reset/reinitialization or physical RAM mapping proven.",
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "differences": differences,
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched stock and reconstructed fourth ResumeCoroutine0 after declared coroutine-three reinitialization")


if __name__ == "__main__":
    main()
