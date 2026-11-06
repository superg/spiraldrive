#!/usr/bin/env python3
"""Audit the generated Keil firmware link manifest and an optional L251 log."""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import re


LINK_INPUTS = re.compile(
    r"(?m)^([^\r\n]*\.obj(?:,[^\r\n]*\.obj)*) TO main\.abs")
UNRESOLVED = re.compile(
    r"\*\*\* ERROR L127: UNRESOLVED EXTERNAL SYMBOL\s+SYMBOL:\s+([^\s]+)",
    re.MULTILINE,
)
LISTING_FUNCTION = re.compile(r"FUNCTION\s+([^\s]+)\s+\(BEGIN\)")


@dataclass(frozen=True)
class Report:
    manifest_entries: int
    manifest_unique: int
    manifest_duplicates: dict[str, int]
    existing_objects: int
    missing_objects: list[str]
    link_entries: int | None
    link_unique: int | None
    link_duplicates: dict[str, int]
    missing_from_link: list[str]
    unexpected_in_link: list[str]
    unresolved_symbols: list[str]
    unresolved_compiler_runtime: list[str]
    unresolved_project_or_external: list[str]
    unresolved_exact_compiled_definitions: dict[str, list[str]]
    unresolved_decoration_mismatches: dict[str, list[str]]
    unresolved_source_without_compiled_definition: list[str]
    unresolved_without_source_match: list[str]


def duplicates(items: list[str]) -> dict[str, int]:
    return {
        item: count
        for item, count in sorted(Counter(items).items())
        if count > 1
    }


def read_manifest(path: Path) -> list[str]:
    entries = [line.strip() for line in path.read_text().splitlines()
               if line.strip()]
    invalid = [entry for entry in entries
               if Path(entry).name != entry or not entry.lower().endswith(".obj")]
    if invalid:
        raise ValueError(f"invalid manifest entries: {invalid}")
    return entries


def parse_link_inputs(text: str) -> list[str] | None:
    match = LINK_INPUTS.search(text)
    if match is None:
        return None
    return [entry.strip() for entry in match.group(1).split(",")]


def is_compiler_runtime(symbol: str) -> bool:
    return symbol.startswith("?C?") or symbol.lower() in {
        "sqrt?", "floor?", "ceil?", "log?", "exp?",
    }


def read_compiled_definitions(object_dir: Path) -> dict[str, list[str]]:
    definitions: dict[str, list[str]] = {}
    for path in sorted(object_dir.glob("*.lst")):
        for symbol in LISTING_FUNCTION.findall(
                path.read_text(errors="replace")):
            definitions.setdefault(symbol, []).append(path.name)
    return definitions


def read_source_texts(source_root: Path | None) -> list[str]:
    if source_root is None:
        return []
    return [
        path.read_text(errors="replace")
        for path in source_root.rglob("*")
        if path.suffix.lower() in {".c", ".h", ".a51"}
    ]


def audit(manifest_path: Path, object_dir: Path,
          link_log: Path | None, source_root: Path | None) -> Report:
    manifest = read_manifest(manifest_path)
    missing_objects = [entry for entry in manifest
                       if not (object_dir / entry).is_file()]
    link_inputs: list[str] | None = None
    unresolved: list[str] = []
    if link_log is not None:
        text = link_log.read_text(errors="replace")
        link_inputs = parse_link_inputs(text)
        unresolved = sorted(set(UNRESOLVED.findall(text)))
    link_set = set(link_inputs or [])
    manifest_set = set(manifest)
    compiler_runtime = [symbol for symbol in unresolved
                        if is_compiler_runtime(symbol)]
    project_or_external = [symbol for symbol in unresolved
                           if not is_compiler_runtime(symbol)]
    definitions = read_compiled_definitions(object_dir)
    source_texts = read_source_texts(source_root)
    exact_definitions: dict[str, list[str]] = {}
    decoration_mismatches: dict[str, list[str]] = {}
    source_only: list[str] = []
    no_source_match: list[str] = []
    for symbol in project_or_external:
        if symbol in definitions:
            exact_definitions[symbol] = definitions[symbol]
            continue
        base = symbol.rstrip("?")
        alternatives = sorted(
            actual for actual in definitions
            if actual.rstrip("?") == base)
        if alternatives:
            decoration_mismatches[symbol] = alternatives
            continue
        source_pattern = re.compile(r"\b" + re.escape(base) + r"\s*\(")
        if any(source_pattern.search(text) for text in source_texts):
            source_only.append(symbol)
        else:
            no_source_match.append(symbol)
    return Report(
        manifest_entries=len(manifest),
        manifest_unique=len(manifest_set),
        manifest_duplicates=duplicates(manifest),
        existing_objects=len(manifest) - len(missing_objects),
        missing_objects=missing_objects,
        link_entries=None if link_inputs is None else len(link_inputs),
        link_unique=None if link_inputs is None else len(link_set),
        link_duplicates=duplicates(link_inputs or []),
        missing_from_link=[] if link_inputs is None else
            sorted(manifest_set - link_set),
        unexpected_in_link=[] if link_inputs is None else
            sorted(link_set - manifest_set),
        unresolved_symbols=unresolved,
        unresolved_compiler_runtime=compiler_runtime,
        unresolved_project_or_external=project_or_external,
        unresolved_exact_compiled_definitions=exact_definitions,
        unresolved_decoration_mismatches=decoration_mismatches,
        unresolved_source_without_compiled_definition=source_only,
        unresolved_without_source_match=no_source_match,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--object-dir", type=Path)
    parser.add_argument("--link-log", type=Path)
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()
    object_dir = args.object_dir or args.manifest.parent
    report = audit(args.manifest, object_dir, args.link_log, args.source_root)
    payload = asdict(report)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    invalid = bool(
        report.manifest_duplicates or report.missing_objects or
        report.link_duplicates or report.missing_from_link or
        report.unexpected_in_link
    )
    return 1 if invalid else 0


if __name__ == "__main__":
    raise SystemExit(main())
