"""Compare C251 listing function sizes with canonical Ghidra function extents."""

import argparse
from pathlib import Path
import re


FUNCTION_MARKER = re.compile(r"^;\s+FUNCTION\s+(.+?)\??\s+\((BEGIN|END)\)$")
INSTRUCTION = re.compile(r"^([0-9A-F]{6})\s+([0-9A-F]+)(?:\s|$)")
ECODE_SIZE = re.compile(r"^\s+ecode size\s+=\s+([0-9]+)")


def canonical_sizes(path):
    sizes = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        entry, name, first, last, _ = line.split("\t")
        start = int(first.split(":", 1)[1], 16)
        end = int(last.split(":", 1)[1], 16)
        sizes[name] = end - start + 1
    return sizes


def listing_sizes(path):
    functions = []
    current = None
    first = None
    end = None
    for line_number, line in enumerate(
            path.read_text(encoding="latin-1").splitlines(), 1):
        marker = FUNCTION_MARKER.match(line)
        if marker:
            name, kind = marker.groups()
            if kind == "BEGIN":
                if current is not None:
                    raise ValueError(f"{path}:{line_number}: nested function")
                current, first, end = name, None, None
            else:
                if current != name or first is None or end is None:
                    raise ValueError(f"{path}:{line_number}: unmatched function end")
                functions.append((name, end - first))
                current = None
            continue
        instruction = INSTRUCTION.match(line)
        if current is not None and instruction:
            address = int(instruction.group(1), 16)
            instruction_end = address + len(instruction.group(2)) // 2
            if first is None:
                first = address
            end = max(end or instruction_end, instruction_end)
    if current is not None:
        raise ValueError(f"{path}: unterminated function {current}")
    return functions


def listing_ecode_size(path):
    for line in path.read_text(encoding="latin-1").splitlines():
        match = ECODE_SIZE.match(line)
        if match:
            return int(match.group(1))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--functions", type=Path, required=True)
    parser.add_argument("--listing", type=Path, action="append", default=[])
    parser.add_argument("--directory", type=Path)
    parser.add_argument("--limit", type=int, default=30)
    parser.add_argument("--modules", action="store_true")
    parser.add_argument("--unmatched", action="store_true")
    args = parser.parse_args()

    canonical = canonical_sizes(args.functions)
    rows = []
    unmatched_rows = []
    listings = list(args.listing)
    if args.directory:
        listings.extend(sorted(args.directory.glob("*.lst")))
    if not listings:
        parser.error("provide --listing or --directory")
    module_rows = []
    for listing in listings:
        compiled_total = 0
        original_total = 0
        for name, compiled_size in listing_sizes(listing):
            original_size = canonical.get(name)
            if original_size is not None:
                rows.append((compiled_size - original_size, compiled_size,
                             original_size, listing.stem, name))
                compiled_total += compiled_size
                original_total += original_size
            else:
                unmatched_rows.append((compiled_size, listing.stem, name))
        if original_total:
            module_size = listing_ecode_size(listing)
            module_rows.append((module_size - original_total,
                                module_size, original_total, listing.stem))
    selected_rows = (unmatched_rows if args.unmatched else
                     module_rows if args.modules else rows)
    selected_rows.sort(reverse=True)
    if args.unmatched:
        print("compiled\tmodule\tfunction")
    elif args.modules:
        print("delta\tcompiled\toriginal\tmodule")
    else:
        print("delta\tcompiled\toriginal\tmodule\tfunction")
    for row in selected_rows[:args.limit]:
        print("\t".join(str(value) for value in row))


if __name__ == "__main__":
    main()
