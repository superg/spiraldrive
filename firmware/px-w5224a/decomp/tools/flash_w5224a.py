#!/usr/bin/env python3
"""Execute a hash-bound PX-W5224A recovery-monitor firmware update on Windows.

Without --execute this tool only validates and summarizes the supplied plan.  An
executing run requires the exact candidate SHA-256 as a confirmation token,
requires the drive to identify as the captured revision-0.00 recovery monitor,
and records every WRITE BUFFER result.  It never retries a command: in
particular, an uncertain final mode-5 result must be investigated through the
protected recovery route rather than submitted again.
"""

from __future__ import annotations

import argparse
import ctypes
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
from typing import Callable

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import firmware_update_plan
from tools import probe_readonly_w5224a as readonly_probe


class FlashExecutionError(RuntimeError):
    pass


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise FlashExecutionError(message)


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def atomic_json(path: Path, value: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", dir=path.parent, delete=False
        ) as stream:
            temporary = Path(stream.name)
            json.dump(value, stream, indent=2)
            stream.write("\n")
        os.replace(temporary, path)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()


def load_bound_plan(
    plan_path: Path,
    image_path: Path,
    backup_report: Path,
    recovery_report: Path,
) -> tuple[dict[str, object], bytes]:
    image = image_path.read_bytes()
    expected = firmware_update_plan.build_plan(
        image,
        image_path=str(image_path.resolve()),
        backup_report=backup_report,
        recovery_report=recovery_report,
    )
    supplied = json.loads(plan_path.read_text(encoding="utf-8"))
    require(supplied == expected, "supplied plan is stale or differs from regenerated plan")
    require("live_stock_backup" in supplied, "plan is not bound to the stock backup")
    require("live_recovery_backup" in supplied, "plan is not bound to the recovery backup")
    candidate = supplied.get("candidate")
    require(isinstance(candidate, dict), "plan has no candidate record")
    require(candidate.get("sha256") == digest(image), "candidate hash differs from plan")
    transfers = supplied.get("transfers")
    require(isinstance(transfers, list) and len(transfers) == 32,
            "plan does not contain the expected 32 transfers")
    require(all(item.get("mode") == 4 for item in transfers[:-1]),
            "plan contains a non-mode-4 staging transfer")
    require(transfers[-1].get("mode") == 5
            and transfers[-1].get("activates_after_command") is True,
            "plan has no single final activating transfer")
    return supplied, image


def transfer_payload(image: bytes, transfer: dict[str, object]) -> tuple[bytes, bytes]:
    offset = int(transfer["offset"])
    length = int(transfer["length"])
    require(0 <= offset < len(image) and 0 < length <= len(image) - offset,
            "transfer range is outside candidate")
    payload = image[offset:offset + length]
    require(digest(payload) == transfer.get("payload_sha256"),
            f"transfer {transfer.get('index')} payload hash differs")
    cdb = bytes.fromhex(str(transfer["cdb_hex"]))
    require(cdb == firmware_update_plan.write_buffer_cdb(
        int(transfer["mode"]), int(transfer["buffer_id"]), offset, length
    ), f"transfer {transfer.get('index')} CDB differs from the stock contract")
    return cdb, payload


def validate_transfer_set(plan: dict[str, object], image: bytes) -> None:
    transfers = plan["transfers"]
    assert isinstance(transfers, list)
    covered = 0
    for expected_index, item in enumerate(transfers):
        require(isinstance(item, dict), "transfer record is not an object")
        require(item.get("index") == expected_index, "transfer indices are not contiguous")
        require(item.get("offset") == covered, "transfer offsets are not contiguous")
        _, payload = transfer_payload(image, item)
        covered += len(payload)
    require(covered == len(image), "transfer set does not cover the candidate exactly")


def sense_summary(sense: bytes) -> dict[str, object]:
    result: dict[str, object] = {"hex": sense.hex()}
    if len(sense) >= 14 and (sense[0] & 0x7F) in (0x70, 0x71):
        result.update({
            "sense_key": sense[2] & 0x0F,
            "asc": sense[12],
            "ascq": sense[13],
        })
    return result


