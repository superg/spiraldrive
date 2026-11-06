#!/usr/bin/env python3
"""Build a read-only PX-W5224A firmware-update command plan.

This tool never opens a device. It validates the candidate container, the
immutable stock reference, PlexFirm.ini, and the retained PXFirm3.exe before
describing the DATA OUT commands used by the stock updater.
"""

from __future__ import annotations

import argparse
import configparser
import hashlib
import json
from pathlib import Path
import struct
import sys

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import firmware_image


ROOT = Path(__file__).resolve().parents[1]
STOCK = ROOT / "data/W5224A104/MONT_104.bin"
UPDATER = ROOT / "data/W5224A104/PXFirm3.exe"
CONFIGURATION = ROOT / "data/W5224A104/PlexFirm.ini"
MONITOR = ROOT / "data/read_memory_ff0000.debug"

UPDATER_SHA256 = "a564786456d9cd1b16ab9c5e0177ab768d949ec923a78be91b996647a4dd50cd"
CONFIGURATION_SHA256 = "0824164c8eb37838ae2bb2c6ee260350ffcb9bcd8a3e58e6cac328b45f6a6e4f"
MONITOR_SHA256 = "7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80"
RECOVERY_PROBE_SCRIPT = ROOT / "tools/probe_readonly_w5224a.py"
WRITE_BUFFER = 0x3B
MODE_DOWNLOAD = 4
MODE_DOWNLOAD_AND_ACTIVATE = 5
MAIN_BUFFER_ID = 0


class UpdatePlanError(ValueError):
    pass


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise UpdatePlanError(message)


def pe32_va_slice(image: bytes, virtual_address: int, length: int) -> bytes:
    require(len(image) >= 0x40 and image[:2] == b"MZ", "updater is not a PE image")
    pe_offset = struct.unpack_from("<I", image, 0x3C)[0]
    require(pe_offset + 24 <= len(image) and image[pe_offset:pe_offset + 4] == b"PE\0\0",
            "updater has no valid PE header")
    section_count = struct.unpack_from("<H", image, pe_offset + 6)[0]
    optional_size = struct.unpack_from("<H", image, pe_offset + 20)[0]
    optional_offset = pe_offset + 24
    require(optional_offset + optional_size <= len(image), "updater optional header is truncated")
    require(struct.unpack_from("<H", image, optional_offset)[0] == 0x10B,
            "updater is not PE32")
    image_base = struct.unpack_from("<I", image, optional_offset + 28)[0]
    require(virtual_address >= image_base, "virtual address is below the PE image base")
    rva = virtual_address - image_base
    section_offset = optional_offset + optional_size
    for index in range(section_count):
        header = section_offset + index * 40
        require(header + 40 <= len(image), "updater section table is truncated")
        virtual_size, section_rva, raw_size, raw_offset = struct.unpack_from(
            "<IIII", image, header + 8
        )
        if section_rva <= rva and rva + length <= section_rva + max(virtual_size, raw_size):
            offset = raw_offset + rva - section_rva
            require(offset + length <= len(image), "updater VA maps beyond file data")
            return image[offset:offset + length]
    raise UpdatePlanError(f"updater VA 0x{virtual_address:08x} is not file-backed")


def validate_updater_recovery_version_gate(updater: bytes) -> None:
    require(
        pe32_va_slice(updater, 0x00405DD7, 16)
        == bytes.fromhex("8a 15 28 1a 44 00 b1 30 3a d1 0f 84 c6 00 00 00"),
        "updater candidate zero-prefix gate differs",
    )
    require(
        pe32_va_slice(updater, 0x00405DE7, 12)
        == bytes.fromhex("38 88 66 01 44 00 0f 84 ba 00 00 00"),
        "updater drive zero-prefix gate differs",
    )
    require(
        pe32_va_slice(updater, 0x00405EC2, 5) == bytes.fromhex("b8 01 00 00 00"),
        "updater accepted successor no longer returns true",
    )


