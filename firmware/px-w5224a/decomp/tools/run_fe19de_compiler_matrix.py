"""Compile and byte-rank C251 source fixtures for RAM:fe19de."""

import argparse
import difflib
import json
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tools" / "compiler_fixtures" / "fe19de"
IMAGE = ROOT / "data" / "W5224A104" / "MONT_104.bin"
IMAGE_OFFSET = 0x619DE
FUNCTION_SIZE = 0x38
FUNCTION_NAME = "MapThreeFieldStatusCode"


def listing_function_bytes(path: Path) -> bytes:
    inside = False
    result = bytearray()
    begin = re.compile(rf"FUNCTION {FUNCTION_NAME}\?? \(BEGIN\)")
    end = re.compile(rf"FUNCTION {FUNCTION_NAME}\?? \(END\)")
    instruction = re.compile(r"^[0-9A-F]{6}\s+([0-9A-F]+)(?:\s+R)?\s{2,}")

    for line in path.read_text(encoding="latin-1").splitlines():
        if begin.search(line):
            inside = True
            continue
        if inside and end.search(line):
            break
        if inside:
            match = instruction.match(line)
            if match and len(match.group(1)) % 2 == 0:
                result.extend(bytes.fromhex(match.group(1)))
    if not result:
        raise ValueError(f"no {FUNCTION_NAME} bytes found in {path}")
    return bytes(result)


def score(candidate: bytes, target: bytes) -> dict:
    matcher = difflib.SequenceMatcher(a=target, b=candidate, autojunk=False)
    longest = matcher.find_longest_match()
    positional = sum(a == b for a, b in zip(target, candidate))
    first_difference = next(
        (index for index, (a, b) in enumerate(zip(target, candidate)) if a != b),
        min(len(target), len(candidate)) if target != candidate else None,
    )
    return {
        "exact": candidate == target,
        "candidate_size": len(candidate),
        "target_size": len(target),
        "positional_matches": positional,
        "longest_common_block": longest.size,
        "first_difference": first_difference,
        "candidate_hex": candidate.hex(),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--keil-root", type=Path, required=True)
    parser.add_argument("--output", type=Path,
                        default=ROOT / "out" / "agent-tests" / "compiler-match-fe19de-matrix")
    parser.add_argument("--levels", default="0,1,2,3,4,5,6,7,8")
    parser.add_argument("--preferences", default="default,size,speed")
    args = parser.parse_args()

    compiler = args.keil_root / "BIN" / "C251.EXE"
    if not compiler.is_file():
        parser.error(f"missing compiler: {compiler}")
    levels = [int(value) for value in args.levels.split(",")]
    preferences = [value.strip().lower() for value in args.preferences.split(",")]
    unknown = set(preferences) - {"default", "size", "speed"}
    if unknown:
        parser.error(f"unknown preferences: {', '.join(sorted(unknown))}")

    target = IMAGE.read_bytes()[IMAGE_OFFSET:IMAGE_OFFSET + FUNCTION_SIZE]
    args.output.mkdir(parents=True, exist_ok=True)
    results = []
    for source in sorted(FIXTURES.glob("*.c")):
        for level in levels:
            for preference in preferences:
                name = f"{source.stem}-o{level}-{preference}"
                work = args.output / name
                work.mkdir(parents=True, exist_ok=True)
                optimize = (f"OPTIMIZE({level})" if preference == "default"
                            else f"OPTIMIZE({level},{preference.upper()})")
                command = [
                    sys.executable, str(ROOT / "tools" / "keil_driver.py"),
                    "--tool", str(compiler), "--root", str(args.keil_root),
                    "--name", "compile", "--output", "probe.obj", "--",
                    str(source), "MODSRC", "ROM(HUGE)", "LARGE", optimize,
                    "CODE", "OBJECT(probe.obj)", "PRINT(probe.lst)",
                ]
                process = subprocess.run(command, cwd=work, stdout=subprocess.PIPE,
                                         stderr=subprocess.STDOUT, text=True)
                record = {"source": source.name, "optimization": level,
                          "preference": preference, "compile_exit": process.returncode}
                if process.returncode == 0:
                    record.update(score(listing_function_bytes(work / "probe.lst"), target))
                else:
                    print(process.stdout, end="", file=sys.stderr)
                results.append(record)

    ranked = sorted(results, key=lambda item: (
        not item.get("exact", False),
        -item.get("longest_common_block", -1),
        -item.get("positional_matches", -1),
        abs(item.get("candidate_size", 0) - len(target)),
    ))
    report = {
        "target": "MONT_104.bin RAM:fe19de",
        "target_hex": target.hex(),
        "compiler": str(compiler.resolve()),
        "results": ranked,
    }
    report_path = args.output / "report.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    for item in ranked[:10]:
        print(f"{item['source']} O{item['optimization']} {item['preference']}: "
              f"exact={item.get('exact', False)} size={item.get('candidate_size', '-')} "
              f"longest={item.get('longest_common_block', '-')} "
              f"positional={item.get('positional_matches', '-')}")
    print(report_path)
    return 0 if any(item.get("exact") for item in ranked) else 1


if __name__ == "__main__":
    raise SystemExit(main())
