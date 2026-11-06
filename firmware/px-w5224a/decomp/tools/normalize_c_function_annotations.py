"""Normalize canonical firmware function comments to compact RAM tags."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


LEGACY_COMMENT = re.compile(
    r"(?P<indent>^[ \t]*)/\*\s*Ghidra:\s*/validated/MONT_104\.bin\s+"
    r"RAM:(?P<address>[0-9a-fA-F]{6})(?:\.\.[0-9a-fA-F]{6})?"
    r"(?P<details>.*?)\*/",
    re.MULTILINE | re.DOTALL,
)

METADATA_WORDS = {
    "acc", "after", "at", "below", "body", "boolean", "boundary", "core",
    "coroutine", "distinct", "dr0", "dr4", "entry", "eret", "false", "input",
    "inputs", "loop", "native", "near", "no", "nonreturning", "plus", "result",
    "ret", "return", "returning", "returns", "shared", "status", "table", "tail",
    "target", "thunk", "through", "to", "true", "with", "wr0", "wr6", "wr14",
}


def _comment_text(details: str) -> list[str]:
    lines = details.splitlines()
    if not lines:
        return []

    first = lines.pop(0).strip().lstrip(".;, ")
    explanation: list[str] = []
    if first:
        sentences = re.split(r"\.\s+(?=[A-Z])", first, maxsplit=1)
        if len(sentences) == 2:
            first = sentences[1]
        else:
            words = set(re.findall(r"[A-Za-z][A-Za-z0-9]*", first.lower()))
            if words <= METADATA_WORDS:
                first = ""
        if first:
            explanation.append(first)

    for line in lines:
        line = re.sub(r"^[ \t]*\*[ \t]?", "", line).rstrip()
        explanation.append(line)

    while explanation and not explanation[0]:
        explanation.pop(0)
    while explanation and not explanation[-1]:
        explanation.pop()
    return explanation


def _format_comment(lines: list[str], indent: str, newline: str) -> str:
    if not lines:
        return ""
    if len(lines) == 1:
        return f"{indent}/* {lines[0]} */"
    rendered = [f"{indent}/* {lines[0]}"]
    rendered.extend(f"{indent} * {line}" if line else f"{indent} *" for line in lines[1:])
    rendered[-1] += " */"
    return newline.join(rendered)


def normalize(path: Path, keep_tag: bool) -> tuple[str, int]:
    original = path.read_bytes().decode("utf-8")
    count = 0

    def replace(match: re.Match[str]) -> str:
        nonlocal count
        count += 1
        indent = match.group("indent")
        details = match.group("details")
        newline = "\r\n" if "\r\n" in match.group(0) else "\n"
        addresses = [match.group("address").lower()]
        addresses.extend(
            address.lower()
            for address in re.findall(
                r"Ghidra:\s*/validated/MONT_104\.bin\s+RAM:([0-9a-fA-F]{6})",
                details,
            )
        )
        details = re.sub(
            r"^[ \t]*\*[ \t]*Ghidra:\s*/validated/MONT_104\.bin\s+"
            r"RAM:[0-9a-fA-F]{6}(?:\.\.[0-9a-fA-F]{6})?[^\r\n]*",
            "",
            details,
            flags=re.MULTILINE,
        )
        explanation = _format_comment(_comment_text(details), indent, newline)
        tags = newline.join(f"{indent}/* RAM:{address} */" for address in addresses)
        tag = tags if keep_tag else ""
        return newline.join(part for part in (tag, explanation) if part)

    return LEGACY_COMMENT.sub(replace, original), count


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()

    source_root = args.root / "src"
    changed = 0
    comments = 0
    for suffix in ("*.c", "*.h"):
        for path in sorted(source_root.rglob(suffix)):
            normalized, count = normalize(path, keep_tag=path.suffix == ".c")
            comments += count
            original = path.read_bytes().decode("utf-8")
            if normalized == original:
                continue
            changed += 1
            if args.write:
                path.write_bytes(normalized.encode("utf-8"))

    action = "normalized" if args.write else "would normalize"
    print(f"{action} {comments} comments in {changed} files")
    return 0 if args.write or changed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