def run_transfers(
    plan: dict[str, object],
    image: bytes,
    send: Callable[[bytes, bytes, int], tuple[int, bytes, int]],
    report: dict[str, object],
    save: Callable[[], None],
) -> None:
    transfers = plan["transfers"]
    assert isinstance(transfers, list)
    records = report["transfers"]
    assert isinstance(records, list)
    for item in transfers:
        assert isinstance(item, dict)
        cdb, payload = transfer_payload(image, item)
        final = bool(item["activates_after_command"])
        record: dict[str, object] = {
            "index": item["index"],
            "mode": item["mode"],
            "offset": item["offset"],
            "length": len(payload),
            "cdb_hex": cdb.hex(" "),
            "payload_sha256": digest(payload),
            "activates_after_command": final,
            "started_utc": utc_now(),
            "outcome": "submitted" if final else "in_progress",
        }
        records.append(record)
        report["state"] = "final_command_submitted" if final else "staging"
        save()
        try:
            status, sense, transferred = send(
                cdb, payload, int(item["device_timeout_seconds"])
            )
        except Exception as error:
            record["completed_utc"] = utc_now()
            record["error"] = str(error)
            record["outcome"] = (
                "indeterminate_after_activation_submission" if final else "failed"
            )
            report["state"] = record["outcome"]
            save()
            raise FlashExecutionError(
                "final command result is indeterminate; do not retry it"
                if final else f"staging transfer {item['index']} failed"
            ) from error
        record.update({
            "completed_utc": utc_now(),
            "scsi_status": status,
            "sense": sense_summary(sense),
            "transferred": transferred,
        })
        if status != 0 or transferred != len(payload):
            record["outcome"] = (
                "indeterminate_after_activation_submission" if final else "failed"
            )
            report["state"] = record["outcome"]
            save()
            raise FlashExecutionError(
                "final command did not return an unambiguous complete result; do not retry it"
                if final else f"staging transfer {item['index']} was not accepted completely"
            )
        record["outcome"] = "accepted"
        report["state"] = "activation_command_accepted" if final else "staging"
        save()


def run_windows_data_out(
    device: str, cdb: bytes, payload: bytes, timeout_seconds: int
) -> tuple[int, bytes, int]:
    """Issue exactly one DATA-OUT CDB through native Windows SPTI."""
    if os.name != "nt":
        raise FlashExecutionError("live execution is supported only on Windows")
    require(1 <= len(cdb) <= 16 and payload and timeout_seconds > 0,
            "invalid DATA-OUT transfer shape")
    device_path = readonly_probe.windows_device_path(device)
    handle = readonly_probe._KERNEL32.CreateFileW(
        device_path,
        readonly_probe._GENERIC_READ | readonly_probe._GENERIC_WRITE,
        readonly_probe._FILE_SHARE_READ | readonly_probe._FILE_SHARE_WRITE,
        None,
        readonly_probe._OPEN_EXISTING,
        0,
        None,
    )
    if handle == readonly_probe._INVALID_HANDLE_VALUE:
        error = ctypes.get_last_error()
        raise OSError(error, f"CreateFileW({device_path}) failed")
    try:
        data = (ctypes.c_ubyte * len(payload)).from_buffer_copy(payload)
        packet = readonly_probe.ScsiPassThroughDirectWithSense()
        packet.sptd.Length = ctypes.sizeof(readonly_probe.ScsiPassThroughDirect)
        packet.sptd.CdbLength = len(cdb)
        packet.sptd.SenseInfoLength = len(packet.sense)
        packet.sptd.DataIn = 0  # SCSI_IOCTL_DATA_OUT
        packet.sptd.DataTransferLength = len(payload)
        packet.sptd.TimeOutValue = timeout_seconds
        packet.sptd.DataBuffer = ctypes.cast(data, ctypes.c_void_p)
        packet.sptd.SenseInfoOffset = (
            readonly_probe.ScsiPassThroughDirectWithSense.sense.offset
        )
        for index, value in enumerate(cdb):
            packet.sptd.Cdb[index] = value
        returned = readonly_probe.wintypes.DWORD()
        success = readonly_probe._KERNEL32.DeviceIoControl(
            handle,
            readonly_probe._IOCTL_SCSI_PASS_THROUGH_DIRECT,
            ctypes.byref(packet),
            ctypes.sizeof(packet),
            ctypes.byref(packet),
            ctypes.sizeof(packet),
            ctypes.byref(returned),
            None,
        )
        if not success:
            error = ctypes.get_last_error()
            raise OSError(error, "IOCTL_SCSI_PASS_THROUGH_DIRECT DATA OUT failed")
        sense = bytes(packet.sense[: packet.sptd.SenseInfoLength])
        return (
            int(packet.sptd.ScsiStatus),
            sense,
            min(int(packet.sptd.DataTransferLength), len(payload)),
        )
    finally:
        readonly_probe._KERNEL32.CloseHandle(handle)


