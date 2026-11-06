# Active context

- **2026-11-05 archive retention decision:** User plans to delete all of `out/` and does not need its drive captures, recovery backups, or other historical artifacts. This supersedes the audit's recommendation to preserve those outputs for this archive. Agent left deletion to the user; no files deleted. Maintained source and canonical Ghidra project are outside `out/`.

- **2026-11-05 archive cleanup audit:** See `docs/ARCHIVE_CLEANUP.md`. No deletions performed. Initial inventory: 6.44 GiB excluding `.git`, including 6.36 GiB under mixed-purpose `out/`. Classified 3.17 GiB of build/cache candidates (exact untracked file list in `out/agent-tests/archive-audit/deletion-candidates.tsv`). Preserve physical captures, recovery backups, exact diagnostic images, historical source snapshots, and the complete saved Ghidra project. Current workspace view has no `data/`; archive original inputs from existing storage. A Git-only archive misses modified/untracked source/docs and current Ghidra database generations.

- **2026-11-05 patcher and patch descriptions moved:** The contents of `tools/fwpatch-plextor/` and the entire `firmware/` directory now live in sibling repository `../spiraldrive-plextor/`. The old patcher directory is empty but Windows currently prevents its removal because another process holds it open. Build and run the patcher from the new repository; original images under `data/` and analysis documents remain here. Earlier entries below describe the former locations.

- **2026-11-04 current physical H: revision is G107:** User changed only the maintained York JSON revision from `P107` to `G107` and explicitly requested rebuilding/reflashing. Rebuilt workspace-root `York_107_patched.bin`; checksum `0xefc2`, SHA-1 `98988dd707ac17fa4562ac18feba14c8669824e1`, SHA-256 `28dc081dd98aa64244967c4e05cf5610fd1eb7c8c92015e76132ec53851c5042`. Independent comparison to the previous P107 image confirms changes only at file `0x18` (revision) and `0x79fff` (checksum); both instruction patches are identical. Redumper `flash::plextor --drive=H:` completed successfully/exit 0 after 12s, without force. Preflight INQUIRY identified PX-W4824A/P107; post-flash `12 00 00 00 60 00` DATA IN returned GOOD/96 bytes and PX-W4824A/G107. Logs and image metadata: `out/agent-tests/york-g107-flash-20261104/`. Original York 1.07 remains the rollback image. Redumper's pre-flash configuration lookup did not recognize P107 and fell back to generic BE; this did not affect its model-based Plextor flash route. Lead-in speed/continuity still unmeasured.

- **2026-11-04 York P107 flashed to physical H: with explicit user authorization:** Preflight standard INQUIRY identified `PLEXTOR / CD-R   PX-W4824A / 1.07` (USB identity `11100E00005496FF`). Ran `../redumper/build/Release/redumper.exe flash::plextor --drive=H: --firmware=C:/Users/G/projects/gplextor/York_107_patched.bin --image-path=C:/Users/G/projects/gplextor/out/agent-tests/york-107-flash-20261104 --image-name=redumper-flash --scsi-timeout=120000`, without force. Redumper transferred the 512-KiB image in 4-KiB blocks, reported `flashing success`, and exited 0 after 12s. Post-flash DATA-IN INQUIRY `12 00 00 00 60 00` returned GOOD/96 bytes and `PLEXTOR / CD-R   PX-W4824A / P107`, confirming activation. Log: `out/agent-tests/york-107-flash-20261104/redumper-flash.log`; pre/post inquiry JSON and raw bytes are in that directory. Flashed SHA-256 `1750aaeeaf4741d6a54e9ae4694acedce1534a6d81d8c289cb2e30312f743099`; stock `data/York_107.bin` retained for rollback. Stated recovery strategy: restore stock via Redumper; Eject-at-power-on rescue if normal boot fails, explicitly not yet verified on this 4824. No recovery intervention, separate reset, or repeat flash was needed. Lead-in speed and data+subchannel continuity are still unmeasured; next work is functional read validation, not another flash.

- **2026-11-04 York patched firmware generated:** Ran the Rust `fwpatch-plextor` on immutable `data/York_107.bin` with maintained `firmware/px-w4824a/york-107.json`. Output is workspace-root `York_107_patched.bin`, 524288 bytes, revision `P107`, checksum `0xefcb`, SHA-1 `ce7f9d33001d454fd146b7e96f453123f4ec9c11`, SHA-256 `1750aaeeaf4741d6a54e9ae4694acedce1534a6d81d8c289cb2e30312f743099`. Independent byte comparison confirms exactly eight changed bytes: two revision bytes, the four instruction bytes at file `0x6c21a..0x6c21b`/`0x6c242..0x6c243`, and two checksum bytes. Both contiguous-container and split-monitor checksum formulations agree; original SHA-1 remains unchanged. This is the first successful actual-image run of the Rust patcher. No drive access or flashing occurred; physical speed/continuity remain unverified.

- **2026-11-04 fwpatch-plextor rewritten in Rust:** `tools/fwpatch-plextor` is now a standalone Rust edition-2024 Cargo binary with CLI, patching, and file-I/O modules. Cargo manages serde/serde_json, sha1, tempfile, same-file, and anyhow; `Cargo.lock` pins dependencies. Removed the C++/CMake files, vendored dependencies/provenance, and old example directory. The maintained patch is `firmware/px-w4824a/york-107.json`, SHA-1 `e75fc1e16ed53a5e9a743ac15fdf1fe302695081`, revision `P107`, with `9c 00` -> `1a 01` at file offsets `0x6c21a` and `0x6c242`. CLI/schema and validation behavior are preserved: strict JSON fields/types/duplicates, original SHA-1/byte guards, York/Mont layout/checksum/erased ranges, no overlapping/reserved-range patches, automatic revision/checksum updates. Output always appends `_patched` in cwd; native OS paths, file-identity checks, synchronized tempfile and atomic replacement protect inputs. `cargo fmt --check` and Windows Release build with `--locked --offline` passed using Rust 1.97.1; executable: `out/agent-tests/fwpatch-plextor-rust/release/fwpatch-plextor.exe`. Per user request, no tests or test infrastructure were added or run; Linux/macOS builds and runtime behavior remain unverified. No patched binary was generated and no drive access occurred. Detailed York instruction and simulator evidence remains in `docs/YORK_107_D8_LEADIN_ANALYSIS.md`.

- **2026-11-01 York 1.07 negative-D8 buffer reuse defect, offline confirmed:** Current task is PX-W4824A **main + subchannel only**, excluding C2. York stores descriptor positions in 24 bits; `RAM:fec21a` and `RAM:fec242` zero-extend them before signed comparisons in `RAM:fec1b2`. Negative buffered positions therefore trigger queue reset/reacquisition. Canonical Mont 1.04 uses full 32-bit descriptor positions. Two same-size replacements `9c 00` -> `1a 01` (`MOVS WR0,R1`) restore sign extension in Keil S251: 16/16 stock/corrected cases pass, covering negative/positive hits, producer boundary, true misses, and empty queue. Report: `docs/YORK_107_D8_LEADIN_ANALYSIS.md`; reproducible harness: `tools/verify_york_d8_buffer_reuse.py`; successful output: `out/agent-tests/york-d8/s251-cases/report.json`. New scoped Ghidra import `/york/York_107.bin` is saved with comments and unchanged ROM hash. No flashable image, live command, flash, or reset was performed. This confirms a source of repeated acquisition overhead; its share of physical slowness and returned Q continuity remain unmeasured. First-acquisition delay may have other causes. Next: distinguish startup/per-command latency on data+subchannel; audit caller position domains before promoting the four-code-byte correction to a production patch. Fixed-language opcode `0x49` operand roles and opcode `0x1a` mnemonic are also wrong; A251/S251 establish the actual instructions. Do not trust this routine's current decompiler pointer flow or apply a global decoder repair without isolated regression.

- **2026-10-01 all stock code hooks removed, offline only:** `gofw-pxw5224a/` now leaves both stock RAM:f85de5..f85de8 and RAM:fda843..fda846 byte-identical. The C4 producer installs a dedicated coroutine-1 entry at RAM:fefa00 before acquisition; the FE C scheduler handles C4 state 2 and returns unrelated states to the stock scheduler. Removed the C4-only admission copy and its ABI bridges; C4 now passes through ordinary stock admission, without D8's fixed read-family and active-write opcode exceptions. Only the INQUIRY vendor field, C4 dispatch/media entries, FD/FE erased-gap payload, and checksum change. Keil C251/A251/L251/OH251 build reports zero diagnostics, 10/10 CTest pass, linked code remains within the FD/FE gaps with no new persistent RAM. Independent image diff finds zero changes outside declared regions, stock admission and D8 dispatch match official bytes, checksum `0xed6b` verifies. Scratch image `out/agent-tests/gofw-no-admission-hook/MONT_104_gofw.bin` SHA-256 `a53e8653b6c5de00624e451f290933e531e25f3c3df7ac204c5246b7aac067fc`. **Not flashed or live-tested.** Verify fresh READ acquisition and state transitions on hardware before relying on this route.


- **2026-09-30 admission fix confirmed by user; diagnostics removed:** User reports that the C4 stream issue is fixed after flashing the build without the non-C4 admission stop. Removed the remaining C4T3 RAM trace, throttle diagnostic counter, trace decoder/tests, and their CMake objects. Kept C4 SEEK's explicit capture stop and normal READ failure handling. Independently built `out/agent-tests/gofw-no-diagnostics-build/MONT_104_gofw.bin` (SHA-256 `4f194138b79f067c9987aae11daa322699ea6d1bfa2e40cb12efd5217ce4effb`); Keil reports zero diagnostics, 9/9 firmware tests pass, and image differences versus the user-validated admission-fix build are confined to custom FD payload and checksum. Stock RAM:fdc1ac admission bytes and D8 dispatch remain unchanged; checksum is valid. In `../redumper/cd/cd_spiral.ixx`, removed the temporary 2048-frame direct/buffered probe, timings, CSV and probe stop command, while retaining the currently active SEEK(0) current-position start and normal error continuation. All 8 SpiralDump tests pass and the Redumper executable builds with Visual Studio 2022. The diagnostic-free firmware image itself has not been flashed.

- **2026-09-30 C4 restart trigger physically identified and Bus Hound corroborated:** User flashed the `C4T2` trace revision and completed the bounded buffered 4x Redumper probe on H:. Elevated read-only F5 on idle H: saved `out/agent-tests/gofw-trace-live/spiraltest_buffer-c4-trace-cause.{bin,csv,json}`. Header reports 129 C4 READ attempts and seven unrelated SCSI packets during active capture; the latest was opcode `0x00` TEST UNIT READY, CDB byte1=0, observed after 114 READs while mode RAM:000c2a=3 and format RAM:000f06=4. Trace READ 113 began mode3/format4 and ended queue163/secondary288/producer289; READ 114 began with mode0/format4, unrelated count advanced 6->7, then reset/rearmed to queue17/secondary16/producer17. The corresponding host READ 114 returned GOOD/full 43872 bytes in 76.253ms (normal ~52.9ms), with valid Q 2783->2791 (seven missing positions). `GofwStopC4ForOtherCommand` explicitly calls `StopCapture` for any non-C4-READ CDB, which changes mode3->0; subsequent C4 READ therefore resets acquisition. The host CSV contains seven periodic long READs after initial startup (commands 2,21,40,58,77,96,114), matching the unrelated-packet count; some Q records are invalid in the middle of the run, so exact loss at every event is not established. Bus Hound text export `E:/ISO/dump/asdf.txt` shows seven host-originated USB mass-storage TEST UNIT READY CDBs interleaved with the C4 READ stream (device 82 command IDs 33,104,183,262,337,416,491). The outgoing CBWs and incoming successful CSWs prove the drive did not originate these CDBs. Two further TEST UNIT READY requests appear after the final C4 SEEK; three appear before setup. Redumper's `dump::spiral` probe loop issues no TUR, and its only TUR call for a specified drive is during initial command setup, so the periodic issuer is outside that loop (OS/service or another process, not identified). Bus Hound's text export has no process identity or timestamps. This proves the command admission stop causes the recurring restart; it does not yet prove that allowing stock TUR during capture is safe. No firmware edit or flash by the agent in the capture analysis.

- **2026-09-30 compact C4 trace live result, recurring restart identified:** User flashed the 20×16-byte record image and reran the 4x buffered probe. Elevated read-only F5 `f5 00 00 00 00 00 16 b0 01 50` on idle H: captured 20 valid records (commands 108..127) in `out/agent-tests/gofw-trace-live/spiraltest_buffer-c4-trace-compact.{bin,csv}`; header count=128, next slot=8, throttle count=1. Host `E:/ISO/dump/spiraltest_buffer.spiral-probe.csv` has seven +7 CRC-valid Q boundary jumps at READ commands 1,20,38,57,75,94,112, separated alternately by 19/18 commands and 1.034/0.980 s. All 128 READs returned GOOD/full 43872 bytes. At command 111 the trace ended with queue=163, secondary cursor=288, producer cursor=289; command 112 began with queue=0, secondary=0, controller state=0, producer active=1, and first descriptor low word reset from 9 to 0. That command took 76.194 ms versus normal median 52.876 ms and Q jumped 2733→2741 (seven missing positions). It finished queue=17/secondary=16/producer=17 and GOOD/full. Thus C4 silently reset/rearmed acquisition at the gap; this is a firmware continuity failure, not Redumper frame processing or queue-pressure throttle (zero entries in captured tail, one total for eight gaps). Source `raw_sector_range.c` resets only if `RAM:000c2a !=3` or `RAM:000f06 !=4`; trace did not capture those pre-reset values. `GofwStopC4ForOtherCommand` at stock admission would set `RAM:000c2a=0` for any non-C4-READ packet, and Redumper's probe loop issues only C4 READs; a periodic external/background packet is a strong hypothesis, not yet proven. Another one-second stock event affecting those fields remains possible. **No C4 behavior code changed in this analysis.** Next: establish which condition changes and whether an unrelated CDB reaches the admission hook before modifying stop-on-other-command policy; avoid calling the background-command hypothesis confirmed.

- **2026-09-30 C4 boundary trace second live attempt and compact candidate:** User flashed the 64×32-byte trace image and reran `E:/ISO/dump/spiraltest_buffer` at 4x. All 128 C4 READs returned GOOD/43872; CRC-valid Q jumps at commands 1,3,21,40,58,77,96,114 were +8,+7,+7,+7,+7,+7,+7,+7, with normal READ median 52.899 ms and jump READs 76.076–78.088 ms. Elevated read-only F5 `f5 00 00 00 00 00 17 f0 08 10` returned 2064 bytes; `out/agent-tests/gofw-trace-live/spiraltest_buffer-c4-trace-lowram.bin` has intact `C4T1` header at RAM:0017f0 (next=0, active=63, total=128, throttle count=1), but all 64 records at RAM:001800..001fff read zero. F5 at RAM:fded2c matched the flashed 0x1800-record candidate; RAM:fdfe60 begins its corresponding `0x1800` address calculation. Thus the trace code ran, but records at RAM:001800 did not survive/are not F5-visible. The single observed pressure-throttle entry cannot account for eight recurring gaps. Canonical 1.04 Ghidra found genuine stock direct accesses in RAM:0015f0..00169f but none in RAM:0016a0..0017ff; F5 reads there return zeros while writes to RAM:0017f0 persisted. A new compact trace candidate keeps the proven header at RAM:0017f0 and stores the last 20 16-byte READ records in RAM:0016b0..0017ef, recording queue/secondary/producer cursors, first descriptor position low16, status, controller/producer-active start state, and throttle delta. The collector reads F5 RAM:0016b0..0017ff and always saves raw bytes. Scratch firmware `out/agent-tests/gofw-trace-lowram-build/MONT_104_gofw.bin` SHA-256 `94180333e7d6f0e382de32d5cc1c552dfe5fe09054cb55ee9f201029b4950677`; C251/A251/L251/OH251 zero diagnostics, 9/9 CTest and 2 decoder tests pass, diff versus initial trace candidate confined to custom FD payload/checksum, D8 dispatch unchanged. **Compact image has not been flashed; physical writeability of RAM:0016b0..0017ef remains unverified.** Next: user builds/flashes from their own cache, runs a bounded 4x buffered probe, then agent captures F5 and aligns the final 20 records with host CSV. No drive write/reset/flash was performed by the agent.

- **2026-09-30 paired live SpiralDrive probe result:** The user ran Redumper `SPIRAL_PROBE_MODE=1` direct and `=2` buffered, 128 C4 READs × 16 frames at 4x in `E:/ISO/dump/spiraltest_direct.spiral-probe.csv` and `spiraltest_buffer.spiral-probe.csv`. Both returned GOOD/full 43872 bytes for all 128 commands. Read-only scans of the respective `.subcode` files found eight adjacent CRC-valid Q jumps in direct and seven in buffered; all occurred at the first frame of a 16-frame READ. Buffered jumps were exactly +7 at commands 7,25,44,62,81,99,118 (18/19-command intervals); direct recurring +7 jumps were commands 2,20,38,57,75,94,113. Every recurring gap coincided with that same READ taking ~75.7-76.2 ms instead of ~52.7-53.0 ms, while buffered inter-READ intervals had median 260 microseconds and no per-frame dump writes. Thus per-frame Redumper file I/O is ruled out as the cause of recurring gaps; the cause lies in a periodic device/transport capture or cursor path not yet pinpointed. Direct mode also had a separate +254 jump immediately after its first command, whose file processing took 819653 microseconds; buffered mode had no analogous startup jump. Next: one reusable firmware trace of queue occupancy/cursors and producer throttle/state at C4 READ start and at first descriptor, correlated with host CSV; avoid attributing exact firmware branch from host timing alone. No firmware edit, flash, or live command was performed by the agent in this analysis.

- **2026-09-30 Redumper SpiralDrive continuity probe added, awaiting paired live runs:** The sibling `../redumper/cd/cd_spiral.ixx` now has one compile-time `SPIRAL_PROBE_MODE` define: 0 off (default), 1 direct file writes, 2 buffered writes after capture. Probe modes cap C4 capture at 2048 frames, retain per-READ timing and CRC-valid boundary-Q records in memory, issue SEEK(0) after capture, and write `<image>.spiral-probe.csv`. Existing normal mode and the user's current-position modifications remain. Earlier temporary runtime-mode tests verified buffered output, a seven-frame Q jump and direct-mode stop; the runtime interface and these tests were removed at the user's request for one source define. Final Release MSVC build of `gtest_spiral` and `redumper` succeeded in `out/agent-tests/redumper-c4-review`, and all 8 existing SpiralDump tests pass with mode 0. No live drive commands, firmware change, or flash occurred. Next: same-speed bounded runs with mode 1 and mode 2, compare CSV boundary deltas and timings; if both gap similarly, add one firmware queue/cursor trace.

- **2026-09-30 C4 current-position READ continuity observation:** User reports that the ordinary-D8-based C4 image runs at 4x when Redumper starts at the current servo position, but its Sub-Q shift starts at +189 on the first post-anchor 16-frame READ and then grows by +7 or +8 at later 16-frame boundaries (synthetic LBAs 1048, 1352, 1640, 1944, 2232, 2536, 2840, 3128). Independent read-only scan of E:/ISO/dump/spiraltest.subcode found nine adjacent CRC-valid positional-Q jumps, all at the first frame of a 16-frame READ; the first is 919->1109 (189 omitted positions), later jumps are seven or eight positions. This confirms actual captured-position discontinuities, with no SCSI errors reported in the supplied excerpt. Redumper calls `write_entry` three times per frame; `write_entry` flushes each call, yielding 48 file flushes per 16-frame READ. Host latency/queue pressure is plausible, but neither its timing nor the exact firmware path causing each jump is established. Next: compare adjacent CRC-valid Q on both sides of command boundaries and measure READ/processing times; run a bounded Redumper capture with buffered writes to test the host-latency hypothesis before changing firmware. No drive access or code changes accompanied this observation.

- **2026-09-30 C4 ordinary-D8 READ offline candidate, physically exercised with continuity unresolved:** The user approved letting an active ATAPI host batch finish before C4 checks its inactivity timeout; an already started batch therefore remains unbounded if its completion interrupt never arrives. The custom C range keeps the stock format-8 descriptor/host-transfer poll but avoids addressed reposition and guards its empty-queue restart. BIT0013=0 selects ordinary-D8 controller events and stock selected-speed initialization; the RAM:fd11cf hook and custom event/speed copies were removed. The C4 acquisition copy at RAM:fdfd90 accepts current-pickup Q without the negative sentinel, positioner, or reset retry. A minimal guarded call-site hook at RAM:f86835 exits stock coroutine-1 recovery for C4 before its drive-control reset; D8 still executes stock reset. Queue pressure waits for host drain without request 5; a stopped producer or lost cursor fails C4 READ and the next READ rearms in place. C4 SEEK remains unchanged. Keil C251/A251/L251/OH251 build has zero diagnostics, map is inside RAM:fdeae0..fdffff with xdata=0, 9/9 host tests pass, and independent 512-KiB diff/checksum audit found 4061 changed bytes confined to payload, C4 dispatch/media, two admission aliases, SCSI admission/acquisition/recovery calls, vendor string, and checksum. Stock D8 dispatch, D8 handler bytes, and RAM:fd11cf remain unchanged. Scratch image `out/agent-tests/gofw-read-normal-vs/MONT_104_gofw.bin` SHA-256 `a5c27746cc36db587ea9ede142b5fb0a4f6ee5931d749ecf7cba609072837713`, checksum `0x1cab`. No flash or drive test of this revision. Next: user builds/flashes from their cache, then perform bounded one- and multi-frame READ, SEEK/READ, and failure-recovery tests. Keep the active-batch cancellation limitation explicit.

- **2026-09-29 C4 live stream discontinuities at default and 4?, analysis only:** The user-flashed packed-path routing revision reports no SCSI errors, but the saved `E:/ISO/dump/spiraltest.subcode` files show command-boundary Sub-Q jumps. In the default-speed capture (last modified 22:02:43; 52,801 frames from synthetic LBA -342), 44 adjacent CRC-valid Q jumps of 124..292 positions sum to 6,530 skipped Q positions; all 44 are at synthetic LBA modulo 16 = 11, the first frame of a new 16-frame READ; 50,015 other adjacent valid-Q pairs advance by one. In the subsequent 4? capture (last modified 22:10:17; 7,409 frames from synthetic LBA -631), 15 adjacent CRC-valid jumps of 39..42 positions sum to 600 skipped Q positions; every jump is at synthetic LBA modulo 16 = 10, the last frame of a 16-frame READ; their intervals are 256 or 272 delivered frames. The 4? result rejects a simple high-speed-only explanation; host throughput is not proven either way. A deterministic C4 producer/host-transfer handoff, capture pause/restart, or queue-pressure path is more plausible than random Sub-Q corruption. The C4 range validates descriptor count/status but not physical Q continuity, so full GOOD transfers can contain these jumps. `GofwAlternateStepThrottle` issues request 5 with the current-pickup sentinel near queue capacity, but no live queue occupancy or state-transition trace ties that branch to these exact jumps. Redumper seeks/writes/flushes each of three files per frame, which could contribute latency, but 4? data prevents treating it as the established cause. Next: correlate descriptor queue counts, controller state, and request-5 transitions with one bounded jump, then remove the actual pause/restart cause while preserving the no-Q-analysis design. No firmware or Redumper source, drive, or original capture changed in this analysis.

