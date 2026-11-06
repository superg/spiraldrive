#!/usr/bin/env python3
"""Capture fixed DATA IN probes from an identified PX-W5224A drive.

All CDBs are fixed here and send no DATA OUT payload. Linux uses sg_raw's
read-only mode. Windows uses native SCSI pass-through, whose IOCTL requires a
read/write-capable handle even for DATA IN. The tool refuses vendor probes
unless standard INQUIRY identifies the expected drive and either normal 1.04
firmware or the captured recovery monitor's 0.00 revision.
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
import re
import shutil
import subprocess
from pathlib import Path

STOCK_SHA256 = "622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2"
MONITOR_SHA256 = "7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80"
NORMAL_REVISION = "1.04"
RECOVERY_REVISION = "0.00"
RECOVERY_INQUIRY_TEMPLATE_OFFSET = 0x7FC0
RECOVERY_INQUIRY_TEMPLATE = b"PLEXTOR CD-R   PX-W5224A0.00"
PROBES = (
    ("inquiry", 96, (0x12, 0, 0, 0, 0x60, 0)),
    ("mode_page03_f5", 8, (0xF5, 0, 0, 0, 0, 0x2B, 0xC6, 0xEF, 0, 8)),
    ("mode_page03_sense10", 16, (0x5A, 0, 0x03, 0, 0, 0, 0, 0, 0x10, 0)),
    ("image_f80050_f5", 16, (0xF5, 0, 0, 0, 0, 0xF8, 0, 0x50, 0, 0x10)),
    ("alias_f00050_f5", 16, (0xF5, 0, 0, 0, 0, 0xF0, 0, 0x50, 0, 0x10)),
    ("flash_id_selector_f5", 2, (0xF5, 0, 0, 0, 0, 0, 0, 0xF8, 0, 2)),
    ("data_ff0000_f5", 16, (0xF5, 0, 0, 0, 0, 0xFF, 0, 0, 0, 0x10)),
    ("alias_f70000_f5", 16, (0xF5, 0, 0, 0, 0, 0xF7, 0, 0, 0, 0x10)),
    ("data_ff2a3e_f5", 16, (0xF5, 0, 0, 0, 0, 0xFF, 0x2A, 0x3E, 0, 0x10)),
    ("alias_f72a3e_f5", 16, (0xF5, 0, 0, 0, 0, 0xF7, 0x2A, 0x3E, 0, 0x10)),
    ("data_ff8000_f5", 16, (0xF5, 0, 0, 0, 0, 0xFF, 0x80, 0, 0, 0x10)),
    ("alias_f78000_f5", 16, (0xF5, 0, 0, 0, 0, 0xF7, 0x80, 0, 0, 0x10)),
)

RECOVERY_PROBE_NAMES = frozenset({
    "inquiry",
    "image_f80050_f5",
    "alias_f00050_f5",
    "data_ff0000_f5",
    "alias_f70000_f5",
    "data_ff2a3e_f5",
    "alias_f72a3e_f5",
    "data_ff8000_f5",
    "alias_f78000_f5",
})
RECOVERY_PROBES = tuple(probe for probe in PROBES if probe[0] in RECOVERY_PROBE_NAMES)


def f5_read_probe(name: str, address: int, length: int) -> tuple[str, int, tuple[int, ...]]:
    if not 0 <= address <= 0xFFFFFF or not 1 <= length <= 0xFFFF:
        raise ValueError("F5 read is outside the 24-bit address/16-bit length contract")
    return (
        name,
        length,
        (
            0xF5, 0, 0, 0, 0,
            (address >> 16) & 0xFF,
            (address >> 8) & 0xFF,
            address & 0xFF,
            (length >> 8) & 0xFF,
            length & 0xFF,
        ),
    )


FULL_BACKUP_PROBES = tuple(
    f5_read_probe(f"backup_{base:06x}_{offset:05x}", base + offset, 0x8000)
    for base in (0xF00000, 0xF80000)
    for offset in range(0, 0x80000, 0x8000)
)
READ_ONLY_PROBES = PROBES + FULL_BACKUP_PROBES

COMMON_REQUIRED_CHECKS = (
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
)
NORMAL_REQUIRED_CHECKS = (
    "mode_sense_page_header_03_06",
    "current_page_f5_equals_mode_sense",
    "current_page_read_supported",
    "flash_id_selector_recognized",
)
RECOVERY_REQUIRED_CHECKS = ("recovery_monitor_identity_supported",)
FULL_BACKUP_REQUIRED_CHECKS = (
    "full_f00000_f7ffff_equals_f80000_ffffff",
    "full_main_f80000_feffff_equals_stock",
    "full_recovery_ff0000_ffffff_equals_capture",
    "full_execution_view_equals_stock_plus_monitor",
)


def required_check_names(recovery: bool, full_backup: bool) -> tuple[str, ...]:
    checks = COMMON_REQUIRED_CHECKS + (
        RECOVERY_REQUIRED_CHECKS if recovery else NORMAL_REQUIRED_CHECKS
    )
    if full_backup:
        checks += FULL_BACKUP_REQUIRED_CHECKS
    return checks


if os.name == "nt":
    from ctypes import wintypes

    class ScsiPassThroughDirect(ctypes.Structure):
        _fields_ = (
            ("Length", wintypes.USHORT),
            ("ScsiStatus", ctypes.c_ubyte),
            ("PathId", ctypes.c_ubyte),
            ("TargetId", ctypes.c_ubyte),
            ("Lun", ctypes.c_ubyte),
            ("CdbLength", ctypes.c_ubyte),
            ("SenseInfoLength", ctypes.c_ubyte),
            ("DataIn", ctypes.c_ubyte),
            ("DataTransferLength", wintypes.ULONG),
            ("TimeOutValue", wintypes.ULONG),
            ("DataBuffer", ctypes.c_void_p),
            ("SenseInfoOffset", wintypes.ULONG),
            ("Cdb", ctypes.c_ubyte * 16),
        )

    class ScsiPassThroughDirectWithSense(ctypes.Structure):
        _fields_ = (
            ("sptd", ScsiPassThroughDirect),
            ("sense", ctypes.c_ubyte * 32),
        )

    _KERNEL32 = ctypes.WinDLL("kernel32", use_last_error=True)
    _KERNEL32.CreateFileW.argtypes = (
        wintypes.LPCWSTR,
        wintypes.DWORD,
        wintypes.DWORD,
        ctypes.c_void_p,
        wintypes.DWORD,
        wintypes.DWORD,
        wintypes.HANDLE,
    )
    _KERNEL32.CreateFileW.restype = wintypes.HANDLE
    _KERNEL32.DeviceIoControl.argtypes = (
        wintypes.HANDLE,
        wintypes.DWORD,
        ctypes.c_void_p,
        wintypes.DWORD,
        ctypes.c_void_p,
        wintypes.DWORD,
        ctypes.POINTER(wintypes.DWORD),
        ctypes.c_void_p,
    )
    _KERNEL32.DeviceIoControl.restype = wintypes.BOOL
    _KERNEL32.CloseHandle.argtypes = (wintypes.HANDLE,)
    _KERNEL32.CloseHandle.restype = wintypes.BOOL

    _GENERIC_READ = 0x80000000
    _GENERIC_WRITE = 0x40000000
    _FILE_SHARE_READ = 0x00000001
    _FILE_SHARE_WRITE = 0x00000002
    _OPEN_EXISTING = 3
    _IOCTL_SCSI_PASS_THROUGH_DIRECT = 0x0004D014
    _SCSI_IOCTL_DATA_IN = 1
    _INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def captured_recovery_identity(monitor: bytes) -> dict[str, str]:
    template = monitor[
        RECOVERY_INQUIRY_TEMPLATE_OFFSET:
        RECOVERY_INQUIRY_TEMPLATE_OFFSET + len(RECOVERY_INQUIRY_TEMPLATE)
    ]
    if template != RECOVERY_INQUIRY_TEMPLATE:
        raise ValueError("runtime-monitor recovery INQUIRY template differs")
    return {
        "vendor": template[0:8].decode("ascii").strip(),
        "product": template[8:24].decode("ascii").strip(),
        "revision": template[24:28].decode("ascii").strip(),
    }


def identify(data: bytes, expected_revision: str = NORMAL_REVISION) -> dict[str, str]:
    if len(data) < 36:
        raise ValueError(f"short INQUIRY response: {len(data)} bytes")
    fields = {
        "vendor": data[8:16].decode("ascii", "replace").strip(),
        "product": data[16:32].decode("ascii", "replace").strip(),
        "revision": data[32:36].decode("ascii", "replace").strip(),
    }
    if (
        fields["vendor"] != "PLEXTOR"
        or "W5224A" not in fields["product"]
        or fields["revision"] != expected_revision
    ):
        raise ValueError(f"unexpected device identity: {fields}")
    return fields


def windows_device_path(device: str) -> str:
    if device.startswith("\\\\.\\"):
        return device
    if re.fullmatch(r"[A-Za-z]:\\?", device):
        return rf"\\.\{device[:2]}"
    raise ValueError("Windows device must be a drive letter such as H: or a \\\\.\\ device path")


def run_windows_data_in(device: str, length: int, cdb: tuple[int, ...]) -> tuple[bytes, int, bytes]:
    """Issue one fixed DATA-IN CDB through native Windows SPTI."""
    if not 0 < length <= 0xFFFFFFFF or not 1 <= len(cdb) <= 16:
        raise ValueError("invalid SCSI transfer shape")
    device_path = windows_device_path(device)
    handle = _KERNEL32.CreateFileW(
        device_path,
        _GENERIC_READ | _GENERIC_WRITE,
        _FILE_SHARE_READ | _FILE_SHARE_WRITE,
        None,
        _OPEN_EXISTING,
        0,
        None,
    )
    if handle == _INVALID_HANDLE_VALUE:
        error = ctypes.get_last_error()
        raise OSError(error, f"CreateFileW({device_path}) failed")
    try:
        data = (ctypes.c_ubyte * length)()
        packet = ScsiPassThroughDirectWithSense()
        packet.sptd.Length = ctypes.sizeof(ScsiPassThroughDirect)
        packet.sptd.CdbLength = len(cdb)
        packet.sptd.SenseInfoLength = len(packet.sense)
        packet.sptd.DataIn = _SCSI_IOCTL_DATA_IN
        packet.sptd.DataTransferLength = length
        packet.sptd.TimeOutValue = 10
        packet.sptd.DataBuffer = ctypes.cast(data, ctypes.c_void_p)
        packet.sptd.SenseInfoOffset = ScsiPassThroughDirectWithSense.sense.offset
        for index, value in enumerate(cdb):
            packet.sptd.Cdb[index] = value
        returned = wintypes.DWORD()
        success = _KERNEL32.DeviceIoControl(
            handle,
            _IOCTL_SCSI_PASS_THROUGH_DIRECT,
            ctypes.byref(packet),
            ctypes.sizeof(packet),
            ctypes.byref(packet),
            ctypes.sizeof(packet),
            ctypes.byref(returned),
            None,
        )
        if not success:
            error = ctypes.get_last_error()
            raise OSError(error, "IOCTL_SCSI_PASS_THROUGH_DIRECT failed")
        transferred = min(int(packet.sptd.DataTransferLength), length)
        sense = bytes(packet.sense[: packet.sptd.SenseInfoLength])
        return bytes(data[:transferred]), int(packet.sptd.ScsiStatus), sense
    finally:
        _KERNEL32.CloseHandle(handle)


def run_probe(sg_raw: str | None, device: str, output: Path, name: str, length: int,
              cdb: tuple[int, ...]) -> dict[str, object]:
    destination = output / f"{name}.bin"
    if (name, length, cdb) not in READ_ONLY_PROBES:
        raise ValueError("refusing a command outside the fixed read-only probe table")
    if os.name == "nt":
        command = ["IOCTL_SCSI_PASS_THROUGH_DIRECT", windows_device_path(device),
                   *(f"{byte:02x}" for byte in cdb)]
    else:
        assert sg_raw is not None
        command = [sg_raw, "--readonly", f"--request={length}",
                   f"--outfile={destination}", "--timeout=10", device,
                   *(f"{byte:02x}" for byte in cdb)]
    record: dict[str, object] = {
        "name": name,
        "direction": "DATA IN",
        "cdb": [f"{byte:02x}" for byte in cdb],
        "requested_length": length,
        "device": device,
        "command": command,
    }
    if os.name == "nt":
        try:
            payload, scsi_status, sense = run_windows_data_in(device, length, cdb)
            destination.write_bytes(payload)
            record["returncode"] = 0 if scsi_status == 0 else 1
            record["scsi_status"] = scsi_status
            record["sense_hex"] = sense.hex()
        except (OSError, ValueError) as error:
            record["returncode"] = 1
            record["error"] = str(error)
    else:
        try:
            result = subprocess.run(command, capture_output=True, text=True,
                                    timeout=15, check=False)
        except subprocess.TimeoutExpired as error:
            record["error"] = f"host timeout after {error.timeout} seconds"
            return record
        record["returncode"] = result.returncode
        record["stdout"] = result.stdout
        record["stderr"] = result.stderr
    if destination.exists():
        payload = destination.read_bytes()
        record["returned_length"] = len(payload)
        record["sha256"] = digest(payload)
        record["hex"] = payload.hex()
    return record


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--device", help="identified drive device, e.g. /dev/sgX")
    parser.add_argument("--output", type=Path, help="new directory for captures and report")
    parser.add_argument("--dry-run", action="store_true", help="print fixed probes without opening a device")
    parser.add_argument(
        "--recovery",
        action="store_true",
        help="require the protected monitor's revision 0.00 identity and use its fixed read-only probe set",
    )
    parser.add_argument(
        "--full-backup",
        action="store_true",
        help="also capture and compare the complete f0..f7 and f8..ff CPU data windows",
    )
    args = parser.parse_args()
    base_probes = RECOVERY_PROBES if args.recovery else PROBES
    selected_probes = base_probes + (FULL_BACKUP_PROBES if args.full_backup else ())
    expected_revision = RECOVERY_REVISION if args.recovery else NORMAL_REVISION
    if args.dry_run:
        print(json.dumps([{"name": name, "direction": "DATA IN", "request": length,
                           "cdb": " ".join(f"{byte:02x}" for byte in cdb)}
                          for name, length, cdb in selected_probes], indent=2))
        return 0
    if not args.device or args.output is None:
        parser.error("--device and --output are required unless --dry-run is used")
    sg_raw = shutil.which("sg_raw") if os.name != "nt" else None
    if os.name != "nt" and sg_raw is None:
        parser.error("sg_raw is required")
    if os.name != "nt" and not Path(args.device).exists():
        parser.error(f"device does not exist: {args.device}")
    if os.name == "nt":
        try:
            windows_device_path(args.device)
        except ValueError as error:
            parser.error(str(error))
    stock = Path(__file__).resolve().parents[1] / "data/W5224A104/MONT_104.bin"
    stock_bytes = stock.read_bytes()
    if digest(stock_bytes) != STOCK_SHA256:
        parser.error("local stock 1.04 reference hash differs")
    monitor = Path(__file__).resolve().parents[1] / "data/read_memory_ff0000.debug"
    monitor_bytes = monitor.read_bytes()
    if digest(monitor_bytes) != MONITOR_SHA256:
        parser.error("local runtime-monitor capture hash differs")
    try:
        recovery_identity = captured_recovery_identity(monitor_bytes)
    except ValueError as error:
        parser.error(str(error))
    expected = stock_bytes[0x50:0x60]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    report: dict[str, object] = {
        "schema_version": 1,
        "scope": "read-only CPU data-address observations; no flash writes or reset",
        "firmware_mode": "recovery" if args.recovery else "normal",
        "expected_revision": expected_revision,
        "stock_sha256": STOCK_SHA256,
        "monitor_sha256": MONITOR_SHA256,
        "captured_recovery_identity": recovery_identity,
        "expected_f80050_hex": expected.hex(),
        "expected_ff0000_hex": monitor_bytes[0:16].hex(),
        "expected_ff2a3e_hex": monitor_bytes[0x2A3E:0x2A4E].hex(),
        "expected_ff8000_hex": stock_bytes[0x78000:0x78010].hex(),
        "script_sha256": digest(Path(__file__).read_bytes()),
        "probes": [],
    }
    try:
        for name, length, cdb in selected_probes:
            item = run_probe(sg_raw, args.device, output, name, length, cdb)
            report["probes"].append(item)
            if item.get("returncode") != 0:
                raise ValueError(f"{name}: command failed or timed out")
            payload = bytes.fromhex(str(item.get("hex", "")))
            if name == "inquiry":
                report["identity"] = identify(payload, expected_revision)
            elif len(payload) != length:
                raise ValueError(f"{name}: returned {len(payload)} of {length} bytes")
        by_name = {str(item["name"]): bytes.fromhex(str(item["hex"]))
                   for item in report["probes"]}
        stock_reference_valid = by_name["image_f80050_f5"] == expected
        report["checks"] = {
            "expected_inquiry_revision": report["identity"]["revision"] == expected_revision,
            "f80050_equals_stock": stock_reference_valid,
            "f00050_equals_f80050": by_name["alias_f00050_f5"] == by_name["image_f80050_f5"],
            "cpu_data_read_alias_supported": stock_reference_valid and by_name["alias_f00050_f5"] == by_name["image_f80050_f5"],
            "ff0000_equals_runtime_capture": by_name["data_ff0000_f5"] == monitor_bytes[0:16],
            "f70000_equals_ff0000": by_name["alias_f70000_f5"] == by_name["data_ff0000_f5"],
            "ff2a3e_equals_runtime_capture": by_name["data_ff2a3e_f5"] == monitor_bytes[0x2A3E:0x2A4E],
            "f72a3e_equals_ff2a3e": by_name["alias_f72a3e_f5"] == by_name["data_ff2a3e_f5"],
            "ff8000_equals_stock_fixed_data": by_name["data_ff8000_f5"] == stock_bytes[0x78000:0x78010],
            "f78000_equals_ff8000": by_name["alias_f78000_f5"] == by_name["data_ff8000_f5"],
        }
        if args.recovery:
            report["checks"]["recovery_monitor_identity_supported"] = (
                report["identity"]["revision"] == RECOVERY_REVISION
                and by_name["data_ff0000_f5"] == monitor_bytes[0:16]
                and by_name["data_ff2a3e_f5"] == monitor_bytes[0x2A3E:0x2A4E]
            )
        else:
            mode_sense = by_name["mode_page03_sense10"]
            page_header_valid = mode_sense[8:10] == bytes((3, 6))
            report["checks"].update({
                "mode_sense_page_header_03_06": page_header_valid,
                "current_page_f5_equals_mode_sense": by_name["mode_page03_f5"] == mode_sense[8:16],
                "current_page_read_supported": page_header_valid and by_name["mode_page03_f5"] == mode_sense[8:16],
                "flash_id_selector_ram_f8_hex": by_name["flash_id_selector_f5"].hex(),
                "flash_id_selector_recognized": by_name["flash_id_selector_f5"] in (bytes.fromhex("bf00"), bytes.fromhex("6200"), bytes.fromhex("2000")),
            })
        if args.full_backup:
            programming_view = b"".join(
                by_name[f"backup_f00000_{offset:05x}"]
                for offset in range(0, 0x80000, 0x8000)
            )
            execution_view = b"".join(
                by_name[f"backup_f80000_{offset:05x}"]
                for offset in range(0, 0x80000, 0x8000)
            )
            expected_execution_view = bytearray(stock_bytes)
            expected_execution_view[0x70000:0x80000] = monitor_bytes
            programming_path = output / "cpu_data_f00000_f7ffff.bin"
            execution_path = output / "cpu_data_f80000_ffffff.bin"
            programming_path.write_bytes(programming_view)
            execution_path.write_bytes(execution_view)
            report["full_backup"] = {
                "programming_view": {
                    "path": str(programming_path),
                    "length": len(programming_view),
                    "sha256": digest(programming_view),
                },
                "execution_view": {
                    "path": str(execution_path),
                    "length": len(execution_view),
                    "sha256": digest(execution_view),
                },
            }
            report["checks"].update({
                "full_f00000_f7ffff_equals_f80000_ffffff": programming_view == execution_view,
                "full_main_f80000_feffff_equals_stock": execution_view[:0x70000] == stock_bytes[:0x70000],
                "full_recovery_ff0000_ffffff_equals_capture": execution_view[0x70000:0x80000] == monitor_bytes,
                "full_execution_view_equals_stock_plus_monitor": execution_view == expected_execution_view,
            })
        required_checks = required_check_names(args.recovery, args.full_backup)
        report["required_checks"] = list(required_checks)
        failed_checks = [
            name for name in required_checks if report["checks"].get(name) is not True
        ]
        report["pass"] = not failed_checks
        if failed_checks:
            raise ValueError("required read-only checks failed: " + ", ".join(failed_checks))
    except ValueError as error:
        report["pass"] = False
        report["error"] = str(error)
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(output / "report.json")
    return 1 if "error" in report else 0


if __name__ == "__main__":
    raise SystemExit(main())