def summary(plan: dict[str, object]) -> dict[str, object]:
    candidate = plan["candidate"]
    assert isinstance(candidate, dict)
    return {
        "candidate_sha256": candidate["sha256"],
        "candidate_checksum": candidate["checksum"],
        "transfer_count": plan["transfer_count"],
        "staging_transfers": 31,
        "activating_transfers": 1,
        "required_live_identity": {
            "vendor": "PLEXTOR",
            "product": "CD-R   PX-W5224A",
            "revision": "0.00",
        },
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--device", default="H:")
    parser.add_argument("--image", type=Path, required=True)
    parser.add_argument("--plan", type=Path, required=True)
    parser.add_argument("--backup-report", type=Path, required=True)
    parser.add_argument("--recovery-report", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--execute", action="store_true")
    parser.add_argument("--confirm-candidate-sha256")
    args = parser.parse_args(argv)
    try:
        plan, image = load_bound_plan(
            args.plan, args.image, args.backup_report, args.recovery_report
        )
        validate_transfer_set(plan, image)
        rendered_summary = summary(plan)
        if not args.execute:
            print(json.dumps({"scope": "validation only; no device opened", **rendered_summary}, indent=2))
            return 0

        require(os.name == "nt", "live execution is supported only on Windows")
        require(args.output is not None, "--output is required with --execute")
        require(not args.output.exists(), "execution output directory already exists")
        expected_hash = str(rendered_summary["candidate_sha256"])
        require(args.confirm_candidate_sha256 == expected_hash,
                "execution confirmation must equal the complete candidate SHA-256")
        args.output.mkdir(parents=True)
        report_path = args.output / "report.json"
        report: dict[str, object] = {
            "schema_version": 1,
            "scope": "live DATA OUT firmware update",
            "state": "validating_live_identity",
            "started_utc": utc_now(),
            "device": readonly_probe.windows_device_path(args.device),
            "plan_path": str(args.plan.resolve()),
            "plan_sha256": digest(args.plan.read_bytes()),
            "image_path": str(args.image.resolve()),
            **rendered_summary,
            "transfers": [],
        }

        def save() -> None:
            atomic_json(report_path, report)

        save()
        inquiry_cdb = (0x12, 0, 0, 0, 0x60, 0)
        inquiry, status, sense = readonly_probe.run_windows_data_in(
            args.device, 96, inquiry_cdb
        )
        require(status == 0, f"recovery INQUIRY returned SCSI status {status}")
        identity = readonly_probe.identify(inquiry, readonly_probe.RECOVERY_REVISION)
        report["live_identity"] = identity
        report["inquiry_sense"] = sense_summary(sense)
        report["state"] = "identity_confirmed"
        save()
        run_transfers(
            plan,
            image,
            lambda cdb, payload, timeout: run_windows_data_out(
                args.device, cdb, payload, timeout
            ),
            report,
            save,
        )
        report["completed_utc"] = utc_now()
        report["state"] = "activation_command_accepted"
        save()
        print(report_path)
        return 0
    except (OSError, ValueError, FlashExecutionError) as error:
        print(f"flash_w5224a: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