- **2026-09-29 C4 packed-path escape fix, user-flashed with no descriptor errors in the latest bounded run:** The descriptor-origin run reported five `05/24/00` descriptors at synthetic stream LBAs 75180..80684. Canonical 1.04 stock packed-subchannel processing RAM:f93068 creates these; this is not C4 CDB rejection or proof of bad disc Q. The C4 event selector now uses RAM:000f0b bit 2 as a persistent stream marker alongside mode3/format4/state6/7, instead of relying only on the last CDB. A new guarded stock ECALL patch at RAM:fdc1ac routes SCSI admission through an FD payload gateway that calls `GofwStopC4ForOtherCommand` after CDB receive and tail-enters stock RAM:fda843; the event selector also stops C4 if an active-operation path overwrites the CDB. C4 SEEK, including zero distance, clears the marker and BITS:0013 before movement. The C4 event bridge no longer contains a branch into the packed-subchannel routine. No new persistent RAM is allocated; Keil links FD payload through RAM:fdffa6 with data=14, xdata=0 and zero diagnostics. All 11 host tests pass; independent image diff/checksum check in `out/agent-tests/gofw-stream-route/verify.py` confirms only vendor/dispatch, the new admission call, existing callsites, payload and checksum changed, with stock D8 bytes unchanged. Generated image SHA-256 `ee78aaf144d88642e944f174ab58f96069e2f98612821a70b63446c624fd8566`, checksum `0x373c`. This is an offline fix for the observed routing vulnerability, not proof of the exact initial interrupt trigger. User subsequently flashed this revision and the latest bounded dump has no reported SCSI descriptor errors, but has command-boundary Sub-Q continuity gaps described above. C4 still uses the older packet-based acquisition startup and negative-mode sentinel; READ timeout remains unenforced and an unbounded disc-end dump may hang. Next: user rebuilds/flashes from their cache and validates bounded repeated READs, SEEK, and unrelated-command transitions before simplifying acquisition/timeout paths.

- **2026-09-29 C4 descriptor failures and disc-end investigation:** The user rebuilt the descriptor-sense revision in their CMake cache (SHA-256 `381353937e47d6c8f824cd0ad08259419e6975ac47632dda2a8d156c8b106a1b`) and supplied a `dump::spiral` log with 63 C4 READ failures at synthetic stream LBAs 70064..241440; every failure is `04/80/04`, proving it reached C4's failed-descriptor reporter rather than CDB rejection. The original descriptor status is no longer retained in sense, so packed-path origin remains an inference. The drive later became unresponsive, and the user rebooted it. On the same disc, read-only H: INQUIRY, formatted TOC, and READ CAPACITY returned GOOD: formatted lead-out LBA 35316 and capacity last LBA 35315 (`out/agent-tests/gofw-c4-endcheck-20260929/report.json`, `report_more.json`). Analysis of the existing `E:/ISO/dump/spiraltest.subcode` (288622 frames, synthetic LBA -45150..243471) found the CRC-valid Q transition from track 01 to `AA` at synthetic stream LBA 37947; its absolute Q `07:52:66` maps exactly to TOC LBA 35316. All 204186 valid positional Q frames in the 205525 captured frames after that point through the last saved frame are track `AA`; 1339 are invalid/nonpositional (`subcode_leadout_scan.json`). The last Q maps to LBA 254653. Thus this run continued deep into the disc's lead-out with no observed second session. Reaching an unreadable outer area plausibly caused the final hang, but the exact stopping condition was not captured. Redumper does not stop on `AA` and C4's pending-poll loop does not enforce its 5000-ms CDB timeout. Next work: bounded READ behavior and a host-side end criterion, then investigate the descriptor source separately. No firmware source change, flash, reset, or C4 READ/SEEK by the agent in this investigation.

- **2026-09-29 C4 descriptor-sense build:** `gofw-pxw5224a/src/c4_sense.c` reports failed descriptors as `04/80/04`, distinguishing them from C4 invalid CDB `05/24/00`; producer table sense, host cancel, generic stop, terminal short, SEEK, and stock D8 paths remain as before. Keil linked RAM:fdf65b..fdf6e1 with zero diagnostics and xdata=0; the relevant tests passed for first-frame and 11/16 mid-batch descriptor failures. Generated firmware `out/agent-tests/gofw-c4-descriptor-sense/MONT_104_gofw.bin`, checksum `0x6e54`; compared with the preceding flashed diagnostic image, all 151 changed bytes are within the FD C4-sense payload or checksum. The previous diagnostic image was confirmed on H: by read-only F5 code readbacks at RAM:f8420a and RAM:fdf65b (`out/agent-tests/gofw-c4-sense-readback-20260929/report.json`); no byte readback of the latest descriptor-sense flash was taken.

- **2026-09-29 C4 state-6 controller-profile change, user-flashed and provisionally working:** The user reports the revision works with Redumper at 4x, 16x, and default `<optimal>` speed; previously the aligned-cursor revision worked at 4x but hung the drive at default speed. No exact run lengths, CDB trace, or image readback were supplied, so long-run stability and causality remain open. Redumper sends `SET CD SPEED` with about 705 KB/s for 4x and `0xffff` for default. Canonical stock `ProcessControllerReadEvent` RAM:f91137..f9115c writes `0x87` to RAM:000efc/controller 0x8005 on state 6 only when BITS:0013 is clear; C4 sets that bit and previously skipped the writes. The revision routes C4 state 6 through its C event copy, performs those two ordinary-D8 writes and the unchanged RAM:000f75/RAM:000f04 updates, then uses the existing state-7 path. The speed policy, capture sentinel, SEEK, and D8 are unchanged. Keil links RAM:fdeae0..fdffa6, xdata=0, zero diagnostics; 11/11 host tests pass. Scratch image `out/agent-tests/gofw-normal-read-state6/MONT_104_gofw.bin`, SHA-256 `a32e282691fa18db1aba521e3a8d37cef2670087066098af57e9ec449a2858f4`, checksum `0xd47d`. Versus the aligned-cursor image, 301 bytes change only in FD custom payload and checksum. Next verify sustained default-speed streaming before another code revision; queue/restart and missing READ deadline remain unresolved. No agent flash/reset.

- **2026-09-29 C4 normal-D8 frame-readiness checkpoint, offline verified, awaiting user test:** Following the user's request to move C4 toward ordinary D8 capture without Sub-Q/LBA acceptance, the first isolated source change adds the ordinary D8 state-7 main-cursor `mod 0x930` gate for speeds below 52x when the slot has not faulted. `gofw-pxw5224a/src/controller_read_event.c` returns a distinct defer code; the minimal `stock_calls.a51` hook jumps to stock RAM:f9139e (`LCALL RAM:f93429; LJMP RAM:f91426`), preserving stock countdown/descriptor-fault handling. The current-pickup route, sentinel, BITS:0013, selected-speed clone, SEEK, and D8 remain unchanged. CMake shifts two custom FD payload segments to make room; Keil code occupies RAM:fdeae0..fdffa6 without overlap or xdata, with zero diagnostics. All 11 host tests pass, including aligned/unaligned, faulted-slot, and 52x cursor cases. The independently generated 512-KiB image is `out/agent-tests/gofw-normal-read-align/MONT_104_gofw.bin`, SHA-256 `49972aa7442f1ebfd7d9c06a7668135befdcc90a4ea69787477cb997ef455f92`, checksum `0xf1e0`; 1365 bytes differ from the flashed stage-4 image, all in the custom FD payload or checksum. Stock D8 bytes and all original code outside existing hook ranges are unchanged. Runtime behavior remains untested; the prior stage-4 hang is not explained. Next checkpoints are controller setup, C4 empty-queue coroutine restart, READ deadline, and eventual sentinel/mode decoupling, one isolated revision at a time.

- **2026-09-29 C4 stage 4 speed, user-flashed and unstable:** The user reports one or two successful runs followed by a hard drive hang requiring a restart. They explicitly declined rollback pending discussion; stage-4 source remains in place, and the agent sent no drive commands. The C speed copy at `gofw-pxw5224a/src/alternate_raw_speed.c` removes the special-mode 4x cap while retaining the other stock limits. Offline build had zero diagnostics, 11/11 host tests, and image SHA-256 `347d264e61e2e53eba951b19edc2ba6b001e1e679e376aaa9c98eb553288afba` in `out/agent-tests/gofw-stage4-speed/`; runtime selected speed and exact hang point are unknown. Comparison with ordinary D8 in canonical 1.04 found three concrete C4 divergences to investigate: C4 BITS:0013 bypasses the normal D8 state-7 main-cursor modulo-2352 gate at RAM:f91327..f913a1 and the similar downstream gate in RAM:f92d0b, while its event copy gathers immediately; the shared empty-queue helper at RAM:fc683c can restart stock coroutine 2 at RAM:f929ab instead of C4's copy; and C4 READ does not enforce its CDB timeout while polling. C4's sentinel acquisition, current-pickup route, and no-position-filter design also differ intentionally from normal D8. No single hang trigger has been proven; get exact last console/CDB or read-only post-hang state when safe, then fix/test one divergence at a time. No firmware source changed during this comparison.

- **2026-09-29 C4 incremental stage 3, count-only revision, user-flashed with intermittent errors:** C4 READ now passes only a frame count to `GofwExecuteRawSectorReadRange`; the fake `-19999` start LBA is removed. On a fresh stream, C4 initializes RAM:001598 to zero as a descriptor sequence, preserves that sequence and the descriptor cursor on a healthy subsequent READ, and does not compare it with RAM:0012c1 in its C producer/error helpers. C4 continues setting BITS:0013 and requesting internal acquisition target `0x10000000` to retain the known working physical startup and raw descriptor flow. Stock 1.04 Ghidra shows that BITS:0013 bypasses LBA position gates in `ProcessRawSectorBufferEvent` (RAM:f92d0b), which increments RAM:001598 after enqueue; `CountValidatedRawReadDescriptorsForHostTransfer` (RAM:fc65f4) only compares descriptor position to host position when BITS:000c is set, and C4 clears that bit. The C4 event hook already forces raw gather rather than the stock RAM:001598 > -1000 packed path. Keil C251/A251/L251/OH251 builds with zero diagnostics; 9/9 host tests pass; map stays within RAM:fdeae0..fdffff with xdata=0. Against the user-flashed first stage-3 image, all 937 changed bytes lie in C4 payload RAM:fdeae0..fdffff and checksum at file `0x79ffe..0x79fff`; stock D8 and fixed data are unchanged. Independent checksum `0xc205`, image SHA-256 `fb465ea7ec205f4607c20a92f1438f613dda8c7b1e48c439393f64c07b0e0229`, scratch build `out/agent-tests/gofw-stage3-count-only/`. The user flashed this revision and reported a dump with intermittent CHECK CONDITION `05/24/00` and large stream-LBA/Q-LBA divergence. The stock descriptor builder still records RAM:001598 as a sequence, and the stock queue-preparation empty/inactive path can restart the stock F9 coroutine; those inherited behaviors remain for later analysis. No agent drive commands, flash, or reset.

- **2026-09-29 C4 incremental stage 3, first revision, user-reported working:** The C4-only producer in `gofw-pxw5224a/src/alternate_raw_step.c` now always requests acquisition target `0x10000000` with adjustment 0, independently of BITS:0013 and the virtual target RAM:001598. Keil listing shows DR4=0x10000000 and DR0=0 before `CallAlternateStepRequest`; this is identical to the former BITS:0013=1 branch that C4 currently uses. The sentinel, C4's setting of BITS:0013, capture startup, descriptor checks, virtual target bookkeeping, and speed cap remain intact for this revision. New host tests exercise the request with the bit set and clear and both negative and positive virtual targets. C251/A251/L251/OH251 built with zero warnings/errors; 9/9 tests pass. The producer segment shrank from RAM:fdef00..fdf012 to RAM:fdef00..fdeffe with xdata=0 and no linker overlap. Compared with the user-tested stage-2 image, only bytes `0x5ef1e..0x5ef1f`, `0x5ef2e..0x5f012`, and checksum `0x79ffe..0x79fff` changed; stock D8 and fixed data are identical. Independent checksum `0x9cfc`, image SHA-256 `c274bd165885c77fecb82490f8929c88186d3d20259c6b6d0c3bcbcbf0a682bc`, scratch build `out/agent-tests/gofw-stage3-capture-choice/`. The user subsequently reports flashing this revision and that it works; exact CDBs and firmware readback were not supplied. No agent flash or reset.

- **2026-09-29 C4 incremental stage 2, user-reported working:** The user reports flashing and testing the stage-2 image successfully; exact exercised CDBs and a failure/recovery trace were not supplied, so fault recovery remains unconfirmed on hardware. The code requires RAM:001459 completed frames to equal RAM:001455 requested frames before GOOD; an incomplete transfer clears the queue, installs stock idle coroutine 2, and returns CHECK through the C4 caller without stock recovery movement. Eight host tests passed, and offline image SHA-256 was `6f00ef2183474343d12c5468777210b4fc91e783d7bb942f3d5e327b21f1b304` at `out/agent-tests/gofw-stage2-failure/`. Stock D8 and fixed data were unchanged. No agent flash or reset.

- **2026-09-29 C4 format-4 main cursor correction, live verified on H::** The user flashed the one-byte cursor change (`ControllerCursor(0x8008u)` to `ControllerCursor(0x800bu)`) in `gofw-pxw5224a/src/controller_read_event.c`. Read-only INQUIRY identified `PLEXTOR CD-R PX-W5224A 1.04`, `SpiralDrive v1.0.0`; F5 readback `f5 00 00 00 00 fd f8 40 00 20` matched the corrected image at RAM:fdf840..fdf85f. Eight consecutive one-frame C4 READs (`c4 02 13 88 00 00 00 01 00 00 00 00`, DATA IN 2742, 15 s host timeout) all returned GOOD/full with valid Q for LBAs 945..952 and nonzero main data. A single stock D8 format-8 read `d8 00 00 00 03 b1 00 00 00 08 08 00` (DATA IN 21936, 15 s host timeout) returned GOOD/full; each of its eight 2742-byte frames matched the corresponding C4 frame **byte for byte**, including main data, C2, and subchannel. This verifies main/Q association and continuity for this clean data-track interval, not damaged-disc C2 or long runs. Exact CDB/status/captures and comparison: `out/agent-tests/gofw-main-cursor-fix/live/`. Prior stage-one image had Q LBA 913 paired with D8 LBA 912 main and Q LBA 914 with zero main despite D8 LBA 914 nonzero. Canonical stock disassembly RAM:f91310..f913c4 proves format-4 normal-path cursor RAM:01800b..01800d; 018008..01800a is selected only for other formats. Offline Keil build passed with zero warnings/errors and 7/7 host tests; generated image differs from stage-one by one code byte at file offset `0x5f84a` (RAM:fdf84a, immediate `08→0b`) and checksum byte `0x79fff`, independent checksum `0x97c6`, SHA-256 `42704269197cd3bcdc19e1b634bee05b5434bb68cd505eb3ef797d33bc979992`. The agent sent no flash or reset command.

- **2026-09-29 C4 incremental stage 1, offline verified:** The user reports flashing and verifying the restored current build; its flashed binary was not read back. Stage 1 removes only the accepted-packet Sub-Q position-estimate rejection in `gofw-pxw5224a/src/acquire_position.c`. Keil C251/A251/L251/OH251 builds with zero warnings/errors; 7/7 host tests pass, including the formerly rejected condition and packet failure. Image `out/agent-tests/gofw-stage1-subq/MONT_104_gofw.bin` has SHA-256 `3c0d6117832ef1ceb49f72debe1014eb7e8b32c05b4b592c0b309606829fceb8`. Against archived baseline, changed bytes are only in the C4 acquisition payload interval at image offsets `0x5fe27..0x5ffdd` (including the now-erased old tail) and checksum. The new function ends at RAM:fdffa6; no new RAM or stock D8 changes. Independent checksum is `0x97c3`. The user later flashed this revision; the live results are summarized above. The agent did not flash it.

- **2026-09-29 C4 stream rewrite rolled back:** The user flashed both the initial pickup-and-stream-control revision and its offline startup/abort repair; dump::spiral hung at its first one-frame C4 READ (c4 02 13 88 00 00 00 01 00 00 00 00), with fast spinning and an unresponsive drive until power-cycle. The precise hardware fault remains unproven. Per user request, gofw-pxw5224a/ source, CMake layout, and patcher have been restored to the immediate pre-plan FD-only position-port revision. The failed source is archived at out/agent-tests/gofw-stream-failed-source/. A fresh Keil payload is byte-identical to out/agent-tests/gofw-position-port-vs/keil/c4.hex; the freshly generated 512-KiB image is byte-identical to the archived image, SHA-256 f8e259996eaa08b80417434e088f8a001112350c76de9fd11dee9c05a377ceb0. Six host tests pass. At rollback time that revision had only offline verification; the user subsequently reported the restored current build working after flash. No drive commands or flash were sent by the agent. Next work should be one small C4 READ change at a time, with a fresh offline image diff before the user decides to flash.

- **Payload placement after rollback:** Active code and ABI bridges fit only in stock 1.04 erased RAM:fdeae0..fdffff. The FE tail remains available for future experiments but is not used or written by this revision. Stock D8 paths and fixed data remain unchanged by the restored image.

- **2026-09-28 FD-only C4 position-acquisition port, offline verified:** `gofw-pxw5224a` no longer patches stock RAM:fc858f or its near call at RAM:fc83c7. A complete four-byte ECALL at RAM:f865ba now targets an FD-resident selector: ordinary/D8 commands tail-enter untouched stock `AcquireDiscReadPosition` RAM:fc818b; C4 READ tail-enters `GofwAcquireC4Position` at RAM:fdfd90. The C copy specializes the CD/sentinel path, retains stock packet acquisition/recovery leaves, and accepts current pickup position. Its small radial/recovery/mode helpers are C at RAM:fdf020; all new code stays in erased RAM:fdeae0..fdffff, with no new persistent RAM. S251 selector probes reached the expected two entries with identical inherited R11/R15 and far-call SP, while C4 cleared pending nudge. Keil C251/A251/L251/OH251 build has zero warnings/errors, 10/10 host tests pass, and independent 512-KiB image diff has 0 changes outside declared ranges; original RAM:fc818b..fc859a is byte-identical. Scratch image SHA-256 `f8e259996eaa08b80417434e088f8a001112350c76de9fd11dee9c05a377ceb0` at `out/agent-tests/gofw-position-port-vs/`. **No hardware test or flash of this revision.**

- **2026-09-28 C4 SEEK uses the stock divided Timer0 countdown:** `gofw-pxw5224a/src/c4_handler.c` now snapshots 16-bit RAM:001200 at SEEK start and tests its modulo difference against the CDB millisecond timeout rounded up to stock 10-Timer0-tick units after each move chunk. It no longer uses CDB bytes 8..11 as a 32-bit interrupt counter. `stock_calls.a51` no longer exports `GofwTimerTick`, and the patcher leaves original RAM:fd134b..fd134e bytes untouched. Canonical Ghidra confirms RAM:fd16fc..fd1702 decrement RAM:001200 every Timer0 phase2; writer xrefs are operating-code, calibration, and scaled-radial paths, with no identified direct call from C4's unscaled movement routine. Read-only elevated F5 on physical H: (`PLEXTOR CD-R PX-W5224A 1.04`, `SpiralDrive v1.0.0`) saw RAM:001200 fall 320 over 3.219 s and RAM:0011f4 rise 3, matching ~10 ms and ~1 s units. C4 SEEK zero, +1/-1, and +50/-50 all returned GOOD with no data; these short moves did not span a full 10-ms tick, so they do not validate long-move timer ownership. Exact CDBs and samples are in `out/agent-tests/gofw-stock-timer-live/report.json`. Scratch Keil C251/A251/L251/OH251 build at `out/agent-tests/gofw-stock-timer-build/` has zero warnings/errors, nine CTests pass, map stays within RAM:fdeae0..fdffff with xdata=0, and generated 512-KiB image SHA-256 is `ba816854bd760bf82773a1ec8a675dbba74c1a79931f246abba2c6f27a9b5675`; stock timer-store bytes `7ab31232` and checksum `0x1f77` are preserved/valid. This new image has **not** been flashed. Remaining limitation: RAM:001200 is a shared stock countdown; a concurrent stock reinitialization during SEEK has not been ruled out on hardware.

- **2026-09-28 C4 alternate coroutine, producer, and helper C ports:** `gofw-pxw5224a/src/alternate_raw_coroutine.c`, `src/alternate_raw_step.c`, and `src/alternate_raw_helpers.c` implement the C4-used stock 1.04 control flow from RAM:f929ab, RAM:f92a69, RAM:f90b91, RAM:f92b52, RAM:f93429, and the latter's near callee RAM:f934d7. The raw-range C routine initializes the C coroutine at RAM:fdee40, which calls the C producer at RAM:fdef00; the four helper C routines occupy RAM:fdf860..fdfab7. The helper ABI adapter saves inherited DR12, and unchanged lower-level stock functions retain their original controller and descriptor work through FD-bank far-return adapters. The former three `LCALL; ERET` gateways at RAM:f9ffd6..f9ffe1 are gone; those original erased bytes remain untouched. The C4 raw-range clone also makes the stock bounds validator RAM:fd709c unreachable for C4; its former opcode alias hook at RAM:fd70be has been removed, leaving the original four bytes intact. Only the two admission opcode reads remain aliased. No new persistent RAM was allocated. Keil C251/A251/L251/OH251 built with zero warnings/errors; nine host CTests pass. Independent byte audit found changes only in declared regions, stock C0/D8 dispatch and the original F9 functions unchanged, and checksum `0xfcd8` valid. Scratch image SHA-256 `7a641e05c71a2ee5849d2e1516234e4d02ea9b2e40d267dc8b1ccc457149b565` in `out/agent-tests/gofw-range-clone/`. **No hardware test or flash was performed.**

- **2026-09-28 C4 raw-range C port:** `gofw-pxw5224a/src/raw_sector_range.c` now implements the C4-reachable control flow of stock 1.04 `ExecuteRawSectorReadRange` (RAM:fd6cf2), and `HandleScsiOpcodeC4` calls it directly. It retains the stock lower-level progress, queue reset, coroutine-2 initialization, transfer poll, acquisition dispatch, yield, and no-sense routines through small ABI bridges. The former C4-only patch at stock RAM:fd6ea6 is removed; C code preserves the next descriptor cursor on reuse. This is deliberately specialized to C4's format 8, positive frame count, and bit-state contract, not a generic replacement for all D8 modes. A scratch Keil build produced a 512 KiB image SHA-256 `a14840f654bef3aa2fe3a9356eac0f5dce5db73057348cb3238aa7fc5fa429b2` in `out/agent-tests/gofw-range-clone/`; linker reported zero warnings/errors and the clone occupies RAM:fded20..fdee22 with no new persistent RAM. Six host CTests pass. Independent byte audit found only declared patch regions changed, stock RAM:fd6ea6 and C0/D8 dispatch unchanged, and checksum `0x7feb` valid. **Hardware behavior of this candidate is untested; user builds/flashes from their own CMake cache.**

