"""Produce diag16: diag14 with only the stock-correct P1.7 input address.

This is an offline isolation experiment, not an accepted reconstruction.
The normal C source retains all three correct input addresses. No drive access.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tools import firmware_image

DIAG14_SHA256 = '0da270c08f862848d60f420221bf23de11378c389a8a52bdefee19b9d7a40669'
DIAG15_SHA256 = '4178646748630d77e986fc898959738050044ed54727fde1e2d3d2ab73dc880a'
OPERANDS = (0x1630a, 0x16317, 0x16324)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--diag14', type=Path, required=True)
    parser.add_argument('--diag15', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True, help='new output directory')
    args = parser.parse_args()
    baseline, corrected = args.diag14.read_bytes(), args.diag15.read_bytes()
    stock = (ROOT / 'data/W5224A104/MONT_104.bin').read_bytes()
    require = firmware_image.require
    require(hashlib.sha256(baseline).hexdigest() == DIAG14_SHA256, 'diag14 identity mismatch')
    require(hashlib.sha256(corrected).hexdigest() == DIAG15_SHA256, 'diag15 identity mismatch')
    for image in (baseline, corrected):
        firmware_image.verify(image, fixed_data_reference=stock)
    delta = {i for i, (a, b) in enumerate(zip(baseline, corrected)) if a != b}
    require(delta == {p + n for p in (*OPERANDS, 0x79ffe) for n in (0, 1)},
            'unexpected diag14/diag15 delta')
    for offset in OPERANDS:
        require(baseline[offset:offset+2] == bytes.fromhex('13 00'), 'old operand differs')
        require(corrected[offset:offset+2] == bytes.fromhex('11 ed'), 'correct operand differs')
    # Linked RAM:f96322: MOV R11,001300; JNB ACC.7. Canonical stock fb01d0
    # instead reads RAM:0011ed. Change the address operand, never the opcode.
    require(baseline[0x16322:0x1632f] == bytes.fromhex('7e b3 13 00 30 e7 04 74 01 80 01 e4 aa'),
            'P1.7 instruction sequence differs')
    candidate = bytearray(baseline)
    candidate[OPERANDS[2]:OPERANDS[2]+2] = corrected[OPERANDS[2]:OPERANDS[2]+2]
    checksum = (sum(candidate[:0x70000]) + sum(candidate[0x78000:0x79ffe])) & 0xffff
    candidate[0x79ffe:0x7a000] = checksum.to_bytes(2, 'big')
    candidate = bytes(candidate)
    validation = firmware_image.verify(candidate, fixed_data_reference=stock)
    differences = [dict(file_offset=f'0x{i:05x}', cpu_address=f'RAM:{0xf80000+i:06x}',
                        diag14=a, diag16=b)
                   for i, (a, b) in enumerate(zip(baseline, candidate)) if a != b]
    require({int(d['file_offset'], 16) for d in differences} ==
            {0x16324, 0x16325, 0x79ffe, 0x79fff}, 'unexpected diagnostic changes')
    require(candidate[0x16322:0x1632f] == corrected[0x16322:0x1632f],
            'P1.7 body differs from tested diag15')
    firmware_image.check_output(args.output / 'PXW5224A_diag16_p1_bit7_only.bin',
                                (args.diag14, args.diag15))
    args.output.mkdir(parents=True, exist_ok=False)
    path = args.output / 'PXW5224A_diag16_p1_bit7_only.bin'
    path.write_bytes(candidate)
    manifest = dict(scope='Diagnostic isolation only; P1.5/P1.6 retain known diag14 defects',
                    baseline_sha256=DIAG14_SHA256, corrected_sha256=DIAG15_SHA256,
                    image_sha256=hashlib.sha256(candidate).hexdigest(),
                    generator_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                    checksum=f'0x{checksum:04x}', differences_from_diag14=differences,
                    validation=validation)
    (args.output / 'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
    (args.output / 'README.md').write_text(
        '# Diag16: isolate P1.7 address correction\n\n'
        'Diagnostic only. Based on hash-pinned diag14; change only the two-byte\n'
        'input address at linked RAM:f96324 and the checksum. P1.5/P1.6 keep\n'
        'the known diag14 address defects. Normal C source stays corrected.\n\n'
        'The complete P1.7 body equals the simulator-tested diag15 body; every\n'
        'other byte except the checksum equals diag14. No relink or layout\n'
        'change. Strict image/checksum/stock-fixed-data checks pass. No new\n'
        'simulator run is claimed. See manifest.json for exact byte changes.\n\n'
        'User flashes with redumper; test normal power-on with a closed, empty\n'
        'tray. Record Windows detection and tray motion. If startup fails,\n'
        'hold Eject at power-on and restore stock using the established path.\n'
        'A failed startup implicates the P1.7 correction or its downstream\n'
        'effects; successful startup leaves P1.5/P1.6 or interactions open.\n')
    print(json.dumps(manifest, indent=2))


if __name__ == '__main__':
    main()
