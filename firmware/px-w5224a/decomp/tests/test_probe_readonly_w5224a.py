import ctypes
import os
import unittest

from tools import probe_readonly_w5224a as probe


class ReadOnlyW5224AProbeTests(unittest.TestCase):
    def test_recovery_identity_is_bound_to_immutable_monitor_capture(self):
        monitor = (probe.Path(__file__).resolve().parents[1] / "data/read_memory_ff0000.debug").read_bytes()
        self.assertEqual(
            probe.captured_recovery_identity(monitor),
            {"vendor": "PLEXTOR", "product": "CD-R   PX-W5224A", "revision": "0.00"},
        )
        damaged = bytearray(monitor)
        damaged[probe.RECOVERY_INQUIRY_TEMPLATE_OFFSET] ^= 1
        with self.assertRaisesRegex(ValueError, "template differs"):
            probe.captured_recovery_identity(bytes(damaged))

    def test_identify_accepts_expected_drive(self):
        response = bytearray(96)
        response[8:16] = b"PLEXTOR "
        response[16:32] = b"CD-R   PX-W5224A"
        response[32:36] = b"1.04"
        self.assertEqual(
            probe.identify(bytes(response)),
            {"vendor": "PLEXTOR", "product": "CD-R   PX-W5224A", "revision": "1.04"},
        )

    def test_identify_rejects_other_device(self):
        response = bytearray(96)
        response[8:16] = b"PLEXTOR "
        response[16:32] = b"DVDR   PX-755A "
        response[32:36] = b"1.08"
        with self.assertRaisesRegex(ValueError, "unexpected device identity"):
            probe.identify(bytes(response))

    def test_identify_accepts_recovery_monitor_only_when_requested(self):
        response = bytearray(96)
        response[8:16] = b"PLEXTOR "
        response[16:32] = b"CD-R   PX-W5224A"
        response[32:36] = b"0.00"
        with self.assertRaisesRegex(ValueError, "unexpected device identity"):
            probe.identify(bytes(response))
        self.assertEqual(
            probe.identify(bytes(response), probe.RECOVERY_REVISION),
            {"vendor": "PLEXTOR", "product": "CD-R   PX-W5224A", "revision": "0.00"},
        )

    def test_windows_device_path_accepts_drive_letter_and_device_path(self):
        self.assertEqual(probe.windows_device_path("H:"), r"\\.\H:")
        self.assertEqual(probe.windows_device_path(r"\\.\H:"), r"\\.\H:")

    @unittest.skipUnless(os.name == "nt", "Windows SPTI layout")
    def test_windows_scsi_pass_through_layout_matches_ntddscsi(self):
        self.assertEqual(ctypes.sizeof(probe.ScsiPassThroughDirect), 56)
        self.assertEqual(probe.ScsiPassThroughDirect.DataBuffer.offset, 24)
        self.assertEqual(probe.ScsiPassThroughDirect.SenseInfoOffset.offset, 32)
        self.assertEqual(probe.ScsiPassThroughDirect.Cdb.offset, 36)
        self.assertEqual(probe.ScsiPassThroughDirectWithSense.sense.offset, 56)
        self.assertEqual(probe._IOCTL_SCSI_PASS_THROUGH_DIRECT, 0x0004D014)

    def test_probe_table_has_data_in_commands_only(self):
        self.assertEqual(probe.PROBES[0][2][0], 0x12)
        self.assertEqual(probe.PROBES[2][2][0], 0x5A)
        self.assertTrue(all(cdb[0] in (0x12, 0x5A, 0xF5) for _, _, cdb in probe.PROBES))
        self.assertTrue(all(length > 0 for _, length, _ in probe.PROBES))
        self.assertEqual({name for name, _, _ in probe.RECOVERY_PROBES}, probe.RECOVERY_PROBE_NAMES)
        self.assertTrue(all(cdb[0] in (0x12, 0xF5) for _, _, cdb in probe.RECOVERY_PROBES))

    def test_full_backup_probe_table_covers_both_windows(self):
        self.assertEqual(len(probe.FULL_BACKUP_PROBES), 32)
        addresses = [
            (cdb[5] << 16) | (cdb[6] << 8) | cdb[7]
            for _, length, cdb in probe.FULL_BACKUP_PROBES
            if length == 0x8000 and cdb[0] == 0xF5
        ]
        self.assertEqual(
            addresses,
            [
                base + offset
                for base in (0xF00000, 0xF80000)
                for offset in range(0, 0x80000, 0x8000)
            ],
        )

    def test_required_checks_are_mode_specific(self):
        normal = probe.required_check_names(False, False)
        recovery = probe.required_check_names(True, False)
        recovery_full = probe.required_check_names(True, True)
        self.assertIn("current_page_read_supported", normal)
        self.assertIn("flash_id_selector_recognized", normal)
        self.assertNotIn("recovery_monitor_identity_supported", normal)
        self.assertIn("recovery_monitor_identity_supported", recovery)
        self.assertNotIn("current_page_read_supported", recovery)
        self.assertTrue(set(probe.FULL_BACKUP_REQUIRED_CHECKS).issubset(recovery_full))


if __name__ == "__main__":
    unittest.main()