- **2026-09-28 SpiralDrive C4 short READ investigation and offline fix:** A user-reported 16-frame READ near physical LBA 18,701 returned GOOD/11 frames, then later READs returned zero until disc reload. In the post-failure F5 capture `C:\Users\G\AppData\Local\Temp\spiral-stalled-20260927-elevated\report.json`, RAM:000f4e=39, RAM:000f2e=5, RAM:001598=150, RAM:000c2a=3, and the raw queue is empty. Canonical stock 1.04 RAM:f912cc switches a special read to `ProcessPackedSubchannelRecordBatch` once RAM:001598 exceeds -1000. C4 starts that virtual target at -20003, so frame 19,004 crosses the threshold; the observed 11/16 split is consistent with the command starting at -1010. At RAM:f933e8..f93421 the packed path's miss counter >32 sets target 150 and enqueues a `05/24/00` descriptor. `ValidateRawSectorDescriptorStatus` RAM:fc5364 rejects nonzero descriptor status in mode 3, and `ExecuteRawSectorReadRange` RAM:fd6f65..fd6fd0 can return 0 without sense for an in-range failure. The C4 wrapper ignored that result. Stock failure resets the queue and installs idle coroutine2, while C4's unconditional cursor hook skipped reinitialization on later READs. The first frame-12 descriptor was not captured, so the exact initial trigger remains a strong inference, not direct observation; nonzero C2 bytes alone are not descriptor failure.
- `gofw-pxw5224a/` now keeps its C4 state-7 event copy on the raw gather path for every virtual target, checks the stock raw-range return and RAM:001459 completed count, and on incomplete READ sets the existing `05/24/00` sense while clearing mode and special-read bit to force fresh acquisition on the next READ. Host tests cover the -1000 boundary across 16 frames, a failed 11/16 transfer, and the following READ starting in mode 0. Keil C251/A251/L251/OH251 built a 512 KiB image SHA-256 `54fe63c8e8da8175b7f69e255b054257c510469c6cd8e353aaa4fb9a63610dba` in `out/agent-tests/gofw-short-read-fix/`; five CTest cases pass. Independent byte audit found only declared patch regions changed, stock C0/D8 dispatch intact, and valid checksum `0xf37d`. The map keeps C4 code in RAM:fdeae0..fdffff with no new RAM allocation. **Hardware behavior of this candidate is untested; user builds/flashes from their own CMake cache.** The host's transferred byte count can report completed frames as `bytes/2742`; sense remains generic under the established C4 contract.

- Read-only review of in-progress `../redumper` SpiralDrive changes (2026-09-27): MSVC/CMake Release build succeeded in `out/agent-tests/redumper-c4-review/`. The new CDB12 struct and endian conversions match C4's `C4 op timeout[2] value[4] zero[4]` wire layout on this Windows build; normalized INQUIRY vendor text `SpiralDrive v1.0.0` parses as version 1.0.0. `cmd_spiraldrive` is currently uncalled, so normal CD reads still use D8. Before integrating sequential READ, require `2742*count` length, retain `sendCommand` transferred length rather than `.first` alone, and use bridge-safe chunks (live H: rejected 32-frame/87744-byte Windows SPTD, accepted 16-frame/43872-byte). The existing `--force-omnidrive` override would spoof OmniDrive even on SpiralDrive and route generic CD reads to stock unsupported C0. Current firmware's CDB timeout is enforced in SEEK but not in READ; a host reader must not treat it as a READ inactivity deadline. No Redumper source edits or hardware actions were made in this review.

- The standalone C4 firmware's default 20-byte INQUIRY vendor-specific field is now `SpiralDrive v1.0.0  `. CMake migrates an existing cache value of `superg v1.0.0` to the new default while retaining other explicit overrides. A scratch rebuild from the old cache produced a 524288-byte image SHA-256 `e596c96eea882f1e78b65baa394c7f0768eb7a05a716c974fea035744b8397c6`; the patcher test passed and the separate `PLEXTOR MONT` identity remained intact. The user builds and flashes from their own cache; no drive action was taken.

## 2026-09-27 C4 C2 physical scan

- User inserted a damaged disc into H: and requested a sequential read until the first nonzero C2 vector. Read-only INQUIRY identified `PLEXTOR CD-R PX-W5224A 1.04` with `superg v1.0.0`; READ CAPACITY(10) reported last LBA 105206, 2048-byte logical blocks. After a GOOD D8 LBA0 read, stock D8 LBA -20149 positioned in lead-in, then C4 SEEK 0 cleared the descriptor queue. One C4 frame and 27 consecutive 16-frame C4 READs all returned GOOD/full transfer. Windows rejected a 32-frame (87744-byte) SCSI pass-through request with WinError87, so the scan used 16-frame requests (43872 bytes). Exact commands, status, and frame hashes are in `out/agent-tests/gofw-c2-scan/`.
- The first nonzero C2 vector occurred at zero-based lead-in scan frame 420 (421st frame), in the 27th 16-frame READ: 3 nonzero bytes in the 294-byte C2 area, at offsets 144=`80`, 150=`0c`, 152=`02`. The command returned GOOD/43872. The affected frame's 2352-byte main area was all zero and its Q `40 00 a2 10 39 16 00 23 24 57 4d 83` had invalid CRC; neighboring lead-in Q records had valid CRCs. This confirms the C4 path can return nonzero C2 bytes on this disc, but does not yet prove C2 correctness for damaged user-data sectors. The scan stopped at this first hit as requested; `hit_batch.bin` and `hit_context.json` retain the evidence.

Last refreshed: 2026-09-28

## Standalone C4 patch project

- `gofw-pxw5224a/` is an independent CMake/Keil C251 firmware patch project. The current C4 interface accepts only operation 1 (relative SEEK) and operation 2 (READ); operation 0 (former HOME) returns the existing stock 05/24/00 error. The HOME C routine, SFR declarations, diagnostic sense trace, exclusive ABI bridges, cache option, and tests were removed. Stock C0 and D8 dispatch remain untouched. D8 negative-range acquisition never used HOME; it moves inward in 200 scaled radial units until it sees confirmed lead-in Q TNO 0.
- Proven prior READ baseline: `out/agent-tests/gofw-d8-clone-vs-audit/MONT_104_gofw.bin`, SHA-256 `3da82d83587b6b361981ab6a4c904526fc6f958627f99ffac96398db543df6b3`. User flashed it; after a D8 LBA 120000 read, C4 READ completed GOOD with 2742 bytes in 3.032 s. Direct D8 negative-range read also completed GOOD with 2742 bytes in 3.031 s. Both returned zero main/C2 and nonzero subchannel. Exact commands and captures: `out/agent-tests/gofw-d8-baseline-live/summary.json`.
- **Faulty flashed no-seek image:** SHA-256 `405b0ce8e1a49059ae1a7248e4449b8a7b5d6cb8d45569ed375e8f94fffaed9b`, retained as `out/agent-tests/gofw-no-seek-read/MONT_104_gofw_INVALID_do_not_flash.bin`. Its patch at RAM:fc83c7 replaced a three-byte `LCALL` and the first byte of the following `MOV R14,ACC` with a four-byte `ECALL`. The resulting instruction stream is corrupt; an additional FD-to-FC near-call bridge in that candidate was also invalid. User reported the drive hangs after flashing. No C4 command was sent in this turn. H: was absent from the sandbox drive list; elevated PnP/CIM still identified PX-W5224A as H: with `MediaLoaded=False`. No further live commands or resets were sent. User was asked whether protected Eject-at-power-on recovery and stock restore succeed.
- **Corrected candidate flashed by user and live-tested:** `out/agent-tests/gofw-no-seek-read-fixed/MONT_104_gofw.bin`, SHA-256 `b75d8d0a02f21fd0f7ff16421443f455a0b9e08376dc9861c5e18ca67019c513`. The stock acquisition call at RAM:fc83c7 and its `MOV R14,ACC` remain byte-identical. A four-byte `EJMP` replaces complete `PUSH R15; XRL B,B` prologue at RAM:fc858f. An FD-bank hook consults C `GofwUseCurrentPosition`: C4 READ clears the pending target nudge and returns 1 through stock FC-bank `POP R15; RET`; all other commands replay the prologue then jump to stock RAM:fc8593. This keeps the D8 sentinel capture/descriptor path while skipping its inward 200-unit positioning decision for C4. The handler and C4 SEEK are unchanged. Linked byte checks enforce both paths and original near call/return boundaries; Keil builds with zero warnings/errors, four host tests pass, 512 KiB image/patch ranges/checksum pass an independent audit, no new RAM allocated. Focused S251 runs separately confirmed stock D8 and patched D8 have identical ready-position return state (`PC=fcfffc`, `ACC=1`, `R15=0x5a`, `SP=0x60`, pending nudge=2), while patched C4 returns the same registers with pending nudge cleared. Results: `out/agent-tests/gofw-no-seek-read-fixed/sim-position/results.json`. An additional synthetic D8 movement probe stalled in the simulator and was terminated without a result; it is not a comparison or a claimed firmware failure. READ timeout and C2 correction remain future tasks.
- **2026-09-27 live result on H:, corrected image:** read-only INQUIRY identified PX-W5224A 1.04 with `superg v1.0.0`; F5 readback matched the corrected image at RAM:fc858f, fc83c7, and fdfc10. One-frame C4 READ `c4 02 13 88 00 00 00 01 00 00 00 00`, DATA IN 2742, host timeout 15s, returned GOOD/2742 in 1.047s. Main 2352 and C2 294 were all zero, 22 of 96 subchannel bytes nonzero. D8 LBA120000 one-frame read `d8 00 00 01 d4 c0 00 00 00 01 00 00`, DATA IN 2352, host timeout 30s, returned GOOD/2352 with nonzero data in 4.062s. C4 READ again returned GOOD/2742 in 2.032s, but main and C2 stayed all zero (30 nonzero subchannel bytes). C4 relative SEEK +50 `c4 01 13 88 00 00 00 32 00 00 00 00`, no data, host timeout 15s, returned GOOD in 0.015s. Immediate C4 READ returned GOOD/2742 in 0.000s with exactly the same SHA-256 `c74e8e90...` as pre-SEEK C4, suggesting stale buffer reuse; fresh acquisition/physical movement is not established. Final INQUIRY and three F5 readbacks remained GOOD. Exact records and 2742-byte captures: `out/agent-tests/gofw-no-seek-read-fixed-live/`. Do not describe this as validated current-position streaming. Next: trace the stock descriptor queue/seek invalidation and find why C4 serves zero main/C2 and an identical post-SEEK frame before further flashing.
- **Flashed descriptor-cursor image, live-tested 2026-09-27:** The user built and flashed the current gofw-pxw5224a/ source from their own CMake cache. The scratch image out/agent-tests/gofw-cursor-keep/MONT_104_gofw.bin SHA-256 1b7ffe4c...fadedbbbc31 has matching RAM:fd6ea6 and RAM:fdfc10 bytes by read-only F5, though the full user image hash was not checked. C4 READ always passes fixed virtual start -19999; a C4-only bridge at stock RAM:fd6ea6 skips re-positioning RAM:fc5af5 so RAM:001298 remains at the next descriptor; SEEK sets RAM:000c2a=0 for reset. On H:, INQUIRY and F5 were GOOD. Two one-frame C4 READs (c4 02 13 88 00 00 00 01 00 00 00 00, DATA IN 2742, host timeout 15s) both returned GOOD/2742; decoded sub-Q had valid CRC and absolute MSF 00:14:33 then 00:14:34. C4 SEEK +50 (c4 01 13 88 00 00 00 32 00 00 00 00, no data) returned GOOD in 0.016s; two following READs returned GOOD/2742 and valid-CRC Q absolute 00:01:15 then 00:01:16. F5 after the fourth read showed RAM:001294=0, RAM:001296=506, RAM:001298=2, consistent with two completed descriptors after queue reset. Captures, CDBs, exact status, and decoder: out/agent-tests/gofw-cursor-keep-live/analysis.json and neighboring files. This physically validates ordered Q progression within each pair, not physical SEEK distance/direction. A further two-frame C4 READ (c4 02 13 88 00 00 00 02 00 00 00 00, DATA IN 5484, host timeout 15s) returned GOOD/5484 in 0.000s. Its two valid-CRC sub-Q records were 00:01:17 and 00:01:18, continuing exactly from the previous single-frame READ at 00:01:16. All six captured frames had zero main-data (2352) and C2 (294) regions, so full frame capture is still broken. Stock RAM:f912cc producer target also crosses its -1000 switch after about 19k frames; empty-queue continuity and long capture remain unverified. No further hardware commands were sent.
- **Further live multi-frame continuation test:** Two back-to-back C4 READ commands, each requesting two frames (`c4 02 13 88 00 00 00 02 00 00 00 00`, DATA IN 5484, host timeout 15s), both returned GOOD/5484. Decoded sub-Q absolute MSF was `00:01:19`, `00:01:20` in the first command and `00:01:21`, `00:01:22` in the second, continuing from the prior `00:01:18`; all four Q CRCs were valid. All four main-data and C2 regions were zero. Exact records: `out/agent-tests/gofw-cursor-keep-live/read_pair_a.json`, `read_pair_b.json`, and `analysis.json`.
- **Flashed relocated C4 read-event image, live-tested 2026-09-27:** `gofw-pxw5224a/` leaves stock `ProcessControllerReadEvent` RAM:f90d22..f9142c byte-identical. Its sole interrupt callsite RAM:fd11cf redirects to a small FD-bank adapter. D8 and all nonselected events restore ACC/PSW and enter the original function. Only active C4 READ, state 7, internal format 4, negative mode, and clear early-abort bit enter `src/controller_read_event.c`, which copies the stock entry and state-7 slot/cursor work in C, reads the separate main cursor at controller RAM:018008..01800a, then joins the stock F9-bank packed/raw/epilogue tails. The previous six-byte hook at RAM:f9130a was removed. Keil C251/A251/L251 emitted zero warnings/errors; 5 host tests pass; code=2239, xdata=0, no new persistent RAM; scratch image `out/agent-tests/gofw-read-event-clone-vs/MONT_104_gofw.bin` SHA-256 `8580da04f84d0e193350db4c7dd555002d660f4dcfca5c701552ca5b1a0c35b9`, checksum `0xf648`. Independent diff is limited to declared hook/dispatch/vendor/payload/checksum regions; stock D8 bytes and entire original event function are unchanged. Focused S251 at the stock raw descriptor tail and return site confirmed stock and patched D8 identical in captured state (`SP=0x60` at RAM:fd11d3), while patched C4 matched stock slot/aux/C2 state and used `RAM:00126e=0x00563412` versus D8's C2 cursor `0x0000aabb`. User flashed their own CMake-cache build. On H:, read-only INQUIRY returned identified `PLEXTOR CD-R PX-W5224A 1.04`; four F5 readbacks matched the scratch image at RAM:fd11cf, f9130a, fdfc27, and fdf75e, establishing those selected hook/function bytes but not a full image hash. Two consecutive C4 one-frame READs (`c4 02 13 88 00 00 00 01 00 00 00 00`, DATA IN 2742, 15-second host timeout) both returned GOOD/2742. Main-data nonzero counts were 2339 and 2335, frame hashes differed, sub-Q CRCs were valid, and absolute MSF advanced 00:16:18 to 00:16:19; C2 294-byte regions were zero for both. Exact captures/status/timing: `out/agent-tests/gofw-read-event-clone-live/`. This physically confirms main-data restoration and one-frame command continuation on the current disc. Real C2 error bytes remain unverified; test later on user's C2 disc. No SEEK was run in this live trial.
- Superseded experimental image: `out/agent-tests/gofw-read-producer/MONT_104_gofw.bin`, SHA-256 `8ae67f223108487bcd3c827cb4ff3a10461a1b6e1056933b94d41f06113b0dfe`. User flashed it; read-only F5 verified the coroutine's 32 bytes at RAM:fdf600. A one-frame C4 READ CDB `c4 02 13 88 00 00 00 01 00 00 00 00` (5-second CDB timeout, 2742-byte DATA IN, 10-second host timeout) failed with Windows error 121 and no SCSI status/data. No further commands were sent. Record: `out/agent-tests/gofw-read-producer-live/read_result.json`. Its C4 coroutine and event router could not establish stock D8's acquisition state, motivating the baseline above.
- Earlier H: tests on HOME-enabled images found C4 SEEK +50 and -50 returned GOOD and stock movement cache showed controller completion, but pickup displacement was not independently measured. C4 one-frame READ hung past the 3-second CDB deadline and 10-second host timeout; follow-up INQUIRY also stalled until power cycle. The old C4 path had no coroutine2; its RAM:000f46=1 disabled the stock empty-queue fallback. The new coroutine addresses that mismatch but has only offline verification.
- Historical HOME traces and exact CDBs are preserved under `out/agent-tests/gofw-c4-live-20260927/`, `out/agent-tests/gofw-c4-homefix-live-20260927/`, and `out/agent-tests/gofw-home-trace-build/`. The user chose to remove HOME because the stream-dumping workflow does not need a mechanical origin; no further HOME troubleshooting is planned.

## Immediate handoff: stop flash/blink loop; investigate SCSI debug monitor

- User says 28 diagnostic flashes consumed too much time; flashing is slow.
  No further one-observation-per-flash request. User proposes a custom SCSI
  command loop for memory reads/writes. Continue offline work toward that.
- Diag28 PHYSICAL: both LEDs continuously on for at least a minute, no blink
  group. No terminal report means no specific phase/fault established. Its
  8191-pass budget cannot expire unless C0 resumes. Diag27 FOUR previously
  proved the actual handshake delay59 returned and cleanup ran.
- Read docs/SCSI_DEBUG_MONITOR_FEASIBILITY.md. Stock F5 selector0 already
  reads CPU memory, physically verified in normal and protected recovery
  modes. Live Ghidra reconfirmed main RAM:fcb61f and monitor RAM:ff2a3e.
  Address CDB[4..7] BE32, length CDB[8..9] BE16. SFRs are separate.
- Monitor command loop RAM:ff5026 uses fixed table RAM:ff7a00+opcode*4,
  not an observed writable dispatch hook. Monitor RAM:ff57bc installs that
  command coroutine and runs its scheduler. WRITE BUFFER2/4 stages data;
  it is not an arbitrary CPU write. No generic stock RAM-write/execute
  command has been established. Mode5 programs/activates flash.
- Proposed, NOT IMPLEMENTED: dedicated main-image monitor based on the
  working recovery transport, leaving protected recovery intact; repeated
  reads, reserved-RAM writes, trace ring, bounded test operations. RAM code
  upload/run requires proven executable mapping/space/ABI. It cannot survive
  arbitrary hung code without an independent recovery mechanism. First audit
  ingress/completion/IRQ/scheduler dependencies and validate native repeated
  commands offline. No new device command or firmware handoff this turn.
- Begun diag29 progress image was canceled after user steering. Maintained
  source changes for it reverted; scratch diag29-build is UNVALIDATED and
  MUST NOT be offered for flashing. No completed diag29 test claim.
  Broad diag29-post-delay simulator experiment ERRORed: reconstructed UV4
  timed out at90s, stock not run. This is not a firmware-failure comparison.
- Concrete offline defect found but NOT YET FIXED: controller_parameters.c
  InitializeControllerRegisterBlock81ThroughBD (stock RAM:f9c48c) omits
  opening RAM:000fca|=8 and RAM:018002 publication before register clears.
  Live stock RAM:f9c494..f9c4a1 contains both writes. Current C goes from
  EA mask directly to clear018081..B0, then retains closing fca&=e7/write.
  Next: add ordered-write regression that fails old code, minimal correction,
  native comparison. Do not claim this causes/cures physical startup yet.
- Actual-Timer0 mismatch remains unresolved: stock controller018087/01808a
  00/00 vs rebuilt80/01 (state0-timer0-aggregate-long/). Stock reset writes
  RAM:f9c4b5 occur in state0-timer0-port-writes/. Distinguish S251 native
  RAM24 indirect vs MOVX X: view before explaining mismatch. Compiler XDATA
  RAM:010000..017fff physical backing remains unmeasured.
- Diag28 archive preserved: diag28-handshake-phases-20260926/
  PXW5224A_diag28_handshake_phases.bin SHA256
  7d4aa9e8012da7b172f3386f44200ad876b82c8d7dd98ff73e7cdaefc3fba0a8.
  Its nine native isolated-C0 scenarios, seven stock comparisons and slow
  frame passed; not full startup proof. See DIAG28_HANDSHAKE_PHASES.md.
- Last diagnostics-OFF build SHA256
  d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e.
  No production fix or Ghidra metadata edit during this investigation.
- Retain proven fixes: controller018xxx, tablesff84fe/ff850a+2*profile low
  byte first, P1.5/6/7 read11ed, TimerPhase0 tests1300, host restart RETI,
  ATA IDENTIFY+100000. Diag14 enumerated with unsolicited open/close/open;
  diag15..18 lost enumeration after corrected P1.7. Earlier zero C1 state
  observations are legitimate mode1 startup (DIAG25), not fault proof.
- User alone flashes using redumper; recovery Eject held at power-on then
  stock restore. No agent flash/reset/activation. Preserve archives/captures
  and user build/. All native uVision runs via private desktop wrapper.

## Current state

The canonical Plextor 1.04 decompilation objective is complete:

- C coverage is 1,717 / 1,717 canonical Ghidra functions (100.00%). Coverage
  is counted by unique entry address; literal native-byte bodies do not count.
- The compact `/* RAM:xxxxxx */` source tags and live
  `/validated/MONT_104.bin` inventory contain the same 1,717 unique addresses.
  The 1,730 source occurrences include intentional host/target or semantic
  variants at the same canonical address.
- Every canonical Ghidra function has a nonempty plate comment. Ghidra plate
  comments are the canonical home for useful per-function ABI, behavior,
  confidence, and validation evidence.
- Accepted C functions have no same-address assembly or literal-DB body.
  Addressless ABI, atomicity, indirect-call, reset-transfer, and monitor/runtime
  adapters remain only where C251 cannot express the native boundary directly.

This is a behavioral C reconstruction. It is not yet a byte-identical or
operational-image reconstruction.

A native-Windows audit on 2026-09-25 confirmed that the Keil C251/A251/L251/
OH251 toolchain can compile and link all 446 target objects and package a
structurally valid 524,288-byte image. The current image SHA-256 is
`f374052001e719e619fb0f35204d51dfc2bda5c60bc4c471e796278c2afba5f6`
(checksum `0xda36`; code 439,094; xdata 6,822; const 18,510). It builds with zero
Keil diagnostics, preserves stock fixed data, and passes 274/274 CTest entries
plus 129 simulator unit tests with one expected skip. The live canonical Ghidra
programs retain the required `80251-fixed:BE:24:default` language, 1,717 main
functions, and 173 runtime functions including the external entry.

