"""Trace a declared firmware-1.04 host restart through two timer overflows and
two routed controller event-40 completions. The generated versioned host cause
0x10 case supplies the validated interrupt-to-initialization checkpoint. This
is a bounded S251 scenario, not physical timer or controller evidence.
"""

from pathlib import Path
import hashlib
import json
import os
import shutil

from tests.simulator.simulator import read_intel_hex, run_uv4, wine_path


def find_uv4() -> Path:
    configured = os.environ.get('KEIL_UVISION_EXECUTABLE')
    candidates = [Path(configured)] if configured else []
    candidates.extend((
        Path('C:/Keil_v5/UV4/UV4.exe'),
        Path.home() / '.wine/drive_c/Keil_v5/UV4/UV4.exe',
    ))
    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()
    raise RuntimeError('UV4.exe was not found; set KEIL_UVISION_EXECUTABLE')

ROOT = Path('out/agent-tests')
CASE = 'host-interface-main-vector-status-cause10-initialization-entry-via-timer-source'
SOURCE = ROOT / 'hostrestart_init_versioned' / CASE
DEST = ROOT / 'hostrestart_initialization_trace' / CASE
TARGET = 0xfdcea9
results = {}
uv4 = find_uv4()
source_identity = json.loads((ROOT / 'hostrestart_init_versioned' / 'results.json').read_text())['run_identity']
application = Path('out/simulator/build/firmware/Release/main.abs')
if hashlib.sha256(application.read_bytes()).hexdigest() != source_identity['application_sha256']:
    raise RuntimeError('linked application differs from the validated source case')

