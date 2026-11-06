#!/usr/bin/env python3
"""Audit intentional C251 absolute-DATA aliases in an L251 map."""

from __future__ import annotations

import argparse
import re
from collections import defaultdict
from pathlib import Path


ABSOLUTE_DATA_RE = re.compile(
    r"^([0-9A-F]{6})H\s+([0-9A-F]{6})H\s+([0-9A-F]{6})H\s+"
    r"BYTE\s+AT\.\.\s+DATA\s+(\S+)$"
)
ALLOWED_ALIAS_ADDRESSES = set(range(0x20, 0x29))


def audit_map(text: str) -> tuple[int, int]:
    segments: dict[int, list[tuple[int, int, str]]] = defaultdict(list)
    errors: list[str] = []

    for raw_line in text.splitlines():
        match = ABSOLUTE_DATA_RE.match(raw_line.strip())
        if match is None:
            continue
        start, end, size = (int(match.group(index), 16) for index in range(1, 4))
        name = match.group(4)
        segments[start].append((end, size, name))

    for address, entries in segments.items():
        if len(entries) <= 1:
            continue
        if address not in ALLOWED_ALIAS_ADDRESSES or any(
            end != address or size != 1 or not name.startswith("?DT?AT_")
            for end, size, name in entries
        ):
            errors.append(
                f"unexpected absolute DATA overlap at 0x{address:06x}: "
                + ", ".join(name for _, _, name in entries)
            )

    if errors:
        raise ValueError("\n".join(errors))

    alias_segments = sum(
        len(entries) for address, entries in segments.items()
        if address in ALLOWED_ALIAS_ADDRESSES and len(entries) > 1
    )
    overlap_count = sum(
        len(entries) - 1 for address, entries in segments.items()
        if address in ALLOWED_ALIAS_ADDRESSES and len(entries) > 1
    )
    if set(
        address for address, entries in segments.items()
        if address in ALLOWED_ALIAS_ADDRESSES and len(entries) > 1
    ) != ALLOWED_ALIAS_ADDRESSES:
        raise ValueError("expected aliases at every address from 0x20 through 0x28")
    return alias_segments, overlap_count


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("map_file", type=Path)
    parser.add_argument("--stamp", type=Path)
    args = parser.parse_args()
    alias_segments, overlap_count = audit_map(
        args.map_file.read_text(encoding="ascii", errors="replace")
    )
    print(
        f"absolute DATA aliases: {alias_segments} segments, "
        f"{overlap_count} intentional overlaps in RAM:0020-RAM:0028"
    )
    if args.stamp is not None:
        args.stamp.write_text(
            f"segments={alias_segments}\noverlaps={overlap_count}\n",
            encoding="ascii",
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
