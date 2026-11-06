"""Run one Keil build step with checked diagnostics and reproducible provenance."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


def wine_path(path: Path) -> str:
    """Translate an absolute POSIX path through Wine's default Z: mapping."""
    return "Z:" + str(path.resolve()).replace("/", "\\")


def wine_argument(value: str) -> str:
    path = Path(value)
    if path.is_absolute():
        return wine_path(path)
    return re.sub(r"(?<=\()/[^)]*", lambda match: wine_path(Path(match.group())),
                  value)


def case_variants(path: Path):
    """Yield existing sibling names that differ only by case."""
    if not path.parent.is_dir():
        return
    folded_name = path.name.casefold()
    for sibling in path.parent.iterdir():
        if sibling.name.casefold() == folded_name:
            yield sibling


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool", type=Path, required=True)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    arguments = args.arguments[1:] if args.arguments[:1] == ["--"] else args.arguments
    environment = os.environ.copy()
    # Pin the tool's search paths; do not record unrelated process environment.
    tool_command = [str(args.tool)]
    if os.name != "nt" and args.tool.read_bytes()[:2] == b"MZ":
        wine = shutil.which("wine")
        if wine is None:
            print("Windows Keil executable requires Wine on this host", file=sys.stderr)
            return 1
        tool_command = [wine, str(args.tool)]
        arguments = [wine_argument(value) for value in arguments]
        environment["C251INC"] = wine_path(args.root / "INC")
        environment["C251LIB"] = wine_path(args.root / "LIB")
    else:
        environment["C251INC"] = str(args.root / "INC")
        environment["C251LIB"] = str(args.root / "LIB")
    for old_output in case_variants(args.output):
        old_output.unlink()
    metadata_path = Path(args.name + ".json")
    metadata_path.unlink(missing_ok=True)
    try:
        process = subprocess.run([*tool_command, *arguments], env=environment,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 text=True, errors="replace", timeout=120)
        # License serials are not build provenance.
        output = re.sub(r"(?i)(?:Serial Number|SN:)\s*[^\s]+", "[serial omitted]", process.stdout)
        Path(args.name + ".log").write_text(output, encoding="utf-8")
        print(output, end="")
        if not args.output.is_file():
            output_variants = list(case_variants(args.output))
            if len(output_variants) == 1:
                output_variants[0].replace(args.output)
        if (process.returncode != 0 or re.search(r"\*\*\*\s*(?:FATAL[- ]?)?ERROR", output, re.I)
                or re.search(r"\b[1-9][0-9]*\s+(?:ERROR|WARNING)\(S\)", output, re.I)
                or not args.output.is_file()):
            args.output.unlink(missing_ok=True)
            raise RuntimeError(f"{args.name} failed; see {args.name}.log and listing/map. "
                               "Check tool license, diagnostics, and absolute placement.")
        version = re.search(r"\bV([0-9]+(?:\.[0-9]+)+)", output)
        metadata = {"tool": str(args.tool.resolve()),
                    "sha256": hashlib.sha256(args.tool.read_bytes()).hexdigest(),
                    "version": version.group(1) if version else "unreported",
                    "arguments": arguments, "working_directory": str(Path.cwd()),
                    "environment": {key: environment[key] for key in ("C251INC", "C251LIB")},
                    "output": str(args.output),
                    "output_sha256": hashlib.sha256(args.output.read_bytes()).hexdigest()}
        metadata_path.write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
        return 0
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as exc:
        args.output.unlink(missing_ok=True)
        metadata_path.unlink(missing_ok=True)
        print(str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
