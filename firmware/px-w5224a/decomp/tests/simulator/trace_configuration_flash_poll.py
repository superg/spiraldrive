"""Compare the fourth scheduler resume through a declared flash-poll reply.

The reply is an S251 test input, not a measurement of the drive's flash chip.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path


CASE_ID = "boot-third-resume0-two-cycles-exact-context"
EXPECTED_PCS = (0x818, 0x81F, 0x826, 0x82D, 0x834, 0x848, 0x872, 0x90E)
POLL_REPLY = 0xFF


def save_line(path: Path, start: int, end: int) -> str:
    escaped = wine_path(path).replace("\\", "\\\\")
    return f'SAVE "{escaped}" 0x{start:06x},0x{end:06x}'


def prepare(source: Path, destination: Path) -> Path:
    shutil.copytree(source, destination)
    old = wine_path(source.resolve())
    new = wine_path(destination.resolve())
    for name in ("run.ini", "run.uvproj"):
        path = destination / name
        content = path.read_text()
        content = content.replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\"))
        content = content.replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source output path")
        if name == "run.ini":
            anchor = "G\n_WBYTE(0x014003, 0)\n"
            if content.count(anchor) != 1:
                raise SimulatorError(f"{path}: fourth-resume insertion point changed")
            lines = []
            for index in range(8):
                lines += ["G", f"_WDWORD(0x016000 + {4 * index}, $)"]
            lines += [
                "MAP 0xf7a000,0xf7afff READ WRITE",
                f"_WBYTE(0xf7a000, 0x{POLL_REPLY:02x})",
                "_WBYTE(0x016030, _RBYTE(0xf7a000))",
                "G",
                "_WDWORD(0x016024, $)",
                "_WBYTE(0x016031, _RBYTE(0xf7a000))",
                "_WBYTE(0x016032, ACC)",
                save_line(destination / "poll-resume-pcs.hex", 0x016000, 0x01603F),
                save_line(destination / "relocated-programmer.hex", 0x000800, 0x000A8D),
                "_WBYTE(0x014003, 0)",
            ]
            content = content.replace(anchor, "\n".join(lines) + "\n")
        path.write_text(content)
    for name in ("poll-resume-pcs.hex", "relocated-programmer.hex", "result.hex"):
        (destination / name).unlink(missing_ok=True)
    return destination / "run.uvproj"


def decode(destination: Path) -> dict[str, object]:
    memory = read_intel_hex(destination / "poll-resume-pcs.hex")

    def value(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + index] for index in range(width)), "big")

    pcs = [value(0x016000 + 4 * index, 4) & 0xFFFFFF for index in range(8)]
    pcs.append(value(0x016024, 4) & 0xFFFFFF)
    entry = pcs[0]
    if (entry < 0xF80000 or pcs[1] not in (0, EXPECTED_PCS[0])
            or tuple(pcs[2:]) != EXPECTED_PCS[1:]):
        raise SimulatorError(f"{destination.name}: unexpected fourth-resume stops: {pcs}")
    reply = [value(0x016030 + index, 1) for index in range(3)]
    if reply != [POLL_REPLY] * 3:
        raise SimulatorError(f"{destination.name}: flash-poll readback/ACC: {reply}")
    programmer = read_intel_hex(destination / "relocated-programmer.hex")
    code = bytes(programmer[address] for address in range(0x800, 0xA8E))
    return {
        "resume_entry_pc": entry,
        "post_resume_pcs": pcs,
        "first_post_entry_capture_valid": pcs[1] == EXPECTED_PCS[0],
        "poll_readback_before": reply[0],
        "poll_readback_after": reply[1],
        "acc_after": reply[2],
        "relocated_programmer_sha256": hashlib.sha256(code).hexdigest(),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--uv4", required=True, type=Path)
    parser.add_argument("--timeout", type=int, default=45)
    args = parser.parse_args()
    source_run = args.source_run.resolve()
    output = args.output.resolve()
    source_report = json.loads((source_run / "results.json").read_text())
    matches = [item for item in source_report["results"] if item["id"] == CASE_ID]
    if len(matches) != 1 or matches[0]["status"] != "pass":
        raise SimulatorError("source run needs the passing versioned scheduler case")
    output.mkdir(parents=True, exist_ok=False)
    traces = {}
    for side in ("stock", "reconstructed"):
        destination = output / side
        project = prepare(source_run / CASE_ID / side, destination)
        run_uv4(args.uv4.resolve(), project, destination / "uv4-trace.log", args.timeout)
        traces[side] = decode(destination)
    comparable = ("poll_readback_before", "poll_readback_after",
                  "acc_after", "relocated_programmer_sha256")
    if (traces["stock"]["post_resume_pcs"][2:] != traces["reconstructed"]["post_resume_pcs"][2:]
            or any(traces["stock"][key] != traces["reconstructed"][key] for key in comparable)):
        raise SimulatorError("stock and reconstructed conditional flash-poll traces differ")
    report = {
        "schema_version": 1,
        "status": "pass",
        "scope": "offline S251 continuation with a declared 0xff poll reply; physical flash response unverified; the first PC capture may be zero under the debugger",
        "source_case": CASE_ID,
        "source_case_sha256": matches[0]["case_sha256"],
        "source_run_identity": source_report["run_identity"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "stock": traces["stock"],
        "reconstructed": traces["reconstructed"],
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print("matched stock and reconstructed fourth-resume stops through RAM:00090e")


if __name__ == "__main__":
    main()
