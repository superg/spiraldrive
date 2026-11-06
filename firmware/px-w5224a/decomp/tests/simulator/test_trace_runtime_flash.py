"""Regression checks for the runtime flash-stop record format."""

from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tests.simulator import trace_runtime_flash as trace


class RuntimeFlashTraceTests(TestCase):
    def test_state_counter_and_stack_pointer_have_separate_fields(self) -> None:
        base = trace.RECORD_BASE
        memory: dict[int, int] = {}

        def put(address: int, value: int, width: int) -> None:
            for offset, byte in enumerate(value.to_bytes(width, "big")):
                memory[address + offset] = byte

        put(base, 0, 4)
        put(base + 4, 0x00F0AAAA, 4)
        put(base + 8, 0x00F05554, 4)
        put(base + 12, 0x00000800, 4)
        put(base + 16, 0xAA00, 2)
        put(base + 20, 0x12345678, 4)
        put(base + 24, 0x1D, 1)
        put(base + 28, 0xFF00082B, 4)

        with patch.object(trace, "read_intel_hex", return_value=memory):
            records = trace.decode_trace(Path("unused.hex"), ((0x82B, 0xF0AAAA, 0xAA00),), ("DR16",))

        self.assertEqual(records[0]["states"], 0x12345678)
        self.assertEqual(records[0]["sp"], 0x1D)
        self.assertEqual(records[0]["pc_samples"], [0, 0x82B])
        self.assertEqual(trace.RECORD_SIZE, 32)
        script = trace.trace_lines(Path("unused.hex"), ((0x82B, 0xF0AAAA, 0xAA00),))
        self.assertIn("_WBYTE(0x017c18, SP)", script)
        self.assertIn("_WDWORD(0x017c1c, $)", script)

        put(base + 28, 0, 4)
        with patch.object(trace, "read_intel_hex", return_value=memory):
            with self.assertRaises(trace.SimulatorError):
                trace.decode_trace(Path("unused.hex"), ((0x82B, 0xF0AAAA, 0xAA00),), ("DR16",))
