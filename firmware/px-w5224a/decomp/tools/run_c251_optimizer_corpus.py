"""Rank practical C251 optimization levels against representative firmware leaves."""

import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "tools" / "compiler_fixtures" / "optimizer_corpus" / "corpus.c"
IMAGE = ROOT / "data" / "W5224A104" / "MONT_104.bin"
IMAGE_BASE = 0xF80000
FUNCTIONS = {
    "DecodeControllerEncodedSignedOffset": (0xFA824F, 28),
    "PackedBcdByteToBinary": (0xFEE831, 16),
    "BinaryByteToPackedBcd": (0xFEEE69, 28),
    "DecodeAsciiHexNibbleOrZero": (0xFE3243, 52),
    "DecodeUppercaseAsciiHexNibbleOrZero": (0xFDA6C7, 41),
    "EncodeSignedByteAsSignMagnitude": (0xFA823A, 21),
    "MinU8": (0xFDDD27, 9),
    "MinU16": (0xFDDD30, 9),
    "MinU32": (0xFDDD39, 9),
    "AbsoluteValueI16": (0xF849E3, 16),
    "MapOperatingCodeToProfileIndex": (0xF8F47A, 111),
    "MapOperatingCodeToTransitionClass": (0xF8F4E9, 49),
    "MapControllerRegisterIndexSkippingGap": (0xF9CD4A, 21),
    "MapThreeFieldStatusCode": (0xFE19DE, 56),
    "PowerOfTenU32": (0xFDA732, 37),
    "IntegerSquareRootU32": (0xFEE6C4, 188),
    "ToggleR3Bit7IfSignedR3Negative": (0xFEE695, 15),
    "ToggleR3Bit7IfSignedWR2Negative": (0xFEE6A4, 16),
    "ToggleR3Bit7IfSignedDR0Negative": (0xFEE6B4, 16),
}


def listing_functions(path: Path) -> dict[str, bytes]:
    begin = re.compile(r";\s+FUNCTION ([A-Za-z0-9_]+)\?? \(BEGIN\)")
    end = re.compile(r";\s+FUNCTION ([A-Za-z0-9_]+)\?? \(END\)")
    instruction = re.compile(r"^[0-9A-F]{6}\s+([0-9A-F]+)(?:\s+R)?\s{2,}")
    current = None
    result: dict[str, bytearray] = {}
    for line in path.read_text(encoding="latin-1").splitlines():
        match = begin.search(line)
        if match:
            current = match.group(1)
            result[current] = bytearray()
            continue
        match = end.search(line)
        if match and current == match.group(1):
            current = None
            continue
        if current:
            match = instruction.match(line)
            if match and len(match.group(1)) % 2 == 0:
                result[current].extend(bytes.fromhex(match.group(1)))
    return {name: bytes(data) for name, data in result.items()}


def compare(candidate: bytes, original: bytes) -> dict:
    matcher = difflib.SequenceMatcher(a=original, b=candidate, autojunk=False)
    return {
        "candidate_size": len(candidate),
        "original_size": len(original),
        "size_error": len(candidate) - len(original),
        "similarity": matcher.ratio(),
        "longest_common_block": matcher.find_longest_match().size,
        "exact": candidate == original,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--keil-root", type=Path, required=True)
    parser.add_argument("--output", type=Path,
                        default=ROOT / "out" / "agent-tests" / "c251-optimizer-corpus")
    parser.add_argument("--levels", default="0,1,2,3,4,5,6,7")
    parser.add_argument("--models", default="LARGE",
                        help="comma-separated C251 memory models")
    parser.add_argument("--preference", choices=("SIZE", "SPEED"),
                        help="optional OPTIMIZE size/speed preference")
    args = parser.parse_args()
    compiler = args.keil_root / "BIN" / "C251.EXE"
    if not compiler.is_file():
        parser.error(f"missing compiler: {compiler}")

    levels = [int(value) for value in args.levels.split(",")]
    models = [value.strip().upper() for value in args.models.split(",")]
    image = IMAGE.read_bytes()
    originals = {
        name: image[address - IMAGE_BASE:address - IMAGE_BASE + size]
        for name, (address, size) in FUNCTIONS.items()
    }
    args.output.mkdir(parents=True, exist_ok=True)
    results = []
    for model in models:
        for level in levels:
            work = args.output / model.lower() / f"o{level}"
            work.mkdir(parents=True, exist_ok=True)
            command = [
                sys.executable, str(ROOT / "tools" / "keil_driver.py"),
                "--tool", str(compiler), "--root", str(args.keil_root),
                "--name", "compile", "--output", "corpus.obj", "--",
                str(SOURCE), "MODSRC", "ROM(HUGE)", model,
                "REGPARMS", "PARM251",
                (f"OPTIMIZE({level},{args.preference})" if args.preference
                 else f"OPTIMIZE({level})"),
                "CODE", "OBJECT(corpus.obj)",
                "PRINT(corpus.lst)",
            ]
            process = subprocess.run(
                command, cwd=work, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, text=True)
            if process.returncode != 0:
                print(process.stdout, end="", file=sys.stderr)
                results.append({"model": model, "level": level,
                                "compile_exit": process.returncode})
                continue
            emitted = listing_functions(work / "corpus.lst")
            functions = {}
            for name, original in originals.items():
                if name not in emitted:
                    raise ValueError(f"{name} missing from {work / 'corpus.lst'}")
                functions[name] = compare(emitted[name], original)
            total_candidate = sum(
                item["candidate_size"] for item in functions.values())
            total_original = sum(
                item["original_size"] for item in functions.values())
            results.append({
                "model": model,
                "level": level,
                "compile_exit": 0,
                "total_candidate_size": total_candidate,
                "total_original_size": total_original,
                "total_size_error": total_candidate - total_original,
                "mean_similarity": sum(
                    item["similarity"] for item in functions.values()) / len(functions),
                "exact_functions": sum(item["exact"] for item in functions.values()),
                "functions": functions,
            })

    ranked = sorted((item for item in results if item["compile_exit"] == 0),
                    key=lambda item: (abs(item["total_size_error"]),
                                      -item["mean_similarity"]))
    report = {
        "target": "MONT_104.bin",
        "source_sha256": hashlib.sha256(SOURCE.read_bytes()).hexdigest(),
        "compiler": str(compiler.resolve()),
        "preference": args.preference,
        "functions": {name: {"address": f"RAM:{address:06x}", "size": size}
                      for name, (address, size) in FUNCTIONS.items()},
        "ranking": [{"model": item["model"], "level": item["level"]}
                    for item in ranked],
        "results": results,
    }
    report_path = args.output / "report.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    for item in ranked:
        print(f"{item['model']} O{item['level']}: "
              f"size={item['total_candidate_size']}/"
              f"{item['total_original_size']} error={item['total_size_error']:+d} "
              f"similarity={item['mean_similarity']:.4f} "
              f"exact={item['exact_functions']}/{len(FUNCTIONS)}")
    print(report_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
