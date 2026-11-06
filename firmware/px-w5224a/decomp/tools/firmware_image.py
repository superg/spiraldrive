"""Strict PX-W5224A 1.04 container tooling; never modifies firmware inputs."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sys
import tempfile

BASE = 0xF80000
SIZE = 0x80000
CHECKSUM_OFFSET = 0x79FFE
HEADER = (b"PLEXTOR CD-R   PX-W5224A1.0404/10/06  17:00     PLEXTOR MONT    000"
          + b"\0" + b"\xff" * 12)
STOCK_SHA256 = "622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2"
ROOT = Path(__file__).resolve().parents[1]


class ImageError(ValueError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ImageError(message)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_hex(text: str) -> dict[int, int]:
    """Parse Intel HEX including 16/20/32-bit address records, bounded to 24 bits."""
    memory: dict[int, int] = {}
    base = 0
    ended = False
    start_seen = False
    for number, line in enumerate(text.splitlines(), 1):
        line = line.strip()
        if not line:
            continue
        prefix = f"HEX line {number}: "
        require(not ended, prefix + "record after EOF")
        require(re.fullmatch(r":[0-9a-fA-F]+", line) is not None,
                prefix + "invalid record syntax")
        try:
            record = bytes.fromhex(line[1:])
        except ValueError as exc:
            raise ImageError(prefix + "invalid hex digits") from exc
        require(len(record) >= 5 and len(record) == record[0] + 5,
                prefix + "invalid byte count")
        require(sum(record) & 0xFF == 0, prefix + "record checksum mismatch")
        count, kind = record[0], record[3]
        offset = int.from_bytes(record[1:3], "big")
        payload = record[4:-1]
        if kind == 0:
            require(offset + count <= 0x10000, prefix + "record crosses 64-KiB boundary")
            for index, value in enumerate(payload):
                address = base + offset + index
                require(address <= 0xFFFFFF, prefix + "address exceeds 24 bits")
                require(address not in memory, prefix + f"overlap at RAM:{address:06x}")
                memory[address] = value
        elif kind == 1:
            require(count == 0 and offset == 0, prefix + "invalid EOF")
            ended = True
        elif kind in (2, 4):
            require(count == 2 and offset == 0, prefix + "invalid extended address")
            base = int.from_bytes(payload, "big") << (4 if kind == 2 else 16)
            require(base <= 0xFFFFFF, prefix + "address exceeds 24 bits")
        elif kind in (3, 5):
            require(count == 4 and offset == 0 and not start_seen,
                    prefix + "invalid or duplicate start address")
            start = (int.from_bytes(payload[:2], "big") * 16
                     + int.from_bytes(payload[2:], "big") if kind == 3
                     else int.from_bytes(payload, "big"))
            require(start <= 0xFFFFFF, prefix + "start address exceeds 24 bits")
            # Physical boot is fixed by the monitor, not by the HEX start record.
            start_seen = True
        else:
            raise ImageError(prefix + f"unsupported record type {kind}")
    require(ended, "HEX has no EOF record")
    require(bool(memory), "HEX contains no payload")
    return memory


def pack(memory: dict[int, int], *, code_only: bool = False,
         fixed_data: bytes | None = None) -> bytes:
    image = bytearray(b"\xff" * SIZE)
    image[:len(HEADER)] = HEADER
    if fixed_data is not None:
        require(len(fixed_data) == 0x1FFE,
                "fixed data must cover RAM:ff8000..ff9ffd exactly")
        image[0x78000:CHECKSUM_OFFSET] = fixed_data
    for address, value in memory.items():
        offset = address - BASE
        require(0x50 <= offset < CHECKSUM_OFFSET,
                f"payload outside code/data ranges: RAM:{address:06x}")
        require(not 0x70000 <= offset < 0x78000,
                f"payload in recovery hole: RAM:{address:06x}")
        require(not code_only or address < 0xFF0000,
                f"executable payload above main code window: RAM:{address:06x}")
        require(0 <= value <= 255, "payload byte out of range")
        image[offset] = value
    checksum = (0x8000 + sum(image[:CHECKSUM_OFFSET])) & 0xFFFF
    image[CHECKSUM_OFFSET:CHECKSUM_OFFSET + 2] = checksum.to_bytes(2, "big")
    result = bytes(image)
    verify(result)
    return result


def verify(image: bytes, *, fixed_data_reference: bytes | None = None) -> dict:
    require(len(image) == SIZE, f"image size must be 0x{SIZE:x}")
    require(image[:0x50] == HEADER, "1.04 header mismatch")
    require(image[0x70000:0x78000] == b"\xff" * 0x8000, "recovery hole is not erased")
    require(image[0x7A000:] == b"\xff" * 0x6000, "trailing padding is not erased")
    if fixed_data_reference is not None:
        require(len(fixed_data_reference) == SIZE and
                sha256(fixed_data_reference) == STOCK_SHA256,
                "fixed-data reference is not immutable stock firmware 1.04")
        require(image[0x78000:CHECKSUM_OFFSET] ==
                fixed_data_reference[0x78000:CHECKSUM_OFFSET],
                "stock fixed data mismatch at RAM:ff8000..ff9ffd")
    stored = int.from_bytes(image[CHECKSUM_OFFSET:CHECKSUM_OFFSET + 2], "big")
    container_sum = (0x8000 + sum(image[:CHECKSUM_OFFSET])) & 0xFFFF
    # Independent expression recovered from ValidateMainFirmwareImage at
    # RAM:ff4dc9 in /validated/read_memory_ff0000.debug (not main firmware).
    monitor_sum = (sum(image[:0x70000]) + sum(image[0x78000:CHECKSUM_OFFSET])) & 0xFFFF
    require(stored == container_sum, "container checksum mismatch")
    require(stored == monitor_sum, "monitor split-range checksum mismatch")
    entry_sum = sum(image[0x50:0x200])
    require(entry_sum not in (0, 0x1AE50), "monitor rejects zero/erased entry window")
    report = {"format": "PX-W5224A-1.04", "size": len(image), "sha256": sha256(image),
              "checksum": f"0x{stored:04x}", "monitor_checksum": f"0x{monitor_sum:04x}",
              "entry_window_sum": entry_sum, "format_valid": True,
              "stock_image_match": sha256(image) == STOCK_SHA256}
    if fixed_data_reference is not None:
        report["stock_fixed_data_match"] = True
    return report


def ranges(addresses) -> list[dict]:
    result = []
    for address in sorted(addresses):
        if result and address == result[-1]["end_exclusive"]:
            result[-1]["end_exclusive"] += 1
        else:
            result.append({"start": address, "end_exclusive": address + 1})
    return result


def compare(image: bytes, reference: bytes) -> dict:
    require(len(image) == len(reference) == SIZE, "comparison requires two 512-KiB images")
    differences = [i for i, (a, b) in enumerate(zip(image, reference)) if a != b]
    return {"identical": not differences, "differing_bytes": len(differences),
            "matching_bytes": SIZE - len(differences),
            "difference_ranges_file_offsets": ranges(differences),
            "vector_region": {"start": "RAM:f80050", "size": 256,
                              "identical": image[0x50:0x150] == reference[0x50:0x150]},
            "note": "Byte equality outside reconstructed ranges is not source reconstruction coverage."}


def check_output(path: Path, inputs=()) -> None:
    target = path.resolve()
    require(all(target != Path(p).resolve() for p in inputs), "output would overwrite an input")
    require(not any(target.is_relative_to(ROOT / folder) for folder in ("data", "ghidra")),
            "output would modify an immutable image/canonical project directory")


def write_output(path: Path, data: bytes, inputs=()) -> None:
    check_output(path, inputs)
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
        os.replace(temporary, path)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    packing = commands.add_parser("pack")
    packing.add_argument("input", type=Path, help="addressed Intel HEX")
    packing.add_argument("output", type=Path)
    packing.add_argument("--code-only", action="store_true")
    packing.add_argument("--fixed-data-from", type=Path,
                         help="512-KiB image supplying RAM:ff8000..ff9ffd")
    checking = commands.add_parser("verify")
    checking.add_argument("input", type=Path)
    checking.add_argument("--fixed-data-from", type=Path,
                          help="require fixed data to match immutable stock firmware 1.04")
    comparing = commands.add_parser("compare")
    comparing.add_argument("input", type=Path)
    comparing.add_argument("reference", type=Path)
    comparing.add_argument("--require-identical", action="store_true")
    comparing.add_argument("--require-vectors", action="store_true")
    for command in (packing, checking, comparing):
        command.add_argument("--report", type=Path)
    args = parser.parse_args(argv)
    try:
        inputs = [args.input]
        if args.command == "compare":
            inputs.append(args.reference)
        if args.report:
            check_output(args.report, inputs)
        if args.command == "pack":
            fixed_data = None
            if args.fixed_data_from is not None:
                reference = args.fixed_data_from.read_bytes()
                require(len(reference) == SIZE,
                        "fixed-data reference must be a 512-KiB image")
                fixed_data = reference[0x78000:CHECKSUM_OFFSET]
                inputs.append(args.fixed_data_from)
            check_output(args.output, inputs)
            require(not args.report or args.report.resolve() != args.output.resolve(),
                    "image and report paths must differ")
            image = pack(read_hex(args.input.read_text(encoding="ascii")),
                         code_only=args.code_only, fixed_data=fixed_data)
            report = verify(image)
            write_output(args.output, image, inputs)
        elif args.command == "verify":
            reference = None
            if args.fixed_data_from is not None:
                inputs.append(args.fixed_data_from)
                reference = args.fixed_data_from.read_bytes()
            report = verify(args.input.read_bytes(), fixed_data_reference=reference)
        else:
            report = compare(args.input.read_bytes(), args.reference.read_bytes())
        output = json.dumps(report, indent=2) + "\n"
        if args.report:
            write_output(args.report, output.encode(), inputs)
        print(output, end="")
        if args.command == "compare":
            require(not args.require_identical or report["identical"], "images differ (see comparison report)")
            require(not args.require_vectors or report["vector_region"]["identical"],
                    "vector bytes differ (see comparison report)")
        return 0
    except (OSError, ValueError) as exc:
        print(f"firmware_image: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