The first native-Windows generated-catalog mismatch, `auto-fa56c5`, exposed ten
local helpers in `controller_record_page_reconciliation.c` that C251 emitted as
out-of-line `LCALL`/`RET` bodies. They added noncanonical stack depth, and the
Windows S251 debugger stopped at the first private `RET` before the public
function boundary. The
helpers are now zero-cost macros at their canonical call sites. The rebuilt
listing contains only the two public reconciliation functions and their final
`ERET`s; the focused host test and stock/reconstruction `auto-fa56c5` case pass.
The old 433/1,717 partial run (362 pass, 70 infrastructure errors, one mismatch)
is preserved at
`out/simulator/runs/generated-pre-inline-20260925-fa56c5-mismatch/`. The
corrected Linux aggregate had already completed 1,717 unique passes with zero
mismatches/errors (`results.json` SHA-256
`21a2242ef6ac2ec7430d725448ac8f7fbc365ba25a9f7aa15bc1f4c532efc081`).
A redundant native-Windows current-image replay was stopped cleanly after 303
unique passes and no errors or mismatches; it remains at
`out/simulator/runs/generated/`. Do not resume the broad replay merely to
duplicate the Linux evidence. Preserve the macro improvement and use bounded
current-image cases only where a changed function or open gate needs them.

Direct Windows CLI runs resolve their output root before uVision changes
working directory. `tools/run_on_private_desktop.py` starts the entire runner
tree on a named non-interactive `WinSta0` desktop, at below-normal priority, in
a kill-on-close job. All native-Windows CMake uVision targets use it. The
catalog driver additionally watches job-scoped `UV4.exe` descendants and kills
only an overdue child after the runner timeout plus a 30-second grace period;
completed live runs have no visible window handle and leave no UV4 orphan.
`tools/run_windows_generated_catalog.py` runs resumable 10, 30, 60, and
120-second passes, stops after a completed pass on any mismatch, and treats
uVision timeouts as retryable infrastructure errors.

The four integrated flashability gates remain open, but the reset-to-rescue
selection within the update/recovery gate is accepted offline. The user reports
that protected rescue firmware at `RAM:ff0000` is never flashed and is entered
by holding Eject during power-on; this is a user-provided hardware premise.
The captured monitor checks forced recovery before main-image validation, and
current paired cases reach `RAM:ff0300` with a checksum-valid nonfunctional
main entry. Do not require live rescue reflashing to close the reconstruction
gate. Distinguish the protected rescue view from the flash-backed main-image
view at overlapping high addresses. The SST command byte/address mapping is
now strongly resolved from the live byte-swapped `bf00` manufacturer word,
exact doubled x16 unlock addresses, and complete read aliases. Actual
programming effects, main-image checksum acceptance/rejection, and activation
remain open.
The read-only canonical validator now requires the relocated programmer's
success path to write `0x99` to `RAM:0000fd` and jump to `RAM:ff0200`, and the
bootstrap to call its forced-recovery predicate at `RAM:ff4e72` before main
handoff. Both canonical programs and the fixed language pass
(`out/agent-tests/reset_handoff_validation.log`). This establishes the stock
CPU handoff, not the outcome of a physical flash write. Immutable 1.04 image
comparison confirms the download's `RAM:ff0000..ff7fff` hole is 32,768 bytes
of `0xff`, whereas the runtime capture has 28,717 non-`0xff` bytes there;
`RAM:ff8000..ff9fff` matches exactly. This supports separate code views at
those addresses but does not measure physical flash alias or protection.
The current reconstructed image preserves that erased hole, all 8,190 stock
fixed-data bytes at `RAM:ff8000..ff9ffd`, and the erased 24-KiB tail.
`tools/firmware_image.py verify --fixed-data-from` now enforces the stock
fixed-data match independently of checksum; the current image passes
(`out/agent-tests/fixed_data_verification.json`) and a checksum-valid one-byte
mutation is rejected (18 focused host tests pass).
This closes the fixed-data packaging invariant, not physical flash behavior.
The regenerated agent-owned CMake/Ninja graph includes `--fixed-data-from`
in both pack and final verify commands. Executing that verifier on the existing
agent-owned package also passes (`out/agent-tests/simulator-no-ghidra/firmware/validation.json`,
image SHA-256 `83834ccf6c417c96c479f0a8aeebf812217f6c927875b6d83092f65ecb822ac8`).
The full target was not rebuilt: its dry run has 447 Keil steps. The current
image's independent pass is `out/agent-tests/fixed_data_verification.json`.

The current emulator reaches a high-bank memory-model limit and cannot supply
LC898095 media sectors, physical interrupt timing, flash bus effects, or the
physical coroutine-stack/programmer mapping. A static monitor-mode staging and
programming command path is verified. Windows now exposes an identified
`PLEXTOR CD-R PX-W5224A` revision 1.04 on `H:` through a USB bridge. Forty-four fixed
DATA-IN commands completed with GOOD status via the native Windows backend in
`tools/probe_readonly_w5224a.py`: Mode Page 03 matched MODE SENSE(10),
`RAM:f80050` matched the immutable stock reset bytes, `RAM:f00050` returned the
same bytes, low RAM selector `RAM:0000f8` returned `bf00`, and representative
`RAM:f7xxxx` reads matched `RAM:ffxxxx` at the protected monitor entry, its F5
handler, and fixed-data window. Full 512-KiB captures of `RAM:f00000..f7ffff`
and `RAM:f80000..ffffff` are byte-identical (SHA-256
`c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`);
the latter exactly equals stock through `RAM:feffff` plus the immutable
runtime-monitor capture at `RAM:ff0000..ffffff`. The report and recovery
backups are in
`out/agent-tests/windows-readonly-w5224a-full-backup-20260925/`.
This establishes normal-mode CPU data-read behavior and the selected loader
branch on this drive. The live `bf00` is the byte-swapped SST manufacturer word
`00bf`; together with CPU `aa00/5500` commands at doubled x16 unlock addresses,
it strongly establishes the stock programmer byte/address mapping. It does not
observe a flash effect. Any flash write still needs separate explicit
authorization and the now-proved Eject-at-power-on recovery strategy.

The read-only helper now has an explicit recovery profile. It requires the
captured monitor revision `0.00`, permits nine fixed INQUIRY/F5 DATA-IN probes
(plus the existing optional full backup), and has no reset or DATA OUT path.
Its negative gate was exercised against the still-normal 1.04 drive: one
standard INQUIRY completed with GOOD status, the 1.04/0.00 mismatch was
rejected, and no vendor command followed
(`out/agent-tests/windows-recovery-profile-negative-normal-20260925-v3/report.json`).
The report now requires all mode-specific identity, immutable-byte, alias, and
optional full-backup checks before setting `pass=true`; its expected recovery
identity is parsed from the hash-pinned monitor template at offset `0x7fc0`.
Nine focused helper tests pass. The positive recovery observation is now
complete: revision `0.00`, 41/41 DATA-IN commands, all 15 recovery checks, and
both full 512-KiB views pass at
`out/agent-tests/windows-recovery-full-backup-current-20260925/report.json`
(report SHA-256
`159e8a77985ba671ef6a1114f6ac638397974a3187aa10cdd4b998943c5d5d10`).
Both captures have SHA-256
`c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`
and equal stock main plus the protected monitor. This confirms live recovery
entry/readback, not DATA OUT or flash effects.
The same current helper passes its normal-mode live check: 12/12 fixed DATA-IN
commands and all 14 required checks pass, with revision `1.04` and selector
`bf00` (`out/agent-tests/windows-readonly-w5224a-current-helper-20260925/report.json`).

The retained stock `PXFirm3.exe` is now independently tied to the host update
sequence: its hash-pinned x86 loop sends 31 buffer-ID-zero WRITE BUFFER mode-4
chunks of 16 KiB, followed by the final 16-KiB chunk at offset `0x7c000` as
mode 5; the final command has the updater's 600-second timeout and activates
the programmer. `tools/firmware_update_plan.py` reproduces that exact 32-CDB
plan without a DATA OUT backend. On the current Windows image it validates
candidate SHA-256
`f374052001e719e619fb0f35204d51dfc2bda5c60bc4c471e796278c2afba5f6`,
stock fixed data, the retained updater/INI hashes, and the identified drive's
full read-only backup including selector `bf00`. The current normal- and
recovery-bound plan is
`out/agent-tests/windows-firmware-update-plan-current-with-recovery-20260925.json`
(SHA-256
`57a4eb3b98dc6866cf467b51d1aac8f906bed38e6634b6eeb0ec164512c8feaf`);
the focused planner/probe/executor unit set passes. This closes command-plan
reproducibility and records the strong SST mapping inference, not physical flash
effects, and sends no drive command.
The hash-pinned updater's version gate at VA `0x00405cb0` bypasses its
later-version comparison when either revision starts with ASCII `0`
(`0x00405dd7`/`0x00405de7` to accepted `0x00405ead`), so recovery-monitor
revision `0.00` is not rejected by that gate. The bound plan records this with
an explicit limitation: live recovery enumeration is now proved, but transfer
remains unproved. The planner additionally maps the relevant PE32 virtual addresses
and rejects the updater unless the exact zero-prefix branch bytes and accepted
success-return bytes are present; the mutation regression test passes.
The planner now accepts an optional `--recovery-report` only if a current-helper
revision-`0.00` run passed its nine fixed probes plus 32-chunk full backup, all
41 records are DATA IN, all 15 recovery checks pass, and both captures
cryptographically equal stock main plus protected monitor. The binding now has
a live positive report; 32 combined image/plan/probe tests pass.
The generated plan now covers all 512 KiB with explicit recovered operation-3
ranges and per-range hashes: candidate code/constants `0x00000..0x6ffff`,
stock-identical fixed data `0x78000..0x79ffd`, and candidate checksum
`0x79ffe..0x79fff` are intended programmer inputs; the stock-identical recovery
hole `0x70000..0x77fff` and erased tail `0x7a000..0x7ffff` are preserved. This
does not promote CPU ranges to verified physical flash effects.

`tools/flash_w5224a.py` is now the non-GUI Windows execution boundary. Its
default mode validates only and does not open the drive. Live mode regenerates
and exactly matches the normal/recovery-bound plan, requires the full candidate
SHA-256 as a confirmation token, re-identifies revision `0.00`, records each
DATA-OUT CDB/payload/result before advancing, stops on the first failure, and
never retries the final self-activating command. A dry run against the current
plan passes; no DATA OUT was sent. Twenty focused executor/planner/probe tests
pass.
`tools/verify_w5224a_flash.py` is the DATA-IN-only post-flash verifier. It
constructs the exact expected runtime composite from the current candidate's
operation-3 ranges plus the immutable protected-monitor/configuration capture,
then compares both complete CPU views. Its current validation-only result
expects composite SHA-256
`87e2fc7227ba5801b79620e92d9e79e1ee8a4308d673e8d6784f9c7774370c43`.
The combined focused executor/planner/probe/post-verifier set has 23 passes.

The user flashed current candidate SHA-256 `f3740520...a5f6` themselves with
redumper. The drive did not start or enumerate normally afterward. A subsequent
read-only recovery-profile attempt against `H:` failed at the initial standard
INQUIRY open with `CreateFileW(\\.\H:) failed`; therefore no vendor CDB, DATA
OUT, reset, or write was sent by the project helper. Evidence is
`out/agent-tests/windows-postflash-recovery-check-f3740520-20260925/report.json`.
The user subsequently entered the protected revision-`0.00` monitor and the
post-flash CPU views were captured read-only; that recovery action is complete.

The user then entered the protected monitor successfully. An elevated,
DATA-IN-only post-flash capture identified `PLEXTOR CD-R PX-W5224A` revision
`0.00`; all 32 F5 chunks returned GOOD and the complete programming/execution
views are byte-identical (SHA-256
`e239a0d29dbe426c7b31e542422e21feab84106093d5092f4bd91632b18ee4d5`).
Every internal-operation-3 programmed byte matches candidate `f3740520...a5f6`:
main `0x00000..0x6ffff`, fixed data, and stored checksum `0xda36`. The checksum
recomputed over the live readback is also `0xda36`. Thus redumper/programming
completed correctly; the reconstructed firmware itself failed after monitor
handoff and before Windows enumeration. The protected monitor range is intact.
Only two bytes in the mutable/preserved runtime-configuration tail differ from
the pre-flash capture: `RAM:ffa077` changed `00 -> 03` and `RAM:ffa07f` changed
`05 -> 08`. Evidence is
`out/agent-tests/windows-postflash-f3740520-recovery-20260925-v2/report.json`.
The user then restored immutable stock `data/W5224A104/MONT_104.bin` (SHA-256
`622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2`) with
redumper. The drive boots and enumerates normally as revision `1.04`. A
DATA-IN-only normal-mode full backup at
`out/agent-tests/windows-stock-restore-full-backup-20260925/report.json`
confirms both complete CPU views are identical and the main image through
`RAM:feffff` matches stock. Its two aggregate immutable-monitor checks are
false only because the same persistent tail bytes remain changed at
`RAM:ffa077=03` and `RAM:ffa07f=08`; all normal-mode representative, alias,
fixed-data, selector, and main-image checks pass. Because stock boots with
those exact tail values, they are not the reconstructed-image boot blocker.

A 2026-09-25 read-only evidence audit verified that all 22 integrated catalog
cases plus two additional pilot/feasibility cases, and all 39 monitor catalog
cases, had matching passing case-definition hashes on the preceding linked
application/map. The source change that produced current image `f3740520...a5f6`
requires those application-bound catalogs to be rerun after the direct catalog.
None of the four flashability gates is
complete. Stop open-ended S251 high-bank probing: the direct observation
attempt exceeded its 45 s wall limit before the stock snapshot. Do not repeat
that S251 probe. A distinct live read-only observation now establishes the
normal-mode `RAM:f00050`/`RAM:f80050` CPU data-read alias, but not the mapping
after P3 changes at a programming handoff. The read-only F5 candidate at
`RAM:2bc6ef` now has a MODE SENSE(10) current-page cross-check: compare its
eight DATA IN bytes with response bytes 8..15 on an identified stock-1.04
drive after verifying GOOD status, length, and the `03 06` page header. This
cross-check passed on the connected 1.04 drive with bytes
`03 06 00 00 00 00 00 00`. See
`docs/FLASHABILITY_INTEGRATION_GATES.md` and `docs/HARDWARE_PLATFORM.md`.

A separate current-image read-only audit of operation-5 traces verified the
current `trace_runtime_flash.py` script hash and image hash. Both declared
`RAM:0000f8` branches have identical ordered stock/reconstructed CPU stores
through the first erase command (14 records for `0x2000`, six for `0xbf00`).
The `0x2000` continuation's later bare `0x3000` store is invalid under the
conditional SST model. Live low-RAM F5 observation selects the `0xbf00` branch.
The byte-swapped manufacturer word and exact SST x16 command addresses/words
strongly resolve the byte lane; actual chip effects remain unmeasured. This
audit found no reconstruction correction. See
`docs/FLASHABILITY_INTEGRATION_GATES.md`.

The read-only mapping probe is now executable as
`tools/probe_readonly_w5224a.py`. Its dry run confirms six fixed DATA IN CDBs
and a stock-image reference at file offset `0x50`; actual vendor probes are
gated on INQUIRY vendor/product/revision. The focused existing
`vendor_f5_tests` passes from `out/agent-tests/command-crosscheck/` and
checks selector-zero direct address/length parsing. It has not run against
a drive.
For the reset overlap, source inspection confirms
`ProgramAndVerifyDriveConfiguration` reinitializes coroutine two before the
runtime programmer but has no coroutine-three reinitialization on the observed
return path. The generic Intel 8XC251Sx manual shows SPX addresses region
`00:` and its own on-chip RAM ends at `0x041f`; this does not establish the
LC898095's physical mapping at `RAM:000900`. The paired S251 overlap remains
conditional and no source correction is justified yet.

A command-path audit confirms that the fixed read-only MODE SENSE(10)
CDB `5a 00 03 00 00 00 00 00 10 00` passes the immutable firmware-1.04
mask at `RAM:ff8cc9`: every prohibited-bit intersection is zero. The
reconstructed response path emits eight header bytes and copies eight current
Mode Page 03 bytes from `RAM:2bc6ef`, yielding a 16-byte response with the
page at bytes 8..15. This validates the proposed cross-check's CDB and
response layout, not physical high-bank access or hardware delivery.

The current-image monitor catalog now has 37 cases. The new non-packet host
cause-`0x01` case passes stock/reconstruction differential execution through
the captured router and `RETI` (414 / 417 states) with matching controller
read `RAM:018057=0`, four ordered writes, packet state `0x10`, protocol zero,
flag clear, and SP=7 (`out/agent-tests/hostcause01_probe_xdata/results.json`).
Its catalog case hash and current application/map/monitor identities match.
Canonical runtime disassembly independently confirms the flash-ID loader
reads a device word and stores it at `RAM:0000f8`, accepts `0xbf00` and
`0x6200`, and normalizes other values to `0x2000`
(`out/agent-tests/flash_id_selector_headless.log`). The identified live 1.04
drive returned `bf00` from that slot through F5 selector zero, selecting the
alternate SST-style branch. Ghidra headless returned exit zero for a failed scratch
inspection script, so `tools/validate_canonical_ghidra_project.sh` now
requires explicit canonical and processor-language PASS markers and rejects
reported script errors. Fake-headless missing-marker/error/pass checks behave
correctly, and real read-only validation passes both canonical programs
(`out/agent-tests/ghidra_validation_wrapper_probe/real-validation.log`).

Read-only canonical `ProgramAndVerifyDriveConfiguration` instructions show
that a nonzero configuration word `0x000f` clears `BITS:0580` and sets
`BITS:0581` before the `RAM:ff4500` monitor call, and restores P3 bit0 plus
conditional bit1 only after a successful call
(`out/agent-tests/configuration_p3_headless.log`). The board wiring of these
pins is unknown. Stock `HandleWriteBufferCommand` also clears both bits at
`RAM:fd2b79..fd2b7b` before operation 3 or 5 through runtime
`RAM:ff4000` (`out/agent-tests/write_buffer_p3_headless.log`), matching
the C path. A read-only F5 alias in normal command mode cannot alone
prove the flash address mapping at either programming handoff. This
does not explain the S251 coroutine-three overlap without physical mapping.

Plextor’s PX-W5224TA installation manual names `PX-W5224A` as the
ATAPI INQUIRY identifier for retail model `PX-W5224TA`; this reconciles
the hardware teardown’s model label with the firmware image target. The
observed SST/LC898095/Winbond components are still one board inspection,
not verification of a future test unit’s chip markings or wiring
(`docs/HARDWARE_PLATFORM.md`).

Canonical runtime dispatch bytes map monitor WRITE BUFFER `0x3b` to
`RAM:ff210d` and READ BUFFER `0x3c` to `RAM:ff25ae`. The mode-4 branch reaches DATA OUT worker `RAM:ff28a6` at
`RAM:ff2427`, and mode 5 can transfer a nonzero payload through the same
worker at `RAM:ff249f` before validation. The mode-5 monitor
WRITE BUFFER body checks the main/auxiliary staged checksum, clears P3 bits
`BITS:0580/0581`, and calls runtime `RAM:ff4000` with operation 3 or 5
(`out/agent-tests/monitor_buffer_commands_headless.log`). The read-only
canonical validator now enforces dispatch entries, mode-4/5 DATA OUT edges, handler
symbols, P3 clears, and both loader call flows; both canonical programs and the fixed
language pass (`out/agent-tests/monitor_buffer_commands_validation.log`).
This is a static monitor-mode recovery candidate. It does not prove a live
DATA OUT transfer, flash write, or successful reflashing. The official
`PlexFirm.ini` declares a 524,288-byte image and 16,384-byte buffer but no
verified recovery procedure (`docs/FIRMWARE_UPDATE_ANALYSIS.md`).

The fixed read-only F5 capture helper now also requests two bytes from
`RAM:0000f8` with CDB `f5 00 00 00 00 00 00 f8 00 02`, to observe the
flash-ID branch selector stored by the runtime loader if low-address F5 reads
reach that RAM slot. The helper still requires stock PX-W5224A 1.04 INQUIRY
identity before vendor commands; syntax and dry-run were checked. A read-only
audit confirms all 24 integrated and 39 monitor case definitions have current
case-hash-matching passes on the current application and map; only monitor
cases require the assembled monitor hash. No drive
node is present and neither the selector nor physical address mapping has
been measured. No LC898095 register/memory-map manual was found in the
current primary-source search; the Sanyo brochure documents a buffer-RAM
mapping function without its register details.

The first probe's read-trace mismatch was a harness watch error: C251 generated
`MOVX A,@DPTR`, which S251 exposes as XDATA `X:8057`; correcting only that
watch made the case pass. No reconstruction source change was needed. Physical
host IRQ delivery and other branches remain open.

An exploratory active-coroutine-zero host cause-`0x01` case remains
inconclusive and was not added to the catalog. The generic S251 Timer0 alias
reasserted and exhausted the single declared cause read (`RAM:fb99fe` stop);
a one-shot variant then ended at linked media recovery `RAM:fcdeec` before
the required `ResumeCoroutine1` checkpoint. Both reports contain only an
unpaired reconstructed-side error (`out/agent-tests/hostcause01_active_probe_v3/`
and `out/agent-tests/hostcause01_active_probe_oneshot/`). Do not interpret
these as firmware failure or repeat unchanged. A clean active-host test needs
a captured scheduler context and a one-shot source that does not perturb
startup command state.

The new focused `staged_checksum_tests` host regression fills the complete
main and auxiliary staged ranges, verifies independent 16-bit expected sums
and read counts, and rejects one-byte mutations; CTest passes in 0.03 s in
`out/agent-tests/simulator-no-ghidra`. This is full-range C evidence, not
a stock dynamic checksum decision or physical bank observation.

Read-only canonical stock disassembly of `ValidateStagedFirmwareChecksum` at
`RAM:fd2c93..fd2d3f` confirms the current C implementation’s main and
auxiliary source ranges, exclusive loop ends, 16-bit sum, and final stored
word locations (`out/agent-tests/staged_checksum_headless.log`). No source
correction follows. The paired main-image checksum continuation still stops
at the S251 bank-`0x02` boundary before a decision; this static result does
not prove physical staged-data access.

The current-image monitor catalog now has 39 cases. A new host cause-`0x02`
case starts from the verified second scheduler checkpoint with coroutine zero
yielded, injects a one-shot generic S251 Timer0 source, copies declared
`12 34 56 78` from `RAM:001455..58` to `RAM:001459..5c`, sets completion,
and reaches the third `ResumeCoroutine0` after `RETI` in both images (469,294 /
296,041 states). SP=10, scheduler SPX `0x0a`, coroutine-zero SPX `0x307`,
and saved coroutine pointers match (`out/agent-tests/hostcause02_yielded_nonzero/results.json`).
The case hash and current application/map/monitor identities match. This
closes the bounded yielded-coroutine host-interrupt scenario. A second
current-image case interrupts coroutine zero while it executes
`ReadDriveConfigurationBytes` during startup, with a one-shot generic S251
Timer0 vector, host cause `0x02`, zero source count, and no packet-state seed.
Both images return through the captured host router to `ResumeCoroutine1`
(395,642 / 215,772 states), record cause `0x02`, preserve the zero count and
`RAM:001596=0`, set `RAM:000c83=0x10`, and reach SP=10 with Timer0 stopped
(`out/agent-tests/hostcause02_active_probe_corrected/results.json`). The
first scratch assertion expected completion 1 but both stock and reconstruction
observed 0; the corrected fixture passes with matching case hash and current
application/map/monitor identity. This closes one active-coroutine host cause
under declared S251 source delivery. Physical IRQ timing, other host causes,
and the natural fourth scheduler turn remain open.

