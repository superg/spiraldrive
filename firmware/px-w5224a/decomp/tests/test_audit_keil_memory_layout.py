import tempfile
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))

from audit_keil_memory_layout import audit


class KeilMemoryLayoutAuditTests(unittest.TestCase):
    def test_physical_layout_and_deficit(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            map_path = root / "main.map"
            map_path.write_text(
                "FE0000H FEFF00H FEFFFFH 000031H CODE\n"
                "000000H F80050H FEB4FFH 07D4ABH ECODE\n"
                "000000H FEB500H FEFEFFH 00684FH HCONST\n"
                "010000H 010000H 017FFFH 001C63H XDATA\n"
                "Program Size: data=102.0 edata+hdata=0 xdata=7267 "
                "const=26703 code=513244\n")
            log_path = root / "link.log"
            log_path.write_text(
                "*** ERROR L107: ADDRESS SPACE OVERFLOW\n"
                "*** ERROR L107: ADDRESS SPACE OVERFLOW\n"
                "*** WARNING L30: MEMORY SPACE OVERLAP\n")

            report = audit(map_path, log_path)

            self.assertTrue(report.valid_physical_layout)
            self.assertEqual(report.main_payload_capacity, 0x6FFB0)
            self.assertEqual(report.main_payload_used, 0x83D2B)
            self.assertEqual(report.main_payload_deficit, 0x13D7B)
            self.assertEqual(report.linker_diagnostics,
                             {"error_L107": 2, "warning_L30": 1})

    def test_wrong_range_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            map_path = Path(temporary) / "main.map"
            map_path.write_text(
                "FE0000H FEFF00H FEFFFFH 000031H CODE\n"
                "000000H F80050H FE8FFFH 000001H ECODE\n"
                "000000H FEB500H FEFEFFH 000001H HCONST\n"
                "010000H 010000H 017FFFH 000001H XDATA\n")

            report = audit(map_path)

            self.assertFalse(report.valid_physical_layout)
            self.assertTrue(any("ECODE range" in error
                                for error in report.layout_errors))


if __name__ == "__main__":
    unittest.main()
