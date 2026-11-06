"""Reach the main-image staged checksum after a declared passing header.

Starts from the current-image paired malformed-header trace. Both S251 runs
receive the immutable stock identity in their staged controller window; the
existing debugger-only controller-copy bank substitution is retained.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from tests.simulator.simulator import SimulatorError, read_intel_hex, run_uv4, wine_path

STOCK_SHA256 = "622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2"
STOCK = Path(__file__).resolve().parents[2] / "data/W5224A104/MONT_104.bin"
POINTS = {
    "stock": 0xFD2C93,
    "reconstructed": 0xF84FB4,
}


def run_side(source: Path, output: Path, checksum: int, identity: bytes, uv4: Path, timeout: int) -> dict[str, object]:
    shutil.copytree(source, output)
    old = wine_path(source.resolve())
    new = wine_path(output.resolve())
    trace = wine_path(output / "main-checksum-entry.hex").replace("\\", "\\\\")
    for name in ("run.ini", "run.uvproj"):
        path = output / name
        content = path.read_text().replace(old.replace("\\", "\\\\"), new.replace("\\", "\\\\")).replace(old, new)
        if old in content:
            raise SimulatorError(f"{path}: stale source path")
        if name == "run.ini":
            seed = "_WBYTE(X:0x0030, 0x00)\n"
            if content.count(seed) != 1:
                raise SimulatorError("staged identity seed changed")
            content = content.replace(seed, "\n".join(f"_WBYTE(X:0x{0x30+i:04x}, 0x{byte:02x})" for i, byte in enumerate(identity)) + "\n", 1)
            anchor = f'SAVE "{wine_path(output / "mode5-copy-entry.hex").replace(chr(92), chr(92) * 2)}" 0x016000,0x01601f\n'
            if content.count(anchor) != 1:
                raise SimulatorError("copy-entry continuation point changed")
            content = content.split(anchor, 1)[0] + anchor + "\n".join([
                "BK *", f"BS C:0x{checksum:06x}",
                "BS C:0xff4000", "BS C:0x000800", "G",
                "_WDWORD(0x016080, $)", "_WDWORD(0x016084, states)",
                "_WDWORD(0x016088, DR0)", "_WDWORD(0x01608c, DR4)",
                "_WDWORD(0x016090, DR60)",
                "_WBYTE(0x016094, _RBYTE(X:0x0030))",
                "_WBYTE(0x016095, _RBYTE(E:0x000fae))",
                f'SAVE "{trace}" 0x016080,0x01609f',
                "EXIT", "",
            ])
            content = content.replace("twatch(500000)", "twatch(1000000)")
        path.write_text(content)
    (output / "main-checksum-entry.hex").unlink(missing_ok=True)
    run_uv4(uv4, output / "run.uvproj", output / "uv4-main-checksum-entry.log", timeout)
    memory = read_intel_hex(output / "main-checksum-entry.hex")
    def val(address: int, width: int) -> int:
        return int.from_bytes(bytes(memory[address + i] for i in range(width)), "big")
    result = {
        "stop_pc": val(0x16080, 4) & 0xFFFFFF,
        "states": val(0x16084, 4),
        "start_offset": val(0x16088, 4),
        "checksum_word_offset": val(0x1608C, 4),
        "spx": val(0x16090, 4),
        "staged_identity_first_byte": val(0x16094, 1),
        "media_state": val(0x16095, 1),
    }
    if result["stop_pc"] != checksum or result["start_offset"] != 0 or result["checksum_word_offset"] != 0x79FFE:
        raise SimulatorError(f"{output.name}: main checksum entry mismatch: {result}")
    if result["staged_identity_first_byte"] != 0x50 or result["media_state"] != 1 or not 1500 < result["states"] < 1000000:
        raise SimulatorError(f"{output.name}: declared main header state mismatch: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-run", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--uv4", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=90)
    args = parser.parse_args()
    stock = STOCK.read_bytes()
    if hashlib.sha256(stock).hexdigest() != STOCK_SHA256:
        raise SimulatorError("immutable stock image changed")
    identity = stock[0x30:0x43]
    if identity != b"PLEXTOR MONT    000":
        raise SimulatorError("stock identity changed")
    source = args.source_run.resolve()
    source_report = json.loads((source / "results.json").read_text())
    if source_report.get("status") != "pass":
        raise SimulatorError("source run needs paired main header rejection")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    observations = {side: run_side(source / side, output / side, checksum, identity, args.uv4.resolve(), args.timeout)
                    for side, checksum in POINTS.items()}
    report = {
        "schema_version": 1, "status": "pass",
        "scope": "Current-image main mode-5 passing staged identity reaches checksum entry with DR0=0 and DR4=0x79ffe on both images; declared media state and S251-only controller-copy bank substitution. No checksum decision or flash-loader execution.",
        "stock_image_sha256": STOCK_SHA256,
        "source_run_identity": source_report["source_run_identity"],
        "source_trace_sha256": source_report["script_sha256"],
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        **observations,
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(observations, indent=2))


if __name__ == "__main__":
    main()
