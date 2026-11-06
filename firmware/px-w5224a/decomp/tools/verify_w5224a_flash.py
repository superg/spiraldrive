#!/usr/bin/env python3
"""Read back and verify a PX-W5224A after an operation-3 firmware update.

The default invocation validates references and prints the expected composite
image without opening a device.  --capture performs only INQUIRY and fixed F5
DATA-IN reads.  It accepts either normal revision 1.04 or, with --recovery, the
protected revision-0.00 monitor after a failed normal boot.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import firmware_image
from tools import firmware_update_plan
from tools import probe_readonly_w5224a as probe


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def expected_post_flash_view(candidate: bytes, monitor: bytes) -> bytes:
    stock = firmware_update_plan.STOCK.read_bytes()
    firmware_image.verify(candidate, fixed_data_reference=stock)
    if len(monitor) != 0x10000 or digest(monitor) != firmware_update_plan.MONITOR_SHA256:
        raise ValueError("protected monitor capture differs")
    # Runtime operation 3 programs candidate offsets 00000..6ffff and
    # 78000..79fff.  The protected monitor and trailing configuration remain
    # from the pre-update runtime capture.
    result = (
        candidate[:0x70000]
        + monitor[:0x8000]
        + candidate[0x78000:0x7A000]
        + monitor[0xA000:0x10000]
    )
    if len(result) != firmware_image.SIZE:
        raise ValueError("post-flash composite is not 512 KiB")
    return result


def compare_view(actual: bytes, expected: bytes, candidate: bytes, monitor: bytes) -> dict[str, object]:
    checks = {
        "complete_view_equals_expected_composite": actual == expected,
        "main_00000_6ffff_equals_candidate": actual[:0x70000] == candidate[:0x70000],
        "protected_monitor_70000_77fff_preserved": actual[0x70000:0x78000] == monitor[:0x8000],
        "fixed_and_checksum_78000_79fff_equals_candidate": (
            actual[0x78000:0x7A000] == candidate[0x78000:0x7A000]
        ),
        "configuration_7a000_7ffff_preserved": (
            actual[0x7A000:] == monitor[0xA000:0x10000]
        ),
    }
    differing = sum(left != right for left, right in zip(actual, expected))
    return {
        "length": len(actual),
        "sha256": digest(actual),
        "expected_sha256": digest(expected),
        "differing_bytes": differing,
        "checks": checks,
        "pass": all(checks.values()),
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--device", default="H:")
    parser.add_argument("--image", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--capture", action="store_true")
    parser.add_argument("--recovery", action="store_true")
    args = parser.parse_args(argv)
    try:
        candidate = args.image.read_bytes()
        monitor = firmware_update_plan.MONITOR.read_bytes()
        expected = expected_post_flash_view(candidate, monitor)
        expected_summary = {
            "candidate_sha256": digest(candidate),
            "expected_composite_sha256": digest(expected),
            "expected_revision": (
                probe.RECOVERY_REVISION if args.recovery else probe.NORMAL_REVISION
            ),
            "commands": 33,
            "direction": "DATA IN only",
        }
        if not args.capture:
            print(json.dumps({"scope": "validation only; no device opened", **expected_summary}, indent=2))
            return 0
        if args.output is None:
            raise ValueError("--output is required with --capture")
        if args.output.exists():
            raise ValueError("capture output directory already exists")
        args.output.mkdir(parents=True)
        inquiry, status, sense = probe.run_windows_data_in(
            args.device, 96, (0x12, 0, 0, 0, 0x60, 0)
        )
        if status != 0:
            raise ValueError(f"INQUIRY returned SCSI status {status}")
        identity = probe.identify(inquiry, str(expected_summary["expected_revision"]))
        report: dict[str, object] = {
            "schema_version": 1,
            "scope": "read-only post-flash verification",
            "device": probe.windows_device_path(args.device),
            **expected_summary,
            "identity": identity,
            "inquiry_sense_hex": sense.hex(),
            "probes": [],
        }
        views: dict[str, list[bytes]] = {"programming": [], "execution": []}
        for name, length, cdb_tuple in probe.FULL_BACKUP_PROBES:
            payload, scsi_status, command_sense = probe.run_windows_data_in(
                args.device, length, cdb_tuple
            )
            item = {
                "name": name,
                "direction": "DATA IN",
                "cdb_hex": bytes(cdb_tuple).hex(" "),
                "requested_length": length,
                "returned_length": len(payload),
                "scsi_status": scsi_status,
                "sense_hex": command_sense.hex(),
                "sha256": digest(payload),
            }
            report["probes"].append(item)
            if scsi_status != 0 or len(payload) != length:
                report["pass"] = False
                report["error"] = f"{name}: incomplete or non-GOOD DATA IN result"
                (args.output / "report.json").write_text(
                    json.dumps(report, indent=2) + "\n", encoding="utf-8"
                )
                return 1
            views["programming" if name.startswith("backup_f0") else "execution"].append(payload)
        programming = b"".join(views["programming"])
        execution = b"".join(views["execution"])
        (args.output / "cpu_data_f00000_f7ffff.bin").write_bytes(programming)
        (args.output / "cpu_data_f80000_ffffff.bin").write_bytes(execution)
        report["programming_view"] = compare_view(programming, expected, candidate, monitor)
        report["execution_view"] = compare_view(execution, expected, candidate, monitor)
        report["views_equal"] = programming == execution
        report["pass"] = (
            report["views_equal"]
            and report["programming_view"]["pass"]
            and report["execution_view"]["pass"]
        )
        (args.output / "report.json").write_text(
            json.dumps(report, indent=2) + "\n", encoding="utf-8"
        )
        print(args.output / "report.json")
        return 0 if report["pass"] else 1
    except (OSError, ValueError, firmware_image.ImageError) as error:
        print(f"verify_w5224a_flash: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
