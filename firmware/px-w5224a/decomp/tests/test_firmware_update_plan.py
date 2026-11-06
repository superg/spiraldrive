import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

from tools import firmware_image
from tools import firmware_update_plan


class FirmwareUpdatePlanTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.stock = firmware_update_plan.STOCK.read_bytes()

    def test_stock_contract_builds_31_downloads_and_one_activation(self):
        plan = firmware_update_plan.build_plan(self.stock, image_path="stock.bin")
        transfers = plan["transfers"]
        self.assertEqual(len(transfers), 32)
        self.assertEqual([item["mode"] for item in transfers[:-1]], [4] * 31)
        self.assertEqual(transfers[-1]["mode"], 5)
        self.assertEqual(transfers[0]["offset"], 0)
        self.assertEqual(transfers[-1]["offset"], 0x7C000)
        self.assertTrue(transfers[-1]["activates_after_command"])
        self.assertEqual(transfers[-1]["device_timeout_seconds"], 600)
        recovery = plan["recovery_compatibility_evidence"]
        self.assertEqual(recovery["monitor_inquiry_revision"], "0.00")
        self.assertEqual(recovery["drive_zero_prefix_test_va"], "0x00405de7")
        self.assertEqual(recovery["accepted_successor_va"], "0x00405ead")
        self.assertIn("positive revision-0.00 read-only preflight", recovery["limitation"])
        mapping = plan["flash_bus_mapping_evidence"]
        self.assertEqual(mapping["manufacturer_id_datasheet_word"], "0x00bf")
        self.assertEqual(mapping["live_cpu_selector_word"], "0xbf00")
        self.assertIn("operation 3", mapping["scope"])
        ranges = plan["device_programmer_range_contract"]
        programmed = ranges["programmed_image_ranges"]
        preserved = ranges["preserved_image_ranges"]
        self.assertEqual(ranges["runtime_operation"], 3)
        self.assertEqual(
            [(item["image_offset"], item["length"]) for item in programmed],
            [(0x00000, 0x70000), (0x78000, 0x01FFE), (0x79FFE, 0x00002)],
        )
        self.assertEqual(
            [(item["image_offset"], item["length"]) for item in preserved],
            [(0x70000, 0x08000), (0x7A000, 0x06000)],
        )
        self.assertEqual(
            sum(item["length"] for item in programmed + preserved),
            firmware_image.SIZE,
        )
        self.assertTrue(all(item["candidate_equals_stock"] for item in preserved))
        self.assertTrue(programmed[1]["candidate_equals_stock"])
        self.assertEqual(
            sum(item["length"] for item in transfers), firmware_image.SIZE
        )

    def test_write_buffer_cdb_uses_standard_three_byte_fields(self):
        self.assertEqual(
            firmware_update_plan.write_buffer_cdb(4, 0, 0x012345, 0x004000),
            bytes.fromhex("3b 04 00 01 23 45 00 40 00 00"),
        )
        self.assertEqual(
            firmware_update_plan.write_buffer_cdb(5, 0, 0x07C000, 0x004000),
            bytes.fromhex("3b 05 00 07 c0 00 00 40 00 00"),
        )

    def test_hash_pinned_updater_contains_recovery_zero_prefix_gate(self):
        updater = firmware_update_plan.UPDATER.read_bytes()
        firmware_update_plan.validate_updater_recovery_version_gate(updater)
        self.assertEqual(
            firmware_update_plan.pe32_va_slice(updater, 0x00405EC2, 5),
            bytes.fromhex("b8 01 00 00 00"),
        )
        damaged = bytearray(updater)
        pe_offset = int.from_bytes(damaged[0x3C:0x40], "little")
        optional_size = int.from_bytes(damaged[pe_offset + 20:pe_offset + 22], "little")
        section_offset = pe_offset + 24 + optional_size
        text_rva = int.from_bytes(damaged[section_offset + 12:section_offset + 16], "little")
        text_raw = int.from_bytes(damaged[section_offset + 20:section_offset + 24], "little")
        damaged[text_raw + (0x405DD7 - 0x400000 - text_rva)] ^= 1
        with self.assertRaisesRegex(
            firmware_update_plan.UpdatePlanError, "candidate zero-prefix gate differs"
        ):
            firmware_update_plan.validate_updater_recovery_version_gate(bytes(damaged))

    def test_container_validation_precedes_planning(self):
        damaged = bytearray(self.stock)
        damaged[0x50] ^= 1
        with self.assertRaisesRegex(firmware_image.ImageError, "checksum mismatch"):
            firmware_update_plan.build_plan(bytes(damaged), image_path="damaged.bin")

    def test_full_backup_is_cryptographically_bound_to_plan(self):
        monitor = (
            firmware_update_plan.ROOT / "data/read_memory_ff0000.debug"
        ).read_bytes()
        capture = self.stock[:0x70000] + monitor
        capture_hash = hashlib.sha256(capture).hexdigest()
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            programming = root / "programming.bin"
            execution = root / "execution.bin"
            programming.write_bytes(capture)
            execution.write_bytes(capture)
            report = {
                "schema_version": 1,
                "stock_sha256": firmware_image.STOCK_SHA256,
                "monitor_sha256": (
                    "7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80"
                ),
                "identity": {
                    "vendor": "PLEXTOR",
                    "product": "CD-R   PX-W5224A",
                    "revision": "1.04",
                },
                "checks": {
                    "flash_id_selector_recognized": True,
                    "flash_id_selector_ram_f8_hex": "bf00",
                    "full_f00000_f7ffff_equals_f80000_ffffff": True,
                    "full_main_f80000_feffff_equals_stock": True,
                    "full_recovery_ff0000_ffffff_equals_capture": True,
                    "full_execution_view_equals_stock_plus_monitor": True,
                },
                "full_backup": {
                    "programming_view": {
                        "path": str(programming),
                        "length": len(capture),
                        "sha256": capture_hash,
                    },
                    "execution_view": {
                        "path": str(execution),
                        "length": len(capture),
                        "sha256": capture_hash,
                    },
                },
            }
            report_path = root / "report.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            plan = firmware_update_plan.build_plan(
                self.stock,
                image_path="stock.bin",
                backup_report=report_path,
            )
            self.assertTrue(plan["live_stock_backup"]["all_required_checks_pass"])

            recovery_report = copy.deepcopy(report)
            recovery_identity = {
                "vendor": "PLEXTOR",
                "product": "CD-R   PX-W5224A",
                "revision": "0.00",
            }
            recovery_report.update({
                "firmware_mode": "recovery",
                "expected_revision": "0.00",
                "captured_recovery_identity": recovery_identity,
                "identity": recovery_identity,
                "script_sha256": firmware_update_plan.digest(
                    firmware_update_plan.RECOVERY_PROBE_SCRIPT.read_bytes()
                ),
                "pass": True,
                "probes": [{"direction": "DATA IN"} for _ in range(41)],
            })
            recovery_checks = (
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
            recovery_report["checks"] = {name: True for name in recovery_checks}
            recovery_report["required_checks"] = list(recovery_checks)
            recovery_path = root / "recovery-report.json"
            recovery_path.write_text(json.dumps(recovery_report), encoding="utf-8")
            recovery_plan = firmware_update_plan.build_plan(
                self.stock,
                image_path="stock.bin",
                recovery_report=recovery_path,
            )
            self.assertEqual(recovery_plan["live_recovery_backup"]["probe_count"], 41)
            self.assertTrue(
                recovery_plan["live_recovery_backup"]["all_required_checks_pass"]
            )
            self.assertIn(
                "proves live recovery enumeration",
                recovery_plan["recovery_compatibility_evidence"]["limitation"],
            )
            recovery_report["identity"] = {
                "vendor": "PLEXTOR",
                "product": "CD-R   PX-W5224A",
                "revision": "1.04",
            }
            recovery_path.write_text(json.dumps(recovery_report), encoding="utf-8")
            with self.assertRaisesRegex(
                firmware_update_plan.UpdatePlanError,
                "does not identify the protected monitor",
            ):
                firmware_update_plan.validate_recovery_report(recovery_path)

            execution.write_bytes(capture[:-1] + bytes((capture[-1] ^ 1,)))
            with self.assertRaisesRegex(
                firmware_update_plan.UpdatePlanError, "hash metadata differs"
            ):
                firmware_update_plan.validate_backup_report(report_path)


if __name__ == "__main__":
    unittest.main()
