#!/usr/bin/env python3
"""Generate relocatable C arrays for overwritten firmware 1.04 data regions."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path


REFERENCE_SHA256 = "622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2"
REGIONS = (
    ("PlextorMainRomData", 0x2320, 0x25E0),
    ("PlextorIndexedStateReferencePayloads", 0x94E3, 73),
)


def emit_array(name: str, data: bytes) -> str:
    lines = [f"const u8 {name}[0x{len(data):x}u] = {{"]
    for offset in range(0, len(data), 16):
        chunk = data[offset : offset + 16]
        lines.append("    " + ", ".join(f"0x{value:02x}u" for value in chunk) + ",")
    lines.append("};")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--firmware", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    image = args.firmware.read_bytes()
    digest = hashlib.sha256(image).hexdigest()
    if digest != REFERENCE_SHA256:
        raise ValueError(
            f"unexpected firmware SHA-256 {digest}; expected {REFERENCE_SHA256}"
        )

    arrays = []
    for name, offset, size in REGIONS:
        data = image[offset : offset + size]
        if len(data) != size:
            raise ValueError(f"firmware is too short for {name}")
        arrays.append(emit_array(name, data))

    output = (
        "/* Generated from immutable Plextor firmware 1.04; do not edit. */\n"
        '#include "platform/firmware_rom_data.h"\n\n'
        + "\n\n".join(arrays)
        + "\n"
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