def load_contract(configuration: Path = CONFIGURATION,
                  updater: Path = UPDATER) -> tuple[int, int]:
    configuration_bytes = configuration.read_bytes()
    updater_bytes = updater.read_bytes()
    require(digest(configuration_bytes) == CONFIGURATION_SHA256,
            "PlexFirm.ini is not the retained PX-W5224A 1.04 configuration")
    require(digest(updater_bytes) == UPDATER_SHA256,
            "PXFirm3.exe is not the retained stock updater")
    validate_updater_recovery_version_gate(updater_bytes)
    parser = configparser.ConfigParser()
    parser.read_string(configuration_bytes.decode("ascii"))
    require(parser.get("UpgradeInformation", "FirmWareFileName") == "MONT_104.bin",
            "unexpected firmware file name")
    image_size = parser.getint("UpgradeInformation", "FirmwareFileSize")
    buffer_size = parser.getint("UpgradeInformation", "BufferSize")
    require(parser.getboolean("Option", "Use Update After Cmd"),
            "retained updater is not configured for final-chunk activation")
    require(image_size == firmware_image.SIZE, "unexpected updater image size")
    require(buffer_size == 0x4000, "unexpected updater transfer size")
    require(image_size % buffer_size == 0, "image is not an integral transfer count")
    return image_size, buffer_size


def write_buffer_cdb(mode: int, buffer_id: int, offset: int, length: int) -> bytes:
    require(mode in (MODE_DOWNLOAD, MODE_DOWNLOAD_AND_ACTIVATE),
            "unsupported update mode")
    require(0 <= buffer_id <= 0xFF, "buffer ID exceeds one byte")
    require(0 <= offset <= 0xFFFFFF, "buffer offset exceeds three bytes")
    require(0 < length <= 0xFFFFFF, "transfer length exceeds three bytes")
    return bytes((
        WRITE_BUFFER,
        mode,
        buffer_id,
        (offset >> 16) & 0xFF,
        (offset >> 8) & 0xFF,
        offset & 0xFF,
        (length >> 16) & 0xFF,
        (length >> 8) & 0xFF,
        length & 0xFF,
        0,
    ))


def image_range_record(
    image: bytes, stock: bytes, *, offset: int, length: int, role: str,
    programming_address: int | None = None,
) -> dict[str, object]:
    require(0 <= offset < len(image) and 0 < length <= len(image) - offset,
            "invalid image range")
    candidate_bytes = image[offset:offset + length]
    stock_bytes = stock[offset:offset + length]
    record: dict[str, object] = {
        "role": role,
        "image_offset": offset,
        "length": length,
        "candidate_sha256": digest(candidate_bytes),
        "stock_sha256": digest(stock_bytes),
        "candidate_equals_stock": candidate_bytes == stock_bytes,
    }
    if programming_address is not None:
        record["cpu_programming_address"] = f"0x{programming_address:06x}"
    return record


def validate_full_backup_captures(
    report: dict[str, object], path: Path
) -> dict[str, dict[str, object]]:
    full_backup = report.get("full_backup")
    require(isinstance(full_backup, dict), "backup report has no full captures")
    captures: dict[str, bytes] = {}
    capture_records: dict[str, dict[str, object]] = {}
    for name in ("programming_view", "execution_view"):
        record = full_backup.get(name)
        require(isinstance(record, dict), f"backup report has no {name}")
        capture_path = Path(str(record.get("path", "")))
        if not capture_path.is_absolute():
            capture_path = path.parent / capture_path
        data = capture_path.read_bytes()
        require(len(data) == firmware_image.SIZE, f"{name} is not 512 KiB")
        require(record.get("length") == len(data), f"{name} length metadata differs")
        require(record.get("sha256") == digest(data), f"{name} hash metadata differs")
        captures[name] = data
        capture_records[name] = {
            "path": str(capture_path.resolve()),
            "length": len(data),
            "sha256": digest(data),
        }

    stock = STOCK.read_bytes()
    monitor = MONITOR.read_bytes()
    require(digest(monitor) == MONITOR_SHA256, "runtime monitor hash differs")
    expected_execution = stock[:0x70000] + monitor
    require(captures["programming_view"] == captures["execution_view"],
            "captured programming and execution views differ")
    require(captures["execution_view"] == expected_execution,
            "captured execution view is not stock main plus retained monitor")
    return capture_records