## Build and flash readiness

The source tree is not flash-ready:

- CMake links `coroutine_initialization.obj` and `firmware_entry.obj` and
  locates `?PR?COROUTINE_INITIALIZATION` at canonical `RAM:fdcea9`.
  `FirmwareMainLoop` is the first function in that segment, so reset now enters
  reconstructed firmware behavior instead of a disabled idle loop.
- Packaging emits `MONT_reconstructed.bin` and a separate `validation.json`.
  Validation currently proves only image shape and checksum integrity.
- Six four-byte `EJMP` veneers occupy the canonical reset/interrupt entries at
  `RAM:f80050`, `RAM:f80054`, `RAM:f8007f`, `RAM:f800cb`, `RAM:f800f6`, and
  `RAM:f80123`; their compiler-generated bodies are freely located.
- The firmware link manifest is generated from every declared C251/A251 object
  rather than maintained as overlapping literal lists. The old invocation had
  422 entries, only 403 unique, with 19 duplicates and 40 omitted translation
  units. The corrected invocation has 446 produced and linked objects, all
  unique, with none missing or unexpected. Former omissions including
  `ScsiCommandProcessorCoroutine0`, `RunDriveOperationResultCoroutine0`, and
  `HandleHostInterfaceInterrupt` now resolve.
- The corrected 2026-09-20 Wine/Keil O5/SIZE pass compiles all 446 objects.
  L251 selects the required `C2SL`, `C2SFPL`, and `C2SDPL` runtime libraries;
  there are no unresolved symbols. Converting relocatable C/A251 boundaries to
  the far 24-bit call model eliminated all 1,189 former improper-fixup errors.
- The physical layout completes L251 with zero warnings and errors, passes the
  absolute-DATA audit, converts through OH251, and packages successfully. The
  map paired with the preceding packaged image reports `data=104.0
  edata+hdata=0 xdata=6824 const=18510 code=437795`, or 456,305
  code-plus-constant bytes within the 458,672-byte main payload. The balanced
  layout uses `RAM:f80050..feb4ff` for ECODE,
  `RAM:feb500..fefeff` for HCONST, and `RAM:feff00..feffff` for CODE.
- The preceding `MONT_reconstructed.bin` was exactly 512 KiB, had
  SHA-256 `2c6e774a886c791cce180dbf57174c6fc68b136c9582d045a56026e7f2b3e1ef`,
  stored checksum `0x9a8e`, and a passing strict format validation. C251
  compiled corrected `leaf_workers.c` with zero diagnostics in a clean
  agent-owned directory using the exact firmware flags; its hash-recorded
  object was used for L251 linking. The linker, absolute-DATA audit, OH251,
  packaging and validation all passed. The target listing at offset
  `0x003d61..003d69` now loads both halves of the 32-bit count at
  `RAM:00156a`. The isolated host `configuration_queue_count_tests` passes
  with count `0x00000200`. The P3 test fixture was corrected to seed the
  low byte checked by canonical `R7`.
- The preceding image passes all 17 versioned integrated cases at
  `out/agent-tests/integration-gates/config-queue-fix-integration-seventeen-run/results.json`
  and all 25 versioned monitor cases at
  `out/agent-tests/integration-gates/config-queue-fix-monitor-twentyfive-run/results.json`.
  Both runs record the same linked application SHA-256
  `4daa9310e66ca9f91942a6e418a1fccaaf089351cadc1cba037833028231e876`
  and map SHA-256
  `44e2ec137d36e59f0177a80a3145f9213d2d6165bb8ddf4e17c0a49abce9db81`.
  These cases remain bounded scenario evidence, not full cold-start, media,
  effective-flash or hardware validation. The full 1,717-case direct catalog
  predates this image and has not yet been rerun.
- Read-only conditional first-erase calculations on the preceding image still wipe
  its reset veneer under the assumed high-byte SST command lane and
  `+0x80000` alias. A first 4-KiB sector erase changes 4,010 non-`0xff`
  bytes and computes checksum `0x5282`; a 64-KiB block erase changes 65,139
  bytes and computes `0x3fbc`, both against stored `0x9a8e`. The actual bus
  alias, device-code branch and chip response remain unmeasured. The image
  is not safe to flash.
- The 96 one-byte absolute DATA segments intentionally alias the nine bdata
  bytes at `RAM:0020..RAM:0028`. Their 87 overlaps are checked from every map
  before OH251 and then suppressed as the verified L251 warning-30 case.
- A reproducible source/Ghidra audit found 292 unique priority ROM/overlay
  addresses. All 25 exact function-entry literals are now linker-resolved
  symbols. The `RAM:f83ef7` SCSI dispatch table, `RAM:ff9691` unsupported
  pointer dependency, and `RAM:f842f7` media-requirement table are now one
  verified symbolic opcode model. The remaining internal ROM reads now use
  linker-resolved objects generated from the hash-verified 1.04 image:
  `RAM:f82320..f848ff` and an explicit 73-byte replacement for three
  code-as-data reads. The packer owns the immutable 80-byte header and
  `RAM:ff8000..ff9ffd` tail, avoiding 8,270 bytes of duplicate linked data.
  Configuration-flash and
  runtime-monitor addresses use named fixed-contract wrappers. The current
  C/header/A251 audit reports zero unresolved priority occurrences. See
  `docs/ABSOLUTE_ADDRESS_RELOCATION.md`.
- Relocatable generated data occupies linker-selected HCONST space. The
  hash-pinned packer restores the immutable header and fixed tail at their
  canonical physical offsets.
- No reconstructed image has passed complete cold-start, drive-level media,
  recovery, timing, or hardware flash testing. Direct simulator cases cover
  interrupt and coroutine entry/termination, and the feasibility suite covers
  one integrated command-completion scenario, but these are not substitutes
  for full subsystem scenarios.

The preceding `MONT_reconstructed.bin` is SHA-256 `2b965412b1e539d0b2cf0da1b27e78b901e4b1e87c950344b10014eed39e1979` with checksum `0x99bc`. C251/L251, absolute-DATA audit, OH251, packaging, strict format validation, and the focused host controller-window copy test pass. With the corrected HDATA relocation harness, that image passes the versioned mode-4 first yield and controlled handler-to-idle completion (`out/agent-tests/integration-hdatafix-correct-xvfb/results.json`, `out/agent-tests/m4-final-complete-xvfb/results.json`). Mode-5 zero-length finalization reaches the first scheduler yield and first revision-copy read, then completes revision rejection with staged ASCII `00` through the written `70/04/40/F5` sense and command idle in both images (`out/agent-tests/integration-hdatafix-correct-xvfb/results.json`, `out/agent-tests/m5-final-copy-entry-xvfb/results.json`, `out/agent-tests/m5-revision-rejection-final-xvfb/results.json`, `out/agent-tests/m5-rejection-completion-final-xvfb/results.json`). A separate staged revision `09` and declared media state 1 reach main-header validation; a zero first staged identity byte versus installed `P` also produces `70/04/40/F5` and command idle (`out/agent-tests/m5-header-rejection-final-xvfb/results.json`). These continuations use an explicit S251-only controller-window bank relocation. All 20 earlier versioned integrated cases and the original 29 monitor cases pass together on this image and corrected harness (`out/agent-tests/integration-hdatafix-correct-xvfb/results.json`, `out/agent-tests/monitor-hdatafix-correct-xvfb/results.json`; application SHA-256 `2eb00ff6152b40a843422d289542eefb42fa263d717f7a411685e6b1441da0bd`, map SHA-256 `f22b8b84fba69f5235d7fecb21aab327f92de73f431e75548f417cb382ae357c`, assembled monitor SHA-256 `51570409be33d1d9b01dfbd960aec2fb31e136a04fab0e8ceba84148f2a4d486`). Two newly versioned main-mode host-interface cause cases in the now 31-case monitor suite pass separately on those same image/map/monitor identities: cause `0x02` snapshots the four-byte count and sets completion (`out/agent-tests/host-interface-transfer-cause2-versioned-v2-xvfb/results.json`); cause `0x04` advances transfer state 8 to `0x0d` (`out/agent-tests/host-interface-data-completion-cause4-versioned-xvfb/results.json`). Two further main-mode host cases pass separately on the same identities: cause `0x41` admits a packet command and selects SCSI protocol (`out/agent-tests/host-interface-command-cause41-versioned-xvfb/results.json`); cause `0x30` publishes status completion and matches 15 ordered controller writes (`out/agent-tests/host-interface-status-cause30-versioned-xvfb/results.json`). The monitor suite then defined 33 cases; the earlier 29-case aggregate had not yet been rerun. The integrated suite then defined 21 cases; the new READ(10) command-to-scheduler-yield case passes separately on the same image (`out/agent-tests/read10-scheduler-yield-versioned-xvfb/results.json`). Its corrected coroutine-two continuation uses the real scheduler ECALL, enters the media task and reaches first `YieldCoroutine2` with target frame 150, 512-slot ring count, limits `01 ff 01 fa`, and matching pipeline state (`out/agent-tests/read10-media-first-yield-ecall-final-xvfb/results.json`). A further reproducible trace returns both images to the exact scheduler ECALL successor, naturally reaches the next command resume, enters batch service, and yields again with 0/1 blocks complete (`out/agent-tests/read10-wait-cycle-final-xvfb/results.json`). A new paired trace declares one status-zero descriptor at the observed empty `RAM:0088bb` ring; both images reach the raw-sector transfer function and controller start, set command `0x88`, length `0x0800`, source `0x002010`, trigger `0x50`, and yield with `RAM:001462=1` (`out/agent-tests/read10-transfer-start-final-xvfb/results.json`). This checks conditional transfer setup, not sector bytes. With the additional declared prefix-acquired state `RAM:00127e=0`, a paired cause-`0x04` interrupt trace enters the physical monitor vector and normal raw-read completion, clears `RAM:001462`, and returns to the scheduler (`out/agent-tests/read10-data-in-interrupt-final-xvfb/results.json`). Controlled selection of the real scheduler `ResumeCoroutine0` ECALL then reaches the READ(10) engine return and next command idle yield with packet state 1, command state 4, latch 0, and issued/requested 1/1 (`out/agent-tests/read10-command-idle-final-xvfb/results.json`). The descriptor and interrupt are declared simulator inputs. A versioned continuation declares a prebuffered second packet with unsupported opcode `0xff`; both images enter the unsupported handler, write `70/05/24/00` sense, and return to idle (`out/agent-tests/read10-second-command-final-xvfb/results.json`). Sector bytes, physical second-packet ingress, and timing remain unproved.

The preceding `MONT_reconstructed.bin` is SHA-256
`061ed2a1722b64030f362d537e6d8c1ffdc31c6619f0c961826db2f2c1ca4e76`
with stored checksum `0xa12f`; application SHA-256
`7a848a669a864d8ebea33bb14149e3bdbf2a4f0c5a90c57a99d73948c74ffbfa`
and map SHA-256
`c8691cd007fc0407fd1ec53c46c4acf9b30bd03c8985dee70297f7ca3d91ad71`.
C251/L251, absolute-DATA audit, OH251, packaging, strict format validation,
and focused startup-status host tests pass. The complete 33-case monitor and
23-case integrated catalogs pass together on this image at
`out/agent-tests/m33fix/results.json` and
`out/agent-tests/i21fix/results.json`. The extended READ(10), mode-4, and
mode-5 continuations in the preceding paragraph still refer to the preceding
linked image; their first checkpoints are covered by the latest catalogs.

