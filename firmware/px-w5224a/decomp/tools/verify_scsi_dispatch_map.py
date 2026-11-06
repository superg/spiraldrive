#!/usr/bin/env python3
"""Verify the symbolic SCSI opcode map against MONT_104.bin."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


ENTRY_RE = re.compile(r"^X\((0x[0-9a-fA-F]+)u,\s*([A-Za-z_]\w*)\)$")
OPCODE_RE = re.compile(r"^X\((0x[0-9a-fA-F]+)u\)$")
RAM_RE = re.compile(r"RAM:([0-9a-fA-F]+)")
BYTE_RE = re.compile(r"0x([0-9a-fA-F]{2})u")


def opcode_bitmap(opcodes: set[int]) -> bytes:
    bitmap = bytearray(32)
    for opcode in opcodes:
        bitmap[opcode >> 3] |= 1 << (opcode & 7)
    return bytes(bitmap)


def source_bitmap(source: str, name: str) -> bytes:
    match = re.search(
        rf"static const u8 {re.escape(name)}\[32\]\s*=\s*\{{(.*?)\}};",
        source,
        re.S,
    )
    if match is None:
        raise ValueError(f"missing 32-byte source bitmap {name}")
    values = bytes(int(value, 16) for value in BYTE_RE.findall(match.group(1)))
    if len(values) != 32:
        raise ValueError(f"{name} has {len(values)} bytes, expected 32")
    return values


def canonical_address(name: str, sources: list[str]) -> int:
    candidates = [name]
    define_re = re.compile(rf"^#define\s+([A-Za-z_]\w*)\s+{re.escape(name)}$", re.M)
    for source in sources:
        candidates.extend(define_re.findall(source))

    for candidate in candidates:
        definition_re = re.compile(rf"\b{re.escape(candidate)}\s*\(void\)")
        for source in sources:
            for match in definition_re.finditer(source):
                prefix = source[max(0, match.start() - 400) : match.start()]
                addresses = RAM_RE.findall(prefix)
                if addresses:
                    return int(addresses[-1], 16)
    raise ValueError(f"no canonical RAM comment found for {name}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--firmware", default="data/W5224A104/MONT_104.bin", type=Path
    )
    parser.add_argument(
        "--map", default="src/scsi/command_dispatch_map.inc", type=Path
    )
    parser.add_argument(
        "--media-map",
        default="src/scsi/command_media_requirement_map.inc",
        type=Path,
    )
    parser.add_argument("--source-root", default="src", type=Path)
    parser.add_argument(
        "--dispatch-source", default="src/scsi/command_dispatch.c", type=Path
    )
    args = parser.parse_args()

    entries: dict[int, str] = {}
    for line_number, line in enumerate(args.map.read_text().splitlines(), 1):
        if not line.startswith("X("):
            continue
        match = ENTRY_RE.fullmatch(line)
        if match is None:
            raise ValueError(f"{args.map}:{line_number}: malformed map entry")
        opcode = int(match.group(1), 16)
        if opcode in entries:
            raise ValueError(f"duplicate opcode 0x{opcode:02x}")
        entries[opcode] = match.group(2)

    image = args.firmware.read_bytes()
    table = image[0x3EF7 : 0x42F7]
    if len(table) != 0x400:
        raise ValueError("firmware image does not contain the complete dispatch table")
    targets = [int.from_bytes(table[index : index + 4], "big")
               for index in range(0, len(table), 4)]
    unsupported = 0x00FAFF96
    expected_opcodes = {opcode for opcode, target in enumerate(targets)
                        if target != unsupported}
    if set(entries) != expected_opcodes:
        missing = sorted(expected_opcodes - set(entries))
        extra = sorted(set(entries) - expected_opcodes)
        raise ValueError(f"opcode-set mismatch: missing={missing}, extra={extra}")

    source_paths = list(args.source_root.rglob("*.c"))
    source_paths.extend(
        (args.source_root / "platform" / "bindings").rglob("*.h"))
    sources = [path.read_text(errors="replace") for path in source_paths]
    failures = []
    for opcode, name in sorted(entries.items()):
        source_address = canonical_address(name, sources)
        if source_address != targets[opcode]:
            failures.append(
                f"0x{opcode:02x}: {name} is RAM:{source_address:06x}, "
                f"table contains RAM:{targets[opcode]:06x}"
            )
    if failures:
        raise ValueError("dispatch target mismatch:\n" + "\n".join(failures))

    media_opcodes = set()
    for line_number, line in enumerate(args.media_map.read_text().splitlines(), 1):
        if not line.startswith("X("):
            continue
        match = OPCODE_RE.fullmatch(line)
        if match is None:
            raise ValueError(f"{args.media_map}:{line_number}: malformed map entry")
        opcode = int(match.group(1), 16)
        if opcode in media_opcodes:
            raise ValueError(f"duplicate media-requirement opcode 0x{opcode:02x}")
        media_opcodes.add(opcode)

    media_table = image[0x42F7 : 0x43F7]
    if len(media_table) != 0x100 or any(value not in (0, 1) for value in media_table):
        raise ValueError("firmware media-requirement table is missing or non-Boolean")
    expected_media_opcodes = {opcode for opcode, value in enumerate(media_table)
                              if value == 1}
    if media_opcodes != expected_media_opcodes:
        missing = sorted(expected_media_opcodes - media_opcodes)
        extra = sorted(media_opcodes - expected_media_opcodes)
        raise ValueError(
            f"media-requirement mismatch: missing={missing}, extra={extra}"
        )

    dispatch_source = args.dispatch_source.read_text()
    if source_bitmap(dispatch_source, "supported_opcode_bits") != opcode_bitmap(set(entries)):
        raise ValueError("supported opcode source bitmap differs from symbolic map")
    if (source_bitmap(dispatch_source, "media_required_opcode_bits") !=
            opcode_bitmap(media_opcodes)):
        raise ValueError("media-required source bitmap differs from symbolic map")

    print(
        f"verified {len(entries)} supported opcodes and "
        f"{256 - len(entries)} unsupported opcodes, plus "
        f"{len(media_opcodes)} media-required opcodes, against {args.firmware}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
