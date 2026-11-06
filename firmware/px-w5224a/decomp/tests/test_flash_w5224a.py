import copy
import json
from pathlib import Path
import tempfile
import unittest

from tools import firmware_update_plan
from tools import flash_w5224a


class FlashW5224ATests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.image = firmware_update_plan.STOCK.read_bytes()
        cls.plan = firmware_update_plan.build_plan(cls.image, image_path="stock.bin")

    def test_transfer_validation_covers_exact_candidate(self):
        flash_w5224a.validate_transfer_set(self.plan, self.image)
        first = self.plan["transfers"][0]
        cdb, payload = flash_w5224a.transfer_payload(self.image, first)
        self.assertEqual(cdb, bytes.fromhex("3b 04 00 00 00 00 00 40 00 00"))
        self.assertEqual(payload, self.image[:0x4000])

    def test_transfer_validation_rejects_stale_payload_hash(self):
        plan = copy.deepcopy(self.plan)
        plan["transfers"][3]["payload_sha256"] = "0" * 64
        with self.assertRaisesRegex(flash_w5224a.FlashExecutionError, "payload hash"):
            flash_w5224a.validate_transfer_set(plan, self.image)

    def test_executor_sends_once_in_order_and_accepts_all(self):
        calls = []
        report = {"state": "identity_confirmed", "transfers": []}

        def send(cdb, payload, timeout):
            calls.append((cdb, payload, timeout))
            return 0, bytes(32), len(payload)

        flash_w5224a.run_transfers(self.plan, self.image, send, report, lambda: None)
        self.assertEqual(len(calls), 32)
        self.assertEqual(calls[0][0][1], 4)
        self.assertEqual(calls[-1][0][1], 5)
        self.assertEqual(calls[-1][2], 600)
        self.assertEqual(report["state"], "activation_command_accepted")
        self.assertTrue(all(item["outcome"] == "accepted" for item in report["transfers"]))

    def test_staging_failure_stops_before_activation_without_retry(self):
        calls = []
        report = {"state": "identity_confirmed", "transfers": []}

        def send(cdb, payload, timeout):
            calls.append(cdb)
            return (2, bytes.fromhex("700005000000000a000000002000") + bytes(18), len(payload))

        with self.assertRaisesRegex(flash_w5224a.FlashExecutionError, "staging transfer 0"):
            flash_w5224a.run_transfers(self.plan, self.image, send, report, lambda: None)
        self.assertEqual(len(calls), 1)
        self.assertEqual(report["state"], "failed")

    def test_final_exception_is_indeterminate_and_never_retried(self):
        calls = []
        report = {"state": "identity_confirmed", "transfers": []}

        def send(cdb, payload, timeout):
            calls.append(cdb)
            if cdb[1] == 5:
                raise OSError("device reset")
            return 0, bytes(32), len(payload)

        with self.assertRaisesRegex(flash_w5224a.FlashExecutionError, "do not retry"):
            flash_w5224a.run_transfers(self.plan, self.image, send, report, lambda: None)
        self.assertEqual(len(calls), 32)
        self.assertEqual(sum(cdb[1] == 5 for cdb in calls), 1)
        self.assertEqual(report["state"], "indeterminate_after_activation_submission")

    def test_load_bound_plan_rejects_modified_plan(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            image_path = root / "stock.bin"
            image_path.write_bytes(self.image)
            # This test only needs to establish that exact equality is enforced;
            # the report validators are replaced with known records.
            original_backup = firmware_update_plan.validate_backup_report
            original_recovery = firmware_update_plan.validate_recovery_report
            firmware_update_plan.validate_backup_report = lambda path: {"ok": "stock"}
            firmware_update_plan.validate_recovery_report = lambda path: {"ok": "recovery"}
            try:
                plan = firmware_update_plan.build_plan(
                    self.image,
                    image_path=str(image_path.resolve()),
                    backup_report=root / "backup.json",
                    recovery_report=root / "recovery.json",
                )
                plan["transfer_count"] = 31
                plan_path = root / "plan.json"
                plan_path.write_text(json.dumps(plan), encoding="utf-8")
                with self.assertRaisesRegex(flash_w5224a.FlashExecutionError, "stale"):
                    flash_w5224a.load_bound_plan(
                        plan_path,
                        image_path,
                        root / "backup.json",
                        root / "recovery.json",
                    )
            finally:
                firmware_update_plan.validate_backup_report = original_backup
                firmware_update_plan.validate_recovery_report = original_recovery


if __name__ == "__main__":
    unittest.main()
