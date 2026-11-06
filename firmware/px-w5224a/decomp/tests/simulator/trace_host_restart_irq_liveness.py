"""Check same-priority IRQ delivery after the real firmware restart interrupt.

Run under tools/run_on_private_desktop.py on Windows. The first stage executes
the captured monitor and each firmware's actual host cause-0x10 restart path.
At InitializeAndEnterFirmwareMain, replace only the subsequent initialization
with a declared synthetic IRQ consumer. Do not RESET the CPU between stages:
that would erase the interrupt-in-service state this regression checks.

This establishes interrupt release, not physical startup or USB enumeration.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tests.simulator.simulator import parse_keil_map, read_intel_hex, run_uv4, wine_path

CASE_ID = 'host-interface-main-vector-status-cause10-initialization-entry-via-timer-source'


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--firmware', type=Path, default=ROOT / 'out/windows/firmware/Release')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/agent-tests/host-restart-irq-liveness')
    parser.add_argument('--keil-root', type=Path, default=Path('C:/Keil_v5/C251'))
    parser.add_argument('--timeout', type=int, default=30)
    parser.add_argument('--negative-control', action='store_true',
                        help='bypass the RETI adapter in debugger memory; the liveness check must fail')
    args = parser.parse_args()
    firmware, output, keil = args.firmware.resolve(), args.output.resolve(), args.keil_root.resolve()
    output.mkdir(parents=True, exist_ok=True)
    cases = json.loads((ROOT / 'tests/simulator/monitor-cases/integration.json').read_text())['cases']
    case = copy.deepcopy(next(item for item in cases if item['id'] == CASE_ID))
    symbols = parse_keil_map(firmware / 'main.map')
    if args.negative_control:
        target = symbols['RestartFirmware']
        stock = (ROOT / 'data/W5224A104/MONT_104.bin').read_bytes()
        case['code_patches'].append({
            'reason': 'Negative control: debugger-only direct restart recreates the missing RETI; stock entry unchanged.',
            'stock_address': '0xf80050', 'stock_bytes': list(stock[0x50:0x54]),
            'reconstructed_address': hex(symbols['RestartFirmwareAfterInterrupt']),
            'reconstructed_bytes': [0x8a, (target >> 16) & 255, (target >> 8) & 255, target & 255],
        })
    cases_path = output / 'cases.json'
    cases_path.write_text(json.dumps({'schema_version': 1, 'cases': [case]}, indent=2))
    uv4 = keil.parent / 'UV4/UV4.exe'
    command = [sys.executable, str(ROOT / 'tests/simulator/simulator.py'), 'differential']
    options = {
        '--uv4': uv4,
        '--base-project': keil / 'Examples/Blinky/BLINKY.uvproj',
        '--application': firmware / 'main.abs',
        '--keil-driver': ROOT / 'tools/keil_driver.py',
        '--keil-root': keil,
        '--a251': keil / 'BIN/A251.exe',
        '--l251': keil / 'BIN/L251.exe',
        '--stock': ROOT / 'data/W5224A104/MONT_104.bin',
        '--stock-application': ROOT / 'out/simulator/stock/STOCK_FIRMWARE.ABS',
        '--stock-memory-application': ROOT / 'out/simulator/stock/FIRMWARE_MEMORY.ABS',
        '--reconstructed-image': firmware / 'MONT_reconstructed.bin',
        # uVision needs the code-space application, not the untyped HEX view.
        '--monitor-hex': ROOT / 'out/simulator/images/MONITOR.ABS',
        '--map': firmware / 'main.map',
        '--listings': firmware,
        '--metadata': ROOT / 'tests/simulator/metadata/W5224A104.json',
        '--cases': cases_path,
        '--output': output / 'entry',
        '--timeout': args.timeout,
    }
    for key, value in options.items():
        command.extend((key, str(value)))
    subprocess.run(command, cwd=ROOT, check=True)
    entry = json.loads((output / 'entry/results.json').read_text())
    if len(entry['results']) != 1 or entry['results'][0]['status'] != 'pass':
        raise RuntimeError('real interrupt-to-restart entry comparison did not pass')
    summary = {'run_identity': entry['run_identity'], 'probe_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
               'negative_control': args.negative_control,
               'model': 'synthetic second Timer0 IRQ after actual routed host cause-0x10 restart; no CPU RESET',
               'results': {}}
    for side in ('stock', 'reconstructed'):
        source = output / 'entry' / CASE_ID / side
        dest = output / 'second-irq' / side
        shutil.copytree(source, dest, dirs_exist_ok=True)
        old, new = wine_path(source), wine_path(dest)
        for name in ('run.ini', 'run.uvproj'):
            path = dest / name
            content = path.read_text().replace(old.replace('\\', '\\\\'), new.replace('\\', '\\\\')).replace(old, new)
            if name == 'run.ini':
                anchor = 'G C:0xf70000\n'
                if content.count(anchor) != 1:
                    raise RuntimeError('generated initial execution anchor changed')
                content = content[:content.index(anchor)] + anchor
                content += '_WDWORD(0x016000, $)\n_WBYTE(0x016004, SP)\n'
                content += '_WBYTE(0x017003, 0)\nBK *\n'
                # The new handler only records delivery and RETIs. Equalizing
                # SP and ordinary SFRs does not clear the hidden in-service latch.
                content += '_WBYTE(D:0xa8, 0)\n_WBYTE(D:0x88, 0)\n_WBYTE(D:0x40, 0)\n'
                content += '_WBYTE(D:0x81, 7)\n_WBYTE(D:0xbe, 0)\n'
                for address, data in (
                    (0xff000b, [0x8a, 0xf7, 0, 0xa0]),  # EJMP marker ISR
                    (0xf70080, [0x75, 0xa8, 0x82, 0x75, 0x88, 0x20, 0x80, 0xfe]),
                    (0xf700a0, [0x75, 0x40, 0xa5, 0x32]),  # MOV 40,#a5; RETI
                ):
                    for offset, value in enumerate(data):
                        content += f'_WBYTE(C:0x{address+offset:06x}, {value})\n'
                content += 'PlextorInstructionLimit()\nG C:0xf70080\n'
                content += '_WDWORD(0x016008, $)\n_WBYTE(0x01600c, _RBYTE(D:0x40))\n_WBYTE(0x01600d, _RBYTE(D:0x88))\n'
                result_path = wine_path(dest / 'second-irq.hex').replace('\\', '\\\\')
                content += f'SAVE "{result_path}" 0x016000,0x01600d\nEXIT\n'
            path.write_text(content)
        (dest / 'second-irq.hex').unlink(missing_ok=True)
        run_uv4(uv4, dest / 'run.uvproj', dest / 'probe.log', args.timeout)
        memory = read_intel_hex(dest / 'second-irq.hex')
        word = lambda address: int.from_bytes(bytes(memory[address+i] for i in range(4)), 'big') & 0xffffff
        expected_entry = 0xfee7ed if side == 'stock' else symbols['InitializeAndEnterFirmwareMain']
        actual = {'restart_pc': word(0x16000), 'restart_sp': memory[0x16004],
                  'stop_pc': word(0x16008), 'second_irq_marker': memory[0x1600c], 'tcon': memory[0x1600d]}
        actual['pass'] = (actual['restart_pc'] == expected_entry and actual['stop_pc'] == 0xf70086
                          and actual['second_irq_marker'] == 0xa5 and actual['tcon'] == 0)
        summary['results'][side] = actual
    summary['pass'] = all(result['pass'] for result in summary['results'].values())
    (output / 'results.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary['results'], indent=2))
    return 0 if summary['pass'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
