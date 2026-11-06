"""Execute York 1.07 buffer-position selection in Keil S251, without hardware.

Run on Windows with Keil C251/uVision; the runner starts uVision hidden. The correction
exists only in simulator memory; this tool does not generate flashable images.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from itertools import product
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tests.simulator.simulator import build_stock_application, create_project, debugger_path, read_intel_hex, run_uv4

YORK_SHA256 = "79553d4e8d8143c5d0d00f72e8ec274985fd292a151cc569d7ea5569b251c978"
ENTRY, END = 0xFEC1B2, 0xFEC2BA
LOADS = (0xFEC212, 0xFEC23A)
ORIGINAL_LOAD = bytes.fromhex("49 03 ff ff 49 13 00 01 9c 00")
# A251 MODSRC: MOVS WR0,R1 sign-extends R1 into WR0, retaining R1.
# The two-byte replacement therefore preserves all instruction boundaries.
SIGNED_LOAD = ORIGINAL_LOAD[:8] + bytes.fromhex("1a 01")
REUSE, EMPTY_REUSE, RESET = 0xFEC283, 0xFEC287, 0xFEC28C


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--output", type=Path, default=ROOT / "out/agent-tests/york-d8/s251")
    ap.add_argument("--keil-root", type=Path, default=Path("C:/Keil_v5/C251"))
    args = ap.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    firmware = (ROOT / "data/York_107.bin").read_bytes()
    assert hashlib.sha256(firmware).hexdigest() == YORK_SHA256
    for address in LOADS:
        assert firmware[address - 0xF80000:address - 0xF80000 + 10] == ORIGINAL_LOAD
    application = build_stock_application(
        ROOT / "data/York_107.bin", {"body_ranges": [f"RAM:{ENTRY:06x}..RAM:{END - 1:06x}"]},
        output / "application", ROOT / "tools/keil_driver.py", args.keil_root / "BIN/A251.exe",
        args.keil_root / "BIN/L251.exe", args.keil_root,
    )
    cases = [
        ("positive_buffer_hit", 100, 101, 4, 104),
        ("positive_large_buffer_hit", 450000, 450001, 4, 450004),
        ("negative_buffer_hit", -20003, -20002, 4, -19999),
        ("negative_upper_buffer_hit", -1005, -1004, 4, -1001),
        ("negative_target_before_buffer", -20000, -20001, 4, -19996),
        ("negative_producer_cursor", -20003, -19999, 4, -19999),
        ("negative_target_past_producer", -20003, -19998, 4, -19999),
        ("negative_empty_at_producer", -20003, -20003, 0, -20003),
    ]
    results = []
    for corrected, case in product((False, True), cases):
        variant = "sign_extended" if corrected else "stock"
        dest = output / variant / case[0]
        dest.mkdir(parents=True, exist_ok=True)
        lines = [
            f'LOG >"{debugger_path(dest / "debugger.log")}"',
            "signal void YorkLimit(void) { twatch(10000); _break_ = 1; }",
            "MAP 0x000000,0x00ffff READ WRITE EXEC",
            "MAP E:0x0420,E:0xffff READ WRITE",
            "MAP 0x010000,0x017fff READ WRITE EXEC",
            "MAP 0xf70000,0xf700ff READ WRITE EXEC",
            "MAP E:0xf80000,E:0xffffff READ WRITE EXEC",
            "MAP C:0xf80000,C:0xffffff READ WRITE EXEC",
            "RESET", "BK *",
        ]

        def put(address: int, data: bytes, low: bool = False) -> None:
            space = "E:" if low else "C:"
            lines.extend(f"_WBYTE({space}0x{address + i:06x}, 0x{v:02x})" for i, v in enumerate(data))

        put(ENTRY, firmware[ENTRY - 0xF80000:END - 0xF80000])
        if corrected:
            for address in LOADS:
                put(address, SIGNED_LOAD)
        for address in (REUSE, EMPTY_REUSE, RESET):
            lines.append(f"BS C:0x{address:06x}")
        for index, (name, oldest, target, count, producer_target) in enumerate([case]):
            control = bytearray(16)
            for off, val in ((0, count), (2, 0), (4, count), (6, 0x1234), (8, 255), (10, 250), (14, 0x8944)):
                control[off:off + 2] = val.to_bytes(2, "big")
            put(0x1112, control, True)
            put(0xEEF, b"\x01", True)  # alternate producer is active
            put(0x153F, (producer_target & 0xFFFFFFFF).to_bytes(4, "big"), True)
            put(0x8943, b"\xa5", True)  # byte before first descriptor is deliberately nonzero
            for slot in range(count):
                descriptor = bytearray(20)
                descriptor[:3] = ((oldest + slot) & 0xFFFFFF).to_bytes(3, "big")
                put(0x8944 + slot * 20, descriptor, True)
            base = 0x16000 + index * 24
            lines.extend([
                "DR60 = 0x520", "DR0 = 0xcccccccc", "DR8 = 0x88888888",
                "DR12 = 0xdddddddd", f"DR4 = 0x{target & 0xFFFFFFFF:08x}",
                f"$ = C:0x{ENTRY:06x}", "YorkLimit()", f"G C:0x{ENTRY:06x},C:0x{EMPTY_REUSE:06x}",
                f"_WDWORD(0x{base:06x}, $)",
                f"_WDWORD(0x{base + 4:06x}, DR0)",
                f"_WDWORD(0x{base + 8:06x}, DR12)",
                f"_WDWORD(0x{base + 12:06x}, DR60)",
                f"_WBYTE(0x{base + 16:06x}, _RBYTE(E:0x001118))",
                f"_WBYTE(0x{base + 17:06x}, _RBYTE(E:0x001119))",
            ])
        marker = dest / "result.hex"
        lines += [f'SAVE "{debugger_path(marker)}" 0x016000,0x0160bf', "LOG OFF", "EXIT"]
        initialization = dest / "run.ini"
        initialization.write_text("\n".join(lines) + "\n", encoding="ascii")
        project = dest / "run.uvproj"
        create_project(args.keil_root / "Examples/Blinky/BLINKY.uvproj", project, initialization, application)
        marker.unlink(missing_ok=True)
        run_uv4(args.keil_root.parent / "UV4/UV4.exe", project, dest / "run.log", 45)
        mem = read_intel_hex(marker)
        for index, (name, oldest, target, count, producer_target) in enumerate([case]):
            base = 0x16000 + index * 24
            word = lambda off, n=4: int.from_bytes(bytes(mem[base + off + j] for j in range(n)), "big")
            hit = count and oldest <= target < oldest + count
            at_producer = target == producer_target
            expected = REUSE if hit or (count and at_producer) else EMPTY_REUSE if not count and at_producer else RESET
            if not corrected and count and target < 0:
                expected = RESET
            pc = word(0) & 0xFFFFFF
            cursor = word(16, 2)
            expected_cursor = target - oldest if hit else count if at_producer else 0x1234
            if expected == RESET:
                expected_cursor = 0x1234
            row = {"variant": variant, "case": name, "target": target, "oldest": oldest,
                   "pc": hex(pc), "expected_pc": hex(expected), "loaded_dr0": hex(word(4)),
                   "cursor": cursor, "expected_cursor": expected_cursor, "spx": hex(word(12)),
                   "pass": pc == expected and cursor == expected_cursor and word(12) == 0x524}
            results.append(row)
    report = {"firmware_sha256": YORK_SHA256, "scope": "Synthetic descriptor queue, actual York machine code; stops before reset or backlog helper, no controller or physical timing model", "results": results}
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))
    return 0 if all(r["pass"] for r in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
