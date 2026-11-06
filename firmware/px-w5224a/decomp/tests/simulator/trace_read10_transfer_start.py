"""Trace READ(10) from its media wait through a declared sector transfer start.

The source case yields to a captured scheduler context. After the real media
coroutine yields, the scheduler naturally resumes the command. At the batch
entry this trace supplies one synthetic, status-zero descriptor at the ring
base already selected by both images. It verifies transfer setup and the next
command wait. It does not supply sector bytes or a completion interrupt.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path
from tests.simulator.trace_read10_media_first_yield import CASE_ID, prepare

CHECKPOINTS = {
    "stock": {
        "resume2": 0xFDCF87, "media": 0xF90000, "yield2": 0xFEE53F,
        "return": 0xFDCF8B, "resume0": 0xFDCF7C, "batch": 0xFC5226,
        "transfer": 0xFC549F, "start": 0xFDE9A2, "command_yield": 0xFEE529,
    },
    "reconstructed": {
        "resume2": 0xFDCF6A, "media": 0xFDE81B, "yield2": 0xFEAA94,
        "return": 0xFDCF6E, "resume0": 0xFDCF59, "batch": 0xFA6323,
        "transfer": 0xFB1E57, "start": 0xF8C742, "command_yield": 0xFEAA7E,
    },
}


def run_side(source: Path, output: Path, points: dict[str, int], uv4: Path, timeout: int) -> dict[str, object]:
    project = prepare(source, output, points["resume2"], points["media"], points["yield2"])
    ini = output / "run.ini"
    content = ini.read_text()
    first = wine_path(output / "read10-media-first-yield.hex").replace("\\", "\\\\")
    anchor = f'SAVE "{first}" 0x016000,0x01603f\n'
    if content.count(anchor) != 1:
        raise SimulatorError(f"{ini}: media-yield continuation point changed")
    trace = wine_path(output / "read10-transfer-start.hex").replace("\\", "\\\\")
    lines = [
        anchor.rstrip("\n"),
        "BK *", f"BS C:0x{points['return']:06x}", "G", "_WDWORD(0x016040, $)",
        "BK *", f"BS C:0x{points['resume0']:06x}", "G", "_WDWORD(0x016050, $)",
        "BK *", f"BS C:0x{points['batch']:06x}", "G", "_WDWORD(0x016060, $)",
        "_WDWORD(0x016090, _RDWORD(E:0x001292))",
        "_WDWORD(0x016094, _RDWORD(E:0x001294))",
        "_WDWORD(0x016098, _RDWORD(E:0x001298))",
        "_WDWORD(0x01609c, _RDWORD(E:0x00129e))",
        "_WDWORD(0x0160a0, _RDWORD(E:0x00145d))",
        "_WBYTE(0x0160a8, _RBYTE(E:0x000021))",
        # The ring has one available descriptor at the existing RAM:88bb base.
        "_WBYTE(E:0x001292, 0)", "_WBYTE(E:0x001293, 1)",
        "_WBYTE(E:0x001296, 0)", "_WBYTE(E:0x001297, 1)",
    ]
    lines += [f"_WBYTE(E:0x{0x88BB + i:06x}, 0)" for i in range(0x16)]
    lines += [
        # Descriptor +4 holds the declared controller source address 0x002000.
        # The command's descriptor mapping bit is clear at this checkpoint.
        "_WBYTE(E:0x0088c1, 0x20)",
        "BK *", f"BS C:0x{points['transfer']:06x}", "G",
        "_WDWORD(0x016070, $)",
        "_WDWORD(0x016074, _RDWORD(E:0x001292))",
        "_WDWORD(0x016078, _RDWORD(E:0x0088bf))",
        "BK *", f"BS C:0x{points['start']:06x}", "G",
        "_WDWORD(0x016080, $)",
        "_WDWORD(0x016084, _RDWORD(E:0x001459))",
        "_WBYTE(0x016088, _RBYTE(E:0x001462))",
        "_WBYTE(0x016089, _RBYTE(X:0x8020))",
        "_WBYTE(0x01608a, _RBYTE(X:0x800d))",
        "_WBYTE(0x01608b, _RBYTE(X:0x800e))",
        "_WBYTE(0x01608c, _RBYTE(X:0x8010))",
        "_WBYTE(0x01608d, _RBYTE(X:0x8011))",
        "_WBYTE(0x01608e, _RBYTE(X:0x8012))",
        "BK *", f"BS C:0x{points['command_yield']:06x}", "G",
        "_WDWORD(0x0160b0, $)",
        "_WDWORD(0x0160b4, _RDWORD(E:0x001459))",
        "_WBYTE(0x0160b8, _RBYTE(E:0x001462))",
        "_WBYTE(0x0160b9, _RBYTE(X:0x8056))",
        "_WBYTE(0x0160ba, _RBYTE(X:0x8022))",
        "_WBYTE(0x0160bb, _RBYTE(X:0x805e))",
        "_WBYTE(0x0160bc, _RBYTE(X:0x8051))",
        f'SAVE "{trace}" 0x016000,0x0160bf',
    ]
    ini.write_text(content.replace(anchor, "\n".join(lines) + "\n", 1))
    run_uv4(uv4, project, output / "uv4-transfer-start.log", timeout)
    memory = read_intel_hex(output / "read10-transfer-start.hex")

    def dword(address: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(4)), "big")

    result = {
        "scheduler_return_pc": dword(0x16040) & 0xFFFFFF,
        "resume0_pc": dword(0x16050) & 0xFFFFFF,
        "batch_pc": dword(0x16060) & 0xFFFFFF,
        "transfer_pc": dword(0x16070) & 0xFFFFFF,
        "start_pc": dword(0x16080) & 0xFFFFFF,
        "command_yield_pc": dword(0x160B0) & 0xFFFFFF,
        "queue_before": dword(0x16090),
        "cursors_before": dword(0x16094),
        "secondary_and_mask_before": dword(0x16098),
        "descriptor_base": dword(0x1609C),
        "block_size_and_threshold": dword(0x160A0),
        "mapping_bit_byte_before": memory[0x160A8],
        "queue_at_transfer": dword(0x16074),
        "descriptor_payload_address": dword(0x16078),
        "issued_blocks_at_start": dword(0x16084),
        "transfer_latch_at_start": memory[0x16088],
        "controller_command_at_start": memory[0x16089],
        "controller_length_at_start": [memory[0x1608A], memory[0x1608B]],
        "controller_address_at_start": [memory[0x1608C], memory[0x1608D], memory[0x1608E]],
        "issued_blocks_at_yield": dword(0x160B4),
        "transfer_latch_at_yield": memory[0x160B8],
        "controller_trigger_at_yield": memory[0x160B9],
        "controller_control_at_yield": memory[0x160BA],
        "controller_status_at_yield": memory[0x160BB],
        "controller_argument_at_yield": memory[0x160BC],
    }
    for field, point in (
        ("scheduler_return_pc", "return"), ("resume0_pc", "resume0"),
        ("batch_pc", "batch"), ("transfer_pc", "transfer"),
        ("start_pc", "start"), ("command_yield_pc", "command_yield"),
    ):
        if result[field] != points[point]:
            raise SimulatorError(f"{output.name}: missing {field}: {result}")
    expected = {
        "queue_before": 0, "cursors_before": 0,
        "secondary_and_mask_before": 0x01FF,
        "descriptor_base": 0x88BB, "block_size_and_threshold": 0x08000000,
        "mapping_bit_byte_before": 1, "queue_at_transfer": 0x00010000,
        "descriptor_payload_address": 0x00002000,
        "issued_blocks_at_start": 1, "transfer_latch_at_start": 1,
        "controller_command_at_start": 0x88,
        "controller_length_at_start": [0, 8],
        "controller_address_at_start": [0x10, 0x20, 0],
        "issued_blocks_at_yield": 1, "transfer_latch_at_yield": 1,
        "controller_trigger_at_yield": 0x50,
        "controller_control_at_yield": 0x80,
        "controller_status_at_yield": 0x97,
        "controller_argument_at_yield": 2,
    }
    for field, value in expected.items():
        if result[field] != value:
            raise SimulatorError(f"{output.name}: {field} differs: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source = args.source_run.resolve()
    source_report = json.loads((source / "results.json").read_text())
    matches = [item for item in source_report["results"] if item["id"] == CASE_ID]
    if len(matches) != 1 or matches[0]["status"] != "pass":
        raise SimulatorError("source run needs the passing READ(10) scheduler-yield case")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {
        side: run_side(source / CASE_ID / side, output / side, points, args.uv4.resolve(), args.timeout)
        for side, points in CHECKPOINTS.items()
    }
    for field in observations["stock"]:
        if field.endswith("_pc"):
            continue
        if observations["stock"][field] != observations["reconstructed"][field]:
            raise SimulatorError(f"READ(10) transfer setup differs: {field}")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "Offline READ(10) command/media wait cycle; one declared synthetic descriptor at the observed empty ring base; matching 2048-byte controller transfer setup, trigger and next command yield. No sector bytes, completion interrupt or command completion tested.",
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched READ(10) declared-descriptor transfer setup and wait yield")


if __name__ == "__main__":
    main()