for side in ('stock', 'reconstructed'):
    source = SOURCE / side
    dest = DEST / side
    shutil.copytree(source, dest, dirs_exist_ok=True)
    old = wine_path(source.resolve())
    new = wine_path(dest.resolve())
    for name in ('run.ini', 'run.uvproj'):
        path = dest / name
        content = path.read_text().replace(old.replace('\\', '\\\\'), new.replace('\\', '\\\\')).replace(old, new)
        if old in content:
            raise RuntimeError(f'stale source path in {path}')
        if name == 'run.ini':
            if content.count('twatch(1500);') != 1:
                raise RuntimeError(f'instruction limit changed in {path}')
            content = content.replace('twatch(1500);', 'twatch(20000);', 1)
            content = content.replace('MAP 0x000000,0x00ffff READ WRITE EXEC', 'signal void PlextorPostTimerLimit(void) { twatch(500000); _break_ = 1; }\nMAP 0x000000,0x00ffff READ WRITE EXEC', 1)
            event_addr = 'X:0x8003' if side == 'stock' else '0x018003'
            source_addr = 'X:0x807f' if side == 'stock' else '0x01807f'
            extra = f'''signal void RestartEventMask(void) {{ while (1) {{ rwatch({event_addr}); if (_RBYTE(0x017003) == 0) continue; _WBYTE(0x017002, 1); _WBYTE({event_addr}, 0xbf); _WBYTE(0x017002, 0); }} }}\nsignal void RestartEventSource(void) {{ while (1) {{ rwatch({source_addr}); if (_RBYTE(0x017003) == 0) continue; _WBYTE(0x017002, 1); _WBYTE({source_addr}, 0xff); _WBYTE(0x017002, 0); }} }}\nsignal void RestartEventTrigger(void) {{ twatch(5); _WBYTE(D:0x88, 3); }}\n'''
            extra += 'signal void RestartAfterSecondEventLimit(void) { twatch(200000); _break_ = 1; }\n'
            content = content.replace('MAP 0x000000,0x00ffff READ WRITE EXEC', extra + 'MAP 0x000000,0x00ffff READ WRITE EXEC', 1)
            anchor = 'G C:0xf70000\n_WBYTE(0x017003, 0)\n'
            if content.count(anchor) != 1:
                raise RuntimeError(f'checkpoint anchor changed in {path}')
            trace = wine_path(dest / 'followthrough.hex').replace('\\', '\\\\')
            lines = [
                'G C:0xf70000',
                '_WDWORD(0x016000, $)',
                '_WDWORD(0x016004, states)',
                'BK *',
                f'BS C:0x{TARGET:06x}',
                'G',
                '_WDWORD(0x016008, $)',
                '_WDWORD(0x01600c, states)',
                '_WBYTE(0x016010, _RBYTE(E:0x0000fb))',
                '_WBYTE(0x016011, _RBYTE(D:0x00a8))',
                '_WBYTE(0x016012, _RBYTE(D:0x00a9))',
                '_WBYTE(0x016013, _RBYTE(D:0x0091))',
                '_WBYTE(D:0x0091, _RBYTE(D:0x0091) | 0x80)',
                '_WBYTE(0x016014, _RBYTE(D:0x0091))',
                'PlextorInstructionLimit()',
                'G',
                '_WDWORD(0x016018, $)',
                '_WDWORD(0x01601c, states)',
                '_WBYTE(0x016020, _RBYTE(D:0x0091))',
                '_WBYTE(D:0x0091, _RBYTE(D:0x0091) | 0x80)',
                '_WBYTE(0x016021, _RBYTE(D:0x0091))',
                'PlextorInstructionLimit()',
                'G',
                '_WDWORD(0x016024, $)',
                '_WDWORD(0x016028, states)',
                '_WBYTE(0x01602c, _RBYTE(D:0x0091))',
                'PlextorPostTimerLimit()',
                'G',
                '_WDWORD(0x016030, $)',
                '_WDWORD(0x016034, states)',
                '_WBYTE(0x016038, _RBYTE(D:0x0091))',
                '_WBYTE(0x016039, _RBYTE(E:0x0000fb))',
                '_WBYTE(0x01603a, _RBYTE(E:0x0014da))',
                '_WBYTE(0x01603b, _RBYTE(D:0x00a8))',
                '_WBYTE(0x01603c, _RBYTE(E:0x0000fc))',
                '_WBYTE(0x01603d, _RBYTE(0x018010))',
                '_WBYTE(0x01603e, _RBYTE(0x018011))',
                '_WBYTE(0x01603f, _RBYTE(0x018012))',
                'RestartEventMask()',
                'RestartEventSource()',
                'RestartEventTrigger()',
                'PlextorInstructionLimit()',
                'G',
                '_WDWORD(0x016040, $)',
                '_WDWORD(0x016044, states)',
                '_WBYTE(0x016048, _RBYTE(E:0x0014da))',
                '_WBYTE(0x016049, _RBYTE(E:0x00122a))',
                '_WBYTE(0x01604a, _RBYTE(E:0x001097))',
                '_WBYTE(0x01604b, _RBYTE(0x018010))',
                '_WBYTE(0x01604c, _RBYTE(0x018011))',
                '_WBYTE(0x01604d, _RBYTE(0x018012))',
                'RestartEventTrigger()',
                'RestartAfterSecondEventLimit()',
                'G',
                '_WDWORD(0x016050, $)',
                '_WDWORD(0x016054, states)',
                '_WBYTE(0x016058, _RBYTE(E:0x0014da))',
                '_WBYTE(0x016059, _RBYTE(D:0x00a8))',
                '_WBYTE(0x01605a, _RBYTE(E:0x0000fc))',
                '_WBYTE(0x017003, 0)',
                f'SAVE "{trace}" 0x016000,0x01605a',
            ]
            content = content.replace(anchor, '\n'.join(lines) + '\n', 1)
        path.write_text(content)
    (dest / 'followthrough.hex').unlink(missing_ok=True)
    run_uv4(uv4, dest / 'run.uvproj', dest / 'uv4-followthrough.log', 45)
    data = read_intel_hex(dest / 'followthrough.hex')
    word = lambda offset: int.from_bytes(bytes(data[offset + i] for i in range(4)), 'big')
    results[side] = {
        'checkpoint_pc': word(0x16000) & 0xffffff,
        'checkpoint_states': word(0x16004),
        'continuation_pc': word(0x16008) & 0xffffff,
        'continuation_states': word(0x1600c),
        'firmware_active': data[0x16010],
        'interrupt_enable': data[0x16011],
        'interrupt_control': data[0x16012],
        'timer_before': data[0x16013],
        'timer_injected': data[0x16014],
        'after_timer_pc': word(0x16018) & 0xffffff,
        'after_timer_states': word(0x1601c),
        'timer_after': data[0x16020],
        'timer_second_injected': data[0x16021],
        'after_second_timer_pc': word(0x16024) & 0xffffff,
        'after_second_timer_states': word(0x16028),
        'timer_second_after': data[0x1602c],
        'post_timers_pc': word(0x16030) & 0xffffff,
        'post_timers_states': word(0x16034),
        'post_timers_status': data[0x16038],
        'post_timers_firmware_active': data[0x16039],
        'event_wait_before': data[0x1603a],
        'event_interrupt_enable': data[0x1603b],
        'event_vector_selector': data[0x1603c],
        'first_fill_address': [data[0x1603d+i] for i in range(3)],
        'after_event_pc': word(0x16040) & 0xffffff,
        'after_event_states': word(0x16044),
        'event_wait_after': data[0x16048],
        'event_mask_after': data[0x16049],
        'event_source_after': data[0x1604a],
        'second_fill_address': [data[0x1604b+i] for i in range(3)],
        'after_second_event_pc': word(0x16050) & 0xffffff,
        'after_second_event_states': word(0x16054),
        'after_second_event_wait': data[0x16058],
        'after_second_event_a8': data[0x16059],
        'after_second_event_selector': data[0x1605a],
    }

expected_pcs = {
    'stock': (0xfee7ed, 0xfdbdf4, 0xfacd57),
    'reconstructed': (0xfeae33, 0xfe9f13, 0xfea0ba),
}
for side, result in results.items():
    checkpoint, timer_poll, first_fill_poll = expected_pcs[side]
    if (result['checkpoint_pc'], result['continuation_pc'], result['post_timers_pc']) != (checkpoint, timer_poll, first_fill_poll):
        raise RuntimeError(f'{side}: unexpected restart checkpoint sequence: {result}')
    if result['first_fill_address'] != [0, 0, 0x1b] or result['second_fill_address'] != [0, 0x80, 0x1b]:
        raise RuntimeError(f'{side}: controller fill did not advance to the second 0x8000-byte chunk')
    if result['event_wait_before'] != 0 or result['event_interrupt_enable'] != 0x81 or result['event_vector_selector'] != 1:
        raise RuntimeError(f'{side}: first controller event preconditions changed')
    if result['event_mask_after'] != 0xbe or result['event_source_after'] != 0xff:
        raise RuntimeError(f'{side}: controller event-40 routing changed')
    if result['after_second_event_wait'] != 1 or result['after_second_event_a8'] != 0:
        raise RuntimeError(f'{side}: second controller completion or interrupt disable changed')

DEST.mkdir(parents=True, exist_ok=True)
(DEST / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
print(json.dumps(results, indent=2))
