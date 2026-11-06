#!/usr/bin/env python3
"""Audit the physical L251 memory contract and report firmware capacity."""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import re


EXPECTED_RANGES = {
    "ECODE": (0xF80050, 0xFEB4FF),
    "HCONST": (0xFEB500, 0xFEFEFF),
    "CODE": (0xFEFF00, 0xFEFFFF),
    "XDATA": (0x010000, 0x017FFF),
}
MAIN_PAYLOAD_START = 0xF80050
MAIN_PAYLOAD_END = 0xFEFFFF

ACTIVE_CLASS = re.compile(
    r"^([0-9A-F]{6})H\s+([0-9A-F]{6})H\s+([0-9A-F]{6})H\s+"
    r"([0-9A-F]{6})H\s+(CODE|ECODE|HCONST|XDATA|DATA)\s*$",
    re.MULTILINE,
)
DIAGNOSTIC = re.compile(r"^\*\*\* (ERROR|WARNING) L(\d+):", re.MULTILINE)
PROGRAM_SIZE = re.compile(
    r"Program Size:\s+data=([0-9.]+)\s+edata\+hdata=(\d+)\s+"
    r"xdata=(\d+)\s+const=(\d+)\s+code=(\d+)")


@dataclass(frozen=True)
class MemoryClass:
    start: int
    end: int
    used: int
    capacity: int


@dataclass(frozen=True)
class Report:
    valid_physical_layout: bool
    layout_errors: list[str]
    classes: dict[str, MemoryClass]
    main_payload_start: int
    main_payload_end: int
    main_payload_capacity: int
    main_payload_used: int | None
    main_payload_deficit: int | None
    program_size: dict[str, int | float]
    linker_diagnostics: dict[str, int]


def parse_active_classes(text: str) -> dict[str, MemoryClass]:
    classes: dict[str, MemoryClass] = {}
    for _base, start, end, used, name in ACTIVE_CLASS.findall(text):
        first = int(start, 16)
        last = int(end, 16)
        classes[name] = MemoryClass(
            start=first,
            end=last,
            used=int(used, 16),
            capacity=last - first + 1,
        )
    return classes


def audit(map_path: Path, link_log: Path | None = None) -> Report:
    map_text = map_path.read_text(errors="replace")
    classes = parse_active_classes(map_text)
    errors: list[str] = []
    for name, expected in EXPECTED_RANGES.items():
        actual = classes.get(name)
        if actual is None:
            errors.append(f"missing active memory class {name}")
        elif (actual.start, actual.end) != expected:
            errors.append(
                f"{name} range is {actual.start:06x}..{actual.end:06x}; "
                f"expected {expected[0]:06x}..{expected[1]:06x}")

    flash_names = ("ECODE", "HCONST", "CODE")
    flash = [classes[name] for name in flash_names if name in classes]
    if len(flash) == len(flash_names):
        ordered = sorted(flash, key=lambda item: item.start)
        if ordered[0].start != MAIN_PAYLOAD_START:
            errors.append("linker-owned flash does not start at main payload")
        if ordered[-1].end != MAIN_PAYLOAD_END:
            errors.append("linker-owned flash does not end at main payload")
        for left, right in zip(ordered, ordered[1:]):
            if left.end + 1 != right.start:
                errors.append("linker-owned flash ranges overlap or have a gap")
    payload_used = (sum(item.used for item in flash)
                    if len(flash) == len(flash_names) else None)
    capacity = MAIN_PAYLOAD_END - MAIN_PAYLOAD_START + 1
    deficit = (max(0, payload_used - capacity)
               if payload_used is not None else None)

    size: dict[str, int | float] = {}
    match = PROGRAM_SIZE.search(map_text)
    if match:
        size = {
            "data": float(match.group(1)),
            "edata_hdata": int(match.group(2)),
            "xdata": int(match.group(3)),
            "const": int(match.group(4)),
            "code": int(match.group(5)),
        }

    diagnostics: dict[str, int] = {}
    if link_log is not None:
        counts = Counter(
            f"{kind.lower()}_L{number}"
            for kind, number in DIAGNOSTIC.findall(
                link_log.read_text(errors="replace")))
        diagnostics = dict(sorted(counts.items()))

    return Report(
        valid_physical_layout=not errors,
        layout_errors=errors,
        classes=classes,
        main_payload_start=MAIN_PAYLOAD_START,
        main_payload_end=MAIN_PAYLOAD_END,
        main_payload_capacity=capacity,
        main_payload_used=payload_used,
        main_payload_deficit=deficit,
        program_size=size,
        linker_diagnostics=diagnostics,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--map", type=Path, required=True)
    parser.add_argument("--link-log", type=Path)
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()
    report = audit(args.map, args.link_log)
    payload = asdict(report)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0 if report.valid_physical_layout else 1


if __name__ == "__main__":
    raise SystemExit(main())