def validate_backup_report(path: Path) -> dict[str, object]:
    report_bytes = path.read_bytes()
    report = json.loads(report_bytes)
    require(report.get("schema_version") == 1, "unsupported backup report schema")
    require(report.get("stock_sha256") == firmware_image.STOCK_SHA256,
            "backup report uses a different stock reference")
    require(report.get("monitor_sha256") == MONITOR_SHA256,
            "backup report uses a different runtime monitor")
    identity = report.get("identity")
    require(identity == {
        "vendor": "PLEXTOR",
        "product": "CD-R   PX-W5224A",
        "revision": "1.04",
    }, "backup report does not identify the expected stock drive")
    checks = report.get("checks")
    require(isinstance(checks, dict), "backup report has no checks")
    required_checks = (
        "flash_id_selector_recognized",
        "full_f00000_f7ffff_equals_f80000_ffffff",
        "full_main_f80000_feffff_equals_stock",
        "full_recovery_ff0000_ffffff_equals_capture",
        "full_execution_view_equals_stock_plus_monitor",
    )
    for name in required_checks:
        require(checks.get(name) is True, f"backup check did not pass: {name}")
    require(checks.get("flash_id_selector_ram_f8_hex") == "bf00",
            "live backup does not select the observed bf00 programmer branch")

    capture_records = validate_full_backup_captures(report, path)
    return {
        "report_path": str(path.resolve()),
        "report_sha256": digest(report_bytes),
        "identity": identity,
        "flash_id_selector": "bf00",
        "captures": capture_records,
        "all_required_checks_pass": True,
    }


def validate_recovery_report(path: Path) -> dict[str, object]:
    report_bytes = path.read_bytes()
    report = json.loads(report_bytes)
    require(report.get("schema_version") == 1, "unsupported recovery report schema")
    require(report.get("firmware_mode") == "recovery",
            "recovery report was not captured in recovery mode")
    require(report.get("expected_revision") == "0.00",
            "recovery report expected a different revision")
    require(report.get("stock_sha256") == firmware_image.STOCK_SHA256,
            "recovery report uses a different stock reference")
    require(report.get("monitor_sha256") == MONITOR_SHA256,
            "recovery report uses a different runtime monitor")
    expected_identity = {
        "vendor": "PLEXTOR",
        "product": "CD-R   PX-W5224A",
        "revision": "0.00",
    }
    require(report.get("captured_recovery_identity") == expected_identity,
            "recovery report is not bound to the captured monitor identity")
    require(report.get("identity") == expected_identity,
            "recovery report does not identify the protected monitor")
    require(report.get("script_sha256") == digest(RECOVERY_PROBE_SCRIPT.read_bytes()),
            "recovery report was not produced by the current read-only helper")
    require(report.get("pass") is True, "recovery report did not pass")

    checks = report.get("checks")
    require(isinstance(checks, dict), "recovery report has no checks")
    required_checks = (
        "expected_inquiry_revision",
        "f80050_equals_stock",
        "f00050_equals_f80050",
        "cpu_data_read_alias_supported",
        "ff0000_equals_runtime_capture",
        "f70000_equals_ff0000",
        "ff2a3e_equals_runtime_capture",
        "f72a3e_equals_ff2a3e",
        "ff8000_equals_stock_fixed_data",
        "f78000_equals_ff8000",
        "recovery_monitor_identity_supported",
        "full_f00000_f7ffff_equals_f80000_ffffff",
        "full_main_f80000_feffff_equals_stock",
        "full_recovery_ff0000_ffffff_equals_capture",
        "full_execution_view_equals_stock_plus_monitor",
    )
    for name in required_checks:
        require(checks.get(name) is True, f"recovery check did not pass: {name}")
    declared_checks = report.get("required_checks")
    require(isinstance(declared_checks, list), "recovery report has no required-check list")
    require(all(name in declared_checks for name in required_checks),
            "recovery report omitted a required check")

    probes = report.get("probes")
    require(isinstance(probes, list) and len(probes) == 41,
            "recovery report is not the nine-probe plus full-backup profile")
    require(all(isinstance(item, dict) and item.get("direction") == "DATA IN"
                for item in probes),
            "recovery report contains a non-DATA-IN command")
    capture_records = validate_full_backup_captures(report, path)
    return {
        "report_path": str(path.resolve()),
        "report_sha256": digest(report_bytes),
        "identity": expected_identity,
        "captures": capture_records,
        "probe_count": len(probes),
        "all_required_checks_pass": True,
    }


