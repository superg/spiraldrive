"""Continue a zero-length auxiliary WRITE BUFFER update to checksum validation.

The scenario declares media state one and a zero minimum auxiliary revision
after the authentic first yield. The latter makes the firmware skip staged
revision copying and enter its auxiliary checksum path without S251's known
high-bank controller-window copy limitation. No flash programming is allowed.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

CASE_ID = "write-buffer-mode5-auxiliary-zero-length-finalize-first-yield"
POINTS = {
    "stock": {"resume": 0xFEE4FD, "checksum": 0xFD2C93, "entry_spx": 0x312},
    "reconstructed": {"resume": 0xFEAA52, "checksum": 0xF84FB4, "entry_spx": 0x317},
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "aux-checksum-entry.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            map_line = "MAP 0x010000,0x017fff READ WRITE EXEC\n"
            if content.count(map_line) != 1:
                raise SimulatorError(f"{path}: low-bank map changed")
            content = content.replace(map_line, "MAP X:0x0000,X:0xffff READ WRITE\n" + map_line, 1)
            anchor = "G C:0xf70000\n_WBYTE(0x017003, 0)\n"
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: first-yield insertion point changed")
            lines = [
                "G C:0xf70000", "_WBYTE(0x017003, 0)",
                "_WBYTE(0xffc03a, 0)",
                "_WBYTE(E:0x000fae, 1)",
                "_WBYTE(X:0x0000, 1)",
                "_WBYTE(X:0xfffe, 0)",
                "_WBYTE(X:0xffff, 0)",
                "BK *", f"BS C:0x{points['checksum']:06x}",
                "BS C:0xff4000", "BS C:0x000800",
                f"$ = C:0x{points['resume']:06x}", "G",
                "_WDWORD(0x016000, $)",
                "_WDWORD(0x016004, states)",
                "_WDWORD(0x016008, DR0)",
                "_WDWORD(0x01600c, DR4)",
                "_WDWORD(0x016010, DR60)",
                "_WBYTE(0x016014, _RBYTE(E:0x000b2a))",
                "_WBYTE(0x016015, _RBYTE(E:0x000fae))",
                "_WBYTE(0x016016, _RBYTE(X:0x0000))",
                "_WBYTE(0x016017, _RBYTE(X:0xfffe))",
                "_WBYTE(0x016018, _RBYTE(X:0xffff))",
                "_WBYTE(0x016019, _RBYTE(0xffc03a))",
                f'SAVE "{trace}" 0x016000,0x01601f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-aux-checksum-entry.log", timeout)
    memory = read_intel_hex(output / "aux-checksum-entry.hex")
    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    result = {
        "checksum_pc": value(0x16000, 4) & 0xFFFFFF,
        "states": value(0x16004, 4),
        "start_offset": value(0x16008, 4),
        "checksum_word_offset": value(0x1600C, 4),
        "spx": value(0x16010, 4),
        "minimum_revision": value(0x16014, 1),
        "media_state": value(0x16015, 1),
        "staged_first_byte": value(0x16016, 1),
        "staged_checksum_word": [value(0x16017 + i, 1) for i in range(2)],
        "configured_minimum_revision": value(0x16019, 1),
    }
    expected = {
        "checksum_pc": points["checksum"], "start_offset": 0,
        "checksum_word_offset": 0xFFFE, "spx": points["entry_spx"],
        "minimum_revision": 0, "media_state": 1,
        "staged_first_byte": 1, "staged_checksum_word": [0, 0],
        "configured_minimum_revision": 0,
    }
    if any(result[field] != expected[field] for field in expected):
        raise SimulatorError(f"{output.name}: auxiliary checksum entry differs: {result}")
    if not 0x300 <= result["spx"] <= 0x33F or not 1500 < result["states"] < 100000:
        raise SimulatorError(f"{output.name}: auxiliary checkpoint stack or progress invalid: {result}")
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
        raise SimulatorError("source run needs the passing auxiliary mode-5 first-yield case")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / CASE_ID / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in POINTS.items()
    }
    comparable = ("start_offset", "checksum_word_offset", "minimum_revision",
                  "media_state", "staged_first_byte", "staged_checksum_word", "configured_minimum_revision")
    differences = {key: (observations["stock"][key], observations["reconstructed"][key])
                   for key in comparable if observations["stock"][key] != observations["reconstructed"][key]}
    if differences:
        raise SimulatorError(f"auxiliary checksum entry differs: {differences}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Latest-image zero-length auxiliary mode-5 packet reaches ValidateStagedFirmwareChecksum with DR0=0 and DR4=0xfffe after declared media state and minimum revision. Function-local entry SPX differs by implementation (stock 0x312, reconstruction 0x317); no checksum decision or physical controller-window data proven.",
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "differences": differences,
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched auxiliary mode-5 checksum entry in both images")


if __name__ == "__main__":
    main()
