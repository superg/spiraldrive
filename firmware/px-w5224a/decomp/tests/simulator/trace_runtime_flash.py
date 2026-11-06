"""Replay the captured runtime programmer's operation-5 erase-store stops.

This uses S251's stop after each write to its mapped flash window. The trace
proves CPU stores and register values under declared inputs; it does not prove
that an SST chip accepted an erase or that the CPU address is a flash alias.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.flash_bus import FlashBusError, Sst39Lf400aBus, image_checksum
from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path


CASE_ID_2000 = "runtime-operation5-first-unlock-checkpoint"
CASE_ID_BF00 = "runtime-operation5-bf00-first-unlock-checkpoint"
RECORD_BASE = 0x017C00
RECORD_SIZE = 32
STORES_2000 = (
    (0x00082B, 0xF0AAAA, 0xAA00),
    (0x000832, 0xF05554, 0x5500),
    (0x000839, 0xF0AAAA, 0x8000),
    (0x000840, 0xF0AAAA, 0xAA00),
    (0x000847, 0xF05554, 0x5500),
    (0x000856, 0xF00000, 0x3000),
    (0x000861, 0xF10000, 0x3000),
    (0x00086C, 0xF20000, 0x3000),
    (0x000877, 0xF30000, 0x3000),
    (0x000882, 0xF40000, 0x3000),
    (0x00088D, 0xF50000, 0x3000),
    (0x000898, 0xF60000, 0x3000),
    (0x0008B4, 0xF78000, 0x3000),
    (0x0008C5, 0xF7C000, 0x3000),
)
TARGET_REGISTERS_2000 = (
    "DR16", "DR20", "DR16", "DR16", "DR20",
    "DR28", "DR28", "DR28", "DR28", "DR28",
    "DR28", "DR28", "DR28", "DR28",
)

STORES_BF00 = (
    (0x0008D6, 0xF0AAAA, 0xAA00),
    (0x0008DD, 0xF05554, 0x5500),
    (0x0008E4, 0xF0AAAA, 0x8000),
    (0x0008EB, 0xF0AAAA, 0xAA00),
    (0x0008F2, 0xF05554, 0x5500),
    (0x0008F9, 0xF00000, 0x5000),
)
TARGET_REGISTERS_BF00 = ("DR16", "DR20", "DR16", "DR16", "DR20", "DR28")


def trace_lines(trace_path: Path, stores: tuple[tuple[int, int, int], ...]) -> list[str]:
    lines = []
    for index in range(len(stores)):
        base = RECORD_BASE + index * RECORD_SIZE
        if index:
            lines.append("G")
        for offset, register in ((0, "$"), (4, "DR16"), (8, "DR20"), (12, "DR28"), (20, "states")):
            lines.append(f"_WDWORD(0x{base + offset:06x}, {register})")
        lines.append(f"_WWORD(0x{base + 16:06x}, WR24)")
        lines.append(f"_WBYTE(0x{base + 24:06x}, SP)")
        lines.append(f"_WDWORD(0x{base + 28:06x}, $)")
    end = RECORD_BASE + len(stores) * RECORD_SIZE
    lines.append(f'SAVE "{wine_path(trace_path).replace(chr(92), chr(92) * 2)}" 0x{RECORD_BASE:06x},0x{end:06x}')
    return lines


def prepare(
    source: Path,
    destination: Path,
    first_stop: int,
    stabilizer: int | None,
    stores: tuple[tuple[int, int, int], ...],
) -> Path:
    shutil.copytree(source, destination)
    old = wine_path(source.resolve())
    new = wine_path(destination.resolve())
    for name in ("run.ini", "run.uvproj"):
        path = destination / name
        content = path.read_text()
        content = content.replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\"))
        content = content.replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source path")
        if name == "run.ini":
            breakpoint = f"BS C:0x{first_stop:06x}\n"
            launch = "G C:0xf70000\n"
            if content.count(breakpoint) != 1 or content.count(launch) != 1:
                raise SimulatorError(f"{path}: expected first-unlock script shape changed")
            content = content.replace(breakpoint, "")
            if stabilizer is not None:
                content = content.replace("LOG > ", f"BS C:0x{stabilizer:06x}\nLOG > ", 1)
            content = content.replace(launch, launch + "\n".join(trace_lines(destination / "flash-stops.hex", stores)) + "\n")
        path.write_text(content)
    for name in ("result.hex", "flash-stops.hex"):
        (destination / name).unlink(missing_ok=True)
    return destination / "run.uvproj"


def decode_trace(
    path: Path,
    stores: tuple[tuple[int, int, int], ...],
    target_registers: tuple[str, ...],
) -> list[dict[str, int]]:
    memory = read_intel_hex(path)

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + offset] for offset in range(width)), "big")

    records = []
    previous_states = 0
    for index, ((expected_pc, expected_address, expected_word), target_register) in enumerate(zip(stores, target_registers)):
        base = RECORD_BASE + index * RECORD_SIZE
        pc_samples = (value(base, 4) & 0xFFFFFF, value(base + 28, 4) & 0xFFFFFF)
        registers = {
            "DR16": value(base + 4, 4) & 0xFFFFFF,
            "DR20": value(base + 8, 4) & 0xFFFFFF,
            "DR28": value(base + 12, 4) & 0xFFFFFF,
        }
        address = registers[target_register]
        word = value(base + 16, 2)
        states = value(base + 20, 4)
        sp = value(base + 24, 1)
        if (expected_pc not in pc_samples or any(pc not in (0, expected_pc) for pc in pc_samples)
                or (address, word) != (expected_address, expected_word)):
            raise SimulatorError(
                f"flash stop {index}: got PC samples {pc_samples}, {address:#08x}, {word:#06x}; "
                f"expected {expected_pc:#08x}, {expected_address:#08x}, {expected_word:#06x}"
            )
        if states <= 0 or (index < len(stores) - 1 and states <= previous_states):
            raise SimulatorError(f"flash stop {index}: invalid execution-state count")
        previous_states = states
        records.append({"post_store_pc": expected_pc, "pc_samples": list(pc_samples),
                        "cpu_address": address, "cpu_word": word, "states": states, "sp": sp})
    return records


def conditional_sst_replay(trace: list[dict[str, int]], image: bytes) -> dict[str, object]:
    """Apply only the complete first command under explicitly assumed wiring."""
    bus = Sst39Lf400aBus(image)
    for item in trace[:6]:
        bus.write(item["cpu_address"], item["cpu_word"])
    bus.require_complete()
    if len(bus.effects) != 1 or bus.effects[0].start_address != 0xF00000:
        raise SimulatorError("first captured erase sequence did not yield the expected model effect")
    effect = bus.effects[0]
    next_write_error = None
    if len(trace) > 6:
        try:
            bus.write(trace[6]["cpu_address"], trace[6]["cpu_word"])
        except FlashBusError as error:
            next_write_error = str(error)
        if next_write_error is None:
            raise SimulatorError("subsequent bare sector command unexpectedly formed a complete SST command")
    limited = Sst39Lf400aBus(image, allowed_effect_ranges=(
        (0xF70000, 0xF78000), (0xF7C000, 0xF80000),
    ))
    range_error = None
    try:
        for item in trace[:6]:
            limited.write(item["cpu_address"], item["cpu_word"])
    except FlashBusError as error:
        range_error = str(error)
    if range_error is None or limited.effects:
        raise SimulatorError("operation-5 allowed-range model accepted the initial erase")
    computed, stored = image_checksum(bus.image)
    return {
        "assumptions": "SST39LF400A, high-byte CPU command lane, +0x80000 execution alias, completed first erase",
        "first_effect": {
            "operation": effect.operation,
            "programming_start": effect.start_address,
            "programming_end": effect.end_address,
            "execution_start": effect.start_address + 0x80000,
            "execution_end": effect.end_address + 0x80000,
            "changed_bytes": effect.changed_bytes,
        },
        "reset_vector_after_first_erase": bus.image[0x50:0x54].hex(),
        "computed_checksum_after_first_erase": computed,
        "stored_checksum": stored,
        "next_bare_sector_store_model_error": next_write_error,
        "operation5_allowed_range_model_error": range_error,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--stock-image", required=True, type=Path)
    parser.add_argument("--reconstructed-image", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=90)
    parser.add_argument("--device-code", choices=("2000", "bf00"), default="2000")
    args = parser.parse_args()
    if args.device_code == "2000":
        case_id, first_stop, stabilizer = CASE_ID_2000, 0x00082B, 0x0008B1
        stores, target_registers = STORES_2000, TARGET_REGISTERS_2000
    else:
        case_id, first_stop, stabilizer = CASE_ID_BF00, 0x0008D6, None
        stores, target_registers = STORES_BF00, TARGET_REGISTERS_BF00
    source_run = args.source_run.resolve()
    output = args.output.resolve()
    source_result = json.loads((source_run / "results.json").read_text())
    matches = [result for result in source_result["results"] if result["id"] == case_id]
    if len(matches) != 1 or matches[0]["status"] != "pass":
        raise SimulatorError("source run must contain the passing versioned first-unlock case")
    output.mkdir(parents=True, exist_ok=False)
    traces = {}
    for side in ("stock", "reconstructed"):
        source = source_run / case_id / side
        destination = output / side
        project = prepare(source, destination, first_stop, stabilizer, stores)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-trace.log", args.timeout)
        trace_path = destination / "flash-stops.hex"
        if not trace_path.exists():
            raise SimulatorError(f"{side}: missing flash-stop trace")
        traces[side] = decode_trace(trace_path, stores, target_registers)
    def normalized(trace: list[dict[str, int]]) -> list[tuple[int, int, int, int]]:
        return [(item["post_store_pc"], item["cpu_address"], item["cpu_word"], item["sp"]) for item in trace]

    if normalized(traces["stock"]) != normalized(traces["reconstructed"]):
        raise SimulatorError("stock and reconstructed flash CPU-store traces differ")
    images = {
        "stock": args.stock_image.read_bytes(),
        "reconstructed": args.reconstructed_image.read_bytes(),
    }
    image_hashes = {side: hashlib.sha256(image).hexdigest() for side, image in images.items()}
    if image_hashes["stock"] != source_result["run_identity"]["stock_sha256"]:
        raise SimulatorError("stock image identity differs from the source run")
    conditional = {side: conditional_sst_replay(traces[side], images[side]) for side in traces}
    report = {
        "schema_version": 1,
        "declared_device_code": args.device_code,
        "scope": "offline S251 CPU stores; flash chip effects and address alias unverified",
        "source_run_identity": source_result["run_identity"],
        "source_case_sha256": matches[0]["case_sha256"],
        "image_sha256": image_hashes,
        "conditional_sst_replay": conditional,
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "stock": traces["stock"],
        "reconstructed": traces["reconstructed"],
        "final_state_counter_drop": {
            side: trace[-1]["states"] < trace[-2]["states"]
            for side, trace in traces.items()
        },
        "status": "pass",
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"{len(stores)} matching CPU flash-window stores; last post-store PC 0x{stores[-1][0]:06x}")


if __name__ == "__main__":
    main()