def build_plan(image: bytes, *, image_path: str,
               configuration: Path = CONFIGURATION,
               updater: Path = UPDATER,
               backup_report: Path | None = None,
               recovery_report: Path | None = None) -> dict[str, object]:
    image_size, buffer_size = load_contract(configuration, updater)
    stock = STOCK.read_bytes()
    require(digest(stock) == firmware_image.STOCK_SHA256,
            "immutable stock firmware hash differs")
    validation = firmware_image.verify(image, fixed_data_reference=stock)
    require(len(image) == image_size, "candidate size differs from updater contract")

    programmed_ranges = [
        image_range_record(
            image, stock, offset=0x00000, length=0x70000,
            programming_address=0xF00000,
            role="main executable and constants",
        ),
        image_range_record(
            image, stock, offset=0x78000, length=0x01FFE,
            programming_address=0xF78000,
            role="stock-identical fixed data",
        ),
        image_range_record(
            image, stock, offset=0x79FFE, length=0x00002,
            programming_address=0xF79FFE,
            role="candidate stored checksum",
        ),
    ]
    preserved_ranges = [
        image_range_record(
            image, stock, offset=0x70000, length=0x08000,
            role="download recovery hole; protected monitor is a separate runtime view",
        ),
        image_range_record(
            image, stock, offset=0x7A000, length=0x06000,
            role="erased trailing container padding",
        ),
    ]
    require(sum(int(item["length"]) for item in programmed_ranges + preserved_ranges)
            == image_size, "programmed/preserved ranges do not cover the image")
    require(all(item["candidate_equals_stock"] for item in preserved_ranges),
            "candidate differs from stock in a non-programmed image range")

    transfers: list[dict[str, object]] = []
    for index, offset in enumerate(range(0, image_size, buffer_size)):
        final = offset + buffer_size == image_size
        mode = MODE_DOWNLOAD_AND_ACTIVATE if final else MODE_DOWNLOAD
        payload = image[offset:offset + buffer_size]
        cdb = write_buffer_cdb(mode, MAIN_BUFFER_ID, offset, len(payload))
        transfers.append({
            "index": index,
            "mode": mode,
            "buffer_id": MAIN_BUFFER_ID,
            "offset": offset,
            "length": len(payload),
            "cdb_hex": cdb.hex(" "),
            "payload_sha256": digest(payload),
            "activates_after_command": final,
            "device_timeout_seconds": 600 if final else 10,
        })

    require(len(transfers) == 32, "unexpected transfer count")
    require(sum(int(item["length"]) for item in transfers) == image_size,
            "transfer plan does not cover the complete image")
    require(all(item["mode"] == MODE_DOWNLOAD for item in transfers[:-1]),
            "non-final transfer is not mode 4")
    require(transfers[-1]["mode"] == MODE_DOWNLOAD_AND_ACTIVATE,
            "final transfer is not mode 5")
    plan: dict[str, object] = {
        "schema_version": 1,
        "scope": "read-only command planning; no device was opened and no CDB was sent",
        "device_model": "PLEXTOR CD-R PX-W5224A",
        "required_installed_revision": "1.04",
        "candidate": {"path": image_path, **validation},
        "stock_reference_sha256": firmware_image.STOCK_SHA256,
        "pxfirm3_sha256": UPDATER_SHA256,
        "plexfirm_ini_sha256": CONFIGURATION_SHA256,
        "stock_updater_evidence": {
            "write_buffer_wrapper_va": "0x00401790",
            "update_loop_va": "0x00406320",
            "mode4_call_va": "0x00406402",
            "final_mode5_call_va": "0x00406464",
            "spti_ioctl": "0x0004d014",
            "note": "Addresses apply only to the hash-pinned retained PXFirm3.exe.",
        },
        "recovery_compatibility_evidence": {
            "monitor_inquiry_revision": "0.00",
            "updater_version_gate_va": "0x00405cb0",
            "candidate_zero_prefix_test_va": "0x00405dd7",
            "drive_zero_prefix_test_va": "0x00405de7",
            "accepted_successor_va": "0x00405ead",
            "conclusion": (
                "The hash-pinned updater bypasses its later-version comparison "
                "when either four-byte revision begins with ASCII 0; monitor "
                "revision 0.00 is therefore not rejected by this version gate."
            ),
            "limitation": (
                "The bound revision-0.00 read-only preflight proves live recovery "
                "enumeration and DATA-IN access, not WRITE BUFFER transfer or flash effects."
                if recovery_report is not None else
                "This does not prove live recovery enumeration or transfer; a "
                "positive revision-0.00 read-only preflight is still required."
            ),
        },
        "image_size": image_size,
        "buffer_size": buffer_size,
        "transfer_count": len(transfers),
        "transfers": transfers,
        "device_programmer_range_contract": {
            "runtime_operation": 3,
            "programmed_image_ranges": programmed_ranges,
            "preserved_image_ranges": preserved_ranges,
            "warning": (
                "These are recovered CPU/staging ranges. The SST command address "
                "and byte ordering are strongly evidenced below; physical erase/program "
                "effects remain unobserved until an authorized write."
            ),
        },
        "flash_bus_mapping_evidence": {
            "confidence": "strong inference for the stock operation-3 update path",
            "board_flash": "SST39LF400A, 4 Mbit x16",
            "manufacturer_id_datasheet_word": "0x00bf",
            "live_cpu_selector_word": "0xbf00",
            "byte_order_conclusion": (
                "The live byte-swapped manufacturer word and the programmer's "
                "0xaa00/0x5500 command words agree that the CPU word high byte "
                "reaches the SST command byte DQ7..DQ0."
            ),
            "address_conclusion": (
                "CPU byte offsets 0xaaaa and 0x5554 divide by two to SST x16 word "
                "addresses 0x5555 and 0x2aaa, exactly matching the manufacturer's "
                "unlock sequence."
            ),
            "read_alias_conclusion": (
                "Bound normal and recovery captures prove the complete CPU-data "
                "f00000..f7ffff view equals f80000..ffffff before programming."
            ),
            "scope": (
                "The planned buffer-ID-zero finalization selects runtime operation 3. "
                "The separate auxiliary operation-5 erase-range concern is not part "
                "of this 32-transfer full-image update."
            ),
            "remaining_observation": (
                "No non-destructive read can observe an erase/program bus cycle; the "
                "first authorized update is the remaining physical-effect observation."
            ),
        },
        "activation": {
            "mechanism": "final WRITE BUFFER mode-5 command",
            "self_activating": True,
            "warning": "The final command enters the drive programmer; this plan is not authorization to send it.",
        },
    }
    if backup_report is not None:
        plan["live_stock_backup"] = validate_backup_report(backup_report)
    if recovery_report is not None:
        plan["live_recovery_backup"] = validate_recovery_report(recovery_report)
    return plan


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", type=Path, required=True)
    parser.add_argument(
        "--backup-report",
        type=Path,
        help="bind the plan to a completed read-only full-backup report",
    )
    parser.add_argument(
        "--recovery-report",
        type=Path,
        help="bind the plan to a passing recovery-mode full-backup report",
    )
    parser.add_argument("--output", type=Path)
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args(argv)
    try:
        image = args.image.read_bytes()
        plan = build_plan(
            image,
            image_path=str(args.image.resolve()),
            backup_report=args.backup_report,
            recovery_report=args.recovery_report,
        )
        rendered = json.dumps(plan, indent=2) + "\n"
        if args.output is not None:
            inputs = (
                (args.image,)
                + ((args.backup_report,) if args.backup_report else ())
                + ((args.recovery_report,) if args.recovery_report else ())
            )
            firmware_image.write_output(args.output, rendered.encode("utf-8"), inputs)
        if not args.quiet:
            print(rendered, end="")
        return 0
    except (OSError, ValueError, configparser.Error) as error:
        print(f"firmware_update_plan: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
