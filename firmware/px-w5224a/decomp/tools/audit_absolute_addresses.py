"""Inventory source-level absolute firmware addresses and their disposition.

This is a source audit, not a replacement for the canonical Ghidra data map.
It deliberately keeps ambiguous numeric constants separate from literals used
in address-like contexts so that a successful link cannot hide unresolved ROM
dependencies.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = ROOT / "src"
HEX_LITERAL = re.compile(r"\b0[xX]([0-9a-fA-F]+)(?:[uUlL]+)?\b")
A51_HEX_LITERAL = re.compile(r"\b([0-9a-fA-F]+)[hH]\b")
ADDRESS_NAME = re.compile(
    r"(?:address|base|table|pointer|port|destination|source|cursor|entry|target)",
    re.IGNORECASE,
)
ADDRESS_CALL = re.compile(r"(?:read|write|copy|clear|fill|validate)", re.IGNORECASE)


@dataclass(frozen=True)
class Occurrence:
    path: str
    line: int
    literal: str
    address: int
    region: str
    disposition: str
    context: str


def scrub_c(text: str) -> str:
    """Remove comments and quoted contents while preserving lines/columns."""
    result = list(text)
    index = 0
    state = "code"
    quote = ""
    while index < len(result):
        char = result[index]
        following = result[index + 1] if index + 1 < len(result) else ""
        if state == "code":
            if char == "/" and following == "/":
                result[index] = result[index + 1] = " "
                index += 2
                state = "line_comment"
                continue
            if char == "/" and following == "*":
                result[index] = result[index + 1] = " "
                index += 2
                state = "block_comment"
                continue
            if char in {'"', "'"}:
                quote = char
                result[index] = " "
                index += 1
                state = "quote"
                continue
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                result[index] = " "
        elif state == "block_comment":
            if char == "*" and following == "/":
                result[index] = result[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char != "\n":
                result[index] = " "
        elif state == "quote":
            if char == "\\" and following:
                result[index] = " "
                if following != "\n":
                    result[index + 1] = " "
                index += 2
                continue
            if char == quote:
                result[index] = " "
                state = "code"
            elif char != "\n":
                result[index] = " "
        index += 1
    return "".join(result)


def classify(address: int) -> tuple[str, str]:
    if address <= 0x0000FF:
        return "internal_ram_or_sfr", "fixed_hardware_or_runtime_contract"
    if address <= 0x017FFF:
        return "runtime_ram", "fixed_runtime_contract"
    if address <= 0x01FFFF:
        return "controller_mmio", "fixed_hardware_contract"
    if address <= 0xF7BFFF:
        return ("external_ram_or_controller_buffer",
                "fixed_runtime_buffer_or_numeric_constant")
    if address <= 0xF7FFFF:
        return "hardware_or_flash_alias", "fixed_hardware_contract_verify"
    if address <= 0xFEFFFF:
        return "main_image_code_or_data", "symbolize_or_emit_at_fixed_address"
    if address <= 0xFF7FFF:
        return "runtime_monitor_overlay", "fixed_external_entry_verify"
    if address <= 0xFF9FFD:
        return "main_image_fixed_data", "emit_and_symbolize_or_fix"
    if address <= 0xFF9FFF:
        return "firmware_checksum", "generated_container_field"
    if address >= 0xFFFFFD:
        return "erased_tail_or_sentinel", "verified_24bit_mask_or_sentinel"
    return "erased_tail_or_sentinel", "constant_or_invalid_pointer_audit"


def is_address_candidate(digits: str, address: int, line: str, start: int) -> bool:
    if address > 0xFFFFFF:
        return False
    if 0xF7C000 <= address:
        return True
    if len(digits) >= 6 and address >= 0x100:
        return True
    if address < 0x100:
        return False
    prefix = line[:start]
    if re.search(r"#\s*define\s+\w*(?:ADDRESS|BASE|TABLE|PORT)\w*\s+$",
                 prefix, re.IGNORECASE):
        return True
    if re.search(r"\b\w*(?:address|base|table|port|source|destination)\w*\s*=\s*$",
                 prefix, re.IGNORECASE):
        return True
    call = re.search(r"\b([A-Za-z_]\w*)\s*\(\s*$", prefix)
    return call is not None and (
        ADDRESS_NAME.search(call.group(1)) is not None or
        ADDRESS_CALL.search(call.group(1)) is not None
    )


def inventory() -> list[Occurrence]:
    occurrences: list[Occurrence] = []
    for path in sorted(SOURCE_ROOT.rglob("*")):
        if path.suffix.lower() not in {".c", ".h"}:
            continue
        original_lines = path.read_text(encoding="utf-8").splitlines()
        scrubbed_lines = scrub_c("\n".join(original_lines)).splitlines()
        for line_number, line in enumerate(scrubbed_lines, 1):
            original = original_lines[line_number - 1].strip()
            for match in HEX_LITERAL.finditer(line):
                digits = match.group(1)
                address = int(digits, 16)
                if not is_address_candidate(digits, address, line, match.start()):
                    continue
                region, disposition = classify(address)
                if "PLEXTOR_CODE_ADDRESS" in original:
                    disposition = "linker_resolved_code_symbol"
                elif ("PLEXTOR_IMAGE_HEADER_ADDRESS" in original or
                      "PLEXTOR_MAIN_ROM_ADDRESS" in original or
                      "PLEXTOR_FIXED_ROM_ADDRESS" in original or
                      "PLEXTOR_INDEXED_STATE_REFERENCE_ADDRESS" in original or
                      "PLEXTOR_IMAGE_HEADER_CANONICAL_BASE" in original or
                      "PLEXTOR_MAIN_ROM_DATA_CANONICAL_BASE" in original or
                      "PLEXTOR_FIXED_ROM_DATA_CANONICAL_BASE" in original or
                      "PLEXTOR_INDEXED_STATE_REFERENCE_CANONICAL_BASE" in original):
                    disposition = "linker_resolved_data_symbol"
                elif "PLEXTOR_CONFIGURATION_FLASH_ADDRESS" in original:
                    disposition = "verified_fixed_flash_alias"
                elif "PLEXTOR_RUNTIME_MONITOR_ADDRESS" in original:
                    disposition = "verified_fixed_monitor_entry"
                occurrences.append(Occurrence(
                    path=str(path.relative_to(ROOT)),
                    line=line_number,
                    literal=match.group(0),
                    address=address,
                    region=region,
                    disposition=disposition,
                    context=original,
                ))
    for path in sorted(SOURCE_ROOT.rglob("*.a51")):
        original_lines = path.read_text(encoding="utf-8").splitlines()
        for line_number, original_line in enumerate(original_lines, 1):
            line = original_line.split(";", 1)[0]
            original = original_line.strip()
            for match in A51_HEX_LITERAL.finditer(line):
                digits = match.group(1)
                address = int(digits, 16)
                if not is_address_candidate(digits, address, line, match.start()):
                    continue
                region, disposition = classify(address)
                if ("EQU" in line.upper() and
                        re.search(r"\b\w*_ENTRY\b", line, re.IGNORECASE)):
                    disposition = "verified_fixed_monitor_entry"
                occurrences.append(Occurrence(
                    path=str(path.relative_to(ROOT)),
                    line=line_number,
                    literal=match.group(0),
                    address=address,
                    region=region,
                    disposition=disposition,
                    context=original,
                ))
    return occurrences


def summary(occurrences: list[Occurrence]) -> dict:
    regions: dict[str, dict[str, int]] = {}
    unique_by_region: dict[str, set[int]] = {}
    for item in occurrences:
        entry = regions.setdefault(item.region, {"occurrences": 0, "unique": 0})
        entry["occurrences"] += 1
        unique_by_region.setdefault(item.region, set()).add(item.address)
    for region, values in unique_by_region.items():
        regions[region]["unique"] = len(values)
    risky = [item for item in occurrences if item.region in {
        "main_image_code_or_data", "main_image_fixed_data",
        "runtime_monitor_overlay", "hardware_or_flash_alias",
    } and item.disposition not in {
        "linker_resolved_code_symbol", "linker_resolved_data_symbol",
        "verified_fixed_flash_alias", "verified_fixed_monitor_entry",
    }]
    unresolved_regions: dict[str, dict[str, int]] = {}
    unresolved_by_region: dict[str, set[int]] = {}
    for item in risky:
        entry = unresolved_regions.setdefault(
            item.region, {"occurrences": 0, "unique": 0})
        entry["occurrences"] += 1
        unresolved_by_region.setdefault(item.region, set()).add(item.address)
    for region, values in unresolved_by_region.items():
        unresolved_regions[region]["unique"] = len(values)
    return {
        "source_root": str(SOURCE_ROOT.relative_to(ROOT)),
        "occurrences": len(occurrences),
        "unique_addresses": len({item.address for item in occurrences}),
        "priority_occurrences": len(risky),
        "priority_unique_addresses": len({item.address for item in risky}),
        "regions": regions,
        "unresolved_priority_regions": unresolved_regions,
    }


def read_ghidra_map(path: Path | None) -> dict[int, dict[str, str]]:
    if path is None:
        return {}
    result = {}
    with path.open(encoding="utf-8", newline="") as stream:
        for row in csv.DictReader(stream, delimiter="\t"):
            result[int(row["address"].split(":", 1)[1], 16)] = row
    return result


def correlation_summary(occurrences: list[Occurrence],
                        ghidra: dict[int, dict[str, str]]) -> dict:
    priority_regions = {
        "main_image_code_or_data", "main_image_fixed_data",
        "runtime_monitor_overlay", "hardware_or_flash_alias",
    }
    addresses = {
        item.address for item in occurrences
        if item.region in priority_regions and
        item.disposition not in {
            "linker_resolved_code_symbol", "linker_resolved_data_symbol",
            "verified_fixed_flash_alias", "verified_fixed_monitor_entry",
        }
    }
    classifications: dict[str, int] = {}
    for address in addresses:
        classification = ghidra.get(address, {}).get("classification", "not_correlated")
        classifications[classification] = classifications.get(classification, 0) + 1
    owners: dict[tuple[str, str, str], dict[str, object]] = {}
    for item in occurrences:
        if (item.region not in priority_regions or
                item.disposition in {
                    "linker_resolved_code_symbol", "linker_resolved_data_symbol",
                    "verified_fixed_flash_alias", "verified_fixed_monitor_entry",
                }):
            continue
        row = ghidra.get(item.address, {})
        key = (
            row.get("classification", "not_correlated"),
            row.get("owner_start", f"RAM:{item.address:06x}"),
            row.get("owner_name", ""),
        )
        owner = owners.setdefault(key, {"occurrences": 0, "addresses": set()})
        owner["occurrences"] = int(owner["occurrences"]) + 1
        addresses_for_owner = owner["addresses"]
        assert isinstance(addresses_for_owner, set)
        addresses_for_owner.add(item.address)
    ranked = []
    for (classification, start, name), values in owners.items():
        owner_addresses = values["addresses"]
        assert isinstance(owner_addresses, set)
        ranked.append({
            "classification": classification,
            "owner_start": start,
            "owner_name": name,
            "occurrences": values["occurrences"],
            "unique_addresses": len(owner_addresses),
        })
    ranked.sort(key=lambda item: (-int(item["occurrences"]),
                                  str(item["owner_start"])))
    return {
        "classifications": classifications,
        "top_owners": ranked,
    }


def write_markdown(path: Path, report: dict) -> None:
    source = report["summary"]
    correlation = report.get("ghidra_correlation", {})
    lines = [
        "# Absolute-address audit (generated)",
        "",
        "This report is a disposable source/Ghidra correlation view. The canonical",
        "address and type evidence remains in `/validated/MONT_104.bin`.",
        "",
        f"- Candidate occurrences: {source['occurrences']}",
        f"- Unique candidate addresses: {source['unique_addresses']}",
        f"- Priority ROM/overlay occurrences: {source['priority_occurrences']}",
        f"- Unique priority ROM/overlay addresses: {source['priority_unique_addresses']}",
        "",
        "## Source regions",
        "",
        "| Region | Occurrences | Unique addresses |",
        "|---|---:|---:|",
    ]
    for region, values in source["regions"].items():
        lines.append(
            f"| `{region}` | {values['occurrences']} | {values['unique']} |")
    lines.extend([
        "",
        "## Unresolved priority regions",
        "",
        "| Region | Occurrences | Unique addresses |",
        "|---|---:|---:|",
    ])
    for region, values in source["unresolved_priority_regions"].items():
        lines.append(
            f"| `{region}` | {values['occurrences']} | {values['unique']} |")
    if correlation:
        lines.extend([
            "",
            "## Canonical classification of priority addresses",
            "",
            "| Ghidra classification | Unique addresses |",
            "|---|---:|",
        ])
        for classification, count in sorted(
                correlation["classifications"].items(),
                key=lambda item: (-item[1], item[0])):
            lines.append(f"| `{classification}` | {count} |")
        lines.extend([
            "",
            "## Highest-use canonical owners",
            "",
            "| Owner | Class | Source occurrences | Literal addresses |",
            "|---|---|---:|---:|",
        ])
        for owner in correlation["top_owners"][:40]:
            label = owner["owner_name"] or owner["owner_start"]
            lines.append(
                f"| `{label}` (`{owner['owner_start']}`) | "
                f"`{owner['classification']}` | {owner['occurrences']} | "
                f"{owner['unique_addresses']} |")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_csv(path: Path, occurrences: list[Occurrence]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(asdict(occurrences[0]).keys()))
        writer.writeheader()
        for item in occurrences:
            row = asdict(item)
            row["address"] = f"RAM:{item.address:06x}"
            writer.writerow(row)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", type=Path, help="write detailed JSON report")
    parser.add_argument("--csv", type=Path, help="write occurrence CSV")
    parser.add_argument("--address-list", type=Path,
                        help="write unique RAM addresses for Ghidra correlation")
    parser.add_argument("--ghidra-map", type=Path,
                        help="TSV produced by ClassifyAbsoluteAddresses.java")
    parser.add_argument("--markdown", type=Path,
                        help="write generated Markdown summary")
    args = parser.parse_args()
    occurrences = inventory()
    ghidra = read_ghidra_map(args.ghidra_map)
    report = {
        "summary": summary(occurrences),
        "occurrences": [
            {**asdict(item), "address": f"RAM:{item.address:06x}",
             "ghidra": ghidra.get(item.address)}
            for item in occurrences
        ],
    }
    if ghidra:
        report["ghidra_correlation"] = correlation_summary(occurrences, ghidra)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if args.csv:
        write_csv(args.csv, occurrences)
    if args.address_list:
        args.address_list.parent.mkdir(parents=True, exist_ok=True)
        addresses = sorted({item.address for item in occurrences})
        args.address_list.write_text(
            "".join(f"RAM:{address:06x}\n" for address in addresses),
            encoding="ascii")
    if args.markdown:
        write_markdown(args.markdown, report)
    json.dump(report["summary"], sys.stdout, indent=2)
    print()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
