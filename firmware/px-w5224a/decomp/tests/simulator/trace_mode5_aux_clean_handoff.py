"""Continue accepted auxiliary mode-5 update to pre-flash runtime handoff."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

CAPTURE_SHA256 = "7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80"
CAPTURE_PATH = Path(__file__).resolve().parents[2] / "data/read_memory_ff0000.debug"

POINTS = {
    "stock": {"stage_bank_byte": 0xFD68C9, "stage": 0xFD68BA},
    "reconstructed": {"stage_bank_byte": 0xF853A8, "stage": 0xF8538E},
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int, capture_config: bytes) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "preflash-handoff.hex").replace("\\", "\\\\")
    staged = wine_path(output / "staged-config.hex").replace("\\", "\\\\")
    staged_before = wine_path(output / "staged-config-before.hex").replace("\\", "\\\\")
    primary = wine_path(output / "primary-config.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source path")
        if name == "run.ini":
            previous = wine_path(output / "acceptance-checkpoint.hex").replace("\\", "\\\\")
            anchor = f'SAVE "{previous}" 0x01d040,0x01d05f\n'
            if content.count(anchor) != 1:
                raise SimulatorError("acceptance continuation point changed")
            lines = [
                anchor.rstrip("\n"),
                *(f"_WBYTE(C:0x{0xffc000 + offset:06x}, 0x{byte:02x})" for offset, byte in enumerate(capture_config)),
                f"_WBYTE(0x{points['stage_bank_byte']:06x}, 1)",
                f'SAVE "{staged_before}" X:0xc000,X:0xc05f',
                "BK *", "BS C:0xff4000", "BS C:0x000800", "G",
                "_WDWORD(0x01d080, $)", "_WDWORD(0x01d084, states)",
                "_WDWORD(0x01d088, DR0)", "_WDWORD(0x01d08c, DR4)",
                "_WDWORD(0x01d090, DR8)", "_WDWORD(0x01d094, DR60)",
                "_WBYTE(0x01d098, _RBYTE(X:0xc000))",
                "_WBYTE(0x01d099, _RBYTE(C:0xffc000))",
                f'SAVE "{trace}" 0x01d080,0x01d09f',
                f'SAVE "{staged}" X:0xc000,X:0xc05f',
                f'SAVE "{primary}" C:0xffc000,C:0xffc05f',
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n", 1)
            content = content.replace("twatch(5000000)", "twatch(7000000)")
        path.write_text(content)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-handoff.log", timeout)
    memory = read_intel_hex(output / "preflash-handoff.hex")
    staged_mem = read_intel_hex(output / "staged-config.hex")
    staged_before_mem = read_intel_hex(output / "staged-config-before.hex")
    primary_mem = read_intel_hex(output / "primary-config.hex")
    def val(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    staged_bytes = bytes(staged_mem[min(staged_mem) + i] for i in range(0x60))
    primary_bytes = bytes(primary_mem[min(primary_mem) + i] for i in range(0x60))
    before_bytes = bytes(staged_before_mem[min(staged_before_mem) + i] for i in range(0x60))
    return {
        "stop_pc": val(0x1D080, 4) & 0xFFFFFF,
        "states": val(0x1D084, 4),
        "dr0": val(0x1D088, 4), "dr4": val(0x1D08C, 4),
        "dr8": val(0x1D090, 4), "spx": val(0x1D094, 4),
        "staged_config_sha256": hashlib.sha256(staged_bytes).hexdigest(),
        "primary_config_sha256": hashlib.sha256(primary_bytes).hexdigest(),
        "config_equal": staged_bytes == primary_bytes,
        "staged_before_sha256": hashlib.sha256(before_bytes).hexdigest(),
        "staged_changed_bytes": sum(a != b for a, b in zip(before_bytes, staged_bytes)),
        "staged_first_byte_before": before_bytes[0],
        "staged_first_byte": val(0x1D098, 1),
        "primary_first_byte": val(0x1D099, 1),
    }


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
        raise SimulatorError("source run needs paired accepted checksum")
    capture = CAPTURE_PATH.read_bytes()
    if hashlib.sha256(capture).hexdigest() != CAPTURE_SHA256 or len(capture) != 0x10000:
        raise SimulatorError("runtime capture identity or length changed")
    capture_config = capture[0xC000:0xC060]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {side: run_side(source / side, output / side, points, args.uv4.resolve(), args.timeout, capture_config)
                    for side, points in POINTS.items()}
    report = {
        "schema_version": 1,
        "status": "pass" if all(item["stop_pc"] == 0xFF4000 and item["dr0"] == 0x00050000 and item["dr4"] == 0 and item["dr8"] == 0xFF4000 and item["config_equal"] and item["staged_config_sha256"] == hashlib.sha256(capture_config).hexdigest() and item["staged_changed_bytes"] > 0 for item in observations.values()) else "mismatch",
        "scope": "Conditional S251 auxiliary update copies the 96-byte captured primary configuration into auxiliary staging and reaches the pre-flash operation-5 handoff after bank-0x10-to-0x01 substitution. Stops before executing flash loader. Physical flash effect unverified.",
        "runtime_capture_sha256": CAPTURE_SHA256,
        "declared_config_sha256": hashlib.sha256(capture_config).hexdigest(),
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    if report["status"] != "pass":
        raise SimulatorError(f"auxiliary handoff mismatch: {observations}")
    print(json.dumps(observations, indent=2))


if __name__ == "__main__":
    main()