The current `MONT_reconstructed.bin` is SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c`, stored checksum `0xa15d`, linked application SHA-256 `76069bba74cc771b329cbfa89dc9388759f7e00039c4c6faa2a3912c5653416e`, map SHA-256 `7902fad70049cf17f65802a746c3ff8b7da6800246a40d59488a9d7bc8c7983a`. C251/L251, absolute-DATA audit, OH251, packaging, strict format validator, focused checksum and write-buffer host tests, all 24 integrated cases, and all 34 monitor cases pass (`out/agent-tests/i24checksumfix/results.json`, `out/agent-tests/m34blockfull/results.json`). The new auxiliary mode-5 zero-length case passes first yield. Its paired continuation reaches `ValidateStagedFirmwareChecksum` with DR0=0 and DR4=`0xfffe` on both sides, then reaches `SetSenseData(4, 0x40f6)`, writes `70/04/40/F6`, and returns to command idle (`out/agent-tests/m5auxentry_final/results.json`, `out/agent-tests/m5auxreject_final/results.json`, `out/agent-tests/m5auxidle_final/results.json`). The reconstructed checksum ABI was corrected in source. The original S251 observation scratch at `X:6000..7fff` overlaps the auxiliary candidate checksum range (`out/agent-tests/m5auxview_final/results.json`). A case-local run moves trace scratch outside the candidate, normalizes the one differing staged byte at `X:1a57`, and uses a debugger-only bank-`0x10` to bank-`0x01` substitution for the three checksum reads in each image. The resulting identical candidate (SHA-256 `2662469194e950652d4a755dcfc4594ce633ff629fc063012b9cd0f81c0be03a`, sum/stored word `0x0001`) is accepted with `ACC=1` in both images (`out/agent-tests/m5auxcleanscratch_final/results.json`, `out/agent-tests/m5auxcleanaccept_final/results.json`). Seeding the 96-byte primary configuration from immutable runtime capture SHA-256 `7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80` at `RAM:ffc000` and applying the same simulator bank substitution for the stage write makes both images copy all 96 bytes to auxiliary staging at `RAM:10c000`; 87 bytes change from the pre-copy snapshot. Both reach pre-flash `RAM:ff4000` with DR0=`0x00050000`, DR4=0, and DR8=`RAM:ff4000` (`out/agent-tests/m5auxcleanhandoff_final2/results.json`). No flash-loader instruction executes in that trace. Actual controller staging, flash effects, activation, and physical recovery remain unverified. The monitor catalog now has 36 versioned cases. Its new main-mode host cause-`0x10` case passes separately on this same image/map/monitor at `out/agent-tests/hostrestart_init_versioned/results.json`: both images enter the captured host vector, record the same cause, write three matching controller acknowledgements, set restart marker `RAM:0000fd=0x98` and error `RAM:0000fe=0xff`, clear the request byte in the interrupt wrapper, execute `RestartFirmware`, clear `SFR:A8`/`SFR:A9`, and reach `InitializeAndEnterFirmwareMain` in 340/397 states. Read-only canonical runtime disassembly identifies `RAM:ff2004` as a finite `WR6 < 0x893` delay. The repeatable staged continuation `tests/simulator/trace_host_restart_initialization.py` (`out/agent-tests/hostrestart_initialization_trace/host-interface-main-vector-status-cause10-initialization-entry-via-timer-source/results.json`) passes that delay with two declared timer overflows, reaches first and second `0x8000`-byte controller fills at `RAM:1b0000` and `RAM:1b8000`, and uses two declared routed event-`0x40` completions to reach a later timer poll in both images at 780,015 / 780,014 states, with `SFR:A8=0` and `RAM:14da=1`. A new versioned standalone INT0 event-`0x40` case passes on the same image (`out/agent-tests/controller_event40_versioned/results.json`, 423 / 452 states). The latest two monitor cases were verified separately; the 34-case aggregate predates them. An exploratory periodic S251 overflow signal (every 5,000 states while `SFR:91` run bit is set) advances both restarted images to the third mode-page defaults copy. Stock at `RAM:fc2a73` has DR0=`RAM:2bc6ef`, DR4=`RAM:ff8a5a`, ACC=8; reconstruction at `RAM:fd89a0` has the same semantic arguments in its local registers. The unmodified high-bank copy stops inside `CopyBytes8` on both sides (`out/agent-tests/hostrestart_flash_id_entry_probe/`); a debugger-only destination-bank substitution to `RAM:01c6ef` copies the same eight immutable bytes `03 06 00 00 00 00 00 00` on both sides (`out/agent-tests/hostrestart_modepage_copy_probe/host-interface-main-vector-status-cause10-initialization-entry-via-timer-source/results.json`). The stock after-call breakpoint returns PC/state zero in S251 and is not accepted as caller-return evidence. This is conditional simulator progress, not high-bank hardware proof. The stock call is Mode Page 03 initialization; MODE SELECT can replace that image and the page-control selector returns its `RAM:2bc6ef` address. A read-only F5 DATA IN candidate for those eight bytes is now documented in `docs/HARDWARE_PLATFORM.md`; no `/dev/sg*` or `/dev/sr*` device is present here. An attempted debugger `SAVE` of eight bytes at `RAM:2bc6ef` timed out at 45 seconds before a first snapshot appeared, so it supplies no copy evidence and should not be repeated unchanged. Restart completion, a post-restart scheduler turn, and physical timer/controller behavior remain unproven. A direct main-loop breakpoint was discarded because initial boot reaches it before interrupt injection.


Canonical `CopyFromControllerBufferWindow` (`RAM:fde0a6`) receives destination in DR0 and controller offset in DR4. A mode-5 call-entry trace showed the reconstructed arguments reversed. The source signature/callers were corrected, and the C251 listing plus current-image call-entry trace now show DR0=`RAM:000b2a`, DR4=0 on both sides. The corrected-image rejection trace reaches `SetSenseData(4, 0x40f5)` on both sides after the 48-byte copy; a separate continuation now confirms the resulting sense bytes and command idle. The malformed-header case likewise completes. Main-image checksum rejection, effective flash programming, activation, and physical recovery remain open; auxiliary checksum acceptance and pre-flash handoff now have conditional paired evidence. An earlier bounded stock-only passing-header diagnostic entered `ValidateStagedFirmwareChecksum` at `RAM:fd2c93` and stopped at `RAM:fd2cb8` with DR4=`RAM:020000` after 3,807,024 states, before any checksum sense call (`out/agent-tests/m5-checksum-reject-stock-probe/`). The current-image paired passing-header continuation now reaches the stock `RAM:fd2c93` and reconstructed `RAM:f84fb4` checksum entries with DR0=0 and DR4=`0x79ffe` (`out/agent-tests/m5mainchecksum_entry_probe/results.json`). A bounded continuation stops at the same S251 bank-`0x02` read limit in both images: stock DR4 and reconstructed DR0 each hold `RAM:020000` after 3,807,024 and 3,418,023 states respectively (`out/agent-tests/m5mainchecksum_boundary_final3/results.json`). Reconstructed PC reporting at the invalid-bank halt can be zero. This is a shared simulator boundary, not a checksum rejection result or reconstructed mismatch. Preceding-image operation-5 first-unlock cases and continuations now pass separately in both declared device-ID branches: 14 matching CPU stores for `0x2000` and six for `0xbf00` (`out/agent-tests/flash-first-stores-current-xvfb/results.json`, `out/agent-tests/flash-bf00-stores-current-xvfb/results.json`). Under the explicit SST high-byte command-lane and `+0x80000` alias assumptions, the first sector or block erase would blank the preceding reconstructed reset veneer and invalidate its checksum (`0x51b0` or `0x3fd0` computed versus `0x99bc` stored). The physical bus mapping and chip effect remain unverified. On the preceding image, the same branches yield 14 and six matching CPU
stores at `out/agent-tests/f2000fix/results.json` and
`out/agent-tests/fbf00fix/results.json`. Conditional first sector/block
erases change 4,010/65,139 reconstructed bytes, blank the reset veneer, and
compute `0x5846`/`0x4922` versus stored `0xa12f`. The bus assumptions remain
unverified. On the current image the same CPU-store branches pass at `out/agent-tests/f2000checksumfix/results.json` and `out/agent-tests/fbf00checksumfix/results.json`; conditional first sector/block erases compute `0x5874`/`0x4950` versus stored `0xa15d` and blank the reset veneer. The new versioned 64-KiB first-block-erased monitor case and the existing 4-KiB first-sector-erased case both reach recovery loop `RAM:ff0300` with `ACC=0` in stock and reconstruction after 5,152,441 states. All 34 monitor cases pass together on the current linked image/map/monitor at `out/agent-tests/m34blockfull/results.json`; the full run was resumed from retained partial results after command time limits. This is conditional monitor decision evidence only, not observed chip effects or monitor-mode reflashing. The correct monitor input for differential cases is the assembled `out/simulator/images/monitor.abs` (SHA-256 `51570409be33d1d9b01dfbd960aec2fb31e136a04fab0e8ceba84148f2a4d486`), not the raw `data/read_memory_ff0000.debug` capture. A raw-capture invocation was discarded after the same Timer0 case passed with the ABS and the complete 29-case monitor run passed.

The earlier apparent `RAM:010000` source was a simulator-harness defect: automatic HDATA relocation scanned an entire compiler listing and rewrote the copy helper’s `0x0010` bank-add immediate to `0x0001`. Restricting relocation to each verified function body, with a synthetic regression test, restores the unmodified `0x0010`; the firmware source arithmetic was reverted. The unmodified first-copy-read trace matches stock at `RAM:100000`, but both executions stop after that read in S251. A standalone CPU probe showed that `MOV R15,@DR8` at `RAM:100000` returns zero and stops even though the debugger can display the mapped byte; at simulator-safe `RAM:010000` it completes and reads the corresponding `X:0000` byte. The new reproducible trace checks each original `0x10` bank immediate, substitutes `0x01` in debugger memory only, seeds ASCII `00` at `X:001a..001b`, and proves matching revision-rejection sense-call arguments. This is declared simulator modeling, not proof of physical controller-window access. Header and auxiliary checksum rejection are conditionally verified through command idle. An equal-byte auxiliary candidate also passes validation and reaches pre-flash handoff on the current image; main-image checksum rejection, effective programming, activation, and recovery remain open.

## S251 differential simulator

- The committed offline metadata at
  `tests/simulator/metadata/W5224A104.json` accounts for all 1,717 canonical
  functions. Case generation and execution require no live Ghidra project.
- The generated deterministic catalog contains 1,717 direct cases, exactly one
  for every canonical function. The corrected aggregate at
  `out/simulator/runs/generated` completed with 1,717 unique passes, zero
  mismatches, and zero errors. Its `results.json` SHA-256 is
  `21a2242ef6ac2ec7430d725448ac8f7fbc365ba25a9f7aa15bc1f4c532efc081`.
  The evidence report classifies 1,716 functions as `smoke` and one as
  `scenario-tested`; these deterministic cases are not proof over all inputs.
  An earlier 1,636-case aggregate remains invalid evidence because its
  dependency-trace overlay crossed a 64 KiB source-mode code bank and corrupted
  simulator state.
- Wine GUI startup on the host X display can spend minutes enumerating the
  6144x2560 multi-monitor mode table. Running Wine and µVision on a fixed
  1024x768 Xvfb display avoids that external startup failure. The former
  `RAM:f9614e`, `RAM:f96729`, and later display-disconnection errors all passed
  when retried under the long-lived isolated display.
- `python3 tests/simulator/run_all.py data/W5224A104/MONT_104.bin` is the
  stock-only entry point. It validates the pinned metadata/image identity,
  configures and serially builds the 446-object firmware, runs headless smoke
  and committed feasibility cases, runs/resumes all 1,717 generated direct
  cases, and emits JSON, text, and JUnit reports. It explicitly disables the
  Ghidra authoring path.
- A separate runtime-monitor capture is optional. The current complete direct
  suite does not require the ignored local `data/read_memory_ff0000.debug`;
  cases declaring `load_monitor: true` require an explicit `--monitor` input.
- The committed feasibility suite passes both the full-firmware arithmetic
  boundary and an integrated SCSI command-completion scenario that executes
  its three real callees. Headless stock-only S251 smoke also passes.
- The runner executes stock and reconstructed images independently, compares
  termination, ABI outputs, stack boundaries, RAM deltas, normalized
  dependency calls, and declared hardware transcripts, and emits JSON/JUnit.
- Dependency shims now support ABI-normalized arguments and return values,
  relocated code pointers, near-RAM side effects, and BITS side effects.
  Direct cases deterministically initialize every directly referenced internal
  bit below `BITS:0080`; explicit case overrides merge by bit address. Both
  stock RET and reconstructed ERET launches establish a safe stack baseline
  before calls, preventing trace-recorder pushes from overwriting bit-addressable
  internal RAM. The aggregate trace overlay omits an unused B save/restore and
  is linker-constrained to `RAM:f00000..RAM:f0ffff`; overflow now fails at link
  time instead of silently crossing the source-mode bank.
- Interrupted catalog runs resume only when the runner, µVision executable and
  project, full normalized case content, images, map, metadata, optional
  monitor, and deterministic dependency-shim source identity all match.
  Keil ABS container bytes are not used for shim identity because their
  metadata can change across equivalent rebuilds.
- Keil `rwatch`/`wwatch` signal functions provide deterministic SFR/controller
  reads and ordered write capture. Direct SFR operands must compile as named
  `sfr` accesses; a generic near pointer can silently become an ordinary RAM
  load and is not equivalent.
- Differential execution has found and corrected source defects in the zero
  sample loop, parsed-Q zero-countdown branch, sample-series direct-SFR access,
  the media-read delay byte (`RAM:00f9e0`, not `RAM:00f9df`), and the status
  mapping event-byte placement (bits 31..24 rather than bits 15..8).
- The current mismatch-reduction pass has also corrected several inherited
  register contracts, local count-pointer calls, signed/divide-by-zero edge
  behavior, descriptor address-width mistakes, controller-address accesses,
  a high-word decrement, and a firmware-specific logarithm edge case. The
  latest verified work clears `RAM:fb5803`, `RAM:fb5c02`, `RAM:fb65d8`,
  `RAM:fb7f5b`, `RAM:fb7ffa`, `RAM:fb7ffe`, `RAM:fb8319`, `RAM:fb9a54`,
  `RAM:fb9b05`, `RAM:fb9b1b`, `RAM:fbb2c5`, `RAM:fbd328`, `RAM:fbd39d`,
  `RAM:fbf750`, and `RAM:fbfbb9`.
- The latest targeted fixes correct reversed floating-point subtraction in two
  adjustment paths, a ROM-table address-space error, an unsigned bounds test,
  and a reversed range-classifier ABI call. Valid input-domain cases also clear
  five false mismatches caused by null pointers aliasing internal CPU storage
  or by a compiler-local stack-buffer address. The affected direct cases are
  `RAM:f97c26`, `RAM:f97d05`, `RAM:fa6560`, `RAM:fbdcde`, `RAM:fbdef0`,
  `RAM:fbe343`, `RAM:fc02b2`, `RAM:fc3fce`, `RAM:fc42c9`, `RAM:fc549f`,
  `RAM:fc59df`, `RAM:fc65c3`, and `RAM:fd365b`.
- All cases 360..379 pass after targeted replays. Those replays established
  valid input-domain overrides, shared-tail return-kind inference, explicit
  reconstructed ABI mappings, and a corrected controller-profile table
  selection in `LookupControllerProfileAuxiliaryValue`. Pure compiler
  arithmetic helpers are kept integrated rather than being misclassified as
  non-returning hardware dependencies.
- The `RAM:f9f5a1` dependency code-pointer case passes after declaring its
  canonical code-pointer parameter. The installed S251 simulator does not
  faithfully access source-mode high HDATA, so the runner relocates declared
  HDATA banks to simulator-safe storage in both images and can find bank
  constants in private helpers across a whole C251 listing module. Dependency
  traces use a 16-bit count and support 1,023 records. Deterministic near-RAM
  hardware-read events and per-case wall-clock limits are supported.
  `RAM:fa0000`, `RAM:fa05ef`, `RAM:fa0ab4`, `RAM:fa1341`, `RAM:fa1ac5`,
  `RAM:fa20f3`, and `RAM:fa24e9` now complete but expose a recurring cursor
  placement mismatch between stock and reconstructed record acquisition.
  The shared `0x19` HDATA helper at `RAM:fa56c5` and `RAM:fa5c30` now passes
  after declaring and regression-testing its relocation. The direct sweep now
  also handles encoded one-byte HDATA-bank ORL operands, shared-tail dependency
  RET sites, caller-specific reconstructed dependency symbols, inherited B
  arguments, exact interleaved SFR read sequences, interrupt-driven countdown
  side effects, and stale memory-overlay provenance checks.
- `NaturalLogFloat32(0)` now reproduces the firmware-specific value
  `0xc08bbbaa`; the dependent zero-input computation returns the canonical
  `-3792` result.
- The simulator harness, flash model, and trace recorder currently have 134 passing Python tests. The final
  evidence report accounts for exactly 1,717 functions with zero untested,
  blocked, or mismatched functions. Generated artifacts remain under
  `out/simulator/` (historical development runs remain under
  `out/agent-tests/`); all version-controlled simulator implementation,
  metadata, cases, and documentation stay under `tests/simulator/` except the
  minimal root CMake inclusion.

- Integrated-gate work added strict simulator stop/progress checks and a reset
  jump launch in `tests/simulator/simulator.py`. The versioned
  `tests/simulator/cases/integration.json` now passes reset vector to initial
  coroutine configuration (72 stock / 63 reconstructed states), reset through
  four coroutine setups to `ResumeCoroutine0` (443 / 476), coroutine-0 startup
  and yield to `ResumeCoroutine1` (391,572 / 57,512), and complete unsupported-
  SCSI dispatch to sense/completion (307 / 340), and a four-byte INQUIRY
  through copied payload, modeled data-in completion, and command return
  (1,259 / 1,369). A new two-call dispatcher case runs unsupported opcode
  then an 18-byte REQUEST SENSE in one execution (2,265 / 2,377 states).
  It verifies the first return PC and stack before changing the CDB, then
  compares the full `70 00 05 ... 24 00` sense response, the timed data-in
  event, final stack, and RAM/controller deltas. A further integrated case
  starts `ScsiCommandProcessorCoroutine0` at `RAM:fdbe7d`, consumes twelve
  modeled controller-window bytes, admits and dispatches INQUIRY, transfers
  four response bytes, publishes status, and reaches the idle-loop edge
  (3,772 / 3,805 states). Both images produce `[05, 80, 00, 21]` at
  `RAM:010000`, reset `RAM:000c32` to one, and match 20 RAM/controller deltas
  and 17 hardware events, including command-code writes `0x80` then
  `0xc0`. Two further cases resume that coroutine with a second packet:
  INQUIRY then unsupported opcode (4,271 / 4,629 states) retains the first
  payload, and unsupported opcode then REQUEST SENSE (5,298 / 5,328) yields
  the full `70 00 05 ... 24 00` sense response. Both verify the first
  completion PC and balanced stack and record two `0xc0` publications.
  A READ BUFFER mode-3 packet now yields `[00, 00, 10, 00]` capacity
  descriptor/controller payload and two command-code writes, reaching the
  idle-loop edge in 4,839 / 4,895 states. A WRITE BUFFER mode-4 staging
  overrun (`0x0ff0 + 0x20 > 0x1000`) returns matching `05/24/00` sense with
  transfer state zero in 612 / 873 states and never enters the runtime loader.
  A versioned monitor-loaded physical-reset case enters the actual
  `ScsiCommandProcessorCoroutine0` in 471 stock / 502 reconstructed states.
  It requires `SP=0`, saved scheduler SPX `00 00 00 0a` at
  `RAM:00115a..00115d`, and initial coroutine-zero saved SPX
  `00 00 03 07` at `RAM:00115e..001161`; the matching result is
  `out/agent-tests/integration-gates/reset-to-scsi-coroutine-entry-versioned-run/results.json`.
  A disposable attempt to inject a WRITE BUFFER CDB immediately at that
  entry reached controller setup but not the next scheduler call within
  one million states. Packet timing must be established after the command
  coroutine's startup yield; this is not accepted staging evidence.
  A positive mode-4 packet now uses the authentic scheduler SPX and
  internal stack captured at physical reset. The versioned
  `write-buffer-mode4-valid-yield-returns-scheduler` case passes at
  `out/agent-tests/m4yieldv/results.json`: both images admit offset `0x10`,
  length 4, prepare DATA OUT, and yield through `ResumeCoroutine1` with
  scheduler `SPX=0x0a`. The reproducible controlled coroutine-zero resume
  at `tests/simulator/trace_mode4_staging_resume.py` reaches DATA OUT
  transport with `ACC=0x3b`, `DR0=4`, `DR4=0x10`, then the DATA OUT helper
  with `RAM:000c33=8` on both sides. The trace then declares payload
  `11 22 33 44` at `RAM:100010..100013` and completion status `0x0d`;
  both images configure length 4 and destination `0x10`, then reach the
  DATA OUT helper return with matching buffer bytes, status and cleanup.
  Corrected result: `out/agent-tests/m4resume-staging-e/results.json`. A further controlled continuation reaches the WRITE BUFFER handler return and command idle with packet state 1, transfer state 5, no sense, intact modeled controller-window bytes, and matching command-code writes `[0x80, 0xc0]`: `out/agent-tests/m4complete-staging-e/results.json`. These results are on the preceding image and do not model physical DMA.
  A packet-ingress READ CAPACITY(10) case now declares media ready,
  `RAM:0012c1=0x00001000` capacity and `RAM:000b00=0x0800` block size.
  Both images return `[00, 00, 0f, 69, 00, 00, 08, 00]` in the response
  and controller payload, then process unsupported opcode `0xff` as a second
  packet with `05/24/00` sense, packet state one and command-code writes
  `[0x80, 0xc0, 0xc0]` (5,310 / 5,648 states). The preceding fourteen-case catalog passes on the prior image at `out/agent-tests/integration-gates/fourteen-controller-xdata-fix-run/results.json`.
  A versioned current-image one-block READ(10) packet-ingress case now
  executes its first authentic `YieldCoroutine0` into a captured scheduler
  context (`SP=10`, scheduler SPX `0x0000000a`) with decoded LBA zero, one
  block, raw-read mode two, target frame 150, and 512-slot ring limits
  `01 ff 01 fa` (`out/agent-tests/read10-scheduler-yield-versioned-xvfb/results.json`).
  The direct launch keeps `SP=0x40` above persistent bit-addressable RAM;
  a prior shallow stack fixture corrupted `RAM:0022`, not firmware source.
  A controlled real scheduler `ECALL ResumeCoroutine2` enters the primary
  raw-read coroutine at stock `RAM:f90000` / linked `RAM:fde81b` and reaches
  its first `YieldCoroutine2`, with matching target, ring and pipeline state
  (`out/agent-tests/read10-media-first-yield-ecall-final-xvfb/results.json`).
  Both images then return to the scheduler ECALL successor, naturally reach
  the next `ResumeCoroutine0` call, enter batch service and reach a second
  command `YieldCoroutine0` with 0/1 blocks complete
  (`out/agent-tests/read10-wait-cycle-final-xvfb/results.json`). An earlier
  direct jump to the ResumeCoroutine2 function omitted its ECALL return
  frame and produced invalid later PCs; the corrected call-site trace
  supersedes it. At that stage, the integrated suite defined 21 cases: 20 passed
  together on the preceding image, and the READ(10) scheduler-yield case
  passed separately. The preceding complete 23-case result is recorded above; the current 24-case result is recorded in the current-image summary. Conditional descriptor transfer, interrupt completion, command
  idle and a prebuffered second packet pass in separate retained traces;
  sector payload and physical packet ingress remain open. A new diagnostic reset case reaches
  the third `ResumeCoroutine2` call in both images using controller-ready,
  SFR:D7 and timer-owned RAM countdown responses. It records matching
  164-event hardware-read/timer transcripts (stock 464,275 / reconstruction
  136,165 states) at
  `out/agent-tests/integration-gates/reset-third-resume-dual-watch-timers-run/results.json`.
  Fourth-resume diagnostics found and fixed two source defects. Canonical
  `ResetRawReadQueueAndTransferState` (`RAM:f927bd`) reads the full 32-bit
  slot count at `RAM:00156a` before storing the low words of count minus
  one and six at `RAM:00129a`/`RAM:00129c`; the old C read only the upper
  half. Both images now produce `01 ff 01 fa` for count `0x00000200`.
  Canonical `ConfigureMainDataRingGeometry` (`RAM:fdbdff`, instructions
  `RAM:fdbe42..fdbe64`) writes the ring size low, middle, high to controller
  `RAM:018013..018015`; the old C wrote high, middle, low. Focused host tests
  and direct stock/reconstruction cases pass for modes one (`00 30 09`)
  and two (`00 60 12`). On a prior image, a versioned fourth-resume case reaches stock
  `ResumeCoroutine3` `RAM:fee51e` and its linked counterpart with 166 matching
  modeled events and exact live coroutine return-PC assertions. Stock and
  reconstruction take 468,297 / 294,939 states. The saved coroutine-one
  and two PCs are the immediate successors of their yield calls in both
  images, and the saved coroutine-three pointer still names its entry.
  A second versioned case stops twice at stock `ResumeCoroutine0`
  `RAM:fee4fd` and its linked counterpart, proving one full 0/1/2/3
  scheduler cycle after reset (468,465 / 295,091 states). Its saved
  coroutine-three PC is the immediate successor of `YieldCoroutine3` in
  each image. The two cases require exact pointer values, matching
  scheduler/coroutine SPX, RAM changes, and 166 ordered modeled events,
  including controller writes `RAM:018046=0x02` then `RAM:018013=0x00`.
  Only memory above saved coroutine SPX and the S251 final controller
  `RAM:018046` snapshot are excluded; the latter differs despite the same
  write-watch event in both images. Definitions are versioned in
  `tests/simulator/monitor-cases/integration.json`. Retained results are
  `out/agent-tests/integration-gates/reset-fourth-resume-exact-context-run/results.json`,
  `out/agent-tests/integration-gates/reset-second-loop-resume0-corrected-run/results.json`,
  and the passing 21-case earlier-image suite at
  `out/agent-tests/integration-gates/monitor-twentyone-scheduler-cycle-corrected-run/results.json`.
  The former second-occurrence `ResumeCoroutine3` stack mismatch was fixed:
  coroutine zero now yields directly with saved `SPX=RAM:000307` in both
  images. The next third-`ResumeCoroutine0` checkpoint passes, proving two
  complete scheduler cycles. A monitor Timer0 request at the second
  `ResumeCoroutine0` checkpoint, while coroutine zero is yielded, returns
  and reaches the third checkpoint with one monitor tick and matching context.
  The paired declared-main-selector Timer0 case also passes with one main
  phase/tick and no monitor tick. Other active-scheduler interrupt sources
  remain open. A fourth-`ResumeCoroutine0` probe intended to prove three cycles
  stops at `RAM:000818` in both images. The complete `0x28e`-byte code snapshot
  at `RAM:000800` matches the captured configuration flash programmer, and
  controlled stock resumes reach repeated `RAM:000872`, where canonical
  runtime code polls a flash read's bit 7. The retained stock register
  capture has `DR28=RAM:f7a000` at the repeated poll; `RAM:00086d` copies it
  to `DR16` before the read. Both images contain only `0xff` in the
  `RAM:ffa000..ffafff` sector under the inferred `+0x80000` execution alias,
  but the physical alias and flash response are unmeasured. This is a shared
  simulator flash-response boundary, not a proven reconstruction mismatch or
  physical write. A corrected paired conditional `0xff` poll-response probe now reaches
  `RAM:00090e` in both images after the `RAM:000872` poll. Its repeatable
  script is `tests/simulator/trace_configuration_flash_poll.py`, with result
  `out/agent-tests/integration-gates/config-flash-poll-paired-recorded-run/results.json`.
  The 654-byte relocated bodies match the immutable 1.04 monitor capture.
  One reconstructed debugger PC sample at `RAM:000818` is zero in that run,
  while a separate manual trace captured it; the later stops from
  `RAM:00081f` through `RAM:00090e` match. This is conditional simulator
  progress, not a measured chip reply, completed programming, or a third
  scheduler cycle. A repeatable first-batch trace at
  `tests/simulator/trace_configuration_flash_first_batch.py` passes for the
  preceding image: both sides complete 31 direct-memory modeled word iterations,
  with matching PC phases, `RAM:f7a060..f7a09d` bytes, source cursor from
  `RAM:001680`, target progression, and count. Result:
  `out/agent-tests/integration-gates/config-flash-first-batch-corrected-run/results.json`.
  A new paired Tstep trace at
  `tests/simulator/trace_configuration_flash_return.py` and
  `out/agent-tests/integration-gates/config-flash-final-word-return-run/results.json`
  proves all 32 direct-memory modeled words match the 64 source bytes. Both
  follow the same relocated PC sequence through `RAM:000a8d ERET` with
  `ACC=1`; stock resumes at `RAM:fd649f`, reconstruction through adapter
  `RAM:f800d9` and caller `RAM:f83f59`. The earlier `G` result `0x55547e`
  occurs later, not at the final word. Bounded caller chunks show both
  traversing configuration verification and startup work before PC zero
  (stock 515,695; reconstruction 352,939 states), retained at
  `out/agent-tests/integration-gates/config-flash-caller-chunks-probe/results.json`;
  the fourth scheduler checkpoint is still unverified. A latest-image paired
  continuation at `out/agent-tests/ccrfix/results.json` follows both returns
  from `ProgramAndVerifyDriveConfiguration` with `ACC=1` into the caller and
  startup coroutine. The reconstructed source had an extra helper call frame
  across `YieldCoroutine1`; removing the helper call and compiling direct
  yields in `RunStartupStatusCoroutine1` made both saved coroutine-one SPX
  `0x507` and scheduler SPX `0x0a` at
  `out/agent-tests/csyfix/results.json`. A separate paired trace at
  `out/agent-tests/csofix/results.json` shows the relocated programmer at
  `RAM:000800..000a8d` overwrites the coroutine-three saved return bytes
  `RAM:000900..000903` in the S251 direct-memory model; the next
  `ResumeCoroutine3` returns to invalid `RAM:55547e` in both images. Physical
  reset, coroutine reinitialization, and RAM banking need resolution before
  assuming that model predicts a drive crash or requiring the fourth
  natural `ResumeCoroutine0`. A controlled latest-image continuation injects
  only the four return bytes and saved SPX that `InitializeCoroutine3` writes
  at the next scheduler `ResumeCoroutine3`; both images then reach the fourth
  `ResumeCoroutine0` with SP10, coroutine-one SPX `0x507`, coroutine-three
  SPX `0x907`, and `RAM:001009=3` at
  `out/agent-tests/c3reinit_final/results.json`. The path has no natural
  coroutine-three reinitialization before that call. This isolates the
  immediate simulator liveness effect but does not prove physical reset,
  banking, or a natural fourth cycle. Physical busy/toggle and alias remain
  open. An earlier
  36-byte INQUIRY probe's 16.7-million-state result was invalid: a 36-byte observation overwrote the old fixed state
  record. The harness now allocates that record after observations. A
  disc-update rejection probe stops at entry under S251 and is not accepted
  recovery evidence. Two mode-5 WRITE BUFFER rejections through the real
  handler pass: invalid buffer ID 3 (587 / 918 states, matching command-
  sequence-error sense) and a main-image request while `RAM:000c9e=1` and
  `RAM:000fae=0` (575 / 885 states, matching `04/40/F6` sense). Both cases
  forbid execution at the captured loader `RAM:ff4000` and relocated
  programmer `RAM:000800`; a disposable handler-to-loader jump stopped at
  `RAM:ff4000` and failed, establishing that the trap fires. This does not
  replace a flash-event log or cover staged header/revision/checksum rejection.
  A versioned optional monitor case now passes an isolated operation-5
  loader-to-first-unlock checkpoint in both images (12,166 states each): the
  captured loader copies `0x40e` bytes into `RAM:000800`, recognizes a declared
  `0x2000` flash ID, and reaches `RAM:00082b` with `DR16=0xf0aaaa` and
  `WR24=0xaa00`. The retained result is
  `out/agent-tests/integration-gates/runtime-operation5-versioned-run/results.json`.
  The formerly stalled one-shot later checkpoint was an S251 stop after a
  mapped flash-window store. A reproducible controlled-resume tool at
  `tests/simulator/trace_runtime_flash.py` now records 14 matching CPU stores
  in the declared `0x2000` operation-5 branch for stock and reconstruction,
  through post-store PC `RAM:0008c5`. The retained result is
  `out/agent-tests/integration-gates/operation5-2000-cpu-store-ring-order-pc-pair/results.json`.
  Stores are the five `AA/55/80/AA/55` unlock/setup words at
  `RAM:f0aaaa`/`RAM:f05554`, followed by `0x3000` at programming
  `RAM:f00000..f60000` in 64-KiB steps, `RAM:f78000` and `RAM:f7c000`.
  The current trace uses a 32-byte record with separate state/SP fields and two PC samples; the previous 24-byte
  layout overwrote one state-counter byte with SP and falsely reported a
  final counter drop. Current stock/reconstruction counters rise from
  12,166 to 12,292 and PC/address/word/SP match. These are CPU stores,
  not confirmed physical flash effects. A disposable
  attempts to continue from the erase stores toward status-poll
  `RAM:000906` did not produce a validated poll checkpoint: one continuation
  stopped at PC zero, and a two-image breakpoint variant failed the 14-store
  prefix check. The existing 14-store trace remains the last accepted
  programmer observation. Further progress needs an explicit flash-status
  read model and controlled debugger continuation, with conditional chip
  behavior kept separate from CPU execution.
  The second versioned case declares `RAM:0000f8=0xbf00`, selects the
  alternate branch and reaches its first unlock edge `RAM:0008d6` in both
  images (12,176 states each), with result at
  `out/agent-tests/integration-gates/runtime-operation5-bf00-versioned-run/results.json`.
  Controlled resumes now also record six matching `0xbf00`-branch CPU stores,
  ending with `0x5000` at programming `RAM:f00000` and post-store
  `RAM:0008f9`, at
  `out/agent-tests/integration-gates/operation5-bf00-cpu-store-ring-order-pc-pair/results.json`.
  The physical drive's device selector, bus byte lane and LC898095 alias are
  unobserved. The Microchip SST39LF400A datasheet requires six writes per
  erase and says commands during internal erase are ignored. Under the
  explicitly assumed high-byte command lane and `+0x80000` alias, the first
  complete six-store command at programming `RAM:f00000` conditionally erases
  execution `RAM:f80000..f80fff`, including reset veneer `RAM:f80050`.
  A disposable image replay changes 3,682 non-FF stock bytes and produces
  checksum `0xf55a` versus stored `0xb3bd`; an earlier reconstruction changes
  4,010 bytes and produces `0x34fe` versus stored `0x782d`. The `0xbf00`
  branch's conditional `0x5000` block erase instead covers execution
  `RAM:f80000..f8ffff`, changes 63,469 stock or 65,136 earlier reconstructed
  bytes, and computes `0x481a` or `0x1fc9` against the respective stored
  checksum (`0xb3bd` stock, `0x782d` earlier reconstruction).
  Both conditional first effects wipe the reset veneer, and the operation-5
  allowed-range guard rejects each. The `0x2000` branch's next bare `0x3000` at
  `RAM:f10000` is not a complete SST erase sequence in the strict model;
  later physical effects cannot be inferred from this trace. The earlier
  conditional `RAM:f78000` calculation remains a possible effect only if
  that raw store is accepted as a command. Flash programming and operation-5
  activation/recovery are not validated. The seven dedicated flash-model,
  125 simulator, and one trace-record Python tests pass together (133).
  The retained traces in this historical comparison use reconstructed
  checksum `0x782d`; the new-image replay is summarized above.
  An earlier seven-case baseline passed at
  `out/agent-tests/integration-gates/final-seven-run/results.json` after an
  idle Wine simulator service was restarted. Before that restart, UV4 returned
  zero without running any case initialization script or writing result files,
  while the independent S251 smoke target still passed; those empty runs are
  invalid evidence, not firmware failures.
  `docs/FLASHABILITY_INTEGRATION_GATES.md` records the four acceptance gates.
  The S251 harness now supports state-timed RAM/SFR/controller writes, checks
  their event trace, and bounds missed checkpoints with a debugger signal. A
  RAM timed-write differential case passed. An initial Timer0 probe reached
  `RAM:ff3fe0` while HEX-loaded router bytes remained `0xff`; that invalid
  checkpoint was discarded. A reproducible A251/L251 builder now links the
  immutable 64-KiB monitor capture as a separate ABS. The versioned optional
  Timer0 scenario passes from reset through the captured router, main handler,
  and authentic RETI to the saved reset PC: 507 stock / 541 reconstructed
  states; matching `SP=7`, timer phase/tick, RAM changes, and timed-event trace.
  The generic S251 Timer0 vector uses an explicit four-byte alias to the
  physical monitor router in this offline hardware model. A versioned prior-image
  scenario injects one pending `SFR:88=0x20` flag with TR0 off at state 1,000
  during coroutine-zero `ReadDriveConfigurationBytes`, then reaches the next
  `ResumeCoroutine1` call. Stock/reconstruction show 395,761 / 215,924
  states, one timer phase/tick at `RAM:001231`/`RAM:001232`, `SP=10`,
  `SFR:A8=0x82`, `SFR:88=0`, `SFR:A9=0`, and matching memory/event traces.
  The case requires those exact observed values and passes at
  `out/agent-tests/integration-gates/boot-coroutine0-single-timer0-expected-run/results.json`.
  It is retained in `tests/simulator/monitor-cases/integration.json`; a
  monitor-mode waiting-coroutine Timer0 case now passes through the third
  scheduler checkpoint with `RAM:0011ce=1`, `SP=10`, cleared `SFR:88`,
  matching saved coroutine-zero PC/SPX, and 166 matching modeled events
  (469,238 / 295,995 states). A paired case explicitly selects the main
  handler at that checkpoint and passes with one main phase/tick, no monitor
  tick, and continued scheduler progress (469,413 / 296,205 states).
  A second selector-
  zero Timer0 case passes through the monitor handler at `RAM:ff03a6`, updates
  `RAM:0011ce`, and executes RETI back to the saved reset PC with `SP=7`
  (332 stock / 333 reconstructed states). The captured `INT0` controller-
  event vector and `INT1` controller-packet vector also pass through their
  monitor-mode handlers and authentic RETI with `SP=7` (329 / 330 and
  228 / 229 states); the event case increments `RAM:0011cd` and writes the
  controller acknowledgement registers. A further main SFR-E1 router
  case with an explicit synthetic Timer0-vector alias clears `SFR:E1` bit 7,
  executes authentic `RETI`, and restores `SP=7` at the interrupted reset PC
  (259 / 320 states). It does not prove actual E1 source delivery. Seventeen
  optional monitor cases have targeted passing results in total, including the two bounded programmer-
  branch checkpoints and two host-interface router cases. S251's generic
  Timer0 source enters the captured physical host vector `RAM:ff000b` without
  a patch. With monitor selector, the handler writes controller
  `018059/01805b/018059 = 0x17/0x37/0x97` and RETI restores `SP=7`
  (376 / 377 states). With main selector and zero cause, the main handler
  records `RAM:00faeb=1`, writes `018059/018002/01805b/018059 =
  0x17/0/0x37/0x97`, and RETI restores `SP=7` (370 / 353 states). The
  retained two-case result is
  `out/agent-tests/integration-gates/host-vector-versioned-run/results.json`.
  Generic Timer0 injection does not prove physical host IRQ delivery or
  pending-cause behavior. Two main-mode `INT1` cases now pass through the
  captured router and authentic `RETI` to the interrupted reset PC with
  `SP=7`. The short-response branch uses declared `RAM:01808e=0x40` and
  ready `RAM:018015=0x20`, publishes `RAM:01808c=0x87` and
  `RAM:00125a=0xc0`, and takes 351 / 389 states. The other branch uses
  `RAM:01808e=0` and `RAM:01808d=0xa5`, captures ten `0xa5` bytes at
  `RAM:00124f..001258`, sets `RAM:0011d9=1` and `RAM:00125a=0x80`, and
  takes 680 / 740 states. Results are retained at
  `out/agent-tests/integration-gates/controller-packet-main-ready-versioned-run/results.json`
  and `out/agent-tests/integration-gates/controller-packet-main-ten-byte-versioned-run/results.json`.
  S251 far-controller read/write watches reported no events despite the
  observed RAM delta in the short-response case, so neither case proves an
  ordered controller-bus transcript. A versioned main-mode physical INT0
  case passed on the preceding image: all-high declared controller status
  enters the captured router, reaches authentic `RETI` and the interrupted
  reset PC with `SP=7`, records `RAM:00122a=0xfe` and `RAM:001097=0xff`, and
  has two matching controller read events (361 stock / 407 reconstructed
  states). The retained result is
  `out/agent-tests/integration-gates/controller-event-main-no-pending-versioned-run/results.json`.
  Stock requires S251's default `X:` read watch at these two sites and the
  linked route requires `watch: "RAM24"`; the earlier all-high probe stopped
  inside `StrobeControllerPortWriteByte` because its watch placement missed
  one side. A second versioned physical INT0 case passes with active-low bit
  one at `RAM:018003=0xfd` and `RAM:000fad=0x02`: both clear the pending bit,
  set `RAM:001302=0x20`, retain mask `RAM:00122a=0xfc`, and return through
  `RETI` with `SP=7` (381 / 427 states). Its result is
  `out/agent-tests/integration-gates/controller-event-main-bit1-pending-versioned-run/results.json`.
  Two more versioned reset-to-scheduler physical INT0 cases pass on the same
  preceding image. With coroutine zero yielded at second `ResumeCoroutine0`,
  the declared main-handler pending-bit event returns and reaches the third
  resume with saved contexts, `SP=10`, `RAM:001302=0x20`, and 168 matching
  modeled events (469,287 / 296,089 states); result:
  `out/agent-tests/integration-gates/boot-main-int0-bit1-while-coroutine0-yielded-versioned-run/results.json`.
  At state 1,000 of active coroutine-zero configuration reading, INT0 likewise
  returns and reaches `ResumeCoroutine1`, with pending bit cleared, flag
  `0x20`, `SP=10`, `SFR:A8=0x83`, `SFR:88=0x01`, no Timer0 tick, and five
  matching events (395,635 / 215,820 states); result:
  `out/agent-tests/integration-gates/boot-coroutine0-main-int0-bit1-active-versioned-run/results.json`.
  The monitor suite now has 29 versioned cases: 25 passed together on the
  preceding image and four new INT0 cases passed individually. Ordered writes,
  other pending event branches, host pending causes, and actual host/E1 source
  delivery remain open.
  The full 21-case earlier-image monitor suite passes at
  `out/agent-tests/integration-gates/monitor-twentyone-scheduler-cycle-corrected-run/results.json`.
  Monitor bootstrap reaches normal handoff `RAM:ff3f7d` on both intact images
  (5,164,567 states each), rejects a one-byte bad checksum to recovery entry
  `RAM:ff0300` (5,152,441 each), and honors declared forced input on both
  intact images (15,839 each). S251 initially exposed the stock stored checksum
  word at `RAM:ff9ffe` for both images despite distinct full-image sums
  (`0xb3bd` / `0x782d`); the cases map the proper word from each tested image.
  Forced input pauses at `RAM:ff4e72`, validates that PC, applies
  `SFR:P0/P1` latch and `PORT0/PORT1` pin values `0x46/0x08`, and resumes.
  Two further cases use a checksum-valid `EJMP RAM:f80050` self-loop entry;
  independent container verification gives `0xb291` / `0x756c`. Normal
  bootstrap accepts both, while forced input reaches `RAM:ff0300` before
  executing the loop. Current-image monitor results are retained at
  `out/agent-tests/integration-gates/monitor-current-boot-decisions-run/results.json`,
  `out/agent-tests/integration-gates/monitor-forced-recovery-current-run/results.json`,
  and `out/agent-tests/integration-gates/monitor-current-self-loop-decisions-run/results.json`.
  A versioned conditional interruption case fills main
  `RAM:f80000..f80fff` with `0xff`, preserves both stored checksum words,
  and reaches recovery `RAM:ff0300` with required `ACC=0` in 5,152,441
  states per image at
  `out/agent-tests/integration-gates/monitor-erased-reset-sector-versioned-run/results.json`.
  This proves a monitor decision for one modeled damaged state, not physical
  flash repair. Flash programming, activation, full scheduler/command integration, and
  physical switch recovery remain untested.

## Retained assembly boundaries

Five `.a51` units remain, with 72 exports total:

- `arithmetic_boundaries.a51`: 63 hidden-register captures, register remaps,
  stack/reset transfers, unsupported RXDAT.3 operations, native floating ABI
  glue, and monitor/runtime calls.
- `return_boundaries.a51`: one inherited-WR6 near-return boundary.
- `configuration_runtime.a51`: one RAM:ff4500 monitor programmer call.
- `vector_veneers.a51`: six fixed four-byte reset/interrupt jumps.
- `firmware_ram_window.a51`: one allocation-free absolute symbol for the
  physical bank-zero near-data window.

Direct bit operations and ordinary typed call wrappers are C251 source.
Relocatable assembly boundaries use the far call model and `ERET`; only
explicitly bank-local native boundaries retain near-return behavior.

## Verified working baseline

- Keil C251 5.60 and A251 4.69.13 run under Wine.
- The entry, vector, coroutine context-switch, compiler stack-transform, and
  retained boundary units compile with zero diagnostics in their focused
  checks; listing review confirms the intended RET/ERET/RETI and register/SP
  operations.
- All 270 existing host/format tests pass, as do the firmware image validation
  and simulator harness test, for 272/272 configured CTest entries.
- The repository-wide readability pass is host-validated. The firmware map
  now centralizes near RAM, extended RAM, controller, SFR/BITS, and
  ROM/configuration locations; direct generic memory accesses use named
  operands. Domain `.c`
  files contain no `PLEXTOR_HOST_TEST` or `Observe...` code: host/target
  substitutions live under `src/platform/bindings/`. Signed reads and near
  bit set/clear operations are platform-memory macros. Named near-address
  constants preserve the original unsigned-long literal type because C251
  code generation is sensitive to integer promotion. A few explicit 16-bit
  casts and file-local adapters remain where the O5/SIZE listing proves they
  are required for established instruction and layout parity.
- The repository `.clang-format` is wired to `format` and `check-format`
  targets, all tracked C/header line endings are LF, `check-format` and
  `git diff --check` pass, and every target C unit preprocesses independently.
- The readability pass has passed its target-equivalence gate with the local
  Wine/Keil installation. A clean pre-refactor rebuild and the refactored tree
  each compile and link 446 objects at O5/SIZE with zero diagnostics. All 446
  normalized emitted instruction/data and relocation listings match; segment
  totals and linker placement match; and both produce checksum `0x6d86` and
  SHA-256
  `563ee1634a7a061e070de5305c508de384355257d0219e8b56b13d7a4db45ad8`.
  See `docs/SOURCE_READABILITY_BASELINE.md`.
- The build wrapper handles Wine invocation, path translation, project include
  paths, and Keil object/listing output locations.
- The practical C251 5.60 baseline is now `MODSRC ROM(HUGE) LARGE REGPARMS
  PARM251 OPTIMIZE(5,SIZE) CODE`. A 19-function corpus brackets the original
  style at O4/O5: O5 preserves two substantial bodies exactly, while O6 loses
  both matches and O7 diverges sharply. This is a reconstruction default, not
  identification of the original compiler options. The complete
  `status_mapping.c` unit and all seven units that formerly overrode O8 compile
  at O5/SIZE with zero diagnostics. No firmware C translation unit now carries
  a different optimizer setting.

Read-only Windows device discovery on 2026-09-25 found a stock
PX-W5224A revision 1.04 at `H:`. The native SPTI extension to
`tools/probe_readonly_w5224a.py` issued only its six fixed DATA-IN CDBs after
INQUIRY identity validation. `RAM:f80050` matched the immutable stock bytes,
`RAM:f00050` matched the same 16 bytes, Mode Page 03 matched MODE SENSE(10),
and `RAM:0000f8` returned `bf00`. Reads at `RAM:f70000`, `RAM:f72a3e`, and
`RAM:f78000` also matched `RAM:ff0000`, `RAM:ff2a3e`, and `RAM:ff8000`,
respectively, including immutable runtime-capture and fixed-data bytes. Exact
status, lengths, CDBs, response bytes, and full-window backups are in
`out/agent-tests/windows-readonly-w5224a-full-backup-20260925/`.
Canonical stock Ghidra instructions at `RAM:fcb62b`, `RAM:fcb730`,
`RAM:fcb73c`, `RAM:fcb832`, and `RAM:fddd8e` independently confirm selector,
address, length, transfer, and source-byte semantics (`out/agent-tests/f5_static/`).
The captured monitor's F5 table entry `RAM:ff7dd4` routes to `RAM:ff2a3e` and
parses the same wire-CDB fields; its self-read checkpoint remains unexecuted
because the drive was not reset into recovery mode. The normal-mode result
does not prove flash write routing, programming-state P3 mapping, SST byte-lane
behavior, or reflashing.

A focused post-failure startup audit found one concrete instruction-level ABI
mismatch at the flash-identification handoff. Stock 1.04 writes
`0x00ff5000` to `RAM:00130e`, reloads `DR8` from that slot at `RAM:fdc535`,
and executes `ECALL @DR8` at `RAM:fdc539`. The reconstruction previously used
a direct `ECALL RAM:ff5000`; its addressless adapter now performs the same
`MOV DR8,EDATA 130EH` plus `ECALL @DR8`. A serial Windows firmware rebuild has
zero assembler/linker diagnostics, and its listing contains the intended
indirect call. The derived image is SHA-256
`e2afa26190203d0a7407e91033dbb21b0bb5c3fa5f7f41f9048c53cbef7e631b`,
checksum `0x14d6`, with stock fixed data intact; the three focused firmware
image/initializer tests pass. This is a real fidelity correction but is not
yet proved to be the physical boot blocker and is not flash-ready.

A first-pass LED cold-boot diagnostic is now available without changing the
ordinary build. `PLEXTOR_LED_BOOT_DIAGNOSTICS=ON` adds an 84-byte reporter and
eleven checkpoint call sites spanning `FirmwareMainLoop`, controller/memory
initialization, the indirect `RAM:ff5000` loader boundary, configuration
validation, runtime-state setup, and initial coroutine configuration. Frames
drive `BITS:0580/0581` through stock-used 01/10 states with 11 separators and
contain one long synchronization pulse plus N short pulses. The user must video
the complete power-on sequence because the frames are a timeline, not persistent
state. The retained flashable image is
`out/agent-tests/led-boot-diagnostic-20260925/MONT_reconstructed_led_diag.bin`,
SHA-256 `452eeb7a5f02a001891c7816ed0f056fa60f8b009df6f198f32d1c4c93699b3f`,
checksum `0x1d83`; it has stock fixed data and linked with zero diagnostics at
439,246 code bytes. Exact maps/listings and the checkpoint interpretation are
in `docs/LED_BOOT_DIAGNOSTICS.md`. Reconfiguring with the option off and
rebuilding restored the ordinary image exactly to SHA-256
`e2afa26190203d0a7407e91033dbb21b0bb5c3fa5f7f41f9048c53cbef7e631b`.

The user physically flashed that LED diagnostic image and observed the final
checkpoint frame: a long yellow synchronization indication followed by eleven
short interleaved yellow/green indications starting with yellow. This confirms
that the two P3 diagnostic states are visible on the two front-panel LEDs and,
more importantly, that physical execution reached checkpoint 11 after
`ConfigureInitialCoroutines` returned. Thus the corrected indirect
`RAM:ff5000` loader returned, the cold initializer passed its success-only path,
and initial coroutine contexts were published. Around the final frame the drive
attempted tray motion (audible actuation, initially reported extension on the
first observation, but on the clarified run no motion), then remained steady
green and did not respond to pushing the tray inward. The user will repeat a
slow-motion recording to preserve the full timeline. Because interrupts are
already enabled during the countdown inside `ConfigureInitialCoroutines`, the
tray sound's position before the code-11 frame does not by itself place the
fault in the post-checkpoint scheduler. Treat the failure as narrowed to live
interrupt/coroutine execution or hardware-control state after successful cold
initialization, not as an early boot or flash-programming failure.

A second-pass scheduler diagnostic now replaces all eleven proven cold-start
calls with six one-shot frames around the first main scheduler pass. It restarts
at stage 1 immediately before `ResumeCoroutine0`, reports stages 2 and 3 after
coroutines 0 and 1 return/yield, stages 4 and 5 after the conditional coroutine
2 and 3 gates, and stage 6 after the post-resume reinitialization gate and loop
end. The reporter now uses one long `01` marker followed only by N short `10`
pulses, and every delay unit invokes the busy-wait worker twice, making it 2x
slower than the first-pass protocol. It disables itself after stage 6. The
retained flashable image is
`out/agent-tests/led-scheduler-diagnostic-20260925/MONT_reconstructed_led_diag.bin`,
SHA-256 `482bcb030871c8ed74bc4b8add8475f769e6c6716905b32da6729a6287a488af`,
checksum `0x61ae`, with stock fixed data intact and code size 439,244. C251 and
L251 completed with zero diagnostics; listings confirm exactly six call sites,
two busy-wait calls per delay unit, fixed-color count pulses, and no initializer
diagnostic calls. This deliberate slow first-scheduler-pass instrumentation can
perturb timing, so use it only to isolate the first non-returning resume/gate.
After archiving the diagnostic artifacts, the Windows build was reconfigured
with the option off and reproduced the ordinary image exactly at SHA-256
`e2afa26190203d0a7407e91033dbb21b0bb5c3fa5f7f41f9048c53cbef7e631b`.

The user physically ran the second-pass scheduler image and reported green
count frames `1,2,3,4,5,6,2,3,4,5,6,2,5,6`, followed by stable yellow and
green illumination while Windows played a device connect/disconnect-like
sound. The first uninterrupted `1..6` is valid evidence: coroutine 0 returned,
coroutine 1 returned, the coroutine-2 and coroutine-3 gates/resumes completed,
and the first scheduler pass finished. The later partial repetitions are an
instrumentation defect, not backward control flow. C251 allocated the intended
one-shot local in `R15`; each native `ResumeCoroutineN` switches SPX and pops
the resumed coroutine's `DR12`, while `YieldCoroutineN` pushes that coroutine's
`DR12` and returns to the scheduler without restoring the scheduler's prior
`DR12`. Thus the local `R15` guard is overwritten at every resume/yield. Do not
use an automatic/register local to retain diagnostic state across a coroutine
resume. A read-only Windows check immediately afterward found no present
Win32 CD-ROM and no present PnP device matching Plextor/PX-W5224A, so the sound
did not produce stable enumeration. The next diagnostic must remove all
scheduler-loop frames and use naturally one-shot entry/progression points or
an explicitly initialized absolute DATA byte/bitmask shared across coroutine
contexts.

That follow-up host-command diagnostic is now built. All scheduler-loop frames
were removed. `LedBootDiagnosticArm` explicitly clears a linked XDATA byte after
`ConfigureInitialCoroutines`; `LedBootDiagnosticCheckpointOnce` uses its eight
bits to suppress repeats across coroutine contexts. The Keil listing confirms
`MOVX` loads/stores for the mask rather than a retained register. Stages 1..8
now progress linearly inside `ScsiCommandProcessorCoroutine0`: entry, processor
initialization return, optional initial drive-startup yield return, command-loop
entry, first idle-yield preparation, first idle-yield return, first received
host packet return, and first completion-handling path. The now-superseded
2x-timing image is
`out/agent-tests/led-host-command-diagnostic-20260925/MONT_reconstructed_led_diag.bin`,
SHA-256 `7dd91720976ae262b83d6f901ee96409aed1532f3ed8868e45c2287cb52c9657`,
checksum `0x1ce3`, code 439,300, XDATA 6,823, stock fixed-data match true, and
zero Keil diagnostics. The focused host command-processor test passes. After
archiving exact map/listings, the normal option-off build again reproduces
`e2afa261...e631b`.

At the user's request, the same host-command diagnostic timing was doubled
again to 4x the original first-pass timing so counts can be read without video.
The only flash artifact to use is uniquely named
`out/agent-tests/diag04-hostcmd-slow4x-20260925/PXW5224A_diag04_hostcmd_slow4x.bin`,
SHA-256 `ba284d0dfc092812dc1bcf32633212256cf68b775f47f01de2fb1c85b3c782ba`,
checksum `0x2560`, code 439,312, XDATA 6,823, with stock fixed data and zero
Keil diagnostics. Future test images must receive a new descriptive filename
for each iteration; do not expose the generic build-tree filename to the user.

Physical testing of diagnostic 04 produced stages `1,2,3,4,7`, then stable
yellow+green illumination and a Windows new-device sound. Stages 5 and 6 are
conditionally inside the `FW_NEAR_HOST_PACKET_STATE == 1` idle-yield loop, so
their absence is expected when a packet is already pending. Stage 7 proves
that `ReceiveScsiCommandPacket` returned for a real host command. A read-only
Windows check immediately afterward still found no present CD-ROM or Plextor
PnP device. Do not treat the missing stage 8 as a clean localization of the
original defect: diagnostic 04 performs the slow seven-pulse frame immediately
after packet receipt and before dispatch/completion, which can itself exceed a
USB-bridge/Windows command timeout and provoke the observed disconnect/reset.
Do not build another slow in-band command-path reporter. The next diagnostic
must record command milestones with bounded memory writes and defer slow LED
reporting until the command is no longer timing-critical (or until a later
coroutine restart/idle state).

A Bus Hound 7.04 power-cycle capture of diagnostic 04 at `E:\ISO\diag1.txt`
confirms that timing hazard at the wire boundary. The live bridge enumerated as
USB `093b:0042`, serial `0042001D9E`, completed descriptors/configuration and
GET MAX LUN, then accepted exactly one Bulk-Only Transport CBW. Its CDB was
standard INQUIRY `12 00 00 00 24 00` with a 36-byte DATA-IN expectation. The
capture contains no DATA-IN payload and no CSW after that CBW. Combined with
the observed stage 7, this proves the foreground received the command while
the host remained unanswered; it does not establish an original dispatch bug
because the 4x-slow stage-7 reporter runs precisely between receive and
dispatch. Do not collect another diagnostic-04 trace or infer handler failure
from its missing response. The next bus-facing experiment must contain no
blocking LED work from packet arrival through data/status completion.

That no-LED bus experiment is packaged without another Keil run from the
already validated option-off corrected image as
`out/agent-tests/diag05-bus-capture-no-led-20260925/PXW5224A_diag05_bus_capture_no_led.bin`.
It is the ordinary image SHA-256
`e2afa26190203d0a7407e91033dbb21b0bb5c3fa5f7f41f9048c53cbef7e631b`,
checksum `0x14d6`; fresh verification reports a valid 524,288-byte format and
stock-identical fixed data. It contains no LED diagnostic option/call path.
Use passive Bus Hound capture on bridge device 27 across power-on; the user
owns the recovery-mode redumper flash operation.

The user's Bus Hound capture of that no-LED image (`E:\ISO\diag2.txt`) has the
same decisive boundary as diagnostic 04. After one detach-time USB transaction
error, bridge `093b:0042` re-enumerates normally, completes SET CONFIG,
SET INTERFACE, and GET MAX LUN, and submits one 36-byte standard INQUIRY CBW
(`12 00 00 00 24 00`). There is again no DATA-IN phase and no CSW. Because
this image has no diagnostic reporter, the missing response is a genuine
reconstruction failure. Combined with diagnostic 04 stage 7, the live evidence
now places it after `ReceiveScsiCommandPacket` returns: protocol selection,
SCSI admission/dispatch, INQUIRY setup, the controller DATA-IN worker, or final
completion publication. The simulator's complete packet-ingress INQUIRY case
passes only with modeled controller DATA-IN completion, so the next physical
diagnostic should use instantaneous/static LED state writes around INQUIRY and
DATA-IN setup/wait/return; never add an in-band delay.

That static diagnostic is now built as
`out/agent-tests/diag06-inquiry-static-led-20260925/PXW5224A_diag06_inquiry_static_led.bin`,
SHA-256 `9d96274d2578282310076f26615db3c7a1a2a61a0b000a876630dcee37fecb27`,
checksum `0x3b41`, code 439,288, XDATA 6,823. The package is structurally valid,
has stock-identical fixed data, and C251/L251 reported zero diagnostics. There
are no calls to the slow checkpoint reporter. After packet receipt it writes
static state 0 (off); `HandleInquiryCommand` writes state 1 (the previously
observed yellow marker state); the DATA-IN worker writes state 2 (the previously
observed green count state) after programming the controller and immediately
before its completion wait; and it writes state 3 (both) only after the wait
ends and controller cleanup runs. These are two SFR bit assignments per state,
with no delay or loop. Interpret final off as failure before INQUIRY entry,
yellow as INQUIRY entry without reaching the DATA-IN wait, green as a DATA-IN
wait that did not return, and both as DATA-IN worker return; correlate all
states with the passive Bus Hound DATA/CSW result because later firmware writes
may also touch the LEDs.

Physical diagnostic-06 observation was `both -> off -> both`, followed by the
Windows device sound. Do **not** interpret the final both state as proof that
the instrumented DATA-IN worker returned. A source audit immediately afterward
found many ordinary foreground, coroutine, timer, and interrupt paths that
write the same `BITS:0580/0581`; the final state is therefore not a persistent
diagnostic latch and may be unrelated drive code. The off transition is
consistent with the post-receive marker but is not enough to disambiguate the
later path. Diagnostic 06's static-state design is retired; do not flash it
again. Its simultaneous Bus Hound trace is authoritative for whether INQUIRY
DATA-IN and CSW crossed the USB bridge. If that trace remains insufficient,
use a deliberate, continuously reasserted breakpoint pattern only after one
selected milestone (and accept that it intentionally stops the command), not a
one-time LED state that normal firmware can overwrite.

The simultaneous Bus Hound 7.04 text export for diagnostic 06 is retained in
the user attachment `Pasted text.txt`. Bridge `093b:0042`, serial
`0042001D9E`, enumerates and accepts repeated Bulk-Only Transport CBWs for the
same 36-byte standard INQUIRY (`12 00 00 00 24 00`). Every completed attempt
shown has its endpoint-1 bulk-IN canceled (`USTS c0010000`) without any payload
or CSW, followed by a USB bus reset; one recovery cycle performs a full bridge
descriptor/configuration enumeration before retrying. The trace contains seven
INQUIRY CBWs and no optical-drive response. Therefore the Windows device sound
is consistent with USB-bridge reset/re-enumeration, not successful Plextor
INQUIRY completion, and the final two-LED state can be ordinary reset/recovery
code. Static comparison confirms the reconstructed DATA-IN register programming
order and the host cause-bit-4 completion state change (`RAM:000c33: 8 -> 13`)
match stock. The next physical discriminator, only if needed, must be a
continuously reasserted breakpoint pattern at one milestone: first immediately
after DATA-IN controller programming, then at the host interrupt cause-bit-4
branch. This is a two-step reachability test; absence of a transient/static LED
state is not evidence.

Diagnostic 07 implemented the first of those two reachability tests. Its retained
image is
`out/agent-tests/diag07-data-in-programmed-breakpoint-20260925/PXW5224A_diag07_data_in_programmed_breakpoint.bin`,
SHA-256 `ae4b11fca20ecc6480d87381561e53878b180df4d831b8e5831d8792b0973374`,
checksum `0x74e3`. It removes the earlier static command markers and, immediately
after the DATA-IN setup write `RAM:01805e=0x97`, enters a permanent repeated
frame of one long yellow pulse plus two short green pulses. The listing proves
the call placement and infinite reporter loop. The image is format-valid,
524,288 bytes, stock-fixed-data-identical, and built with zero Keil diagnostics.
It intentionally prevents INQUIRY completion; no Bus Hound capture is needed.
The CMake diagnostic option was returned to OFF after archiving without
rebuilding the ordinary artifact.

The user reports diagnostic 07 showed "one yellow 2 green". This proves the
shared ExecuteControllerDataInTransfer worker reached the post-programming
breakpoint. It does not alone prove INQUIRY was the caller: ATA IDENTIFY also
uses this worker. Correct payload bytes and physical completion remain open.

Diagnostic 08 is retained at:
`out/agent-tests/diag08-data-in-outcome-20260925/PXW5224A_diag08_data_in_outcome.bin`,
SHA-256 `8e7270a2dfa6509985f7f1517aac87369aabae27ce37de7017909de9abe78ae2`,
checksum `0x3352`. It replaces the setup stop with outcome stops: one long
yellow plus N green, repeated. N=1 is host ISR cause-4 branch entry, 2 is the
existing DATA-IN timeout after setting sense, 3 is the configuration/operation
abort predicate, 4 is the RAM:000c83 bit-0x10 abort, 5 is the host ISR's
firmware/coroutine restart request, 6 is any other DATA-IN wait exit. These
are mutually terminal observations, not successive stages. A volatile XDATA
active byte (linked RAM:011a34) is initialized at FirmwareMainLoop entry and
set at DATA-IN worker entry; unrelated earlier interrupts cannot trigger a
stop. Interrupts stay enabled until the reporter masks EA and loops forever.
No command completion or USB response is implied by count 1. No repeating
pattern is inconclusive. No further Bus Hound capture is requested.
Target listings and package checksum/fixed-data checks pass with zero Keil
diagnostics. Focused host DATA-IN transfer and host-interface-interrupt tests
pass; an inadvertently started full foundation test did not finish promptly
and was stopped, so no new full-suite pass is claimed. The diagnostic option
is OFF again after archiving the image/listings/map/application. User owns
flashing and the established Eject-at-power-on recovery strategy still applies.

The user now reports diagnostic 08 repeatedly emits one long yellow and one
short green: outcome 1. This proves the host ISR cause-4 branch was reached
while the shared DATA-IN worker was active, before CompleteHostDataTransfer.
It does not identify the opcode or prove physical payload/USB completion.

The targeted follow-up found a confirmed C address-translation error in ATA
IDENTIFY: live stock RAM:fdb4df ADD WR12,#0x10 changes the HIGH half of DR12,
adding 0x100000, not 0x10 to the low half. With reserved base RAM:160000, the
old builder wrote RAM:160010 instead of RAM:260000. The C implementation now
uses FirmwareAddressIncrement24(base, FW_ADDRESS_BANK_STRIDE). Its target
listing emits ADD WR4,#0x10 on the high half of DR4. Canonical Ghidra plate
comment and learned pattern were corrected and the intended program saved.
This is a proven source defect, but its responsibility for failed enumeration
is not yet established on hardware.

The host IDENTIFY tests had encoded the wrong address; corrected expectations
fail on old source and pass on the fix. New cases check low-word wrap,
untouched controller-offset memory, and 24-bit wrap. Assertions are explicitly
enabled even under Release NDEBUG. The S251 override had masked the defect
with different input bases (stock f18010, reconstruction 018000). Both now use
f18000 with expected CPU result RAM:018000, offset zero. Its zero-loop patch
offset is refreshed from the listing (function +58). A focused simulator
configuration unit test passes; NO new uVision/differential pass is claimed.
Previously retained aggregate passes do not establish this address arithmetic.

Latest supplied physical candidate is
`out/agent-tests/diag09-identify-buffer-fix-20260925/PXW5224A_diag09_identify_buffer_fix.bin`,
SHA-256 `51c96fc28c1ed2620cb2b8d5917353d92be0bcb4261513e209b0813435aff95d`,
checksum `0xdea6`. All LED diagnostics are compiled OUT; the normal transfer
completion path runs without diagnostic stops. Fixes diagnostic-disabled
no-op macros to avoid Keil C138 warnings. Stale diagnostic objects were
regenerated; final map has no LedBootDiagnostic symbols. Build has zero Keil
warnings/errors, is 524288 bytes, and passes independent checksum/stock-fixed-
data verification. Listings/map/application/validation are archived alongside.
User owns redumper flashing; Eject-at-power-on recovery and known-good stock
restore remain the fallback. No hardware commands were sent by the agent.

The user supplied a subsequent inline Bus Hound excerpt after the diag09
handoff (image identity inferred from conversation, not independently read
back). Device 27 is the same bridge 093b:0042, serial 0042001D9E. Records
1..10 show USB descriptors/configuration and maximum LUN zero. Record 11
submits a 36-byte device-to-host INQUIRY (CDB 12 00 00 00 24 00, padded to
12 bytes); record 12 cancels bulk-IN, record 13 completes a USB reset, and
record 14 retries INQUIRY. No bulk-IN payload or CSW appears in this excerpt;
the IN data preceding it consists only of USB descriptors/control replies.
No timestamps or final retry outcome are provided. Assuming diag09, the
IDENTIFY address correction is insufficient to restore the captured INQUIRY
exchange. Keep the evidence-backed fix; do not claim it fixed enumeration.
Diagnostic 10 now distinguishes the caller after completion-helper return:
`out/agent-tests/diag10-command-identity-20260926/PXW5224A_diag10_command_identity.bin`,
SHA-256 `7386b5af4e193b76e0ada7d254d0a1bf6b66b7ded00d83dde5040ee7c8e9d7b6`,
checksum `0x6ae6`. Same slow repeating long-yellow/N-short-green pattern.
N=1 ATA IDENTIFY, 2 SCSI INQUIRY, 3 other/untagged DATA-IN. These stops are
AFTER CompleteHostDataTransfer but BEFORE ISR epilogue/RETI: neither interrupt
return nor foreground wait exit nor USB completion is proved. Fallback counts
4=timeout after sense, 5=config/operation abort, 6=completion flag 0x10 abort,
7=ISR firmware/coroutine restart request, 8=other foreground wait exit.
No repeating pattern is inconclusive. No Bus Hound capture is requested.

The processor sets command kind immediately before dispatch (ATA=1; validated
SCSI opcode 0x12=2; other=3), clears it afterward; the worker snapshots it at
entry (zero maps to 3). Both volatile XDATA bytes are explicitly initialized
at main-loop entry and survive coroutine yields: command kind RAM:0119d3,
active snapshot RAM:0119d4 in diag10. Live canonical RAM:fdc19a..fdc1d0
confirms protocol/opcode routing; RAM:fdb0f6..fdb10b confirms ATA IDENTIFY
is the ATA dispatcher's only DATA-IN call. The earlier address fix is kept.
Target listings confirm tag placement, snapshot, and report after helper call.
Clean Keil compile/link, independent checksum/fixed-data validation, enabled
and disabled tag tests (assertions active under NDEBUG), focused processor and
host-ISR executables pass. No uVision or hardware commands. Artifact, map,
listings, application, and README are archived.
Diagnostic option restored OFF and ordinary firmware rebuilt: SHA-256 is
exactly diag09's `51c96fc2...aff95d`, proving the new instrumentation compiles
away without changing the non-diagnostic image. Archived diag10 is unchanged.

The user reports diag10 gives three green pulses. This reaches the report
AFTER CompleteHostDataTransfer returns, but category 3 intentionally merges
explicit other-SCSI dispatch and an unset command tag (zero mapped to 3 at
DATA-IN entry). Therefore it does NOT identify an actual opcode, exclude
INQUIRY if the tag is absent/incorrect, or prove ISR return. Inspection of
the archived listings confirms this ambiguity is in the diagnostic design,
not a new decoded firmware result. Next diagnostic must retain the zero-tag
distinction and snapshot actual protocol/CDB opcode at DATA-IN entry, with
explicit caller-site tags if needed. Do not classify this as an unknown SCSI
command or a successful transfer on the strength of count 3 alone.

Diagnostic 11 now replaces the ambiguous completion stop with a frozen
DATA-IN ENTRY snapshot, before native transfer state/register programming:
`out/agent-tests/diag11-command-snapshot-20260926/PXW5224A_diag11_command_snapshot.bin`,
SHA-256 `c763466dddf4e907b0e99991aa17ac78f02185e461ad381d2be2fbc3223243ca`,
checksum `0xa329`. EA is cleared before snapshot reads. It captures dispatch
tag separately, protocol RAM:000c78, and opcode (controller RAM:018057 if
protocol zero; CDB RAM:000b0a otherwise). Live canonical routing/read sources
were checked. Current command tag links to XDATA RAM:0118cc.

Six slow yellow-marker/green-count frames repeat with an extra-long dark gap:
first tag 1=ATA, 2=INQUIRY, 3=other SCSI, 4=missing zero, 5=unexpected tag;
second protocol 1=raw zero, 2=raw one, 3=other nonzero; last four are opcode
two-bit digits, high first, each plus one (counts 1..4). Decode opcode as
64*(third-1)+16*(fourth-1)+4*(fifth-1)+(sixth-1). Example absent-tag INQUIRY
is 4,2,1,2,1,3. Await all six counts, not just the first. The image stops
before starting this transfer, so no completion/IRQ/USB conclusion is made
from its pattern. No Bus Hound capture/video is needed. Register snapshot
does not independently prove what the bridge sent or correct CDB reception.

Initial expanded reporter overflowed existing code area. Removed obsolete
diag08/10 outcome stops and unused one-shot/static LED helpers, leaving all
functional fixes intact; no linker region expansion. Final build has zero
Keil diagnostics; 524288-byte package passes independent checksum/fixed-data
checks. Target listing verifies capture placement and interrupt masking.
515 host snapshot cases test all 256 opcodes with both missing/other tags,
plus ATA/INQUIRY/invalid contexts, and two identical report cycles despite
mutating the live inputs; disabled no-op test passes. No uVision/hardware
commands. Artifact/listings/map/application and reporter sources archived.
Diagnostic option returned OFF; rebuilt ordinary image still matches diag09
SHA-256 `51c96fc2...aff95d` exactly. Archived diag11 remains the test artifact.

User reports diag11 sequence 1,1,3,3,1,2. Decode: dispatch tag 1 (ATA),
protocol count 1 (raw zero, ATA), opcode 64*2+16*2+4*0+1 = 0xa1,
ATA IDENTIFY PACKET DEVICE. This identifies diag11's first captured DATA-IN
entry, not a failed command or completed transfer: this image deliberately
stops before programming it. It does not retrospectively identify diag10's
ambiguous count-3 observation; the difference in tag remains unexplained.
Diagnostic 12 now lets ATA identification proceed and targets received SCSI
INQUIRY (protocol RAM:000c78=1, CDB RAM:000b0a=0x12) specifically:
`out/agent-tests/diag12-inquiry-progress-20260926/PXW5224A_diag12_inquiry_progress.bin`,
SHA-256 `a6c673d8514b5da63178358c6bd35c1a0689a7d50fb7a0f09bf938f19d1cba71`,
checksum `0x8b48`. One repeating long-yellow/N-short-green frame, same slow
timing; NOT six fields. No earlier LED delays or ATA diagnostic stops.

N=1 INQUIRY received but no DATA-IN entry; 2 DATA-IN entered; 3 controller
setup complete; 4 cause-4 completion helper returned; 5 foreground DATA-IN
cleanup complete with RAM:000c33=13; 6 that successful cleanup plus SCSI
dispatcher returned; 7 DATA-IN cleanup with a status other than 13. Report
is triggered after admission/dispatch returns, on failed DATA-IN cleanup,
or when host ISR requests firmware/coroutine restart. Thus 2..5 normally
identify progress before that restart; 1 can also mean admission rejection
or return without DATA-IN. Count 6 still stops BEFORE final command-status
publication, so it does not prove USB response/CSW. No pattern is inconclusive.
User should report ONE count (or no repeating pattern); no capture needed.

Stage is explicitly initialized volatile XDATA RAM:011a93. Brief atomic
mark updates preserve EA and prevent ISR/foreground progress regression;
cause-4 before DATA-IN starts cannot advance received-only stage. Reporting
masks EA permanently. Archived listings confirm hook locations and DATA-IN
(not DATA-OUT) exit. Clean Keil build, independent checksum/fixed-data checks,
enabled/disabled progress tests (all opcodes, all 255 non-success statuses,
success/return, reset reports, EA restoration, monotonic progress) and focused
processor/host-ISR executables pass. No uVision/hardware commands. Archive
contains image, map, listings, application, test driver and reporter sources.
Option restored OFF and ordinary image rebuilt to the unchanged diag09 hash
`51c96fc2...aff95d`; archived diag12 remains the intended physical test image.

User reports diag12 power-on: briefly both LEDs, then off, then both LEDs
with Windows device sound; no repeating diagnostic frame reported. Treat
this as no observed report, not an INQUIRY milestone. Ordinary firmware can
write the LEDs and the USB bridge can enumerate/reset independently. Diag12
arms silently and reports only at outcome hooks, so this cannot distinguish
failure before INQUIRY receipt from a stall after receipt but before a report
hook (nor rule out instrumentation/state problems). Next useful discriminator
is a terminal repeating breakpoint at positively identified INQUIRY receipt,
before admission/handler/transfer work; if absent, check ATA completion/next
packet receipt rather than assuming INQUIRY is stuck. Do not ask to repeat
diag12 or infer a drive reset from the Windows sound alone.

Diagnostic 13 is ready:
`out/agent-tests/diag13-inquiry-received-20260926/PXW5224A_diag13_inquiry_received.bin`,
SHA-256 `9ee26479f1e0bced681a4cd052c430153aa5c2238566a851836f9ccf9a652d60`,
checksum `0x943e`. Minimal change from diag12: matching protocol=1/opcode=12
in BeginCommand immediately masks EA and loops on literal checkpoint 1,
before admission/dispatch/transfer. Expected ONE long yellow + ONE short
green, repeating at unchanged slow timing. ATA/other commands bypass it.
No mutable stage is used to select this receipt count. No capture/video needed.
Absence remains no observed report, not proof of absence of a USB command.

Zero Keil diagnostics; target listing confirms comparisons, EA clear, literal
1 loop and placement before validation. Enabled/disabled host tests cover
all opcodes under ATA/SCSI contexts, the immediate repeated receipt stop,
and retained progress helper regressions. Independent checksum/fixed-data
verification passes. Artifact/map/listings/application/test driver/reporter
sources archived. No uVision or hardware commands. User subsequently reports
the same both/off/both plus Windows sound as diag12, with no repeating frame.
Option restored OFF; ordinary image rebuilt to unchanged diag09 hash
`51c96fc2...aff95d`. Archived diag13 is the intended test image.

The user has Bus Hound available. Prefer a passive Bus Hound capture before
another post-packet LED build: select the USB bridge and any optical/SCSI child,
start capture before reset/power-on, stop shortly after the Windows device
sound, and retain timestamps, USB bulk/control records, SCSI CDB bytes, data,
status, and sense. Never use Bus Hound's command-submission or bus-reset
features for this observation. Capture the same sequence with working stock
firmware as a baseline when practical. The bridge-facing trace may not expose
the bridge-to-drive ATAPI electrical bus, but it can distinguish a submitted
CDB plus response/status from a transaction that disappears without CSW/status
during reset. Ask for the native saved capture plus a text export.

Two disposable current-image reset-to-first-fill S251 attempts are retained at
`out/agent-tests/cold-start-first-fill*/`. Both exhausted the Windows uVision
wall limit while waiting on startup timing/hardware behavior and produced no
stock/reconstruction comparison result. Do not rerun the same cases. Existing
event-driven initialization evidence remains useful, but a new trace must use
reactive timer/controller events or a smaller checkpoint rather than a plain
reset run.

## Next priorities

1. Obtain diag23's one repeating slow count1..8. Interpret using the immediate
   handoff and docs/DIAG23_HOST_ZERO_ORIGIN.md. It deliberately stops normally
   executing firmware; Windows detection is not expected.
2. Trace the zero-state origin in the interval selected by the report. Do not
   force c32=1 or infer corruption before establishing initialization history.
3. Keep unresolved actual-Timer0 controller deltas separate; retain proven
   production corrections and the unmeasured XDATA backing constraint.
4. User flashes; protected Eject-at-power-on recovery remains the fallback.



