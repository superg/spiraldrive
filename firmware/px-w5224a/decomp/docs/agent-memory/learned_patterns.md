# Learned patterns

- On 2026-11-04, physical PX-W4824A H: accepted the York 1.07-derived `P107` image through Redumper's standard Plextor flash route (4-KiB WRITE BUFFER transfers, final save command). Redumper reported success/exit 0 after 12s; a subsequent standard INQUIRY returned GOOD and revision `P107` without an agent-issued separate reset. Evidence: `out/agent-tests/york-107-flash-20261104/redumper-flash.log` and `post-flash/inquiry.json`. This confirms transfer and activation of this exact image, not lead-in speed, optical correctness, or 4824 rescue-mode operation.

- In PX-W4824A York 1.07 (SHA-256 `79553d4e8d8143c5d0d00f72e8ec274985fd292a151cc569d7ea5569b251c978`), the descriptor builder at `RAM:fe0000` stores only 24 position bits in a 20-byte descriptor. Buffer reuse at `RAM:fec1b2` clears R0 at `RAM:fec21a` and `RAM:fec242`, zero-extending negative positions before signed comparisons and reaching reset `RAM:fec28c` even for buffered negative targets. Mont 1.04 descriptors are 22 bytes with full 32-bit positions. Actual York execution in Keil S251 confirms the erroneous branch; substituting two-byte `MOVS WR0,R1` (`1a 01`) restores negative reuse in the tested cases without changing tested positive hits or true misses. All 16 stock/corrected cases pass; this is synthetic-queue execution evidence, not physical timing or optical validation. See `docs/YORK_107_D8_LEADIN_ANALYSIS.md` and `tools/verify_york_d8_buffer_reuse.py`.

- The installed `80251-fixed` language's opcode `0x49` word indirect-displacement load has reversed register roles: York bytes `49 03 ff ff` mean `MOV WR0,@WR6-1`, confirmed by Keil A251 MODSRC and S251, while Ghidra renders the reverse register assignment. Opcode `0x1a` is displayed as MOVZ although its P-code uses sign extension and A251 calls it MOVS (`1a 01` = `MOVS WR0,R1`). These findings require isolated decoder regression before changing canonical analysis; no language repair was applied during the York comparison.

- In canonical PX-W5224A 1.04, the D8 read-family rejection at RAM:fda98c and active-write finalization opcode selection at RAM:fdabfc are inline instruction chains, not editable opcode tables. C4 is absent from both. With C4's dispatch and media-required entries installed, leaving stock RAM:fda843 admission unchanged lets C4 use general stock admission while omitting those two D8-specific exceptions. Evidence: canonical Ghidra disassembly, reconstructed `src/scsi/command_admission.c` and `src/scsi/active_media_operation_admission.c`, and the 2026-10-01 independently audited generated image. This routing has not yet been live-tested.

- In canonical PX-W5224A 1.04, `InitializeCoroutine1` (RAM:fee4c1) writes the four-byte initial entry at RAM:0500..0503 and resets coroutine-1 stack state at RAM:1162..1165. `YieldCoroutine1` (RAM:fee534) saves the current SPX at RAM:1162; it does not rewrite RAM:0500. Thus RAM:0500 retains the last initialized coroutine-1 entry across yields and can distinguish the C4 entry from the stock entry before a C4 producer request. Evidence: canonical Ghidra disassembly and the Keil listing for `GofwAlternateRawReadStep` (2026-10-01). The new C4 entry's behavior is verified offline only.

- In canonical PX-W5224A 1.04, the common admission function RAM:fda843 has three stock callers and begins with a complete four-byte `MOV ACC,RAM:0012f7`; its return is far. The two formerly patched opcode reads RAM:fda98c and RAM:fdabfc belong to that admission flow and can remain stock when C4 READ is selected at the entry. The active-operation subroutine RAM:fdab34 returns near and its busy branch does not reach the RAM:fdabfc opcode read, so a payload C admission specialization may call that branch through an FD-bank `LCALL`/`ERET` adapter. Evidence: canonical Ghidra disassembly/decompilation and `gofw-pxw5224a` Keil map/listing plus independent generated-image diff (2026-10-01). This is an offline ABI and routing result, not physical validation.

- In canonical PX-W5224A 1.04, coroutine 1 state 2 begins at RAM:f85de5 with `MOV ACC,RAM:00103a`, `ORL A,#0x20`, the matching store, a near `LCALL` to `ProcessReadAcquisitionState` RAM:f86475 at RAM:f85def, then a near finalizer call at RAM:f85df2. The acquisition worker uses near-returning helpers at RAM:f890e8 and RAM:f868f8; those cannot be called through an ordinary far tail bridge from the FD/FE payload. `InitializeCoroutine1` RAM:fee4c1 writes the scheduler PC at RAM:0500..0503 and resets coroutine-1 stack state at RAM:1162..1165. Evidence: canonical Ghidra disassembly/decompilation and the accepted root C reconstructions `src/media/state_coroutine_dispatch.c`, `src/media/read_acquisition_state.c`, and `src/platform/coroutine_initialization.c` (2026-10-01). This establishes the offline routing contract; it does not prove the new C4 FE-tail worker on hardware.

- On user-flashed SpiralDrive PX-W5224A 1.04, a 4x buffered 128×16-frame C4 probe produced periodic GOOD/full transfers whose adjacent CRC-valid Q position jumped forward seven frames. The on-device 20-record trace at RAM:0016b0..0017ef captured one such boundary: READ 111 ended with descriptor queue count 163, secondary cursor 288, producer cursor 289; READ 112 began after a reset with count 0, secondary 0, controller state 0, active producer 1, and descriptor position low word 0 rather than 9. READ 112 then returned GOOD/full while Q advanced 2733→2741. This confirms a C4 acquisition/descriptor reset at a continuity gap, independent of the host's buffered file I/O. The trace header counted only one queue-pressure throttle event over 128 READs, so that branch cannot explain seven gaps. The trigger that changed C4's pre-READ mode/format remains unobserved. Evidence: `out/agent-tests/gofw-trace-live/spiraltest_buffer-c4-trace-compact.{bin,csv}`, `E:/ISO/dump/spiraltest_buffer.spiral-probe.csv`, and `gofw-pxw5224a/src/raw_sector_range.c` (2026-09-30).

- On a user-flashed SpiralDrive PX-W5224A 1.04 at H:, read-only F5 selector-zero DATA IN of CPU RAM:002000, 002fe0, 002ff0, 003000, and 004000 returned fixed `0x2b` bytes, including a 4112-byte read of the intended trace ring. The custom payload bytes at RAM:fded2c and fdf720 matched the traced build, so missing instrumentation was not explained by an older image. F5 returned live data at RAM:001200 and zeros through RAM:0017f0..001fff. Thus F5 cannot observe a trace stored at RAM:002ff0..003fff on this drive; read-only observations alone do not prove that RAM:0017f0..001fff accepts writes. Evidence: `out/agent-tests/gofw-trace-live/spiraltest_buffer-c4-trace.bin`, read-only F5 samples from 2026-09-30, and image-byte comparison to `out/agent-tests/gofw-trace-build/MONT_104_gofw.bin`.

- On the next user-flashed SpiralDrive 1.04 trace image, RAM:0017f0..0017ff retained a C4 READ trace header with total count 128 and throttle-entry counter 1, while F5 returned zeros throughout intended record storage RAM:001800..001fff. The flashed trace-start code at RAM:fded2c matched the candidate and the trace-end code at RAM:fdfe60 encoded the intended `0x1800` record base. This establishes that the C4 tracing path executed but RAM:001800..001fff did not provide F5-visible records. The same host run had eight Q position jumps, so this particular queue-pressure throttle branch cannot be the recurring trigger unless its count was lost independently of the retained header. Evidence: `out/agent-tests/gofw-trace-live/spiraltest_buffer-c4-trace-lowram.bin`, `E:/ISO/dump/spiraltest_buffer.spiral-probe.csv`, and read-only live code-byte comparison (2026-09-30).

- PX-W5224A 1.04 configures the raw main-data ring for 512 slots at controller initialization (`RAM:00156a=0x200`), with a supported 256-slot geometry mode. C4's alternate coroutine sets its descriptor-ring mask from this count. After a successful C4 READ, the C4 handler clears only the command-in-progress bit while retaining raw mode 3 and its stream marker; the next C4 READ reuses the secondary descriptor cursor rather than resetting the queue. Its producer throttles near capacity when `(slot_count-6)-queued_count < 0x40` with the active-producer flag. This establishes intended cross-command buffering and backpressure, not that a particular live desync was caused by queue saturation. Evidence: `src/platform/controller_asic_initialization.c`, `src/platform/leaf_workers.c` `ConfigureMainDataRingGeometry`, `gofw-pxw5224a/src/alternate_raw_coroutine.c`, `raw_sector_range.c`, `alternate_raw_helpers.c`, `c4_handler.c` (2026-09-30).

- In canonical PX-W5224A 1.04, `StartBatchedRawReadHostTransferFromSecondaryCursor` writes `RAM:000f47=1` at RAM:fd74a5. Ghidra shows only one write clearing this byte: `CompleteBatchedHostTransferBuffers` at RAM:fde989, called from the host data-completion interrupt. `PollRawSectorReadHostTransferStatus` waits on this byte at RAM:fd7083. The completion routine also advances the secondary descriptor cursor and completed frame count, so clearing the flag in a C4 timeout path without cancelling the hardware transfer would leave a late interrupt free to mutate reset queue state. This establishes the stock software ownership of the flag, not a safe hardware cancellation protocol. Evidence: canonical `/validated/MONT_104.bin` RAM:000f47 xrefs and `src/platform/host_interface_interrupt.c`, `src/platform/leaf_workers.c`, `src/scsi/raw_sector_batched_host_transfer.c` (2026-09-30).

- PX-W5224A 1.04 `ProcessPackedSubchannelRecordBatch` (RAM:f93068) is **not** a Sub-Q CRC or C2 quality gate. It converts controller subchannel symbols to four 18-byte packed records, examines each record's header, class, and sequence, resets RAM:000f4e on an accepted record, and increments that counter once after the group when the **fourth** record was not accepted. In its BITS:0003-clear branch, RAM:000f4e > 32 enqueues descriptor `05/24/00`. Thus the counter can advance when a raw C4 stream is accidentally handled as packed records; it does not establish that the actual disc Sub-Q was bad. Evidence: canonical 1.04 function and `src/media/packed_subchannel_record_batch.c` (2026-09-29).

- In PX-W5224A 1.04, the descriptor ring entry is 22 bytes: key at +0x10 and ASC/ASCQ at +0x0e/+0x0f. The intended C4 raw-sector gather (`ProcessRawSectorBufferEvent`, RAM:f92d0b, under BITS:0013) builds status zero, whereas stock mode-3 validation at RAM:fc5364 rejects a nonzero key without publishing its cause. A failed C4 descriptor therefore came from a different producer/failure path, and its original key/ASC/ASCQ can be read from the secondary cursor before queue reset. The C4 reporter now does so while preserving its completed-frame information. Canonical 1.04 disassembly, reconstructed source, Keil listing, and host tests establish this software path; a new runtime failure is needed to identify the producer behind the 2026-09-29 long-dump errors.

- In `dump::spiral`, Redumper treats valid track-`AA` Q as positional data and keeps capturing after lead-out; absent `--lba-end`, it has no end-of-disc stop condition. In the C4 payload, `GofwExecuteRawSectorReadRange` repeatedly yields when the stock poll returns `0xfe` but never checks the CDB timeout. The stock `PollRawSectorReadHostTransferStatus` at RAM:fd6fe0 also has an internal yield loop while RAM:000f47 reports a host transfer in progress; a deadline checked only around the outer poll cannot bound that wait. A controller that ceases producing descriptors or completing a transfer can therefore leave a C4 READ pending indefinitely. Evidence: `../redumper/cd/cd_spiral.ixx`, `gofw-pxw5224a/src/raw_sector_range.c`, canonical PX-W5224A 1.04 disassembly, and `src/scsi/raw_sector_host_transfer_poll.c` (2026-09-29). On the user's one-track disc, formatted TOC lead-out LBA 35316 matched the CRC-valid Q transition to track `AA` exactly; the captured stream continued much farther into `AA`, and the drive eventually became unresponsive. That observation supports disc-end overrun as a plausible trigger but does not prove the precise final failure mechanism.

- In stock PX-W5224A 1.04, `ProcessPackedSubchannelRecordBatch` at RAM:f93068 sets RAM:001598 to 150 and enqueues a failed descriptor with key `05`, ASC/ASCQ `24/00` after its miss counter RAM:000f4e exceeds 32; the reconstructed media producer code has no other `0x2400u, 5u` descriptor enqueue. Its caller is the stock state-7 controller event path, which selects packed processing when BITS:0013 is set and signed RAM:001598 > -1000. The C4 raw-stream design also sets BITS:0013 and uses a positive RAM:001598 descriptor sequence, so every state-7 event must stay on the C4 raw gather path. The user ran the descriptor-origin reporter and received repeated `05/24/00` on C4 READ, establishing packed-path entry; it is **not** C4 CDB validation. The C4 event hook currently tests mutable last-CDB bytes RAM:000b0a/0b0b. An idle read-only F5 snapshot showed C4 mode3 and BITS:0013 persisting while that opcode byte became F5; however F5 was itself the probe, so it proves a routing vulnerability, not the precise trigger during the dump. Evidence: canonical disassembly, `src/media/packed_subchannel_record_batch.c`, `gofw-pxw5224a/src/read_position.c`, user dump, and `out/agent-tests/gofw-c4-052400-post/report.json` (2026-09-29).

- In stock PX-W5224A 1.04, `PollRawSectorReadHostTransferStatus` at RAM:fd6fe0 writes table-derived sense and the completed count on producer error RAM:000fae. In raw mode 3, `ValidateRawSectorDescriptorStatus` at RAM:fc5364 returns zero for a descriptor with nonzero key at offset +0x10 without publishing that descriptor's sense; the key and ASC/ASCQ remain at offsets +0x10 and +0x0e..+0x0f. `GetRawReadSecondaryCursorDescriptorAddress` at RAM:fef09e computes the slot from the **low word of the 32-bit ring base** RAM:00129e plus 22 times cursor RAM:001298. `ResetRawReadQueueAndTransferState` at RAM:f927bd sets that base to 0x000088bb and clears the cursor; C4 must read and classify the descriptor before resetting the queue. Stock `SetSenseData` at RAM:faff63 writes key/ASC/ASCQ to RAM:000b1a/000b24..000b25 and `SetSenseInformation` at RAM:faffd2 sets the valid bit in RAM:000b18 with a big-endian four-byte information field at RAM:000b1b..000b1e. `PrepareAtaCompletionStatusFromSense` at RAM:fdaf3e uses the nonzero key for CHECK status. Evidence: canonical Ghidra disassembly and reconstructed C sources (2026-09-29).

- Redumper `dump::spiral` calls `SET CD SPEED` before C4 access even when no speed option is supplied. Its default sends `0xffff` (optimal); `--speed=4` sends approximately 705 KB/s. Stock PX-W5224A 1.04 `SelectRequestedReadSpeed` maps 705 to 4x and `0xffff` to its maximum selected speed, subject to media limits. Stock `ProcessControllerReadEvent` at RAM:f91137..f9115c writes `0x87` to RAM:000efc and controller port 0x8005 on state 6 when BITS:0013 is clear; when set, it skips both writes but still sets RAM:000f75 and advances RAM:000f04 to state 7. The user reports that a C4 copy of these state-6 writes works at 4x, 16x, and default speed; longer-run stability and precise causality remain unverified. Evidence: `../redumper/redumper.ixx`, `src/scsi/set_cd_speed.c`, canonical 1.04 Ghidra disassembly, and user runtime report (2026-09-29).

- For PX-W5224A 1.04 ordinary D8 format-8 READ, BITS:0013 is clear. State-7 controller handling at RAM:f91310..f913c4 reads the main cursor from controller ports 0x800b..0x800d and, below 52x with no slot fault, defers raw gathering when the cursor is not aligned to 0x930 (2352) bytes. `ProcessRawSectorBufferEvent` at RAM:f92d0b also applies a low-speed main-cursor alignment gate when BITS:0013 is clear. The current C4 event copy reads the same cursor but bypasses both gates by setting BITS:0013 and gathering immediately. This is a confirmed control-flow difference; its contribution to the user's stage-4 hard hang is unproven. The shared batch-preparation helper RAM:fc683c can also reinstall the stock alternate coroutine RAM:f929ab when the queue is empty and the producer is inactive, even after C4 installed its own coroutine. Evidence: canonical Ghidra disassembly/decompilation and `gofw-pxw5224a/src/controller_read_event.c` (2026-09-29).

- In stock PX-W5224A 1.04, `InitializeRawReadSpeedForCurrentDriveState` at RAM:fbf67c is called by the alternate raw coroutine RAM:f929ab, its queue-pressure helper RAM:f90b91, and the request-5 pipeline-target dispatcher RAM:fbf2d8. It copies selected speed RAM:000c37 to current speed RAM:000e46, applies drive-family, format/media, BITS:000e, BITS:0013, RAM:000c85, and recovery-count limits, and publishes RAM:000f3a/RAM:000e49 while clearing RAM:000f22/RAM:001037. BITS:0013 is the specific 4x cap on the negative-D8 path; removing that condition in a C4-only copy preserves the other ceilings. Evidence: canonical `/validated/MONT_104.bin` Ghidra decompile and accepted `src/platform/status_mapping.c` reconstruction (2026-09-29).

- Canonical PX-W5224A 1.04 Ghidra shows the alternate raw producer's queue-pressure helper at RAM:f90b91 invokes `DispatchReadAcquisitionRequest5ForPipelineTarget` at RAM:fbf2d8 after setting RAM:000f10=-10. The nonreturning raw producer finalizer at RAM:f92815 also invokes RAM:fbf2d8 before idling. That dispatcher computes RAM:000c8e from RAM:000f10+RAM:001598+RAM:00159c, sets request kind 5 at RAM:000c8c, and calls RAM:fdc96f. Therefore reinterpreting RAM:001598 as a descriptor sequence requires C4-specific handling at both sites. A post-dump user-flashed count-only C4 RAM snapshot had RAM:001598=150, RAM:000f46=0, RAM:000f04=0, and 412 queued descriptors; this does not by itself identify which path first failed or prove mechanical movement. Evidence: `/validated/MONT_104.bin` and `out/agent-tests/c4_live_snapshot.json` (2026-09-29).

- In stock PX-W5224A 1.04, `DispatchReadAcquisitionRequest5ForPipelineTarget` at RAM:fbf2d8 computes a pending target from RAM:001598, RAM:00159c, and RAM:000f10, sets acquisition request kind 5, and dispatches it. The stock `ExecuteRawSectorReadRange` failure branch at RAM:fd6cf2 calls this after resetting the raw queue and installing idle coroutine 2. Omitting that dispatch in C4's separate failure branch removes this explicit post-failure movement request; the caller's existing error path clears RAM:000c2a so the following C4 READ reinitializes acquisition. This is code-path evidence, not yet physical proof of post-fault recovery.

- On the user-flashed stage-one SpiralDrive PX-W5224A 1.04 image, C4 returned GOOD/full 2742 bytes and valid-CRC Q for LBA 913 while its 2352-byte main region matched stock D8 LBA 912 exactly; the next C4 frame had Q for LBA 914 but zero main bytes, while stock D8 format 0 and 8 at LBA 914 both returned the same nonzero main bytes. This establishes a C4 main/subchannel association fault, not merely an all-zero disc sector. By contrast, a later all-zero C4 frame in audio track 2 at Q `00:23:46` may be genuine silence: stock D8 LBA 1646 was also all-zero. Evidence: `out/agent-tests/gofw-stage1-live/`.

- Canonical PX-W5224A 1.04 disassembly at RAM:f91310..f913c4 shows that `ProcessControllerReadEvent` reads the normal raw format-4 main-data cursor from controller RAM:01800b..01800d and retains it through the format-4 branch. RAM:018008..01800a is selected only for other formats. Negative-D8 mode skips the 01800b read and can therefore reuse the auxiliary/C2 cursor. The C4 event copy incorrectly used 018008; a live zero-main RAM snapshot showed its main and C2 cursors equal at RAM:00126e and RAM:00126a (`0x001267e4`). After the one-byte correction to 01800b was flashed, eight consecutive C4 format-8 frames at Q-derived LBAs 945..952 matched stock D8 format-8 frames **byte for byte** across main, C2, and subchannel. The clean interval validates the correction for those frames; damaged-disc C2 and longer runs remain untested. Evidence: `out/agent-tests/gofw-main-cursor-fix/live/`.

- Stock PX-W5224A 1.04 `InitializeRawReadSpeedForCurrentDriveState` at RAM:fbf67c explicitly caps RAM:000e46 to 4 when BITS:0013 is set (RAM:fbf6de..fbf6ef); canonical disassembly confirms the branch. The current C4 alternate coroutine calls this stock speed initializer while using BITS:0013 to select `0x10000000` acquisition and the C4 event path. Thus clearing BITS:0013 to gain speed also changes acquisition/event routing; a C4 speed policy needs those roles decoupled first or a C4-specific speed initializer.

- Stock PX-W5224A 1.04 `RunMediaStateCoroutine1` calls `ProcessReadAcquisitionState` with a three-byte near `LCALL` at RAM:f85def, immediately followed by another near call at RAM:f85df2. The worker calls `AcquireDiscReadPosition` with a complete four-byte far `ECALL` at RAM:f865ba. Canonical Ghidra disassembly confirms these boundaries. Replacing the latter is a localized C4 selector patch; moving it to the former would require a bank-crossing trampoline and instruction-boundary repair, not a net reduction in stock changes.

- In the immutable PX-W5224A 1.04 download, RAM:fef194..feffff (file offsets `0x6f194..0x6ffff`) is a contiguous 3,692-byte `0xff` region after the canonical function at RAM:fef170..fef193. Canonical Ghidra labels it `PostCodeErasedRegion` and its full FE-tail data-region scan reports no xrefs. It lies inside the main executable image, before the RAM:ff0000 monitor-overlay boundary. This establishes candidate payload capacity, not runtime validation of newly linked code there. The rolled-back FD-only C4 revision does not use this region.

- In stock PX-W5224A 1.04, `ProcessReadAcquisitionState` has a complete four-byte far call at RAM:f865ba to `AcquireDiscReadPosition` RAM:fc818b. Routing this call through an FD-bank selector preserves the original far-return frame: focused S251 probes reached the unchanged stock entry for D8 and the FD C copy for C4 with SP=0x63, R11=0x33, and R15=0x5a from the same synthetic call context. This allows removal of a patch at the near-returning positioning callee RAM:fc858f without adding code outside the FD erased gap. The C4 copy remains untested on hardware.

- In stock PX-W5224A 1.04, `ValidateRawSectorReadFrameRangeAgainstMediaBounds`
  at RAM:fd709c is called only by stock `ExecuteRawSectorReadRange` at
  RAM:fd6cf2 (canonical Ghidra callers). C4's C raw-range clone does not call
  that validator, so the former C4 opcode alias at RAM:fd70be is unnecessary.
  After removing it, an independent generated-image comparison confirms its
  four stock bytes remain unchanged. This is offline reachability evidence;
  the revised image has not been flashed.

- Stock PX-W5224A 1.04 producer step RAM:f92a69 and helpers RAM:f90b91,
  RAM:f92b52, RAM:f93429, and RAM:f934d7 return with near RET. An FD-bank
  C251 ECALL cannot directly enter these near-returning functions. C copies
  linked in the FD bank avoid the F9 `LCALL; ERET` gateways entirely; only
  unchanged far-returning lower-level functions need ABI adapters. The stock
  throttle at RAM:f90b91 saves DR12 (including WR14) across calls/yields;
  its C4 adapter must preserve that inherited register state. Canonical 1.04
  disassembly and the C4 Keil listing/map establish these return and register
  contracts. The C port remains untested on hardware.

- On a user-supplied damaged disc in the flashed PX-W5224A 1.04 C4 image, a sequential C4 lead-in scan returned a nonzero 294-byte C2 vector at the 421st frame after D8 -20149 positioning and C4 SEEK 0 queue reset. The three nonzero bytes were vector offsets 144=`80`, 150=`0c`, 152=`02`; the 16-frame C4 command returned GOOD/full 43872 bytes. This is physical evidence that C4 can transfer nonzero C2 data, although the frame was a lead-in TOC frame with zero main data and invalid Q CRC, so it does not establish C2 accuracy for damaged data sectors. See `out/agent-tests/gofw-c2-scan/` for exact CDBs, logs, and binary batch.

- On the user-flashed PX-W5224A 1.04 C4 read-event-copy image (selected F5
  code bytes matched the scratch image SHA-256 `8580da04...`), two successive
  C4 one-frame READs both returned GOOD/2742. The 2352-byte main regions had
  2339 and 2335 nonzero bytes; their frame hashes differed. Both sub-Q CRCs
  were valid, with absolute MSF 00:16:18 then 00:16:19. This physically
  confirms that this earlier cursor selection produced nonzero main data on
  those two frames while the descriptor cursor advanced. Later live evidence
  found wrong main/subchannel association and zero main on other frames, so
  it did not establish correctness of RAM:018008..01800a for format 4. Both 294-byte C2
  regions were zero; without known error sectors this does not validate C2
  error pointers. Exact CDB, readbacks, and captures are in
  `out/agent-tests/gofw-read-event-clone-live/`.

- In stock PX-W5224A 1.04 `ProcessControllerReadEvent`, the two-byte
  controller slot word at RAM:f911d3..f911e9 uses the first port read as the
  high byte and the second as the low byte. The auxiliary word at
  RAM:f911ed..f91203 follows the same order. A focused S251 case with port
  bytes `20/01` and ring size `0x100` reached RAM:f913d0 with adjusted slot
  RAM:001262=`0x2100`; the relocated C4 copy initially produced `0x021f`
  using the old decompilation's reversed expression. Correcting the C copy
  to sequential high-byte-first reads matched stock slot and auxiliary word.
  The selected main and auxiliary three-byte cursors must be read in controller
  port order; Keil C251 reordered
  volatile reads when expressed in one C expression, while separate C
  statements preserved the stock order in its listing.

- In stock PX-W5224A 1.04, `ProcessControllerReadEvent` RAM:f9130a..f9130f
  branches around the main-data cursor read in negative mode. After a C2
  cursor is put in stack scratch, internal format 4 also skips the alternate
  main cursor read at RAM:f913ad; RAM:00126e then inherits the C2 cursor.
  A C4-only trampoline can select the existing RAM:f913ad path while preserving
  ordinary and D8 destinations. A focused S251 case with separate synthetic
  C2 and main controller cursors gave stock/patched D8 identical
  RAM:00126e=`0000aabb` and patched C4 RAM:00126e=`00563412`, with the
  original SPX restored at RAM:f913d0. This is an offline proof of cursor
  selection, not physical validation of the resulting main-data transfer.
  On a physical disc, one positive D8 format-8 READ returned 2335 nonzero
  main bytes and 294 zero C2 bytes with valid sub-Q, demonstrating that a
  zero C2 vector alone is possible when main data is present. Canonical
  RAM:fe0037..fe003b copies the independently sampled C2 cursor RAM:00126a
  into the special descriptor's RAM:0012ac field, and RAM:fd739b adds 0x22
  before programming the C2 host transfer address. The C4 cursor override
  leaves those stock C2 paths intact. Real C2 error bytes on lead-in media
  remain a hardware question.

- In firmware 1.04, `ExecuteRawSectorReadRange` at RAM:fd6cf2 writes each
  request's exclusive end to RAM:000f14, then derives the pipeline target from
  its start plus the format bias. `PositionRawReadPipelineAtFramePosition` at
  RAM:fc5af5 chooses a descriptor cursor for that virtual target; requesting
  the same negative D8 start can revisit buffered data. After repeated C4
  one-frame reads with the hardcoded start, a live F5 snapshot on H: showed
  BITS:0013 set, RAM:000f14=`ffffb1e2` (previous exclusive end),
  RAM:000f32=`ffffb1e1` (last start), RAM:000c2a=3 and raw queue indices
  RAM:001294=0, RAM:001296=506, RAM:001298=1. The consecutive post-SEEK
  C4 captures were byte-identical. Canonical RAM:fc65f4 scans from secondary
  cursor RAM:001298, and RAM:fd6fe0 transfers validated descriptors; the
  live cursor value 1 after a one-frame read is consistent with that advance.
  Retaining this cursor at only the RAM:fd6ea6 raw-range position call is a
  smaller C4 continuation than advancing RAM:000f14. On a flashed 1.04 image,
  two C4 READ pairs around a SEEK returned valid-CRC sub-Q absolute MSF
  00:14:33/00:14:34 and 00:01:15/00:01:16; after the second pair,
  RAM:001298=2 while RAM:001294=0. This verifies one-frame sub-Q progression
  across C4 READ commands on that disc. One two-frame C4 READ returned 5484
  bytes with valid-CRC Q 00:01:17 and 00:01:18, continuing from the preceding
  one-frame command. Two further two-frame commands yielded valid-CRC Q
  00:01:19/00:01:20 and 00:01:21/00:01:22, confirming continuation across
  the multi-frame command boundary on that disc. All main and C2 bytes were zero, so
  it does not verify complete frame capture. The stock producer at RAM:f912cc still switches
  special-mode reads with its internal target above -1000 into packed
  subchannel handling, regardless of C4's fixed request start.

- In Keil C251 5.60, reading or writing a `volatile u32` through a dynamic
  near or huge pointer emitted two 16-bit memory operations in the C4 movement
  build. Declaring the already-existing stock delay word as
  `volatile u32 near ... _at_ 0x11fc` emitted one direct 32-bit `MOV` load or
  store, matching firmware 1.04 RAM:fd0f57..fd0f77. The linker records the
  alias as absolute EDATA RAM:0011fc..0011ff; it does not allocate fresh RAM.

- Firmware 1.04 HOME RAM:fb084e calls YieldCoroutine1 at five sites, and
  pickup move RAM:fd07de plus its finalizer RAM:fd0ecf call YieldCoroutine1
  at three sites total. These routines are unsafe to invoke unchanged from a
  SCSI coroutine-0 handler. The C4 project clones them into the same fd code
  bank, substitutes RAM:fee529 YieldCoroutine0, and byte-checks all changes
  against the immutable stock image. RAM:fd00a5 returns with ERET, so its
  far bridge must tail-jump, while near clones returning with RET need local
  LCALL/ERET adapters. Canonical 1.04 disassembly and Keil listings/map
  establish the call and return classes; later live tests are recorded below.

- In Keil C251 v5.60, a direct `if(sbit)` compiles to `JNB` for the PX-W5224A
  D8 bit test; `if(sbit != 0u)` compiled to a longer carry/ACC sequence.
  Expressing the CDB validity OR as one expression, with the CDB[1] term
  first and CDB[11] term second, emitted the stock `ORL ACC,B` order. The
  initial standalone C0 D8 clone then matched all 109 stock D8 bytes except
  the eight changed bytes across three ECALL target operands, verified from
  generated firmware bytes after linking. The later OmniDrive CDB handler
  deliberately changes that first-level byte sequence.

- Firmware 1.04 INQUIRY vendor-specific response bytes 36..55 come from image
  offsets 0x1c..0x2f through `InitializeInquiryResponseBuffer` at RAM:f84900.
  The separate `PLEXTOR MONT` update identity begins at image offset 0x30.
  Firmware 1.04 stock opcode reads at RAM:fda98c, RAM:fdabfc and RAM:fd70be
  each use the four-byte `MOV ACC,RAM:000b0a`; an ECALL alias at those sites
  can present C0 as D8 to their following decisions without changing CDB[0].
  The stock C0 dispatch and media entries are image offsets 0x41f7 and
  0x43b7; D8 entries are 0x4257 and 0x43cf.

- Stock 1.04 TimerDispatchPhase0 RAM:fd1b3d checks drive-control state
  RAM:001300==0x0a at RAM:fd1b85 on the debounced P1.6 rising edge when
  RAM:00faed bit0 is set. The old C incorrectly read RAM:000c43 there;
  request-code writes at RAM:fd1bed/fd1c4c still correctly use RAM:000c43.
  Corrected source passes 65,536 independent state/request host pairs with
  Release assertions enabled. Two explicit S251 edge scenarios mismatch
  before and pass after (declared transition-marker stub). Evidence:
  `docs/DIAG16_P1_INPUT_STARTUP_ANALYSIS.md` and
  `tests/simulator/cases/timer-phase0-drive-state.json`. This does not prove
  a cause or cure for the physical P1.7-associated startup failure.

- Firmware 1.04 predicates RAM:fb01c2 (P1.5), RAM:fb0664 (P1.6), and
  RAM:fb01d0 (P1.7) read RAM:0011ed, the debounced P1 sample, with masks
  0x20/0x40/0x80. RAM:001300 is independent drive-control state. The old C
  helpers and their host tests both used 001300; generated zero/zero smoke
  seeds masked the defect. All six independent asserted/released S251 cases
  mismatch before and pass after the address correction. The host regression
  checks all 65,536 sample/state pairs and the exact read address. Canonical
  plate comments, `tests/simulator/cases/debounced-p1.json`, and
  `docs/DIAG14_ENUMERATION_AND_TRAY_ANALYSIS.md` retain the evidence. This does
  not identify individual physical switches or establish a physical tray fix.

- The 2026-09-26 diag3.txt physical capture following diag14 contains actual
  36-/96-byte Plextor INQUIRY payloads and matching success CSWs, establishing
  optical command completion beyond USB bridge enumeration. Its endpoint
  RESET/SSTS data-overrun rows coexist with completed short replies and
  correctly accounted CSW residue; they alone do not establish drive resets.
  Example: MODE SENSE allocation 0xfff0, payload 72 bytes, residue 0xffa8,
  status zero. Capture SHA-256 b288fc0b439c617d091cf776b86a7b8e403260019a3ea3eb49596e209740a262;
  image identity inferred from user flashing workflow, not a live readback.

- Firmware 1.04 host ISR RAM:f8007f has a nonreturning restart arm, but still
  executes RETI at RAM:f800b5 before entering RestartFirmware at RAM:fee819.
  Replacing that arm with a direct C call abandons the interrupt-in-service
  latch. The S251 cause-0x10 restart-entry comparison alone misses this defect:
  both reach initialization, but a second same-priority IRQ without CPU RESET
  is serviced by stock and remains pending in the old reconstruction.
  `tests/simulator/trace_host_restart_irq_liveness.py` verifies delivery after
  the corrected addressless RETI adapter and fails when its debugger-only
  negative control bypasses that adapter. This is a confirmed CPU-boundary
  requirement, not evidence that physical enumeration/tray behavior is fixed.

- Firmware 1.04 RAM:fd902a, RAM:fd902f and RAM:fd9034 are 32-, 16- and
  8-bit width entries into one integer-to-binary64 core. The core consumes a
  separate ACC sign indication, converts negative raw input to magnitude,
  left-normalizes within the selected width, returns a zero pair when no set
  bit exists, and otherwise assembles sign, exponent and mantissa. Explicit
  signed C interfaces are exact for all three source widths.

- Firmware 1.04 RAM:fd92af `AddFloat32` treats every exponent-zero binary32
  encoding as zero, so subnormal payloads are not accumulated, and routes
  every exponent-255 input (including an infinity encoding) to the fixed raw
  result `0xffffffff`. RAM:fd92ac `SubtractFloat32` is only a raw sign-bit
  inversion of the right operand followed by the same addition core. Ordinary
  normalized finite inputs use the runtime's alignment, cancellation,
  normalization and rounding paths.

- Firmware 1.04 RAM:fd9367 `MultiplyFloat32` flushes exponent-zero operands
  and underflow to positive zero, treats exponent-255 operands as infinities,
  and returns raw `0xffffffff` when zero and exponent-255 classes are combined.
  RAM:fd9407 `DivideFloat32` returns that fixed invalid result for zero/zero or
  exponent-255/exponent-255, positive zero for a zero numerator, exponent-255
  divisor or underflow, and sign-XOR infinity for an exponent-zero divisor or
  exponent-255 numerator. Their ordinary normalized finite routes use the
  runtime's binary32 product/quotient normalization and rounding.

- Firmware 1.04 RAM:fd90db `ConvertFloat32ToFloat64` flushes every binary32
  exponent-zero encoding to positive binary64 zero, widens normalized finite
  values exactly, maps infinity by sign, and maps a nonzero exponent-255
  payload to raw `0xffffffffffffffff`. RAM:fd911d
  `ConvertFloat64ToFloat32` flushes exponent-zero input and narrowing underflow
  to positive zero, maps every exponent-0x7ff input (including infinity) to raw
  `0xffffffff`, and maps finite overflow to signed binary32 infinity.

- Firmware 1.04 RAM:fd94ce, RAM:fd94d3 and RAM:fd94d8 are width-selection
  entries into one integer-to-binary32 normalization core. The native ACC sign
  indication tells the core whether to negate the raw input magnitude; the
  32-, 16- and 8-bit entries load loop widths 32, 16 and 8, with the narrower
  entries clearing unused high bytes. Explicit signed C interfaces preserve
  the source-level conversions; all signed 8- and 16-bit values are exact,
  while signed 32-bit values use binary32 rounding.

- Firmware 1.04 RAM:fd950d `ConvertFloat32ToInteger32` is a raw 32-bit
  truncation helper, not ISO C's overflow semantics. Magnitudes below one
  return zero; unbiased exponents zero through 31 shift the implicit
  significand into a dword and negative inputs are restored by two's
  complement; exponents above 31, including exponent-255 encodings, return raw
  `0xffffffff` regardless of sign.

- Firmware 1.04 RAM:fd924b `NegateFloat64` is a raw sign-bit operation with
  two high-word guards: values whose first binary64 word is `0x0000` or
  `0xffff` return unchanged, while every other encoding toggles bit 63. Thus
  positive subnormals in the guarded `0x0000` slice are not negated, negative
  zero becomes positive zero, and the guarded negative special slice remains
  unchanged.

- Firmware 1.04 RAM:fd9079 `ConvertFloat64ToInteger32` has the same raw result
  policy as the binary32 conversion: magnitudes below one return zero,
  unbiased exponents zero through 31 truncate the high significand toward zero
  and reapply sign by two's complement, while larger exponents—including
  exponent-0x7ff—return raw `0xffffffff` regardless of sign.

- Firmware 1.04 RAM:fe345c is a no-input ACC-Boolean ERET worker. Fixed-
  packet mode aligns from working-track start minus five in packet-size-plus-
  seven steps; a negative preliminary count is corrected by another packet-
  plus-seven and uses `aligned + 2*packet_size + 15` as the bounded controller
  end. Ordinary sentinel mode uses 300 minus the target/start displacement,
  while working-track types two, three and five use end minus target minus two
  without the ordinary ten-frame floor. Every range/acquisition failure maps
  to 03/0c08 at the current target; prefix success publishes target minus one.

- Firmware 1.04 RAM:fe25d4 has no inherited input and returns through near
  RET. RAM:0088b5 equal to one restricts it to the Write-Type-2 lead-in scan;
  the ordinary route repeats the boundary-distance helper until nonzero,
  advancing only across type-one records after each zero. Index zero publishes
  relative distance minus one and marks the current packet special; index one
  publishes relative zero and sets RAM:0014d0; later indices publish no tuple.
  After decoding the selected Q type, values zero or above three derive the
  next index-one type-one boundary minus 150 at RAM:0014d1.

- Firmware 1.04 RAM:fed50b has no inherited input despite preserving WR14:
  only write type two consumes the record byte and that path overwrites WR14
  from byte zero of the current eight-byte track record before use. Q types
  zero, one, two, three and 0x41 publish fixed layouts; types eight, nine and
  thirteen select geometry from record families 0x10/0x30/0x20, respectively,
  while types ten through thirteen publish packed-MSF descriptors with tag
  two (type eight uses tag one). Record high bits add a 0x60 auxiliary region
  without changing the primary component length at RAM:001470. Unknown
  nonzero Q types return without even clearing controller address-five byte
  two.

- Firmware 1.04 RAM:febbd1 takes unsigned frame spacing in DR4 and returns
  Boolean status through near RET. Configuration other than two runs one
  window, configuration two ordinarily runs five, and indexed state 0x11 runs
  up to 1000 using the ATIP lead-out minus two as every endpoint. That route
  invalidates and reruns OPC after indices congruent to 19 modulo 20 and maps
  failure to status eight, or seven when RAM:00fad7 bit three is set. Indexed
  state 0x1f forces start 45000 and exits after one window; state 0x11 forces
  start 750. Successful completion conditionally builds the Q queue and clears
  RAM:001302 bit two.

- Firmware 1.04 RAM:feb8b4 takes operation mode in ACC and requested speed in
  R7 and returns Boolean status through near RET. After the common reset,
  readiness and two-stage OPC sequence, mode zero prepares synthetic raw-read
  state; mode three repeatedly acquires windows using the zero-extended word
  at RAM:00f9df until failure; mode 0x1f does the same with spacing ten; and
  mode four performs one spacing-30 acquisition then initializes coroutine one
  at RAM:f84b0b. These three dispatch blocks are instruction-confirmed even
  though Ghidra currently prunes them from pseudocode. Mode-four speed selects
  10x/8x/4x from direct P0.6/P0.2 when media class is one, while classes two
  and three force 24x.

- Firmware 1.04 RAM:fe7e4b is the no-input, ACC-Boolean ERET planner shared
  by BLANK and FORMAT UNIT. Operations zero and one acquire lead-in-minus-1000,
  the wrapping lead-in-to-zero span and distinct terminal ranges; operation
  two applies the 145-frame adjustment only when working-track control bit
  0x10 is set; operations three and six use the ten-record Q sequence; four
  starts at RAM:00e602 plus 145; and five uses both inclusive stored ranges.
  Its near calls to RAM:fe825b leave the signed start's high byte in inherited
  R15 through the native DR12-to-DR4 move, so a C-facing adapter must copy R7
  to R15 before LCALL rather than treating that register as unconstrained.

- Firmware 1.04 RAM:fe9821 takes expected sector count in DR4 and Q-record
  count in WR2 and returns Boolean status in ACC through near RET. A zero
  expected count bypasses all initialization. Otherwise it processes
  `expected_count / 10` groups, copies ten Q bytes from RAM:008012 plus ten
  times the group index, captures ten source-kind-ten sectors per group, and
  finishes with one source-kind-six capture. The preserved WR14 copy of the
  Q-record count is decremented once per group but has no observable effect.
  Main-data cursor advancement wraps in 0x930-byte units against 0x930 times
  the 32-bit slot count at RAM:00156a.

- Firmware 1.04 RAM:fe5f15 returns zero for terminal failure, one only after
  completed blocks equal requested blocks, and 0xfe for resumable/pending
  service. Its controller-active poll counter starts at 25, yields after the
  old value exceeds 30, then repeats from zero. At counter values congruent
  to seven modulo eight it may publish one descriptor early when the little-
  endian controller offset at RAM:018016 is below the remaining byte span.
  For write type two, compare result zero advances the track/Q state before
  finding the preceding type-one record; result one only decodes that record,
  while results two or greater report a position sequence error.

- Firmware 1.04 RAM:fe59b4 snapshots write type once after common raw-read
  state reset and dispatches modes in native order 0, 2, 3, 1. Mode zero has
  a two-stage acquisition-plan path around the close/sync continuation poll;
  its second-plan failure intentionally leaves RAM:0014b4/0014b6/0014b7
  asserted. Mode two acquires lead-in descriptors only while RAM:001404 is
  zero. Mode three decrements RAM:001598 with wrap, acquires one frame, and
  selects a 16-byte bank-one clear for Q type one or a 96-byte fill for Q
  types two and three; unknown nonzero Q types retain RAM:00127f.

- Firmware 1.04 RAM:fe577e classifies the current write speed before its first
  Q-record queue build. Below 12x, failure is terminal. At 12x or above,
  failure forces 4x, runs OPC unless RAM:000c46 bit zero bypasses it, retries
  the queue build, and restores RAM:000c38 only after retry success. A nonzero
  write type returns success with RAM:00128b still asserted; only failure or a
  completed write-type-zero acquisition clears it.

- Firmware 1.04 RAM:fde4cf prepares the FORMAT UNIT code-0x11 Type-2
  controller-record pipeline. It clears the controller-record index-state
  table through a stored 24-bit pointer whose low 16 bits are incremented,
  performs three distinct volatile masks on RAM:000c8b (0xfb, 0xfd, 0xfe),
  and gates success in order on primary slots, the forty-frame base window,
  and the TDI sequence before building all workspace pages.

This file contains durable architecture and reconstruction facts. Current
status, priorities, and hypotheses belong in `active_context.md`; workflow and
safety rules belong in `AGENTS.md`; exact per-function and per-object facts
belong in the canonical Ghidra programs.

- Firmware 1.04 `RAM:fe19de..fe1a15` is a 56-byte near-RET compiler fixture
  with source-order inputs `flags`, `class`, and `detail` naturally assigned by
  C251 to `ACC`, `R7`, and `R6`. Its instruction shape is two nested selectors:
  the outer selector destructively decrements R7 across values zero through
  two, and the inner selector cumulatively adds `0xf8`, `0xf8`, and `0xf0` to
  R6 for original values 8, 16, and 32. C251 5.60 O3-O6 reproduces the outer
  nested-switch lowering and shared return but not the complete body; default
  O0-O8 and O3-O8 SIZE/SPEED sweeps contain no exact match.

## Processor and ABI

- Firmware 1.04 RAM:fe0000 is an extended-return descriptor builder with
  conditional inputs: WR6 supplies the two-byte descriptor status and ACC the
  descriptor mode only when RAM:00127e is one. The ordinary path ignores those
  inputs, copies RAM:001272..00127b into RAM:0012ac..0012b5, uses the low word
  of RAM:001266 for Q type one or RAM:001262 otherwise, and publishes a fresh
  Q type at RAM:0012b7. Q type one reads controller metadata at
  `RAM:00156e + 16*RAM:001266 - 1` and maps bit seven to RAM:00127c; Q type
  0x0d reads `RAM:00126e + 0x12` and mirrors bit five into RAM:0012b6 bit six.
  RAM:0014d0 or the signed `RAM:001598 > RAM:0014d1` gated condition forces
  descriptor bit seven and clears RAM:0014d0; otherwise RAM:00127c controls it.

- Firmware 1.04 RAM:fe9af2 receives `alternate_plan` in ACC, retains it across
  the coordinator, and returns normalized Boolean status through near RET. A
  false result from RAM:fe9d02 is deliberately translated to success, not
  failure. The first RAM:fe9cce result feeds RAM:fea65a; after that succeeds, a
  fresh RAM:fe9cce result is passed in ACC to RAM:fea6c4 while the retained
  alternate-plan flag is passed in R7. Write types zero and one refresh
  RAM:001285 and call RAM:fe9e64 with ACC zero while its inherited R15 fallback
  is the second eligibility result. The mapped status inputs are loaded in the
  native order RAM:00dd33 to R6, RAM:00dd32 to R7, then RAM:00dd31 to ACC.

- Firmware 1.04 RAM:fea6c4 receives `state_eligible` in ACC and
  `alternate_plan` in R7 and returns normalized Boolean status through ERET.
  Its initial RAM:fe9e64 call receives ACC zero and the still-live R7
  alternate-plan value as the fallback class; decompiler output that shows
  only the ACC argument is incomplete. Write types one and two use the
  ordinary segment policy. Write type zero selects a flagged policy when
  RAM:00128b or RAM:0014b4 equals one, an ordinary policy for working-track
  type zero or exact alternate-plan one, and otherwise a forced five-frame
  policy. The flagged policy uses seven- and twelve-frame splits, decrements
  RAM:001598 only after successful suffix/Q acquisition, and populates from
  the ring cursor captured before the acquisition helpers.

- Firmware 1.04 RAM:fecb50 returns a three-valued status in ACC through ERET:
  zero is terminal rejection/unavailability, 0xfe means poll again, and one
  means the batch completed. A complete batch emits five leading, thirty-two
  payload, and two trailing descriptors. Each payload auxiliary record stores
  the current three-byte Q MSF plus byte two at cursor+0x10000c, then two
  copies of the RAM:000c71 Mode-2 subheader, while the fixed twelve-byte prefix
  comes from RAM:ff967a. The relocation split count is the unsigned quotient
  `(ring_slots * 0x930 - destination) / 0x930`; if it is below the batch count,
  the transfer wraps through controller address zero. Trailing capacity
  failure latches context but does not abort descriptor emission.

- Firmware 1.04 RAM:fec121 receives an exact Boolean initialization request in
  ACC and returns normalized Boolean status through ERET. Its saved DR12 is
  preservation only: R12 is explicitly cleared before the 64-entry recovery
  scan, while R13 holds completion status, R14 the pending split count and R15
  the pending source index. Controller mode bit zero permits a 32-sector
  batch; otherwise the native path caps it at 31. Recovery compares four-byte
  prefixes in the 64-entry table at RAM:f84490 and loads replacement targets
  from RAM:f84690 using the same four-byte stride.

- Firmware 1.04 RAM:fe8b66 receives `first_record_index` in ACC and returns
  normalized Boolean status through near RET. It clears ten-byte records from
  that index through 219 but builds output from record zero. ADR-1 records use
  the table at RAM:2bb03b, ADR-3 at RAM:2bb423 and ADR-4 at RAM:2bb4ff, all
  with one-based source indices and ten-byte strides. After ADR-3 and ADR-4
  append phases it clears tuple bytes greater than or equal to the selected
  track and subtracts records for which all six tuple bytes were cleared;
  this adjusts the output cursor without separately compacting the table.
  The synthetic seed uses configuration offset 0x7a, seeds the PRNG from the
  configuration dword's low word and stores the low 24 product bits as BCD.

- Firmware 1.04 RAM:fea45c takes an exact Boolean partition request in ACC
  and returns a signed range in DR4 through near RET. With partitioning it
  computes `RAM:00dd3d-RAM:00dd51`, compares that and
  `RAM:00dd3d-RAM:00dd39` signed against 150, and writes segment state in the
  native order. Long ranges can write RAM:001430 first from the track distance
  and then overwrite it from the selected range; when only the track distance
  is at most 150, mode three remains even though the final first count becomes
  `range-150`. RAM:fe2181 `ComputeFrameDistanceToNextType1TrackRecordBoundary`
  is a near-RET helper and requires explicit `PLEXTOR_NEAR_FUNCTION` typing.

- Firmware 1.04 RAM:fd0380 takes a 32-bit reference in DR4, unsigned radial
  quotient in WR2 and operating code in ACC, then returns an unsigned 32-bit
  reference in DR4 through near RET. Its dynamic model divides the reference
  signed by 750, scales through RAM:000cf9, 1000, 113 and 355, adds 62500 and
  takes an integer square root. RAM:00fc25 is zero below root 250, otherwise
  `(root-250)/67` for code one and `/83` for other codes. The final model uses
  the signed byte offset `MapOperatingCodeToProfileIndex(code)-1` from
  RAM:ff89e6, so profile zero deliberately reads the preceding word at
  RAM:ff89e4. The fixed path selects RAM:000ce9/ceb/ced/cef/cf1/cf3 for codes
  0x0a/0x18/0x20/0x28/0x34-or-ff under the native enable/override gates.

- Firmware 1.04 RAM:f870cd takes a signed 32-bit position window in DR4 and
  returns status zero, one, or two in ACC through near RET. Its saved DR12 and
  eight-byte scratch frame are function-owned target/window state. Acquisition
  result `0xff` restores the original RAM:000e3b target only when no callee has
  changed the windowed value; unexpected results other than zero, one, or
  `0xff` deliberately loop forever. The worker uses two independently scaled
  position thresholds, `(65 * effective_code) / RAM:00fafc` and
  `(50 * effective_code) / RAM:00fafc`, followed by a fixed 30-frame wait.
  P0.7 low triggers controller reinitialization, while P0.7 high proceeds
  through profile configuration and two readiness probes.

- Firmware 1.04 RAM:f86d78 is a no-input near-RET media-state-five
  coordinator; its saved DR12 and nine-byte frame are function-owned, not an
  inherited ABI. The frame holds a byte retry count plus signed 32-bit position
  window and previous-position state, while DR12 retains the acquisition target
  across helpers. Status two restores that target and retries; other nonzero
  status clears RAM:00f971 bit zero and exits. The instruction-only RAM:001305
  bit-five extraction before the window call is a real volatile read even
  though its scalar result is discarded. External completion copies BITS:0033
  into RAM:001302 bit three before restoring the read path.

- Firmware 1.04 RAM:f86b2a is a no-input, ACC-false near-RET state-three
  indexed-transition worker. Every indexed-state/configuration test is a fresh
  volatile read. State 0x0c (not 0x0f) dispatches immediately; state 0x0e uses
  a signed 32-bit comparison against RAM:000fb4 minus 0x1d4c. State four writes
  RAM:f7c01f through a full 24-bit pointer and manages FAAC bits 9 through 11,
  signed RAM:00fae9/RAM:0011da countdowns and coroutine-zero entries
  RAM:fbfbb9/RAM:feb6bb; state nine may restart at RAM:f84c30 and then still
  performs its reload-zero/dispatch tail when the restart helper returns.

- Firmware 1.04 RAM:f876bb has no effective input and returns a signed 32-bit
  state-five position window in DR4 through near RET. It snapshots RAM:000e47
  to RAM:00faea before a fresh-read speed-16 cap, computes the absolute
  difference of signed target/current quotients by 75, applies exact speed and
  distance multipliers, and clamps results at least 0x816 to exactly 0x816
  only while the freshly read target is at most 0x57e4. The previously noted
  5,643-frame clamp was incorrect; canonical instructions use 0x816 (2,070).

- Firmware 1.04 RAM:f85fdf is a no-input near-RET media-state-one worker. Its
  saved WR14 is function-owned: R14 is cleared after each acquisition setup and
  records whether the nonordinary wide-direction path has run. Its four stack
  bytes retain the signed comparison position. RAM:000fba entry normalization
  is two ordered volatile writes (`OR 1`, then `AND 0xfd`), and the
  RAM:00f79a/RAM:00fbf1 restart path intentionally retains the acquisition
  status already held in R15. The function mixes near calls at RAM:f887d4,
  RAM:f863f4, RAM:f86909, RAM:f88a63, RAM:f890d3, RAM:f889c9, RAM:f88984,
  RAM:f88909, RAM:f8877b and RAM:f889e4 with extended helper calls.

- Firmware 1.04 RAM:f87af9 is a normal no-input near-RET worker despite its
  coroutine role: YieldCoroutine1 returns to it, and it performs no stack-
  context transfer. It temporarily saves RAM:000e43 while publishing current
  speed there, selects target RAM:000fe9 from RAM:00114f, the sum of
  RAM:00fb8c/RAM:00fb90 plus 0xa82, or fixed 0x34bc0, and restores the saved
  byte before calibration. Calibration failure states two, three and four are
  intentionally left untouched; other failures clear state and yield.

- Firmware 1.04 RAM:f87dca has no explicit inputs and returns the state-six
  calibration-sequence status in ACC through near RET; saved DR12 is local.
  It clears sixteen big-endian sample words at RAM:00100a before each mode-two
  calibration attempt, accepts exact-one positioning results, gives the
  read-path/window stage three attempts and the position-sample stage four,
  and optionally runs ten mode-four terminal attempts when RAM:000fef is one.
  Sample status one copies RAM:00facf to RAM:001589 and RAM:00ef85; status
  0x0b directly publishes acquisition result 0x1b and returns 0x0b, while
  other stabilized failures flow through RAM:f89c73.

- Firmware 1.04 RAM:f881fd takes unsigned calibration target mode in ACC and
  the tolerance gate in R7, returns normalized Boolean status through near
  RET, and owns its saved R13/WR14 plus native stack frame. Modes zero through
  four use distinct read-position formulas; mode two selects operating codes
  40/32/24/20/16 from RAM:000ff5 bits 11/10/9/8/default when bit 12 is set.
  Only exact tolerance result one is accepted. Acquisition result 0xff is a
  valid retry sentinel only while RAM:000faf is six and RAM:000e3f is one.
  Ordinary terminal exhaustion copies RAM:000e41 to RAM:00faab, but mode four
  intentionally suppresses that copy.

- Firmware 1.04 RAM:f8974a is a no-input ACC-Boolean near-RET state-six
  position-window worker. Poll/status failures and rejected position bounds
  clear only RAM:001301 bit zero, intentionally leaving RAM:000efc and
  controller port RAM:018005 at 0x40. Accepted phase-one or phase-two exits
  additionally clear both of those controller-mode bytes. Its alternate
  window permits a restore/readiness retry only at `RAM:000fe9-0x486`; the
  later `RAM:000fe9-RAM:000fee-0x409` checkpoint fails directly.

- Firmware 1.04 RAM:f8a172 returns status zero, one, two, three or five through
  near RET. Its saved DR12 is private, but the sole native caller reaches the
  LCALL with DR0 equal to the acquisition target plus five; WR0, the signed
  high word, is consumed by RAM:fb9a54 as the interior-spacing reference. The
  evaluator selects big-endian calibration words at RAM:ff92eb below 12x and
  RAM:ff9337 otherwise, interpolates between the selected sample pair, and on
  its first pass copies RAM:00100a plus twice the selected left index to
  RAM:000fd7 before incrementing RAM:00fae1.

- Firmware 1.04 RAM:f8c1a1 takes the unsigned operating code in ACC and
  returns through RET. Profiles 0..9 are eight-byte records at RAM:ff8516 and
  write ports 0x8094/95/96/97/92/91/93/90 in that order. The decompiler's
  removed RAM:f8c356 block is reachable: original operating codes at least
  0x20 replace record offset three with RAM:00fab7 for port 0x8097. Codes
  24/32, 40 and the default class call RAM:fcee18 and skip table writes when
  RAM:001006 is not exactly one; active table routes clear RAM:001302 bit seven.

- Firmware 1.04 RAM:f9dec5 takes the unsigned profile selector in ACC and
  returns through RET. For selectors below 0x14, its main 24-byte source is
  RAM:f8363d plus RAM:00f05f times 24 while inherited R9 selects only the
  four-byte auxiliary record at RAM:00e823 plus R9 times 34. For selectors at
  least 0x14, selector minus 0x14 selects both the main RAM:00e806 record and
  the auxiliary record. Main offsets 6, 9, 13, 16, 20 and 23 pass through
  RAM:f9cd4a; the other eighteen bytes are direct. RAM:000fca is freshly read
  for the final bit-three/bit-four clear before interrupt gates are restored.

- Firmware 1.04 RAM:fd781e takes an unsigned CD-TEXT pack index in DR4 and
  returns Boolean validity in ACC through RET. It derives the source from
  `(ControllerBufferCapacityBytes - 0x8ffc) + index*18`, then adds 0x10 to
  that result's high word. Sequence continuity is checked and advanced before
  CRC validity, so a bad CRC still advances RAM:0015f1; a nonzero sequence
  discontinuity instead clears BITS:001e and returns immediately, while
  sequence zero restarts the expected value at one. The CRC transform copies
  bytes 0..15, appends two zero bytes, and for each byte XORs the two big-endian
  RAM:00f596 table bytes into the next two workspace positions. Only exact
  header 0x8f accumulates payload lengths: offsets 12..15 for type one and
  4..7 for type two, adding each nonzero byte plus one with word wrap.

- Firmware 1.04 RAM:fd74cb takes the READ TOC allocation length in WR6 and
  returns through ERET. Its retry loop increments RAM:0015ee before checking
  the old value against 14, so acquisition work runs for attempt values 1
  through 14; the byte becomes 15 only in the terminating boundary check.
  Status 0x63 accepts an immediate Full-TOC error descriptor or the third
  ordinary nonzero error descriptor and then calls the near empty-response
  helper RAM:fd79bd. Status one accepts an exact non-four `(record_count * 18)
  + 4` length, a stable length after attempt seven, or any result after attempt
  twelve. Admission rejection calls only the empty-response helper; common
  raw-read/coroutine/pin cleanup belongs only to admitted paths.

- Firmware 1.04 RAM:fd72d9 is a no-input near-RET batched raw-transfer worker.
  The apparent inherited DR0 values in decompiler output are register side
  effects of RAM:fef09e's descriptor lookup: the semantic sources are the
  descriptor main address at +4, slot index at +8 and C2 address at +10. It
  programs main address +0x110, C2 address +0x22, per-slot 16-byte metadata or
  per-slot 96-byte subchannel addresses in that order, so a later component
  intentionally overwrites the active controller address and length.

- Firmware 1.04 RAM:fd6cf2 takes signed start in DR4, signed exclusive end in
  DR0 and format in ACC, and returns zero/one in ACC through ERET. RAM:000f14
  is updated with the exclusive end (not the start), while the continuation
  delta uses start minus the prior RAM:000f14 value. Its zero poll-result path
  constructs a signed failure position from the big-endian word reached via
  RAM:001457 as the high half and the literal low half 0x1455; this odd value
  is instruction-confirmed and must not be normalized into an ordinary block
  counter. The format option mask selects the primary table whenever any
  0xfa bit is set, including the 0x02 C2-tail bit; the 294-byte tail is then
  added with 16-bit wrap.

- Firmware 1.04 RAM:fd5875 is the no-input ERET inverse of the mode-1/ID-2
  snapshot builder and its sole post-TOC caller uses ECALL. It restores the
  35-byte state block from controller offset 0x1a, then uses those restored
  lead-in and Full-TOC lengths for subsequent copies. The twelve-byte restore
  from offset 0x1ff6 supplies FirstTocTrackNumber, highest ADR-1 track and all
  PMA table counts before their dynamic copies; after the ADR-1 copy, a zero
  first-track value is normalized to 0xff.

- Firmware 1.04 RAM:fd56b6 takes the unsigned expected snapshot length in DR4
  and returns normalized Boolean in ACC through ERET; its sole caller uses
  ECALL. It first compares the staged big-endian 16-bit length against the
  zero-extended DR4 value and then against 0x28e2, short-circuits vendor and
  product identity comparisons at offsets two and ten, compares the staged
  checksum at 0x28e0 with the additive checksum from controller offset zero,
  and publishes snapshot validity only after every gate succeeds.

- Firmware 1.04 RAM:fd50ae is the no-input WR6-result ERET builder for the
  READ BUFFER mode-1/ID-2 snapshot; its sole caller uses ECALL. It clears the
  full 0x28e2-byte reserved region, serializes twenty fixed/dynamic regions,
  appends the additive checksum over bytes 0..0x28df at offset 0x28e0 and
  returns 0x28e2. The 35-byte state block at snapshot offset 0x1a consists of
  RAM:000cf5[6], RAM:00fbe0[2], RAM:00fbde[2], RAM:00f992[2], RAM:00fc0f,
  RAM:00fc16, RAM:0012bf, RAM:0012c0, RAM:0012be, RAM:001008, RAM:0012c9,
  RAM:0012ca[2], RAM:0012dc[2], RAM:0012c1[4], and RAM:00fad3[8], in that
  serialized order. PMA copying temporarily maps FirstTocTrackNumber 0xff to
  zero and the final zero-to-0xff step also maps a pre-existing zero to 0xff.

- Firmware 1.04 RAM:fd1ed8 is the no-input near-RET SFR:da/db measurement
  worker; saved DR12 is function-owned scaling scratch. It stores each SFR:db
  byte as a word in the sixteen-entry ring at RAM:00100a, adds 0x11 before
  acquisition and 0x78 afterward, and arms at adjusted value 0xbf. Its window
  code is not ordinary frame/27000: it divides the low byte and middle two
  bytes independently by 75, combines them as low quotient plus 256 times the
  middle quotient with 16-bit wrap, divides by 360, then adds 11. Direct bits
  0028/002c/002d reside in bit-addressable RAM byte 25, bit 0036 in byte 26,
  and bit 0039 in byte 27.

- Firmware 1.04 RAM:fd1b3d is the no-input ERET phase-zero timer worker. Its
  saved R15 is not inherited input because P1 overwrites R15 immediately. It
  derives selected active-low P1 inputs as `(P1 & 0xe4) ^ 0xff`, applies a
  three-sample majority filter with RAM:0011ee and RAM:0011ed, and uses the
  debounced P1.6 edge/state to maintain RAM:00faed/00faef handshakes, invoke
  the near P1.6 transition worker, and publish drive-state timeout flags.

- Firmware 1.04 RAM:fd1693 is the no-input ERET phase-two timer worker. Its
  saved WR14 is not inherited semantic input because the body immediately
  clears R14 and R15. It services fast counters every call and the larger
  countdown/serial/SCSI set when the old signed RAM:001212 value is nonpositive,
  reloading that divider to nine. BITS:003d/003e form a one-way alternation,
  the two serial completion paths call the near-RET P1.6 transition worker,
  and the final P3 handshake is gated by RAM:0011dc bit zero with divider
  values nine or four.

- Firmware 1.04 RAM:fcace5 takes include-all in ACC and alternate-format in
  R7, returns total response length in WR6 through near RET, and filters the
  count-prefixed five-byte tables at RAM:ff8c54 and RAM:ff8c86. Each accepted
  record omits metadata byte zero and appends bytes one through four. Primary
  format derives eligibility from metadata bits seven/six, the record's type
  mask, state-one capacity and state-two media family, and may OR the active
  eligibility mask into emitted byte two. Alternate format only tests record
  byte two against the state mask and copies payloads unchanged. Response
  bytes two/three publish state class and normalized include-all; the leading
  big-endian length is total length minus two.

- Firmware 1.04 RAM:fcab1a is the no-input ERET vendor-EB handler. It rejects
  nonzero CDB-byte-one low-five bits, echoes the low nibble of CDB byte two as
  the format and low two bits of byte three as options, and emits three speed
  words. Format one uses raw X multipliers; all other formats convert through
  `WriteCdSpeedKilobytesPerSecond`. Option bit zero selects requested and
  eligible/fallback speeds instead of current speeds. With option bit one,
  the fallback is four outside controller state two; state two selects 24 for
  media codes two/three, zero for family below two, and four otherwise. The
  response is always staged as ten bytes before allocation-length clipping.

- Firmware 1.04 RAM:fca870 is the no-input ERET vendor-E9 handler. CDB byte
  two selects response 01, 02, or bb; CDB byte one bit four gates persistent
  updates. Selector bb rejects any nonzero CDB byte four through eight before
  initializing its payload. Selector two caps the requested magnitude at two
  and reports negative stored adjustments as magnitude-or-0x80. Selector bb's
  disable route writes 40 to RAM:000c43 and caps RAM:000c37 to it. All valid
  responses set byte one to six, zero bytes two through seven before filling,
  and transfer `min(allocation_length, 8)` bytes; non-status-four transfer
  results call `SetNoSenseData`.

- Firmware 1.04 RAM:fc9f94 is a no-input ACC-Boolean/ERET parameter-FC
  calibration. It averages eight unsigned D8-minus-D9 samples and converts the
  resulting byte exactly to binary64 only to compare it with exact integer
  constants 77.0 and 85.0, so the inclusive test is equivalently `77..85` in
  C. Higher means subtract four from FC and lower means add four. RAM:00120a
  is an asynchronous signed timeout; candidates zero and 0xfc stop the loop.
  State zero publishes FC at RAM:00fac0; state one rejects FC at least 0xb0,
  otherwise publishes FC+0x40 and FC at RAM:00fac0/fac1. Parameter restoration
  occurs only on success.

- Firmware 1.04 RAM:fc9a5b is the near-RET adaptive calibration search for
  parameter codes 00/04/1c/24/2c/3c. Codes zero and four invert the candidate
  direction relative to the other four codes. Its wide window is baseline
  plus-or-minus 15, widened by four for code 2c and 42 for code 3c; its narrow
  window is plus-or-minus three. Candidate bytes 0x7c..0x82 are rejected as
  the signed range outside -125..123, RAM:001220 is an asynchronously updated
  signed timeout, and eight SFR:db samples are bracketed by direct IE.7 clear
  followed by IE.7/IE.6/A9.2 sets. The only caller is already reconstructed C,
  so it now supplies the byte through C251's ordinary near-call R7 contract.

- Firmware 1.04 RAM:fc818b is a no-input ACC-result/ERET read-position
  acquisition state machine. Saved R13 and WR14 are function-owned: R13 is
  initialized to the recovery-speed policy before use and WR14 is overwritten
  by packet/position results. It returns 0xff on the controller-abort flag,
  one on accepted near-target or positioned acquisition, and zero on ordinary
  failure. Failed-packet recovery compares ReadAcquisitionResult with seven,
  and controller reinitialization failure also publishes seven. The low-target
  completion path moves four bytes inward and, when controller operation is
  enabled, strobes value eight to controller port RAM:018007.

- Firmware 1.04 RAM:f9199e is a no-input near-RET completed-sector state
  machine. Its low-speed overlapping-stack division sequence is an exact
  `RAM:00126e % 0x930` test; misalignment selects countdown status 0x1105 or
  0x1106 from RAM:000c3c bit zero. Saved R15 is function-owned and indexes the
  seven descriptor expansion used when RAM:0015a3 high bits equal 0xe0 and
  fixed-packet overhead remains. Direct packed flags can be expressed as
  C251 `bdata`/`sbit` objects, so this worker requires no native adapter.

- Firmware 1.04 RAM:f92d0b is a no-input near-RET raw-sector buffer-event
  worker. In its ordinary-mode low-speed path, the overlapping stack-word
  divisions are a 24-bit `RAM:00126e % 0x930` alignment test; a nonzero
  remainder delegates to RAM:f93429. Its accepted signed target window is
  current-7 through current+2 while signed RAM:000f22 is below ten. In the
  active path it publishes target plus one before testing BITS:0018; when the
  old RAM:000f5e value exceeds 18 it then adds 0x6e or subtracts 0x80, giving
  net target changes of +111 or -127, and subtracts 110 from RAM:000f36.

- Firmware 1.04 RAM:f95381 is a no-input near-RET profile initializer. It
  selects eleven-byte profile rows using clamped speed codes 2/4, 8/16 or 10
  and RAM:00f05f, with RAM:00ef72 bit three overriding the pointer to one of
  two fixed rows. Offset zero and offset three are unsigned Series00 base and
  slope bytes; offsets one and four are big-endian words; offset seven is the
  signed mode-one Series01 adjustment byte. Mode four omits RAM:00f06a and
  RAM:00ef78 and does not clamp either series. Other modes clamp Series00 to
  0..126 and Series01 to 0..24000; mode one additionally adds the signed,
  truncating binary64 result `profile[7] * 0.1 * index`. Modes 2/3/6 set
  RAM:00ef72 bit six, modes 1/4 clear it, and other modes preserve it.

- Firmware 1.04 RAM:f95af2 is a no-input near-RET packet-polling loop. It
  clears RAM:00ef75, calls extended RAM:fb8e03 once per outer attempt, then
  calls extended RAM:fb8e30 repeatedly without rearming while ACC is 0xfe. A
  nonzero RAM:00fb94 is copied to RAM:00ef7e and selects status four. Otherwise
  current position equal to target minus one sets RAM:001039 bits zero and
  three; signed current position below wrapping target plus fifteen restarts
  the outer attempt, while current at or above that bound selects status one.

- Firmware 1.04 RAM:f95ce3 is a no-input near-RET Series00/01 evaluator. It
  scans each fifteen-word series for equality with entry zero, calls near-RET
  RAM:f96141 to set the regression interval to indices two through fourteen,
  and smooths signed Series06 indices one through thirteen from an unchanged
  snapshot. Nonconstant Series00 and Series01 passes accumulate unsigned
  low-32-bit sum-X-squared, sum-X, sum-Y and sum-XY against Series06, with
  fresh source reads, then call RAM:f95fbf. A first state-two solver result
  exits. Predictions use signed profile byte offset eight as
  `(profile-coefficient_e2)/coefficient_de`; RAM:00ef85 is capped at 126 only
  by a signed comparison, while RAM:00ef87 is capped unsigned at wrapping
  RAM:00f074+0x1100. A constant Series00 publishes its first word and state
  one; a constant Series01 publishes its first word. The nonconstant Series01
  path deliberately does not check solver state before predicting, then calls
  the mode-two classifier and Series00-offset selector.

- Firmware 1.04 RAM:f95fbf is a no-input near-RET regression solver. It
  snapshots sum-X-squared at RAM:0012e6, sum-X at RAM:0012ea twice, sum-Y at
  RAM:0012ee, sum-XY at RAM:0012f2 and the byte count at RAM:0012f6. The
  degeneracy test compares the low 32 bits of `second_sum_x*first_sum_x` with
  `count*sum_x_squared`; equality sets RAM:00ef80 to two without changing the
  coefficients. Otherwise, with each conversion and operation narrowed to
  float32, RAM:0012e2 receives `((sum_y*sum_x_squared)/second_sum_x-sum_xy) /
  ((count*sum_x_squared)/second_sum_x-first_sum_x)`, and RAM:0012de receives
  `(sum_xy-first_sum_x*coefficient_e2)/sum_x_squared`.

- Firmware 1.04 RAM:f9614e is the no-input near-RET mode-two twin of
  RAM:f96ccb. Its 1,499-byte canonical body differs from the equal-length
  mode-six body only in the low-three-bit mode literal and relocated local
  branch destinations. It therefore shares the unsigned Series01 interval
  classification and every special/ordinary-media state-three/state-four
  adjustment formula, publication and completion-bit behavior; only the
  required selector is two rather than six.

- Firmware 1.04 RAM:f9678f is a no-input near-RET wrapper around the same
  fifteen-element Series04-to-Series05 transform used by RAM:f96a65. Before
  transforming it treats Series04 words as signed: entries one above 150 and
  four above 200 select state four; otherwise entry thirteen below 200 selects
  state three. State four updates only when signed RAM:00ef77 is nonnegative,
  subtracting four from it and adding 100 to RAM:00ef78. State three updates
  only when signed RAM:00ef77 is below one, adding four and subtracting 100.
  All byte arithmetic wraps. A passing profile uses the shared calibrated,
  saturated and snapshot-smoothed transform, then calls ordinary near-RET
  RAM:f9745b instead of the mode-six derivation.

- Firmware 1.04 RAM:f96a65 is a no-input near-RET mode-six Series04-to-Series05
  transformer. For every index it calls RAM:f972a6, constructs a coordinate as
  `float((Series00-f06a) * (((Series01-f074)*0.0089+14.3)*0.01) +
  (Series01-f074)*0.001 - 12)` with unsigned wrapping deltas and a fresh second
  Series01 read, then computes `s32(gradient*coordinate/signed Series04*1000)`.
  A zero Series04 denominator yields 0x7fff; other results saturate to signed
  16-bit. It snapshots all fifteen raw Series05 words before replacing indices
  one through thirteen with signed three-sample averages, then writes endpoint
  sentinels 0x7fff and 0x8000 and calls near-RET RAM:f97a1b.

- Firmware 1.04 RAM:f96ccb is a no-input near-RET classifier for mode six. It
  compares RAM:00ef87 as unsigned against Series01 entries two and twelve,
  publishing state three below, state four above and state one within. Only
  low-three-bit mode six adjusts out-of-range states. Media states two/three
  increment RAM:00f08b, adjust RAM:00f089 by -20 for state three or +20 for
  state four only when the new count is below two, and scale RAM:00f06e by the
  signed word times 0.001; other media scale by the signed profile word at
  offset four times 0.001 minus 0.03 for state three or plus 0.04 for state
  four. The adjusted float is narrowed after adding 4.6592, divided by 0.0043,
  rounded upward and added wrapping to RAM:00f074 for RAM:00f117. RAM:00f115
  is the zero-extended low byte of the upward-rounded coordinate reconstructed
  with constants 0.001, 12, 0.0089, 14.3 and 0.01 plus signed RAM:00f06a;
  special media uses RAM:00f06e plus the counter for state three and minus
  three times the counter for state four. Both paths set RAM:00ef72
  bit five.

- Firmware 1.04 RAM:f972a6 takes the Series04 index in ACC, returns float32 in
  DR4, and uses near RET. It constructs a shared float32 scale from the
  wrapping unsigned difference `RAM:00efa7-RAM:00f074` through the binary64
  expression `(delta*0.0089+14.3)*0.01`, then multiplies it by the signed byte
  at offset three of the profile addressed by RAM:00f1bb. Index zero uses
  `-series[2]/2 + 2*series[1] - 3*series[0]/2`; index fourteen uses
  `3*series[14]/2 - 2*series[13] + series[12]/2`; interior indices use
  `-series[index-1]/2 + series[index+1]/2`. Each Series04 word is signed, the
  native float32 operation order is significant, and the result is the
  finite difference divided by the shared scale.

- Firmware 1.04 RAM:f9745b is a no-input near-RET Series05 evaluator whose
  first call is RAM:f97d5a; unmatched RAM:00fb82 values therefore preserve the
  caller's entry WR6 as the comparison threshold. It scans signed Series05
  from index 13 down, validates the interval, and transforms each endpoint as
  `float((Series00-f06a) * ((Series01-f074)*0.0089+14.3)*0.01 +
  (Series01-f074)*0.001 - 12)`, with both subtractions wrapping to u16 before
  binary64 conversion. The Series05 difference divided by the transformed
  difference yields the float32 slope at RAM:0012de; the intercept is stored
  at RAM:0012e2 and the selected threshold is projected back to a position.
  Modes two/three may average that position with one tenth of the indexed word
  at RAM:f83070. The scale selected by RAM:f97e2a produces RAM:00f060/f062,
  a signed profile-word product narrowed through binary64 times 0.001 produces
  RAM:00f064, and the scaled float is stored at RAM:00f06e. RAM:00ef87 is the
  wrapping base plus `ceil((adjusted+4.6592)/0.0043)`, capped at base+0x1100;
  mode zero recomputes without that cap at scale 105 when the first result is
  above RAM:00f072-0x23. RAM:f97c26 then derives the bounded RAM:00ef85 code.

- Firmware 1.04 RAM:f97a1b is a no-input near-RET mode-six Series05 evaluator.
  It scans signed Series05 from index 13 down against the profile-selected
  threshold, validates the selected index, then accumulates two separate
  regressions over fixed indices 2..13. The first uses Series00 as independent
  values and the second uses Series01; both use Series05 as dependent values
  and publish sum-x-squared, sum-x, sum-y and sum-x-y at RAM:0012e6,
  RAM:0012ea, RAM:0012ee and RAM:0012f2 with count 12 at RAM:0012f6. The first
  solver's state-two result exits immediately. Otherwise each prediction is
  `(threshold - intercept@RAM:0012e2) / slope@RAM:0012de`; RAM:00ef85 is
  unsigned-capped at 126, RAM:00ef87 at wrapping RAM:00f074+0x1100, and
  success state one invokes RAM:f95cb4. The threshold, ordering and post-update
  C helpers at RAM:f97dad, RAM:f97f45 and RAM:f95cb4 are near-RET functions.

- Firmware 1.04 RAM:f98099 is a no-input near-RET Series02 evaluator. It first
  snapshots all fifteen signed big-endian words, then rewrites indices 1..13
  with saturated three-sample averages computed from the unchanged snapshot.
  Its scan compares the rewritten words as unsigned values against
  RAM:00f9a0, using a mode-selected byte threshold. A first-index crossing
  publishes low retry state four and subtracts four or eight from RAM:00ef77;
  no crossing publishes high retry state three and adds four or eight; an
  interior crossing validates index-minus-one and otherwise publishes state
  two. Success selects RAM:00ef89[index], publishes it at RAM:00f06b, stores
  its low-byte difference from the mode baseline in RAM:00f06d/f06a, and
  clears RAM:00ef78/ef77. C251 needs no inherited R13 or WR14 adapter.

- Firmware 1.04 RAM:f98b03 is a no-input near-RET READ SUB-CHANNEL command
  coordinator. The accepted formats are 1,2,3,F0,F1. Formats 1/2/F0 require
  track zero; 3/F1 require a nonzero track within cached or effective bounds.
  CDB byte two bit six gates SubQ payload generation. Format one emits the
  twelve-byte current-position payload, two the twenty-byte MCN payload, F0 an
  eight-byte vendor status, and 3/F1 the twenty-byte identification payload.
  The four-byte command header always precedes the payload length, and the
  final transfer is clamped to the allocation length before DATA IN.

- Firmware 1.04 RAM:f98eea takes a low-16-wrapping far destination in DR0 and
  returns through near RET. When BITS:000e is clear or RAM:001292 is at most
  eight, media states two/three run bounded cooperative acquisition waits and
  require RAM:001303 bit two; otherwise the consumer descriptor's word at
  offset eight selects `RAM:00156e + 16*slot`, whose ten controller bytes are
  read atomically under the external-interrupt bracket, with bytes one through
  nine converted from packed BCD. Output source priority is the completed
  capture at RAM:00faf2, the lead-out packet at RAM:00f578, then current Q at
  RAM:001245. MSF output copies absolute/relative tuples; LBA output subtracts
  150 from absolute and negates relative when index is zero. Addressless near
  adapters preserve DR12/DR0; a C251 `sbit` helper exposes BITS:000e as a
  normalized value.

- Firmware 1.04 RAM:f9943d takes an unsigned track in ACC and returns through
  near RET. Cached-TOC state derives start/end from six-byte records at
  RAM:00f2b6+6*track; working-track state loads RAM:00dd3d and reloads the
  original track after probing the next. Types five, six and seven suppress
  the selected-end query on the last track. The acquisition target is start
  plus 75 and the initial capture end is target plus 220; an end above 150 can
  extend that end by 100 or 200 before disc/track clamping. The function clears
  then preserves/restores ten bytes at RAM:000f51e across acquisition and
  finishes by clearing capture-option bits zero through three. The direct
  BITS:000e clear is represented by a C251 `sbit` helper.

- Firmware 1.04 RAM:f99794 takes response format in ACC and a low-16-wrapping
  far destination in DR0, returning through near RET. With RAM:001259 bit two
  clear it writes sixteen zero bytes. Otherwise byte zero is 0x80; bytes one
  through five are successive six-bit fields at shifts 26,20,14,8,2 from
  RAM:000f51f..f522 plus 0x30, and bytes six/seven are the two nibbles of
  RAM:000f523 plus 0x30. Standard format adds five similarly biased nibbles
  from RAM:000f524, f525 and the high nibble of f526. Vendor format 0xf1
  instead emits shifts 20,12 and 4 of the 24-bit f524..f526 value, then pads.
  Ordinary C251 assigns the second C argument to DR4, so the reconstruction
  retains an addressless DR0-to-DR4 near adapter that also preserves DR12.

- Firmware 1.04 RAM:f99acf is a no-input near-RET READ TOC format-zero
  handler. Nonzero RAM:000c9e emits one lead-out descriptor for start 0xaa or
  fixed track-one plus lead-out descriptors otherwise; RAM:000c88 bit seven
  and CDB MSF bit select the fixed address constants, live high-frame MSF, or
  published capacity. Cached state normalizes start zero to the first track,
  can force starts at or below two to track two under summary bit three, and
  streams six-byte track records from RAM:00f2b6+6*track as eight-byte MSF or
  LBA descriptors. The staging buffer flushes a fixed 160 bytes whenever the
  current track number is divisible by 20, not simply after every twenty
  emitted descriptors. The canonical function ends in RET.

- Firmware 1.04 RAM:f99f71 is a no-input near-RET READ TOC format-one
  multisession handler. Nonzero RAM:000c9e emits the fixed active-media
  descriptor with an MSF address of 00:02:00 or 00:16:3c according to
  RAM:000c88 bit seven. Cached state scans one-based eleven-byte Full TOC
  records for the last usable session, reducing the advertised session on a
  discontinuity, then selects the descriptor three entries later. Summary
  bit three with descriptor four emits the fixed 00/14/01/00 descriptor;
  otherwise the control/ADR nibbles are swapped and the address is copied as
  MSF or converted to LBA minus 150. The native tuple-copy site explicitly
  supplies DR0, DR4 and ACC; it does not consume inherited R9.

- Firmware 1.04 RAM:f9a1fa takes the legacy packed-BCD selector in ACC and
  returns through near RET. It regenerates Type-2 full-TOC state when
  RAM:000c9e is two or when the session count is zero during another nonzero
  state, selects CDB session zero as session one, and streams eleven-byte
  descriptors beginning at the first matching one-based table entry. The
  optional conversion matrix is keyed by the original control/ADR low nibble
  and point byte: ADR-one points below A0 convert bytes 3..6 and 8..10 while
  clearing byte seven; A0/A1/A2 use narrower masks. Other ADRs distinguish
  points below B0, B0 (including the byte-four 0xff sentinel), B1..B4 and C0.
  The native copy site supplies all DR0/DR4/ACC arguments explicitly and does
  not consume inherited R9.

- Firmware 1.04 RAM:f9ab19 is a no-input near-RET READ TOC format-three PMA
  handler. CDB byte six rejects nonzero values; allocation zero publishes
  no-sense without a transfer. Media state or last-track zero returns the
  four-byte `00 02 00 00` response. Otherwise it counts and stages one-based
  ADR-3 records at RAM:2bb423+10*i, ADR-4 at RAM:2bb4ff+10*i, ADR-5 at
  RAM:2bb5e5+10*i, ADR-6 at RAM:2bb77f+10*i and retained ADR-1 track records
  at RAM:2bb03b+10*i. The optional fixed ADR-2 record begins at RAM:2bb5db;
  only its three tuple-one bytes are converted from binary to packed BCD.
  Every descriptor is eleven bytes, and RAM:00155a supplies the rolling
  controller-buffer offset.

- Firmware 1.04 RAM:f9ba19 takes request type in ACC, starting feature in
  WR6 and initial response length in WR4, returning the final length in WR6
  through near RET. Request type zero appends every implemented feature at or
  above the start, type one compacts only descriptors whose runtime-patched
  current bit is set, and type two accepts only an exact implemented feature;
  it does not advance to the next feature across a numeric gap. Twenty-two
  templates come from RAM:ff90fe and RAM:ff9102..ff91a9. Feature 0x0024 is
  present only with a nonzero current profile and either zero RAM:000c9e or
  RAM:000c88 bit seven clear. Feature 0x0020 publishes RAM:000c9a capacity
  geometry when profile ten is active, while feature 0x0108 advances only
  when near-RET RAM:f9b9e0 appends its eight-byte serial payload.

- Firmware 1.04 RAM:f9b178 is a no-input near-RET READ TOC format-four ATIP
  handler. CDB byte six must be zero. Media state zero, or the conjunction of
  nonzero RAM:00dd79 and RAM:000c88 bit one, emits `00 02 00 00`; otherwise it
  builds the 28-byte response at RAM:000b2a. Optional groups are enabled by
  RAM:00fb7e bits 2/1/0 or RAM:00fb5b bits 4/3/2. Packed bytes intentionally
  reread RAM:00fb85, 86, 87, 89 and 8b: low contribution first except for
  RAM:00fb87, whose high contribution is read first. All four helpers are
  ECALL/ERET boundaries while the handler returns with RET.

- Firmware 1.04 RAM:f9b78a is a no-input near-RET GET EVENT STATUS
  NOTIFICATION handler. It examines requested class bits in priority order
  02,04,08,10,40, accumulating four response bytes for every inspected
  requested class until a descriptor with nonzero top byte is found; an exact
  one-class request selects even a zero descriptor. If all requested
  descriptors are zero, response byte two is the first requested supported
  class bit; if no supported class was requested it is 0x80 and the response
  is four bytes. Its RAM:f9b45c, f9b4e7, f9b622 and f9b6e0 descriptor helpers
  are LCALL/RET boundaries, while RAM:f9b644 remains ECALL/ERET. The f9b4e7
  call receives the instruction-proven inherited-event seed 0x10.

- Firmware 1.04 RAM:fe30b5 takes a controller-buffer base in DR4 and returns
  through near RET. It copies 0x60 template bytes from RAM:ff9269 through the
  +1-MiB alias, then overwrites offsets 0x40..0x4f from RAM:00dcd5, so the
  total RID01 extent is 96 bytes rather than 112. Configuration offset 0x1a
  supplies five ASCII-hex characters. The initial patch is
  `[n0, n1<<4|n2, n3<<4|n4]`; if the byte read from configuration offset 0x41
  is zero, it becomes `[ascii0>>2, ascii0<<6|n1<<4|n2, n3<<4|n4]`. The three
  bytes are copied to controller offsets 0x18..0x1a. Its sole nibble decoder
  RAM:fe3243 accepts decimal, uppercase and lowercase hexadecimal and returns
  zero otherwise through native near RET.

- Firmware 1.04 RAM:fcd0b8 is a no-input ACC-Boolean ERET coordinator for an
  eligible first track whose RAM:00dd93 control bits equal 0x50 and whose
  RAM:00dd9c bit zero is set. It brackets the RAM:00dd94 MSF conversion with
  RAM:00102f one then zero, derives a 30-frame endpoint, and probes the
  wrapping differences -30, -25, -20 and -15. Status 0x0f initializes ring,
  descriptor, MIP and runtime state; 0xfe first resets and is accepted;
  status five fails; four terminal 0xff results enter the fixed-offset TDI
  fallback. Accepted media states one and three probe TDI with selector zero,
  then order locate-before-scan when RAM:000c9f is zero and scan-before-locate
  otherwise. Every failed follow-up restores detection state. RAM:fcd225 is a
  near RET reset helper despite its earlier unqualified C declaration.

- Firmware 1.04 RAM:f8c004 takes a signed 32-bit transition value in DR4 and
  returns through ERET. An unchanged operating code skips the transition
  work but still republishes RAM:000e4d, copies RAM:000e4c to RAM:000e4e,
  clears RAM:001306 bit four and synchronizes low-code state. A changed code
  replaces nonpositive input with 150, stores it at RAM:00125e, selects a
  countdown of 80 when both old/new codes are below eight, 500 for other new
  codes below 16, and 1000 otherwise, then waits with arguments one/zero and
  clears RAM:00125a bit zero. For enabled codes at least 24 it emits CA/CB
  setup `08/00, 04/01, 00`, then after the wait emits `08/table[2*i],
  04/table[2*i+1], 00`, where `i` is the code's profile index minus seven and
  the byte-pair table begins at RAM:ff84fe.

- Firmware 1.04 RAM:fa90de takes a Boolean layout selector in ACC and returns
  through ERET. It always clears RAM:8012 through RAM:804d, then emits POINT
  A0, A1, A2, 01, B0, C0 and C1. Selector zero uses seven contiguous ten-byte
  raw-Q records at RAM:8012 and converts A2/B0/C0 tuples to packed BCD;
  nonzero uses Full TOC descriptors one through seven at RAM:2b800b, prefixes
  each with session one and retains binary tuples. It publishes big-endian
  values 3, 6 and 7 at RAM:8008, RAM:800a and RAM:800c respectively.

- Firmware 1.04 RAM:fa9a16 is a no-input ERET builder for the exact 80-byte
  MIP controller-record snapshot at RAM:2ba5df. It emits `MIP`, version one,
  the full RAM:2ba525 cursor, low 24 bits of RAM:2ba529, a mode-dependent
  0x40/0x80 flag with mismatch bit 0x10, packed low-20-bit positions, the
  geometry count, and eight packed entries from each table at RAM:2ba537 and
  RAM:2ba557. Every emitted portion rereads its source dword. When RAM:2ba5a3
  is nonzero, RAM:2ba532 becomes the signed quotient of the wrapping value
  `(source + 0xa7) << 5` by 39 through DR4/DR0; zero clears RAM:2ba532.

- Firmware 1.04 RAM:faa502 takes bank index in ACC and page index in R7 and
  returns through ERET. It clears RAM:2ba62f through RAM:2bae2e, emits the MDT
  header, and derives `bank * 8 + page` in two widths: the conditional header
  compares the full 16-bit value with RAM:2ba588, while the table/record gate
  truncates it to a byte. Valid pages read dwords at
  `RAM:f84590 + byte_index * 4`, use bytes one and two for summaries, and emit
  256 six-byte records `20 00 00 byte1 byte2 00`; invalid pages set six
  summary bytes to 0xff. The byte truncation can make a wrapped page valid
  even when its full-width header gate is false.

- Firmware 1.04 RAM:faae3b is an ACC-input, no-result ERET clone of the
  raw-read preparation segment policy, but it is not behaviorally identical
  to RAM:fe1a7b. Its initial dispatch maps plan codes 7, 9 and 10 to the
  big-endian audio-pause count at RAM:000c4f with mode one, and plan 11 to
  five frames with mode two. Its type-two working-track distance refinement
  retains the shared groups: plans 2/3/4/5/6/8 select mode two, plans 7/9/10
  select mode one, and other plans preserve the second mode. The RAM:000c48
  at-most-four override comparison is signed, so 0x80 selects mode three.

- Firmware 1.04 RAM:fab049 is a no-input ACC-Boolean ERET coordinator for
  write-controller buffer ranges. It stores the low byte of the range-builder
  result at RAM:2ba5cc, enters the loop while start is below that fresh count
  or RAM:2ba5cf equals one, and calls RAM:fab26e with ACC=start/R7=count.
  Each range preserves the dword at RAM:000b02 across working-track setup and
  acquisition, publishes RAM:2ba5bc to RAM:001598, arms RAM:2ba5c8 and
  RAM:2ba5ca only around the write-pipeline wait, then restarts the raw-read
  prefix. Both restart waits report failure code one and return false when
  RAM:00fb9f equals two.

- Firmware 1.04 RAM:fab693 is a no-input ERET read-acquisition controller
  configurator. Its operating code is RAM:000e4f, its speed selector is
  RAM:000e47, and its raw-Q type is RAM:001285. It publishes FCA shadow
  transitions `| 0x08`, `& 0xe7`, `| 0x08`, then later `& 0xe7`; after fixed
  port setup it publishes FAD `& 0xdf`, clears BITS:0043, and publishes FAD
  `& 0xef`. Those intermediate controller writes are observable behavior and
  must not be collapsed to their final shadow values. It finishes by setting
  BITS:0547, BITS:0546, and SFR-A9 bit two in that order.

- Firmware 1.04 RAM:fb034e is a no-input ACC-result ERET state-ten drive
  controller; saved DR12 and the native frame are private locals. Its second
  transition offset is the truncated float32 result of unsigned word 0x9000
  times 13 divided by 49152 (the preceding times-12 value is overwritten),
  giving nine. When BITS:003b starts set, countdowns at most 87 repeatedly
  submit 0x9000 until the at-most-81 route submits 0xc800 and latches the
  transition complete. Ordinary settling submits 0xda00. Results are zero for
  expired countdown, one for input completion, and two for abort/failure.

- Firmware 1.04 RAM:fb0672 is a no-input near-RET startup status router; its
  saved R15 is dead local bookkeeping rather than inherited input. Pending
  values 4, 8 and 7 map to status codes 0x25, 0x26 and 0x49; other values map
  to 0x22 with exact 0x23/0x24 overrides from RAM:001008, RAM:00f9b5,
  ConfigurationWordFAAC bit two and RAM:00103a bit four. ConfigurationWordFAAC
  bit eight suppresses maintenance. Active startup states 0x13, 0x1f and six
  select coroutine-zero entry RAM:feb6bb, while state 0x0f selects RAM:fb3ed6.

- Firmware 1.04 RAM:fc6e53 is an ACC-force/DR4-signed-start/DR0-count,
  WR6-result ERET controller-record bank scanner. C251 therefore needs the C
  parameter order `(u8 force, s32 start, u32 count)` to preserve that native
  tuple. It scans 64 banks, admits ordinary, special alternate-offset or
  forced record ranges, and publishes paired expanded extrema at
  RAM:2b9700/04 and RAM:2b9708/0c. RAM:00fc42 counts only the initial accepted
  normal-record continuity run: adjacent endpoint pairs may advance by +1,
  or by +8 when both previous expanded positions minus 150 are divisible by
  39; the first discontinuity prevents later growth.

- Firmware 1.04 RAM:f93b88 is a no-input near RET raw-read position tracker;
  saved WR12/R15 are local scratch rather than inherited semantic inputs. An
  ADR-1 packet is recognized from the low nibble of the first of ten fresh
  RAM:018026 reads, but its MSF comes from reads eight through ten. The valid
  inferred-position window is strictly above `target + format_offset - 150`
  and at or below `target + format_offset`, where any nonzero format index
  selects offset -1. Media state two arms capture only at the exact upper
  bound; other media states enqueue status 0x0200/mode three inside the window.

- Firmware 1.04 RAM:f937fa is a no-input near RET raw-read position validator.
  It accepts decoded deltas from -30 through +30 inclusive, tests the old
  signed retry byte before treating the decremented value as expired, and
  compares the old secondary count as a zero-extended word against signed
  `(RAM:000c3d - 2)`. Mode two emits a zero-status terminal descriptor when
  the signed target reaches RAM:000f36; mode one sets BITS:0009 only after
  ordinary enqueue when the big-endian queued count exceeds 30.

- Firmware 1.04 RAM:facc3b is a DR4-address/WR2-count/ACC-value ERET
  controller-buffer fill primitive. It writes address bytes low-to-high to
  RAM:018010..018012 and count bytes low-to-high to RAM:01800d..01800e,
  repeating each tuple five times before a final tuple. RAM:000ca0 equal to
  one selects command 0x88; other values select 0x80. RAM:000fad bit six is
  armed before the command and cleared from a fresh read after RAM:0014da
  becomes nonzero.

- Firmware 1.04 RAM:fa6eeb is a no-input near RET read-acquisition
  calibration state machine. It selects modes from RAM:00f08c, clamps the
  positioning target against signed `(RAM:00114f - 0x0b2c)`, derives a
  float32 positioning scale when clamped, permits three recovery attempts,
  changes tolerance restart from one to zero after rejection, and directly
  switches P3.0/P3.1 around the configured measurement capture. Its three C
  callers and the range-start/range-capture helpers all use LCALL/RET.

- Firmware 1.04 RAM:fa6894 is a no-input ERET operating-adjustment publisher.
  Mode one optionally doubles RAM:10b4 with 16-bit wrapping, divides it as a
  signed value by seven, and adds RAM:102a before the EC/ED write. Other modes
  optionally replace RAM:102a with the low word of the 83.5-scale helper. Its
  response tables are RAM:ff91f8/ff9205 for the mapped primary operating code
  and RAM:ff91c4/ff91d1 for the mapped secondary code, selected by a fresh
  BITS:0031 read; it finishes by setting RAM:1039 bit seven.

- Firmware 1.04 RAM:fa6560 is a no-input ERET operating-code-0x14 bound
  calculator. Its first FLOAT64 chain is
  `((signed_delta * 0.0043) - 4.6592)` narrowed to float32 and scaled by the
  signed ROM:f8390f selector word before RAM:f97d05. Its second chain builds
  `((signed_delta * 0.0089) + 14.3) * 0.01`, multiplies that by signed
  `(RAM:ef85 - zero-extended RAM:f06a)`, adds a separately refreshed signed
  delta times 0.001, subtracts 12.0, narrows/scales through the paired
  ROM:f8390d word, and calls RAM:f97c26. It reads the position delta three
  times and RAM:f05f twice; those reads must not be cached across phases.

- Firmware 1.04 RAM:fa619b is a no-input ERET disc-control initializer. It
  clears 99 big-endian words at RAM:f1f0, reads configuration offsets 0x2e
  (one byte) and 0x30 (six bytes), and treats only exact return value one from
  each fresh `IsIndexedDriveStateActive(0x1f)` call as active. For type two,
  its control byte is doubled with byte wrapping, biased away from zero by one,
  then divided by two using the canonical signed ACC/B helper. Timing values
  zero and 0xffff normalize to 0x0b80; RAM:f074 is the wrapping difference
  from 0x0b80.

- Firmware 1.04 RAM:fdc655 is a no-input ERET drive-configuration initializer.
  Configuration-byte zero can enable the special mode only with P0.6/P1.6
  low and one of two P0.1/P0.2 polarities selected by ROM:f7c040. It writes
  RAM:001220=0xfa, but must still preserve fresh signed-positive reads and the
  P1.6 rejection loop because timer-owned state can change asynchronously.
  Configuration values 0x11 and 0x12 force startup enabled; 0x12 additionally
  sets RAM:00130a bit four. The ring mask/high-water fields are the low words
  of the full 32-bit slot count minus one and six, including underflow.

- Firmware 1.04 RAM:fdc43b is a no-input ERET cold-start initializer. Its two
  RAM:ff1f00 runtime probes use fixed `(DR4,WR2)` tuples `(0x20,0x36)` and
  `(0x300,0x540)`; only the second result survives, and a still-set BITS:0032
  after the first 0x60-byte clear forces it false. `FillControllerBufferRange`
  takes its address in DR4 and count in DR0, so the instruction-verified calls
  are address/count `(0x1b0000,0x10000)` and `(0,0x1b0000)`, correcting the
  reversed Ghidra pseudocode. Status seven is unreachable because R14 is set
  to one, and the later status-seven block is unreachable because ACC is set
  to one immediately before JNZ; reachable startup results are 0, 1, 2 and 10.

- Firmware 1.04 RAM:fdb787 is a no-input extended controller ASIC initializer.
  It publishes main/auxiliary/subchannel/metadata bases 0x126000, 0x126100,
  0x14ae00 and 0x156f00, reserved/capacity values 0x160000 and 0x1b0000,
  raw slot count 0x200, safety margin 0x82 and region bases 0xb5000/0xd9c00.
  Every RAM:000fca mask transition is immediately mirrored to port 0x8002;
  several geometry ports are intentionally overwritten by later reset state.
  Its only semantic calls are extended delays 12/9 and controller-register-one
  value one, and it returns through native ERET.

- Firmware 1.04 RAM:fdb4d9 is a no-input near ATA IDENTIFY PACKET DEVICE
  builder. RAM:fdb4df adds 0x10 to WR12, the HIGH word of DR12: the
  controller-to-CPU window translation is +0x100000, not +0x10. Default
  reserved base RAM:160000 therefore produces the payload at RAM:260000. It
  clears 512 bytes with the same bank-preserving low-word increments, performs
  eight copies with counts 20,6,8,4,8,16,4,12, and reads indexed big-endian
  words from ROM tables ff9449, ff9453, ff945d, ff9467, ff9471, ff9479,
  ff9481 and ff9489. RAM:fdb4b6 takes 0xb2 as a word count and is a near
  LCALL/RET helper; it therefore requires `PLEXTOR_NEAR_FUNCTION`.

- Firmware 1.04 RAM:fdb06a is an ordinary no-input extended ATA task-file
  dispatcher. Its recognized command set is exactly 20,90,a1,da,e0,e1,e6,ec,
  ef,fd,fe,ff; all other bytes publish the unsupported-command ATA registers.
  Commands 20, da and ec share one controller-reset completion path. Command
  ff returns literal ACC 0x64, not normalized one, while ordinary paths return
  zero and command fd invokes the restart handoff. RAM:fdb4d9 and RAM:fdb3dc
  are both native LCALL/RET dependencies; in particular
  `ApplyAtaSetFeaturesCommand` requires `PLEXTOR_NEAR_FUNCTION`.

- Firmware 1.04 RAM:fdab34 is an ordinary no-input Boolean near worker. While
  the read-operation busy bit is set, its unconditional opcode allowlist is
  exactly 03,12,46,4a,eb,f5; opcode 52 is additionally allowed only when
  RAM:000c9e is nonzero. In the normal active-operation state, the immediate
  allowlist is exactly 00,03,0a,12,2a,35,46,4a,52,5b,5c,aa,eb,f5,f7,f8;
  08,28,a8,b9,be,d8,d9 finalize only when write type RAM:000c44 is zero; and
  1a,23,25,2e,51,55,5a require nonzero RAM:000c9e. Opcode 1e has its distinct
  tray, write-type and CDB-byte-four gates before using the already-checked
  finalization tail. C251 preserves the ACC Boolean and native RET contract.

- Firmware 1.04 RAM:fda843 is an ordinary no-input Boolean ERET admission
  policy. It compares the four-byte dispatch entry at RAM:f83ef7 plus four
  times the opcode against the unsupported-handler entry stored at RAM:ff9691,
  accepts opcode 0xee before later policy gates, and calls RAM:fdab08 and
  RAM:fdab34 through near LCALL/RET contracts. Its no-media allowlist is
  exactly 00,03,12,1a,1b,1e,35,4a,52,58,5a,5b,5c,f4,f5, with 0xee admitted by
  the earlier exception.

- Firmware 1.04 RAM:fda4eb is an ordinary no-input ERET C worker. It accepts
  the configured vendor only when byte two is `A` through `Z`, accepts the
  product when byte ten is uppercase or when byte ten is a space and byte
  eleven is uppercase, and suppresses serial conversion only when one of
  configuration bytes 26..30 is `0xff`. Its fixed six-position decimal loop
  deliberately writes `:48575` for hexadecimal `FFFFF`; this is observable
  firmware behavior, not a formatting error to repair. RAM:fda6f0
  `ParseUppercaseAsciiHexU32` and RAM:fda732 `PowerOfTenU32` both use native
  LCALL/RET contracts and therefore require `PLEXTOR_NEAR_FUNCTION`.

- Firmware 1.04 RAM:fd66c7 `ReadDriveConfigurationBytes` uses a function-
  specific C251 contract equivalent to
  `(fw_addr_t offset, fw_addr_t destination, u16 count)`: destination arrives
  in DR0, the 24-bit configuration offset in DR4, and count on the stack. Live
  callee instructions and a C251 parameter-order matrix establish this; the
  prior destination-first/u16-offset declaration silently produced a different
  register ABI.

- Firmware 1.04 RAM:fda144 is an ordinary no-input ERET C initializer. Its
  first configuration tuple is offset 0x4c, destination RAM:000b2a, count
  three; its second is offset one, destination RAM:000040, count one. The
  startup protocol logic uses direct P0 strap bits, and the same-address native
  body is unnecessary once those direct-bit operations are isolated behind
  addressless near helpers.

- Firmware 1.04 RAM:feaeca is an ordinary no-input ERET C worker. Its near
  capacity predicate means "continue waiting" only when the result is exactly
  one, whereas the extended acquisition restart treats every nonzero result as
  success. Retry failure begins only when the unsigned counter is strictly
  greater than the selected limit of zero or two.

- Firmware 1.04 RAM:fead99 is a normal near C worker with signed frame count
  in DR4 and capture source kind in ACC. It always captures and populates the
  subheader; source kind four selects forward relative-Q advancement, while
  every other source kind selects reverse advancement.

- Firmware 1.04 RAM:feabbe is a normal near C worker with prefix mode in ACC
  and capture source kind in R7. Canonical callers constrain the mode to 0, 1,
  or 2, selecting frame-index windows `[0,5)`, `[5,7)`, or `[0,1)`; the source
  kind independently controls capture/subheader policy and relative-Q MSF
  direction.

- Firmware 1.04 RAM:feb0d3 takes its signed count in DR4 and its byte selector
  in ACC, despite the current Ghidra decompiler annotation naming R8. The entry
  immediately saves ACC and every one of its eight caller families loads ACC
  immediately before LCALL; Keil C251's ordinary near `(s32, u8)` signature
  reproduces DR4/ACC directly.

- Firmware 1.04 RAM:fd8bc6 is a normal near C-representable worker with its
  unsigned magnitude in WR6; the saved DR12 is compiler workspace, not an
  inherited input. Thresholds 100, 250, 500, 1000 and 1500 select the six
  big-endian pointers at RAM:ff85a8..ff85bf, ordinary four-byte records call
  `SubmitControllerWordPair(value, selector)`, and selector 0xf000 writes port
  0x849e using RAM:001130/RAM:001131 overrides before the RAM:000fae value-three
  extra write.
- Firmware 1.04 RAM:fd89ad is a near C-representable radial-pickup transaction
  with direction in ACC and magnitude in WR6. It composes ports 0x849a..0x849c
  from paired ROM nibbles at RAM:ff8908..ff890d, programs the low magnitude at
  port 0x80ee and its high byte with direction bit seven at 0x80ef, then while
  signed port 0x80f8 remains negative yields before polling trigger port 0x849d;
  a nonnegative status abort leaves the trigger uncleared, while completion
  bit zero causes a zero write to that trigger.
- Firmware 1.04 RAM:fd8846 is a no-input near C-representable restore worker;
  saved R15 is its local wait-policy flag. Its unsigned boundaries are 62 and
  124, it writes value seven (not ten) to RAM:001300, waits 600 or 800 scaled
  units, and only the path that skipped the initial tolerance wait publishes a
  30/60/100 countdown before calling the tolerance worker with both flags one.

- Use `80251-fixed:BE:24:default` release 1.3 with `srcMode=1`; the stock
  preliminary Ghidra 80251 language is not acceptable for this corpus.
- The decompiler stack model deliberately omits hardware return-address pushes,
  following Ghidra's decompiler-oriented MCS-51 approach. Explicit stack
  operations remain real.
- Dynamic `@SPX` accesses, caller stack slots, condition flags, shared tails,
  non-contiguous bodies, coroutine `ERET` paths, and thunks require inspection
  of instructions and callers.
- There is no safe universal firmware ABI. Apply custom register storage only
  to functions whose callers and bodies prove it; leave mixed or composite
  contracts unforced until source adapters can represent them faithfully.
- A callee whose final instruction-producing operation leaves a byte in ACC
  can have a real scalar result even when an earlier standalone reconstruction
  treated it as void. Inspect immediate post-call instructions: RAM:f87057
  stores ACC directly after calling RAM:f8e6d4, proving that the latter returns
  its final controller port-0x80c7 byte.
- A zero-index shared-entry thunk can be reconstructed as a normal C wrapper
  once its shared worker is already represented. If the shared native worker
  takes a non-C register such as R3, keep only an addressless adapter that sets
  that register; if the shared worker is C, no assembly adapter is needed.
- A native register residue that is semantically observable on a real path can
  be made an explicit C parameter instead of forcing a guessed value. Firmware
  1.04 RAM:fc8d77 uses inherited B only for unenumerated upward transitions;
  its C interface exposes that seed in ACC and otherwise preserves the proven
  staged ceilings, overrides, limits and ACC/ERET result.
- A near C body can preserve a single inherited extended byte register with a
  first-operation addressless capture adapter. RAM:f92a69 uses this for signed
  R14: C251 calls the adapter before any body state change, owns the local
  countdown thereafter, emits RET, and its sole C caller retains an LCALL.
- A near C body with ordinary DR4/DR0/ACC formals can also capture inherited
  R15 when C251 first spills those formals to overlay storage. RAM:fe825b then
  forwards the captured byte as the third C parameter, which C251 places in
  R3; its acquisition adapter only needs to set the native R2 flag before the
  canonical near call. Do not assume later byte parameters use R7.
- Firmware 1.04 RAM:fa099c and RAM:fa1227 are ordinary C-representable
  eight-slot coordinators. C251 maps their acquisition-mode/index/signed-frame
  calls to ACC/R7/DR0 and returns through ERET. The primary worker selects
  RAM:2ba537 or RAM:00fb04 sources and bounds retries against RAM:00114f; the
  secondary worker copies RAM:2ba557 sources to RAM:00fb2c and bounds retries
  against signed RAM:2ba595 minus 0x4e0. Rejections mark the corresponding
  five-byte slot and retreat shared state plus the slot/table position by 32.
- Firmware 1.04 RAM:f94dc0 is a no-input near RET coordinator. It treats
  RAM:00ef74 nonzero as already complete; otherwise it targets signed
  RAM:00114f minus 0x47e, permits three acquisition/controller-recovery
  attempts, changes its tolerance restart argument from one to zero after the
  first failure, brackets the near RAM:f94ed0 probe with RAM:001301 bit zero,
  and publishes status one or status four with result seven, five, or the
  acquisition result. C251 emits RAM:f94ed0 as LCALL/RET, not ECALL/ERET.
- Firmware 1.04 RAM:fc858f is a no-input near Boolean coordinator. Its saved
  R15 is compiler preservation and temporary result storage, not an inherited
  semantic input. The positioning helpers at RAM:fc86c7, RAM:fc90e5,
  RAM:fc912a, RAM:fc9169, RAM:fc9194, RAM:fc91b5, RAM:fc9226, RAM:fc9254 and
  RAM:fc93db use near LCALL/RET contracts; declaring them extended silently
  changes both call and return encodings.
- Firmware 1.04 RAM:f93946 is an ERET interrupt-context worker whose controller
  progress sample is low byte RAM:018079 plus high byte RAM:01807a. It wraps
  the delta by the low word of RAM:00156a, publishes descriptor dword zero plus
  three at RAM:000f32 and descriptor word eight at RAM:000f5c, then uses
  descriptor bytes 17 and 16 for port reset and transfer completion. Its
  RAM:000f36 test compares the signed value before decrement. C251 `sbit`
  helpers implement the direct BITS:000e/001a accesses.
- An implausibly large local array is a decoder/stack-model warning, not evidence
  of source-level storage.
- In fixed-language release 1.3, the `MOV WRj,@WRj+dis16` form reverses its
  displayed destination and pointer registers. A251 4.69 proves that bytes
  `49 f3 00 06` and `49 e3 00 04` encode `MOV WR30,@WR6+6` and
  `MOV WR28,@WR6+4`, not the displayed inverse forms. Until the language is
  corrected and revalidated, decode this form from its operand nibbles and
  corroborate the dataflow before translating it.

## Firmware image

- Firmware versions 1.01â€“1.04 are each exactly `0x80000` bytes and map at
  `RAM:f80000`.
- Firmware 1.04 enters at `RAM:f80050`.
- The stored big-endian checksum is at file offset `0x79ffe` and equals
  `(0x8000 + sum(image[0:0x79ffe])) & 0xffff`.
- File offsets `0x7a000..0x7ffff` are erased (`0xff`) in the downloadable
  images. A matching checksum proves container validity, not behavioral safety.
- Preserve original images byte-for-byte; alternative images must be separately
  generated and reproducible.

## Runtime and recovery

- Direct comparison of immutable firmware 1.04 and the `read_memory_ff0000`
  capture shows the download's file offsets `0x70000..0x77fff` (mapped
  `RAM:ff0000..ff7fff`) are all `0xff`, while the capture has 28,717
  non-`0xff` bytes at those addresses. Their `RAM:ff8000..ff9fff` slices
  match byte-for-byte; the download is erased at `RAM:ffa000..ffffff` while
  the capture has 118 non-`0xff` bytes there. This is a file-level mapping
  distinction, not physical chip-select or protection evidence.
- The 64-KiB capture maps at `RAM:ff0000..ffffff` and is a separate build target
  from the downloadable main firmware.
- The ROM vector table routes five interrupts either to monitor handlers or main
  firmware through `MainFirmwareInterruptVectorSelect` at `RAM:0000fc`.
- Three different ROM loaders copy mutually exclusive routines to physical
  `RAM:000800`; model them as separate Ghidra overlays and separate build
  units, never one simultaneous body.
- Reset evaluates forced recovery before main-image validation. Monitor INQUIRY
  revision `0.00` is the tested practical recovery-mode discriminator.
- Vendor F5 mode 0 is a memory-read mechanism in both environments. In
  firmware 1.04, stock `RAM:fcb62b` reads selector CDB[3] at `RAM:000b0d`;
  the selector-zero branch loads a source dword from CDB[4..7] at
  `RAM:fcb730`, a length word from CDB[8..9] at `RAM:fcb73c`, and calls
  the DATA IN worker through `RAM:fcb832`. Its copy worker reads the source
  byte through `@DR0` at `RAM:fddd8e`. This is direct instruction evidence
  for a read-only CPU data-address probe, not evidence of a flash write alias.
  The separate runtime capture's F5 dispatch entry at `RAM:ff7dd4` contains
  `00 ff 2a 3e`, routing to `RAM:ff2a3e`. That handler reads the same wire-CDB
  fields from monitor storage `RAM:000f05`, `RAM:000f06`, and `RAM:000f0a`;
  its DATA IN worker reads source bytes at `RAM:ff32ed` and `RAM:ff32fa`.
  Runtime entry `RAM:ff2000` performs hardware initialization and should not
  be confused with F5 or the relocated flash programmers.
- Directly decompiling the runtime thunk at `RAM:ff3f7d` produces Ghidra's
  `Bad instruction` warning because its valid `EJMP RAM:f80050` target belongs
  to the separately mapped main-firmware program. The runtime program records
  that destination as an external function; the warning does not indicate a
  decoder or instruction-boundary failure.

## C/C++ data modeling

- Firmware 1.04 RAM:fae689 is the near Boolean recordable-media READ TRACK
  INFORMATION builder. It selects TOC source three or working-table source
  five from CDB mode, signed adjusted position, media state and track bounds;
  preserves the special track-0xff live/saved-table rules; and writes the
  complete 28-byte response at RAM:000b2a. Its C entry captures inherited B
  before any firmware read and forwards it only to the two explicit C track
  loaders. TOC responses zero NWA/free blocks, while working-table responses
  use the near NWA/free/size helpers; all multi-byte response fields are
  big-endian and arithmetic subtracting 150 wraps at 32 bits.

- Firmware 1.04 RAM:f8e2ef is a no-input extended SFR sample-series worker.
  It captures 20 D8/D9 pairs only after separate fresh F1 bit-zero and bit-one
  readiness reads, sorts both arrays, and averages sorted indices 6 through
  13 after adding 0x78 to each D8 value and 0x11 to each D9 value. It then
  captures 16 DB samples gated by F1 bit three and publishes their sum shifted
  right four. SFR E1 finishes at one and RXDAT/E3 bit one is restored clear.

- Firmware 1.04 RAM:fdc28c is a no-input near SCSI controller-preparation
  worker. It clears the controller flag only when BITS:0008 is clear,
  RAM:000f46 is zero and RAM:000fae is not three, then busy-polls controller
  port 0x8058 bit two. A pending RAM:00e751 trigger sets both P1 timer pins,
  enables external interrupt zero, writes parameter D6=0 and selects P3.0
  high/P3.1 low unless fresh pipeline-state reads equal one or two. Its
  optional port-0x8041 handshake uses signed RAM:001204 expiry and publishes
  status 0x81 instead of 0x01 at port 0x8050 on timeout.

- Firmware 1.04 RAM:f8d96b is a no-input extended controller transaction. It
  snapshots channel two and words 0x0339/0x035d, clears both words, issues
  command 0x03 after a 2,000-unit settle, and uses the standard 500-tick
  yielding wait. Success reads port 0x80c7 and writes the 16-bit two's
  complement of that byte shifted left eight to word 0x03af; timeout writes
  zero after the unbounded cleanup wait. It then restores word 0x0339, fresh
  channel one, word 0x035d and saved channel two in that order. The existing
  near wait adapters reproduce its inherited DR0/R7/ACC tuples for C callers.

- Firmware 1.04 RAM:fe5cf7 is a no-input near Boolean write-prefix worker. It
  invokes recovery only for exact RAM:00127d value one with zero RAM:00fb9a.
  Write type zero skips fixed-packet mode, compares the big-endian target with
  the working cursor plus seven using 32-bit wrap, rewinds by seven, installs
  track type one and runs the prefix with ACC one/R7 zero. Prefix failure
  deliberately leaves the rewound target and temporary track type installed;
  planning failure additionally leaves RAM:0014b6 set. Only full success
  clears that flag and restores the original track type. Write type two always
  publishes state six/zero and only exact marker one plus zero target clears
  the marker, advances the track/Q record, then refreshes ISRC. RAM:fe2b01 and
  RAM:fe2292 are proven near RET callees and require near C declarations.
- Firmware 1.04 RAM:f8b8c2 takes a reload selector in ACC and returns through
  ERET. Only selector value one enables the changed-code profile reload and
  channel-three update; RAM:001304 bit three independently forces the profile
  reload. It reads CurrentOperatingCode once when below four and twice when at
  least four before asserting P1 bits zero/one, snapshots it once for the
  parameter-profile dispatch, but uses fresh reads for common-word setup,
  applied-code comparison, profile loading and channel-three mapping. It always
  stores 0xff at RAM:00125d and passes a fresh SecondaryOperatingCode read to
  the final near worker. RAM:f8b420 is also a proven near RET helper, so its C
  declaration must be `PLEXTOR_NEAR_FUNCTION` for the canonical LCALL class.
- Firmware 1.04 RAM:fcea44 is a no-input near-RET eight-sample acquisition
  worker. Both its initial ten-sample readiness result and each pulse-window
  result must equal exactly one; an initial failure clears RAM:001305 bits
  three/four and RAM:00f99e, then restores bit three only for parameter state
  two, while an in-loop failure merely restores controller parameters and
  publishes no measurement or flags. Success averages eight unsigned samples,
  uses threshold 0x50 for state two and 0x32 for every other nonzero state,
  sets bit three only below the threshold, stores the low mean byte, restores
  parameters, and sets RAM:001307 bit one plus RAM:001132 bit five. C251 emits
  native RET and does not use inherited DR12 for the replacement body.
- Firmware 1.04 RAM:fb8249 is a no-input near Boolean supplemental-ATIP
  coordinator. It writes countdown ten and clears exact-frame matching, then
  repeatedly publishes AtipLeadInStartFramePosition minus 0x0d2f while the
  freshly read countdown is signed-positive; an acquisition success bypasses
  a subsequent expiry check. After applying the operating code and arming the
  packet capture, each signed-positive iteration compares a fresh countdown
  plus two against the zero-extended initial countdown, accepts marker 0xe0
  only after a difference greater than two, otherwise parses, conditionally
  rearms, yields, then validates mode two. Capability bit one scales the full
  16-bit value `2 * RAM:00fb84 + 2` by 0x1194 and publishes nominal lead-out
  plus adjustment minus 0x1a5e with 32-bit wrap. C251 uses no R15 in the C
  body, so the canonical inherited-R15 save is unnecessary in the replacement.
- Firmware 1.04 RAM:f9c3d0 is a no-input ERET GET CONFIGURATION coordinator.
  It captures the
  big-endian allocation word at RAM:000b11 before modifying the response,
  publishes current profile zero when RAM:000fae is nonzero and otherwise
  maps media states zero/one/two/other to profiles 8/9/10/8, with nonzero
  RAM:00dd79 plus RAM:000c88 bit one forcing profile eight. Its retained near
  builder takes request type in ACC, starting feature in WR6 and initial
  length eight in WR4; the returned length minus four is zero-extended after
  16-bit wrap into RAM:000b2a, while staging uses the full returned length and
  DATA IN uses the unsigned minimum of that length and the captured allocation.
  C251 emits this exact retained-builder tuple and ERET without allocating
  inherited DR12, so no assembly adapter is required.
- Firmware 1.04 RAM:f8e2b0 takes yield mode in ACC, timeout enforcement in R7,
  and a signed timeout in inherited DR0; ACC returns normalized Boolean status
  through ERET. It publishes DR0 big-endian at RAM:0011f8, tests controller
  port RAM:0180f8 bit seven before each loop body, optionally yields for any
  nonzero ACC selector, and only then rejects an enforced timeout when a fresh
  signed RAM:0011f8 read is nonpositive. An addressless DR0-to-DR4 capture plus
  the existing fixed-call adapters are sufficient assembly exceptions.
- Firmware 1.04 RAM:f8e41c is an ordinary no-input ERET calibration worker.
  It tests port 0x8097 candidates in the exact order
  0x18..0x1f,0x00..0x09, runs two command-0x08 readiness transactions per
  candidate, and accepts signed port-0x80c7 feedback at least -97. Timeout or
  exhaustion stores 0xc0 in RAM:00fab7 and writes it to port 0x8097. Its two
  calls to RAM:f8e2b0 are C-representable through addressless near adapters
  for DR0/R7/ACC = 500/1/1 and 0/0/1; no same-address native body is needed.
- Firmware 1.04 RAM:f8cf2a is a no-input ERET calibration coordinator. It
  accepts RAM:fc99d7 only when the result is exactly one, but propagates each
  non-one result from the three fixed 10-tick controller-ready waits. The
  command sequence is 0x50/word 0x0309/port 0x80a0, then 0x60/word
  0x0319/port 0x80a2, then 0x80. RAM:001304 bit five enables reference
  captures of words 0x0309 and 0x0319 at RAM:00fbe2..00fbe5; a fresh word
  0x0319 read always supplies RAM:001056. The fixed wait tuple
  DR0/R7/ACC = 10/1/1 is C-representable through the existing addressless
  near adapter, so no same-address assembly body is needed.
- Firmware 1.04 RAM:fbfe72 receives a packed DR4 descriptor handle: the low
  word remains the descriptor address, while the upper word is the raw-read
  buffer window advanced by 0x10. Its destination upper word is obtained by
  reading the big-endian word at descriptor+2, then the big-endian word at
  that address, and adding 0x10; the original descriptor low word is retained.
  With IE.EA clear it copies twelve bytes, converts ADR-1 bytes one through
  nine from packed BCD unless BITS:0013 is set (with 0xaa exempt only at byte
  one), appends 00/type/ff/e0, writes sixteen bytes, and restores IE and A9.
- Firmware 1.04 RAM:fbf1c4 is a near vendor-F3 cycle worker. A zero maximum
  exits without any controller write; otherwise indices zero through the
  unsigned maximum are inclusive, with an explicit 0xff guard before
  increment. Only index zero and positive multiples of ten enter the long
  selector-three/selector-four drive phase. Its countdowns intentionally use
  both signed-positive and signed-nonnegative loops, and the 120-tick input
  wait checks RAM:0011ed bit two before testing negative timeout.
- Firmware 1.04 RAM:f89c73 is a no-input near Boolean worker. Result 0x1e
  publishes target 150 and state three. Equal requested/observed classes use
  far-ROM words RAM:ff92eb, RAM:ff932d, or RAM:ff9337 according to write speed
  and class; the high-speed class-three route conditionally reduces speed.
  Unequal classes retry below count five when RAM:001304 bit five is clear or
  below count ten when it is set, adjusting the encoded offset each time.
  RAM:f89db9 and RAM:f89fb5 are also near RET helpers; their earlier C ERET
  declarations were incorrect and are now housed with the calibration worker.
- Firmware 1.04 RAM:f8a934 takes the controller-port value in ACC and an
  inherited far port address in DR0, then returns through ERET. A first-step
  addressless `MOV DR4,DR0; ERET` adapter lets C251 capture the hidden address
  before executing the maintainable C body. The exact observable order is IE
  read, A9 read, EA clear, RAM:000fca bit-three set and RAM:018002 publication,
  port write, RAM:000fca bits-three/four clear and second publication, IE
  restore, then A9 restore.
- Firmware 1.04 RAM:febb0c is a no-input near Boolean coordinator. It chooses
  the big-endian RAM:001153 endpoint plus one only when RAM:00fbf1 is zero,
  RAM:001008 is one, and RAM:00130a is four; every other combination subtracts
  two with 32-bit wrap. Any zero result from range preparation, prefix
  preparation, or raw-Q queue construction exits immediately, while any
  nonzero result continues and final success is normalized to one. After the
  prefix and before Q construction it clears BITS:002f..0035 in order; after
  Q success it ORs 0x0040 into the complete big-endian word at RAM:00fad7,
  not merely one byte. RAM:febe9f returns with RET and is called by LCALL, so
  its C declaration must be `PLEXTOR_NEAR_FUNCTION`.
- Firmware 1.04 RAM:fbb2c5 is a no-input near function returning status zero
  or two in ACC. Its apparent inherited B/R3 reset inputs are decompiler
  artifacts: RAM:fba4f7 and RAM:fb8be5 are zero-index wrappers. The routine
  saves and restores RAM:00fc0f around the media-state-two parameter setup,
  rereads RAM:001008 and RAM:001305 at each native gate, copies RAM:000cf5 and
  RAM:000cf9 into retained post-TOC fields, stores the low words of two
  logarithmic results, and maps any nonzero final initialization result to
  zero. RAM:fba4f7, RAM:fbb396, and RAM:fbb3ed are all near RET boundaries;
  their C declarations must make callers emit LCALL.

- Firmware 1.04 RAM:feb1ed consumes controller-buffer destination in DR4 and
  raw-sector source kind in ACC, returns true in ACC through ERET, and calls
  near RAM:feb355 with transfer-length output in DR0 and capture-mode output in
  DR4. It clears slot five, writes 0xe0/mode at RAM:018020/018021, serializes
  destination low byte first at RAM:018010..018012, sets/mirrors RAM:000fad bit
  six, issues command 0x80 at RAM:018022, waits for RAM:0014da, then reverses
  the bit and slot state. Two C forwarding wrappers use DR8/R3, so an
  addressless bridge moves those into DR4/ACC before the canonical C entry.
- Firmware 1.04 RAM:feb355 consumes transfer-length output in DR0,
  capture-mode output in DR4, and raw-sector source kind in ACC. C251 assigns
  the first generic-pointer source parameter to DR0 and the second to DR4, so
  the maintainable declaration intentionally orders its semantic outputs as
  `(u16 *transfer_length, u8 *capture_mode, u8 raw_sector_source_kind)`.
  Reversing those two source parameters silently reverses the native pointer
  registers even though both types are generic pointers.
- Firmware 1.04 RAM:fe3b86 is a no-input ERET coordinator. Failed calls to
  RAM:feb0d3 or RAM:feabbe return without clearing RAM:0014b3, whereas every
  non-acquisition route and every successful route clears that marker. Its
  canonical calls to RAM:fe1a16, RAM:fe02f5, and RAM:fe0398 are LCALLs to RET
  workers; their C251 declarations must therefore remain explicitly near.
- Firmware 1.04 RAM:feeb12 and RAM:feeb80 both take signed frame position in
  DR4 before the output address in DR0; source declarations therefore use
  `(s32 frame_position, fw_addr_t output_tuple_address)`. RAM:feeb12 truncates
  destination arithmetic to the low 16 bits, while RAM:feeb80 retains the full
  24-bit bank across increments. Both add 450000 once for negative inputs and
  return via ERET.
- Firmware 1.04 RAM:f9ecad consumes a float32 profile coordinate in DR4 and
  returns a word in WR6 through near RET. With unsigned words `scale` from
  RAM:00f9fa and `factor` from RAM:000ffe, its nonzero path evaluates float32
  `base = ((float)scale / 100 * (float)factor) / 32`, separately rounds
  `base * 16` and `base * profile`, subtracts the former from the latter,
  truncates through signed 32-bit conversion, and returns the low word.
- Firmware 1.04 RAM:faab68 consumes controller-buffer addresses in DR4 and
  DR0 plus a 32-bit controller word count on the native stack, and returns
  through ERET. It shares RAM:faaa8c's two low-byte-first 24-bit address
  serializers, shadow update, completion clear, command-0x80 trigger and busy
  wait, but writes the supplied word count directly rather than multiplying a
  sector count by 0x498.
- Firmware 1.04 RAM:faaa8c consumes controller-buffer addresses in DR4 and
  DR0 plus sector count in ACC, and returns through ERET. It serializes each
  24-bit address low byte first at RAM:018070..018072 under selectors zero and
  one, programs the low three bytes of `sector_count * 0x498` at
  RAM:018073..018075, clears RAM:2ba5c0, issues command 0x80 at RAM:01806f,
  and busy-polls the completion byte.
- Firmware 1.04 RAM:fb9624 consumes its wide-window selector in ACC and
  returns Boolean through ERET. With BITS:0031 clear it accepts iff controller
  port RAM:018032 bit five is clear. Otherwise it atomically reads the signed
  big-endian word at RAM:018072/018071, computes float32 14794.92 divided by
  a fresh unsigned RAM:000e51, and uses strict `-threshold < sample < threshold`
  bounds. Wide mode uses 1764 at values <=16 or trunc(base*3); every selector
  other than exact one uses 58 at values <=16 or trunc(base*0.1).
- Firmware 1.04 RAM:fea188 and RAM:fea27d are distinct near entries for the
  ISRC and media-catalog restoration call sites but contain the same 245-byte
  algorithm. Each consumes its relative-MSF decrement flag in ACC
  and returns constant one in ACC through near `RET`. If RAM:0014ab equals
  one, it copies each byte of RAM:0088aa..0088b3 first to the current packet
  and then to the saved packet before clearing the latch. Otherwise it restores
  RAM:001491..00149a, applies RAM:0014aa to the index byte only for write type
  two, and calls RAM:fe9ecc with the original flag.
- Firmware 1.04 RAM:fea372 is a near `RET` function whose skip flag enters in
  ACC and whose relative-MSF restore-decrement flag enters in R7. With C251,
  the matching C declaration orders the ACC byte first and the R7 byte second.
  It always updates ordinary Q control, snapshots RAM:001272..00127b only when
  both identification-active bytes are zero, and then independently restores
  or builds ADR-3 ISRC and ADR-2 media-catalog packets while honoring fresh
  RAM:001284 gates and clearing RAM:0014ab only when it equals one.
- Firmware 1.04 RAM:facae1 is a near `RET` function with a far source in DR4
  and far destination in DR0. It copies raw TOC payload bytes zero and two,
  forces destination byte one to zero, and converts packed-BCD offsets 3
  through 9 via RAM:fee831. Source and destination offset arithmetic wraps
  only the low sixteen bits and retains each pointer's bank.
- Firmware 1.04 RAM:fabc8e is a near `RET` function with a far source in DR4
  and far destination in DR0. It copies raw Q-record byte zero unchanged and
  converts packed-BCD offsets 2,3,4,5,7,8,9 through RAM:fee831, preserving
  each pointer's bank while offset arithmetic wraps only the low sixteen bits;
  destination bytes one and six are untouched.
- Firmware 1.04 RAM:fe2b6f is a no-input near Boolean `RET` function. A
  nonzero RAM:001596 always blocks; nonzero RAM:000c9e blocks at queued count
  195 or higher. The ordinary path can restart acquisition at a wrapping
  high-water-minus-safety-minus-reserve threshold, floors the capacity reserve
  at three, and separately checks fixed-packet overlap and remaining capacity
  against the active host-transfer block count. Blocking paths set RAM:0088b4
  but nonblocking paths do not clear it.
- Firmware 1.04 RAM:fa7bd7 is a no-input near `RET` function. It rereads
  CurrentOperatingCode on the non-four path and waits either 250 units or
  floor(1000/code), then shares the ten-sample D8/D9 capture/sort pattern.
  Its offset middle-four averages form signed 16-bit terms `D8-refD8` and
  `refD9-D9`; their difference is sign-extended, multiplied by 1000, divided
  by their signed sum, and the quotient low word is stored at RAM:00f03f plus
  twice the big-endian RAM:00f03d sample counter.
- Firmware 1.04 RAM:fa7ae1 is a near `RET` function with its series index in
  ACC. Each of ten samples waits for SFR:F1 bit zero and then performs a fresh
  SFR:F1 read for bit one before reading D8 and D9. After sorting both ten-byte
  arrays, indices three through six contribute to indexed big-endian sums;
  each D8 sample and 0x78 offset and each D9 0x11 offset and sample are
  separate wrapping word read-modify-writes.
- Firmware 1.04 RAM:fd99dd is a no-input near `RET` dispatcher. Modes 0,1,3,
  9,14 subtract two from RAM:00fba2; mode 2 subtracts four; modes 8 and
  10..13 subtract three. Each supported path writes RAM:00fba6 and LCALLs
  RAM:fd9b81; all other modes write status seven to RAM:00fba0.
- Firmware 1.04 RAM:fab563 is a no-input ordinary `ERET` function. BITS:0031
  is cleared when BITS:0033 is clear or RAM:000c46 bit zero is exactly set;
  otherwise it is set. A speed above 40 and signed target above 0x47694 saves
  the selected speed, caps both speed bytes at 40, and sets RAM:00fad7 bits
  two and five through separate fresh word RMWs. The adjusted target is read
  separately for publication and target-minus-one MSF conversion.
- Firmware 1.04 RAM:faaca4 is a no-input ordinary `ERET` function that emits
  six zero-fill requests for region A, main data, auxiliary/C2, subchannel,
  per-slot metadata, and the trailing reserved range. RAM:000c8a bit four
  selects a main range beginning at 0x16650 with wrapping span-minus-base
  length and is cleared only after that fill; every RAM:00156a/base tuple is
  read afresh in the native order.
- Firmware 1.04 RAM:fad287 is a no-input ordinary `ERET` function. It clears
  Full TOC descriptor entries 1..499 while leaving reserved entry zero
  untouched, using field order 0,3,2,1,6,5,4,7,10,9,8. It then clears the
  cursor/session/byte-count and four-byte retained state before a fresh
  read-modify-write sets RAM:001132 bit six.
- Firmware 1.04 RAM:fad958 is a no-input near `RET` function that clears
  RAM:0012be and RAM:00fc1c, then clears 101 six-byte records at RAM:00f2b6,
  41 ten-byte records at RAM:00f7c4, and four ten-byte records at RAM:00f79c.
  The native code writes fields in deliberately non-linear orders; preserve
  those ordered volatile stores rather than substituting `memset`.
- Firmware 1.04 RAM:fad340 is a no-input `ERET` function. With fewer than two
  Full TOC sessions it delegates to the complete workspace reset. Otherwise it
  rebuilds decoded A0/A1/A2, ordinary track, B2/B3/B4 and indexed ADR-five
  state only from descriptors preceding the newest session, synthesizes the
  terminal AA record, clears descriptor records from the newest-session cursor
  through 499, and decrements the session count. Its long-lead-in flag requires
  summary flags low bits exactly one and a converted first-track tuple of at
  least 2,250 frames.
- Firmware 1.04 RAM:facd6e is a no-input `ERET` function gated out only for
  write type two with a non-C0 auxiliary byte. Its four logical record groups
  form one contiguous far-memory range, RAM:2bb03b..RAM:2bb918, but every
  ten-byte record is cleared in offset order 0,1,2,6,5,4,3,9,8,7. After the
  raw-Q parser it computes PMA start from one descriptor-count read, then reads
  the count again to choose first-track 0xff/one and publish mutually exclusive
  RAM:001132 bits seven/five through two separate fresh read-modify-writes.
- Firmware 1.04 RAM:fabd63 is a no-input `ERET` function that appends one
  session to the Full TOC table. Raw-Q payloads begin at RAM:008012 with a
  ten-byte stride: control/ADR is offset zero, POINT offset two, and the
  P tuple offsets seven through nine. It always emits retained A0/A1/A2,
  followed by an inclusive first-through-last track range; ADR-five presence
  bits then append B0, B1, B2, B3, B4, newly indexed records, C0 and C1 in that
  order. Every descriptor is eleven bytes, begins with the fresh session
  number, and advances both RAM:0012dc and RAM:0012ca.
- Firmware 1.04 RAM:fab902 is a no-input near `RET` parser. It snapshots the
  RAM:00800c raw-Q count, resumes ADR-one and ADR-three-through-six indices at
  each retained count plus one, and publishes each index minus one afterward.
  ADR-one raw minute/second/frame bytes are converted from packed BCD only
  below 0x9a, 0x5a, and 0x75 respectively; values at or above each threshold
  are retained literally. ADR-two captures control, converted tuple-one bytes,
  raw tuple-two byte one, and its presence flag without copying a full record.
- Firmware 1.04 RAM:faddb5 `CopyTenBytesFar` is a near `LCALL`/`RET` boundary
  with source in DR0 and destination in DR4. Because C251 assigns the first
  generic 24-bit parameter to DR4 and the second to DR0, its maintainable C
  prototype must be destination-first even though the operation is naturally
  described source-first.
- Firmware 1.04 RAM:faca48 is a normal near `RET` C function. Its two
  `fw_addr_t` parameters naturally compile to the proven DR4 source and DR0
  destination register tuple. It copies payload byte zero, forces descriptor
  byte one to zero, and copies bytes two through nine; all offsets increment
  only the low 16 address bits.
- Firmware 1.04 RAM:fa807e can directly own the full sample-window selector in
  near C; its R5/B-initializing tail thunk is not a required ABI. The rare
  mapped-value average adds bytes with 8-bit wrap and interprets the wrapped
  result as signed before division by two, so a narrow addressless adapter is
  retained. RAM:fa8095 is separately represented by a near-C wrapper around
  the complete algorithm, without the original scratch-register precondition.
- Firmware 1.04 RAM:fa7f94 can be represented as a near Boolean C function
  that directly owns the complete four-sample algorithm. Its R6/R7-clearing
  tail thunk is implementation scaffolding, not a required ABI. RAM:fa7faa is
  separately represented by a near-C wrapper around the complete algorithm,
  without the original scratch-register precondition.
- Firmware 1.04 RAM:fb9838 is a no-input signed-WR6-result ERET worker. Both
  RAM:fa8a40 and RAM:fb97ec consume WR6 immediately after ECALL; RAM:fa8a40
  sign-extends it, unlike the zero-extended RAM:fa8b1d companion. The worker
  captures and sorts ten synchronized D8/D9 pairs, averages indices three
  through six after fixed 0x78/0x11 offsets, optionally publishes both
  averages for media state six, stores a zero/sum/0xff saturated metric from
  the positive signed baseline deltas, substitutes deltas one and three when
  that metric is at most 16, and returns
  `1000 * (d8_delta - d9_gap) / (d8_delta + d9_gap)`.
- Firmware 1.04 RAM:fa8b1d is a normal near C boundary: no input is inherited,
  its u16 result is returned in WR6, and its sole caller uses LCALL. Its
  Timer-1 overflow test must precede each signed bit-seven controller-status
  test. C251 `sbit` helpers implement the Timer-1 accesses; an addressless near
  adapter supplies the fixed no-timeout cleanup tuple. Near declarations are
  required so C251 emits LCALL to RET bodies.
- Firmware 1.04 RAM:fa8909 has no inherited divide frame. It independently
  reads its sample count before each signed/unsigned division, returns through
  ERET, and its sole caller uses ECALL. Ordinary C can own the full finalizer;
  only addressless DR4/DR0-to-native quotient adapters are required for the
  two function-local divide ABIs.
- Firmware 1.04 RAM:fa870a is a near ACC-Boolean function whose saved R13,
  WR14, and four-byte stack slot are private state. The uncached path scans
  one-based 11-byte Full TOC records through a freshly read cursor, caches A2
  indices, and sets bit five only after a complete no-match scan. The cached
  path scans cache entries 0..98 and returns false immediately if a cached
  record is not A2. Its RAM:fee841 call supplies the tuple in DR0, requiring
  only an addressless DR4-to-DR0 adapter for an ordinary C caller.
- Firmware 1.04 RAM:fa861d is an ordinary no-input ERET function with no
  inherited register contract. A zero live result status skips RAM:00f79a and
  ready/result publication but still clears RAM:00105c bit three then bit two
  through separate fresh RMWs and clears RAM:001062. Nonzero status maps to two
  or four; an unready result either copies live capture state when a fresh bit
  two is set or publishes count zero, frame 150, and reverse-cleared sums.
- Firmware 1.04 RAM:fa84c4 is a near `RET` accumulator called with `LCALL`,
  not the ERET/inherited-register boundary described by its former extraction
  comment. It stores each of eight wrapping 16-bit sums immediately, compares
  signed `CurrentDecodedFramePosition` against wrapping window-start plus 74,
  publishes fresh count/start/sum reads, then clears live sums from channel
  seven down to zero. The caller's saved WR14 belongs to the caller's
  interrupt/SFR preservation, not an input contract for the accumulator.
- Firmware 1.04 RAM:fa8382 calls RAM:f8a803 with controller addresses in DR0,
  unlike the ordinary reconstruction C ABI's first `fw_addr_t` argument in
  DR4. It captures exactly five RAM:018083 bytes then four RAM:018084 bytes to
  RAM:001067..00106f. Only after all nine stores does it read RAM:01808e bit
  six and freshly clear or set RAM:00105c bit four. Fixed-port addressless
  adapters preserve the local DR0 call contract without retaining its body.
- Firmware 1.04 RAM:fa826b represents its RAM:00121a countdown as a signed
  interrupt-mutated byte. Exact RAM:00f1b7 bit zero starts a direct-high
  terminal wait and clears both sequence flags after expiry; subsequent
  negative countdowns alternate the indexed output state and both-lines-high
  phase through bit one. RAM:00f1b9 increments with byte wrap, while the
  RAM:00f1b8 limit comparison is unsigned and reads the limit before the fresh
  count. Preserve those volatile reads as separate sequenced C statements.

- Firmware 1.04 RAM:f92815 is representable as a near, non-returning C
  coroutine sink. It reads RAM:000f45 once for the signed-positive gate and
  again for the stored speed, performs one unconditional coroutine-two yield
  before testing RAM:000e40 on the BITS:0005 path, then yields forever. C251
  `sbit` helpers implement the direct BITS:0005 read and ordered
  BITS:0008/0022/001f clear.
- Firmware 1.04 RAM:f93e5a reads the per-slot metadata source at
  `RAM:00156e + RAM:000f5c * 16`, publishes the incremented cursor at
  RAM:0015d8 before each of ten atomic controller-buffer reads, then copies
  the completed ten-byte local record to RAM:000faf2. Its C boundary must
  preserve inherited DR12 across every nested call; narrow addressless
  capture/restore adapters satisfy that contract without retaining a
  same-address native body.
- Firmware 1.04 RAM:fadbe3 selects its three-byte strategy record from
  RAM:f82c98/f82d8e/f82e84/f82f7a for secondary operating codes other,
  0x10, 0x14, and 0x18. All three table bytes are signed deltas: byte zero is
  scaled by nine, byte two by eighteen, and codes 0x20/0x28 disable all three.
  The positive-direction gates use signed word comparisons, while the
  negative-direction gates use unsigned carry comparisons.

- Multi-byte firmware values are big-endian unless direct evidence says
  otherwise.
- Keep CPU pointers, controller-buffer addresses, SFR/bit references, and flash
  programming aliases as distinct source types or access paths.
- Preserve volatile access order for MMIO, interrupts, coroutines, and
  hardware-polled state.
- Use packed records only with compile-time size/offset assertions.
- Model phase-reused RAM with explicit lifetime views. A broad clear or copy
  does not prove that adjacent typed fields form one array or structure.
- Preserve exact ring geometry, cursor units, wrap behavior, and producer/
  consumer ownership before substituting an abstract queue.
- Firmware 1.04 RAM:fd7aaa treats RAM:00130b as signed for its positive-busy
  rejection while treating RAM:00fbf1 as unsigned. Its pseudorandom target is
  `((media_end * (random15 >> 2)) >> 13) + ((random15 >> 2) & 0x3f)`; it reads
  the media bound separately for multiplication and upper rejection, publishes
  every candidate before validation, and rereads the state counter after track
  lookup.
- Firmware 1.04 RAM:fd7e74 uses the same random scaling with a fixed upper
  bound derived from the ATIP lead-out minus 9000/18000/36000/72000 for speeds
  1/2/4/8. Its RAM:001202 gate is signed nonpositive, unsupported speeds trap,
  and a positive state instead checks the controller operating window.
- Firmware 1.04 RAM:fab26e receives start index in ACC and record count in R7.
  It aligns the first packed-record frame through signed truncating division by
  39 around a 150-frame offset, uses full RAM:2b bank addresses, compares the
  range threshold unsigned, compares the widened count bound signed, and can
  return a wrapping preceding index.
- Firmware 1.04 RAM:f8f127 always runs at least one Timer-1 interval, even for
  zero duration. It multiplies the WR6 duration by nine, chunks at 0xffff,
  writes TH1 before TL1, and tests the ACC yield flag for exact equality to one.
  The canonical loop and its atomic TF1/TR1 accessors are represented in C251.
- Firmware 1.04 RAM:f93429 decrements RAM:000f23 with byte wrap and applies
  signed expiry thresholds. With BITS:000e set and unsigned RAM:001292 at most
  75, it advances RAM:001598 by two and subtracts two from RAM:000f36 when the
  signed countdown is at most 23 for media state two or at most 15 otherwise.
  Above that queue threshold, an expired countdown reloads to 25. Final expiry
  calls the near parsed-Q enqueue helper RAM:f934d7 and then sets BITS:0009.
- Firmware 1.04 has 27 inactive indexed-state dispatch targets at RAM:fd7aa4,
  RAM:fd7aa6, RAM:fd7aa8, RAM:fd7bed, RAM:fd7bef, RAM:fd7bf1, RAM:fd7cff,
  RAM:fd7de4, RAM:fd7e4e..RAM:fd7e60, and RAM:fd7e62..RAM:fd7e72. Each is an
  independent canonical function that returns byte zero through ERET. C251's
  ordinary `u8 function(void)` ABI emits the exact `CLR A; ERET` body, so none
  requires a native assembly boundary.
- Firmware 1.04 RAM:fbb2c3, RAM:fbb2c4, and RAM:fd12ae are distinct canonical
  one-instruction no-op functions. An ordinary C251 `void function(void)`
  emits their exact single-byte `ERET`, so these return points do not require
  assembly despite their interrupt/coroutine callers.
- Firmware 1.04 RAM:f8e72e and RAM:f8e731 are pure atomic P1.4 clear/set
  functions ending in ERET. C251 can express them in C with an address-bound
  `sbit`; at `OPTIMIZE(8)` it emits the exact `CLR 094H; ERET` and `SETB 094H;
  ERET` bodies. At `OPTIMIZE(0)` C251 also modifies CY, so keep this pair in
  its dedicated optimized translation unit.
- In 80251 source mode, legacy bit address `0xaa` selects `SFR:A8.2`; it
  does not select `SFR:A9.2`. A251 `SETB 0A9H.2` emits the stock extended
  encoding `A9 D2 A9` (and `CLR 0A9H.2` emits `A9 C2 A9`). Firmware 1.04
  `RAM:fee815` uses the extended set. The wrong C251 `sbit=0xaa` alias
  produced `SFR:A8=0xc6` at a repeated scheduler checkpoint versus stock
  `0xc2`; the corrected encoding restored `0xc2` in both images.
- Firmware 1.04 RAM:fee80e atomically clears IE.EA and returns; RAM:fee811
  atomically sets IE.EA, IE.6, and SFR:A9.2 in that order and returns. C251
  address-bound `sbit` assignments at `OPTIMIZE(8)` emit their exact canonical
  instructions and ERET terminators, so these interrupt-control helpers are C
  implementations rather than necessary assembly exceptions.
- Firmware 1.04 RAM:f8f08a is a one-byte ERET function reached with a
  preexisting extended-return frame. A C251 `void function(void)` emits the
  identical single byte, so its nonlocal use does not itself require assembly.
- Firmware 1.04 RAM:f9e2ca has no hidden register input: it writes 0x14 and
  0x18 to RAM:001058/001059, calls RAM:f9d777, and returns through RET. Its C
  body must be `PLEXTOR_NEAR_FUNCTION`; keeping it in a small translation unit
  avoids the C251 64-KiB ceiling reached by `status_mapping.c`.
- Firmware 1.04 RAM:f9d45c has no hidden register input and returns through
  RET. Operating code 0x0a publishes range 4..10 and conditionally calls
  RAM:f9d777; codes at least 0x18 conditionally call RAM:f9e2ca before
  publishing range 0x14..0x18. It is safely represented as a near C function
  in the same small unit as RAM:f9e2ca.

## Reconstruction claims

- In the Linux agent environment, the installed Keil tools are usable through
  Wine at `/home/hbrych/.wine/drive_c/Keil_v5/C251/BIN/`: C251 5.60 and
  A251 4.69.13 execute successfully. The build driver must prepend `wine` for PE
  executables and translate absolute POSIX paths, including paths embedded in
  `INCDIR(...)`, through Wine's `Z:` mapping. Every C251 step needs the project
  `src` include directory in addition to the compiler's own INC/LIB paths.
  Pass explicit `OBJECT` and `PRINT` paths under `out/agent-tests/` so
  verification never writes into the user-managed `build/` tree.
- In the installed Keil tools, use `MODSRC`, `SEGMENT ECODE AT` for 24-bit
  absolute assembly placement, and OH251 `H386` with an explicit high-address
  `RANGE`. A default-range OH251 invocation can emit only EOF for a valid
  high-address linked program. The CMake build reproduces this working setup.
- A251 requires an explicit `EDATA` class on low RAM direct references such as
  `RAM:0000fa` to retain the stock direct16 encoding; an unqualified numeric
  address selects the shorter direct8/SFR form. A251 listing and byte comparison
  established this distinction. Keil DR56/DR60 correspond to Ghidra DPX/SPX
  here.
- A251 `LJMP` relocation is limited to its reachable code bank.  Reconstruction growth moved the floating return tails across that boundary and produced L251 `L121 IMPROPER FIXUP`; use `EJMP` for an intentional cross-bank native tail transfer, preserving the `ERET` ABI at the destination.
- The source reconstruction target uses C251 `ROM(HUGE) LARGE`: automatic
  storage is then allocated in XDATA, avoiding the 128-byte DATA allocation
  limit encountered with `SMALL`. Keep this separate from any eventual
  evidence-backed mapping of persistent firmware RAM fields. The target still
  uses `NODEFAULTLIBRARY`, but the full link has unresolved C251 runtime helpers;
  any selected library or replacement must have its behavior and image impact
  recorded.
- C251 enforces a 64-KiB code-size ceiling per C translation unit even with
  `ROM(HUGE) LARGE`. `src/platform/status_mapping.c` has reached that ceiling:
  adding either the 174-byte CLOSE SESSION validator or the small RAM:fe1f20
  ring-cursor worker caused error C8. Moving those functions to
  `leaf_workers.c` restored zero-diagnostic builds. Place subsequent leaf
  reconstructions in a smaller unit unless a cohesive source split is
  performed.
- Under the project's C251 `ROM(HUGE) LARGE` model, ordinary C functions emit
  `ERET`; applying the source-level `near` function qualifier emits native
  `RET`. Use `PLEXTOR_NEAR_FUNCTION` only where the canonical function exit is
  proven to be `RET`, and confirm the result in the C251 listing.
- C251's `reentrant` qualifier gives the multi-argument refinement helper at
  RAM:fc450b the required stack-passed third `s32` parameter. This remains a
  local ABI fact, not a universal firmware calling convention.
- Firmware 1.04 RAM:fa63ff passes the frame-position dword in DR4 and the
  destination tuple address in WR2 to RAM:feeb12
  `ConvertFramePositionToBinaryMsfTuple`. The shared source declaration's
  ordinary argument order does not produce that call-site ABI under C251;
  verify the generated listing and use a translation-unit-local target
  declaration where required. RAM:fa8811 is independently proven to be a near
  `RET` function from its own exit and the RAM:fa6490 `LCALL` site.

Track these independently:

1. Behavioral match: tests reproduce outputs and state transitions.
2. Function-byte match: the selected toolchain emits an identical bounded
   routine.
3. Image match: code, tables, placement, padding, header, and checksum match.

Decompiler output is evidence, not source equivalence. A compiling image or a
correct checksum does not establish hardware safety.

- Target fixed `const` data uses the L251 `HCONST(0xFE0000-0xFEFFFF)` class placement. This keeps C-derived profile records below the reserved recovery boundary at `RAM:ff0000`; do not relax the image recovery-hole gate.

- Firmware 1.04 `RAM:f8f47a` `MapOperatingCodeToProfileIndex` has 13 output buckets: 0 for input 0; then 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, and 12 for the upper-bound thresholds 2, 4, 8, 10, 12, 16, 20, 24, 32, 40, 48, and infinity respectively. This is verified from canonical instruction branches, not inferred from profile-table contents.

- Firmware 1.04 `RAM:fc9548` `ResetControllerParameterState` initializes zero `RAM:00fc14` to `0xd8`, then emits a fixed controller-parameter sequence. `RAM:001008 == 2` selects FC0E for FA, F4 down-adjustment, FF/E0 values, EC low bits, P0/E7/E5 setup, and EE bit zero; `RAM:001304` bit zero conditionally emits `0xfb81`. This is verified from the complete canonical instruction sequence and host transaction traces.
- Firmware 1.04 RAM:fd643b invokes the separate recovery monitor at RAM:ff4500 with the Boolean in ACC and receives its Boolean result in ACC. The main image must preserve this as an explicit interface and must not import recovery-monitor payload; pre-call it conditionally drives BITS:0580 low/BITS:0581 high, while successful return restores P3 according to RAM:000fae.

- Canonical decompiler optimization can remove controller-shadow handshake
  writes even when the decoded instructions are correct. Recheck MMIO/shadow
  store order from instructions; a Boolean parameter type can additionally hide
  exact byte selectors (firmware 1.04 RAM:fbd495) or ACC low-bit semantics
  (RAM:fbe586). Correcting bool to byte alone does not restore volatile order.
- MSVC Release defines NDEBUG and removes C assert expressions, including any
  calls inside them. The existing general reconstruction host target therefore
  is not an assertion-enabled behavioral gate. The controller-enable/setup
  target explicitly undefines NDEBUG before including assert.h; Debug linking
  exposed missing readback helpers previously masked by Release compilation.

- Reconstruction coverage counts canonical addresses, not source occurrences.
  Once a canonical function has accepted C, it cannot retain a same-address
  assembly or literal-DB body. Addressless ABI adapters are not canonical
  function bodies and do not affect coverage. The live read-only dependency
  inventory can identify recorded-callee readiness, but shared-tail transfers
  still require instruction review before translation (for example the
  integer-to-float entry chain).

- Firmware 1.04 controller-buffer byte copies have asymmetric pointer rules:
  RAM:fde0a6 reads through full 24-bit physical addresses and increments only
  the destination low word; RAM:fddd78 increments the source low word and
  the full physical destination. Both return initial controller offset plus
  initial count in DR4, proven by MODE SELECT/MODE SENSE and TOC callers.
  Their native count slots are reread each loop (word versus dword); the C
  value interfaces explicitly require non-overwritten caller count storage.

- Fixed-language indexed WR addressing zero-extends the base before adding
  displacement; it differs from an explicit WR increment, which wraps to
  16 bits. Firmware 1.04 RAM:fb7ffe and RAM:feecc1/feed99 exercise this
  distinction; their +8/+0x15 and +1/+2 loads may cross RAM:00ffff.
- Firmware 1.04 packed-BCD decrement RAM:feed99 explicitly stores 0x74,
  0x59, 0x99 when decrementing zero. The earlier C used decimal 74 and its
  legacy assertion copied that mistake. Independent position-based oracles
  now cover all 450000 valid tuples; inspect instructions even when an old
  behavior-tested label and a self-consistent test already exist.
- Firmware 1.04 RAM:f892dd treats the media-state countdown as expired only
  when BITS:0032 is clear, the big-endian reload at RAM:000fb4 is nonzero, and
  the signed big-endian countdown at RAM:000fb0 is negative. These checks
  short-circuit in that order, so the suppressed RAM reads are observable.
- Firmware 1.04 RAM:f8a9a1 writes `0x80` three separate times and then zero to
  volatile RAM:018002 before loading controller operating profile zero and
  resetting operating state. The Ghidra decompiler collapses the repeated
  stores, so the instruction listing is required to preserve this sequence.
- Firmware 1.04 RAM:f8e068 returns a real 16-bit value in WR6 despite its stale
  Ghidra `void` signature: its sole caller at RAM:f8dbd9 stores and tests WR6.
  On success the result is port RAM:0180c7 as the high byte and RAM:0180c6 as
  the low byte; timeout returns zero after an unbounded settling wait.
- Firmware 1.04 RAM:f8e0d9 forms its second controller parameter by
  zero-extending RAM:00fc14, subtracting 0x24 as a 16-bit word, and then ORing
  0xf400. Values below 0x24 therefore produce `0xffxx`, not byte-wrapped
  `0xf4xx`; retain the word-width subtraction in C.
- Firmware 1.04 RAM:fd4081 decodes the READ MASTER CUE transfer count as the
  top 24 bits of the big-endian dword at RAM:000b10, then compares the wrapping
  RAM:0012c1-plus-0x1c0c endpoint to RAM:001153 with a signed 32-bit comparison.
  Unsigned endpoint validation changes behavior across the sign boundary.
- Firmware 1.04 RAM:fbe5ac calls the configuration-range writer with source
  RAM:00f087 in DR0, destination offset 0x34 in DR4, and byte count two on the
  caller stack. The decompiler's apparent zero-valued arguments are not a safe
  source interface; the pushed word is a callee argument, not a local array.
- Firmware 1.04 RAM:f92955 performs two distinct reads of RAM:000efb around
  controller-port 0x8004 initialization, reads RAM:000fad only once before
  mirroring its masked value to port 0x8003, and writes port 0x8003 before
  port 0x802d. The decompiler collapses/reorders parts of this MMIO sequence;
  retain the decoded instruction order in C.
- Firmware 1.04 RAM:fdcab6 writes RAM:000fbe before its first direction sample,
  samples RAM:000c8d again when composing bit six, and updates RAM:000fba with
  three separate volatile read-modify-writes for bits zero, four, and six. A
  cached direction byte and one combined flag store do not preserve this
  observable sequence.
- Firmware 1.04 RAM:fdca1e reads the big-endian end target at RAM:000c92
  before the pending target at RAM:000c8e for its unsigned acceptance test.
  Source must use sequenced temporaries; placing both volatile reads directly
  in a relational expression leaves their evaluation order unspecified in C.
- Firmware 1.04 RAM:fd9b81 is a near function ending in native RET, despite a
  stale decompiler annotation describing ERET. Its three callers all use
  LCALL. For function-local ABI classification, the canonical terminal opcode
  and decoded caller instructions take precedence over decompiler prose.
- Firmware 1.04 RAM:fd9659 publishes `RAM:000fca | 0x08` to both the shadow
  and controller port RAM:018002 immediately after disabling global interrupts
  on preparation failure, before clearing RAM:018080 and RAM:0180c0. Ghidra's
  decompiler omitted this first shadow/port publication; preserve the decoded
  instruction order in the C coroutine body.
- Firmware 1.04 RAM:fd9f6e masks RAM:000fca with 0xe7 and immediately
  publishes the result to controller port RAM:018002 before initializing the
  remaining startup-mode registers. The decompiler displayed that port write
  later; instruction order is authoritative. Startup mode three is an
  intentional non-yielding infinite trap, while the other modes return ERET.
- Firmware 1.04 RAM:fdcbdf rereads RAM:000fae independently for its zero,
  two, and three tests; the matching route therefore performs one, two, or
  three volatile state reads. Do not cache this state byte in the C form.
- Firmware 1.04 RAM:fafd77 READ BUFFER CAPACITY reads RAM:00129c once for the
  total-byte field and again for available capacity. With RAM:000c84 bit three
  set, it reads RAM:001292 before comparing against a fresh RAM:00129c, then
  rereads both operands in high-water-then-queued order for subtraction. Clamp
  queued-above-high-water to zero; otherwise multiply the 16-bit sector count
  by 0x930 and cap the CDB allocation length at twelve bytes.
- Firmware 1.04 RAM:f84a00 and RAM:f84b00 are representable as nonreturning C
  coroutine entries: the former clears RAM:001591/RAM:000f4a and resets drive
  state before repeatedly yielding coroutine two, while the latter clears
  RAM:00fb9a before repeatedly yielding coroutine three. RAM:fdc225 is the
  corresponding pure coroutine-zero yield loop. C251 optimization eight emits
  each permanent loop without an unreachable ERET.
- Firmware 1.04 RAM:fbd412 receives the frame delta in DR4 and a caller-owned
  selector in ACC; its sole caller computes that selector from RAM:000c84 bit
  four immediately before LCALL. The branch is therefore not derived from the
  delta's zero flag and must remain an explicit C parameter.
- Firmware 1.04 RAM:fdccc1 receives a signed base target in DR4 and signed
  adjustment in DR0. Their addition wraps at 32 bits. A nonpositive result is
  clamped to one only after short-circuit reads show BITS:001b and BITS:0021
  both clear. It writes request kind one to RAM:000c8c before the big-endian
  target at RAM:000c8e, dispatches acquisition, yields coroutine two while
  RAM:000e40 is nonzero, and returns RAM:000e41 in ACC through ERET.
- Firmware 1.04 RAM:fdc5be performs two distinct read-modify-writes of SFR:88:
  it first sets bit four and then applies mask 0xf5 using a fresh read. It clears
  the controller buffer first from 0x010000 for 0x1b0000 bytes, then from the
  big-endian base at RAM:00155a for `0x1b0000 - base`; preserve both interrupt
  gates and the exact subsystem-initializer call order between those fills.
- Firmware 1.04 RAM:fb5c02 rounds `WorkingTrackPacketProgressBlocks /
  WorkingTrackPacketSize` upward only for a nonzero remainder, then uses
  `packet_count * (WorkingTrackPacketSize + 7)`. Media state two compares the
  wrapping unsigned candidate `RAM:00dd49 + span` against RAM:00dd45 and only
  raises the latter; other states add the span directly to RAM:00dd45.
- Firmware 1.04 RAM:fafa20 validates its requested RESERVE TRACK block count as
  unsigned, but compares the published WorkingTrackEndFramePosition against
  AtipLastPossibleLeadOutFramePosition with signed 32-bit JSLE. Fixed-packet
  mode requires an exact multiple and publishes `start +
  (packet_size + 7) * packet_count - 5`; other modes publish `start + count +
  2`. The end is written before the signed lead-out rejection.
- Firmware 1.04 RAM:fb892e uses a signed divide-by-ten remainder while aligning
  PmaStartFramePosition: positive remainders one through four add to the PMA
  start, five through nine subtract `10 - remainder`, and zero or negative
  remainders leave it unchanged. All lead-in, current-position and fallback
  comparisons are signed; the fallback equality sets RAM:001132 bit seven.
- Firmware 1.04 RAM:fbd7d6 handles pre-existing status only when RAM:000fae is
  exactly one and reads the ASC/ASCQ word before the sense-key byte from the
  status tables. Its acquisition-busy loop tests RAM:000e40 before each yield;
  after a zero acquisition result, the measurement loop tests RAM:00f08c bit
  two, yields, services an immediate host command, and returns early only if
  that service reports zero before mapping RAM:00f08d.
- Firmware 1.04 RAM:fb28b3 validates the RECEIVE DIAGNOSTIC RESULTS allocation
  length as a 16-bit unsigned value against nine, but then passes only its low
  byte to the eight-byte minimum helper. Thus an allocation of 0x0100 produces
  a zero-length transfer attempt rather than eight bytes; retain the explicit
  `u8` truncation in C.
- Firmware 1.04 RAM:fdcc69 has two observably different completion waits. The
  ordinary route reads RAM:000fba before each possible yield. The target-aware
  route first reads signed CurrentDecodedFramePosition at RAM:001235, then
  PendingReadAcquisitionTarget at RAM:000c8e, and returns immediately when the
  target is reached without reading RAM:000fba or mapping RAM:000fbc.
- Firmware 1.04 RAM:fddff4 adds a 1-MiB window bias to the controller-buffer
  offset in RAM:00155a, advances that source as a full 24-bit pointer, but
  advances its DR0 RAM destination through the low 16-bit word only. A copy
  across `xxffff` therefore enters the next source bank while wrapping within
  the destination's existing bank.
- Firmware 1.04 RAM:fd0d1c uses four contiguous ten-record pickup-profile
  tables at RAM:008867, RAM:00888f, RAM:0088b7, and RAM:0088df. Each four-byte
  record supplies separate big-endian values for controller selectors 0x011d
  and 0x011e. Only distances at least 256 use fixed selector-0x011d value
  0x00b0; the decompiler's apparent unconditional zero submission is false.
- Firmware 1.04 RAM:fd0ecf retains controller port 0x80e3 bits 7, 6, 1, and 0
  with mask 0xc3. A masked zero returns status one; bit 1 clear in controller
  port 0x80f8 returns status two; after cleanup, retained bits 0/1 return three,
  bit 6 returns four, and bit 7 alone returns zero. Treating the initial port
  read as Boolean destroys this classification.
- Firmware 1.04 RAM:fd4f13 writes volatile RAM:018079 first with 0x44 and later
  with 0x40 around its controller-shadow reset sequence. It then invokes the
  format-two raw-sector reader for the wrapping half-open frame interval
  `[position + 2, position + 3)`. The decompiler can obscure the repeated
  port write, so preserve both stores and unsigned 32-bit wrap in C.
- Firmware 1.04 RAM:fe1370 reads WorkingTrackControlFlags at RAM:00dd36 twice:
  the first value is copied into CurrentRawReadQPacket, while the fresh second
  value selects the data-track acquisition path. Data-track counts of five or
  more first call the fixed-prefix worker with start zero and count nine, then
  subtract five frames; a prefix failure reports sense 03/0c07 with the
  current ReadPipelineTargetAddress.
- Firmware 1.04 RAM:fe0246 copies its three-byte Q-MSF source into
  RAM:001279..00127b using increments of only the source pointer's low 16-bit
  word. A source at 0x12ffff is therefore read from 0x12ffff, 0x120000 and
  0x120001, not from the next bank. The capacity failure returns before any Q
  or target read, while capture failure reports sense 04/4400 and reads
  ReadPipelineTargetAddress.
- Firmware 1.04 RAM:f9241f decrements the parsed-Q countdown as a wrapping
  byte, applies signed thresholds, and decrements RAM:000fa2 before testing
  its old signed value. Thus a zero capacity countdown briefly underflows to
  0xff and continues into descriptor construction, where it is replaced by
  ten. The stable path calls near RAM:f92769 and sets BITS:0016 before
  BITS:0017 for in-range targets; the unstable path sets BITS:0017 before
  clearing BITS:0016 and builds status 0x2100/mode five. Both enqueue before
  choosing BITS:0009 termination versus target increment and state reset.
- Firmware 1.04 RAM:f9ec58 selects its alternate controller-profile table
  family only when RAM:00fb5e equals exactly one. Float32 profile values at or
  below 20.0 select RAM:f83eb6 (ordinary family) or RAM:f83ee3 (alternate
  family); values strictly above 20.0 select RAM:f83ec5 or RAM:f83ef2.
  Returning the
  indexed signed byte directly is value-equivalent to the native signed-byte
  to float32 to signed-int conversion tail for every possible table byte.
- Firmware 1.04 RAM:fb83a4 initially rejects packet markers below 0xe0, then
  uses RAM:00fb7e bits zero through two as a selector. RAM:001306 bit zero plus
  validation mode two forces selector seven. Selectors six, seven and four
  perform a fresh RAM:00fb5b read and respectively require 0xf8, 0xfc and 0xf0;
  all other selectors accept the initial marker and invoke the ATIP decoder.
- Firmware 1.04 RAM:fae3db builds the basic READ DISC INFORMATION response with
  sixteen ordered writes. Its length byte is the wrapping value
  `((RAM:00e74b << 3) & 0xf8) + 0x20`; after writing 0xff to RAM:000b3b it
  rereads that volatile byte once and fans the sampled value out to RAM:000b41,
  RAM:000b40, RAM:000b3f, RAM:000b3d and RAM:000b3c in that order.
- Firmware 1.04 RAM:fae4c0 retains each Full TOC session's POINT A0 P-second
  byte and returns it only when the same session also has a point at most 99
  with control bit 0x40. It reads each matching record's point twice, examines
  the next record's session at offset eleven for the boundary, and falls back
  to session one's POINT A0 before returning 0xff.
- Firmware 1.04 RAM:faf5b1 indexes ten-byte records from RAM:2bb03b by the ACC
  track number. A matching record converts MSF tuple offset seven before offset
  three and returns the wrapping `second - first - 2` result; a mismatch uses
  RAM:faf635. Its native value contract is ACC input, DR4 result and RET.
- Firmware 1.04 RAM:faf635 derives the Full TOC descriptor count by dividing
  the big-endian byte count at RAM:0012ca by eleven, scans one-based far records
  at RAM:2b8000 for the requested track's session, and then consumes that
  session's POINT A1 last-track byte and POINT A2 lead-out tuple. It converts
  decoded six-byte summaries at RAM:00f2b9 plus six times the track number and
  returns lead-out minus start for the last track, otherwise the next decoded
  boundary minus start; subtraction wraps as a dword. Its native value contract
  is ACC input, DR4 result and RET; saved DR12 is private and R9/B are not
  inherited inputs.
- Firmware 1.04 RAM:faf43d computes READ TRACK INFORMATION track capacity for
  working-track types zero through seven. Fixed-packet routes use
  `packet_size * ((frame_span + 5) / (packet_size + 7))`; linear routes use
  the selected end minus start minus two, except non-packetized type six uses
  its boundary minus start. Type zero selects the working end or ATIP lead-out
  through the option-bit and saved-track gates, and the final result is clamped
  to zero unless positive as a signed dword. Its native value contract is no
  inputs, DR4 result and RET. The native inherited-DR28 fallback is reachable
  only for invalid types outside the sole caller's zero-through-seven invariant.
- Firmware 1.04 RAM:faf206 computes READ TRACK INFORMATION free blocks. Types
  four through seven return zero. With RAM:000c84 bit three clear, track types
  one and three measure from RAM:00dd45 plus five, making their linear result
  endpoint minus cursor minus seven; with the bit set and write type zero they
  measure directly from RAM:00dd4d and subtract two. Fixed-packet routes return
  `packet_size * (span / (packet_size + 7))`. Write types two and three use the
  dedicated RAM:00dd6d and RAM:00dd69/RAM:00dd65 boundaries, and negative
  signed results clamp to zero. Its native value contract is no inputs, DR4
  result and RET; inherited DR28 matters only for invalid caller states.
- Firmware 1.04 RAM:fae0bd builds the recordable READ DISC INFORMATION response
  at RAM:000b2a. It distinguishes the fixed-packet saved-table path, clamps the
  last-track sentinel 100 to 99, maps RAM:000c9e states one through three into
  media-mode flags (and fail-stops on other nonzero values in mode two), packs
  the optional PMA ADR-2 bytes as `10000*a + 100*b + c`, selects converted MSF
  tuples versus 0xff sentinels from RAM:00dd79/RAM:000c9e, and copies eight
  bytes per fresh RAM:00e74b OPC record from RAM:00e72b to RAM:000b4c. Its
  native value contract is no inputs, ACC Boolean result and RET.
- Firmware 1.04 RAM:fadde0 accepts SEND CUE SHEET only for write type two and
  an eight-byte-aligned transfer of at most 0x2800 bytes. After clearing and
  receiving RAM:00b4bd, it stores the entry count at RAM:00b4bb, scans backward
  for point 0xaa to publish record bytes five through seven at RAM:001592, and
  scans track points 1..99 with ADR nibbles other than two/three: bit-six-clear
  entries set RAM:000c87 bit seven, while absence of a qualifying bit-six-set
  entry sets RAM:00158e bit one during the mode-one speed path. Transfer result
  four exits before parsing; successful parsing clears RAM:00128e and sense.
  The native function has no inputs or result and returns with ERET.
- Firmware 1.04 RAM:fafee3 failure cleanup rereads and rewrites RAM:000c84
  twice, first with mask 0x7f and then with mask 0xbf. On success it copies the
  full big-endian dword RAM:001488 to RAM:00dd45, changes working-track type six
  to one or five to three (with a fresh second type read for the latter test),
  and finalizes SCSI opcode 0x35.
- Firmware 1.04 RAM:fdd287 performs distinct volatile read-modify-writes around
  PrepareWritePipelineControllerRecordState: it clears RAM:000c88 bit six,
  RAM:000c89 bit three and RAM:000c8a bit zero, sets RAM:000c85 bit zero and
  RAM:000c84 bit three, then freshly rereads RAM:000c85, RAM:000c84 and
  RAM:000c8a to clear those two preparation bits and bit seven respectively.
- Firmware 1.04 RAM:fb8319 inherits R15 even though its typed Ghidra signature
  has no parameters. The value reaches RAM:f983b1 and is stored at RAM:00f05f
  only on the media-state-two/no-classifier route. Capture inherited R15 before
  any C helper calls; the six BCD conversions and two frame-position conversions
  otherwise follow the exact straight-line sequence ending in ERET.
- Firmware 1.04 RAM:fe7d5a calls RAM:fea372 with decrement-relative-MSF in R7
  and skip-identification in ACC. Keil C251 assigns the first ordinary byte
  argument to ACC and the second to R7, so a semantic C dispatch taking
  `(decrement, skip)` must reverse them at the native call. The worker rereads
  RAM:001030 independently before advancing each of RAM:001275 and RAM:001279.
- Firmware 1.04 RAM:fee8b0 clears RAM:001459..00145c before its five byte-state
  clears, then copies the big-endian block-size word RAM:000b00 to RAM:00145d
  and block-count dword RAM:000b06 to RAM:001455. It returns the wrapping
  unsigned value RAM:000b02 plus 150 in DR4 and exits through ERET; BITS:0011
  is cleared between the byte-state clears and the two copies.
- Firmware 1.04 RAM:feea32 treats the first start-LBA bound and media-end
  selection as signed, but compares the wrapping start-plus-block-count result
  to the endpoint as unsigned. Negative LBAs -140 through -1 are accepted
  without a band bit, -141 through -999 are rejected, -1000 through -19999 set
  BITS:0013, -20000 through -29999 set BITS:001c, and values at most -30000 set
  BITS:001d. Rejection reads RAM:000f0c before rereading RAM:000b02 for sense
  information, so that volatile order must remain explicit in C.
- Firmware 1.04 RAM:fd3ba5 uses an inclusive signed special band of -150
  through 300. Outside its early mode gates it clears BITS:0016 before
  BITS:0017, accepts negative starts through the lead-in route only when the
  relevant mode state permits, and applies a wrapping unsigned end comparison
  after a signed start comparison on the positive constrained route. Sense
  information preserves fresh RAM:000f0c, RAM:000b02, and, for endpoint
  rejection, RAM:000b06 reads in native order.
- Firmware 1.04 RAM:fcecdc reads the 32-bit radial-model scale twice. It forms
  a word ratio by executing `0x5618 / (u16)(scale >> 8)`, maps strictly positive
  frame seconds by wrapping addition around 0x04cb0000 and zero/negative values
  by wrapping magnitude and subtraction, takes the integer square root, then
  returns the low word of the root-times-ratio product shifted right eight.
  Preserve the target word-DIV instruction with an addressless adapter because
  its zero-divisor behavior has no portable C meaning.
- Firmware 1.04 RAM:fc8a4c is a near-call function and RAM:fc94a7, its final
  pending-target-nudge clear, is also near. The initializer divides the signed
  target by 75 before radial mapping, remaps exactly sentinel 0x10000000 as
  zero, derives the tolerance from the mapped word's high byte, and selects
  retry limit 1 versus 3 and timer 0x3c versus 0x50 from independent fresh
  gates. C declarations must retain both near qualifiers to reproduce RET and
  the final LCALL.
- Firmware 1.04 RAM:fcb8c3 selector values one and two write gate value four
  to RAM:018041 before reading controller ports RAM:018042 or RAM:018043, then
  clear the gate. The decompiler exposed only the clear; instruction review is
  required to retain both volatile writes. Selectors three and four read the
  same ports without gating, and the default path packs active-low P0 bits one,
  two, six, zero, and five into result bits zero through four.
- Firmware 1.04 RAM:fd4a5d preserves an all-ones frame-position sentinel on
  the non-MSF PLAY CD path; other big-endian starts receive wrapping +150.
  For the MSF path, a semantic C declaration ordered `(end, start, mode)` is
  proven by C251 to emit the native DR4=end, DR0=start, ACC=mode tuple.
- Firmware 1.04 RAM:fceedd reads the signed acquisition target twice, retains
  only the low byte of the first target/4500 quotient, compares that raw byte
  with RAM:00125c before dividing it by five, and caps the resulting bucket at
  fifteen. Operating codes 0x18, 0x20, 0x28, and 0x34 select tables
  RAM:ff904e, RAM:ff905e, RAM:ff906e, and RAM:ff907e; unknown codes enter the
  canonical self-loop.
- Firmware 1.04 RAM:fd3b0d clears BITS:001b on entry. With RAM:0015d7 nonzero,
  RAM:000c88 bit seven routes only negative LBAs through the 32+7 mapper and
  sets BITS:001b; nonnegative values use track mapping. With the bit clear,
  only signed values from zero through the inclusive maximum at RAM:2ba529 use
  the alternate-record mapper; negative and greater values use track mapping.
- Firmware 1.04 RAM:f87625 is a near RET worker and does not touch inherited
  R15. It samples RAM:001008 independently at its initial stage gate, its
  state-one profile route, and its final state-two route. When RAM:000fd0 bit
  three is set it clears that bit and atomically sets BITS:003b, BITS:0039,
  BITS:003a, and BITS:0036. The state-one profile route rereads RAM:001137 for
  comparison, the profile-call coordinate, and the final RAM:00fa00 store.
- Firmware 1.04 RAM:f87587 is a signed-DR4 near RET worker. For media state one
  without the option override, it uses the low byte of target/4500 as the
  controller-profile coordinate and selects RAM:00fbc3 or RAM:00facf as the
  big-endian EA/EB value according to secondary/current operating code. The
  override tests BITS:0044, conditionally programs D4/E2 code 0x14, and always
  writes zero EA/EB. Every route freshly reads RAM:001008 before writing the
  RAM:00fab5-derived E8/E9 value.
- Firmware 1.04 RAM:fd458e treats only input start -1 as the resume-current
  sentinel. It snapshots signed RAM:001235, forms the exclusive end with
  wrapping 32-bit addition, and validates start/end through the near RET worker
  RAM:fd4b11. Only the sentinel path can resume: RAM:000faf must equal one and
  RAM:000fba bit four must be clear. A nonzero playback-mode byte then sets bit
  three using a fresh read, and the computed exclusive end is passed in DR4 to
  RAM:fdcb43 before no-sense completion. All other accepted paths forward
  start/end/mode in DR4/DR0/ACC to RAM:fd4ec5.
- Firmware 1.04 RAM:fc8e1a snapshots RAM:000e43 to RAM:000e4d before all
  selection work. Selector zero stores code one; selector five maps requested
  code zero to one and nonzero to eight. Selector one acts only when RAM:000e45
  equals one: request one selects RAM:000e47, while other requests are passed
  through the high-code floor helper and then reread, with zero forcing code
  one. Other selectors are reread and passed to that helper. The supplied DR4
  threshold is preserved for RAM:f8b9e6, and RAM:001304 bit three is consumed
  with a fresh read-modify-write.
- Firmware 1.04 RAM:fcf967 must preserve inherited DR12 because its high-code
  child RAM:fcee18 can consume that state outside its four recognized table
  codes. C251's C body is acceptable because its listing contains no DR12/R12
  access. The worker uses tuple two when parsed-Q status bit three is set;
  otherwise it uses tuple-one byte zero, clamps values at least 0x5a to zero,
  and forces RAM:00125d to 0xff. With controller operation disabled it calls
  RAM:fcfda0 using ACC=RAM:00124c and DR0=RAM:00123d.
- Firmware 1.04 RAM:fc8ec1 and RAM:fcfc9d are both near functions ending in
  RET, and their callers use LCALL. Their C declarations and definitions must
  carry `PLEXTOR_NEAR_FUNCTION`; treating the earlier unqualified C bodies as
  ordinary ERET functions produces an ABI mismatch despite correct high-level
  behavior.
- Firmware 1.04 RAM:fc8ff3 clears RAM:000ee3 first. With controller operation
  enabled it updates the packet-frame distance, then stores the radial-model
  distance. FullTocSummaryFlags bit two forces distance 50. A zero previous
  frame converts the RAM:001241 tuple and stores it in RAM:000ef1; the native
  body writes 300 and then, after freshly rereading the still-disabled
  controller state, overwrites it with 50. The ordinary route stores the low
  16 bits of packet distance shifted right three plus five, applies a signed
  lower clamp at 10, then an unsigned upper clamp at 200.
- Firmware 1.04 RAM:fc909c is a near function ending in RET and its caller at
  RAM:fc8ff3 uses LCALL. Its declaration and definition must carry
  `PLEXTOR_NEAR_FUNCTION`; C251 listing verification confirms the corrected
  RET boundary.
- Firmware 1.04 RAM:f92296 is a near Boolean classifier. RAM:000c3c bit zero
  selects direct construction: phase one with BITS:0004 clear builds tuple
  WR6=0/ACC=0; otherwise it builds WR6=0x1803/ACC=1. On the ordinary route,
  RAM:000f27 bit six queues ACC=3/WR6=0x1106, RAM:000f26 bit six clear builds
  the zero tuple, and media state two plus phase three plus BITS:000b overrides
  a set RAM:000f26 bit by building the zero tuple. The remaining path queues
  ACC=3/WR6=0x1105. The scratch builder is ECALL-class, the countdown service
  is LCALL-class, and the function itself returns through RET.
- Firmware 1.04 RAM:f921f0 is a near Boolean phase-one classifier. RAM:000c3c
  bit zero clear enters completion-state handling; bit three of RAM:000f31
  immediately queues 0x1105, while RAM:000f27 bit six queues 0x1106 unless
  signed zero-extended RAM:000f24 is less than wrapping `(u16)RAM:000c3d-10`.
  The valid RAM:000f31 pattern requires bits seven/six clear and bit five set;
  invalid patterns call RAM:f93a5e before queuing 0x1105. Bit four of
  RAM:000f31 or bit six of RAM:000f30 selects descriptor 0x1804/mode one;
  otherwise the tuple is zero. RAM:000c3c bit zero set instead selects the
  0x1803/mode-one tuple only when BITS:0004 is set. All descriptor builds
  return true, all countdown-service paths false, and the function exits RET.
- Firmware 1.04 RAM:f93a5e is a near function ending in RET, and RAM:f921f0
  calls it with LCALL. Its existing C declaration/definition must retain
  `PLEXTOR_NEAR_FUNCTION`.
- Firmware 1.04 RAM:f9230d shares RAM:f921f0's direct BITS:0004 descriptor
  selection, RAM:000f31 bit-three 0x1105 rejection, and signed wrapping
  RAM:000f24 versus `(u16)RAM:000c3d-10` 0x1106 threshold. For invalid
  RAM:000f31 patterns, RAM:000f24 equal to one and RAM:0015d7 equal to zero
  enter the recovery sample: write five to RAM:000fca and RAM:018002 before
  reading RAM:018004..018007 into RAM:0015a4..0015a7, then write four to both
  mode locations. A wrapping eight-bit sum of the four snapshots equal to
  zero builds the zero descriptor and returns true; otherwise it queues
  0x1105 and returns false. Valid patterns choose zero versus 0x1804/mode-one
  exactly like phase one. The function is near and returns through RET.
- Firmware 1.04 RAM:f928ae accepts base descriptor status in WR6 and mode in
  ACC. Its active timing gate is entered only when BITS:0013 and BITS:001b are
  clear and RAM:0015d7 is nonzero. A positive signed RAM:000fa2 value is
  decremented before speed initialization and early return; zero underflows
  to 0xff and continues. The second gate compares zero-extended RAM:000f24
  against wrapping `(u16)RAM:000c3d - (BITS:000c89.0 ? 0 : 10)`.
- Firmware 1.04 RAM:f925be decrements RAM:000f24 with byte wrap, but its
  expiry and low-count thresholds are signed. BITS:0024 is a persistent
  source-three/status-0x1105 phase latch: it is set when the decremented
  countdown equals wrapping RAM:000c3d minus one, retained while its recovery
  predicates fail, and cleared on source/status mismatch or forced expiry.
  Descriptor construction reloads base status into WR6 and source kind into
  ACC; terminal completion sets BITS:0009 and clears BITS:0004.
- Firmware 1.04 RAM:fb9c4d uses CurrentOperatingCode 0x08, 0x04, or the
  default route to select its phase-zero index from RAM:00f965, RAM:00f964,
  or RAM:00f963. It reads the big-endian word two entries ahead in the table
  at RAM:ff92eb. Its independently reread phase-two path clears bit two of
  the big-endian word at RAM:000ff5 and resets RAM:000fed.
- Firmware 1.04 RAM:fb9b2c selects the big-endian calibration table at
  RAM:ff92eb below operating code twelve and RAM:ff9337 otherwise. Phase zero
  submits the word two entries ahead without advancing RAM:000ff4. Other
  nonterminal phases store the current indexed word at
  `RAM:00efa5 + 2 * phase`, submit it, and increment RAM:000ff4 with byte
  wrap; phase five also snapshots the word at RAM:000fd9 and calls
  RAM:fb9c19. Phase six stops capture, clears RAM:000ff5 bit one, and resets
  the phase. Its repeated phase and index reads are volatile and observable.
- Firmware 1.04 RAM:fa905b reads the big-endian dword RawReadRingSlotCount
  twice: its low word minus one becomes the queue index mask and its low word
  minus six becomes the high-water count. It publishes descriptor-controller
  address 0x000088bb, clears both ring indices and the main cursor, copies
  AuxiliaryC2RingBase to AuxiliaryRingCursor, then selects controller-record
  state one and raw-read Q type 0x0a.
- Firmware 1.04 RAM:fab60f short-circuits an expected-versus-captured
  three-byte packed-BCD target comparison in expected-byte/read-packet order.
  Descriptor mode codes zero through three select controller profile byte
  0x47; all other bytes select 0x40. Success sets BITS:0043, sets RAM:000fad
  bit four and mirrors the retained value to RAM:018003, then clears
  BITS:0034.
- Firmware 1.04 RAM:fd9afd reads ControllerBufferDescriptorModeCode once.
  Modes zero and nine subtract two from the 32-bit read-measurement target;
  modes eight and ten through thirteen subtract three. All other modes,
  including fourteen, publish status seven without reading the target.
  Earlier decompiler documentation that included mode fourteen was stale;
  the full selector instruction chain and exhaustive 256-byte tests prove
  the corrected set.
- Firmware 1.04 RAM:fd6b13 validates the staged main-firmware revision,
  header and checksum before disabling interrupts and invoking the relocated
  RAM:ff4000 flash programmer. The addressless native adapter supplies the
  canonical operation-three loader tuple; the C coordinator checks checksum
  bytes 0 through 0x79ffe, writes RAM:00fa = 0xff, and never performs a host
  hardware action in tests.
- Firmware 1.04 RAM:fde6ad initializes the Format Unit code-0x11/type-two
  controller-record pipeline. It reads the ring-slot count twice, stores the
  wrapping low-word minus-one and minus-six limits at RAM:00129a/00129c,
  installs descriptor address 0x000088bb, copies RAM:001562 to RAM:00126a,
  initializes geometry and lead-in state, and conditionally rebuilds geometry
  plus the active boundary table only when controller-record acquisition
  returns nonzero. Its return is always one.
- Firmware 1.04 RAM:fec05d treats OPC as required only when
  `IsOpcRequiredForWriteSpeed` returns exactly one. That route resets the
  raw-read transfer state, starts coroutine two at RAM:f84a00, publishes the
  pending flags/state, and cooperatively waits both for RAM:0015b2 to settle
  and for the freshly dereferenced pending-status byte to leave 0x11. It maps
  the pending result into sense data, returns zero on nonzero sense, otherwise
  marks OPC valid and updates the calibration speed. RAM:000c84 bit three is
  cleared at exit only when RAM:00fbf1 is zero.
- Firmware 1.04 RAM:f8d355 enters live parameter-FC calibration for media
  state two or when the indexed-state-five/eight predicates return exactly
  one. It polls controller port 0x80ac until its low seven bits clear, brackets
  calibration with channel-two and measurement-state reset/restore calls, and
  returns the calibration byte after a 4000-unit non-yielding wait. The stored
  route rereads the selected RAM:00fac0/00fac1 byte, maps 0xff to zero in
  RAM:00fc0f, writes FC for operating code four, and returns one.
- Firmware 1.04 RAM:fdea32 uses a signed comparison against frame 150.
  Below it, RAM:00127c is one, Q index is zero, and the relative packed-BCD MSF
  tuple is fixed at frame four; otherwise the state is zero, index one, and
  the relative position is target minus 150. Control/ADR is 0x71, TNO is one,
  the separator is zero, and the absolute tuple uses the firmware's
  negative-position plus-450000 rule.
- Firmware 1.04 RAM:fdbbca begins by disabling global interrupts, setting
  RAM:000fca bit three, and mirroring that value to controller port 0x8002.
  It stages ports 0x8005/0x8006/0x800e through zero and 0x08 before their
  0x82/0x80/0x88 values, conditionally clears RAM:00131c and port 0x8080 from
  RAM:001302 bit three, clears RAM:000fca bits three/four and mirrors it again,
  then restores EA/EC/SFR:A9.2. BITS:000e selects zero versus RAM:000fad bit
  seven for port 0x8003 before the final 0x802d, register-one, and 0x802b
  writes. These initial shadow and staging writes are absent from the concise
  Ghidra pseudocode summary but explicit in the instruction listing.
- Firmware 1.04 RAM:f88692 normalizes SecondaryOperatingCode 0xff to 0x28 and
  subtracts its unsigned-16 products by 0x41, 0x2d, and 0x14 from the signed
  target position. Each position wait must return exactly one, while the
  readiness and final-window predicates accept any nonzero result. Only a
  failed final-window predicate restores the read path with argument one;
  success returns zero and every failure returns two through near RET.
- Firmware 1.04 RAM:fedb7d copies the WRITE AND VERIFY starting LBA twice
  from RAM:2bc707 using the native low-word-first load order: once into
  RAM:000b02 and again when subtracting it from the exclusive endpoint at
  RAM:2bc70b for RAM:000b06. It temporarily makes RAM:000c88 bit seven equal
  RAM:000c8a bit two while the read/verify range executes, restores the prior
  bit only when a change was needed, then finalizes and exits ERET.
- Firmware 1.04 RAM:fd2635 is a void ERET READ CAPACITY (10) handler; the
  former unresolved scalar return was a decompiler limitation. Mode zero uses
  zero media capacity as one before the final decrement, otherwise rereads
  RAM:0012c1 and subtracts 150 before decrementing. Nonzero mode uses capacity
  0x400 when RAM:000c88 bit seven is set or calls the geometry worker. It
  emits the big-endian last LBA and zero-extended RAM:000b00 block size at
  RAM:000b2a, transfers eight bytes, and sets no-sense unless DATA IN returns
  code four.
- Firmware 1.04 RAM:f8b71f takes its word input in WR6, atomically sets
  BITS:0039, stores that word at RAM:00facf and RAM:00f96e, sets bit one of
  RAM:00f971, and exits through ERET. C251 can express BITS:0039 directly by
  fixing a `bdata` byte at RAM:0027 and binding an `sbit` to its bit one;
  `OPTIMIZE(8)` emits `SETB 039H` without the extra carry update seen at
  optimization zero.
- Firmware 1.04 RAM:fdcb43 is an ordinary one-argument ERET function, not an
  unavoidable DR4 assembly boundary. Each of its three callers explicitly
  loads the playback endpoint into DR4 immediately before ECALL, and C251's
  first `s32` parameter occupies DR4. The function stores that raw 32-bit
  value in big-endian order at RAM:000fc6 through RAM:000fc9.
- Firmware 1.04 RAM:fde7b0 owns REQUEST SENSE synthesis only when
  RAM:000b1a is zero. It maps a nonzero RAM:000fae status first, otherwise
  synthesizes 02/0401, then may override that with 02/0408, 02/0404, or
  00/0404 from controller/media state. It transfers at most 18 bytes from
  RAM:000b18 and clears sense unless DATA IN returns status four.
- Firmware 1.04 RAM:fae02a rereads the byte at RAM:00e74b for every condition
  of its response-buffer clear loop and clears offsets zero through
  `record_count * 8 + 0x22` inclusively. After the selected response builder,
  it reads the record count again to cap the transfer; a false recordable
  builder result returns immediately, while zero allocation length publishes
  no-sense without attempting DATA IN.
- Firmware 1.04 RAM:f8eb9b treats zero RAM:00f9e1 as expiry, writing 0xff and
  clearing RAM:001302 bit two plus RAM:000c84 bit three. A nonzero countdown
  compares the selected ten-byte TOC record position against RAM:001153; state
  0x1f/status three transitions when the track is at least 94 or the signed
  lead-out distance is at most 126000 frames, then decrements the countdown
  and sets both active flags.
- Firmware 1.04 RAM:f8ee55 receives the calibration stage in ACC. Stage-one
  accepts measurements outside the inclusive 0x60..0xa0 neutral window;
  stage two accepts signed words at or beyond -6144/+6144, and stage three at
  or beyond -8192/+6072. Outcomes below both retry counters three and six
  restart at RAM:f84b0b; exhausted outcomes resolve drive control and restart
  at RAM:f8f07c. Stage-five's stack-local configuration read aliases persistent
  byte RAM:ffc020.
- Firmware 1.04 RAM:fc64f2 rereads RAM:000fae three times on the nonzero
  producer-status path, scans type-nine descriptor markers inclusively from
  0xaa through 0xac, and advances RAM:001298 with the raw-read index mask.
  Ordinary valid descriptors return zero only at queue high water and 0xfe
  otherwise; invalid descriptors forward their pointed sense key while using
  the descriptor address as ASC/ASCQ.
- Firmware 1.04 RAM:fc5cd8 treats RAM:001296 as signed for its greater-than-30
  producer gate, captures 36 bytes at main-ring cursor plus twelve, selects a
  repeated-header offset of eight only when bytes four through seven repeat,
  and searches at most 100 sixteen-byte steps for RAM:000f65. A match rewinds
  the cursor by one and captures either sixteen or twenty-four bytes including
  the marker.
- Firmware 1.04 RAM:f95c1c consumes an inherited Boolean in R15 only on its
  unhandled-mode fallback. An addressless entry adapter can capture R15 before
  the C body clears RAM:00ef80; C251 then emits local calls to all four native
  evaluators and returns the normalized Boolean through ACC with near RET.
- Firmware 1.04 RAM:f95789 and RAM:f95837 compare decoded packet positions as
  signed 32-bit values after wrapping target additions. Both perform separate
  volatile read-modify-writes for their two capture flags. RAM:f95837's saved
  R15 is only its locally initialized completion flag, not an inherited input;
  it returns whether a terminal measurement status was already present.
- Firmware 1.04 RAM:faeef2 receives its far output byte address in DR0 and
  returns 0/1/0xfe in ACC through near RET. Capture DR0 before any C operation.
  A clear descriptor status byte bypasses validation and is accepted; a
  nonzero status byte requires the validator. The producer-error path rereads
  RAM:000fae independently for the two-byte ASC table and the sense-key table.
- Firmware 1.04 RAM:faee30 indexes six-byte binary-MSF tuples from RAM:00f2b9,
  maps a converted zero position to frame 150, clears P3.0/P3.1 and four raw
  mode bits, and initializes one requested block before either positioning an
  active raw-read pipeline or restarting coroutine two at RAM:f90000. It polls
  RAM:faeef2 through a one-byte output pointer, yielding on every 0xfe; terminal
  failure restarts coroutine two at RAM:f84a00, dispatches request five, yields,
  and returns 0x0f. C251 passes the explicit output pointer in DR4, so a narrow
  addressless DR4-to-DR0 adapter preserves the legacy poll contract.
- Firmware 1.04 RAM:faecfc builds READ TRACK INFORMATION from the Full TOC
  using signed frame-position comparisons, preserves the native near RET
  boundary, and calls the already-C track lookup at RAM:faef6d through its
  proven near interface. Its former 308-byte same-address assembly body is no
  longer part of the reconstruction.
- Firmware 1.04 RAM:faeffa inherits DR12 only for its unrecognized write-type
  fallback. An addressless entry capture makes that value explicit before the
  C body evaluates the recognized branches; exhaustive write-type/branch and
  packet-cursor vectors cover the reconstructed near RET function.
- Firmware 1.04 RAM:f9de74 receives its radial coordinate in ACC and inherits
  the low-speed selector in R15. At write speeds of at least 24 it instead
  maps the coordinate and big-endian RAM:00f9fc through the native float
  helpers, clamps to RAM:001058..001059, and forces 0x10 for controller types
  two or three when RAM:00fb9b is zero. Addressless capture and float-call
  adapters preserve the local ABI while the canonical 77-byte body is C.
- Firmware 1.04 RAM:f9de23 changes the secondary controller profile only when
  RAM:00103a bit seven is set and the selected profile differs from RAM:00fa03.
  It applies the profile, conditionally interpolates through BITS:0038, writes
  parameter 0xf049 only at write speed ten for profiles at least eight, then
  stores the selector to RAM:00fa03, RAM:000e51, and RAM:000e4f in that order.
  The decompiler's apparent disabled-path rewrites are not present in the
  canonical instructions.
- Firmware 1.04 RAM:f9dd92 divides the signed big-endian position at
  RAM:00103d by 4500 and uses the quotient's low byte plus inherited R15 to
  select the media-state-five controller profile. Profiles at least 0x14 use
  an index reduced by 0x14; profile 0x10 substitutes RAM:00ef85/00ef87 after
  the normal table loads. The inactive code-0x14 path delegates bound
  computation, while other inactive codes restore those fixed words. Its sole
  caller immediately clears ACC, so the function has no meaningful scalar
  result despite the decompiler's residual-byte return.
- Firmware 1.04 RAM:f9dd3d clears RAM:00103a bit seven, restores it for
  controller type one only when the operating code is not ten and RAM:000c84
  bit five is clear, or for other controller types at operating codes 24 and
  above, then writes the state once. An enabled result calls near RAM:f9d45c;
  C251 emits that LCALL and the canonical ERET, so the former target assembly
  duplicate is unnecessary.
- Firmware 1.04 RAM:fcd87e copies 0x200 four-byte units, not 512 bytes. Its
  destination is RAM:104009 plus bank*0x4000 plus page*0x800, and its observed
  bank/page callers bound both indices to zero through seven. Native inputs
  are DR0/ACC/R7/R6; C251 uses DR4/ACC/R3/R2 for the equivalent C prototype,
  so narrow entry captures and an addressless C-caller bridge are required.
- Firmware 1.04 RAM:fde8d8 inherits B, DR4, and WR2 from the host-interface
  interrupt dispatcher. Capture them in B/DR4/WR2 order because returning WR2
  through WR6 would overwrite half of an uncaptured DR4. The two WR8 pointer
  loads immediately before its DR4/DR0 comparison have no data consumer but
  are observable volatile reads and remain in the C body.
- Firmware 1.04 RAM:feca71 initializes write-command raw-read state with an
  exact 39-write trace. It raises RAM:018021 to 0x20 while copying
  RAM:000c71..RAM:000c74 to RAM:018025..RAM:018028, lowers the gate to zero,
  derives RAM:001288 from RAM:00dd36 bit five, and conditionally loads ISRC
  characters before publishing Q type 0x0a and initializing the transfer
  layout. Its helper boundaries are mixed: RAM:fe2c5a, RAM:fe3277, and
  RAM:fcff2e use ECALL, while RAM:fe2c81, RAM:fe2e40, RAM:fe0ff8, and
  RAM:fe0f77 use LCALL and return through RET; the outer function returns
  constant one in ACC through ERET.
- Firmware 1.04 RAM:f90b91 performs its queue-capacity comparisons as unsigned
  16-bit arithmetic after subtracting six from the low word of the 32-bit slot
  count. BITS:000e selects threshold 128; otherwise RAM:000f46 selects 16 or
  64. It rereads queue/slot state after every coroutine-two yield, rereads
  RAM:000f46 before optional speed initialization, and rereads current speed
  before restoring RAM:000e49. The near boundary preserves inherited DR12;
  addressless capture/restore adapters let the canonical body remain C.
- Firmware 1.04 RAM:fc8c63 indexes its 36-word transition-countdown matrix at
  RAM:ff952c as `applied_class * 12 + current_class * 2`. It retains the
  initial signed-positive countdown, compares half of that value against fresh
  RAM:001200 reads, counts qualifying P0.7 highs to 100 and P1.CEX0 lows to 20
  independently, and yields coroutine one until a counter or countdown ends
  the wait. A downward transition arms P1.1/P1.0 immediately; only current
  operating code one permits its P0.7 highs to accumulate. The final P1 set is
  unconditional, the function returns through RET, and inherited DR12 must be
  preserved.
- Firmware 1.04 RAM:faa80c receives bank/page in ACC/R7 and copies exactly
  0x800 bytes from controller offset `0x94000 + bank*0x4000 + page*0x800` into
  fixed CPU workspace RAM:2ba62f. Bank values zero through seven replace the
  base; every other value retains inherited DR12 as the base. RAM:fde0a6's
  instructions prove DR4 is the controller-relative source and DR0 the CPU
  destination, correcting the historical function-name/decompiler prose.
- Firmware 1.04 RAM:f849f3 is a pure native-frame wrapper: it computes binary64
  ceiling by ECALLing RAM:fd924b negate, RAM:f84a14 floor, and RAM:fd924b
  negate again, then ERET. C251 reproduces that four-instruction shape from a
  void C wrapper without materializing or altering the DR0:DR4 operand frame.
- Firmware 1.04 RAM:fc46c4 reads RAM:001153, then RAM:00dd3d, refines the
  boundary, and rereads RAM:001153 both before the three-offset probe and on
  the status-one store path. Statuses 4/7 select working-track type six and
  RAM:00dd51, status 6 updates RAM:00dd45 from RAM:000f41, status 5 returns
  zero, status 1 updates RAM:00dd45 from the fresh lead-out, and every other
  status returns inherited R15. A first-operation addressless capture makes
  the complete ordinary near function representable in C.
- Firmware 1.04 RAM:fc471d shares RAM:fc46c4's initial/refined probe sequence
  and inherited-R15 default. Its status map differs: status 1 stores a fresh
  RAM:001153 value to RAM:00dd45, statuses 4/6/7 store RAM:000f41 to
  RAM:00dd51, status 5 returns zero, and all other statuses return inherited
  R15. The same first-operation capture adapter makes this near body C-safe.
- Firmware 1.04 RAM:fd7bf3 treats its ACC terminal-handoff byte as enabled only
  when it equals one. Layouts 0x0f, 0x10, and 0x18 select the observed fixed
  track/position pairs; terminal nonzero cursors set media coroutine state
  three and return one. Unsupported layouts leave RAM:00fbf8 unchanged and
  publish inherited R15 as both cursor and target track before clearing
  BITS:0018, setting the 750-frame span, and initializing coroutine zero at
  RAM:fd4f8e. A first-operation R15 capture makes the body C-representable.
- Firmware 1.04 RAM:fbf39a is a near controller-buffer bank-stride diagnostic:
  ACC supplies the starting pattern and receives normalized Boolean success,
  and the function returns with RET. It reads the big-endian reserved-region
  base directly from RAM:00155a, computes the low byte of
  `(0x1b0000 - base) >> 15`, writes `pattern + index` at each 0x8000-byte
  stride through RAM:fbf736, then rereads through RAM:feea1c and rejects the
  first mismatch. The C251 near qualifier preserves the ACC/RET contract.
- Firmware 1.04 RAM:f8f342 accepts an unsigned word in WR6, converts it to
  float32, divides by the exact 32767.0 bit pattern, calls near
  RAM:f8f38b NaturalLogFloat32, divides by the firmware's float32 ln(10)
  approximation, multiplies by 2000.0, and converts to signed DR4 through
  ERET. An ordinary C251 float expression in that explicit statement order
  emits the same runtime helper classes and constants. RAM:f8f38b is also a
  near-C implementation of the firmware logarithm policy and approximation.
- Firmware 1.04 RAM:f99a34 is an ERET READ TOC/PMA/ATIP dispatcher. It first
  validates mask descriptor RAM:ff8dd3, then uses CDB byte two's low nibble as
  the primary format. Format zero additionally maps CDB byte nine bits 7..6:
  0 selects formatted TOC, 0x40 multisession, 0x80 full TOC with ACC one, and
  0xc0 unsupported. Formats one through five select multisession, full TOC
  with ACC zero, PMA, ATIP, and CD-TEXT respectively; CD-TEXT receives the
  big-endian CDB allocation length at RAM:000b11. Formats zero through four
  are near calls while validation, rejection, length read, and CD-TEXT are
  extended calls.
- Firmware 1.04 RAM:fd8b16 reads controller port 0x849f, divides the byte by
  four, and returns a piecewise radial-pulse displacement in ACC through RET.
  Direction zero maps samples below/above 0x20 to `0x20-sample` and
  `0x60-sample`; every nonzero direction maps them to `sample+0x20` and
  `sample-0x20`, with equality returning zero. The branch-specific order of
  RAM:0010be/RAM:0010bf writes is observable, and both shadows are freshly
  read before publication to controller ports 0x8494 and 0x8495.
- Firmware 1.04 RAM:fd3d0a is the shared ERET READ VERIFY executor used by
  VERIFY(10), VERIFY(12), WRITE AND VERIFY readback, and full-media recovery.
  BITS:000a selects whether the signed command LBA is preserved at RAM:0015dc
  and mapped before a wrapping subtraction of 150; otherwise BITS:000c is
  cleared. After range validation it starts coroutine two at RAM:f90000,
  publishes the reset target, yields coroutine zero for each 0xfe poll result,
  then restores coroutine two at RAM:f84a00 and the P3.0/P3.1 handshake pins.
- Firmware 1.04 RAM:f93ace advances RAM:00126e by one 0x930-byte main-data
  slot and subtracts RAM:00155e once when the new cursor reaches or exceeds
  the ring size. Its RAM:000f1f post-decrement gate is a signed-byte `< 3`
  test, so wrapped values 0x80..0xff enter the check. The block Ghidra marks
  unreachable at RAM:f93b71 is the real nonzero `expected % 0x930` recovery
  branch; aligned addresses instead compare a fresh cursor with expected.
- Firmware 1.04 RAM:fd6fe0 is a near three-valued raw-sector host-transfer
  poll: zero means terminal failure/unavailable work, one means completed, and
  0xfe means pending. Producer errors are read three times and index the key
  table at RAM:f84454 and big-endian code table at RAM:f84468 before wrapping
  command-start plus completed-block information is reported. Mode three
  publishes the validated batch count, prepares RAM:00145d's block size,
  starts the secondary-cursor transfer, and yields until RAM:000f47 clears.
- For stock 1.04 raw-range integration, RAM:fee4d5 takes the coroutine-2
  entry address in DR0 and returns with ERET; C251 passes one `u32` argument
  in DR4, so a bridge must move DR4 to DR0. The raw transfer poll RAM:fd6fe0
  returns with RET, requiring an FD-bank LCALL/ERET bridge for a far C251
  caller. These contracts are confirmed by canonical disassembly and the
  linked `gofw-pxw5224a` Keil listing; they are function-specific.
- Stock 1.04 alternate raw coroutine RAM:f929ab has no entry prologue; it
  calls its producer step RAM:f92a69 with near LCALL, and that step returns
  with RET. Its finalizer RAM:f92815 does not return under normal execution.
  An FD-bank far caller cannot directly call the F9-bank near step: it needs
  an F9-bank LCALL/ERET gateway. Stock RAM:f9ffd6..f9ffff is an erased
  42-byte gap after the preceding function ends at RAM:f9ffd5; canonical
  Ghidra has no function or xrefs at the gap. The standalone C4 image uses
  only RAM:f9ffd6..f9ffd9 for the gateway and guards the original FF bytes.
- Firmware 1.04 RAM:fd3eaf is the ERET one-sector READ HEADER acquisition
  executor. Its validation setup clears BITS:000c/001d/001c/0013/001b and sets
  BITS:000d; a failed range validation returns with BITS:000d still set, while
  acceptance clears it and forces CommandTransferBlockCount to one. Existing
  pipeline mode two is repositioned at the reset target; other modes reset and
  start coroutine two at RAM:f90000. Only terminal poll status zero performs
  idle-coroutine recovery at RAM:f84a00; every accepted terminal status restores
  RAM:001226 and P3.0/P3.1.
- Firmware 1.04 RAM:fcbcde is an ordinary no-input ERET WRITE AND VERIFY
  handler. It sets BITS:0002 before selecting the transfer-length layout;
  invalid reserved fields and a zero result from RAM:fcbe46 intentionally
  return without clearing that bit. Accepted paths treat a fresh RAM:000c8a
  bit-six result or unsigned RAM:001292 below 0x0249 as no-sense completion;
  otherwise they execute RAM:fcc246 and only a nonzero result invokes
  RAM:fedb7d plus fresh RAM:000c85 bit-zero and RAM:000c84 bit-three clears.
  C251 preserves the DR4 start/DR0 count and DR4 count call tuples directly;
  C251 `sbit` helpers implement the atomic BITS:0002 transitions.
- Firmware 1.04 RAM:fcbc19 is a call-free no-input ERET READ/VERIFY finalizer.
  It samples RAM:000c9e once: zero is a no-op; one and two share the
  RAM:2ba5b1/RAM:2ba5af descriptor gates; three uses only the signed
  RAM:000c89 gate; every other value fail-stops in a self-loop. Primary paths
  set RAM:000c89 bit three and choose 20/60 from RAM:00e750 bit zero;
  secondary paths set RAM:000c8a bit zero and choose 10/60 from RAM:000c8b
  bit three. Ordinary C251 emits both ERET and the explicit self-loop without
  any assembly adapter.
- Firmware 1.04 RAM:fa31f6 is a call-free ACC-result ERET FORMAT UNIT code-11
  parameter parser. A zero big-endian dword at RAM:000b2e selects offset eight;
  otherwise offset four is used. Prefix bytes are masked by firmware table
  RAM:ff8e8f (`ff 5d ff 00 ff ff ff ff`), RAM:000b2d must equal eight, the
  format byte divided by four must equal 0x24, and the preceding sentinel at
  RAM:000b2a plus the offset must equal 0xffffffff. The following three bytes
  form an unsigned count: zero and one succeed, one sets RAM:001586, and larger
  counts return 0xff after RAM:000c88 bit six has already been set. Successful
  paths set RAM:000c9e to two and return the count.
- Firmware 1.04 RAM:fccd59 is an ERET controller-record boundary-table
  builder. Mode three uses byte count RAM:2ba588; other modes scan 64 unsigned
  big-endian thresholds at RAM:f84690 for the first value greater than
  RAM:2ba59f. If none matches, native R7 remains the count, so exact C needs a
  first-operation inherited-R7 capture. Entries before the last use the next
  RAM:f84490 dword minus five, the last uses RAM:2ba58d, and RAM:2bae2f always
  receives table entry zero minus five even when the count is zero.
- Firmware 1.04 RAM:fcda9d is the MIP Type-2 variant of the controller-record
  boundary-table builder. It reads RAM:000c9e once for the mode-three gate and
  again on the non-three path for a mode-one-only threshold scan. Mode three
  uses RAM:2ba588, mode one selects the first RAM:f84690 threshold greater than
  RAM:2ba59f, and every other mode plus a mode-one no-match retains inherited
  R7. Its RAM:2bae33 and RAM:2bae2f writes otherwise match RAM:fccd59.
- Firmware 1.04 RAM:fa9910 maps Type-2 slot `i` to controller offset
  `0x94004 + (i << 14)`. It imports the big-endian dword through the
  controller's plus-1-MiB CPU alias, increments with 32-bit wrap, mirrors the
  result at RAM:2ba584 and RAM:2ba633, and copies RAM:2ba633 back to that same
  controller offset.
- Firmware 1.04 RAM:f89dda reduces write speed only when its ACC enable gate
  is nonzero. Media mode one/state six forces 8x at current speed 16x or
  above and publishes RAM:00fad7 bits two and twelve in two separate
  read-modify-write operations. Other mode-one states select the lowest
  enabled speed from bits twelve through five and suppress overwriting the
  saved prior speed once bit two is already set. Media classes two and three
  reject coroutine state six and current speeds below 24x/32x respectively;
  strategies d7/d4 choose 20x, d8 chooses 16x, e2/e3/e4 choose 24x, and
  those special strategies clear RAM:00103a bit seven.
- Firmware 1.04 RAM:fb0bac treats persistent configuration range 0x6c as an
  18-byte big-endian image. Bytes 12..13 are an unsigned sample count accepted
  when `0xffff` or below 10000; bytes 14..17 accumulate RAM:0014d6. If the
  word at bytes 4..5 is not `0xffff`, it becomes `0xffff` and bytes 0..3 are
  reset before accumulating RAM:00fbed. DriveControlFlags1302 bit zero also
  refreshes image byte 7 from RAM:00faea, bytes 8..9 from RAM:000c3a and byte
  10 from RAM:00fb5d. Any pending attempt clears that flag and RAM:0014d6 even
  when count 10000 or above suppresses persistence; RAM:00fbed is retained.
- Firmware 1.04 RAM:fb0a9c always applies the active-drive maintenance flag
  check and ends with RAM:fb0bac. DriveControlFlags1304 bit five first writes
  12 bytes from RAM:00fbda to configuration offset 0x60. Under media mode one,
  RAM:001308 zero or drive state 0x13 writes three bytes from RAM:00fabd to
  offset 0x29; drive state 0x1f writes RAM:00f9b5 to offset 0x3c. Independently,
  RAM:001308=1, RAM:00fab1=1, and drive state other than five writes 26 bytes
  from RAM:00fab4 to offset 0x20. A zero range-update result invokes drive
  state ten, sets RAM:0015c4, and restarts coroutine one at RAM:f853cf.
- Firmware 1.04 vendor opcode ED rejects nonzero CDB bytes 2..7 or byte 10.
  CDB byte 1 bit four enables option mutation: bit zero sets RAM:00fad7 bit
  zero, while clear removes it and, when bit two was active, also clears bit
  two and restores both current and selected speed from RAM:00fad3. Its
  eight-byte response at RAM:000b2a carries length six, option state in the
  high byte at RAM:000b2c, and a CD-speed word at RAM:000b2e. Speed selection
  is enabled only for RAM:000fae zero and nonzero RAM:001008; transfer length
  is min(CDB allocation length, eight), and DATA IN result other than four
  invokes SetNoSenseData.
- Firmware 1.04 RAM:fc766a is a call-free ERET page-copy leaf. It reads the
  big-endian dword at RAM:2ba51c low word first, adds 0x10 to its high word
  (the controller-buffer +0x00100000 CPU mapping), and copies 256 consecutive
  six-byte records to RAM:2b971c. Source offset addition carries through the
  full 24-bit address rather than wrapping only the low 16 bits.
- Firmware 1.04 RAM:fadcba is a call-free WR6-result ERET lookup. A positive
  RAM:001235 frame position is reread and its low 24 bits are divided by 75
  and then 300, producing five-minute buckets; buckets below six return zero,
  while later buckets subtract six and clamp at ten. The strategy-row selector
  is read at `base + ((RAM:00f05f + 1) * 11 - 1)`, with bases RAM:f833bd for
  RAM:00fb88 values two/three, RAM:f832b5 for write speed ten, RAM:f83084 for
  RAM:00fb80 value two, and RAM:f831ad otherwise. A nonzero selector chooses
  an eleven-byte row at RAM:f82320, and the returned byte is zero-extended.
- Firmware 1.04 RAM:fc87fe is the call-free ERET read-acquisition speed
  fallback. It rejects RAM:000c89 bit zero, speeds at most four, retry state
  above five, and disallowed enabled-controller/result/coroutine combinations.
  Valid reductions are exactly 52->40->32->24->8->4; any other admitted speed
  fail-stops. ACC must equal exactly one to force 4x and RequestedOperatingCode
  four. It publishes the final speed to RAM:000e49/RAM:000f45, saturates retry
  state to five for an enabled controller or recovery count at least five, and
  sets RAM:001305 bit two.
- Firmware 1.04 RAM:fb87de is the call-free near PMA completeness predicate.
  With no last TOC track it returns true and sets RAM:001132 bit seven when
  PMA start is earlier than ATIP lead-in minus 0x3df or any ADR-2..6 state is
  retained. With tracks present it accepts each ADR-1 slot when either the
  ten-byte PMA record exists or the corresponding six-byte decoded-TOC point
  is neither zero nor 0xaa, then checks each one-based counted ADR-3..6 slot.
  ADR-1 failure and tracked-disc success clear bit seven; counted-table failure
  returns false without modifying that bit.
- Firmware 1.04 RAM:fadb03 is a call-free ERET operating-adjustment lookup.
  MediaStrategyRecordIndex below nine, nonpositive frame positions, the first
  45,000-frame bucket, and SecondaryOperatingCode 0x28 or 0x20 all return zero.
  Later positive positions select bucket `min(frame / 45000, 8) - 1` and row
  `MediaStrategyRecordIndex - 9`; the byte offset is `row * 8 + bucket`.
  Codes 0x18, 0x14, and 0x10 select tables RAM:f82a50, RAM:f82808, and
  RAM:f825c0 respectively, while every other code selects RAM:f82378.
- Firmware 1.04 RAM:fa6765 computes the paired controller operating-profile
  adjustment words. In media mode one, RAM:001281/write type/BITS:0033 may
  latch BITS:0039; an unset latch aborts after clearing RAM:0010b4. Admitted
  codes below 0x34 add 18 times RAM:fadb03's frame-bucket value. When
  RAM:00f971 bit one is clear, codes 0x34/0x28/0x20/0x18/0x14 select the five
  words at RAM:00e752..00e75a and code 0xff selects the indexed word table at
  RAM:00eeaa; a nonzero value is multiplied by RAM:00f9e2 and divided by
  10,000 before RAM:00fad1 is added into RAM:00facf. The paired outputs are
  RAM:00fbc3/RAM:00fbc5. Other media modes use RAM:fadcba plus RAM:00ef81 and
  clamp only signed-positive values above 126, leaving wrapped negative words
  unchanged.
- Firmware 1.04 RAM:f97d5a is a near WR6-result duration selector driven by
  RAM:00fb82. Selector values 0, 2, 4, 6, 8, 10, 12, and 14 return 500, 600,
  750, 900, 1100, 1350, 1650, and 2000 respectively. Every other value leaves
  incoming WR6 unchanged; the sole caller does not seed WR6 immediately before
  the call, so that fallback is part of the observable function-local ABI.
- Firmware 1.04 RAM:fd365b classifies a READ CD start/end frame range. A
  nonzero RAM:000c9e returns class one immediately; otherwise modes other than
  three clear BITS:0010. It begins with lead-in track bounds, uses valid parsed
  TOC/PMA bounds when RAM:001008 is nonzero, and returns class two for the
  incomplete Full-TOC flag pattern `(RAM:0012be & 0x13) == 1`. An ordered
  signed prior start and in-range RAM:0015e9 can replace the first track. It
  publishes the new start at RAM:0015e5, returns class two when the start
  matcher with bias 0x96 succeeds, and otherwise invokes the end matcher with
  bias 0x95 before returning class one regardless of that second result.
- Firmware 1.04 RAM:fc691d acquires one PLEXTORPREMIUM raw-sector batch. It
  clears BITS:001d/001c/0013/000c, resets the queue, starts coroutine two at
  RAM:f90000, and publishes the signed start plus 150 at RAM:001598. Each wait
  iteration yields before comparing RAM:001292 to the requested count; a
  matching count still requires a fresh BITS:0008 producer-active check. On
  success it returns the big-endian main-buffer dword at offset four of the
  consumer descriptor; otherwise it returns 0xffffffff. Both outcomes service
  pending host work, reset the queue, restore coroutine two at RAM:f84a00,
  dispatch request kind five, and yield once before returning.
- Firmware 1.04 RAM:fd6936 validates the PLEXTORPREMIUM signature sector. It
  requires the media/TOC gates, compares acquired offset 0x10 with the
  fourteen-byte marker at RAM:ff9608, wrap-sums 0x400 big-endian words from
  acquired offset 0x940 against the word at offset 0x1e, and compares sixteen
  bytes at offset 0x24 with the initialized INQUIRY product-identification
  field at RAM:00dcd5. That last field is not an erased marker.
- Firmware 1.04 RAM:f8e560 wrap-sums sixteen signed words at RAM:00100a,
  signed-divides the resulting 16-bit word by sixteen, and publishes that
  quotient at RAM:00102a before replacing it with the low word of the existing
  83.5 scale. The original floating chain `ceil(quotient - 0.5)` is exactly
  the integer quotient. CurrentOperatingCode four with RAM:001008 equal to one
  additionally truncates a positive 0.9 scale; BITS:003a then adds the
  zero-extended RAM:00f970 byte with 16-bit wrap. Each stage is separately
  written back to RAM:00102a.
- Firmware 1.04 RAM:faf7ba has a native no-match edge that reloads its result
  from uninitialized stack words when RAM:000c85 bit two is set and no
  qualifying raw track/Q record exists. Do not translate that edge to
  ordinary C as if the value were defined. Valid raw-track state is treated as
  requiring a qualifying ADR-1 record; the C implementation documents that
  invariant and uses the current track start only as a defensive fallback when
  it is violated.
- Firmware 1.04 RAM:fafe24 handles REPAIR TRACK. It validates mask
  RAM:ff8d2c plus current write/media eligibility, rejects a requested track
  below RAM:00dd7e or above RAM:00dd7f, and resolves 0xff to RAM:00dd7f unless
  that byte is sentinel 100. After clearing and loading working-track state it
  requires `(RAM:00dd36 & 0xd0) == 0x50`. Types five and six finalize directly;
  type seven first reports 02/3004; every other type reports no-sense and
  returns. Finalization temporarily sets RAM:000c84 bit three and clears it
  with a fresh read afterward. The RAM:fc3731 loader consumes inherited R7,
  which the C caller must capture before its validation calls.
- Firmware 1.04 RAM:fcee18 updates controller word 0x035f only while
  RAM:001006 is zero, the operating code is at least 0x18, and either progress
  or the applied code changed. It clamps RAM:00125c/5 to 15. Codes
  0x18/0x20/0x28/0x34 select companion words at RAM:000ceb/ced/cef/cf3 and
  profile tables RAM:ff904e/905e/906e/907e; other admitted codes retain the
  inherited WR12 controller word and R15 profile. The child calls require
  WR4=word/WR6=0x035f and ACC=profile/R7=bucket/R6=1, after which a fresh
  RAM:00125c read is copied to RAM:00125d.
- Firmware 1.04 RAM:fba73c positions at the saved next-session boundary plus
  150 frames with 32-bit wrap and holds RAM:000ed8 only across the positioning
  call. Media state zero succeeds exactly when ParsedQStatusFlags bit two is
  clear. Other media start signed countdown RAM:001203 at two; non-state-two
  paths require an exact-one ten-sample pulse result, while state two rearms
  Q capture, treats 0xfe as pending, yields after each admitted pending result,
  resets its consecutive-failure counter on any nonzero pulse result, and
  fails on failure 21. A terminal packet succeeds unless RAM:001305 bit seven
  is set.
- Firmware 1.04 RAM:fccc53 takes its signed target in DR4 and mapping mode in
  ACC. Mode one publishes the target directly; mode two uses the track-packet
  mapper and every other mode uses the alternate-record mapper, publishes the
  mapped value minus 150 at RAM:000b02, then replaces the acquisition target
  with RAM:fee8b0's reset result. Nonnegative targets clear state/pins, may set
  BITS:0002/001b when RAM:000c9e is two and RAM:2ba5af is zero, launch
  coroutine two at RAM:f90000, yield before every RAM:fee0a0 poll until the
  result differs from 0xfe, reset the pipeline with mode two, wait for
  RAM:000e40 to clear, conditionally clear both active bits when BITS:001b is
  set, publish state four, restore both pins and return the terminal status.
- Firmware 1.04 RAM:faf878 handles RESERVE TRACK. It rejects write types two
  and three, normalizes write speed before the ordinary write/media gates, and
  indexes the tail track record at RAM:00dd87 + 19 * RAM:00dd7f. Track type
  one reports 05/7203 and every other nonzero type reports command-sequence
  error. The native multiplication leaves its high product byte in B for the
  RAM:fc3731 loader; the C reconstruction supplies that byte explicitly. On
  success it validates the PMA start, clears the two far staged-record counts,
  consumes the big-endian dword at RAM:000b0f, builds an ADR-1 record, and
  scopes RAM:000c84 bit three around raw-read preparation with fresh reads.
- Firmware 1.04 RAM:faf946 performs RESERVE TRACK raw-read preparation. It
  always resets the raw-read queue, starts coroutine two at RAM:f84a00 and
  yields once. Write-option bit zero bypasses OPC; otherwise OPC success is
  required. After the preparation call it clears both staged-Q counts before
  branching on the saved result. Success maps write-type one/type-zero tracks
  to types two/three, sets the type-zero end to start minus five, conditionally
  updates and snapshots the track table, then clears working-track state.
  Failure copies ASC/ASCQ to RAM:000c3a and, only for write type zero with
  0x0c07 or 0x0200, marks type five and snapshots RAM:001484 into both working
  cursors before updating the track table.
- Firmware 1.04 RAM:faface handles SEND OPC INFORMATION. Its parameter-list
  length is the big-endian word at RAM:000b11, must be at most 32 and a
  multiple of eight, and is copied to RAM:00e72b; DATA OUT result four owns
  completion. It stores the record count at RAM:00e74b. CDB byte-one bit zero
  requests OPC unless write-option bit zero is set. With retained calibration
  outside state two, measured class values 96..100 report 01/7301 and values
  at least 101 report 03/7302. An admitted run resets the raw-read pipeline,
  starts coroutine two at RAM:f84a00, yields, clears calibration validity and
  scopes RAM:000c84 bit three around the OPC worker.
- Firmware 1.04 RAM:fafbd0 begins OPC by setting P3.0 and clearing P3.1. A
  nonzero RAM:000fae maps directly through the key table at RAM:f84454 and
  word table at RAM:f84468. Otherwise it starts read acquisition state six,
  waits for RAM:000e40 to clear, maps a nonzero RAM:000e41 through the status
  sense helper, then services/yields until RAM:0015b2 leaves one and
  RAM:00faab leaves 0x11. Result 0x12 marks calibration valid and classifies
  RAM:000e47. Other results clamp the failed speed to 0x10, set RAM:001302 bit
  zero and copy the result-indexed word at RAM:f84416 to RAM:000c3a.
- Firmware 1.04 RAM:fb0000 performs startup input debouncing. P1.7 is checked
  for exact Boolean one, then rechecked after a signed-positive ten-tick wait;
  either success sets RAM:001301 bit five. Otherwise it clears P1.0/P1.1 with
  controller word 0x0252, starts the P3 pattern, applies configuration state,
  and loops while debounced P1.5 is low until state flag 0x11ef.5 is taken or
  state one in mode 0x0e/0x0b has RAM:00fae2 at most 200000. Qualified modes
  then wait 30 or five signed ticks respectively, wait for P1.6 release and
  increment RAM:00fae2 with 32-bit wrap.
- Firmware 1.04 RAM:fb00ee is a no-input near-RET controller probe; both
  callers use LCALL, and saved R13/WR14 are private rejection/sample scratch
  rather than inherited inputs. It asserts P1.1/P1.0, writes controller ports
  0x8006=0x80, 0x80a8=0 and 0x80cc=0x12, waits 8,000 non-yielding units, then
  samples the unsigned big-endian 0x80a9/0x80a8 word while signed RAM:001221
  remains positive. Values at least 0x5000 reject; port 0x80cc bit four also
  rejects a final value at most 0x1000. Acceptance publishes RAM:000cf5=
  0x00014ccc and RAM:000cf9=0x0640, sets RAM:001300/RAM:00fbec to one,
  reasserts both P1 bits and resolves drive control to zero.
- Firmware 1.04 RAM:fb0226 is a no-input ACC-status near-RET state-nine drive
  controller; its sole wrapper uses LCALL, and saved R13/WR14 are private
  constants six/fifty. It sets BITS:003b and RAM:001208=50, then checks signed
  expiry and debounced P1.7 before each yield. Fresh unsigned countdown reads
  select controller word 0x4800 for values 33..44 and 0x6f00 at values at most
  32; values at most ten abort when RAM:001302 bit one is clear. Only an
  exact-one RAM:0011ef bit-five helper result enters the 1,200-unit settling
  exit. Status zero clears BITS:003b on expiry, status one retains it while
  clearing RAM:001302 bit one, and both status-two exits clear it and set
  ConfigurationWordFAAC bit eight.
- Firmware 1.04 RAM:f89a08 maps write speeds 1, 4 and 8 to sign-magnitude
  calibration bytes RAM:00fadc, RAM:00fadd and RAM:00fade; speeds 16, 20, 24,
  32, 40 and 52 share RAM:00fadf. Calibration state two subtracts and state
  three adds with byte wrap. The adjustment is one when big-endian
  RAM:00faac bit one is set and two when it is clear. The function deliberately
  reads the current write speed again before choosing the destination slot.
- Firmware 1.04 RAM:fe9d02 rejects write type two before initialization and
  checks it again after building the packet state. For write types zero and
  one, alternate-range value exactly one replaces RAM:001598 with RAM:00dd51;
  otherwise the selected signed range may be subtracted from RAM:00dd3d or
  the current RAM:001598 under RAM:0014b3/14b4/14b6/14b7 gates. The saved R15
  is the retained ACC alternate-range byte, not an inherited input, and is the
  fallback class supplied to the sector-class selector.
- Firmware 1.04 RAM:fb4fa0 builds a synthetic type-four track-table entry at
  RAM:00dd93 from PMA ADR-1 record one at RAM:2bb045. Its seven copied bytes
  map source offsets 0,7,8,9,3,4,5 to destination offsets 0..6, while
  destination offset seven is constant four. It copies the big-endian
  RAM:001153 lead-out value to RAM:00dd69, zeros RAM:00dd6d and RAM:001455,
  invokes the tail-bound, controller-word-clear, control-intersection and two
  staged-Q helpers through LCALL, then invokes the descriptor acquisition
  worker through ECALL. Only acquisition result zero writes status five to
  RAM:00fba0, and the controller words are cleared again on every exit before
  ERET.
- Firmware 1.04 RAM:fb4daa always resets raw-read state, starts coroutine two
  at RAM:f84a00, yields, and requires OPC before close/cache preparation.
  Track types five and six set RAM:000c84 bit six before the four-byte
  RAM:001455 clear. Preparation failure updates the working-track failure
  state, clears bits seven then six through separate volatile accesses, and
  sets RAM:000c85 bit three only when bit zero is set. Success performs the
  same two clears; type one copies RAM:001488 to RAM:00dd41 and finalizes
  opcode 0x5b, type two calls the near close-state commit, type three also
  finalizes opcode 0x5b, and types five/six become type four with RAM:00dd5f
  set before the near tail commit. Type six first copies the same end frame.
- Firmware 1.04 RAM:fa360f handles expiry of the FORMAT UNIT code-11 primary
  countdown. It conditionally finalizes an active write, clears RAM:000c85
  bit zero and RAM:000c84 bit three, normalizes speed, resets raw-read state,
  starts coroutine two at RAM:f84a00 and yields before reasserting bit three
  for OPC. OPC failure only clears that bit. Success forces RAM:000c9e=2 and
  RAM:001586=1, publishes the exact c89/c88/c89/c8a/c8a flag sequence,
  normalizes speed again, initializes the type-two pipeline with ACC=2, clears
  RAM:0088b7 only for a nonzero result, clears c89 bit one then c8a bit three
  then c89 bit four, and dispatches host completion.
- Firmware 1.04 RAM:f90c46 initializes a raw-read position probe. It clears
  BITS:0014/0015/0007, programs controller ports 0x8002/0x8004/0x8006, selects
  0x8005 value 0xcc when any of BITS:0013/001c/001d is set and 0x4c otherwise,
  then writes 0x802d=0x28 and clears port 0x8019. RAM:000f06 is interpreted as
  signed: zero and 0x80..0xff use the base path and publish shadow RAM:000fad
  OR 0x20 to port 0x8003. Positive formats first publish RAM:000eff OR 0x60
  to 0x8019; format one masks RAM:000efe to five bits and then ORs 0x86 in two
  ordered shadow/0x8018 writes, while other positive formats OR 0xe6 once.
  They publish RAM:000fad OR 0x28 to 0x8003. The function finally clears IE0,
  enables EX0 and stores countdown 0x7d before near RET.
- Firmware 1.04 RAM:f92c27 arms raw-sector buffer capture by first clearing
  RAM:000fad bit five and publishing that disarmed value to port 0x8003, then
  writing 0x20 to 0x802d and clearing BITS:0009. BITS:0013 selects constant
  countdown five; otherwise RAM:ff95b5[RAM:000f06] supplies it. The big-endian
  RAM:001296 producer index is multiplied by 0x930; its low/high bytes go to
  ports 0x801c/0x801d and the product's low three bytes go to 0x8007..0x8009.
  RAM:000f0b bit one selects metadata length eight or twelve, format four
  selects 0x8019 value 0x81 rather than 0x80, and the function finishes with
  another 0x802d=0x20 write before reasserting RAM:000fad/0x8003 bit five.
- Firmware 1.04 RAM:f9352d is the raw-read position-probe coroutine-two entry.
  It clears the position offset and alternate/special modes, mirrors current
  read speed into the selector, sets selection value and RAM:00127e to one,
  initializes all queue indices to zero, derives the 16-bit ring mask and
  high-water values as fresh RAM:00156a minus one and minus six, publishes
  descriptor base 0x000088bb and copies RAM:001562 to the auxiliary cursor.
  Until BITS:0009 is set it calls the near probe step; every nonzero result
  yields coroutine two at least once and continues yielding while RAM:000f04
  equals nine. Exit clears RAM:000ff9 bit two and then bit one through separate
  accesses, yields once, and calls the near shared finalizer before ERET.
- Keil C251 5.60 defaults C `double` to binary32. Translation units that
  reconstruct firmware binary64 helper chains must be compiled with the
  `FLOAT64` directive; `__FLOAT64__` and the installed `FLOAT.H` confirm the
  mode. RAM:f97c26 demonstrates the gate: default compilation emitted
  `?C?FP*` helpers and rounded constants, while `FLOAT64` emitted `?C?DP*`
  helpers plus the exact canonical 0.001, 0.0089, 14.3, and 0.01 binary64
  bit patterns. Apply `FLOAT64` per translation unit rather than globally.
- Firmware 1.04 RAM:fe2dad returns its wrapped next main-data-ring cursor in
  DR4 even though older metadata described it as void. Its final
  `MOV DR4,DR28` is consumed directly by callers such as RAM:feb282; C callers
  that chain sectors must use the returned cursor rather than assuming DR4 is
  preserved or recomputing it independently.
## Controller DATA transfer ABI and polling cadence

- In firmware 1.04, RAM:fdddad and RAM:fddf0b both consume the controller
  buffer address in DR4 and the transfer length in DR0. With C251's ordinary
  two-`u32` register assignment, the maintainable C signatures must therefore
  place the address first and length second. Existing C callers that used
  length-first source order emitted the opposite register tuple and had to be
  corrected.
- Both transfer workers increment the yield counter before comparing its old
  value to `0x32`; the first cooperative yield is therefore on polling pass
  52, not pass 51. Their timer timeout similarly acts when the previous count
  is at least ten, i.e. on the eleventh matching cadence sample.

## Controller position-field decoding

- Firmware 1.04 RAM:fee8f0 treats controller position bytes as packed decimal
  when the raw minute/second/frame values are at most `f9/59/74`; it does not
  reject individual nibbles above nine. Minutes `0xaa` through `0xad` select a
  raw sentinel path whose published position is the pipeline target, current
  decoded position, or wrapping target-minus-one according to the status-check
  input and BITS:000f. Other out-of-range tuples are accepted only when
  BITS:0013, BITS:001c, or BITS:001d is set, in that short-circuit order.

## Premium-update recovery wait

- Firmware 1.04 RAM:fd6bab is a near-return recovery worker: it initializes
  coroutine one at RAM:f853cf, raises P3.0/P3.1, performs exactly 300 calls to
  the 59-unit delay, and then services/yields while RAM:0015c4 remains zero.
  Each wait-loop pass sets BITS:0547, BITS:0546, and SFR:A9.2 in that order.
  Once ready, RAM:00faef bit two publishes `0x98` at RAM:0000fd and enters the
  non-returning restart path; with the bit clear, the worker returns normally.

## Operating-profile Timer-1 word

- Firmware 1.04 RAM:fa6b2a maps RAM:000e4f through
  `MapOperatingCodeToProfileIndex`, reads a big-endian word from
  RAM:ff9212 plus twice that index, and writes the one's complement of the
  low 16 bits of `scale * word` to SFR:8d then SFR:8b. Its wait condition
  reads RAM:01807f bit three before testing Timer-1 overflow, so a cleared
  controller bit short-circuits the overflow read. Timer overflow is cleared
  before start and again after Timer 1 is stopped.

## Read-measurement state reset

- Firmware 1.04 RAM:fd9e94 clears the read-measurement workspace and only
  clears dwords RAM:001513, RAM:00150f, and RAM:00150b when RAM:0014d6 is
  zero. It snapshots RAM:000ee4 into RAM:00fbc9. When BITS:0032 is clear, it
  clears BITS:0547, publishes RAM:000fca OR 0x08 to RAM:018002, clears
  RAM:018080, then publishes RAM:000fca AND 0xe7 to RAM:018002 before setting
  BITS:0547, BITS:0546, and SFR:A9.2 in that order. BITS:0032 is cleared on
  both paths before the final RAM:00fbd6 and RAM:001555 clears.

## Media-state-five controller threshold

- Firmware 1.04 RAM:f8e5f1 retains a signed-bound and float32 threshold
  calculation using the three-byte record at RAM:ff8576 plus three times
  RAM:00fb5e. The in-range path reads the family selector independently for
  the signed coefficient and signed base word, publishes and clamps the
  computed value, but every path then unconditionally overwrites
  RAM:00f9e2 with 10000. The preceding reads and intermediate publication are
  still observable behavior and are retained in the C reconstruction.

## Scaled radial-move controller setup

- Firmware 1.04 RAM:fd8753 clears BITS:0542 and RAM:000fad bit two before
  setting up controller word 0x0251 and channel-two value two. With operating
  control enabled and CurrentOperatingCode at least 0x18, it first publishes
  SecondaryOperatingCode 0xff, then selects 0x18, 0x20, or 0x28 only when the
  corresponding signed threshold at RAM:00f9ee, RAM:00f9f2, or RAM:00f9f6 is
  strictly below signed ReadAcquisitionTarget. Each threshold is read before
  the target; equality retains 0xff, and code 0x34 is outside the final range.

## Vendor opcode EE transfer-mode selection

- Firmware 1.04 RAM:fcb976 treats CDB byte one bit four as the enable gate and
  its low two bits as the completion selector. Multiword-DMA indices clamp to
  two and publish protocol three; Ultra-DMA requires RAM:000c86 bit three,
  clamps to two and publishes protocol seven; PIO indices clamp to four but
  deliberately do not publish RAM:0000fe. Requested mode 0xff always selects
  PIO four and MWDMA two, then selects UDMA two/mode 0x42 only when both
  RAM:000c86 bits three and four are set, otherwise mode 0x22. Selector three
  publishes its requested byte without decoding it, selectors two and three
  complete through `SetNoSenseData`, and every other path writes restart marker
  0x99 at RAM:0000fd and enters `RestartFirmware`.

## Indexed-state subsystem initialization

- Firmware 1.04 RAM:fd8043 is a non-returning but ordinary C-reconstructible
  coroutine worker. It tests RAM:00fab4 bit seven to select zero versus
  RAM:00fab5 for E8/E9 setup, installs strategy index 44, lead-out minute 80,
  frame position 360000 and operating/write value 40, passes the big-endian
  RAM:00f9fc word to profile generation, then reads exactly two persistent
  configuration bytes at offset 0x56 for EA/EB scaling. It finishes by setting
  BITS:0580, clearing BITS:0581, ORing RAM:001302 with 0x10, and permanently
  yielding coroutine one; the infinite loop does not itself require a retained
  same-address assembly body.

## Vendor F2 controller-measurement response

- Firmware 1.04 RAM:fbda50 enters after the shared predecessor at RAM:fbda47
  has already incremented B from zero. Its `CMP B,#0xff; JC` loop therefore
  clears exactly 255 bytes, RAM:000b2a through RAM:000c28; RAM:000c29 is not
  touched. The response header uses fresh RAM:00ef73 and RAM:00ef72 reads, and
  dispatches on another fresh RAM:00ef73 low-three-bit mode read. Mode one
  publishes length 0x0134, modes two and six publish 0x0169, mode three
  publishes 0x00c6, and mode four publishes 0x007c; unsupported modes leave
  the cleared length bytes unchanged.

## Primary Type-2 controller-record descriptor initialization

- Firmware 1.04 RAM:fcd65f reaches backward to RAM:fcd614 when inherited B is
  below eight. Each source group starts at RAM:2bb938 plus three times B, but
  the native overlapping dword load discards the first byte and transforms
  the following three bytes with `value | 0xfff00000`. It writes the low word
  before the high word into the five-byte slot at RAM:2bfb04 plus five times B
  and the four-byte mirror at RAM:2ba537 plus four times B. After the optional
  partial copy, it always scans all eight primary slots as signed dwords and
  writes the minimum to RAM:2ba577. The RAM:fcd610 entry is only the zero-index
  public tail-call form.

## Secondary Type-2 controller-record descriptor initialization

- Firmware 1.04 RAM:fcd755 reaches backward to RAM:fcd712 when inherited B is
  below eight. It reads overlapping dwords at RAM:2bb950 plus three times B,
  masks them to 24 bits, and writes each value low-word-first into the
  five-byte slot at RAM:2bfb2c plus five times B and four-byte mirror at
  RAM:2ba557 plus four times B. It then scans all eight slots as signed dwords,
  writing the lower bound to RAM:2ba57b and upper bound to RAM:2ba57f; a new
  lower bound skips the upper-bound comparison on that pass. RAM:fcd70e is the
  zero-index public tail-call form.

## Type-2 MIP state initialization

- Firmware 1.04 RAM:fcd2f0 copies RAM:2bb921..RAM:2bb924 to
  RAM:2ba525..RAM:2ba528 and maps RAM:2bb931 bits seven/six to layout modes
  0, 1, 3 and 3. Bit four is the comparison-match count. Active layouts also
  set RAM:00e625/RAM:000c3c to 0x80, set RAM:2ba5af, and clear RAM:000c88 bit
  seven and RAM:000c86 bit six. After geometry derivation it publishes
  RAM:00faff as 0x0000b4bb and stores the low word of signed
  `(RAM:2ba59f - 0x92) / 39` at RAM:2ba5db. State-table indices zero through
  that result receive five; subsequent indices below RAM:2ba5d9 receive zero.
- The mode-zero boundary-builder count at RAM:fcda9d is a genuine residual-R7
  dependency across the near RAM:fcd912 call. If payload bit four is set, R7
  is the low byte of the post-fill table index. If it is clear, RAM:fcd912's
  low-word-first payload read leaves the low byte of RAM:2ba5a3 in R7. The C
  reconstruction exposes this value through addressless comparison/boundary
  helpers; no same-address native body is required.

## Type-2 runtime record initialization

- Firmware 1.04 RAM:fcd998 writes the Type-2 runtime block at
  RAM:000ca2..RAM:000ce0 from working-track and TrackInformationTable entry-one
  state. It stores 0x96 at RAM:000ca7, derives 0x6ae by rereading that field and
  adding 0x618, and derives 0x500 by rereading the cleared RAM:000cab field.
  RAM:00dd41 is read once for the direct RAM:000cb7 copy and again for the
  wrapping five-doubling calculation `(signed)(end * 32) / 39 - 150` at
  RAM:000cbb. RAM:000cdf preserves bits seven through one and replaces only bit
  zero from RAM:00dd9c.

## Vendor F2 mode-two/six staging

- Firmware 1.04 RAM:fbdef0 first copies 30-byte series at RAM:00f09d,
  RAM:00f0bb, RAM:00f0d9 and RAM:00f0f7 to response offsets RAM:000b38,
  RAM:000b56, RAM:000b74 and RAM:000b92, then appends the shared fields and
  stages 0xd4 bytes at the controller reserved-region cursor from RAM:00155a.
  It replaces the workspace with series RAM:00ef89/00efa7/00efc5/00efe3 and
  RAM:00f01f for mode six or RAM:00f03f otherwise, writes scalar words at
  RAM:000bc0, 000bc2, 000bc4 and 000bc8 (leaving 000bc6..000bc7 intact), then
  stages 0xa1 bytes at the cursor returned by the first transfer.

## Frame-position radial-model mapping

- Firmware 1.04 RAM:fce56c treats RAM:000cf5 as a signed big-endian 32-bit
  scale divided by 65.536 and RAM:000cf9 as an unsigned big-endian 16-bit
  span. It applies the signed frame position through float32 operations in
  canonical order, adds 62500, crosses the native float32-to-float64,
  square-root and float64-to-float32 runtime bridge, then returns
  `scale * 60 / (root * 0.2 * pi)`. Keil C251 treats source-level `double` as
  float in this memory model, so an addressless adapter is required to retain
  the firmware's explicit binary64 square-root bridge.

## Operating-code measurement-bound rounding

- Firmware 1.04 RAM:f97d05 accepts a float32 value in DR4, adds binary64
  4.6592, deliberately narrows the sum to float32, widens it again, divides
  by binary64 0.0043, and applies binary64 ceiling before taking the low
  16-bit result. It adds a fresh native-word read of RAM:00f074 and compares
  the candidate and a separately read base-plus-0x1100 limit as signed
  16-bit values; the clamp path rereads the base a third time. Keil C251's
  `FLOAT64` option can express this complete native chain directly in C.

## Playable-audio frame classification

- Firmware 1.04 RAM:fd4c63 has two track-selection models. With
  RAM:001008 zero it scans six-byte decoded-TOC entries from
  RAM:00f2bf + 6*track until the next start exceeds the signed frame, clamps
  to LeadInLastTrackNumber, and accepts when the corresponding control byte
  at RAM:00f2b7 + 6*track lacks bit six. Otherwise it scans loaded working
  tracks by signed RAM:00dd3d start position, clamps to the effective last
  track, rejects RAM:00dd36 bit six, and accepts only working types 4, 5, 6,
  or 7.

- Firmware 1.04 RAM:fd4b11 validates signed PLAY AUDIO intervals against the
  lead-in end at RAM:0012c1 plus 75, or against the selected working-track
  frame plus 75 only when the effective last track loads as type four. It
  probes the requested start, then start-minus-75 clamped to zero, then
  wrapping start-plus-75. Range/context failures report 05/2100; failure of
  all three playable-track probes reports 05/6400.

## PLAY AUDIO MSF range dispatch

- Firmware 1.04 RAM:fd4162 receives the end MSF tuple in DR4, start tuple in
  DR0, and playback mode in ACC. An all-0xff start tuple means resume from the
  current decoded frame at RAM:001235, so only the end tuple is validated and
  converted. When media coroutine state RAM:000faf is one and RAM:000fba bit
  four is clear, that route optionally sets bit three, publishes the converted
  end at RAM:000fc6, and returns no-sense without revalidating the range.
  Other resume cases and ordinary two-tuple requests pass converted signed
  endpoints through RAM:fd4b11 before starting playback.

## PLAY AUDIO TRACK/INDEX dispatch

- Firmware 1.04 RAM:fd4284 treats start track/index `ff/ff` as a resume form:
  it substitutes RAM:001233 and index one, then an active media coroutine
  publishes a terminal frame minus one and no-sense. In decoded-track mode,
  an in-range resume uses the requested end track's RAM:00dd3d start, while an
  end beyond the effective last track uses that last track's RAM:00dd41 end
  unless RAM:00dd79 selects the lead-in table fallback. Ordinary requests use
  the start track's start frame and `min(end,last)` track's end frame minus
  one, publish kind two/direction zero at RAM:000c8c/000c8d, store the frame
  delta at RAM:000f36, clear BITS:0018, and begin format-2 acquisition.

## PLAY TRACK RELATIVE (12) dispatch

- Firmware 1.04 RAM:fd4631 reads a signed big-endian relative start from CDB
  RAM:000b0c and clamps zero or negative values to zero; the big-endian count
  at RAM:000b10 remains unsigned. Lead-in mode adds the offset to the selected
  track start, rejects a result at or beyond that track's end, and clamps the
  terminal frame against RAM:0012c1 minus one. Decoded mode applies the same
  start rule to RAM:00dd3d and enforces a boundary only when the requested or
  effective-last loaded track has type four. A zero count publishes kind three
  and dispatches acquisition synchronously; a nonzero count publishes the
  wrapping terminal-minus-start span, clears BITS:0018, and starts format-2
  acquisition.

## Opcode BA/CD positioning modes

- Firmware 1.04 RAM:fd487b decodes RAM:000b13 bits seven and six as: zero =
  big-endian LBA plus 150, one = validated binary MSF at RAM:000b0d, two =
  track-based start/end selection, and three = unsupported. RAM:000b0b bit
  four selects reverse direction and the track-end form. The lead-in
  preliminary bound is a signed `start <= end + 75` comparison, while the
  shared final tolerance comparison is unsigned. Forward mode publishes
  `terminal-start`; reverse mode publishes `start-150`, subtracts 18 from the
  acquisition start, and sets BITS:0019. Both modes set BITS:0018 and
  RAM:000fba bit two before starting format-2 acquisition.

## READ HEADER response polling

- Firmware 1.04 RAM:fd3f70 returns ACC `fe` only while raw-descriptor batch
  preparation is pending, zero for suppression/error/empty/invalid routes,
  and one after DATA IN setup. It copies RAM:000c3f to RAM:00145f, publishes
  the validated count at RAM:001463, and conditionally validates the secondary
  descriptor only when descriptor offset 0x10 is nonzero. The response mode
  byte comes from descriptor offset 0x12. Notably, the frame is loaded through
  a near pointer encoded at RAM:000b0d..000b0e; this odd indirection is direct
  instruction evidence and is preserved. The allocation value is the
  big-endian dword at RAM:000b10 shifted right eight bits, narrowed to 16 bits,
  and clamped to eight.

## Controller startup status handshake

- Firmware 1.04 RAM:fda757 is a five-state cooperative handshake around
  controller ports RAM:018041, RAM:018043 and RAM:018046. It delays once,
  arms signed countdown RAM:00121e to 45 while polling status bit five, then
  arms signed countdown RAM:001204 to six while polling bit four. Bit-five
  success clears RAM:000c87 bit one; its timeout sets bit one. Bit-four
  success sets bit two; its timeout clears bit two. Each unfinished state
  yields coroutine zero. Either phase treats a cleared status bit zero as a
  nonreturning restart after writing `ff` to RAM:0000fe, so RAM:018041 is
  cleared only on a normally returning completion path.

## Controller measurement tolerance wait

- Firmware 1.04 RAM:fd05d6 uses ACC to enable the entire measurement wait and
  R7 as the lower-bound-only tolerance selector. When enabled it first waits
  until the signed RAM:001200 countdown has advanced by four decrements, then
  requires exact result one from the ten-percent tolerance predicate. A
  negative countdown terminates the poll. After more than 30 elapsed ticks,
  40 samples of signed controller word 0x0253 at least 0x4000 assert both P1
  outputs. A separate strict window asserts them when the initial countdown
  lies between current-plus-200 and plus-250 in drive-control state two, or
  current-plus-300 and plus-350 otherwise. Finalization always submits
  0x7fff/0x0369, conditionally strobes RAM:018006 with c0, restores indexed
  channel three from RAM:00f98f, and sets both P1 outputs.

## Current-frame controller read-path probe

- Firmware 1.04 RAM:f89367 performs three decoded-position waits: current
  frame plus one for index zero, then the current frame for indices one and
  two. It writes controller port 0x80c0 values 00, 20 and 70 across those
  phases and freshly selects 0xc4 for port 0x80cf only while RAM:001157 is
  exactly two, otherwise 0xc0. The completion loop arms signed RAM:001217 to
  18, accepts only exact result one from the port-0x80c0 bit-three handshake,
  yields on every other result, and fails once the countdown is negative.
  Success restores 0x80c0 value 20, refreshes 0x80cf and strobes 0x8006 with
  c8.

## Operating-code-budgeted Q-packet capture

- Firmware 1.04 RAM:f89194 conditionally refreshes RAM:000ff9 bit zero from
  only the low bit of the P1/CEX0 threshold result when bit two is exactly
  one. Once admitted, it rearms Q capture and stores signed-byte `120 /
  CurrentOperatingCode` in RAM:001220. It processes packets while the result
  is fe and that signed budget remains nonnegative, clearing bit zero after
  three consecutive zero pulse results when bit two is one. RAM:000e3f
  completion returns immediately without yielding; ordinary exits clear
  RAM:000e40 outside media state one and yield coroutine one once.

## Media-state-six ATIP position window

- Firmware 1.04 RAM:f895d1 arms controller three-byte capture with signed
  countdown RAM:001209 set to 40 and classifies fresh RAM:001235 position
  reads against signed bounds RAM:000fe9 minus 0x46a, 0x406 and 0x3fc. The
  lower window requires exact result one from near helper RAM:f896ee and
  derives RAM:000fee from the upper-bound distance; the ten-frame narrow
  window writes zero. Classified and timeout exits clear RAM:001301 bit zero,
  but the direct at-or-past final-bound return intentionally leaves it set.
  The shared finalizer writes RAM:000fef value three, applies the unsigned
  102/105 offset bands, and publishes target 150, result 0x1d and media state
  three only for offsets 102 through 104.

## Media coroutine state three

- Firmware 1.04 RAM:f86951 reloads big-endian countdown RAM:000fb0 from
  RAM:000fb4, uses signed acquisition results zero, minus one and one as the
  only valid statuses, and traps permanently for every other status. Result
  zero clears busy/state and sets P3.0 while clearing P3.1; minus one and the
  RAM:000e3f asynchronous-completion routes clear only the completion latch.
  The result-one loop yields before measurement/controller service, accepts
  only exact one from the countdown predicate, accepts any nonzero indexed
  transition result, but requires exact one from indexed completion. That
  final route copies one fresh RAM:0012bf byte to RAM:000e42 and RAM:000fbb,
  reloads the acquisition target and advances media state to one.

## Drive-initialization coroutine one entry

- Firmware 1.04 RAM:f84b0b is installed as a coroutine entry and has no
  ordinary static caller. It treats the residual ACC left by extended helper
  RAM:fb01de as a real result: two restarts coroutine one at RAM:f84c30, zero
  publishes RAM:000fae=0x0f plus RAM:001009/RAM:00fae7=3 and restarts at
  RAM:f853cf, and every other value yields, sets RAM:000efb and RAM:018004,
  initializes coroutine three at RAM:f84b00, then restarts at RAM:f84c30.
  Before dispatch it clears the acquisition/controller state, conditionally
  resets channel two, restores read speed 0x18 from RAM:0011ef bit two, and
  clears that bit. An addressless extended adapter is sufficient to expose
  the helper's residual ACC to C; no same-address assembly body is needed.

## Startup-sequence coroutine one entry

- Firmware 1.04 RAM:f84c30 clears startup/acquisition state, writes
  0x80000000 to RAM:001598 and RAM:001484, clears BITS:0017, and then calls
  near `InitializeStartupMediaState`. Its P3.0/P3.1-high decision uses a fresh
  signed read of RAM:00faef after that call; it always clears bit seven
  afterward. It then initializes RAM:0011ef bit three, drives P1.T2EX/P1.T2
  low around controller parameter D6=0x7e and the debounced-input sequence,
  raises both pins, writes D6=0, and restarts coroutine one at RAM:f852eb.
  Addressless near C251 `sbit` helpers represent the direct bit operations and
  return with `RET`; the canonical C coroutine body retains `ERET`.

## Media-state coroutine one dispatcher

- Firmware 1.04 RAM:f85d74 is a nonreturning coroutine-one dispatch loop for
  media states zero, one, two, three, five and six; every other state enters a
  permanent self-loop. It clears RAM:000fb9/RAM:00f971 bit zero every
  iteration and clears RAM:00fae8 unless both the previous and current state
  are two. State two sets RAM:00103a bit five before its worker/finalizer;
  state six sets RAM:001306 bit three after its worker. The original calls
  all workers with `LCALL`. When already-C finalizers live in a large module
  as extended functions, small addressless `LCALL`/`ECALL` bridges preserve
  the dispatcher call class without moving those finalizers into the near
  code segment or retaining their same-address assembly bodies.

## Drive-initialization completion coroutine one

- Firmware 1.04 RAM:f852eb clears RAM:000fba bits two and zero in two ordered
  read-modify-write operations, clears RAM:000e40/RAM:00fbd7..RAM:00fbd9,
  sets RAM:000fae/RAM:000fbc, raises P3.0/P3.1 and runs the drive-control
  state-nine wrapper. Result two yields and restarts at RAM:f84b0b; zero
  publishes the three status bytes and restarts at RAM:f853cf. For other
  results, exact indexed-state result one for state 0x0b loads signed
  RAM:00120a with five, busy-waits until it is nonpositive, clears
  RAM:001300 and restarts at RAM:f84b0b. Ghidra's decompiler removed that
  reachable block because it did not model asynchronous countdown changes;
  instruction flow is authoritative. The ordinary fallback yields and
  selects RAM:f84f6a or RAM:f84c30 from RAM:0011ef bit three.

## Media coroutine state zero

- Firmware 1.04 RAM:f85eac is a near worker. It optionally sets P3.0 and
  clears P3.1 for indexed operation, resets drive control, writes controller
  D6=0x7e and holds P1 bits 0481/0480 low while yielding and invoking
  RAM:f8873d plus RAM:f85fa3 until RAM:000e3f is exactly one. It restores the
  pins, writes D6=0, derives RAM:001304 bit four from media states one/two/three
  plus RAM:001008, RAM:00f553, RAM:001132 bit six and RAM:0012be bit two, with
  RAM:000c89 bit zero forcing it clear. Requested operating code is one only
  when RAM:00fbec is exactly one, otherwise eight. Calibration failure clears
  RAM:000fb9 bit one, RAM:001300, RAM:000e40 and media state, publishes result
  seven at RAM:000e41, and yields once. The worker always clears RAM:001304
  bit four before its native `RET`.

## Raw-track selection and identification scan

- Firmware 1.04 RAM:fe28de scans `RAM:00b4bb` unsigned 16-bit records using
  eight-byte, low-word-wrapped addressing from RAM:00b4bc. Record type is the
  low nibble at offset one; type one uses track number offset two to record
  track-zero/track-0xaa indices and the first nonzero track number, while the
  first type-two and type-three records publish indices at RAM:0014a1 and
  RAM:0014a3. Offset four bit 0x40 sets RAM:0014a6. After the scan, a zero
  type-two index invokes the MCN loader (so a real type-two record at index
  zero deliberately shares the sentinel behavior), while only a nonzero
  type-three index invokes the ISRC loader. The function finally clears the
  full 32-bit RAM:0014ac field and RAM:0014a7 before its near `RET`.

## Read-acquisition pickup-move dispatch

- Firmware 1.04 RAM:fc8705 uses unsigned 16-bit thresholds at retained move
  magnitude 150 and 50. Magnitudes at least 150 use the scaled radial move;
  50..149 use chunked movement; smaller values replace RAM:000edc with the
  fine-magnitude result, using chunked movement at ten or above and a byte
  move plus DriveControlState one below ten. The coarse route subtracts 50
  near the modeled disc endpoint and can independently subtract 200 when
  RAM:001006 and RAM:000ed7 are each exactly one. Both subtractions wrap as
  16-bit arithmetic. RAM:000ee9 is compared as a signed 32-bit value against
  2000 or 4000, while endpoint distance arithmetic wraps at 16 bits. Every
  route clears ACC and returns through native `RET`.

## Read-position packet acquisition transition

- Firmware 1.04 RAM:fc8ee4 first polls the controller-Q path while signed
  RAM:001207 remains positive. After each pending result it repolls directly
  when RAM:001008 is zero, controller operation is nonzero, or the current-TOC
  packet predicate is exactly one. With RAM:00f9e9 bit two set, BITS:001c not
  exactly one and BITS:001d clear also force repolling without the timing
  handshake. A timing result exactly one repolls while ConfigurationWordFAAC
  bit five is clear; otherwise the worker clears that bit, wraps-increments
  RAM:00fae8, enables controller operation, clears RAM:001303 bit two, selects
  RAM:000ee9 as threshold only when RAM:000e3f is exactly one (RAM:000e3b
  otherwise), and switches to the three-byte packet parser. Failure first
  publishes RAM:00fb94, then overwrites it with RAM:00f79a only if controller
  operation is zero at the terminal check.

## Signed-domain radial distance to acquisition target

- Firmware 1.04 RAM:fc928f snapshots signed RAM:000e3b to RAM:000eed before
  reading the signed current position at RAM:001235. Each nonnegative position
  is divided by 75 and passed to the ordinary frame-seconds radial mapper;
  each negative position is converted to its two's-complement magnitude,
  divided by 75, and passed to the lead-in magnitude mapper. RAM:000ed3 holds
  the current ordinary radial value only when both observed domains remain
  nonnegative; any negative current or target makes it 0xffff. The return is
  the unsigned absolute difference for positions in the same sign domain and
  a wrapping 16-bit sum when the positions straddle zero.

## Radial step-scale calibration samples

- Firmware 1.04 RAM:fc88fa consumes RAM:0011ef bit seven whenever that gate is
  initially active, including every rejected sample path. For an eligible
  mapped-position delta of at least 5000, it computes
  `ratio = (1000 * delta) / RAM:00112e` and then
  `candidate = 0x03e80000 / ratio`; the second dividend is a shifted 1000,
  not 1000. It accepts the candidate inclusively between 12/16 and 20/16 of
  the big-endian default scale at RAM:f848fe, but stores the original 16-bit
  delta—not the candidate—in the 64-word big-endian ring at RAM:000e53. A
  cursor of at least 0x3f recomputes the full-ring mean into RAM:000ffa and
  RAM:001000; the wrapping increment maps 0xf0 to 0x40.

## Media-state operating-code selection

- Firmware 1.04 RAM:fc8b2a consumes ACC as an exact-one settling selector.
  RAM:000e44 directly selects codes 1, 4, 8, 24, 32, 40, or 52, while value
  five bounds RAM:000e49 by RAM:000e46 and value one optionally bounds
  RAM:000e46 by 52. The transition-constraint helper consumes residual ACC as
  a seed: supported paths leave the selected code, while an unsupported
  selector reaches it with selector minus one because of the native dispatch
  arithmetic. A changed code or RAM:001304 bit three reloads the controller;
  media mode two also clears/sets RAM:001307 bit zero at code 32 and adds nine
  with byte wrap to RAM:00fab5 for the high-code E8/E9 value. ACC input exactly
  one waits only for a changed code; every other value finalizes using signed
  RAM:000ef5. The worker finally copies a fresh RAM:000e43 to RAM:000e4d.

## Raw-read host-transfer descriptor counting

- Firmware 1.04 RAM:fc65f4 scans from the raw-read secondary cursor without
  consuming it: the cursor is restored on every return. The scan is capped at
  `min(HostTransferTotalBlockCount - HostTransferCompletedBlockCount, 22)`.
  A nonzero descriptor status must validate; RAM:001468 then retains that
  accepted descriptor but stops the scan. When BITS:000c is set, each accepted
  descriptor's big-endian frame at offset two must match the logical sequence;
  opcode b9 with RAM:000c9e set uses logical block plus 150, while other cases
  use the controller frame mapper. A failed validation or position comparison
  removes the tentative descriptor from the returned count. After each
  accepted record the temporary cursor advances through the ring, and scanning
  stops when queued entries after that cursor are at most RAM:000f46.

## READ/READ CD raw-sector batch service

- Firmware 1.04 RAM:fc5226 returns signed ACC statuses: one for complete,
  zero for a suppressed or stopped batch, fe for pending/restarted work and ff
  for transfer-configuration failure. Producer failure reports information as
  `CommandStartLogicalBlockAddress - RAM:000f0c +
  HostTransferCompletedBlockCount`. An empty mapped batch advances the
  secondary cursor when descriptor byte 0x13 is ten; otherwise a zero status
  byte resets the queue, starts coroutine two at RAM:f90000 and publishes a
  restart target from base LBA plus completed blocks, using plus 150 directly
  for opcode b9 under RAM:000c9e or the ordinary frame mapper. Nonempty batches
  hold BITS:003c only around raw-sector transfer configuration. Successful
  configuration still returns fe until the full 32-bit completed and total
  block counts match, and RAM:001466 or RAM:001467 suppresses final success.

## Shared READ-command raw-sector execution

- Firmware 1.04 RAM:fc5107 is the common raw-sector engine for READ(6),
  READ(10), and READ(12). It clears mapping adjustment/state before checking
  command-completion bit six, rejects FullTocSummaryFlags classes one or the
  failed class-five probe with 05/6400 sense, and either clears BITS:000c or
  saves and maps the command-start LBA before range validation. Accepted work
  lowers both P3 handshake pins, reloads RAM:000fb0, updates raw-read progress,
  and either repositions an existing mode-two pipeline or resets the queue and
  starts coroutine two at RAM:f90000. Batch status fe yields and retries;
  status zero triggers recovery through RAM:f84a00 and acquisition request
  five. Every accepted terminal route writes RAM:001226=4 and raises both
  handshake pins. Addressless direct-bit helpers used from C must be declared
  near when they return with `RET`; C251 listing review caught and confirmed
  this call-class requirement.

## READ CD format-specific raw-sector execution

- Firmware 1.04 RAM:fc59df consumes the raw-read format index in ACC and
  returns the signed batch status in ACC through `ERET`. It clears the
  completed-block count, mapping adjustment, format bits and retry latches.
  With BITS:000a clear it clears BITS:000c; otherwise it saves the original
  command LBA and maps it minus 150 except when RAM:000c9e is nonzero and the
  opcode is B9. An existing mode-two pipeline is reused only when RAM:000f06
  exactly matches the requested format; otherwise coroutine two restarts at
  RAM:f90000 with target `CommandStartLogicalBlockAddress + 150` and the new
  format is published. Batch status fe yields and retries. Status zero starts
  recovery at RAM:f84a00 only when BITS:0011 is clear and secondary-descriptor
  byte 0x13 is not four. Every range-accepted terminal path restores
  RAM:001226 to four and raises both P3 handshake pins while preserving the
  batch status as the return value.

## Raw-read pipeline positioning

- Firmware 1.04 RAM:fc5af5 masks external interrupt zero while selecting a
  raw-read secondary cursor for a signed target frame. Descriptor offset two
  is a big-endian 16-bit pointer to a big-endian signed 32-bit frame position.
  When the target lies between the consumer and latest queued positions, the
  cursor becomes `(consumer + target - consumer_frame) & index_mask`; a target
  beyond the latest position can reuse the producer cursor, and an empty queue
  can reuse the consumer cursor, only when BITS:0008 is exactly one or
  RAM:000f46 is nonzero and RAM:001598 equals the target. Reuse through the
  producer or relative route invokes backlog limiting. All other routes
  reenable the interrupt, reset the queue, select coroutine-two entry
  RAM:f90000 for mode two or RAM:f929ab otherwise, publish the target and
  yield. External interrupt zero is enabled on every return.

## Raw-sector descriptor status validation

- Firmware 1.04 RAM:fc5364 receives the near descriptor address in WR6 and
  returns a normalized Boolean in ACC through `ERET` (not `RET`). Descriptor
  byte 0x10 is the sense key, word 0x0e is ASC/ASCQ, and word 0x02 points to
  the big-endian frame position used for sense information. Zero status is
  accepted; mode three or RAM:000c89 bit zero suppresses a nonzero status.
  ASC/ASCQ 1105/1106 clears RAM:001468 and RAM:001466, manages retry latch
  RAM:001467 through RAM:000c3c bits four/five, and can accept on either bit.
  ASC/ASCQ 1803/1804 is accepted without publication unless mode bit two is
  set; then it sets RAM:001466, publishes sense/information, and mode bit one
  chooses a retry result whose acceptance is controlled by bit five. Other
  codes publish sense and reject. Published information wraps as descriptor
  frame minus RAM:000f0c minus 150.

## Raw-Q position probe at pipeline target

- Firmware 1.04 RAM:fc5e81 seeds RAM:000f41 from signed RAM:001598. Targets
  zero through 450 reject with status seven when RAM:001132 bit four is set or
  when BITS:0017 and RAM:00127e are both nonzero; otherwise BITS:0017 is
  cleared. The accepted probe lowers the host pins, selects raw-read mode 11,
  resets the queue, starts coroutine two at RAM:f9352d, and yields through
  coroutine three until BITS:0008 is no longer exactly one; RAM:000fae aborts
  immediately with status four before cleanup. Descriptor word 0x0e maps
  0281/2100 to seven, zero to one, 800b to a tuple scan, and other values to
  four. For 800b, descriptor dword 0x04 seeds the ring cursor, which is
  normalized by ring size when below 0x930 and reduced by 0x924. Up to four
  four-byte tuples are consumed; marker bits below C0 advance the cursor by
  0x92c, while C0/E0 converts the first three packed-BCD bytes and adds one or
  two frames. A signed difference outside ±150 replaces that candidate with
  the previous tracked position plus one. A found tuple returns six; exhaustion
  returns seven. Normal terminal paths restore RAM:001226=4 and both P3 pins.
- RAM:fc627c `ReadMainDataRingByteAtCursorPlus16` is a near `RET` worker and
  must be declared `PLEXTOR_NEAR_FUNCTION`; live disassembly and the C251
  caller listing confirm `LCALL`, correcting the earlier generic C ABI.

## Raw payload signature acquisition

- Firmware 1.04 RAM:fc6019 obtains the raw-sector payload source from the
  consumer descriptor's big-endian dword at offset four, adds the 1-MiB CPU
  alias and the 24-byte raw-sector header (`+0x00100018`), and copies exactly
  0x800 bytes before recognizing the four-byte MIP, MDT, SIP, or SDT prefix.
- A single hand-preserved A251 byte block can contain more than one canonical
  function. The former RAM:fc6019..fc62ab block also contained the complete
  RAM:fc6247 and RAM:fc627c bodies, so removing the block retired all three
  assembly representations after each had an accepted C implementation.

## P1.6 drive-state transition service

- Firmware 1.04 RAM:fd1d1e always normalizes RAM:001302 bit six,
  RAM:00faed bit one, RAM:00faef bit four, and RAM:0011ef bit five before
  evaluating media state. For nonzero media state, RAM:00122d is a signed
  byte: only a strictly positive value defers the request through
  RAM:000c8b bit four; values 0x80..0xff continue through the ordinary media
  routes.
- The post-request indexed-state suppression test uses state code 0x0c, not
  0x0f. When inactive, RAM:001309 equal to one requests RAM:0011e5 bit two;
  otherwise RAM:0011ef bit six is set and RAM:001301 bit six permits both P3
  handshake lines to be driven high.

## Write controller-buffer transfer start

- Firmware 1.04 RAM:fcfe05 receives the block count in ACC and the controller
  address in DR4. It adds the count to big-endian RAM:2ba5d4, advances the
  wrapping byte counter at RAM:2ba5c6, mirrors it to RAM:2ba5c7, and programs
  only the low three address bytes to RAM:018010..018012 in little-byte order.
- The length is `block_count * 0x800`; this worker publishes only its low and
  middle bytes at RAM:01800d..01800e. RAM:018050 bit zero clear selects command
  0x88; when set, counts below 0x20 select 0xd8 and larger counts select 0x8a.

## Write-transfer 32-block cursor alignment

- Firmware 1.04 RAM:fedd6d divides the signed big-endian cursor at
  RAM:2bc703 by 32 with truncation toward zero, then reconstructs its group
  base by multiplying the quotient by 32. An aligned cursor advances one group
  only when the unsigned DR4 block count exceeds 32; an unaligned cursor
  advances when RAM:fee38a reports that the command crosses the next boundary.
- Every unaligned input replaces the returned count with byte RAM:2ba5c6,
  whether or not it crosses; the aligned path does so only when it advances.
  RAM:000c8a bit two selects fixed-packet rather than record-overhead mapping,
  after which the host range initializer's result is mirrored to both
  RAM:001598 and RAM:008000.

## Raw-read controller transfer-batch configuration

- Firmware 1.04 RAM:fed36f consumes block count in WR2 and an inherited main
  data-buffer fallback address in DR4, despite the earlier one-parameter
  decompiler prototype. It adds the count to big-endian RAM:001459, programs
  the shared transfer-address registers, and walks byte count RAM:001469 over
  big-endian dword component lengths beginning at RAM:001470.
- More than one component publishes each component length's low two bytes to
  controller ports RAM:01800d..01800e before address selection. Every selected
  buffer address is written low-byte first to RAM:018010..018012. Final length
  is unsigned `RAM:00146d * block_count`; controller bit RAM:018050.0 clear
  selects command 0x80, while set selects 0x82 only for exact length 0x10000
  and 0xd0 otherwise.

## Write-preservation descriptor batch polling

- Firmware 1.04 RAM:fee0a0 returns 0xfe when the latest descriptor is absent
  or a forced/full path has fewer than 32 queued entries. The incremental path
  decrements the 16-bit producer and count without masking, then enqueues from
  `producer * 0x930 + 0x2df0` with byte count `0x20 - producer_low`.
- The full path copies the consumer index to the secondary cursor and scans
  the inclusive unsigned range through consumer+31. Nonzero descriptors whose
  status record is neither 0x1804 nor 0x1803 are enqueued; result one survives
  only while every such status record has byte 0x13 equal to 0x0b.
- This path preserves a firmware descriptor convention also seen elsewhere:
  the signed 32-bit frame value is assembled from the big-endian word at the
  address stored in descriptor +2 as its high word, with the descriptor's own
  16-bit address as the low word.

## Write-preservation descriptor-range enqueue

- Firmware 1.04 RAM:fee1da receives the main-data cursor in DR4, signed frame
  position in DR0 and descriptor count in ACC, and returns with `RET`. It
  initializes the preservation/Q metadata, publishes the frame position at
  RAM:001598, and builds packed-BCD absolute tuples at RAM:001428 and
  RAM:001279 plus the frame-minus-150 relative tuple at RAM:001275.
- Its fixed-packet-overhead result is interpreted as signed: zero and
  0x80..0xff publish a descriptor, while 1..0x7f skip publication. Accepted
  positions program controller block RAM:001428, submit mode 0xaa, update Q
  control, enqueue the current descriptor and advance the controller-buffer
  cursor. Every iteration, including skipped positions, increments the signed
  target and all three packed-BCD MSF tuples.

## Synthetic 32-descriptor write batch

- Firmware 1.04 RAM:fede81 receives a signed logical start in DR4, but its
  first descriptor cursor is the result of
  `ResumeMainDataRingCursorFromLatestDescriptor` plus `0x2df0`; it is not
  derived from the input start address. RAM:000c88 bit seven selects
  fixed-packet mapping, otherwise record-overhead expansion is used, and 150
  frames are added before installing packed-BCD tuple two.
- The worker snapshots both packed-BCD Q MSF tuples as signed adjusted frame
  positions, emits exactly 32 descriptors with controller mode `0xaa` and
  source kind four, and advances both Q state and the ring cursor on every
  iteration. It then reconstructs both snapshots, restores them exactly, and
  copies restored tuple two to RAM:001428. RAM:018028 is refreshed from
  RAM:00142b before every controller submission.

## Controller parameter-F4 calibration

- Firmware 1.04 RAM:fca1cc averages four unsigned results from
  `MeasureAveragedSfrD8MinusD9WithOffsets` by shifting their word sum right
  twice. Its compiler FLOAT64 comparisons are exactly an inclusive 91..107
  acceptance window: means above 107 subtract `0x24` from F4, and means below
  91 add `0x24`.
- Candidate F4 begins at `0xd8`; each trial writes parameter word `0xf400 |
  candidate` before rejecting exact candidates `0xfc` and zero. RAM:00120a is
  a fresh signed, timer-owned retry gate seeded to ten. A negative terminal
  value publishes fallback `0xd8`; otherwise RAM:00faca receives the final
  candidate after the D4/E2 restoration sequence.

## Controller progress-profile transition

- Firmware 1.04 RAM:fcef6e receives profile index in ACC, progress bucket in
  R7 and transition mode in R6. A three-byte near C function reproduces this
  tuple directly under C251, so the two reconstructed C callers need no
  addressless ABI adapter.
- Operating codes 0x34 and 0x28 select progress thresholds one and three;
  other high codes use six. Non-one transition modes reduce the comparison
  threshold by one. At that boundary, profile five under code 0x34 is
  replaced by RAM:ff907f; other profiles increment.
- RAM:001302 bit seven records an active transition. A newly active mode-one
  transition with applied code at least 0x20 may run a signed five-count
  coroutine-one settle loop when RAM:00103a bit five is set, but its 200-unit
  post-profile wait depends only on the newly active/high-code/mode-one gates,
  not on that bit-five settle-loop gate.

## Controller-Q ADR-one position publication

- Firmware 1.04 RAM:fcfa0e is a near no-argument worker. On the first packet
  with TNO `0xaa`, it preserves the prior ten-byte positional record from
  RAM:001245 at RAM:00f578 unless the prior decoded TNO was already `0xaa`.
  TNO and index/point values below `0xa0` are converted from packed BCD;
  special values are retained verbatim.
- It decodes both packet MSF tuples, but index/point `0xa0` replaces the
  decoded middle byte of tuple two with raw packet byte RAM:001257. Tuple one
  is always copied to RAM:001241..001243. Ordinary packets publish tuple two
  at RAM:00123d..00123f and use its minute as progress; lead-in packets leave
  that positional tuple untouched, optionally use tuple-one minute under a
  nonzero RAM:001008 mode, clamp values at least 90 to zero, and force the
  last-applied progress byte to `0xff`.
- RAM:fcfc06 `ParseControllerQPacket` is also a near worker with native RET.
  C251 verification of the reconstructed ADR-one caller corrected its earlier
  extended declaration: both parser and tuple decoder are reached by LCALL,
  while the copy, BCD, progress, and position helpers use their verified
  extended contracts.

## Pickup homing against the debounced P1 input

- Canonical PX-W5224A 1.04 has exactly two direct callers of stock HOME
  RAM:fb084e. At RAM:fb07ff the drive-control reset path skips HOME when
  RAM:001300 is already 8; otherwise, after clearing CPU:01849d and
  resolving drive-control state, it calls HOME only if RAM:000fb9 bit one is
  clear. At RAM:fb0f85 the state-6 operating calibration path first probes
  and validates the controller and enters state five, then calls HOME when
  RAM:00fbec is zero; retry calibration can call it again. Both call sites
  were checked against live Ghidra instruction bytes. This is materially
  different from invoking a HOME clone directly in SCSI coroutine zero.
- In the diagnostic C4 HOME image flashed on 2026-09-27, two full HOME passes
  returned CHECK 05/24/00 after 5,203 ms. In-command autosense trace
  `48 59 02 03` at bytes 3..6 showed raw SFR:P1.2 high on every sampled
  iteration, no RAM:0011ed bit-2 hit, and deadline expiry. Firmware 1.04
  RAM:fd1b3d computes the selected active-low P1 sample as
  `(P1 & 0xe4) ^ 0xff`, so raw high/debounced low is expected. The trace
  establishes no observed sensor actuation during this run; it does not
  establish whether the pickup moved. Autosense retained the trace, while
  a subsequent F5 read of RAM:000b18 found sense bytes 3..6 cleared.
  Evidence: `out/agent-tests/gofw-home-trace-live-20260927/`.
- Firmware 1.04 RAM:fb084e is a near worker that snapshots P1.1, sets it, and
  drives code three for a signed 120-count phase while checking RAM:0011ed bit
  two before every yield. Expiry causes a port-0x849d stop, a signed five-count
  wait, and a second code-four/120-count phase. Second-phase expiry emits an
  additional stop before the shared stop path; sensor completion does not.
- The shared tail disables drive code four, waits signed count nine, stops
  port 0x849d, publishes radial estimate `0x0855` and validity byte one, waits
  signed count one, restores the exact prior P1.1 value, and resets controller
  channel two. Keil C251 can express the native direct-bit behavior with an
  `sbit` declaration while leaving inherited R15 untouched and returning with
  RET.

## Floating-runtime private tuples can become explicit C records

- Firmware 1.04 RAM:fd91e2 simultaneously transforms WR0, WR8, WR16 and WR24
  and publishes the XOR of operand signs in PSW.F0. Once all callers are
  reconstructed and no caller retains that private tuple, the semantic result
  is safely represented by an explicit C record containing both significand
  headers, both doubled exponent fields, and the sign XOR. The near C251
  implementation retains RET.
- RAM:fd9259 admits the same treatment for its binary32 WR0/WR4/R8/ACC/F0
  tuple. RAM:fd9205 and RAM:fd9272 expose their retained rounding bit,
  mantissa, exponent and sign as ordinary parameters and return packed
  binary64/binary32 values. C251 FLOAT64 emits ERET for the composite
  binary64 result and ordinary extended ERET for the packed binary32 word.
- This conversion is valid only after caller reconstruction removes the
  private register/flag dependency; it is not a general claim that arbitrary
  compiler-runtime entry points have a source-level C ABI.

## Explicit C models for stack, coroutine, and interrupt primitives

- Firmware 1.04 RAM:fee571/fee58a push/pop DR0 through DR24 and relocate
  return-address words at hard-coded SPX-relative offsets. Once all semantic
  callers are reconstructed, their private register/SPX tuple can be replaced
  by an explicit C state record containing DR0..DR24, SP, and stack memory.
  This preserves the ordered stack transformation without pretending that an
  ordinary C call frame is the original ABI.
- RAM:fee4fd..fee559 contains nine coroutine resume/yield/restart primitives.
  C251 can express their target behavior without a helper call or compiler
  stack frame by directly accessing the DR12 register-file location, EDATA
  saved-SP slots, SP/SPH SFRs, and EA bit. The C bodies still terminate with
  ERET after installing the destination stack, so the nonlocal continuation is
  represented without retaining a same-address assembly implementation.
- Keil C251 5.60 `interrupt` functions supply RETI wrappers, but their generic
  prologue is not byte-identical to the original entries. The observable
  semantics must therefore be stated explicitly in C: SFR:D1 bit handling for
  controller entries, RAM:00fa clear plus restart transfer for the host entry,
  timer dispatch, and interrupted-PSW publication for the SFR:E1 entry. The
  reset entry compiles to an extended jump to the C firmware main loop.
- These reconstructions establish source-level behavioral equivalence, not
  original vector size, placement, prologue bytes, or whole-image identity.
  Keep exact-match claims separate from the 1,717-address C coverage count.

## Media-state-five calibration accumulation

- Firmware 1.04 RAM:fa7d9e appends RAM:00fb95 at
  `RAM:001046 + RAM:001045`, then increments the wrapping byte count. Count
  three only classifies the first three samples; count four uses the
  four-sample selector, with RAM:00faac bit two forcing success; count seven
  uses the window selector and accepts only the exact result one.
- Both completed selection paths set RAM:00fab0 to one and clear RAM:000c84
  bit three. Success publishes RAM:00fab1=1, RAM:00fac4=2,
  RAM:00fac5=RAM:001042, and sign-magnitude RAM:00f06d at RAM:00fac7. Only the
  count-four success path also copies RAM:001042 to RAM:001044. Failure
  publishes RAM:00fab2=2 and RAM:00fab1=0.
- The original four stack bytes are duplicates of selected/published values,
  not caller-visible inputs or results. The observable routine is therefore
  representable as ordinary no-argument C with ERET; its classification and
  selection helpers are near LCALLs and its sign-magnitude encoder is an
  extended ECALL.

## Media-read transition coroutine

- Firmware 1.04 RAM:fa6e33 is an ERET coroutine-one continuation with no
  explicit inputs. It always sets RAM:00f08c bits zero and one. Non-mode-two
  media clears RAM:000faf and restarts at RAM:f85d74 without running the
  shared calibration.
- Mode two maps RAM:00fb80 values one/two to speed 2/4; otherwise RAM:00fb88
  value one selects 8 and all other values select 20. RAM:000c84 bit three is
  set only across the near calibration call and is cleared on both outcomes.
- Calibration failure resets drive control, maps RAM:000e41 to operation
  status 4/1 and status-or-0x40, emits P3 pattern one once, then repeats P3
  pattern zero plus coroutine-one yield forever. Success clears exact
  completion value one; other values publish media state three and copy the
  current decoded frame to the read-acquisition target before the fixed
  RAM:f85d74 restart.

## Read-range controller measurement start

- Firmware 1.04 RAM:fa747a uses the mapped CurrentOperatingCode index to read
  a big-endian timer interval from RAM:ff9234 plus twice the index and a byte
  from RAM:ff922c plus the index; it separately uses the unmodified operating
  code to select the scale byte from RAM:ff922c.
- Timer-1 reload is `0xffff - interval`, written high byte to SFR:8d and low
  byte to SFR:8b. The routine polls overflow without yielding after clearing
  global interrupts, then stops and clears Timer-1 before controller
  parameter publication.
- RAM:00fb88 values two or three add 0x0ce0 to RAM:00f074; value one adds
  0x0c40; other values add 0x0b40. The tail writes words
  `0xfa00 | RAM:00fc0d` and `0xe720 | RAM:00fc15`, with global interrupt,
  controller interrupt and SFR:A9 bit two enabled between those writes.

## D8/D9 controller measurement series

- Firmware 1.04 RAM:fa78fd processes phases below 15. CurrentOperatingCode
  two/four/eight selects 7/5/3 captures after non-yielding waits of 300/120/50
  units. Other codes retain inherited R15 as the capture count; the C boundary
  preserves that fallback with a narrow addressless adapter.
- Each capture produces four-sample D8/D9 sums. After sorting, the routine
  sums indices one through count-minus-two and divides by
  `(count - 2) * 4`, publishing phase-indexed averages at RAM:00efc5 and
  RAM:00efe3.
- With `d8_delta = average_d8 - RAM:00f9a0` and
  `d9_gap = RAM:00f9a2 - average_d9`, RAM:00f03f receives
  `1000 * (d8_delta - d9_gap) / (d8_delta + d9_gap)`. When RAM:00ef72 bit six
  is set, RAM:00f001 receives `1000 * (d8_delta + d9_gap) / average_d8`,
  clamped to zero only when the signed quotient is negative.

## Configured read-measurement range acquisition

- Firmware 1.04 RAM:fa7317 snapshots CurrentDecodedFramePosition, clears
  RAM:00f08c bit three, and starts measurement only when a subsequently
  decoded frame equals the wrapping RAM:00f091 start minus one. If the start
  has already been passed while bit three remains clear, it fails with result
  code eight.
- While the measurement is active, parser wait completion, P0.7 loss, signed
  timer expiry, controller error, or a skipped/backward frame all run the
  measurement end helper. Skipped/backward frames publish result six; P0.7,
  timeout, and missed-start routes publish eight; controller errors propagate
  the fresh RAM:00fb94 value.
- Consecutive positions are checked with signed 32-bit comparisons. At or
  beyond the signed RAM:00f095 end, the routine clears control bit three,
  ends measurement, publishes result zero and returns Boolean true in ACC via
  native RET. Before the end it advances operating-stage updates once per
  accepted frame.

## Raw-read transfer-batch polling

- Firmware 1.04 RAM:fa439b returns 0xff for SCSI completion bit four or the
  shared raw-read abort state, clearing RAM:000c8a bit one only for the latter.
  It returns zero for the external-stop byte or when the signed position
  window countdown is nonpositive with at least three target matches.
- After descriptor preparation, an empty eligible secondary descriptor with
  byte `+0x13 == 10` advances the masked secondary cursor. If byte `+0x10` is
  zero instead, it resets the queue, starts coroutine two at RAM:f90000, maps
  RAM:0015dc plus completed blocks, stores the new pipeline target, and returns
  0xfe.
- Nonempty batches add the validated word count to the wrapping completed
  dword. Exact total completion advances the masked cursor, limits backlog,
  and maps exact target-match result one to status one and every other result
  to status two; incomplete work returns 0xfe.

## Media write-pipeline preparation

- Firmware 1.04 RAM:fa4263 normalizes write speed, converts media coroutine
  state two to three while setting completion byte one, clears RAM:001037,
  resets the raw-read queue, initializes coroutine two at RAM:f84a00 and
  yields before setting the temporary RAM:000c84 bit-three and RAM:000c89
  bit-one flags.
- OPC failure clears only RAM:000c84 bit three; RAM:000c89 bit one remains
  set. OPC success rebuilds the four raw-read indices, derives mask/high-water
  words as ring-slot-count minus one/minus six, restores controller address
  0x000088bb, zeroes both ring cursors while copying the auxiliary base, then
  clears controller-buffer, sector-control, pipeline-status and transfer state.
- The success tail brackets Type-2 geometry recalculation and record-window
  acquisition with RAM:0088b7 and RAM:000c8a bit three, then clears
  RAM:000c89 bit one, RAM:000c84 bit three, RAM:000c8a bit three and
  RAM:000c8b bit zero. RAM:000c84 bit three is cleared again on the common
  exit.
- Firmware 1.04 RAM:fe5dd0 is a near Boolean finalizer. Write type zero writes
  signed `ReadPipelineTargetAddress - 1` to RAM:008000 unless write-option bit
  one is exactly set; types one and three only mark preparation ready. Type
  two builds the lead-out tuple from record bytes zero, two and three at
  `RAM:00b4c1 + index * 8`. An exact tuple match sets pipeline state three and
  calls the lead-out acquisition worker at most ten times: the first call gets
  restart zero and retries get one. Each retry requires preparation status
  four and RAM:00fbec other than two, clears RAM:001490 and RAM:00127d, and
  clamps write speed above 16x. Exhaustion reports failure five; success clears
  sense. A nonmatching tuple simply marks preparation ready.

## Controller response operating-profile application

- Firmware 1.04 RAM:fa6a02 is a no-input near `RET` function. Media state one
  maps RAM:000e51 through RAM:ff91de or RAM:ff91eb; every other state maps
  RAM:000e4f through RAM:ff91aa or RAM:ff91b7. BITS:0031 selects the second
  table in each pair.
- It clears then publishes RAM:001055 at RAM:01807f, calls the scaled-profile
  writer with ten, reloads Timer-1 with `0xffff - selected_table_byte`, and
  busy-polls overflow without yielding. Non-state-one paths additionally
  write `0xfa00 | RAM:00fc0d`.
- The tail clears RAM:01807f, writes eight to RAM:01802e, selects D4/E2 code
  0x14 when BITS:0044 is clear or 0xff when set, writes
  `0xe720 | RAM:00fc15`, sets BITS:0038, clears BITS:003a and RAM:00fad9 bit
  one, stores ten at RAM:00121c, then performs three fresh RAM:000fd0
  read/modify/write operations: OR one, OR six, AND 0xef.

## Media-read control initialization

- Firmware 1.04 RAM:fdbc83 disables BITS:0547 before five fresh
  RAM:000fca read/modify/write publications to controller port 0x8002. Their
  masks are clear-0x18, set-0x08, clear-0x18, set-0x08, clear-0x18. Controller
  type five alone writes 0x40 to port 0x8031. The fixed sequence writes 0x41
  then later zero to ports 0x8008/0x8009, writes 0x7f then 0xff to 0x800c,
  publishes RAM:000efb..000eff at ports 0x8004/5/6/0x8018/19, and writes
  zero to ports 0x8007..0x8009 and 0x801c..0x801d. It finishes by setting
  BITS:0547, BITS:0546 and SFR:A9 bit two in that order.

- Firmware 1.04 RAM:fdcd1e resets the raw-read queue and cooperatively waits
  while RAM:000c83 bit five remains set. It stores RAM:0012c1 minus 150 at
  RAM:0012c5, maps the last-track tuple at
  `RAM:2bb03e + RAM:001149 * 10`, and stores the converted position and its
  minus-150 companion at RAM:00157a and RAM:00157e.
- RAM:0015e9 starts from RAM:0012bf. In media state zero, RAM:000f63 becomes
  one only when RAM:001147 is greater than six and RAM:001148 masked by 0xbf
  is nonzero. In other media states, RAM:0015e9 changes to RAM:0012be only
  when that byte is not 0xff and does not exceed RAM:00f2bd.
- The initializer installs coroutine three at RAM:fbc9e0, publishes signed
  maximum 0x7fffffff at RAM:0015e5, masks RAM:000c84 with 0x77,
  RAM:000c85 with 0xf5 and RAM:000c86 with 0xfd, and initializes retained
  OPC/control bytes RAM:00e74b..RAM:00e74e, RAM:00e751 and RAM:00e622.

## Common read/seek acquisition dispatch

- Firmware 1.04 RAM:fb23c8 accepts signed LBA in DR4, adds 150 with 32-bit
  wrap, and clears RAM:0015e2. Media state zero validates the target against
  RAM:0012c1 plus 75. Other states use that same limit when RAM:001153 is
  below RAM:0012c1, otherwise RAM:001153 is the inclusive upper bound.
- Invalid targets publish sense 05/2100 and the original LBA bit pattern.
  Valid targets below one clamp to one. With media coroutine state three, an
  existing decoded position is reused from target-minus-three through
  target-plus-ten inclusive; reuse resets the raw-read queue, installs
  coroutine two at RAM:f84a00 and clears sense.
- Targets outside that asymmetric window, or every target outside media
  coroutine state three, clear RAM:000c8c, publish the target at RAM:000c8e,
  and synchronously dispatch read acquisition.

## Initial coroutine configuration

- Firmware 1.04 RAM:fdcfad writes five to RAM:001214 and busy-waits while its
  signed byte value is positive. A nonzero RAM:001009 installs coroutine zero
  at RAM:fdbe7d and coroutine one at RAM:f853cf, then resumes them in that
  order forever.
- The ordinary route installs coroutine zero at RAM:fdbe7d, coroutine three
  at RAM:f84b00, coroutine two at RAM:f84a00 and coroutine one at RAM:f84c30.
  RAM:001308 values other than one return without a state-specific override.
- RAM:00130a values 0, 8 and 16 are explicit no-ops; 5 replaces coroutine one
  with RAM:f84b0b; 15 replaces coroutine zero with RAM:fb3ed6; and 31 first
  replaces coroutine one with RAM:f84b0b then coroutine zero with RAM:feb6bb.
  Values 1, 2, 3, 4, 6, 7, 17, 20, 21, 22, 23, 0xf2, 0xfd and 0xff select
  RAM:feb6bb unless RAM:00fbf1 is nonzero; state four is additionally
  suppressed when RAM:f7c01f equals one. All remaining values select
  RAM:fdc225 for coroutine zero.

## Firmware main scheduler

- Firmware 1.04 RAM:fdcea9 is a nonreturning C-representable scheduler, not an
  unavoidable assembly boundary. It performs cold initialization only when
  RAM:0000fb differs from one, derives RAM:00faeb from RAM:001308,
  RAM:001157 and RAM:f7c01f, clears P3.0/P3.1 when RAM:00faef bit seven is
  clear, and then configures the initial coroutines.
- Indexed state five copies RAM:018040 to RAM:001157, persists configuration
  offset 0x47/count one, and installs coroutine one at RAM:f853cf only when
  persistence returns zero. Each scheduler pass prioritizes the drive-state
  transition over the pending restart, conditionally dispatches read and P1
  state services, resumes coroutines zero/one unconditionally, resumes two
  and three unless BITS:0029/BITS:0046 equal one, and reinstalls coroutine
  zero at RAM:fdbe7d when RAM:000c88 bit five is set.
- Firmware 1.04 RAM:fee7ed can remain C-owned despite its destructive stack
  reset by splitting at an addressless native adapter: the adapter executes
  `MOV SPX,#0xff` and `EJMP`s a C continuation. That continuation clears
  SFR:a8/a9, invokes RAM:ff2000 through a fixed-address adapter, initializes
  drive subsystems, publishes RAM:0000fb=1, enables interrupt sources and
  tail-jumps the C firmware main loop. This pattern is suitable only when the
  canonical function is proven nonreturning and the pre-reset frame is meant
  to be discarded.
- Firmware 1.04 RAM:fee819 is a simple nonreturning C tail: it clears SFR:a8
  and SFR:a9 and C251 emits an `EJMP` to RAM:fee7ed. Its leading native NOP is
  inert and need not be modeled as source-level behavior.
- C251's `<intrins.h>` `_nop_()` emits a real one-byte NOP, and a volatile SFR
  `|= 0x02` emits single-instruction `ORL direct,#2`. Together with an sbit
  assignment, this lets RAM:fee8a8 remain entirely C-owned while preserving
  its eight-byte SETB/PCON/NOP/NOP/ERET behavior without an assembly helper.

## MODE SENSE page append sequence

- Firmware 1.04 RAM:fc2b4a supports page codes 01, 02, 03, 05, 07, 08, 0d,
  0e, 1a, 1d, 2a and 3f and rejects page control three with sense 05/3900.
  MODE SENSE(6) alone emits the optional eight-byte block descriptor: current
  values use the command block size, changeable values use all ones, and
  default values use 0x0800. MODE SENSE(10) always reports block-descriptor
  length zero; the apparent long-descriptor branch in the decompiler is
  unreachable in the canonical instructions.
- Firmware 1.04 RAM:fc2dcd treats page code 0x3f as an exact recursive
  sequence of pages 01, 02, 03, 05, 07, 08, 0d, 0e, 1a, 1d and 2a. Each
  returned controller-buffer address becomes the destination of the next
  page; this is not a table scan over every nominal MODE page code.
- A single append computes an eight-bit complete-page length as payload length
  plus two, selects the page-control image before any runtime refresh, refreshes
  only current page 1Ah or 2Ah when page control is zero, and returns the
  advanced 32-bit address from RAM:fddd78. The canonical function returns with
  RET and recursively calls itself with LCALL.

## Mode Page 2Ah runtime speed fields

- Firmware 1.04 RAM:fc31c4 derives the performance-descriptor speed ceiling
  as 0, 4, 8, 10, 24 or 52x from media state, active command state,
  controller subtype and runtime flags. A zero ceiling is displayed as 4x in
  the fixed maximum-speed field but is still passed as zero to RAM:fc3460.
- The final fields at RAM:00e6bf and RAM:00e6c7 are not cleared, despite the
  decompiler rendering. The operand sequence loads saved speed RAM:000c7e,
  optionally bounds it by RAM:fbd4f5 for media state one, then bounds it by
  the derived ceiling and encodes that same result into both fields.
- RAM:fc3460 clears the runtime image from its caller-supplied count offset
  through offset 0x3f, then reads a count byte followed by five-byte records
  at RAM:ff8c54. Accepted records become four-byte descriptors containing a
  normalized 0/1 flag word and the record's final speed word; the two-byte
  descriptor count precedes the list. Media state two uses a controller mask
  and `speed / 0xb0 <= ceiling`; media state one with RAM:00fbec zero ignores
  the ceiling, while nonzero RAM:00fbec requires bit seven, the ceiling and a
  clear record bit 0x20; other media states require only bit seven.

## Selected write-controller range anchors

- Firmware 1.04 RAM:fc69ac clears 32 eight-byte records at RAM:2b9600,
  compresses the requested inclusive frame span, and scans 64 controller
  segments at RAM:194000 plus index times 0x800. Segment 20-bit bounds live at
  offsets 0x11 and 0x15, the big-endian record count at offset eight, and
  six-byte records start at offset 0x20.
- Accepted records store the expanded second 20-bit code first and the
  expanded first code second, then are Shell-sorted ascending by the first
  stored dword. The record gate carries the preceding accepted second code
  into the next iteration. Its lower three bytes are uninitialized for the
  first native record; reconstructed C defines them as zero, so only malformed
  initial all-zero record behavior is normalized.
- RAM:fc8141 and RAM:fc8166 are native near RET conversion helpers. Their C
  declarations and definitions require `PLEXTOR_NEAR_FUNCTION`; RAM:fc69ac
  calls each with LCALL.
- Firmware 1.04 RAM:fab306 iterates from RAM:2ba5cd through RAM:2ba5ce
  inclusively using an eight-bit index compared after zero extension to the
  sixteen-bit end-plus-one value. An end index of 0xff therefore retains the
  native wraparound/nontermination behavior.
- Each RAM:2b9600 eight-byte record contributes two big-endian words. The
  low byte of the first word minus RAM:2ba5bc selects `(offset + 5) * 0x930`,
  where the plus-five addition is sixteen-bit; the low byte of the second word
  minus RAM:2ba5b4 selects `RAM:001572 + offset * 0x930`. After transferring
  one sector, the function writes packed-BCD MSF plus type byte two at
  destination offset 12 through the plus-1-MiB controller window.

## Media-strategy profile initialization

- Firmware 1.04 RAM:f8cb86 reads the 15-byte media-strategy record at
  RAM:008000 plus `15 * RAM:00fb5d`. It recomputes that address three times:
  the first record supplies offsets 0..11, the second supplies candidate speed
  offset 12, and the third supplies the big-endian fallback word at offsets
  13..14. Preserve the fresh strategy-index read at each selection.
- Record offsets 0..3 are unsigned bytes scaled by ten into RAM:00fb61/63/65/67.
  Offsets 4..7 pass through signed-byte to float32 to integer32 in the original,
  which is an exact identity for all byte values before the low byte is stored
  at RAM:00f963..966. Offsets 8..11 copy directly to RAM:00f968..96b.
- A signed lead-out position below 0x4ce78 sets RAM:00fad7 bit four. When
  RAM:00fbec is exactly one, a fresh position at most 0x15f90 clears it again.
  A zero record fallback word selects 3000 for family zero or 0x0af0 otherwise;
  record index ten plus RAM:f7c01f exactly one forces speed 32. RAM:001159
  nonzero suppresses final write-speed application.

## Primary raw-read capture batch

- Firmware 1.04 RAM:f94500 snapshots the main-data cursor at RAM:00126e, the
  low-word metadata slot at RAM:001262 and the auxiliary cursor at RAM:00126a;
  it advances and wraps all three only in local storage. This preparatory batch
  never commits those cursor globals.
- Signed-positive RAM:000f06 enables 12-byte Q metadata records at
  `(RAM:00156e + slot*16 + 0x100000) & 0xffffff`. RAM:000f0b bit one enables a
  0x126-byte clear at `(auxiliary_cursor + 0x100000) & 0xffffff`. The apparent
  native `ADD WR4,#0x10` is the 1-MiB controller-window alias, not a 16-byte
  field offset.
- Each submitted frame writes controller block RAM:001428, submits the local
  main cursor with parameter 0xaa, then increments packed-BCD tuples at
  RAM:001428, RAM:001279 and RAM:001275. The initial packet is built from three
  fresh RAM:001598 reads: absolute controller tuple, target-minus-150 relative
  tuple and absolute Q tuple.

## Fixed-packet overhead classification

- Firmware 1.04 RAM:f93f07 selects its initial track from RAM:0012bf when
  RAM:0012c9 is nonzero, otherwise track one when RAM:001149 is zero, or
  RAM:001147. It scans through the effective last track until the signed input
  plus 150 is below a decoded track end; a zero decoded end falls back to the
  big-endian position at RAM:001153 and also breaks when the input is below it.
- The selected 19-byte track record must have bit zero at offset 0x08 set and
  a nonzero big-endian packet size at offset 0x09. Relative positions before
  the decoded track start return zero. Other positions are reduced modulo
  packet-size plus seven: data remainders return zero and the seven overhead
  remainders return seven through one.
- RAM:fd95b1 returns its unsigned quotient in DR4 and remainder in DR0. For C
  callers that need the remainder, the addressless `DivideU32RemainderNative`
  adapter can preserve the native DR4/DR0 argument tuple, move DR0 to the C251
  DR4 return register, and avoid a default-library division dependency.

## Thirty-two-entry write-preservation batch

- Firmware 1.04 RAM:fab406 waits until RAM:00fb9a is zero, clears
  RAM:00127d, and only builds descriptors when big-endian RAM:2ba5d4 is
  nonzero. Its initial absolute Q position is
  `trunc(CommandStartLogicalBlockAddress / 39) * 39 + 150`.
- The batch contains exactly 32 mode-0xaa descriptors beginning at the
  resumed main-data cursor plus 0x2df0. Each entry copies RAM:001279..127b to
  RAM:001428, publishes type two, submits the current cursor, writes
  RAM:000c71..000c74 to controller ports 0x8025..0x8028 while port 0x8021 is
  0x20, clears port 0x8021, then enqueues and advances the Q tuple and ring.

## Read-measurement mode-one preparation

- Firmware 1.04 RAM:fd987d is a near-returning policy dispatcher. Acquisition
  selector RAM:0088b6 equal to one runs descriptor preparation and uses
  RAM:0088b8 values 0/1 for workspace reset or 5/6 for latest-session removal.
  Nonzero RAM:000c9e runs descriptor preparation then reset plus append when
  status RAM:00fba0 remains zero.
- On the ordinary route, RAM:0088b7 with RAM:00e5fb selects remove plus
  append/finalize, while zero RAM:00e5fa selects reset plus append. Otherwise
  RAM:00fb9c selects descriptor preparation and an option/status-gated append;
  RAM:00fb9d values three and two select low-descriptor or descriptor/direct-
  bit-gated append respectively, and all other values publish status six.

## Drive-initialization transition service

- Firmware 1.04 RAM:fdd0e3 always clears RAM:001302 bit six and RAM:001301
  bit two before applying any guard. RAM:000c35 nonzero or RAM:000c84 bit
  three set stops further processing.
- With RAM:001308 equal to one and RAM:00130a equal to 5, 10 or 12, the
  service requires and consumes RAM:0011ef bit six. RAM:001309 equal to one
  independently repeats that fresh-read requirement, so activating both gates
  consumes the bit in the first gate and returns at the second. Selector 0x12
  writes 0x80, zero and zero to RAM:018032, RAM:018033 and RAM:018035 before
  the later operation-state guards.
- RAM:000c9e mode two always defers: RAM:000c89 bit one has priority and
  selects action kind zero, bit zero next selects kind two, and neither
  selects kind one. Kinds zero and one set RAM:000c8a bit seven; kinds zero
  and two set RAM:2ba5b0 to one. Modes one and three defer kind two for
  RAM:000c89 bit zero or kind three for RAM:000c8b bit zero, in that order.
  Other accepted cases clear RAM:001009, initialize coroutines one/two at
  RAM:f84b0b/RAM:f84a00, then resume them in order.

## Indexed state-04 raw-read coroutine

- Firmware 1.04 RAM:fbfbb9 saves RAM:000c86 bit six, clears it during raw-read
  validation, and restores the saved value before handing off permanently to
  RAM:fdc225. The decompiler's apparent unconditional final clear is not
  instruction-accurate.
- Its first RAM:fbfc9f call uses RAM:001153 after decrementing only the low
  word, then subtracting 0x3cfe from the full value; its count is 10000. If
  successful, RAM:00fad6 low nibble zero selects RAM:001153 minus 0x6a0e for
  the second start, while nonzero selects fixed 0x00048792; both use count
  0x57e4.
- A negative first or second acquisition stores failure code 0x3c or 0x3e,
  respectively. After requesting media state zero and waiting for
  RAM:0015c3, failure installs coroutine one at RAM:f8f773 and leaves P3.0
  and P3.1 high; success leaves P3.0 high and P3.1 low. Both paths clear
  RAM:000c84 bit three and enter the coroutine-zero idle loop.

## Media-read control coroutine three

- Firmware 1.04 RAM:fbc9e0 sets RAM:001588 bit four and, for media state zero,
  initially copies selected speeds RAM:000c37/RAM:000c38 to current speeds
  RAM:000e46/RAM:000e47. Selected read speed at most eight clears
  RAM:001591; larger values are temporarily capped at eight. Other media
  states use initial speed four. After the first yield, selected speeds are
  restored unconditionally.
- A valid write-buffer snapshot is cleared with priority over initialization.
  Without one, media state zero resets the Type-2 controller-record pipeline;
  other states initialize the media-read track model and pipeline.
- With RAM:000fae zero, the coroutine sets RAM:000c85/RAM:00faef bit one,
  updates the track-record control flag, consumes RAM:000c83 bit three when
  present or sets RAM:000c85 bit seven otherwise, sets RAM:001584 bit zero,
  clears RAM:001588 bit two, and either initializes acquisition for operation
  mode zero or stores status seven. It then copies eight bytes from
  RAM:00e4f2 to RAM:00fad3, clears RAM:000c87 bit six and yields coroutine
  three forever.

## SEND DIAGNOSTIC command routing

- Firmware 1.04 RAM:fb2915 accepts SEND DIAGNOSTIC only after the fixed CDB
  mask check. Self-test bit two requires a zero parameter length; the
  parameter route requires exactly eight bytes, while a clear self-test bit
  and zero length succeeds without resetting the acquisition pipeline.
- Both active routes reset raw-read state, initialize coroutine two at
  RAM:f84a00, dispatch acquisition request five, yield coroutine zero and
  clear eight words at RAM:00dcbd. The self-test failure priority is boot or
  configuration memory, RAM:001009, then controller-buffer addressing. The
  parameter route tests boot validity for an exact result of one, tests
  RAM:001009 only when requested, and clears RAM:00dcc1.
- The command records diagnostic state seven at RAM:000c2a. Self-test failure
  reports sense 04/4000; accepted self-test and parameter paths clear sense.
  C251 emits the data-out tuple in DR4/DR0, sense arguments in ACC/WR6, and a
  native ERET.

## Current-host command dispatch

- Firmware 1.04 RAM:fa34b2 first marks RAM:000c89 bit seven when that bit is
  clear and signed RAM:2ba52e exceeds signed RAM:2ba532. RAM:2ba5b1 then has
  priority over RAM:2ba5af: the first latch is consumed and starts the primary
  20/60-tick countdown, while the second sets RAM:000c8a bit zero and starts
  the secondary ten-tick countdown.
- With neither latch set, RAM:000c83 bit four selects its cleanup helper.
  Otherwise RAM:000c78 zero dispatches the ATA task-file command and may
  restart firmware; nonzero dispatches the active CDB opcode through the
  four-byte pointer table at RAM:f83ef7. An addressless A251 adapter preserves
  the canonical opcode load, multiply-by-four, pointer load and indirect
  ECALL, allowing the surrounding coordinator to remain maintainable C.
- Completion bit one suppresses ordinary ATA completion publication. It is
  consumed unless RAM:000c89 bit four is set and the current opcode is READ(6)
  08, READ(10) 28, READ(12) a8, or READ CD be; those four commands restore bit
  one and consume RAM:000c89 bit four. Without bit one, bit zero skips status
  preparation and is consumed before completion publication.

## Active-operation host-command service

- Firmware 1.04 RAM:fa4103 treats RAM:2ba5b0 as a completion latch only when
  it equals one. That route consumes the latch, sets RAM:001588 bit two,
  routes a pending drive-initialization action to kind one for FORMAT state
  two or kind three otherwise, updates the matching c8a/c8b flag, and clears
  RAM:000c89 bit zero without scanning or dispatching.
- The ordinary route normalizes FORMAT states one and three through
  RAM:000c8b bit zero, clears RAM:000c89 bit zero, and scans controller-record
  banks with ACC=1, DR4=RAM:001153 and DR0=0. A zero result snapshots a fresh
  RAM:001153 once to RAM:000ce5 then RAM:000ce1. A nonzero result independently
  reads RAM:2b9704 for RAM:000ce1 and again for RAM:000ce5.
- Host dispatch and completion then follow the ATA/indirect-SCSI, restart,
  bit-four cleanup, bit-zero consumption and bit-one suppression rules used
  by RAM:fa34b2, except this service always consumes asynchronous bit one and
  has no four-opcode deferred-READ restoration.

## Active-operation packet admission

- Firmware 1.04 RAM:fa457d leaves pending state one untouched. State 0x10 sets
  RAM:000c8a bit one and becomes state one. Only state 0x11 reads exactly
  twelve bytes from controller port RAM:018000 into RAM:000b0a..000b15 and
  proceeds to SCSI admission; state four and all other values become state one
  and return.
- Admission failure prepares and publishes completion. Admission success reads
  the opcode once for classification. Opcodes 04, 08, 0a, 28, 2a, 2e, 35,
  a1, a8, aa, b9, bb and be set RAM:000c8a bit one and defer. START STOP 1b
  dispatches only for byte four equal to three; other values defer, and value
  two also sets RAM:000c8b bit zero.
- GET EVENT 4a clears RAM:001588 bit two when RAM:2ba5b0 is nonzero before
  dispatch. It and every other admitted non-deferred opcode use the verified
  RAM:f83ef7 table adapter, then prepare and publish ATA completion.

## Controller three-byte packet capture

- Firmware 1.04 RAM:fb94e1 begins accepted. P0.7 clear rejects immediately;
  P0.7 set with signed-negative RAM:01802a skips the counter and drive-state
  gates but still accepts and copies RAM:01802b..01802d to RAM:001133..001135.
- For signed-nonnegative status, RAM:00fc21 bit zero increments the 16-bit
  counter at RAM:00fc22, and RAM:00fb69 bit zero increments the 32-bit counter
  at RAM:00fb6a. RAM:001308 equal to one saturates RAM:00fb95 at 0xff and,
  when indexed state four is active, increments RAM:00fb96 below 0xffff only
  inside the inclusive signed window from lead-out minus 0x34bc through
  lead-out minus 9000. Both window comparisons freshly read lead-out and the
  decoded position.
- Drive-control state one rejects when RAM:001301 bit zero is clear. Bit one
  independently advances RAM:001235 even after that rejection. Accepted
  packets clear RAM:001136 and set RAM:00fb5c bit seven; rejected packets set
  RAM:000fad bit two and publish the same shadow to RAM:018003.
- RAM:000ff5 bits one or two call the near flag-word consumer, with
  short-circuit fresh word reads. A fresh bit-three test calls the extended
  D8/D9 measurement consumer. The packet-capture function itself returns by
  ERET, correcting the stale earlier RET description.

## Controller measurement recovery fallback ABI

- Firmware 1.04 RAM:f9599d owns its R14 retry counter and R15 restart-timeout
  value locally; neither is an inherited input. The timeout begins at one and
  becomes zero only after a tolerance check requests a measurement restart.
- Its final near series evaluator at RAM:f95c1c consumes that fallback in
  inherited R15. A maintainable C caller can preserve the contract with a
  narrow addressless adapter that accepts the explicit C byte in ACC, moves
  it to R15, performs the near LCALL, and returns the evaluator's ACC result
  through ERET.
- Acquisition failure permits three successful controller reinitializations
  and fails on the fourth acquisition result. Measurement status four permits
  three retries and then publishes failure five; status three publishes 0x1e.

## Controller state-six establishment

- Firmware 1.04 RAM:fb0f85 treats the path-validation and state-five results
  as generic nonzero success, but every port-0x80f8 retry, parameter-FB
  calibration and finalization result must equal one exactly.
- When RAM:00fbec is zero, a successful first retry is followed by a mandatory
  channel-zero reset, pickup homing and another retry. A failed first retry
  instead gets one recovery attempt; success publishes RAM:00fbec=2, while
  failure toggles RAM:00fb98 between zero and two and sets RAM:001304 bit two.
- The media-state-two readiness loop stores five at RAM:001209, repeatedly
  reads the signed interrupt-owned countdown, and tests P0.7 only while that
  countdown remains positive. Expiry sets RAM:001304 bit two and RAM:00fb98=1.
  Ghidra's decompiler removed this native RAM:fb10d9 block as unreachable, so
  operand-level review is required to preserve the real asynchronous route.

## Current-drive-state calibration routing

- Firmware 1.04 RAM:fb1e1f exits successfully without calibration when
  RAM:001304 bit zero is clear. When controller operation is enabled it yields
  once before routing; the disabled route does not yield.
- For RAM:001308 other than one, disabled operation uses RAM:00fab4 bit five
  to choose stored-word versus full calibration, while enabled operation uses
  bit two and conditionally gates the full calibration with an exact-one
  ten-sample P1/CEX0 pulse result.
- RAM:f8e1e6 is typed as a void side-effect wrapper, but its terminal ECALL to
  the word-0319 calibration leaves a Boolean in ACC and RAM:fb1e1f branches on
  that residual value. The maintainable RAM:fb1e1f C unit reconstructs the
  wrapper's word-0x0204 submission/mirror and returns the subordinate Boolean
  explicitly instead of depending on an untyped incidental register result.

## Controller calibration-path sample validation

- Firmware 1.04 RAM:fb10f1 performs the fixed no-timeout port-0x80f8 wait with
  DR0=0, R7=0 and ACC=1 three times. The existing near addressless adapter is
  the safe C boundary for this otherwise function-local register tuple.
- RAM:00fab4 is tested as a signed byte after the first delay sequence: a
  negative value selects E8/E9 value zero, while a nonnegative value selects
  RAM:00fab5. This is independent of the later ATIP-required state gate.
- With RAM:00fb98 zero, port-0x80c7 samples 0..13 fail immediately without
  publishing calibration-failure state, samples 14..79 run a state-two
  calibration pass, and samples 80..255 are accepted without that pass.
  Failed initial or state-two calibration sets RAM:001304 bit two and
  RAM:000e41=0x16.

## Controller operating-state initialization countdown

- Firmware 1.04 RAM:fb0ce4's operating-code-four path is not terminal. After
  successful port calibration it stores 50 at RAM:001220 and spins while a
  fresh signed byte read remains nonnegative. The interrupt owner eventually
  decrements the byte below zero.
- On expiry, native RAM:fb0e0f restores the saved controller word 0x0204 and
  joins the same state-five/current-drive-state calibration tail used by the
  active drive-control route. Ghidra removed this block and rendered the loop
  infinite, so source reconstruction must follow operands rather than that
  structured pseudocode.
- Successful completion sets RAM:001304 bit three and then clears bit zero in
  a separate fresh read/write. It runs controller command 03 only when a fresh
  pre-publication bit-zero test is nonzero.

## Controller operating-state reset

- Firmware 1.04 RAM:fb1f9a is a no-input ERET controller reset. It saves P1.1
  and then P1.0 individually, clears each immediately after capture, restores
  them after the register-D4/D5 initialization, and later sets P1.1/P1.0 twice
  in that order. These direct-bit operations are ordered hardware effects.
- Its port-0x80ac command writes 0xdf and polls fresh reads until the low seven
  bits are zero, with no timeout. The subsequent DriveControlFlags1304 bit-zero
  test is fresh and independently controls profile-zero loading.
- The four local register helpers at RAM:fb21aa, RAM:fb215a, RAM:fb21d3 and
  RAM:fb21ef are near RET functions; their C declarations and definitions must
  retain `PLEXTOR_NEAR_FUNCTION`. RAM:fb22ba is also a near RET tail even though
  the enclosing RAM:fb1f9a reset returns through ERET.

## Port-0x80f8 calibration attempt

- Firmware 1.04 RAM:fb1431 accepts its mode in ACC and returns Boolean through
  near RET. Only mode exactly one toggles the controller flag and E8/E9 values.
  Its two RAM:f8e2b0 calls use the fixed tuple DR0=0, R7=0, ACC=0; this differs
  from the established unbounded adapter whose ACC is one.
- The signed RAM:001220 sampling loop increments its wrapping byte error count
  only when the configuration byte at RAM:f7c01f is nonzero, RAM:001008 is not
  two, the fresh countdown has fallen by at least two, and status bit zero is
  clear. Final ordinary acceptance requires status bit zero set and fewer than
  four counted errors.
- When DriveControlFlags1304 bit zero is exactly one and RAM:00fbec is zero,
  the ordinary error-count gate is replaced by a signed 250-unit shared-timer
  wait. Status bit zero clearing before expiry resets channel zero to 0x99 and
  fails; timer expiry while the bit remains set succeeds.

## State-six sample-series calibration

- Firmware 1.04 RAM:fb1600 is a no-input Boolean ERET worker. It computes two
  fifteen-byte series from the D8 and D9 histories: endpoints are copied and
  indices 1..13 are three-sample integer averages. It copies each series to
  RAM:00f9b7, sorts twenty bytes after padding indices 15..19 with 0xff, and
  derives each candidate as `0xc0 - 8 * source_index`.
- A unique sorted minimum is accepted only when the next two sorted values map
  within two source positions of the first. A repeated minimum averages the
  candidates for the complete equal-value run and requires each source index
  not to exceed `first_index + run_length - 1`. RAM:001303 bit six bypasses
  acquisition and stability failures; without it failures publish
  RAM:000e41=0x15.
- The signed RAM:0011fc, RAM:001209 and RAM:001215 loops are interrupt-owned
  asynchronous waits. Ghidra's structured decompiler discarded the tail at
  RAM:fb1a51 and later blocks, but the native instructions continue through
  channel, word, parameter, operating-range and pulse-result publication.
  Native leaves the accumulated-candidate frame word stale on an indexed
  diagnostic-override route; maintainable C initializes it to zero and records
  that narrow behavior difference rather than invoking undefined C behavior.

## State-six operating-calibration finalization

- Firmware 1.04 RAM:fb1aef is a no-input Boolean near RET worker. Its initial
  RAM:001215 waits use signed-positive comparisons and require RAM:0011ef bit
  one to remain set; failure freshly ORs bit two into RAM:001304.
- The controller-port-0x802a wait treats signed RAM:001215 zero as active
  (`JSGE`), rereads port bit six after that inner wait, and resets its local
  failed-pulse count only for pulse result exactly one. Three consecutive
  non-one results terminate the retry phase even while signed RAM:001209 is
  still positive.
- Positive remaining RAM:001209 forces RAM:001006 to one and normalizes
  RAM:001008 to one unless it is two. Expiry in mode two strobes ports 0x803b
  and 0x8079, calls the measurement-tolerance worker with ACC=0/R7=0, clears
  RAM:00fb98 and fails. Indexed state-five outcome publication clears
  RAM:00fab4 masks in three separate fresh operations for nonzero mode, then
  the terminal signed RAM:001209 wait accepts only a set P0.7 bit.

## START STOP UNIT action matrix

- Firmware 1.04 RAM:fb2520 validates CDB mask 0xff8e0c and dispatches byte
  RAM:000b0e values zero through three; every other value uses the unsupported
  command path. RAM:000b0b bit zero selects immediate behavior.
- Actions zero and one require a clear status byte and media-ready bit, drive
  P3.0/P3.1 low before stopping or loading, and finish with P3.0 high/P3.1
  low. Action one copies the big-endian request dword at RAM:000e3b into
  RAM:000c8e before synchronous dispatch or immediate queueing.
- Action two rejects the medium-removal and active-read states, finalizes a
  pending write in the exact ATA-completion/write-preparation order, then
  coordinates drive states nine and ten around the eject worker. Action three
  requires RAM:000c84 bit two, clears bit one, runs the load worker, and maps
  terminal status three or fifteen to 02/0401 or 02/5300 according to the
  immediate mode.

## FORMAT UNIT top-level geometry

- Firmware 1.04 RAM:fb2af5 accepts only CDB byte-one low-five-bit format
  codes 0x11 and 0x17 after validating mask 0xff8e6f, write type zero,
  option bit one, track mode five after masking bit one, nonzero packet size,
  data-block type at least eight, drive state two, and both controller gates.
- Code 0x17 validates twelve payload bytes against the mask table at
  RAM:f78e83, requires RAM:000b2d=8, derives format flags from RAM:000b32
  bits seven and six, and computes packet overhead as
  `7 * (block_count / packet_size) + 0x91`. Frame limits use signed 32-bit
  comparisons even though the geometry arithmetic wraps as unsigned.
- Later-session reference recovery scans eleven-byte descriptors in bank
  RAM:2b from index one, matching the preceding session and descriptor type
  0xb0, then converts the three binary MSF bytes at descriptor offsets four
  through six. The native temporary stack copy inherits R9 incidentally; the
  maintainable C path converts those bytes directly and has no hidden-register
  dependency.

## FORMAT UNIT successful track-state commit

- Firmware 1.04 RAM:fb32e0 saves R15 before loading it from SelectedTrackMode
  and masks bit one locally. R15 is therefore callee-saved scratch, not an
  inherited function input; a normal near C function is sufficient.
- New-disc creation installs type four/track one with start frame 0x96;
  existing-session creation uses LeadInLastTrackNumber and start
  `RAM:00e616 + 0x97`; append updates the loaded record and always propagates
  RAM:00e612..RAM:00e61e into the working boundary fields.
- Parameter byte RAM:000c45 equal to 0xc0 commits the current record and
  creates a following type-zero record ending 0x1194 frames after RAM:00e61e.
  Other values mark the table active and clear both reference indices. Every
  path sets BITS:000a and performs a final table commit, even when an
  existing-session tail gate skips record population.

## FORMAT UNIT failed track-state commit

- Firmware 1.04 RAM:fb3598 mirrors the successful track commit but publishes
  working-track type five and captures the failure position in RAM:00dd51.
  New-disc failure initializes that position to 0x96 without calling the
  data-block-field helper; existing-session failure calls the helper and then
  recopies the resulting working start frame into RAM:00dd51 even when its
  tail-population gate was skipped.
- The existing-session failure route loads TrackTableReferenceEntryIndex
  (RAM:00dd7e), while append failure loads LeadInLastTrackNumber
  (RAM:0012c0). Extension indices on both routes are nevertheless derived
  from LeadInLastTrackNumber plus one. The function shares the success path's
  callee-saved-local R15 pattern and near RET contract.

## FORMAT UNIT primary staged raw-Q plan

- Firmware 1.04 RAM:fb3863 always emits four ten-byte records at RAM:2bc11b
  and publishes count four at RAM:2bc119. The record sequence is A0, A1, A2,
  then the active track; all unused tuple bytes are explicitly cleared.
- The function queries the effective last track once. Append mode uses it for
  A0, A1 and the active-track point; a new disc uses track one; an existing
  nonappend session uses LeadInLastTrackNumber plus one. A2 encodes
  RAM:00e61a, while the active-track record encodes RAM:00e616+0x97 for
  nonappend or WorkingTrackStartFramePosition for append, with all three MSF
  bytes converted separately to packed BCD.

## FORMAT UNIT secondary staged raw-Q plan

- Firmware 1.04 RAM:fb3b5c builds at RAM:2bc519 and publishes the 16-bit
  count at RAM:2bc517. Operation 0x40 emits B0 with an all-0xff first tuple;
  operation 0xc0 emits B0 with the packed-BCD MSF of RAM:00e61e+0x1194;
  operation zero omits B0 unless PMA records exist or media state is two.
- PMA ADR-3/5 records follow the optional B0. C0 is emitted only when B0
  exists, its suppression gate allows it (or media state is two), and signed
  RAM:00dd69 is negative. Media state two then appends C1. When B0 exists its
  separator byte at RAM:2bc51f is finalized from the total record count via
  PackedBcdByteToBinary.

## Indexed-state-0x0f coroutine C safety

- Firmware 1.04 RAM:fb3ed6 is scheduler-installed and nonreturning but does
  not manipulate SPX, pop a saved continuation, or perform a nonlocal ERET.
  Its control flow is ordinary C around existing YieldCoroutine0 and
  InitializeCoroutine1 boundaries, so the body itself does not require
  assembly.
- Strategy indices 0x91, 0xb1 and 0xd1 select write speeds 4, 10 and 24.
  Other indices publish terminal status one. The fixed plan uses packet size
  32, block count 180000 and overhead `7*(180000/32)+0x91`; operation failure
  publishes status four, reference-payload failure status six, and success
  installs coroutine one first at RAM:f84f6a and then RAM:f84b0b before an
  infinite yield loop.

## SYNCHRONIZE CACHE(10) command state

- Firmware 1.04 RAM:fb42dc validates mask 0xff8da7. A clear RAM:000c84 bit
  three with nonnegative signed RAM:000c84 takes the immediate no-sense exit;
  otherwise CDB byte-one bit one sets the temporary completion bit and invokes
  no-sense completion before processing continues.
- Negative signed RAM:000c84 clears BITS:0030 and BITS:0035 and dispatches
  write type one to the close/cache operation, types two and three to command
  sequence error, and type zero to working-track repair/cache finalization.
  The nonnegative active route instead invokes RAM:fb4471 and restores the
  selected speed for write types two/three at RAM:001592 >= 0x51, or for other
  types when signed RAM:001153..001156 >= 0x00058fd4 and state RAM:000c9e is
  zero.
- Countdown states one/two select the 20/60 primary timer when the two media
  gates are clear, or the 10-unit secondary timer for negative RAM:000c89 when
  the alternate gate is set. State three independently selects that secondary
  timer for negative RAM:000c89. Every full route clears RAM:000c85 bit zero
  and RAM:000c84 bit three.

## Active write-pipeline finalization

- Firmware 1.04 RAM:fb4471 first clears the four-byte host-transfer block
  count at RAM:001455. BITS:0035 ownership triggers clears of BITS:0035 and
  BITS:0030, an optional RAM:00fb9a yield drain when RAM:000c9e is zero, and
  producer restart when BITS:0032 is set.
- Write type one with working status two acquires the signed-positive missing
  tail `(end - 2) - cursor` only when signed `cursor + 2 < end`. State zero
  then handles optional fixed-packet padding and prefix preparation; nonzero
  state executes the command block range and converts readback bit 0x20 into
  completion bit 0x40 after verification.
- Write types one and two build raw-Q state. Speeds below ten make one attempt;
  higher speeds retry at speed four after an initial failure, optionally run
  OPC when option bit zero is clear, and restore the selected speed on every
  retry exit. OPC failure reports sense 03/7304. Type zero advances or directly
  increments packet state, while the final dispatch selects type-one/type-two
  track finalizers, the type-three RAM:f84c30 coroutine restart, or opcode
  0x35 track finalization.

## Pending host command service during media operations

- Firmware 1.04 RAM:fb4739 is an ordinary near-RET service despite the native
  body's saved R15 scratch. Pending state 0x11 consumes exactly twelve reads
  from controller port RAM:018000 into CDB RAM:000b0a..000b15; state 0x10 sets
  RAM:000c89 bit two, normalizes the pending state and, when enabled, programs
  ports 0x8056/0x8050/0x8051/0x804a with 0x41/4/3/0xc0.
- When both active-operation gates are set, only REQUEST SENSE and a nonzero-
  state READ TRACK INFORMATION are dispatched; other commands receive 02/0404
  or 02/0408. With only the secondary gate and no shared completion latch,
  opcodes 08, 0a, 28, 2a, 2e, 35, 5b, a1, a8, aa, b9, be and 04 are deferred,
  as is START STOP with byte four equal to one.
- The ordinary readiness policy always admits opcodes 03, 12, 46 and eb;
  GET EVENT clears RAM:001588 bit two for state two with the shared latch;
  READ TRACK INFORMATION requires nonzero RAM:000c9e. Other opcodes select
  tray handling or sense 02/0404 versus 02/0408 from RAM:000c8b/000c87 bit
  five. Nondeferred paths always prepare then publish ATA completion.

## CLOSE TRACK/SESSION command routing

- Firmware 1.04 RAM:fb4a22 validates mask 0xff8cd4 and captures inherited B
  for its later RAM:fc3731 track-loader call. Media states one and three only
  finalize an active write and clear command bits; state two clears controller
  state bits, optionally completes immediately, rebuilds controller-record
  state through coroutine two at RAM:f84a00, and records preparation failure.
- Selector two validates session table state. Parameter 0xc0 with high index
  at least 99 and type four at RAM:00e4e0 temporarily changes the parameter to
  0x40 for session completion, then restores it and scans Full TOC descriptors
  backward for point 0xb0, replacing its three following tuple bytes with
  0xff. Selector one resolves track 0xff from the high index and enforces the
  reference/high bounds before loading the working record.
- Test-write option bit zero rejects execution. Track types zero/four/seven
  select unsupported/no-sense/sequence-error outcomes; type two requires write
  type one, types one/three require write type zero and write-state validation,
  and types one/six also require the PMA-start gate. Accepted selector one uses
  the shared close/cache operation; selector two uses OPC-backed session
  completion. Both clear RAM:000c85 bit zero and RAM:000c84 bit three.

## Controller event-bit-two measurement progress

- Firmware 1.04 RAM:fb621b routes controller event bit two through an early
  buffer-setup advance, an error-triggered measurement reinitialization, or
  the ordinary RAM:fb65d8 setup service before dispatching
  ReadMeasurementControllerSubstate. Substates 3, 7, 8, 10 and 12 program
  distinct `0x80c0/0x80cf/0x80c3/0x80c6` profiles; notably substate 8 writes
  `0x80c0=0` but uses the high `0xc4/0xc0` `0x80cf` profile, unlike substates
  3 and 12, which use `4/0`.
- Substate 13 advances the big-endian queued count at RAM:001292 by five and
  moves the consumer at RAM:001294 backward by five under the RAM:00129a ring
  mask. The optional operating adjustment is an unsigned 16-bit sum capped at
  126. Out-of-window completion clears BITS:002b/BITS:0043, derives the next
  big-endian target as RAM:001235 minus two, masks the three published control
  bytes, and finalizes the controller bit-four update.
- RAM:fb65d8 publishes descriptor-derived 24-bit controller addresses
  low-byte first at ports 0x8007..0x8009 and 0x800a..0x800c. Its two-byte
  address-four value is likewise low byte at 0x801c and high byte at 0x801d.
  The RAM:00fbae ring cursor is 16-bit, but its wrap comparison is against the
  zero-extended 32-bit slot count at RAM:00156a; the wrapped subtraction uses
  that count's low word.

## ATIP controller initialization

- Firmware 1.04 RAM:fb8085 owns WR14/R15 as retry and retry-request locals and
  returns through ERET; they are not inherited inputs. A failed controller
  initialization is recoverable only when the initializer reasserts
  RAM:001304 bit two after the caller clears it. When RAM:f7c01f equals one
  and RAM:000e41 equals seven, that failure advances the retry count by three
  total; exhausted failures map result 0x16/0x15/other to status 0x16/0x15/0x14
  and diagnostic byte 6/5/3.

## PMA acquisition

- Firmware 1.04 RAM:fb8497 is an ordinary no-input ERET worker returning
  status zero or four in ACC. Its first PMA-table reset call reaches a thunk
  at RAM:fb8be5 that clears R3 before entering the shared reset tail, so R3 is
  not inherited input; saved R13/WR14 become function-owned pulse/control
  counters. It uses signed RAM:001203 budget checks, lead-in-minus-1000 as its
  anchor, a Timer 1 quotient from signed 0x1733 divided by operating code four,
  and finalizes only when RAM:fb87de returns exactly one.
- Firmware 1.04 RAM:fb89c5 is an ordinary no-input near worker returning true
  in ACC. Accepted-Q mask bits zero through five independently select ADR-1
  through ADR-6 handling. ADR-1 copies its indexed record only when its signed
  adjusted frame position is at most RAM:00124c, treats table byte three as a
  high-time marker at values 0x5f and above, and maps current-Q control bit six
  to Full TOC summary bit one when set and bit zero when clear. ADR-2 is a
  singleton; ADR-3 through ADR-6 update byte high-water indices before their
  ten-byte table copies.

## WRITE range coordinator

- Firmware 1.04 RAM:fbbb0d is a normal near worker with signed start LBA in
  DR4, unsigned transfer count in DR0, no scalar result and RET. Saved DR12 is
  callee preservation rather than inherited input. New ranges set command
  bits three then four, map the start address, reset raw-read state, initialize
  coroutine two at RAM:f84a00 and yield before OPC/raw-Q preparation; continued
  ranges instead reconcile the requested start. Both validate the extent and
  publish the transfer count before starting or resuming the raw-read pipeline.
- The coordinator's native calls prove RET contracts for RAM:fbd495,
  RAM:fbd098, RAM:fbc33d, RAM:fbc6f7, RAM:fbcde5, RAM:fbc6b0, RAM:fbc80f and
  RAM:fbd262. Their C declarations must be near so C251 emits LCALL rather than
  silently converting these established boundaries to ECALL/ERET.

## Vendor F2 shared measurement response

- Firmware 1.04 RAM:fbdcde is a no-input near-RET response worker. It copies
  30-byte series RAM:00f11b and RAM:00f139 to RAM:000bb0 and RAM:000bce,
  respectively, and publishes big-endian words RAM:00f060, RAM:00f062 and
  RAM:00f064 at RAM:000bee, RAM:000bf2 and RAM:000bf6. Mode three directly
  publishes RAM:00ef85/RAM:00ef87 as the final pair.
- Other modes read a signed table byte through the big-endian pointer at
  RAM:00f1bb and a float32 measurement at RAM:00f06e. The first pipeline
  narrows after multiplying `table * 0.001 * measurement` and again after
  adding 4.6592, then ceilings division by 0.0043 and offsets RAM:00f074 to
  form the reference word. The second pipeline computes
  `((delta * 0.001 - measurement) + 12.0) /
  ((delta * 0.0089 + 14.3) * 0.01) + RAM:00f06a`, narrows to float32, ceilings
  it, and publishes its low byte with the reference word at RAM:000bfa and
  RAM:000bfc.

## Vendor F3 selector 0Dh controller mode

- Firmware 1.04 RAM:fbe343 takes the requested mode in ACC and returns through
  ERET. It rejects negative RAM:00fab4 or controller state two in bits four and
  five of RAM:00103b. Accepted modes persist at RAM:00f1ba, install speed/code
  two, and synthesize fallback strategy index four and lead-out frame 0x514c8
  for every mode except two.
- With global interrupts disabled it first sets controller shadow RAM:00fca
  bit three and publishes it to port 0x8002. Modes one/three write ports
  0x80b9/0x80ba/0x80bd as 0x40/0x50/1, mode zero writes 0x40/0x50, and mode
  two writes 0x30/0x10. It then clears shadow bits three/four, republishes the
  shadow, and enables EA, EC and SFR A9 bit two.
- Mode one reads the big-endian retained word RAM:00f087 once for the 0xffff
  gate and again for application. The final P3 RX/TX and drive-control bit-four
  policy selects 1/0 and clears the flag only when RAM:000c8a bit three is set
  and RAM:f7c01f is zero; otherwise it selects 0/1 and sets the flag. Accepted
  completion publishes controller state one in RAM:00103b bits four/five and
  clears sense.

## Vendor F3 measurement-result response

- Firmware 1.04 RAM:fbe744 uses CDB byte two's high nibble to select the
  controller-command-0E family; high nibble zero selects eight-channel data,
  whose low two bits choose the 16-byte compact form only when both are zero
  and the 26-byte extended form otherwise. Unavailable results are explicitly
  zero-filled, while available start positions are serialized after wrapping
  subtraction of 150.
- The compact eight-channel response publishes sums zero and five; the
  extended response publishes all eight sums. The command-0E response folds
  control bit five into response byte three, publishes control bit six at
  response byte fourteen, and emits primary/companion averages. Each result
  status is read once for publication and again for its availability branch.
- The unsupported command-0E capability route still reaches DATA IN and uses
  inherited R15 as its transfer cap. Other forms use 16 or 26; all are clipped
  by CDB byte ten. Transfer status four preserves readiness, while every other
  result clears RAM:001098 bit five and RAM:001082 before clearing sense.

## Vendor F5 media-strategy response

- Firmware 1.04 RAM:fbeb4c rejects absent media readiness with sense 02/0401;
  otherwise it clears exactly 82 bytes at RAM:000b2a, publishes response length
  0x50 and the common ATIP/strategy fields, clips the big-endian CDB allocation
  word at RAM:000b12 to 0x52, and performs DATA IN. Transfer status four
  preserves sense; every other result clears it.
- Fixed-media state one serializes RAM:00fb5e/63/65/67, RAM:00f964..f966,
  computed words RAM:00e752..e758, a truncated float32 product
  `(BE16(RAM:00f9fa) / 100) * BE16(RAM:000ffe)`, RAM:00f968..f96b, three
  big-endian dwords RAM:00f9ee/f2/f6 and RAM:00fad4. The capability word from
  RAM:00fad7 is bit-reversed and forced odd when RAM:00fc21 bit two is set.
- Recordable family two indexes 11-byte profiles at RAM:00f83084 and three-byte
  auxiliaries at RAM:00f83469; family four additionally uses RAM:00f831ad/
  RAM:00f834ba and RAM:00f832b5/RAM:00f83505 with fixed blocks RAM:00f83451
  and RAM:00f83459. Signed profile bytes are sign-extended into big-endian
  words. Native family values one, three and five reach this path with an
  uninitialized auxiliary pointer and therefore remain undefined behavior,
  not a verified table format.

## Raw-read pipeline speed recomputation

- Firmware 1.04 RAM:fbf444 is a call-free ERET leaf. Before transition state
  five it starts at RAM:000c37 and applies ordered caps at 40x, 32x, 24x, 8x
  and 4x from media mode, BITS:000c/001c/001d, active-write state, command B9,
  write speed, recovery, raw format, readiness, queue position and retry
  state. RAM:00faef bit zero also reduces RAM:000e47 to 8x.
- An active RAM:001591 queue latch is retained and forces 8x only while the
  signed big-endian dword RAM:001598 is at least 150 and below the zero-
  extended big-endian word RAM:00129c; otherwise the latch is cleared. A retry
  count RAM:0015e2 of at least ten forces 8x when the current speed is at least
  8x, but clears the count when the speed is below 8x.
- The primary path publishes its chosen speed in order to RAM:000f3a,
  RAM:000e49 and RAM:000f45. BITS:000f can then override only RAM:000e46 to
  4x, so the published values deliberately differ. BITS:001b and BITS:0021
  are independent final overrides; each writes 8x to RAM:000f45, RAM:000e46
  and RAM:000e49, even when both are set.

## Raw-read fixed-pattern descriptor validation

- Firmware 1.04 RAM:fbf806 takes an unsigned descriptor count in WR6 and
  returns normalized Boolean success in ACC through RET. It seeds the
  secondary cursor RAM:001298 from consumer index RAM:001294, writes media
  coroutine state three and RAM:000e3f value one, and yields once before each
  descriptor inspection.
- The descriptor stores the signed big-endian frame at offset two and the
  big-endian payload pointer at offset six; validation starts sixteen bytes
  into that payload. Offsets 0..23 must contain zero, ten 0xff bytes, zero,
  the packed-BCD absolute MSF plus marker one, the big-endian frame minus 150,
  and the same packed-BCD absolute MSF plus marker one.
- Payload offsets 24 through 0x80f comprise 510 four-byte words, each byte
  equal to the low byte of frame minus 150. A mismatch does not short-circuit
  the remaining checks within that descriptor, but prevents cursor advance
  and ends the batch afterward. Passing descriptors advance RAM:001298 as
  `(cursor + 1) & RAM:00129a`.

## Raw-read fixed-pattern range acquisition

- Firmware 1.04 RAM:fbfc9f takes a signed start in DR4 and unsigned count in
  DR0, returning signed ACC status through RET. It clears RAM:000f0c,
  BITS:000c/000d/001d/001c/0013 and special raw-read state, publishes the start
  to RAM:0015dc and RAM:000b02, derives the batch ceiling as the big-endian
  RAM:00129c word minus sixteen, and rejects an end at or beyond
  `BE32(RAM:001153) + 0x19c6` with status 0xfd.
- Each in-range batch resets the queue, points coroutine two at RAM:f90000,
  resets the transfer target and polls until status differs from 0xfe. Status
  zero becomes 0xff. An operating code at most four retries the same range;
  the third such result becomes 0xff. Higher codes validate the batch through
  RAM:fbf806, returning 0xfe on mismatch and advancing start/remaining only on
  success.
- Every in-range exit restores state even on failure: it clears RAM:001037,
  selects operation seven, resets the queue, points coroutine two back to
  RAM:f84a00, publishes target 150, dispatches request five and yields once.
  The initial 0xfd boundary rejection is the only route that skips restoration.

## GET PERFORMANCE command dispatch

- Firmware 1.04 RAM:fc036c validates against RAM:ff8ead. CDB byte ten selects
  type zero nominal/write descriptors or type three table descriptors; other
  values reject. For type zero, CDB byte one low three bits select nominal
  zero or write-speed four, with other selectors rejected. The big-endian CDB
  word at RAM:000b12 limits descriptors rather than response bytes.
- Type-three records start at RAM:ff8c59, have five-byte stride, and are bounded
  by the count at RAM:ff8c54. Generic/no-media selection uses record byte two
  bit seven. Fixed media with a nonzero RAM:00fbec and lead-out within
  0x20f59..0x4ce77 uses record byte zero bit six; outside that range it uses
  byte two bit seven. Recordable media uses record byte zero masks 4, 0, 1 or
  2 from RAM:00fb88 and RAM:00fb80.
- Each table-helper call inherits native WR14 as
  `(wrapping_u8(5 + 5 * index) << 8) | index`; this value is semantic input to
  RAM:fc02b2 and must be explicit in C. GET PERFORMANCE append functions at
  RAM:fc0000, RAM:fc005d, RAM:fc0106, RAM:fc024e and RAM:fc02b2 are all near
  RET boundaries, so their declarations and definitions require
  `PLEXTOR_NEAR_FUNCTION`.
- The response starts at RAM:000b2a with big-endian data length
  `response_length - 4`; RAM:000b2e is zero for nominal/table results and two
  for write-speed results. DATA-IN status four preserves sense; other results
  clear it.

## SET CD SPEED command dispatch

- Firmware 1.04 RAM:fc0789 maps the big-endian requested read rate at
  RAM:000b0c into 1/4/8/10/24/32/40/52x bands, records the uncapped band at
  RAM:00fc20, and caps active selection RAM:000c37 by RAM:00fc1f. The highest
  request remains 40x only when both RAM:000c88 and RAM:00faed bit two are
  clear; otherwise it selects 52x.
- The big-endian requested write rate at RAM:000b0e maps through the ordinary
  4/8/16/20/24/32/40/52x bands or the recordable-media 4/10/24x rules. When
  RAM:00fad7 word bit two is set, lower-or-equal signed requested speeds replace
  RAM:000c38 and clear the bit; higher requested speeds are staged in
  RAM:00fad3 without replacing the active speed.
- For active media state zero with RAM:000c85 bit one set, a changed read speed
  may be bounded by cached RAM:000f45, is published to RAM:000e46/000e49, and
  restarts the raw-read queue through coroutine-two target RAM:f84a00. Media
  state four directly publishes selected read and write speeds without that
  restart. The handler uses native ERET, not RET.

## BLANK command dispatch

- Firmware 1.04 RAM:fc0a70 implements BLANK types zero through six. Types two
  and three select a track, type four maps the CDB frame through fixed-packet
  overhead, and types five/six derive session ranges from Full TOC A2 and A0
  descriptors. Full TOC descriptors are eleven bytes at RAM:2b8000 plus an
  index-times-eleven offset; the scan is one-based and bounded by the
  big-endian descriptor-byte count at RAM:0012ca divided by eleven.
- Current-session A2 descriptors provide RAM:00e61a from MSF bytes at offsets
  eight through ten. Previous-session A0/B0 descriptors provide the reference
  from offsets four through six. First-session bounds use ATIP lead-in and add
  0x1a5e to the end; later sessions subtract 0x1194 from the reference and add
  0x08ca to the end.
- Accepted BLANK commands set RAM:000c84 bit three around raw-read reset, OPC,
  and plan execution. Success clears RAM:000c86 bit one, updates track state,
  optionally resets Type-2 records, and clears sense. Failure still updates
  track state and converts an IMMED request into RAM:000c85 bit three before
  clearing the temporary IMMED/active flags. The handler ends with ERET.

## Blank/format track-state finalization

- Firmware 1.04 RAM:fc166b always clears working-track state before dispatching
  BlankOrFormatOperationType zero through six, then always commits the working
  track and reloads TrackTableHighIndex. Unknown operation values still run
  that common clear/commit/reload tail.
- Operations zero and one install the same canonical empty track model:
  control 0x41, track/reference/high/session indices one, ATIP lead-in copied
  to RAM:00dd69, and all working/session frame bounds zero. Operation two
  classifies or truncates the selected track; three clears the effective last
  track; four publishes RAM:00e602 plus 145; five rolls back one session; and
  six selects full reset or tail truncation from RAM:00dd79, RAM:00dd64 and the
  tail-uninitialized result.
- RAM:fc1a28 `FindLastTrackTableIndexForSessionByte` and RAM:fc1b17
  `ClearTrackTableRecordAtIndex` are native near RET functions. Their C
  declarations and definitions require `PLEXTOR_NEAR_FUNCTION`; canonical
  RAM:fc166b calls both with LCALL.

## MODE SELECT parameter processing

- Firmware 1.04 RAM:fc1bb4 receives the parameter list through the reserved
  controller buffer and makes a validation pass before an application pass.
  Supported full-byte page selectors are 01, 02, 03, 05, 07, 08, 0d, 0e,
  1a, and 1d; page 01 length six bypasses the general length helper, and page
  1d is validated but intentionally has no application-stage copy or helper.
- The optional eight-byte block descriptor is consumed only for MODE
  SELECT(6). MODE SELECT(10) accepts a block-descriptor-length field of zero
  or eight but begins page parsing immediately after its eight-byte header,
  so a declared descriptor is interpreted as page data. This canonical quirk
  is covered by focused tests rather than normalized away.
- Accepted MODE SELECT(6) block sizes are 0x800, 0x804, 0x808, 0x914, 0x91c,
  0x920, 0x924, 0x930, 0x940, 0x990, and 0xa56. A changed value restarts the
  raw-read pipeline only when media status RAM:000fae is zero and RAM:000c85
  bit one is set. The function uses native ERET; all ten page validators, the
  page-length helper, and the page-03 applicator are native near RET workers.

## Controller-position sample acquisition

- Firmware 1.04 RAM:f89fc3 selects calibration target words RAM:00fb61,
  00fb63, 00fb65, or 00fb67 for write speeds 1, 4, 8, or
  16/20/24/32/40/52. Its signed frame target is RAM:000fe9 minus 0x489,
  minus 15 times RAM:000fee, plus five times `(3 - RAM:000fef)`.
- The capture loop treats parser result 0xfe as pending. At the exact target it
  sets RAM:000ff5 bit three; target+1 fails when that bit is not set. At
  target+4 with RAM:00f9a4 at most 0x10 it returns 11 when RAM:000ff1 is zero
  or three otherwise. At target+5 or later it clears the window/active flags
  and delegates the five-sample evaluation.

## High-write-speed calibration flags

- Firmware 1.04 RAM:f8a5ae runs only at write speeds of at least 16. Its
  calibration value is `max(0, 70 - (((signed(f99f + 120) - signed(fd5)) *
  8.35 / 1000) * 41.6667)) * signed(ffe) + signed(fd9 - facf)`, with float32
  rounding after the native compiler-helper operations and 16-bit wrapping
  before both signed conversions.
- The first four comparison thresholds are the big-endian words at
  RAM:00e752/54/56/58 plus 0x100 with word wrapping. The fifth is
  `signed(f9fa) / 100 * signed(ffe) + 256`, converted to a 32-bit integer and
  compared through its low unsigned word. Each metric comparison performs the
  same float-to-integer conversion and uses strict unsigned less-than.
- Flag selection is enabled when RAM:00fad7 bits zero and one are both set,
  RAM:001008 equals two, or RAM:00fad7 bit twelve is set. Only the first
  matching flag is ORed into RAM:00fad7, in order 0x0200, 0x0100, 0x0080,
  0x0040, 0x0020; existing bits are preserved.

## Media-state-6 calibration threshold

- Firmware 1.04 RAM:f89ae5 maps operating codes 1/4/8 to base bytes
  RAM:00f963/64/65 and encoded offsets RAM:00fadc/dd/de. Codes 16, 20/24,
  and 32/40 use RAM:00f966 plus zero, four, or ten and RAM:00fadf. High-speed
  routes add two to the optional decoded RAM:00fabf adjustment; all additions
  wrap as bytes.
- The summed threshold is clamped to zero when nonpositive as a signed byte.
  ConfigurationWordFAAC bit one cancels the separate adjustment. Speeds below
  12 compare candidate 0x21; other speeds compare decoded adjustment plus
  0x1e. An unsigned candidate at or below the threshold selects class three
  and replaces the threshold, while a final zero threshold selects class two.

## Synthetic raw-read pattern generation

- Firmware 1.04 RAM:febee5 initializes its producer counter to 45, so the
  first capacity check occurs after five descriptors; after a successful
  check it resets the counter and checks every 50 descriptors. A rejected
  check reports failure zero and deliberately leaves BITS:0030/BITS:0035 set.
- Each applicable synthetic sector is filled with the low byte of the
  magnitude remainder of `(ReadPipelineTargetAddress - 150) / 256`, then gets
  the fixed twelve-byte RAM:ff967a auxiliary pattern. Its descriptor repeats
  the packed three-byte MSF plus byte one at offsets 0x1c and 0x24 and stores
  the signed source frame at offset 0x20 before enqueue/Q/ring advancement.

## Track-packet LBA mapping

- Firmware 1.04 RAM:fd38b8 starts every mapping with a wrapping 150-frame
  bias. When BITS:000b is set, it scans 19-byte track records from the
  Full-TOC or parsed-TOC first track through the effective last track. A zero
  scan endpoint falls back to the signed dword at RAM:001153.
- A valid packet record requires bit zero at record offset 0x05, a nonzero
  big-endian packet size at offset 0x06, and a packet endpoint at offset zero
  not below the biased address. Nonnegative distance from the offset-minus-
  three packet origin adds seven frames per completed packet and publishes
  that adjustment at RAM:000f0c. A negative distance clears BITS:000c only
  when RAM:0015d7 is zero and BITS:0021 is not set; otherwise it preserves the
  prior flag.

## Raw-sector host transfer layout

- Firmware 1.04 RAM:fc549f reads descriptor type from the secondary raw-read
  descriptor at offset 0x13. With BITS:000d clear it validates the host block
  size and adjusts the descriptor's main-buffer address by 0x0c, 0x10, or
  0x18 for the observed 0x804/0x80c/0x924, 0x800/0x808/0x920, and
  0x914/0x918 layout classes. Size 0x0a56 first publishes length 0x930, then
  switches to the descriptor's offset-10 address and length 0x126.
- With BITS:000d set, RAM:fc549f uses the canonical tables at RAM:ff8eba,
  ff8efa, ff8f3a, and ff903a to validate and select main-data layout, address
  offset, byte count, and controller command. C2 contributes 0x126 bytes;
  format two contributes a normalized 16-byte region from the RAM:00156e
  ring and selects RAM:001556 minus 0x20, while other nonzero formats add a
  96-byte region selected from the RAM:001566 ring.
- RAM:fc549f adds RAM:001463 to the big-endian completed-block count at
  RAM:001459, sets RAM:001462, launches the host transfer from the product
  returned by RAM:fde9cf, and yields coroutine zero until RAM:001462 clears.
  Invalid-layout sense 05/6400 retains the canonical post-sense indirect read
  through RAM:6402 when constructing sense information. The function returns
  normalized Boolean status through native ERET.

## READ MASTER CUE response layout

- Firmware 1.04 RAM:fc62ac reads the queued descriptor's big-endian 32-bit
  controller-buffer address from descriptor offsets four through seven. The
  two word-load instruction bytes construct that dword even though the current
  live disassembly renders their destination/base registers in the opposite
  order; the surrounding register flow and the same compiler idiom at
  RAM:fbf806 establish the pointer construction.
- Descriptor selection atomically reads four bytes at controller address
  +0x1c. Byte one must equal CDB byte four, while byte zero in 0xaa..0xac or a
  zero byte three admits the descriptor. The response first transfers four
  zero bytes, then main data at controller address +0x10 in 0x800-byte chunks.
  Consecutive sectors are 0x930 bytes apart and their selector byte is read at
  sector base +0x0d. RAM:001298 is restored on both scan failure and successful
  completion.
- Firmware 1.04 RAM:fa8bdc initializes Type-2 controller-record geometry only
  when RAM:001008 equals two. It partitions the signed lead-out endpoint into
  5616-frame segments and 39-frame residual records, clamps endpoints above
  360000, rounds the active segment count into groups of eight, fills two
  eight-entry signed boundary tables, copies defaults into empty five-byte
  slots or imports occupied-slot overrides, and publishes the signed minimum
  of each active table. All persistent dword stores use low-word-first byte
  order; inactive table entries are exactly 0x00ffffff.
- Firmware 1.04 RAM:fa5c30 maps controller-record page `(group,page)` to CPU
  address `0x194000 + group*0x4000 + page*0x800` and controller address
  `0x094000 + group*0x4000 + page*0x800`. It walks backward from the selected
  page using unsigned 24-bit header boundaries at offsets 0x11 and 0x15,
  swaps six-byte boundary records when ordering crosses, then propagates
  forward. Payload end selection prefers header word 0x0a, then 0x0c, then
  `record_count*6+0x20`; payloads beyond offset 0x26 are shifted through
  controller scratch address 0x0b4000 using two word transfers.
- Firmware 1.04 RAM:fa56c5 mirrors the page reconciler in the opposite
  direction: it starts from the selected page's terminal six-byte record,
  scans forward while later first/last 20-bit positions still overlap, swaps
  into the next page when ordering crosses, then propagates backward. For a
  payload end beyond 0x26 it shifts `(end-0x26)/2` controller words in place
  from page controller offset 0x26 to offset 0x20 before writing the displaced
  terminal record and refreshing the 0x11/0x15 header triplets.
- Firmware 1.04 RAM:fa538a rebuilds a selected controller-record page header
  in RAM:00fa08..00fa27 after compacting the page. RAM:2ba51c holds the
  controller-side payload start and RAM:2b9710 holds each type-0x20/type-0x31
  search result. Type 0x20 determines the compacted offset and upper summary;
  type 0x31 supplies fallback tail summaries. Missing type 0x20 zeroes the
  offset, fills both summary triplets with 0xff, and clears the page-presence
  bit before the final 32-byte +1-MiB-alias writeback.
- Firmware 1.04 RAM:fa4776 maps a signed frame position through parallel
  64-entry boundary tables at RAM:f84590 and RAM:f84790. A direct zone spans
  256 frames from the first table; a gap zone spans from the second table's
  current entry through its next entry minus 257. Zone index bits 3..5 select
  one of eight 0x4000-byte groups and bits 0..2 select a 0x800-byte page.
- Gap-zone updates search at most 64 pages for a type-0x20 insertion point,
  first forward and then backward after passing group seven. They write the
  low 24 bits of the signed position, rebuild the destination header, clear
  stale presence and reconcile toward the original page; exhaustion sets
  RAM:000c8b bit one.
- Direct-zone updates accept zero through eight groups of 32 six-byte records.
  Preserved type-0x20 records are normalized to type 0x30; other non-0x30
  records contribute six skipped bytes. The remaining records are shifted,
  the retained workspace is copied back in dwords, and the type-0x20/0x31
  searches rebuild offsets and first/last summary triplets before the 32-byte
  header is written back.
- Firmware 1.04 RAM:fa3ef4 probes each eligible FORMAT UNIT code-11 segment
  in eight consecutive 32-block slices. A polled status of 0xff aborts; status
  zero or two waits for the raw-read engine to become idle and conditionally
  refreshes a state-one controller-record page, while other statuses retain
  the current logical position as the next cleanup reset target.
- The probe divides `(mapped_frame - 0x91)` by signed 0x27 and adds only the
  quotient's low 16 bits to the big-endian base pointer at RAM:000faff before
  testing the state byte. Optional P3 capture state is cleared around each
  slice and restored on both success and failure.
- Firmware 1.04 RAM:fa36f0 preserves the original RAM:000c88 bit seven in
  RAM:000c8b bit six for the duration of the code-11 scan. It collects up to
  32 signed frame positions at RAM:00fa2b, distinguishes continuous and
  discontinuous poll results, and flushes state-one controller-record page
  updates whenever the accumulated block count is divisible by 32.
- A nonzero/non-one/non-two scan status advances RAM:2ba5a3 by the fixed-
  packet mapping of the accumulated block count and 145, then derives the
  resume boundary RAM:2ba532 as signed `((pre-145 cursor + 0x138) << 5) / 39`.
  Status 0xff services the active host command during cleanup; other terminal
  statuses set RAM:000c8b bit zero and clear RAM:000c89 bit zero.
- Final code-11 cleanup scans controller-record banks at the ATIP last possible
  lead-out frame with a zero frame count. No matches mirror that lead-out into
  RAM:000ce1 and RAM:000ce5; matches publish the minimum and maximum first-
  position summaries from RAM:2b9704 and RAM:2b970c.
- Firmware 1.04 RAM:fa32d0 always clears RAM:000c8b bit zero before either
  FORMAT UNIT code-11 variant. Variant zero performs one queue/coroutine reset,
  prepares the Type-2 record pipeline, and schedules RAM:00122b to two on
  success; failure publishes sense 03/0c07 and unwinds the active flags.
- A nonzero format variant performs the queue/coroutine reset twice around the
  OPC gate, forces RAM:000c9e to two and RAM:001586 to one, and initializes the
  Type-2 pipeline with mode zero. Only a successful initializer clears the
  active byte, publishes RAM:000c89 bit four and dispatches host completion.
- Firmware 1.04 RAM:fa2e53 divides controller-record indices into 144-entry
  segments using boundary tables at RAM:f84490 and RAM:f84690. Table frame
  positions are translated to state-table indices as unsigned
  `(position - 145) / 39`; the state table base at RAM:000faff is incremented
  only in its low 16 address bits.
- Partial first and partial last ranges are processed independently, so a
  same-segment request can intentionally revisit Type-3 entries. Complete
  middle segments and state-index loops use signed 16-bit comparisons. If any
  block refresh occurs, a matched final bank scan copies RAM:2b9704 to both
  RAM:000ce1 and RAM:000ce5 rather than using RAM:2b970c for the latter.
- Firmware 1.04 RAM:fa299d maps controller-record index `n` to frame
  `n*39+145` and emits five configured prefix frames, 32 unconfigured middle
  frames and two configured suffix frames per inclusive index, followed by
  one terminal frame. The prefix reverses relative Q-MSF advancement only
  while the incremented target is below frame 150 and zeroes the relative
  tuple exactly when that increment reaches 150.
- After each 39-frame index, RAM:fa299d services the host and yields. SCSI
  status bit four clears completion for operation mode three but sets the
  abort bit for other modes; the independent RAM:2ba5b0 latch also aborts and
  acknowledges RAM:2ba5b1. Both aborts shorten the Type-3 refresh range to
  the current index. Cleanup always emits the terminal descriptor, drains the
  producer count to at most one, resets the ring cursors, and publishes the
  last emitted target at RAM:2ba59f.
- Firmware 1.04 RAM:fa24e9 initializes Type-2 lead-out acquisition from the
  dword at RAM:2ba58d with raw-Q track number 0xaa and a zero relative tuple.
  Each successful attempt emits five configured prefix descriptors, 0x1a57
  capacity-gated body descriptors, two configured suffix descriptors and one
  terminal descriptor; every command-0x80 submission uses parameter zero.
- A false producer-capacity check or nonzero RAM:00fba0 skips the lead-out
  suffix and terminal frame. A failed final preparation check follows the
  same failure route. RAM:fa24e9 latches first-failure context, completely
  resets raw-read ring state and retries from the original high-window frame
  at most three times; final exhaustion reports failure code five, while
  success clears sense.
- Firmware 1.04 RAM:fa20f3 synthesizes a Type-2 TDI acquisition at frame zero.
  Its thirteen-byte RAM:00143c payload is `54 44 49 01 50 01 01 01 01 91 00
  00 32`; it copies that payload through the +1-MiB alias to offset 0x18 of
  each of 138 middle raw sectors, servicing pending host work and yielding
  after every copied descriptor.
- Each RAM:fa20f3 attempt emits five configured prefix, 138 payload, two
  configured suffix and one terminal descriptor. All command-0x80 parameters
  are zero and all Q-MSF advances use reverse-relative mode. The terminal
  target minus one is published at RAM:2ba59f. A failed final preparation
  check latches context, resets the raw-read ring and retries up to three
  times; exhaustion reports failure code zero and success clears sense.
- Firmware 1.04 RAM:fa1ac5 begins Type-2 lead-in acquisition at the signed
  dword RAM:00114f. Its variable middle count is wrapping
  `(((RAM:2ba577 + 150) / 32) * 39 - 7) - RAM:00114f`, using signed division
  and low-product multiplication. Every middle descriptor is preceded by the
  producer-capacity check.
- RAM:fa1ac5 sources ten-byte Q records from RAM:008012 plus ten times the
  selected record index. Selection alternates in groups of three between a
  low range starting at zero and a high range starting at RAM:008008+1, with
  low and high wraps at RAM:008008+1 and RAM:00800a+1 respectively. A false
  capacity poll advances this selection without emitting a descriptor. ADR
  one records receive the current packed relative tuple at RAM:001275.
- The lead-in worker emits five configured prefix descriptors, the derived
  middle span, two configured suffix descriptors and a terminal descriptor.
  A nonzero preparation status skips the suffix and terminal route. Failed
  attempts latch first-failure context and completely reset the raw-read ring;
  three failures report code four, while success clears sense.
- Firmware 1.04 RAM:fa1871 normally scans zero-state controller-record runs
  through `ControllerRecordIndexCount-1`. Mode three first compares the
  highest marked index with the last active segment boundary, expressed as
  unsigned `(boundaryFrame-145)/39`; when lower, it searches earlier segment
  boundaries backward and extends the selected quotient by 175 indices.
- After the run scan, RAM:fa1871 treats RAM:000c89 bit two as a controller
  fault: it clears that bit and RAM:000c88 bit six, recalculates Type-2
  geometry, rebuilds selector-zero lead-in TOC records and reacquires the base
  window. A separate acknowledged run-failure latch sets RAM:001588 bit two,
  clears RAM:2ba5b0 and returns failure. Success sets RAM:2ba5af, clears
  RAM:000c88 bit six and returns true.
- Firmware 1.04 RAM:fa1341 emits exactly five configured prefix descriptors,
  32 workspace descriptors, two configured suffix descriptors and one
  terminal descriptor for each secondary Type-2 record slot. All command-0x80
  parameters and Q-MSF reverse-relative arguments are zero.
- The 32 workspace descriptors select pages in the exact order
  `0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7,0,1,0,1,2,3,2,3,4,5,4,5,6,7,6,7`.
  Acquisition mode zero builds each workspace page; nonzero mode copies it
  from the corresponding banked region. Each page is marked with byte 0x53
  and copied as 0x200 dwords from RAM:2ba62f to the raw-sector cursor plus
  0x18 through the +1-MiB alias.
- RAM:fa1341 retries the entire sequence once after resetting raw-read queue
  state. Both attempts reinstall the caller's signed start position and its
  relative `start-150` and absolute packed-MSF Q tuples. Success clears sense;
  two failures report code five.
- Firmware 1.04 RAM:fa0ab4 emits the same five-prefix, 32-workspace,
  two-suffix and terminal geometry for a primary Type-2 slot, but sources a
  complete ten-byte Q packet before every descriptor from RAM:008012 plus ten
  times the selected word index. ADR-one packets receive the current packed
  MSF tuple in bytes three through five.
- Its Q selector begins at record zero and alternates low and high index
  ranges in groups of three across descriptor-phase boundaries. The low range
  wraps at RAM:008008+1 and the high range spans RAM:008008+1 through
  RAM:00800a. The suffix completes the final three-record group without
  rotating, so the terminal descriptor reuses that selected Q record.
- Acquisition mode exactly one calls
  `IncrementType2ControllerRecordSlotValue`; other nonzero modes skip that
  increment but still select banked workspace-copy mode. A failed acquisition
  resets the raw-read queue and retries once; two failures report code four.
- Firmware 1.04 RAM:fa05ef reloads the signed Type-2 middle-frame position at
  RAM:2ba595 at the start of every attempt and publishes it to RAM:001598 and
  RAM:008000. It constructs relative `start-150` and absolute packed-MSF Q
  tuples, then emits exactly 5+32+2+1 descriptors with forward Q-MSF advance.
- Each of its 32 middle descriptors builds the MIP snapshot, writes the 0x53
  marker at the RAM:2ba5df workspace, and copies 0x14 dwords from that
  workspace to raw-sector cursor+0x18 through the +1-MiB alias. It retries
  after complete queue reset up to three times; success clears sense and
  exhaustion reports failure five.
- Firmware 1.04 RAM:fa0000 reloads its base-frame position from RAM:2ba591 on
  every attempt, publishes it to RAM:001598 and RAM:008000, initializes the
  low/high Q-record selector and advances the Type-2 capture cursor once.
- It emits exact 5+32+2+1 descriptors. Ten-byte Q records rotate in the same
  cross-phase three-descriptor low/high pattern as the primary slot worker,
  with current packed-MSF overlay for ADR-one records. Each middle descriptor
  builds an MIP snapshot and copies 0x14 dwords from RAM:2ba5df to
  cursor+0x18 through the +1-MiB alias. Three failures report code four.
- Firmware 1.04 RAM:f9ff54 is directly expressible as
  `double reentrant SplitFloat64FractionAndIntegral(double, double huge *)`
  under C251 `FLOAT64`. Keeping the signed cast nested in the binary64 cast
  avoids an extra 32-bit local: C251 then reproduces the canonical 16-byte
  frame, stack pointer offsets, signed conversion helpers, output store,
  subtraction and 130-byte ERET instruction shape. The operation truncates
  through signed 32-bit range and is therefore narrower than a general
  binary64 `modf`.
- Firmware 1.04 RAM:f9f5a1 derives its media-strategy identity as
  `0xa914 + 75 * AtipLeadInStartMsf.second +
  10 * (AtipLeadInStartMsf.frame / 10)`. The low nibble of packed-BCD frame
  byte RAM:00fb55 selects the initial record family and many per-identity
  variants; exact signed lead-in and unsigned lead-out frame positions refine
  exceptional records. The selected index is finally checked against the
  family byte at RAM:ff9491+index: primary-family mismatches fall back to zero
  and alternate-family mismatches fall back to one.
- Firmware 1.04 RAM:f9ee39 takes the dynamic coordinate in ACC and operating
  code in R7, returning through ERET. Code 0xff normalizes negative signed
  coordinates by adding 100, searches up to fifty 34-byte generated profile
  records by their byte-33 source coordinate, and advances/wraps RAM:00fa01
  after publishing one matched record. The controller write order is byte
  zero, nine interleaved columns from record ranges 1..9, 10..18 and 19..27,
  then auxiliary bytes 28..32. Recognized fixed codes are 0x14, 0x18, 0x20,
  0x28 and 0x34. When enabled, both generated and fixed paths store
  `((s32)(computed_word * threshold_word) / 10000) + RAM:00fad1` to the
  big-endian words at RAM:00facf and RAM:00fbc3; the generated adjustment also
  runs for a current-index mismatch when media state is one.
- Firmware 1.04 RAM:f9e627 takes float32 profile value in DR4 and sample index
  in ACC, returns a byte in ACC through near RET, and returns zero for indices
  above 35. RAM:00f96b selects paired 28-byte rows at RAM:f83b1d and
  RAM:f83cdd. Indices 0..8 combine a 0x40 signed-magnitude base with a signed
  byte at offset index+1; 9..17 decode 0x20 signed magnitude at index+1 and
  re-encode negative results in that format; 18..26 use the high nibble at
  index+1; 27..35 combine the 0x40 base with the negated low nibble at offset
  index-8. Every family evaluates `(end-start)/2048 * x*x +
  (start - (end-start)/2048 * 256)` in float32 and floors through float64.
- Firmware 1.04 RAM:f9e43e takes float32 profile value in DR4 and returns a
  signed baseline in ACC through near RET. It fills RAM:f9b7..f9c8 from sample
  indices 0..17 and RAM:f9cb..f9dc from indices 18..35, selects the minimum
  signed value among the first nine D8 entries, and forces the baseline to
  -15 if that minimum is below -15 or if its wrapping byte difference from
  any D9 entry 9..17 exceeds signed 15. It subtracts the baseline from and
  nonnegative-clamps D8 entries 0..8, packs D9 entries 0..8 as the original
  low nibble in the high half plus a bounded negated delta in the low half,
  clears D9 entries 9..17, then replaces the first five cleared slots with
  auxiliary lookups 0..4.
- Firmware 1.04 RAM:f9e2da takes a profile index in ACC and an unsigned radial
  endpoint in WR6, returns float32 in DR4 through ERET, and reads its scale and
  span as big-endian values at RAM:000cf5 and RAM:000cf9. It evaluates the
  early radial stages in float32, remaps only signed indices 96 through 127 by
  subtracting 100, applies factors 60 and pi before adding 625, widens only the
  square-root operand to binary64, narrows the root to float32, and finishes
  `root * 2 * pi * endpoint / (scale * 60)` in float32. Byte indices 0x80
  through 0xff remain negative and do not take the 96-through-127 remap.
- Firmware 1.04 RAM:f9e142 is a no-input near-RET column builder. It rereads
  RAM:00f05f for the two adjacent source rows at RAM:f8372d plus 24 times the
  row, and rereads RAM:001057 for both source and every destination address.
  It publishes a float32 slope/intercept at RAM:0012de/0012e2, floors through
  binary64, keeps the converted integer's low signed byte, clamps that byte to
  0..31, and writes records of stride 34 from RAM:00e806. Although native code
  retains switch cases for ranges 0x18..0x1c and 0x1c..0x20, its selector is
  initialized to zero and guarded by signed `selector <= 0`, so firmware 1.04
  executes only the 0x14..0x18 range before exiting.
- Firmware 1.04 RAM:f9d49d is a no-input near-RET packed interpolation-slice
  builder. Endpoint pair 4..10 selects source arrays RAM:f83451/f83459 and
  pair 0x14..0x18 selects RAM:f83461/f83465. For the column in RAM:001057 it
  first interpolates source bits 7..5, floors through binary64 and overwrites
  the destination high three bits, then independently interpolates bits 4..0
  and ORs the floored low byte. Codes below 0x14 use their code as the record
  index; codes 0x14 and above subtract 0x14. Records have stride 34 and the
  selected packed field begins at RAM:00e823.
- Firmware 1.04 RAM:f9d777 has an instruction-confirmed uninitialized-local
  dependency on its 4..10 route: stack float slots at SPX-3/-1 are consumed by
  both interpolation passes but assigned only when RAM:001058 equals 0x14.
  The 0x14..0x18 route is initialized normally. Do not replace the low-range
  behavior with guessed zeroes or a fabricated endpoint; retain the native
  body until the prior stack contents or intended source can be proven.
- Firmware 1.04 RAM:f9d2d6 takes an unsigned endpoint in WR6 and returns by
  ERET. It runs profile indices 0..99, computes and floors each radial
  coordinate through binary64 then back to float32, stores the first floored
  low byte at RAM:00fa02, and emits a record only when that floored float
  differs from the last accepted value and fewer than fifty records exist.
  Each 34-byte record at RAM:00e806 contains encoded baseline, all 18 D8
  bytes, D9 entries 0..8, then D9 entries 13,9,10,11,12 and the source index.
  The scaled-word helper receives the original unrounded coordinate on every
  iteration, and its 100 big-endian results occupy RAM:00eeaa..00ef71 exactly
  after the fifty-record region ending at RAM:00eea9.
- Firmware 1.04 RAM:f9cd5f takes the generated-profile endpoint in WR6 and
  returns through ERET. It initializes fixed and generated profile storage,
  runs RAM:f9d2d6, then builds fixed 34-byte records at RAM:00e75c for float32
  values 20, 24, 32, 40 and 52. Bytes 0..32 contain the encoded signed
  baseline, D8 entries 0..17, D9 entries 0..8, then D9 entries 13,9,10,11,12;
  initializer-owned byte 33 remains 0xff. The five scaled words are stored
  big-endian at RAM:00e752..00e75b.
- Firmware 1.04 RAM:f9c525 is a no-input ERET controller-profile programmer.
  Indexed state one selects 14-byte or 28-byte ROM rows for secondary codes
  below 0x14, or delegates fixed codes to RAM:f9f426; for codes below nine,
  destination fields 12..17 and 20..27 are zeroed without advancing the ROM
  cursor. Ordinary state selects a three-byte source for ports B2..B4 when
  RAM:00103b bit zero is clear or a 24-byte remapped source when set. The
  mapped offsets 6,9,13,16,20,23 pass through the near-RET mapper RAM:f9cd4a.
  Both paths preserve the RAM:000fca/RAM:018002 and EA/EC/A9.2 gate order.
- Firmware 1.04 RAM:f9cd4a takes and returns a byte in ACC through near RET,
  not ERET. It adds five above 0x0f and clamps the wrapping result to 0x1a;
  C callers must declare it `PLEXTOR_NEAR_FUNCTION`.
- Firmware 1.04 RAM:f9dec5 consumes inherited R9 on its selector-below-0x14
  path. R9 selects four auxiliary bytes at RAM:00e823 + 34*R9 even though the
  explicit selector arrives in ACC; R9 is assigned from selector-0x14 only on
  the generated-record path. Its near wrapper RAM:f9dec1 and both immediate
  callers do not initialize R9. Do not replace this body with an ordinary
  one-parameter C function until the outer R9 contract is recovered or a
  proven addressless adapter carries it explicitly.
- Firmware 1.04 RAM:f94fc3 is a no-input near-RET computed-target measurement
  engine. It performs at most three acquisition attempts, repolls result one
  on the same attempt when its one-shot tolerance restart fails, applies
  float32-scaled 65/45/20-frame stages, and selects the target-window capture
  only while RAM:00ef72 bit one is set.
- Firmware 1.04 RAM:f94b91 has no inherited-register input and returns status
  one, two, or three in ACC through ERET. Final classification ORs the stored
  phase bytes RAM:00ef79..00ef7d; in particular, the canonical listing uses
  stored RAM:00ef7d rather than rereading RAM:00ef7f as the decompiler shows.
- Firmware 1.04 RAM:f949ba is a no-input ERET coroutine worker with an
  unsigned five-attempt outer measurement limit. Its RAM:f94d1d series-clear
  call is near: that entry clears B, tail-jumps to RAM:f94d9c, and ultimately
  returns with RET, so C declarations and callers must use the near contract.
- Firmware 1.04 RAM:f9360f has no inherited-R15 input: R15 is saved for caller
  preservation and initialized to five before semantic use. Its two native
  decrement/test loops allow six attempts (old values five through zero), and
  its decoded-position acceptance window includes exactly -150 and +150.
- Firmware 1.04 RAM:f93068 is a no-input near-RET four-record packed-
  subchannel processor; saved R13/WR14 and R15 are all function-owned. It
  consumes four 18-byte records at RAM:2bb919, uses BITS:0003 as the observed
  type-0x80 latch, and resets the raw-read ring when a nonzero unexpected
  header is seen while the retry byte at RAM:000f4b is below two.
- Firmware 1.04 RAM:f91655 is a no-input near-RET primary raw-read producer
  worker; its saved DR12 and ten-byte stack frame are function-owned. It calls
  RAM:fee8f0 with ECALL but RAM:f9241f, RAM:f94363 and RAM:f94427 with LCALL,
  so the latter two accepted C functions require `PLEXTOR_NEAR_FUNCTION`.
  Its positive-position retry compares the old signed RAM:000f2c value to 75,
  and its negative speed retry compares the old signed RAM:000f9c value to 5.
- Firmware 1.04 RAM:f9142d is a no-input near-RET parsed-Q producer worker.
  Its saved DR12 and six-byte stack frame are function-owned. It bypasses
  position validation when any of BITS:0013/001c/001d is set, publishes the
  decoded delta for the inclusive range -170..170, compares the old signed
  RAM:000f2c retry value to 30, and resets RAM:000f23 to one when its signed
  value is at most 17 before the terminal status-0x83 service.
- Firmware 1.04 RAM:f90d22 is a no-input near-RET controller read-event
  dispatcher. Saved R15 is initialized from RAM:000f30 before semantic use
  and is not inherited. The block at RAM:f91330..RAM:f9139e is an exact
  24-bit controller cursor modulo-0x930 test; when the remainder is nonzero
  and RAM:000f62 is clear it calls RAM:f93429. In state seven, negative
  special mode plus format four deliberately retains the auxiliary/C2 cursor
  loaded from ports 0x8019..0x801b (or the zero-initialized local if that
  option is absent) instead of reading the main cursor ports.
- Firmware 1.04 RAM:f908e6 is a no-input near-RET leaf. Native zero stores at
  RAM:000f1c and RAM:000f3b are word writes and clear the following byte too.
  RAM:000e4a is decremented before testing the old signed value; expiry reloads
  five and enters the backoff ladder. RAM:000e49 is finally capped unsigned to
  a fresh RAM:000e46 read. Its only bit dependencies are BITS:000b/001c/001d/
  0020, all representable directly with C251 `sbit` objects.
- Firmware 1.04 RAM:f9019b is a no-input near-RET primary raw-read pipeline
  step. The sole caller loads WR14 with 0x0f04, making inherited R15 the
  initial signed retry byte; other DR12-family uses are assigned temporaries.
  The ordinary request adjustment is RAM:00159c plus the big-endian profile
  dword copied to descriptor 0x0f04+0x0c, despite the decompiler's overlapping-
  register self-add artifact. Native calls to RAM:f9407c and RAM:f94872 are
  LCALLs, so both accepted C declarations require `PLEXTOR_NEAR_FUNCTION`.
- Firmware 1.04 RAM:f90000 is a no-input coroutine-two entry ending in ERET.
  Its call to RAM:f9019b occurs with WR14 fixed at 0x0f04, so an addressless
  near adapter can preserve that callee's inherited R15=4 retry seed while
  leaving the same-address body in C. Its thirteen BITS accesses compile as
  direct C251 bit operations, and the final RAM:f815da call remains a near
  LCALL immediately before ERET.
- Firmware 1.04 RAM:f8fbe1 is a no-input ACC-Boolean ERET worker. It probes
  controller commands 0x14 then 0x16, with thresholds 0x50 and either 0x50 or
  0x68 for strategy record index 0x37. A first-probe hit skips the second
  probe. BITS:0031 is a direct gate, indexed state four suppresses reduction,
  and the write-speed eligibility predicate is intentionally reevaluated
  after RAM:f8fdcd before being passed to RAM:f89dda.
- Firmware 1.04 RAM:f8f8e1 is a no-input ERET controller sampling worker.
  Although it saves WR14, R14 is cleared at entry and becomes the local
  threshold-latched flag; there is no inherited WR14 semantic input. The
  phase is `(RAM:000fd0 + 2) & 6`, controller-result thresholds depend on
  operating code, signed RAM:001235 and strategy index 0x37, and indexed state
  four suppresses a latch only through position 315000 inclusive.
- Firmware 1.04 RAM:fe2e9d is a no-input ERET raw-read reset leaf with no
  callees. It clears ten bytes at RAM:001272, four dwords at RAM:001262, the
  18-byte RAM:00127c state region with RAM:001281 restored to two, writes zero
  twice to the dword at RAM:008000, clears five words at RAM:008008..008011,
  all 220 ten-byte records at RAM:008012, and RAM:0088aa..0088b9 before
  publishing RAM:0014cc=0xbb and clearing RAM:001554..001555.
- Firmware 1.04 RAM:fc7766 takes a 32-bit key in DR4 and a mode in ACC and
  returns a controller-buffer address in DR4 through ERET. It reads page
  bounds at RAM:2ba51c/RAM:2ba520 low word first, scans their +0x100000 CPU
  mapping, and always reads all six record bytes. Modes 0x20 and 0x31 match
  high-nibble types two and three; all other modes require type two plus the
  packed 20-bit value `(record[3]&0x0f)<<16 | record[4]<<8 | record[5]`.
- Firmware 1.04 RAM:fc78f1 takes its inclusion selector in ACC and returns by
  ERET. It clears 256 paired dwords at RAM:2b9d1c, selects either consecutive
  leading type-three records or every type-zero/type-one record from the
  0x600-byte scratch page at RAM:2b971c, sorts by the second or first packed
  triplet respectively, rebuilds the selected six-byte prefix, and writes
  `count*6` bytes in 255-byte chunks plus one final remainder call.
- Firmware 1.04 RAM:fcb050 is a no-input vendor-F3 dispatcher ending in ERET.
  Its selector-two word and selector-0x14 word clamp at 0x5dc0, selector 0x0b
  maps drive-cycle results zero through seven to status words 0x4080 through
  0x4087, and selector 0x1f accepts only a zero reserved byte plus port mode
  zero or one. Its fixed acquisition ranges are minutes 5..70, 70..74, and
  23..70, with end position converted before start position.
- Firmware 1.04 RAM:fcb61f is a no-input vendor-F5 dispatcher ending in ERET.
  Selector four constructs its payload address as the consumer descriptor's
  low word plus a high word read indirectly through the descriptor word at
  offset two, then advances the combined address by four. Selector eight
  reports RAM:00fbd7 plus zero-extended RAM:00fbd9 and consumes them only when
  CDB byte four bit zero is set and the zero-length or non-status-four path
  completes. The common capacity is `0x001affcd - RAM:00155a`.
- Firmware 1.04 RAM:fcbe46 takes start-address bits in DR4 and block count in
  DR0, returns Boolean in ACC through ERET, and has no inherited DR12 input:
  DR12 is immediately overwritten by startAddress. A contiguous non-fixed
  continuation still flushes the prior range when signed startAddress has
  zero remainder modulo 0x1100. The current command count is RAM:000b06 and
  its start address is RAM:2bc703; the WRITE AND VERIFY snapshot is RAM:2bc707.
- Firmware 1.04 RAM:fcc246 takes blockCount in DR4 and returns Boolean ACC via
  ERET; saved DR12 is immediately overwritten with blockCount and is not an
  inherited input. With queued raw-read data it temporarily rounds
  RAM:001598 down to the signed 39-frame grid based at 145, prepares/waits for
  the prefix, then restores the exact target. It saves and restores RAM:00fb6e
  only after that first prefix route and publishes the exclusive WRITE AND
  VERIFY endpoint at RAM:2bc70b.
- Firmware 1.04 RAM:fcc572 takes signed startAddress in DR4 and returns Boolean
  ACC through ERET. Saved DR12 is function-owned. It initializes the host
  range, optionally acquires a missing preceding 39-frame record index,
  truncates the start toward zero to a 32-block boundary, and selects either
  preservation acquisition or synthetic descriptor setup while restoring the
  command-start dword.
- Firmware 1.04 RAM:fcc7de takes unsigned blockCount in DR4 and returns Boolean
  ACC through ERET. It prepares only ranges whose signed end remains below the
  next signed 32-block boundary. The controller-record route temporarily makes
  RAM:000c88 bit seven agree with RAM:000c8a bit two and restores it; both
  preparation routes restore RAM:001598 and leave the raw-read ring reset.
- Firmware 1.04 RAM:fcca94 takes signed startAddress in DR4 and an exact-one
  alternate policy in ACC, returning normalized Boolean via ERET. It always
  computes signed startAddress/0x1100, uses only the quotient low byte for the
  RAM:2bae33 dword table, and tests mapped RAM:000b02 plus 0x91 unsigned against
  the selected boundary and RAM:2ba59f before the exact-one marked-index test.
- Firmware 1.04 RAM:fcd42a is a no-input near-RET Type-2 MIP geometry worker.
  It reads 24-bit coordinates at RAM:2bb931 and RAM:2bb934, matches the first
  against `base + row*256 + (column+1)*32` for row 0..2 and column 0..7, falls
  back to base+32, and maps coordinates as `(low32(value*39)>>5)-0xa7`.
- Firmware 1.04 RAM:fcdc24 takes an exact-one fallback selector in ACC and
  returns Boolean through near RET. It scans at most eight banks by eight
  pages, selects primary positions from RAM:00fb04, fallback positions from
  `-214-bank*32`, or secondary positions through the signed low-product 39/32
  transform, and handles probe statuses 11/fe/ff/0f as retries versus status
  five as hard failure. Accepted pages are validated against RAM:2bb91c,
  copied through the addressless four-register bridge and reflected in
  RAM:000c8b completion bits.
- Firmware 1.04 RAM:fce0d4 is a no-input ACC-status ERET radial-calibration
  coordinator. Saved WR14 is function-owned and starts as the zero lead-in
  fallback flag. It first measures at frame 147, switches to the converted
  lead-in target only after the first state-zero acquisition failure, polls
  state-two controller Q packets through 0xfe pending statuses, propagates the
  span-calibration result, computes RAM:001000 as the unsigned product of
  ROM:f848fe and RAM:000cf9 divided by 1600, conditionally refines at frame
  150, then snapshots RAM:000cf5/RAM:000cf9 to RAM:00fbe6/RAM:00fbea.
- Firmware 1.04 RAM:fce2e1 is a no-input WR6-result near-RET measurement
  worker. It issues command 0x12 ten times, polls RAM:0180cc bit four, reads
  sample high/low bytes from ports 0x80cf/0x80ce, yields after each sample and
  returns the unsigned mean. With RAM:00fc32 bit zero set, only busy-poll
  iterations for samples five through nine read controller word 0x0233; each
  sample's signed max-minus-min delta is accumulated, divided signed by five,
  stored at RAM:00fc30, and sets RAM:00fc32 bit one at an unsigned metric of
  0x48 or greater.
- Firmware 1.04 RAM:fce645 is a no-input ERET radial-table rebuild. It maps
  frame 306000 unless RAM:00f9e9 bit two is set and signed RAM:0012c1 is at
  least 306000, publishes four float32-derived inverse thresholds, and selects
  the 40.2 factor only for nonnegative endpoints below 382500 or with that
  flag set; otherwise it uses 38.0. State one rebuilds controller profiles.
  Its 160-entry table converts unsigned RAM:000cf5 to float32, divides by
  65.536, rereads unsigned RAM:000cf9 for each index, applies the exact staged
  formula including a float64 square root, and stores the converted low word
  at RAM:000cfb+2*index.
- Firmware 1.04 RAM:fcf36f is the no-input ACC-status/ERET captured-controller-Q
  dispatcher. It returns zero on missing drive readiness, signed timeout, or
  an invalid bounded alternate ADR packet; one on accepted content; and 0xfe
  while pending or after rearming/acknowledging capture. Ordinary mode handles
  ADR 0/1/2/3/5, including the B0..B4/C0/C1 and track-point record stores, and
  gates most position acceptance on RAM:000ff9 bit two. Alternate mode clears
  RAM:00125b and accepts ADR 1..6 into masks 1/2/4/8/0x10/0x20, with decoded
  point bounds 1..21 for ADR 3/4 and 1..40 for ADR 5/6. All fourteen canonical
  callers use ECALL. Its position advance, ADR 0/1/2/3/5 capture/copy, decode,
  and acknowledge helpers use LCALL, while measurement dispatch, rearm, BCD
  conversion and far copy use ECALL.
- Firmware 1.04 RAM:fd016f is a no-input ERET controller-transition worker.
  Although both canonical ECALL sites load a direction value into ACC, the
  callee never reads ACC; saved R15 is cleared before use and is only the
  function-owned override set by drive state two/current code one. It selects
  reference position 150 for the applicable special target, otherwise the
  active/original acquisition target in drive state four or RAM:00125e in
  other states, and calls near RAM:fd0380 with the low-word quotient of
  unsigned RAM:000cf5/65 plus a saved secondary operating code. State four,
  current code at least 16, operating-enabled exact one and non-ff secondary
  code scale the resulting unsigned word through float32 times 1.1. The first
  controller submission uses selector 0x035f. The second derives 0x7fff divided
  by signed-normalized RAM:00fc25, with override and secondary-code force
  rules, and uses selector 0x0369. It then writes `(RAM:00f98f & 0xf8)|0xa2`
  to port 0x80a6 and refreshes the profile only for current code at least 24
  while controller operation is disabled.
- Firmware 1.04 RAM:fd07de is a WR6-distance, ACC-direction, ACC-Boolean
  near-RET pickup-move transaction. Both callers use LCALL. It waits on
  controller port 0x80f8 bit seven before setup and after execution, updates
  the big-endian radial estimate at RAM:001031 by the low-word product of
  unsigned RAM:000cf9/12000 and distance (add for direction zero, subtract
  otherwise), refreshes fixed parameter tables only when the cached
  distance-one class, operating-code class, or operating-enable context no
  longer matches, and programs the long-distance profile at distance ten or
  greater. Completion status exactly one restores channels/words and publishes
  RAM:00fc2b..00fc2f; other statuses clear port 0x849d and return false.
- Firmware 1.04 RAM:fd10e9 is a no-input ERET controller-event dispatcher.
  It snapshots active-low port 0x8003 and source port 0x807f, acknowledges
  source bits through ports 0x802e/0x806f and event bits through 0x802d,
  dispatches event bits in canonical 6,3,5,4,2,1,7 order, and republishes
  RAM:000fad and RAM:001038 to ports 0x8003 and 0x807e. BITS:0043 is bit three
  of bit-addressable RAM:0028 and is cleared by event four before event two
  tests it. RAM:f90d22 `ProcessControllerReadEvent` has a far ERET contract;
  its sole canonical caller at RAM:fd11cf uses ECALL.
- Firmware 1.04 RAM:fd13dd is a no-input near-RET word-0x0251 sweep tick. It
  submits 0x60 times ROM:f848c7[phase], samples D8/D9 at phases 11..14 and
  35..38, divides each completed four-call sum by 16, averages the pair, and
  at phase 39 publishes low-byte absolute differences plus the combined
  average according to the pass index. Pass zero publishes nothing; pass one
  seeds parameter FB to 0xb8; later passes clear RAM:001301 bit two at FB 0x50
  or subtract eight above 0x50. Phase 48 clears all six words and advances the
  pass. RAM:f8e3d8 returns a WR6 sum through ERET: four D8 samples each plus
  0x78, or four unadjusted D9 samples. Its earlier void/discard C model was
  incorrect.
- Firmware 1.04 RAM:fd5dd7 is the no-input ERET opcode-F0 configuration-update
  handler. It admits requests only when unsigned 32-bit
  `parameter_length + offset - 8` is at most 0xa0, validates the staged
  eight-byte identity, and conditionally constrains configuration bytes 0x41
  and 0x4c. After programming `parameter_length - 8` bytes it deliberately
  rereads and compares only one fewer byte. Comparisons apply to absolute
  configuration offsets below 0x5e and 0x60 through 0x7d, skipping 0x5e,
  0x5f, and all offsets from 0x7e onward. A compared mismatch sets RAM:001009
  and restarts coroutine one without publishing new sense data.
- Firmware 1.04 RAM:fd60f2 receives its normalized C arguments entirely in
  registers: 24-bit source in DR4, destination offset in WR2 and byte count in
  WR0; Boolean success returns in ACC through ERET. The former DR0/WR6/stack
  description was incorrect. It validates an existing non-erased primary
  checksum, applies the paired-byte invariant before copy, after copy and after
  checksum regeneration, and independently limits offset to 0x9f and count to
  0xa0 (there is no combined offset-plus-count bound). Offset above 0x60 selects
  extended-only programming. Failure codes are 1, 2, 3, 5, 6, 7, 8, 9 and 10;
  each sets RAM:001009 and RAM:0012f7 to one before persisting the code.
- Firmware 1.04 RAM:f8b9e6 is the unsigned-DR4, no-result ERET extended
  controller setup worker. It maps the current operating code twice and reads
  big-endian words from RAM:ff84ce + 2*(profile-1) and RAM:ff84e6 +
  2*(profile-1); profile zero therefore reads RAM:ff84cc and RAM:ff84e4.
  Codes 1/4/8/10/12/16 select their fixed parameter profiles and port-80a8
  values 00/12/13/14/14/14. Every other code takes the high-code path. Code
  10 with RAM:00103a bit six set overrides the common-port flag, port value,
  and words 036f/037f to 1, 12, 77a3, and 445e, and first submits RAM:000ce9
  as word 035f. RAM:f8bc59 `SelectSecondaryOperatingCodeForThresholdValue`
  is a near RET helper; RAM:f8c1a1 is also called with LCALL.
- Firmware 1.04 RAM:f8d06d is a no-input, ACC-Boolean ERET word-0x0319
  calibration worker. It accepts RAM:f8d355 only when ACC is exactly one,
  sign-extends the byte from near RAM:f8d2e8, divides corrections other than
  zero and plus/minus one by two, then shifts the signed result into the high
  byte and adds it to the current word with 16-bit wrap. RAM:00120a is set to
  ten but decremented asynchronously; negative expiry forces sentinel 0x007f,
  while zero expiry retains the current non-sentinel correction. Sentinel
  failure restores the saved baseline word. A zero correction publishes the
  baseline delta and, only for exact indexed-state result one outside
  configuration two, clears RAM:00fab4 bits one and four through two separate
  volatile read/modify/write operations and reports stage two. Every success
  route restores word 0x0339 to zero and channel two to 0xc1, whereas the
  prerequisite-failure route restores the saved word and RAM:00f98e.
- Firmware 1.04 RAM:f8da66 is a no-input, ACC-Boolean ERET word-0x0204
  calibration worker. Each pass captures forty nonzero command-0x0E words
  while stepping word 0x0204 from 0xd000 upward by 0x0200, smooths the endpoints
  unchanged and interior samples with a three-word sum that wraps at 16 bits,
  and publishes the smoothed table at RAM:00116e. Equal maxima select the last
  index; the baseline is the greater of the inclusive-left and exclusive-right
  minima, and `(maximum - baseline) * 4` wraps at 16 bits before division by
  five. The first and last samples strictly above `maximum - prominence`
  define a midpoint candidate. Two candidates within 0x0400 are averaged;
  otherwise a third pass selects the closest pair, retaining the first pair on
  a tie that does not improve its distance. Capture failures publish words two
  through four at RAM:0011be, and a missing strict interval publishes five.
  RAM:f8e029 is a near WR6-result wrapper, not void: it issues command 0x0E and
  tail-calls near RAM:f8e068, which returns port 0x80c7 as the high byte and
  port 0x80c6 as the low byte or zero on timeout.
- Firmware 1.04 RAM:f84f6a is a no-input, no-result ERET coroutine-one state.
  It clears BITS:0039, waits cooperatively for RAM:00faeb, and maps recognition
  results 2/0x11/0x14/0x15/0x16/0/other to drive states
  4/8/0x0c/0x0e/0x0f/0/5 and phases 2/4/6/6/6/3/4. After indexed-state and
  media-read setup, exact indexed-state result one plus media type two selects
  continuation RAM:fa6e33; every other outcome selects RAM:f85d74.
- Firmware 1.04 RAM:f850ca is a no-input, near-RET indexed-state transition
  worker called with LCALL from RAM:f84f6a. It owns its R15 rejection flag;
  state 0x1f with media type two accepts only strategy index 0x91. Its fatal
  reason bytes are 0x27, 0x39, 0x3a, 0x3b and 0x3d. Eligible state five
  persists controller calibration, while state 0x12 restarts coroutine one at
  RAM:f859f0.
- Firmware 1.04 RAM:f853cf is a scheduler-installed, no-input, non-returning
  ERET coroutine-one status worker. Statuses 3, 5, 8, 11, 12, 13 and 14 set
  the indicator pattern once and then repeatedly clear it before yielding;
  statuses 6 and 7 clear it forever without yielding; status four directly
  sets P3.0/P3.1 and then only yields; every other status yields before each
  repeated pattern clear. The terminal ERET is unreachable.
- Firmware 1.04 RAM:f85583 is a no-input near-RET startup initializer with
  three bounded clears: sixteen words at RAM:00100a, six bytes at RAM:000fcb,
  and 160 words at RAM:000cfb. It loads RAM:001000 from the immutable default
  word at RAM:f848fe, installs radial scale 0x00014ccc, span 0x0640 and read-
  speed limit 0x34, and captures the position snapshot only when indexed-state
  predicate four is nonzero. RAM:f89f42 is a near-RET helper and must be called
  with LCALL.
- Firmware 1.04 RAM:f859f0 is a no-input near-RET indexed media/read
  configurator. Its explicit RAM:00130a cases are 0, 1-7, 9, 0x0a, 0x0c,
  0x0e, 0x11-0x17, 0x1f, 0xf2-0xf4, 0xf6, 0xf9, 0xfd and 0xff; all other
  values set MediaCoroutineState to one. State 0x1f clears the six
  interleaved RAM:00f9ab/00f9ac pairs and sets BITS:0037, while state 0xf9
  sets RAM:001301 bits six/seven and directly drives P3.0 low/P3.1 high.
  The final call loads WR6 with one before RAM:fd7b56.
- Firmware 1.04 RAM:fd7b56 clears WR4, not all of DR4: WR6 is the unsigned
  16-bit PRNG seed and the stored 32-bit state is its zero extension.
- Firmware 1.04 RAM:f84d25 is a no-input ERET persistent-calibration loader.
  It reads configuration tuples offset/destination/count
  0x20/RAM:00fab4/0x1a, 0x30/RAM:00fac4/4 and 0x3f/RAM:00fae0/1. Its
  RAM:000ffe value is the signed-integer truncation of unsigned RAM:00fabc
  converted to binary32, multiplied by 234.0 and divided by 100.0. It mirrors
  RAM:00fae0 bit two directly to BITS:0044 and sets RAM:00fae0 bit six when
  indexed-state predicate four is nonzero.
- Firmware 1.04 RAM:fde12d is the no-input ERET read-acquisition request
  dispatcher. RAM:0011e5 request priority is bits 2, 0, 1, 3, 5, then 4.
  Bit five tests RAM:001218 as a signed byte before retreating the track cursor;
  bit four tests the signed big-endian dword at RAM:0012c1 against 0x17124 and
  targets frame 90000. When primary handling does not return, RAM:0011e1 bits
  two/four fold into RAM:000fba bits five/six through distinct volatile
  read/modify/writes. Its direct SETB BITS:0580 / CLR BITS:0581 pair is retained
  only as an addressless near adapter for C251.
- Firmware 1.04 RAM:fe8871 takes signed start in DR4 and unsigned frame count in
  DR0 and returns Boolean ACC through native RET. Its acquisition begins at
  start minus five and compares the target with the wrapped unsigned exclusive
  end. A successful sector preserves main data, clears its subchannel slot,
  applies Mode-2 source kind four, updates/enqueues Q state, and advances MSF,
  target and ring. Packet completion uses prefix arguments true/false; terminal
  completion uses false/false followed by true/true.
- Firmware 1.04 RAM:fe3e0d is the no-input, ACC-Boolean ERET Q-record queue
  initializer. It derives the 16-bit index mask/high-water values from fresh
  32-bit ring-count reads, sets descriptor controller address 0x88bb and copies
  the auxiliary C2 base. Its count repair restores one head descriptor when
  queued equals expected minus one; larger shortfalls discard one tail then
  restore one head. Write type two tolerates table-build/repair failures and a
  status-four shortfall. RAM:fe2a0e `DiscardRawReadDescriptorsFromQueueTail`
  is a near WR6/ACC/RET helper and C callers must use LCALL.
- Firmware 1.04 RAM:fe85ff is the no-input, ACC-Boolean ERET FORMAT raw-read
  coordinator. It derives both ring thresholds from separate fresh reads of
  RAM:00156a, computes the ordinary range as
  `(packetSize + 7) * (RAM:00e5fc / packetSize) - 5`, applies an additional
  working-track span subtraction when RAM:00e5fb is nonzero, and then selects
  either a ten-record Q sequence or the synthesized Q queue before lead-in/out
  acquisition. Controller mode 0xc0 adds the final mode-two range beginning at
  RAM:00e61e + 1. RAM:0088b7 is cleared on every terminal path.
- Firmware 1.04 RAM:fe8337 uses the standard C251 register order DR4=count,
  ACC=Q-metadata mode, R3=preserve-main and R2=clear-subchannel when the two
  Boolean parameters are written in semantic C order. Operations zero/one
  derive preservation from the lead-in track, two through four from working-
  track control bit 0x40, and five/six from the selected TOC track while
  forcing subchannel clearing off. The loop captures wrapping
  `frame_count + 1` sectors; mode two keeps RAM:00127c set through target 150
  and clears it only after the target becomes greater than 150.
- Firmware 1.04 RAM:fe4f4e is the no-input, ACC-Boolean ERET lead-in/out
  coordinator. Both phase retry counters permit nine retries after the initial
  attempt. The optional terminal call uses start RAM:0015f2, count 4500 and
  mode two; it also forwards the lead-out retry count as inherited R15 into
  RAM:fe825b. The call result is intentionally ignored before RAM:0014b5 is
  set. Existing-session lead-in acquisition is rejected immediately when the
  signed dword at RAM:00dd69 is negative.
- Firmware 1.04 RAM:fe13eb is a no-input near-RET TOC-summary initializer that
  only acts for write type two. It locates the first nonzero type-one entry in
  the eight-byte table at RAM:00b4bd, walks to track marker 0xaa, and publishes
  first/last track, A0 P-second and the marker record's minute/second/frame in
  packed BCD at RAM:001400..001408. RAM:000c4a value 0x10 forces A0 P-second
  0x10 and initial track two; source flag 0x40 advances both published track
  bounds to the following track. The ordinary route derives A0 P-second from
  source-byte-three bits 0x30 and uses the record before 0xaa as last track.
- Firmware 1.04 RAM:fe7ab2 takes an unsigned frame count in DR4 and always
  returns ACC true through near RET. It captures source kind seven in groups of
  three, ignores the capture helper result, clears sixteen bytes at the main
  cursor plus 0x100000, copies the selected ten-byte Q record into
  RAM:001272..00127b, and substitutes the current packed-MSF tuple at
  RAM:001275 when the Q control/ADR low nibble is one. RAM:0014c7 is a 16-bit
  within-group count. Complete groups alternate RAM:0014c1 and RAM:0014c3
  cursor ranges through RAM:0014c5/RAM:0014c9, with equal range boundaries
  forcing the first range.
- Firmware 1.04 RAM:fe3827 takes omit-final-prefix in ACC and wait-for-
  completion in R7, returns Boolean ACC, and uses ERET. Its frame/window
  acquisition helpers advance RAM:001598, so terminal RAM:008000 publication
  must read the post-helper target: Q types below four publish that value for
  the two-frame path or decrement/publish it for the three-frame path; fixed
  windows likewise use the fresh target. Caching the entry target across those
  calls is behaviorally incorrect.
- Firmware 1.04 RAM:f86475 is a no-input near-RET read-acquisition state
  worker. Its function-owned R12 bypass is set by RAM:000e46 value one, or by
  RAM:000ff9 bit two clear plus RAM:00f9e9 bit two set and BITS:001c/001d both
  clear. R13 counts consecutive successful pulse tests, signed R14 is the
  capture retry count initialized and reset to three, and R15 holds the 25/70
  pulse threshold. The post-acquisition signed wait on RAM:001034 is gated by
  BITS:0013, not RAM:000f46. Capture failure status seven forces minimum-speed
  recovery; calibration failure publishes seven to both RAM:000e41 and
  RAM:000fbc.
- Firmware 1.04 RAM:f8f38b takes and returns binary32 in DR4 through near RET.
  Its binary64 divide by 1.41421356 followed by exponent decomposition selects
  the same power-of-two boundary as a binary32 mantissa comparison: fraction
  0x3504f3 remains below the boundary and 0x3504f4 advances the exponent. The
  remaining algorithm is a four-stage binary32 continued fraction followed by
  exponent times 0.6931472f. Its sole caller RAM:f8f342 uses LCALL.
- Firmware 1.04 RAM:f84a14 uses the standard C251 FLOAT64 DR0:DR4 argument and
  result frame and returns with ERET. Its floor behavior is equivalent to
  clearing all binary64 significand bits below the unbiased exponent, then
  subtracting one only when the input is negative and discarded bits were
  nonzero; integral values, signed zero, infinities and NaNs pass through.
- Firmware 1.04 RAM:fe5189 is a no-input ACC-Boolean ERET close/synchronize
  raw-read preparer. Track type one fills to 300 frames and may run a 10/20-
  record sequence; types two, three and five use the common end-range path;
  type six may prepare an alternate plan and fill from track start; type four
  enters terminal-prefix handling directly. Normal Q-queue failure at 12x or
  above retries at 4x after OPC and restores RAM:000e47 from RAM:000c38 only
  after the second build succeeds. RAM:fe9ef2 and RAM:fe2a8d are near-RET
  helpers and C declarations must preserve their LCALL class.
- Firmware 1.04 RAM:fe04ce is a no-input near-RET synthetic Q-table builder.
  It reads raw-track MSF bytes from offsets 4, 6 and 7 of each eight-byte
  RAM:00b4bd record, emits A0/A1/A2 followed by one record per track group,
  and stops at track marker 0xaa. RAM:008008 receives the count before the
  optional B0/C0/C1 suffix; RAM:00800a and RAM:00800c both receive the final
  count. The decompiler's apparent zero stores at the end are incorrect: the
  canonical instructions copy WR14 to both words. Computed B0 tuples add
  11,250 frames with no full-TOC sessions or 6,750 frames otherwise. Its
  RAM:fe0475 lookup dependency is a near WR6-result/RET function and C callers
  must use LCALL.
- Firmware 1.04 RAM:fe3fd0 is a no-input Boolean near-RET Q-table builder and
  acquisition producer. Write type two converts qualifying type-one raw-track
  entries into ten-byte Q records and schedules five sectors per record; an
  odd final record receives all ten sectors, so the data-capture count is
  `5*n` for even `n` and `5*n+5` for odd `n`. Every acquired sequence then
  adds one source-kind-six terminal descriptor. Ordinary PMA/staged modes use
  full ten-sector groups, while the special write-type-two path captures ten
  source-kind-ten sectors directly. Producer-capacity failure reports pipeline
  code three, capture failure reports sense 04/4400 plus RAM:001598, and PMA
  ADR3/ADR5 overflow reports 03/7304 after latching the first failure context.
- Firmware 1.04 RAM:fe63a2 takes the initial-position selector in ACC and the
  resume-after-failure selector in R7, returns Boolean ACC through native RET,
  and owns the complete lead-in descriptor sequence. It clears all 220 raw-Q
  records, selects ATIP/default RAM:00114f or resume RAM:001484, builds the
  type-two synthetic Q table or merges staged Q state, and captures
  source-kind-seven frames in three-frame groups while rotating Q-record
  cursors. Write type two has a distinct ten-frame source-kind-ten RID01 path.
  When RAM:0014ca is nonzero, the producer-capacity route writes the auxiliary
  tuple at main cursor + 0x10000c before mode-two subheader population; the
  direct route clears sixteen bytes at main cursor + 0x100000. Capture failure
  reports sense 04/4400 plus RAM:001598, while capacity failure latches the
  first read context. The final capture count is one for write type two and two
  otherwise, after which RAM:001289 is cleared and the type-two track indices
  are advanced.
- Firmware 1.04 RAM:fe6e5b takes restart-after-failure in ACC and returns
  Boolean ACC through native RET. It selects a 2,250-frame lead-out when
  RAM:00dd69 is nonnegative and a 6,750-frame lead-out when negative; the
  RAM:001008/e5fb/session-one combination also forces the long route. The
  input restart flag remains live even when RAM:00dd69 is nonnegative; the
  decompiler's apparent assignment of false is not present in the canonical
  instructions. Non-type-two media acquire one initial frame and subtract two
  from the main count, while type two subtracts one. Main frames use source
  kind eight, alternate saved Q packets every three frames only on the short
  route, toggle RAM:00127c beginning at frame index 149, and yield every 32
  frames. The terminal loop copies current Q tuple one into the relative-MSF
  tuple before each of two source-kind-seven captures, then publishes
  ReadPipelineTargetAddress minus one at RAM:0015f2.
- Firmware 1.04 RAM:feee85 scales a raw binary64 value by adding a signed
  16-bit scale to its stored 11-bit exponent. An adjusted exponent below one
  clears all eight bytes. An adjusted exponent above 0x7ff first sets the
  mantissa/exponent payload to ones and then repacks the unmasked wrapped
  16-bit exponent; for example +1.0 scaled by 1025 produces raw bits
  0x800fffffffffffff, not IEEE positive infinity. Exact zero bypasses, while
  negative zero enters the nonzero path because its sign bit is set.
- Firmware 1.04 RAM:feef53 decomposes binary64 by writing stored exponent
  minus 0x3fe and replacing the stored exponent field with 0x3fe while
  preserving sign and fraction. Only raw positive zero takes the zero case;
  negative zero produces exponent -1022 and raw mantissa
  0xbfe0000000000000. Subnormal fractions are not shifted, and exponent-0x7ff
  payloads produce exponent 1025 while retaining their sign/fraction under
  the replacement exponent.
- Firmware 1.04 RAM:fee5e0 splits its R3 input into low and high nibbles,
  indexes the adjacent table at RAM:fee607 (`3f 06 5b 4f 66 6d 7d 27 7f 6f
  77 7c 33 5e 79 71`), and returns the low-nibble pattern in R3 and the
  high-nibble pattern in R2. The pair is faithfully representable as one
  big-endian `u16`; no direct canonical callers require an inherited-register
  adapter.
- Firmware 1.04 RAM:fd9231 is a no-input floating-runtime result tail that
  returns raw binary64 bits `0xffffffffffffffff` across DR0:DR4 and terminates
  with ERET. C251 `FLOAT64` represents that register pair with an ordinary
  `double` return, so the function does not require an assembly adapter.
- Firmware 1.04 RAM:fd923d returns raw positive infinity
  `0x7ff0000000000000` when BITS:0685 is clear and raw negative infinity
  `0xfff0000000000000` when it is set. C251 supplies a byte argument in ACC
  and a FLOAT64 result in DR0:DR4, allowing the implicit bit input to be
  replaced by an explicit Boolean C parameter; native tails need only
  materialize that bit as zero or one in ACC.
- Firmware 1.04 RAM:fd929d returns raw binary32 positive infinity
  `0x7f800000` when BITS:0685 is clear and negative infinity `0xff800000`
  when set. An explicit C byte sign arrives in ACC and the `u32` result leaves
  in DR4, so native runtime tails need only convert the bit to an ACC Boolean.
- Firmware 1.04 RAM:fd9238 is a no-input positive binary64 zero tail: it
  clears DR4, copies that zero to DR0 and returns through ERET. An ordinary
  C251 `FLOAT64` `double` return uses the same composite DR0:DR4 frame.
- Firmware 1.04 RAM:fee780 clears R0, R1 and R2 while retaining its inherited
  R3 byte, then stores that byte through displacement -0x1877. In the 24-bit
  address space the destination is exactly RAM:ffe789 plus the unsigned byte;
  all 256 possible inputs remain within RAM:ffe789..ffe888.
- Firmware 1.04 RAM:fd954c performs signed byte division with dividend in ACC
  and divisor in B, returning the truncation-toward-zero quotient in ACC and a
  remainder carrying the dividend's sign in B. Its three canonical callers
  consume only ACC. The divisor-zero `DIV AB` result is architecturally
  undefined and is not part of the portable C contract.
- Firmware 1.04 RAM:fd956e accepts signed dividend in WR6 and divisor in WR4,
  then returns truncation-toward-zero quotient in WR6 and a dividend-signed
  remainder in WR4. C251 maps a four-byte struct ordered `{ remainder,
  quotient }` onto exactly that WR4/WR6 result pair, so the complete dual
  result is representable in C without an assembly adapter. Divisor-zero
  output remains architecturally undefined.
- Firmware 1.04 RAM:fd9602 accepts signed dividend in DR4 and divisor in DR0,
  then returns truncation-toward-zero quotient in DR4 and a dividend-signed
  remainder in DR0. C251 maps an eight-byte struct ordered `{ remainder,
  quotient }` onto exactly that DR0/DR4 result pair. The overflow pair
  `INT32_MIN / -1` wraps the quotient to raw `0x80000000` with zero remainder;
  divisor-zero behavior is not part of the portable contract.
- Firmware 1.04 RAM:fd95b1 accepts unsigned dividend in DR4 and divisor in
  DR0, then returns quotient in DR4 and remainder in DR0. C251 maps an
  eight-byte struct ordered `{ remainder, quotient }` onto that exact result
  pair. Its nonzero domain is ordinary unsigned division. On divisor zero,
  fixed-language P-code proves word `DIV` leaves its operand registers
  unchanged: a dividend at most `0xffff` returns itself with remainder zero;
  a wider dividend returns `(low_word << 16) | 0xffff` with `low_word`
  remainder after the native sixteen-step continuation.
- Firmware 1.04 RAM:fee555 is a pure four-byte tail thunk to RAM:fee559.
  C251 assigns a sole `u32` argument to DR0 and, at optimization level eight,
  compiles `RestartCoroutine1AndYield(entry_point)` as exactly one extended
  jump to the nonlocal implementation. No ABI adapter or added call frame is
  required. RAM:fee559 is now a C target primitive that writes the entry point
  and saved SP image explicitly before restoring the scheduler SP and exiting
  with ERET.
- Firmware 1.04 RAM:feb6bb is a scheduler-installed, non-returning C-safe
  coordinator rather than a context-switch primitive. It maps fifteen
  RAM:00130a values to raw-read mode/speed tuples, passes the two configuration
  updates as `(RAM:00fb96, 0x3c, 2)` and `(RAM:00fad6, 0x3e, 1)`, and reaches
  every terminal state through an ordinary call to a low-level coroutine
  primitive or an infinite `YieldCoroutine0` loop. C251 preserves those
  call tuples and emits no return path, so coroutine scheduling alone does not
  require retaining a same-address assembly body when the function does not
  manipulate SPX or saved register frames itself.
- Firmware 1.04 RAM:fd8fc5 returns the strict binary64 less-than condition in
  CY; equality leaves the condition clear, and RAM:f84a30 proves a negative
  operand compared with zero takes the carry-set path. Because every
  canonical caller has a C reconstruction, the private paired-register/CY
  interface can be replaced by `u8 CompareFloat64(double, double)` without an
  assembly adapter; C251 normalizes its FLOAT64 comparison to zero or one.
- Firmware 1.04 RAM:fd915a tests the raw binary64 sign before exponent class:
  every negative encoding, including negative zero, returns the fixed
  all-ones NaN. Nonnegative exponent-zero encodings pass through unchanged,
  while every nonnegative exponent-0x7ff encoding (including NaN payloads)
  maps to positive infinity. Positive finite normals follow square-root
  semantics; an ordinary FLOAT64 C wrapper can preserve these gates and tail
  into C251's `sqrt` runtime because all canonical callers are reconstructed C.
- Firmware 1.04 RAM:fdbe7d is a C-safe scheduler coroutine despite its
  nonreturning entry: it does not manipulate SPX or saved coroutine frames.
  Its one irreducible low-level operation is the opcode-table dispatch at
  RAM:f83ef7, whose addressless adapter reads RAM:000b0a, loads the 24-bit
  target and performs `ECALL @DR4`. The surrounding startup, idle service,
  twelve-byte CDB copy, ATA/SCSI routing and completion loop compile as
  ordinary C251 control flow with no return path.
- Firmware 1.04 RAM:fdd3bf is C-safe even though it is interrupt-owned:
  RAM:f8007f performs the register-save and RETI shell, while the called
  worker uses the ordinary extended-call/ERET contract. Its C body can own
  host status, command and transfer dispatch; only direct bit-space access
  needs addressless near adapters. The controller-mode-three branch is an
  intentional infinite trap, not a decompiler artifact.
- Firmware 1.04 RAM:f9d777 has one genuine uninitialized automatic float32
  read on its 4..10 route. Declaring the near function reentrant makes C251
  keep that object on the native stack; a volatile uninitialized C object
  preserves the residue contract without guessing a value and compiles with
  zero diagnostics. The second interpolation table is not independently
  uninitialized: its low endpoint is the last float computed by the first
  table loop. Firmware 1.03 contains the identical native body.
- Firmware 1.04 RAM:fd9488 is the binary32 analogue of RAM:fd8fc5: its
  externally useful result is the strict-ordering condition in CY. Once all
  callers are reconstructed C, it can be represented as
  `u8 CompareFloat32(float, float)`; C251 normalizes the condition to zero
  or one and returns through ERET.
- Firmware 1.04 RAM:fd8d05/fd8e16/fd8f26 use the same raw exponent-class
  policy already established for the binary32 runtime: exponent-zero inputs
  are treated as zero, invalid zero/infinity combinations return fixed
  all-ones NaN, multiplication/division produce signed infinity for the
  applicable exponent-0x7ff or zero-divisor cases, and finite underflow
  flushes to positive zero. With all canonical callers in C, ordinary
  FLOAT64 wrappers can expose those rules and delegate finite arithmetic to
  C251's DPADD/DPMUL/DPDIV runtime. RAM:fd8d01 is raw right-sign inversion
  followed by AddFloat64 and optimizes to a tail transfer.

## Reconstruction identity and evidence storage

- Canonical C function identity is the compact source tag
  `/* RAM:xxxxxx */`. The 1,717 unique tags match the 1,717 live function
  entries in `/validated/MONT_104.bin`; repeated source occurrences represent
  intentional host/target or semantic variants at the same address.
- Per-function native ABI, behavior, confidence, and validation evidence
  belongs in canonical Ghidra plate comments. A separate reconstruction
  manifest or generated progress report is not an analysis authority and is
  unnecessary once its unique evidence has been migrated to Ghidra.

## C251 direct-bit source representation

- Keil C251 5.60 can emit direct atomic 80251 bit operations from C without an
  A251 wrapper. Declare SFR bits by their encoded 8051 bit address (for example
  BITS:0446/0447 as `sbit` addresses 0x8e/0x8f), and declare internal
  bit-addressable RAM through a `bdata` byte plus `sbit byte ^ index`. At O8,
  simple C assignments and reads emitted direct CLR, SETB, and MOV-C/RLC-A
  sequences with the appropriate near RET or extended ERET.
- A251/C251 symbol decoration encodes the return class: a near RET boundary is
  exported without a trailing `?` and must be declared
  `PLEXTOR_NEAR_FUNCTION` by every C caller; an ERET boundary uses the trailing
  `?` far symbol. Mixing those forms produces L251 unresolved externals even
  when the spelling otherwise matches.
- After the source-boundary audit, four A51 units remain: inherited/custom
  register and monitor/runtime glue in `arithmetic_boundaries.a51`, the
  RAM:ff4500 monitor programmer call, one inherited-WR6 return boundary, and
  six fixed reset/interrupt veneers. Relocatable A51 exports use far decorated
  symbols and `ERET`; the veneers are four-byte `EJMP` instructions. Direct
  bits and ordinary typed calls compile cleanly as C251 and do not need
  assembly representations.

## Operational main-entry build wiring

- With `MODSRC`, C251 emits `src/platform/coroutine_initialization.c` into the
  module segment `?PR?COROUTINE_INITIALIZATION`; `FirmwareMainLoop` is the first
  emitted function at segment offset zero. L251 can therefore place the
  reconstructed canonical entry at `RAM:fdcea9` by locating that module segment
  at `0xfdcea9`. The explicit link inputs must include both
  `coroutine_initialization.obj` and `firmware_entry.obj`; merely declaring their
  CMake custom commands does not make them reachable from the firmware target.

## C251 optimizer baseline evidence

- In a 19-function firmware 1.04 corpus, C251 5.60 O4 and O5 preserve the
  complete 111-byte `RAM:f8f47a` and 49-byte `RAM:f8f4e9` bodies exactly. O6
  changes both, and O7's tail merging/simplification produces substantially
  smaller, structurally different code. O3 is a plausible lower bound, while
  `OPTIMIZE(5,SIZE)` is the highest evidence-compatible practical default;
  this does not identify the original compiler release or flags.
- The reconstruction's explicit `REGPARMS PARM251` controls emit the same
  function bytes as C251 5.60's implicit defaults. Among TINY, SMALL, XTINY,
  XSMALL, and LARGE at O5, all retain the same two exact corpus functions;
  LARGE gives the closest aggregate size and agrees with the firmware's common
  external-data treatment of low-address globals.
- Every firmware C translation unit now uses the shared O5/SIZE setting. The
  seven units that formerly carried explicit O8 controls and the status-mapping
  unit that formerly used O1 all compile at O5/SIZE with zero diagnostics;
  `FLOAT64` remains an orthogonal per-unit numeric-width control.

## Absolute code-address relocation

- Firmware 1.04 source references 25 exact coroutine/function entry addresses
  as stored runtime values. C251 5.60 O5/SIZE can represent them without
  canonical numeric literals: casting a ROM(HUGE) function symbol to `u32`
  emits `WORD0`/`WORD2` relocations, while a near function emits `WORD0` plus
  `?C?CODESEG`. Host builds can retain the canonical numeric value solely for
  observable ABI assertions. These entries now use the shared definitions in
  `firmware_code_addresses.h`.
- The reproducible source/Ghidra absolute-address audit initially found 292
  unique high-priority ROM/overlay addresses: 262 data, 25 exact function
  entries, three function-interior code-as-data reads, and two undefined
  literal addresses. Removing the 25 numeric entry dependencies leaves 267
  addresses requiring owned data emission or an explicitly verified fixed
  hardware/overlay contract.
- The firmware 1.04 SCSI table at `RAM:f83ef7` has 256 four-byte big-endian
  code pointers: 78 supported opcode entries selecting 77 distinct handlers,
  and 178 entries selecting `HandleUnsupportedScsiCommand` at `RAM:faff96`.
  `command_dispatch_map.inc` is the symbolic reconstruction, and
  `verify_scsi_dispatch_map.py` verifies every route against the immutable
  image and canonical `RAM:` tags. Admission must test symbolic opcode support,
  not compare pointer bytes at the fixed-data copy `RAM:ff9691`.
- The firmware 1.04 byte table at `RAM:f842f7` is Boolean: 50 opcode entries
  require media and 206 do not. `command_media_requirement_map.inc` replaces
  that fixed-ROM lookup and is checked by the same dispatch-map verifier.

## Absolute ROM-data relocation

- All remaining firmware 1.04 internal ROM data dependencies fall within an
  80-byte image header, the contiguous `RAM:f82320..f848ff` range (9,696
  bytes), the contiguous `RAM:ff8000..ff9ffd` range (8,190 bytes), or three
  adjacent function-interior reads spanning `RAM:f894e3..f8952b` (73 bytes).
  `generate_firmware_rom_data.py` accepts only the immutable 1.04 SHA-256 and
  emits the 9,696-byte relocatable range and 73-byte code-as-data range. The
  packer owns the immutable header and fixed tail, eliminating 8,270 bytes of
  duplicate linked constants while restoring them at canonical offsets.
- C251 5.60 O5/SIZE represents an address into a far constant array with
  `WORD0 symbol+offset` and `WORD2 symbol+offset` fixups. Consequently target
  source can use linker-resolved array symbols while host builds retain
  canonical numeric addresses for behavioral tests.
- Configuration storage at `RAM:f7c000..f7c060` is an intentionally fixed
  flash/execution alias. Addresses in the separately mapped
  `RAM:ff0000..ffffff` recovery/runtime image are intentionally fixed overlay
  contracts, not relocatable main-image objects. The source audit distinguishes
  both from internal ROM references and currently reports zero unresolved
  priority addresses across C, headers, and retained A251 assembly.
- Relocatable generated objects occupy linker-selected HCONST. The packer
  copies `RAM:ff8000..ff9ffd` from the hash-pinned reference image and verifies
  the exact 8,190-byte extent, so physical fixed-data ownership is explicit and
  independent of linker placement. The final image verifier can now require
  byte identity of that range against the SHA-256-pinned stock 1.04 image,
  independently of additive checksum; a checksum-valid mutation is rejected
  and the current reconstructed image passes (`tests/test_firmware_image.py`).

## Keil firmware link-manifest ownership

- A hand-maintained L251 object CSV plus a second append list is not a safe
  firmware manifest. The former arrangement passed 422 entries containing 19
  duplicate object names but omitted 40 declared C251 translation units. The
  omissions included command-processing, host-interrupt, mode-page, controller,
  and media-state owners and therefore appeared misleadingly as unresolved
  project symbols.
- `keil_step` now registers every declared C251/A251 `.obj` output in one global
  property, rejects duplicate registrations, generates `link_manifest.txt`,
  makes all registered objects link dependencies, and replaces the L251 input
  placeholder with that unique list. The resulting manifest contains 445
  produced and linked objects with no duplicates, omissions, or extras.
- After manifest correction, `audit_keil_link_manifest.py` separated runtime,
  decoration, and target-definition failures. Correcting 99 caller-side
  near/far declarations, enabling the target implementation block formerly
  hidden by `PLEXTOR_HOST_TEST`, adding the omitted `timing.c` unit, and adding
  evidence-backed addressless adapters reduced project-owned unresolved names
  to zero. L251 then selects `C2SL`, `C2SFPL`, and `C2SDPL` for all required
  C251 runtime/math helpers; the complete diagnostic link has zero unresolved
  symbols.

## Physical firmware link constraints

- The linker-owned main payload is the contiguous range
  `RAM:f80050..feffff`; `RAM:ff0000..ff7fff` remains an erased recovery hole
  and `RAM:ff8000..ff9ffd` remains packer-owned fixed data. The payload provides
  458,672 bytes shared by ECODE, HCONST, and the final CODE page.
- The verified physical partition is ECODE `RAM:f80050..feb4ff`, HCONST
  `RAM:feb500..fefeff`, and CODE `RAM:feff00..feffff`. On 2026-09-20 the
  446-object O5/SIZE/LARGE build used 439,131 ECODE bytes, 18,510 HCONST bytes,
  and 49 CODE bytes; L251 and OH251 completed with zero warnings and errors.
- The packed 512-KiB reconstruction has SHA-256
  `563ee1634a7a061e070de5305c508de384355257d0219e8b56b13d7a4db45ad8`
  and checksum `0x6d86`. Matching container and monitor checksum calculations
  prove structural integrity only, not behavioral equivalence or flash safety.
- All 1,189 former L251 improper-fixup errors were cross-bank `LCALL` fixups to
  relocatable near functions. Converting those boundaries to the far 24-bit
  call model removed every improper fixup.
- Six canonical vector locations contain four-byte `EJMP` veneers at
  `RAM:f80050`, `RAM:f80054`, `RAM:f8007f`, `RAM:f800cb`, `RAM:f800f6`, and
  `RAM:f80123`. Their C interrupt/reset bodies remain relocatable.
- The map contains 94 one-byte absolute DATA segments over the nine
  bit-addressable bytes `RAM:0020..RAM:0028`. Exactly 85 segment overlaps are
  intentional aliases. `audit_keil_absolute_data.py` verifies every entry and
  overlap before the build suppresses L251 warning 30.
- On the C251 target, native volatile `huge` 16/32-bit accesses are materially
  smaller than hand-expanded byte accessors for the affected big-endian memory
  operations; host-test builds retain bytewise emulation. Encoding the two
  SCSI opcode predicates as verified 256-bit maps also reduced linked payload
  size while preserving all 256 truth-table entries. These source-shape
  changes recovered 5,237 bytes without changing compiler options. Direct
  target `sfr`/`sbit` declarations likewise let C251 use native bit operations:
  two timer/P1 routines shrank by 350 bytes while their host-test access path
  remained unchanged. A target-only LC898095 base pointer also reduced one
  repeated-write reset routine by 118 bytes without reordering its MMIO writes.

## Keil S251 differential-testing patterns

- µVision's S251 simulator can run the stock source-mode image and the linked
  reconstruction headlessly through Wine. `rwatch(address)` executes before a
  CPU read and can supply a queued value; `wwatch(address)` executes after a
  CPU write and can record the written value. For 80251 direct SFR accesses the
  debugger space is `D:`, not the user-defined `S:` space.
- Isolated dependency shims must jump to a verified canonical/listing RET site
  for near-return functions. Writing a synthetic RET at `entry+4` corrupts
  short functions and is unsafe. ERET dependencies can return directly from a
  relocatable shim.
- Original and reconstructed callees may use different parameter registers.
  Canonical ABI metadata describes stock calls; reconstructed dependency
  arguments must be recovered from the C251 listing prologue and normalized by
  semantic parameter name before trace comparison.
- Timer- and scheduler-dependent isolated functions require declared callee
  side effects. The generated A251 trace shim can apply near-RAM byte writes or
  BITS writes after recording a dependency call, allowing deterministic yield
  ticks and completion flags without executing the full scheduler.
- Internal BITS addresses `0x00..0x7f` map to bit-addressable DATA bytes
  `I:0x20..I:0x2f`; for example `BITS:0009` is `I:0x21.1`. This permits
  deterministic initial bit state in a debugger script without conflating it
  with EDATA.
- A constant passed through a generic `volatile u8 near *` SFR accessor may
  compile to `MOV A,@WRn`, which accesses near RAM rather than a direct SFR.
  Named C251 `sfr` operands are required when the canonical instruction is a
  direct SFR access. Extended address `0xe3` is not declared as an ordinary
  C251 `sfr` in this project and retains the explicit near-address form.
- Non-returning functions stopped at a semantic checkpoint may have different
  compiler-local partial stack frames. Normalize this as a non-return boundary;
  stack-balance comparison remains applicable to actual returns.
- Runtime differential execution does not require Ghidra when the reviewed
  canonical function/ABI snapshot is committed and hash-bound to the immutable
  stock image. Ghidra remains an authoring/refresh tool, not a test dependency.
- Resume identity must include normalized case content, not only case IDs. It
  must also bind the runner, simulator executable/project, stock and linked
  images, map, metadata, optional monitor image, and generated shim source;
  otherwise a same-named edited case can incorrectly reuse a stale result.
- The `RAM:ff0000` runtime-monitor capture is not part of the stock MONT image
  and lives under ignored local data. Direct cases that stub monitor contracts
  must not require it; a case that executes monitor code declares
  `load_monitor: true` and requires an explicit capture input.
- S251 `twatch(n)` can deliver deterministic state-timed SFR/RAM writes in a
  debugger signal; a two-event RAM write case matched stock and reconstructed
  traces and memory deltas. A timed Timer0 SFR request from the reset path
  reached the simulator vector alias, but this alone does not prove router
  execution. In the current source-mode project, `LOAD` of the 64-KiB captured
  monitor as Intel HEX left direct `C:0xff3fe2` reads at `0xff`; code bytes
  must be verified after loading before accepting a monitor/interrupt case.
  A251/L251 ECODE segments placed at `RAM:ff0000..ffffff` load as an ABS in
  S251. With an explicit generic Timer0 vector alias at `RAM:ff000b`, a timed
  Timer0 request routed through the captured monitor and main handler; RETI
  restored `SP=7` and resumed the saved reset PC in stock and reconstruction.
  With `RAM:0000fc=0`, the same captured router instead selects the monitor
  handler at `RAM:ff03a6`. An authentic Timer0 request through that path
  increments `RAM:0011ce`, executes the captured `RETI` at `RAM:ff03d2`, and
  returns to the interrupted reset PC with `SP=7` in both images.
  The captured `INT0` vector at `RAM:ff0003` jumps to the controller-event
  router `RAM:ff3f84`, while `INT1` at `RAM:ff0013` jumps to the controller-
  packet router `RAM:ff3fb2`. With selector zero, timed S251 external-
  interrupt requests execute the captured monitor handlers at `RAM:ff0304`
  and `RAM:ff037b` and their `RETI` instructions, restoring `SP=7` and the
  interrupted reset PC. The event handler increments `RAM:0011cd` and writes
  controller acknowledgement registers in both image runs; the packet case
  establishes routing and context return only under its declared inputs.
  In main-selector mode, the same physical `INT1` route reaches firmware
  1.04 `HandleControllerPacketInterrupt` at `RAM:fcfce6`. Declared
  `RAM:01808e` bit 6 and ready `RAM:018015` bit 5 take its short branch:
  it writes `RAM:01808c=0x87`, leaves `RAM:00125a=0xc0`, and executes
  authentic `RETI`. With bit 6 clear, ready asserted, and constant
  `RAM:01808d=0xa5`, it captures ten bytes at `RAM:00124f..001258`,
  leaves `RAM:0011d9=1` and `RAM:00125a=0x80`, and returns through `RETI`.
  Both routes restore `SP=7` at the interrupted reset PC in stock and
  reconstruction. S251 far-controller read/write watch signals did not
  observe those accesses even though RAM snapshots showed their effects;
  the current cases prove final state and return, not ordered bus accesses.
- S251's debugger-visible huge buffer for the integrated four-byte INQUIRY
  transfer is at `RAM:010000` even though the firmware source computes a
  controller buffer bank stride of `0x100000`; both stock and reconstruction
  placed `[05, 80, 00, 21]` there. Case observation bytes must be stored before
  a nonoverlapping simulator state record: the old fixed `RAM:017f20` record
  was overwritten by observations of 28 bytes or more and produced false
  near-24-bit state counts. The runner now places that record after the
  observation payload and stop PC.
- The captured runtime monitor and downloadable main image overlap at
  `RAM:ff0000..ff7fff` in physical address, but S251 does not expose one
  uniform view for the monitor bootstrap. Its sum at `RAM:ff4dc9..ff4e25`
  accumulates image-specific bytes over `RAM:f80000..ff9ffe`: stock `0xb3bd`,
  reconstruction `0x56f3`. The following `MOV WR6,@DR0` at `RAM:ff4e25`
  initially sees the stock stored word `0xb3bd` in both runs. A simulator-only
  patch of the two bytes at `RAM:ff9ffe` from each current image's file offset
  `0x79ffe` restores the intended comparison without changing the summed
  range. With that mapping, both images reach normal handoff `RAM:ff3f7d`;
  changing one checksum-covered reset byte without updating the word sends
  both to recovery entry `RAM:ff0300`. This is offline monitor decision
  evidence, not a physical flash bank or programmer model.
- For firmware 1.04 with the captured monitor loaded, physical reset
  reaches the first `ScsiCommandProcessorCoroutine0` entry through the real
  `ResumeCoroutine0` stack switch in both stock and reconstructed images.
  At that edge, saved scheduler SPX at `RAM:00115a..00115d` is
  `00 00 00 0a`, coroutine-zero's initial saved SPX at
  `RAM:00115e..001161` remains `00 00 03 07`, and low `SP` is zero.
  The versioned differential case normalizes only relocated coroutine-one,
  -two, and -three entry pointers.
  Replaying the captured scheduler SPX and side-specific internal stack
  permits a valid WRITE BUFFER mode-4 packet (`offset=0x10`, `length=4`)
  to yield through `ResumeCoroutine1` in both images. A controlled
  `ResumeCoroutine0` continuation reaches `ExecuteBufferDataTransfer` with
  `ACC=0x3b`, `DR0=4`, `DR4=0x10`, then the DATA OUT helper with
  `RAM:000c33=8`. Declaring four payload bytes at `RAM:100010..100013`
  and status `RAM:000c33=0x0d` after the controller setup causes both
  images to reach the DATA OUT helper return with matching length,
  destination, data, and cleanup. This is a controlled coroutine/transport
  checkpoint. A later controlled continuation reaches handler return and command idle with matching ordered controller events; physical DMA remains modeled.

- In the captured 1.04 runtime monitor, a successful main/auxiliary
  relocated-programmer path at `monitor_service_relocated::000bf2..000bf9`
  writes `0x99` to `RAM:0000fd` and jumps to bootstrap `RAM:ff0200`.
  Bootstrap calls `CheckForcedRecoveryCondition` at `RAM:ff4e72` from
  `RAM:ff021e` before main-image handoff. These exact instructions and
  flow pass the read-only canonical validator
  (`out/agent-tests/reset_handoff_validation.log`). They establish a CPU
  handoff route, not a physical flash outcome.

- In the captured 64-KiB monitor, `CheckForcedRecoveryCondition` at
  `RAM:ff4e72` tests `BITS:0401`, `BITS:0402`, `BITS:0406`, and `BITS:0486`
  before the main-image checksum. In S251, writing only the `SFR:P0/P1`
  latches to `0x46/0x08` after monitor initialization leaves the first three
  bit tests' `R15` count at two. Pausing at `RAM:ff4e72` and setting both the
  latches and external `PORT0/PORT1` pins to `0x46/0x08` produces `R15=0`,
  predicate `ACC=1`, and a branch to recovery entry `RAM:ff0300` in 15,839
  states for both intact 1.04 images. The two-stage harness records the exact
  pause PC; this is an offline input-model result, not physical switch proof.
- A disposable image variant replaces the 1.04 main reset veneer at
  `RAM:f80050` from `8a fd ce a9` with `8a f8 00 50`, an `EJMP` back to
  `RAM:f80050`. The byte-sum delta is minus 300; the corresponding valid
  stored checksums are `0xb291` for the stock-derived image and `0x756c`
  for the current reconstruction-derived image. Both standalone variants pass
  `tools/firmware_image.py verify`. The captured monitor accepts their
  checksums on its normal branch and reaches `RAM:ff3f7d`, while the declared
  forced-recovery inputs route them to `RAM:ff0300` before the self-loop can
  execute. This demonstrates the offline recovery decision for a checksum-
  valid but nonfunctional main entry, not physical flash recovery.

- In firmware 1.04 with the captured runtime monitor, an offline image
  whose first 4-KiB main sector `RAM:f80000..f80fff` is filled with `0xff`
  while the stored checksum is unchanged reaches monitor recovery entry
  `RAM:ff0300` with `ACC=0` for both the stock and current reconstructed
  image. The versioned case uses a compact symmetric fill patch and requires
  the observed ACC value. This is a conditional first-erase aftermath, not
  evidence of physical erase or successful reflashing.

- In the S251 differential harness, explicit `BS C:0xff4000` and
  `BS C:0x000800` execution breakpoints can guard WRITE BUFFER rejection
  scenarios against entering the captured runtime loader or its relocated
  programmer. A disposable `EJMP RAM:ff4000` at the handler entry stopped at
  `RAM:ff4000` and failed the expected-return assertion. This establishes a
  working loader-entry trap, not a flash-bus write observer.

- In the captured 1.04 runtime monitor, the relocated main/auxiliary flash
  programmer's erase setup at `monitor_service_relocated::000824..0008b1`
  reaches a `MOV @DR28,WR24` at `::0008b1` with `DR28` selecting programming
  address `RAM:f78000` and `WR24=0x3000` on the `0x2000` device-code path
  for both operations 3 and 5.
  Operation 5's later byte-program loop skips `RAM:f78000..f7bfff`. The
  firmware 1.04 staging copy at `RAM:fd68ba` instead reads 0x60 bytes from
  runtime `RAM:ffc000` into `RAM:10c000`, which operation 5 maps to
  programming `RAM:f7c000`. These are instruction-level facts; whether the
  erase command changes the corresponding execution-view bytes requires a
  validated flash/alias model or physical observation.

- In an isolated firmware-1.04 S251 operation-5 probe, the captured
  `RAM:ff4000` loader copies `0x40e` bytes to `RAM:000800`. With the declared
  normalized `0x2000` flash ID, stock and reconstructed images reach
  `RAM:00082b` after 12,166 states, following the first unlock-store
  instruction at `RAM:000828`; `DR16=0xf0aaaa` and `WR24=0xaa00` match.
  A single `G` toward the later `RAM:0008b1` checkpoint stops at
  `RAM:00082b` because S251 yields after a mapped flash-window store. The
  reproducible controlled-resume tool in
  `tests/simulator/trace_runtime_flash.py` records fourteen post-store
  checkpoints in the declared `0x2000` operation-5 path, through
  `RAM:0008c5`. Both images issue `AA/55/80/AA/55` at
  `RAM:f0aaaa`/`RAM:f05554`, then `0x3000` at programming
  `RAM:f00000..f60000` every 64 KiB, `RAM:f78000` and `RAM:f7c000`.
  These are CPU writes, not observed chip effects. The first trace record
  layout overlaid SP on the state counter and falsely suggested a final
  counter drop. With separate fields and paired PC samples in the 32-byte record, stock
  and reconstructed counters rise from 12,166 to 12,292 while
  PC/address/word/SP match.
  A parallel case with declared `RAM:0000f8=0xbf00` selects the alternate
  branch and reaches its first unlock-store successor `RAM:0008d6` after
  12,176 states in both images. Controlled resumes record six matching
  stores, ending with `0x5000` at programming `RAM:f00000` and post-store
  `RAM:0008f9`. Neither declared ID is a physical-drive measurement.

- The first six stores in the declared `0x2000` operation-5 trace form a
  complete SST-style sector-erase sequence at programming `RAM:f00000`
  only under an assumed high-byte command lane. The inspected part's
  manufacturer datasheet requires a fresh six-write sequence per sector
  erase and ignores commands issued during internal erase; subsequent bare
  `0x3000` CPU stores cannot be treated as confirmed sector effects. Under
  the additional `+0x80000` execution alias assumption, accepting the first
  erase would wipe executable `RAM:f80000..f80fff` including reset entry
  `RAM:f80050`. The disposable model changes 3,682 non-FF stock bytes and
  recomputes checksum `0xf55a` against stored `0xb3bd`; the current
  reconstructed image (SHA-256 `8178fc3c8f3189c9665fed5bfa33e234b3d56ec6da27715bbbada83bea23760a`)
  changes 4,010 bytes and computes `0x34fe` against stored `0x782d`.
  The declared `0xbf00` branch instead forms an SST-style `0x5000`
  block-erase command at programming `RAM:f00000`; under the same assumptions,
  the corresponding execution range is `RAM:f80000..f8ffff` and the reset
  veneer is erased. The disposable model changes 63,469 stock bytes and
  computes `0x481a` versus stored `0xb3bd` (current reconstruction:
  65,136 bytes, `0x1fc9` versus `0x782d`). Operation 5's declared program ranges exclude
  the first effect on both paths, so the range-guarded model rejects each.
  The actual drive's device-code result,
  bus lane, alias and chip response are still unobserved.

- Board inspection identifies the PX-W5224TA flash as SST39LF400A. Its
  manufacturer datasheet specifies a 512-KiB x16 part with uniform 4-KiB
  sectors and 64-KiB blocks, erased by `0x30` and `0x50` commands respectively.
  The captured programmer has both command paths and selects through the
  near-RAM result at `RAM:0000f8`; the physical drive's result and LC898095
  CPU-to-flash alias are not observed. If `RAM:f78000` aliases the first
  fixed-data sector at `RAM:ff8000` and a `0x30` command reaches this chip,
  the stock image would lose 3,730 non-`0xff` bytes and its computed checksum
  would be `0x8ef0` rather than stored `0xb3bd`. This is a reproducible
  read-only conditional calculation, not a measured erase.

- After many S251/Wine project launches on 2026-09-24, UV4 returned zero for
  fresh integrated projects while writing no debugger result or pre-run
  snapshots, even though the separate `emulator-smoke` target passed and all
  hashes of the application, overlays, runner, and cases matched prior passes.
  With no active UV4 process, restarting the idle Wine service via
  `wineserver -k` restored a fresh reset case and all seven integrated cases.
  Missing debugger artifacts must be classified as an invalid simulator run,
  never as a firmware behavior result.

- For integrated firmware 1.04 SCSI dispatch from `RAM:fa34b2`, two far-call
  trampoline slots at `RAM:f70000` and `RAM:f70004` let S251 run two commands
  without resetting shared state. The harness records and verifies the first
  return PC and `SP` before changing the CDB. Unsupported opcode followed by
  18-byte REQUEST SENSE, with a state-2000 write of 4 to `RAM:000c33`, yields
  identical `70 00 05 ... 24 00` response bytes at `RAM:010000` and balanced
  stacks in stock and reconstruction. This checks two dispatcher calls; packet
  ingress and the outer coroutine idle transition require separate scenarios.

- In S251, a firmware `MOVX @DPTR` access to the controller window with
  `DPTR=0x8000` is intercepted by `rwatch(X:0x8000)` and written with
  `_WBYTE(X:0x8000, value)`. A plain watch at `0x018000` does not fire,
  although the case still names the canonical controller address
  `RAM:018000`. This was verified with a twelve-byte INQUIRY packet in both
  firmware-1.04 stock and reconstructed command coroutines; both produce
  matching CDB bytes and fourteen hardware read events (two SFR:D7, twelve
  controller-window) before data-in completion. A `wwatch(X:0x804a)` positive
  control also records the command-code writes `0x80` and `0xc0` through
  status publication.

- Firmware 1.04 `ScsiCommandProcessorCoroutine0` at `RAM:fdbe7d` accepts a
  second packet when `RAM:000c32` is set to `0x11` at the first completion's
  idle-loop edge. Both stock and reconstruction consume another twelve bytes
  from controller `RAM:018000`, return packet state to one, and publish a
  second completion. INQUIRY then unsupported opcode and unsupported opcode
  then REQUEST SENSE passed differential scenarios with verified first-edge
  PC/stack, second completion, controller event order, and payload/sense bytes.
  This proves two packet transitions at the edge; executing the intervening
  idle-yield body remains a separate scheduler scenario.

- In a firmware-1.04 packet-ingress one-block READ(10) differential case,
  both images reach the first `YieldCoroutine0` in the shared raw-sector
  engine. They set raw-read mode two, target frame `RAM:001598=150`, and
  coroutine-two entry `RAM:f90000` or its linked counterpart. Both enter
  canonical `InitializeControllerBufferCursorRegistersIfNeeded` at
  `RAM:f92955`. A direct launch with default shallow SP let the linked
  call tree leave persistent bit byte `RAM:0022=0xf8`, consistent with a
  return-address spill (`SP=0x21` at helper entry versus stock `SP=0x15`), falsely
  suppressing cursor-port writes. With initial `SP=0x40`, both retain
  `RAM:0022=0x10`, match controller `RAM:018004=0x80` and
  `RAM:01802d=0x20`, and record ordered writes `0x8004=[0,0x80]`,
  `0x802d=[0x20]` before the yield. Direct C251 coroutine launches need a
  stack base that cannot trample firmware bit-addressable RAM; this case
  establishes acquisition setup and cursor writes, not sector completion.

- Firmware 1.04 `ScsiCommandProcessorCoroutine0` also accepts READ BUFFER
  mode 3 from a twelve-byte controller packet. With `RAM:001556=0x00001000`
  and four-byte allocation, both images return `[00, 00, 10, 00]` in the
  descriptor and controller payload, publish command codes `0x80` and `0xc0`,
  and reach the idle-loop edge (4,839 / 4,895 states). A state-4,500 data-in
  completion falls after transfer setup in both implementations; an earlier
  state-3,500 completion was overwritten, while state 6,000 let stock enter a
  coroutine yield before completion. The accepted case records 17 matching
  hardware events. WRITE BUFFER mode 4 also rejects a capacity overrun
  (`0x0ff0 + 0x20 > 0x1000`) before transfer with `05/24/00` sense and no
  runtime-loader entry (612 / 873 states). These are specific input-domain
  scenarios, not complete read/write buffer coverage.

- In the captured firmware-1.04 runtime, the physical `RAM:ff0033` vector
  jumps to the SFR-E1 router at `RAM:ff3fc9`. A bounded S251 case with a
  declared generic-Timer0-vector alias to that router and main selector
  `RAM:0000fc=1` runs the main handler, clears `SFR:E1` bit 7, executes
  authentic `RETI`, and resumes the interrupted reset PC with `SP=7` in
  stock and reconstruction (259 / 320 states). This validates the router,
  handler, and context restoration under the explicit alias; actual E1 source
  delivery is not established by the generic S251 peripheral model.

- In firmware 1.04's captured runtime, physical vector `RAM:ff000b` jumps
  to host-interface router `RAM:ff3f9b`. S251's generic Timer0 source enters
  that physical slot without a code patch. Monitor selection writes
  controller `RAM:018059=0x17`, `RAM:01805b=0x37`, then
  `RAM:018059=0x97` and executes authentic `RETI` (376 / 377 states).
  Main selection with a zero controller cause records `RAM:00faeb=1`, writes
  `RAM:018059=0x17`, `RAM:018002=0`, `RAM:01805b=0x37`, then
  `RAM:018059=0x97`, and executes `RETI` (370 / 353 states). Both restore
  `SP=7` at the interrupted reset PC with matching ordered hardware traces.
  Generic source injection proves these router/handler paths, not LC898095
  host IRQ signaling or nonzero-cause handling.

- In firmware 1.04, a READ CAPACITY(10) packet through
  `ScsiCommandProcessorCoroutine0` at `RAM:fdbe7d` is admitted when media
  status `RAM:000fae=0` and ready bit one at `RAM:000c85` is set. With
  `RAM:0012c1=0x00001000` and `RAM:000b00=0x0800`, both stock and
  reconstruction return `[00, 00, 0f, 69, 00, 00, 08, 00]` through the
  eight-byte DATA IN response, reflecting the firmware's 150-block offset
  and last-LBA decrement. A second unsupported packet in the same coroutine
  returns `05/24/00` sense while retaining the capacity payload and
  publishing command codes `[0x80, 0xc0, 0xc0]`. This is a bounded offline
  geometry-response case with declared media state and host transfer
  completion; it does not exercise disc acquisition or sector reads.

- In the 2026-09-23 linked 1.04 reconstruction, relocated `RAM:f8a806` and
  `RAM:f8a80a` lie in `WriteControllerParameterWord`'s SFR:D7 low-nibble
  completion poll (`RAM:f8a7dc` entry in the current L251 map). The same
  numeric addresses lie in stock `WaitControllerPortReadyAndReadByte` at
  canonical `RAM:f8a803`, as confirmed by the read-only Ghidra listing.
  Simulator stop PCs in the linked image must be resolved through that
  image's map and listing before assigning canonical behavior. A bounded
  S251 reset probe advanced when D7 zero completion responses were supplied;
  static controller-ready responses alone did not clear this poll.

- In S251's firmware-1.04 reset scenario, stock `@DR` reads of controller
  `RAM:018015` are intercepted by `rwatch(0x018015)`; `X:0x8015` and
  `E:0x018015` did not intercept that stock helper. The linked C251
  `MOVX @DPTR` route is intercepted by `rwatch(X:0x8015)`. The harness's
  `watch: "RAM24"` changes only debugger watch placement and keeps the event
  identity `CONTROLLER:018015`. At the drive-state-10 entry the two modes
  captured identical 151-event input transcripts in stock and reconstruction.
  For writes, stock `wwatch(0x018046)` reported the old zero while
  `wwatch(X:0x8046)` reported two; S251 write values for such accesses need
  instruction and alias checks before interpretation.
  In the 1.04 main-mode physical INT0 no-pending case, the two stock reads
  of controller `RAM:018003` and `RAM:01807f` instead require default `X:`
  read watches, while the linked C dispatcher requires `watch: "RAM24"`.
  Side-specific placement yields identical read events and the same mask,
  status and restored `SP=7` after authentic `RETI` (361 / 407 states).
  S251 watch placement must therefore be checked at each access site; it
  cannot be inferred globally from stock or reconstructed provenance. With
  active-low bit one in the event port (`RAM:018003=0xfd`) and its pending
  flag at `RAM:000fad=0x02`, both physical INT0 traces clear that flag, set
  `RAM:001302=0x20`, and return with `SP=7` (381 / 427 states).
  The same declared event while coroutine zero is yielded, and separately
  while it executes configuration reading, preserves scheduler progress to
  the next resume checkpoint with matching saved contexts and `SP=10` in
  both stock and reconstructed images. The active case retains `SFR:A8=0x83`
  because startup enables Timer0 as well as the declared INT0 enable.

- For firmware 1.04 `InitializeControllerAsic` at `RAM:fdb787`, the canonical
  three-byte address registers at controller `RAM:018013..018015` are written
  through `MOVX @DPTR` (for example `RAM:fdb92b..fdb93e`). The reconstructed
  `WriteControllerAddress24` helper must use `FirmwareWriteController8` so
  C251 emits `MOVX`, not the generic 24-bit huge-store route. The current
  image compiles with zero C251 warnings/errors and passes the targeted
  direct `RAM:fdb787` differential case. An earlier integrated-reset
  `RAM:018013` mismatch was traced to the separate
  `ConfigureMainDataRingGeometry` source function, whose byte order was
  corrected; the `InitializeControllerAsic` write form remains unchanged.

- In the offline S251 firmware-1.04 reset model, a one-shot Timer0 request
  can be represented by `SFR:88=0x20` (TF0 pending, TR0 clear) with
  `SFR:A8=0x82`. Injection at state 1,000 occurs inside coroutine-zero
  `ReadDriveConfigurationBytes` (stock `RAM:fd66d9`; linked `RAM:f83c0b`).
  The captured monitor router selects the main handler; both images record
  exactly one timer phase/tick at `RAM:001231`/`RAM:001232`, restore
  `SFR:A8=0x82`, `SFR:88=0`, `SFR:A9=0`, and reach the next scheduler
  resume with `SP=10`. This is a declared simulator interrupt model, not a
  measurement of the physical timer cadence.

- Firmware 1.04 `ResetRawReadQueueAndTransferState` at `RAM:f927bd` reads the
  entire big-endian 32-bit ring slot count from `RAM:00156a`, then stores the
  low 16 bits of count minus one and count minus six at `RAM:00129a` and
  `RAM:00129c`. Canonical instructions `RAM:f927d7`/`f927e1` load DR4;
  `RAM:f927db` decrements DR4, `RAM:f927e5` subtracts six, and
  `RAM:f927dd`/`f927e9` store WR6. For count `0x00000200`, the required
  four bytes are `01 ff 01 fa`; the current host test and direct S251
  stock/reconstruction case both produce them. Reading only the upper
  16 bits of the count produced `ff ff ff fa` and was a reconstruction defect.

- At the firmware-1.04 fourth scheduler resume checkpoint under the declared
  offline startup response model, the saved coroutine SPX values are identical
  in stock and reconstruction: `RAM:000508` for coroutine one, `RAM:000707`
  for coroutine two, and `RAM:000907` for coroutine three. The live four-byte
  pointers at `RAM:000501`, `RAM:000700`, and `RAM:000900` are exact semantic
  counterparts. Coroutine one saves stock `RAM:f84bf9`, immediately after
  `ECALL YieldCoroutine1` at `RAM:f84bf5`; linked `RAM:fb2e5c` immediately
  follows the same call in the C251 listing. Coroutine two saves stock
  `RAM:f84a12`, immediately after its `YieldCoroutine2` call at
  `RAM:f84a0e`; linked `RAM:fce693` immediately follows that call. Coroutine
  three initially saves the `IdleCoroutine3` entry (`RAM:f84b00` /
  `RAM:fce69b`). At the second `ResumeCoroutine0` checkpoint after one full
  scheduler cycle, it saves `RAM:f84b09` / `RAM:fce6a4`, the immediate
  successors of its `YieldCoroutine3` call. Forty-two other differing
  stack bytes lie above the saved SPX values and are not live frames at
  these checkpoints. Exact per-image pointer assertions are required;
  function membership alone would miss an incorrect internal return site.
  The remaining `RAM:018046` controller snapshot difference persists despite
  identical ordered `0x02` write watches, so the physical bus value is not
  established by S251's final XDATA view.

- In the S251 firmware-1.04 fourth-resume probe, `SAVE` of explicit
  `X:0x8010..8050` and `E:0x018010..018050` creates distinct controller
  snapshots: the XDATA view is encoded at HEX `0x018010..018050`, while the
  EDATA view is encoded at HEX `0xf2018010..f2018050`. At offsets 0x13 and
  0x46, the EDATA view is zero for stock and reconstruction; XDATA is zero
  for stock and `0x12`/`0x02` for reconstruction. An ordered XDATA
  write-watch at the same checkpoint records `RAM:018046=0x02` then
  `RAM:018013=0x00` for stock, versus `0x02` then `0x12` for the prior
  reconstruction. Canonical `ConfigureMainDataRingGeometry` instructions
  subsequently proved the reconstructed byte order was wrong. After that
  correction, the ordered XDATA write-watch records `0x02` then `0x00` in
  both images. The remaining `RAM:018046` saved-memory mismatch still
  requires address-view or hardware-bus interpretation.

- Firmware 1.04 `ConfigureMainDataRingGeometry` at `RAM:fdbdff` writes the
  low, middle, then high bytes of its 24-bit ring size to controller
  `RAM:018013..018015`. Canonical `RAM:fdbe42` loads the 32-bit size,
  `RAM:fdbe46` and `RAM:fdbe4a` place its two words on the stack, then
  `RAM:fdbe4e..fdbe64` load `@SPX`, `@SPX-1`, and `@SPX-2` into ACC before
  successive `MOVX @DPTR` stores. Mode one (`0x00093000`) yields bytes
  `00 30 09`; mode two (`0x00126000`) yields `00 60 12`. The former C
  reconstruction wrote high/middle/low; the corrected function now passes
  focused host vectors and direct stock/reconstruction differential cases
  for both modes. The fourth-resume controller write transcript at
  `RAM:018013` now matches zero in stock and reconstruction.

- In the 1.04 two-cycle reset/scheduler extension, both stock and reconstructed
  code reach the captured `RAM:ff4500` configuration programmer. Its relocated
  body at `RAM:000845` stores `0x3000` to programming `RAM:f7a000`; at the
  repeated `RAM:000872` poll the retained stock register capture has
  `DR28=RAM:f7a000`, and `RAM:00086d` copies it to `DR16` before the read at
  `RAM:00086f`. Both downloadable images have an all-`0xff`
  `RAM:ffa000..ffafff` sector under the inferred `+0x80000` execution alias.
  The physical alias and response remain unmeasured, so this does not prove
  a chip erase or an `0xff` polling result.

- For the 1.04 scheduler configuration-flash path, S251 can advance both
  stock and reconstructed images from the `RAM:000872` erase poll to
  `RAM:00090e` when the test explicitly maps `RAM:f7a000..f7afff` and
  supplies `0xff` at `RAM:f7a000` after reaching the poll. The paired
  debugger snapshots of `RAM:000800..000a8d` equal the immutable 1.04
  monitor capture byte for byte. A uVision `$` PC capture can transiently
  report zero at the first `RAM:000818` post-entry stop; later stops and
  readback should be checked explicitly. This model says nothing about the
  physical flash reply or completion of the following program sequence.

- In the 1.04 configuration-flash programmer’s `B != 0` route, canonical
  `configuration_flash_relocated::0008e3..0008f3` selects 32 words from
  `RAM:001680` for target programming `RAM:f7a060` onward. Under a declared
  S251 direct-memory map, stock and reconstruction complete 31 matching
  word iterations with matching target, source and count progression. The
  next-unlock stop is `RAM:00090e` where the debugger captured a nonzero PC;
  one reconstructed sample is zero and is reported as a capture gap. The 62
  modeled target bytes equal the 62 source bytes in both images. This verifies the CPU loop under
  that model, not SST busy behavior or a physical write. A subsequent
  bounded uVision Tstep trace verifies the 32nd word as well: all 64 target
  bytes match `RAM:001680..0016bf`, the count reaches zero, both executions
  follow the same relocated PCs through `RAM:000a8d ERET`, and `ACC=1`
  survives the return. Stock resumes at `RAM:fd649f`; the reconstructed
  addressless adapter resumes at `RAM:f800d9` before its C caller. The
  previously observed invalid PC follows later caller execution, not the
  final program word.

- Canonical 1.04 `ProgramAndVerifyDriveConfiguration` at `RAM:fd643b` reads
  the full 32-bit count at `RAM:00156a` with `MOV DR4,0x00156a` at
  `RAM:fd661f` and again at `RAM:fd6629`; it subtracts 1 and 6 respectively
  before writing the low 16 bits to `RAM:00129a` and `RAM:00129c`. A 16-bit
  big-endian read at `RAM:00156a` instead takes the high word, giving wrong
  limits for count `0x00000200`. The corrected C uses the existing 32-bit
  near accessor, and the isolated host CTest regression passes. In the same
  canonical function, `RAM:fd645d MOV R12,R7` tests the low byte of
  `ReadDriveConfigurationWord(0x000f)` for the P3 handoff; the test fixture
  must seed `RAM:ffc01f`, not `RAM:ffc01e`, when declaring value 1.

- When C251 compilation starts a persistent Wine server outside Xvfb, later
  headless uVision runs can return without debugger artifacts even though
  `xvfb-run` itself provides a working X display. A bounded `WINEDEBUG`
  diagnostic reported `nodrv_CreateWindow` and explorer startup failure,
  while an independent S251 smoke project lacked its readiness marker.
  With no active Wine application, stopping that idle server and launching
  Wine anew inside `xvfb-run` restored the smoke test and then complete
  17-case integrated and 25-case monitor differential runs on the same
  corrected image. Empty uVision artifacts are infrastructure failures,
  never firmware evidence.

- On 2026-09-24, two C251 attempts to rebuild the large `leaf_workers.c`
  translation unit inside the crowded `out/simulator/build/firmware/Release`
  directory generated complete zero-error listings but stalled before an
  object appeared. The same source and exact compiler arguments completed in
  about five seconds in a clean agent-owned directory, yielding a hash-recorded
  object. Copying that object and its metadata into the agent build tree and
  restatting all four Ninja output paths (including absolute-path aliases)
  allowed L251, the absolute-DATA audit, OH251, packing and validation to
  complete with zero errors. The resulting image is SHA-256
  `2c6e774a886c791cce180dbf57174c6fc68b136c9582d045a56026e7f2b3e1ef`.

- For firmware 1.04, canonical `CopyFromControllerBufferWindow` at `RAM:fde0a6` takes destination in DR0, controller-relative offset in DR4, and a word count through the native parameter slot. A differential mode-5 call-entry trace exposed the reconstructed argument reversal. With the C signature ordered as `(controller_offset, destination, byte_count)`, C251 places destination in DR0 and offset in DR4; the current call-entry trace matches stock. The controller data window is `RAM:100000` plus the offset. This establishes the call contract, not complete mode-5 validation.

- The S251 automatic HDATA relocation must inspect the linked instruction rows of the verified function body only. Scanning the whole translation-unit listing for each reachable function changed an unrelated `CopyFromControllerBufferWindow` `ADD WR0,#0x0010` immediate to `0x0001` when another function in `memory.c` declared a bank-`0x10` relocation. The original image, generated memory ABS/HEX, and C251 listing all contained `0x0010`, while the old debugger run read `0x0001`. A synthetic same-listing two-function test now rejects cross-function patching, and the corrected mode-5 first-read trace again has controller-window source `RAM:100000` on both sides. This is a simulator fixture defect, not a firmware arithmetic defect.

- The differential runner's `--monitor-hex` argument must receive the assembled simulator monitor application (`out/simulator/images/monitor.abs`, SHA-256 `51570409be33d1d9b01dfbd960aec2fb31e136a04fab0e8ceba84148f2a4d486`). The immutable 64-KiB `data/read_memory_ff0000.debug` is the raw source capture, not a loadable S251 application. A run supplied with that raw capture stopped in the monitor at `RAM:ff458a`; the same Timer0 case passed when supplied with the assembled ABS, and the full 29-case monitor catalog then passed.

- In the S251 offline 1.04 mode-5 controller-window copy, debugger writes to `E:100000` alias the displayed `E:010000` byte, but a native register-indirect read at `RAM:100000` returns zero and stops before the next store. The same instruction with DR8=`RAM:010000` completes and reads the seeded `X:0000` byte. A reproducible differential revision-rejection trace explicitly changes only the verified copy helper bank immediate from `0x10` to `0x01` in each debugger-loaded image, seeds staged ASCII `00` at `X:001a..001b`, and observes both images reach `SetSenseData(4, 0x40f5)` after the 48-byte copy. This is an S251 fixture behavior and conditional simulator result, not physical controller-window alias evidence (`tests/simulator/trace_mode5_revision_rejection.py`; current image `2b965412b1e539d0b2cf0da1b27e78b901e4b1e87c950344b10014eed39e1979`).

- In firmware 1.04 main-mode host-interface ISR, a declared controller cause `0x02` at `RAM:01805a` snapshots four bytes from `RAM:001455..58` to `RAM:001459..5c`, marks `RAM:001596=1`, clears `RAM:001462`, and sets host completion flags bit 4 at `RAM:000c83`. Cause `0x04` changes an active transfer state at `RAM:000c33` from 8 to `0x0d`. Both differential cases enter the captured `RAM:ff000b` vector/router, match four ordered controller acknowledgement writes, execute `RETI`, and restore `SP=7` at the interrupted reset PC on stock and reconstructed images. S251 injects through its generic Timer0 source, so this establishes routed handler behavior, not physical host-IRQ source delivery (`tests/simulator/monitor-cases/integration.json`, current image `2b965412b1e539d0b2cf0da1b27e78b901e4b1e87c950344b10014eed39e1979`).

- In a firmware-1.04 one-block READ(10) packet scenario with ring count `RAM:00156a=0x00000200`, stock and reconstructed code set `RAM:00129a..00129d` to `01 ff 01 fa`, target raw frame `RAM:001598=150`, and return from the first command `YieldCoroutine0` to a captured scheduler SPX `0x0000000a`. Entering the real scheduler `ECALL ResumeCoroutine2` at stock `RAM:fdcf87` / linked `RAM:fdcf6a` then runs the primary raw-read task at stock `RAM:f90000` / linked `RAM:fde81b` with SPX `0x700` to its first `YieldCoroutine2`, with `RAM:000f05=2`, `RAM:000e44=1`, and `RAM:00127e=1`. The media yield returns to the exact scheduler call successor; both images naturally reach the next `ResumeCoroutine0` call, enter batch service and yield the command again with 0/1 blocks complete. A direct jump to the `ResumeCoroutine2` function omits its scheduler ECALL return frame and cannot establish that return path. This is bounded command/media wait behavior, not sector transfer or completion (`tests/simulator/cases/integration.json`, `tests/simulator/trace_read10_wait_cycle.py`; current image `2b965412b1e539d0b2cf0da1b27e78b901e4b1e87c950344b10014eed39e1979`).

- In a firmware-1.04 one-block READ(10) S251 continuation, an empty raw-sector
  descriptor ring has base `RAM:0088bb`, 512-slot mask `0x01ff`, and block size
  `0x0800` on both stock and current reconstructed images. A declared
  status-zero descriptor with controller source `0x002000` makes both enter
  `ConfigureAndTransferRawReadSector` and set controller command `0x88`, length
  `0x0800`, source `0x002010`, trigger `0x50`, control `0x80`, argument `2`, and
  status `0x97`. Both reach the next command yield with `RAM:001459=1` and
  transfer latch `RAM:001462=1`. These final controller bytes use S251's
  `X:0x80xx` view. The synthetic descriptor does not prove real sector
  acquisition, payload bytes, write ordering, interrupt completion, or command
  idle (`tests/simulator/trace_read10_transfer_start.py`;
  `out/agent-tests/read10-transfer-start-final-xvfb/results.json`).

- In the firmware-1.04 READ(10) synthetic-descriptor continuation, clearing
  `RAM:00127e` at descriptor insertion models the store made by canonical
  `AcquireRawReadPrefixAtCurrentPipelineTarget` (`RAM:fe3b86`). A declared
  host cause `0x04` then routes through `RAM:ff000b` to normal raw-read host
  completion in both stock and current reconstructed images. The transfer
  latch `RAM:001462` clears at the fourth instruction step in each completion
  worker. Both return through the ISR and `YieldCoroutine0` to the scheduler;
  a controlled real `ResumeCoroutine0` ECALL then reaches the READ(10) engine
  return and next idle yield with `RAM:000c32=1`, `RAM:001226=4`,
  `RAM:001462=0`, and issued/requested `1/1`. The post-handler path uses fixed
  instruction steps and skips unrelated scheduler coroutine turns. No media
  payload, subsequent packet, or physical IRQ timing is established
  (`tests/simulator/trace_read10_data_in_interrupt.py`,
  `tests/simulator/trace_read10_command_idle.py`).

- Firmware 1.04 `HandleHostInterfaceInterrupt` at `RAM:fdd3bf` has two
  separately verified main-mode branches through the captured `RAM:ff000b`
  router. Declared controller cause `0x41` admits a packet from
  `RAM:000c32=1` to `0x11` and sets SCSI protocol `RAM:000c78=1`. Cause
  `0x30` with host state zero publishes completion flags `RAM:000c83=0x18`,
  operation flags `RAM:001588=0x21`, copies status/error bytes, and makes
  15 ordered controller writes through its recovery-buffer response. Both
  branches match stock and reconstruction through `RETI` in bounded S251
  runs; the simulator uses a generic Timer0 request to enter the captured
  host vector, so physical interrupt wiring is not established.

- Firmware 1.04 host cause `0x10` is a status branch: with command flags bit
  five clear it sets `RAM:0000fd=0x98`, `RAM:0000fe=0xff`, and requests a
  firmware restart. The `RAM:f8007f` interrupt wrapper clears `RAM:0000fa`
  before entering `RestartFirmware`. A current-image paired S251 case executes
  that worker, matches three ordered controller writes and these RAM values,
  observes `SFR:A8` and `SFR:A9` cleared, and reaches
  `InitializeAndEnterFirmwareMain` in stock and reconstruction
  (`out/agent-tests/hostrestart_init_versioned/results.json`). Read-only runtime-capture
  disassembly shows `RAM:ff2004` increments WR6 inside a finite loop until
  `0x893`, then `RAM:ff2000` performs register initialization and returns.
  With two declared `SFR:91` timer overflows and two declared routed controller
  event-`0x40` completions, the staged stock and reconstructed restart paths
  both progress through two `0x8000`-byte buffer fills at `RAM:1b0000` and
  `RAM:1b8000` to a later timer wait (`tests/simulator/trace_host_restart_initialization.py`;
  `out/agent-tests/hostrestart_initialization_trace/host-interface-main-vector-status-cause10-initialization-entry-via-timer-source/results.json`).
  Restart completion, a post-restart scheduler turn, and electrical timer or
  controller delivery remain untested.

- In firmware 1.04, controller event `0x40` is active low in the mask read from `RAM:018003`. With a declared mask `0xbf` and source status `0xff`, the captured INT0 router executes `HandleControllerEvent40Completion`, writes `RAM:14da=1`, and returns through `RETI` in both stock and reconstructed images (423 / 452 S251 states; `tests/simulator/monitor-cases/integration.json`; `out/agent-tests/controller_event40_versioned/results.json`). This establishes routed handler behavior under a declared controller response, not physical controller timing.

- Firmware-1.04 `InitializeModePageStateFromFirmwareDefaults` makes its third `CopyBytes8` call at stock `RAM:fc2a73` with DR0=`RAM:2bc6ef`, DR4=`RAM:ff8a5a`, and ACC=8; canonical Ghidra decodes a three-byte `LCALL RAM:fc2a18` there. The current reconstructed call at `RAM:fd89a0` passes the same source, destination, and count with function-local register placement. The runtime-capture bytes at `RAM:ff8a5a..ff8a61` are `03 06 00 00 00 00 00 00`. In a paired S251 restart continuation with a declared destination-bank substitution from `RAM:2bc6ef` to `RAM:01c6ef`, both images copy those eight bytes (`out/agent-tests/hostrestart_modepage_copy_probe/host-interface-main-vector-status-cause10-initialization-entry-via-timer-source/results.json`). The substitution validates conditional copy behavior only; physical `RAM:2bxxxx` mapping and the stock caller return were not established by that probe.

- In firmware 1.04, `RunStartupStatusCoroutine1` yields directly from its coroutine body. A reconstructed nonreturning helper inserted an extra `LCALL` return frame across `YieldCoroutine1`, saving coroutine-one SPX `0x50a` where stock saved `0x507`. Removing that helper call makes the stock and reconstructed `YieldCoroutine1` entry SP `3`, saved SPX `0x507`, and scheduler SPX `0x0a` in paired S251 traces (`tests/simulator/trace_configuration_flash_status_yield.py`; latest reconstructed image SHA-256 `061ed2a1722b64030f362d537e6d8c1ffdc31c6619f0c961826db2f2c1ca4e76`; `out/agent-tests/csyfix/results.json`). Coroutines must preserve call frames across yields, not just RAM effects.

- In the firmware-1.04 configuration programmer path under S251's declared direct-memory flash response, the relocated programmer body occupies `RAM:000800..000a8d`, overlapping the saved coroutine-three return bytes at `RAM:000900..000903` (saved SPX `0x907`). On both stock and reconstructed images, those bytes become programmer code `b4 55 54 7e`; the next `ResumeCoroutine3` returns to invalid `RAM:55547e` (`tests/simulator/trace_configuration_flash_stack_overlap.py`; `out/agent-tests/csofix/results.json`). This establishes a shared simulator-memory overlap, not a physical crash: drive reset, coroutine reinitialization, or actual RAM banking could change the continuation.

- Firmware 1.04 `InitializeCoroutine3` writes the coroutine-three entry pointer to `RAM:000900..000903` and saved SPX `0x907` to `RAM:00116a..00116d`. In a declared S251 continuation after configuration programming, applying exactly those RAM effects at the next `ResumeCoroutine3` replaces the overwritten return slot; stock and the latest reconstruction both reach the following `ResumeCoroutine0` with `SP=10`, coroutine-one SPX `0x507`, coroutine-three SPX `0x907`, and `RAM:001009=3` (`tests/simulator/trace_configuration_flash_reinitialized_coroutine3.py`; `out/agent-tests/c3reinit_final/results.json`). This proves a conditional liveness path only. The observed natural path does not execute `InitializeCoroutine3` before that scheduler call, and the physical memory mapping/reset behavior remains unknown.

- Firmware 1.04 `ValidateStagedFirmwareChecksum` at stock `RAM:fd2c93` takes the start offset in DR0 and exclusive end offset in DR4. A paired auxiliary mode-5 call-entry trace found the reconstructed arguments reversed. With C251 formal order `(end_offset, start_offset)`, corresponding direct-call argument order, and inverted target binding, the current linked call emits DR0=0 and DR4=`0xfffe`, matching stock (`tests/simulator/trace_mode5_aux_checksum_entry.py`; reconstructed image SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c`; `out/agent-tests/m5auxentry_final/results.json`). Function-specific native ABI must be checked at the generated call instruction.

- The S251 auxiliary mode-5 checksum fixture shares `X:6000..7fff` with the candidate `X:0000..7fff` checksum region: the debugger view at function entry shows observation scratch there on both stock and reconstructed runs (`tests/simulator/trace_mode5_aux_staging_view.py`; `out/agent-tests/m5auxview_final/results.json`). The paired rejection still reaches written `70/04/40/F6` sense and command idle under declared memory (`tests/simulator/trace_mode5_aux_rejection_completion.py`; `out/agent-tests/m5auxidle_final/results.json`), but a simple staged-byte/stored-word pair is not a clean checksum acceptance control in this fixture. This is a simulator memory-layout fact, not a physical controller-window mapping.

- The S251 differential runner relocates firmware-1.04 high-HDATA bank `0x10` to bank `0x00` by default, and its normal trace scratch overlaps the auxiliary staged checksum range. In a case-local paired run, moving scratch to `X:a000`, `X:d000`, and `X:e000..efff`, then setting the three checksum-read bank immediates to simulator bank `0x01`, lets stock and current reconstruction validate identical declared auxiliary bytes with sum and stored word `0x0001` (`tests/simulator/trace_mode5_aux_clean_scratch.py`, `tests/simulator/trace_mode5_aux_clean_acceptance.py`; `out/agent-tests/m5auxcleanaccept_final/results.json`; reconstructed image SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c`). This is a debugger memory substitution; it does not establish physical bank mapping.

- In the same paired firmware-1.04 auxiliary accepted path, seeding `RAM:ffc000..ffc05f` with the 96 configuration bytes from immutable `data/read_memory_ff0000.debug` (SHA-256 `7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80`) and mapping the stage write to simulator bank `0x01` makes both images change 87 staged bytes and copy all 96 bytes to `RAM:10c000..10c05f`. Both then reach `RAM:ff4000` with DR0=`0x00050000`, DR4=0, and DR8=`RAM:ff4000` before any loader instruction executes (`tests/simulator/trace_mode5_aux_clean_handoff.py`; `out/agent-tests/m5auxcleanhandoff_final2/results.json`). The assembled S251 monitor had `0xff` in that configuration window until the capture bytes were declared. This proves conditional configuration staging and pre-flash handoff, not a physical flash write.

- With firmware-1.04 main mode-5 revision and header accepted under declared S251 controller-window mapping, stock and current reconstruction call `ValidateStagedFirmwareChecksum` with DR0=0 and DR4=`0x79ffe` (`tests/simulator/trace_mode5_main_checksum_entry.py`; `out/agent-tests/m5mainchecksum_entry_probe/results.json`; reconstructed image SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c`). Both bounded continuations then stop with the source cursor at `RAM:020000` (stock DR4, reconstruction DR0) before checksum decision (`tests/simulator/trace_mode5_main_checksum_boundary.py`; `out/agent-tests/m5mainchecksum_boundary_final3/results.json`). The reconstructed S251 debugger may report PC zero at this invalid-bank halt. This establishes a shared simulator address-model limit, not checksum equivalence or physical staged data.

- In the current firmware-1.04 34-case monitor suite, the captured bootstrap routes both stock and reconstructed images to recovery loop `RAM:ff0300` with ACC=0 after either a declared 4-KiB sector erase at `RAM:f80000..f80fff` or a declared 64-KiB block erase at `RAM:f80000..f8ffff`, preserving each stored checksum word. Both cases execute 5,152,441 S251 states and pass in the same linked application/map/monitor aggregate (`tests/simulator/monitor-cases/integration.json`; `out/agent-tests/m34blockfull/results.json`; reconstructed image SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c`). This is a conditional monitor checksum/recovery decision, not a physical erase, activation, or reflashing result.

- In stock Plextor firmware 1.04, the immutable MODE SENSE(10) field-mask
  descriptor at `RAM:ff8cc9` is `0a 00 07 00 ff ff ff ff 00 00 fc`.
  `ValidateScsiCdbFieldMasks` treats the nine bytes after the reserved byte
  as prohibited-bit masks for CDB bytes 1..9. The read-only CDB
  `5a 00 03 00 00 00 00 00 10 00` has zero intersection with each mask.
  The accepted C reconstruction selects page-control zero/page 03, emits an
  eight-byte MODE SENSE(10) header, and copies the eight-byte current image
  at `RAM:2bc6ef` into response bytes 8..15. This establishes a static
  command/response comparison contract, not an observed hardware response.

- In firmware 1.04's main-mode host-interface interrupt, declared cause
  `0x01`, command state zero, packet state one, and controller status
  `RAM:018057=0` select the non-packet command branch: packet state becomes
  `0x10`, protocol becomes zero, host-command flag bit five clears, and the
  monitor router returns through `RETI` (414 stock / 417 reconstructed S251
  states; `out/agent-tests/hostcause01_probe_xdata/results.json`; current
  application SHA-256 `76069bba74cc771b329cbfa89dc9388759f7e00039c4c6faa2a3912c5653416e`).
  C251 emits `MOVX A,@DPTR` for the `RAM:018057` read, so S251's XDATA
  `X:8057` read watch captures it; a 24-bit RAM watch does not. This is
  conditional routed-interrupt evidence, not physical source delivery.

- Under a declared one-shot S251 host-vector source in firmware 1.04, host
  cause `0x02` can interrupt after the second scheduler `ResumeCoroutine0`
  while coroutine zero is yielded. With source count `12 34 56 78` seeded at
  `RAM:001455..58`, both stock and current reconstruction copy it to
  `RAM:001459..5c`, set `RAM:001596=1` and completion flags
  `RAM:000c83=0x10`, return through the monitor router, and reach the third
  `ResumeCoroutine0` with `SP=10`, scheduler SPX `0x0a`, and coroutine-zero
  SPX `0x307` (`out/agent-tests/hostcause02_yielded_nonzero/results.json`;
  current application SHA-256
  `76069bba74cc771b329cbfa89dc9388759f7e00039c4c6faa2a3912c5653416e`).
  This is conditional scheduler/host-ISR evidence, not physical IRQ delivery.

- In firmware 1.04, a one-shot generic S251 Timer0 vector declared as host
  cause `0x02` during active coroutine-zero `ReadDriveConfigurationBytes`
  leaves `RAM:001596=0` when the source count at `RAM:001455..58` is zero,
  but sets `RAM:000c83=0x10` and returns through the captured host router to
  the next `ResumeCoroutine1` in both stock and reconstruction. The bounded
  case has 395,642 / 215,772 states, SP=10, and matching source/saved zero
  counts (`out/agent-tests/hostcause02_active_probe_corrected/results.json`;
  application SHA-256
  `76069bba74cc771b329cbfa89dc9388759f7e00039c4c6faa2a3912c5653416e`).
  The scheduler-context-dependent completion byte must be checked against
  stock behavior; physical host IRQ delivery remains unmeasured.

- Firmware 1.04 stock `ValidateStagedFirmwareChecksum` at
  `RAM:fd2c93..fd2d3f` uses a 16-bit sum. For the caller
  `end_offset=0x79ffe`, it reads staged bytes at
  `RAM:100000..16ffff` and `RAM:178000..179ffd`, then compares the
  stored word at `RAM:179ffe`. For `end_offset=0xfffe`, it reads
  `RAM:100000..107fff` and `RAM:10ff00..10fffd`, then compares at
  `RAM:10fffe`. Read-only canonical instructions, including the
  exclusive-end comparisons and final `MOV AB,@DR0`, are retained in
  `out/agent-tests/staged_checksum_headless.log`. This is CPU address
  semantics; physical backing of those banks is unmeasured.

- The firmware 1.04 captured relocated flash-ID loader
  `flash_identification_relocated::000800..000869` reads a device word at
  `::000833` and stores it to `RAM:0000f8` at `::000836`; it keeps
  `0xbf00` or `0x6200` and writes normalized `0x2000` otherwise
  (`::00084b..000863`, read-only canonical listing in
  `out/agent-tests/flash_id_selector_headless.log`). On an INQUIRY-identified
  live PX-W5224A revision 1.04, the firmware's F5 selector-zero DATA-IN path
  returned `bf00` from `RAM:0000f8`, so that drive selects the alternate
  branch. This does not establish the physical flash bus lane.

- On the same live PX-W5224A 1.04, fixed F5 DATA-IN reads returned the exact
  immutable reset bytes `8a fd ce a9 a9 d3 d1 ca 2b ca 4b ca 5b ca 6b ca`
  from both `RAM:f80050` and `RAM:f00050`. An independent standard MODE
  SENSE(10) response also matched the F5 read of current Mode Page 03. The
  Representative reads also proved `RAM:f70000 == RAM:ff0000`,
  `RAM:f72a3e == RAM:ff2a3e`, and `RAM:f78000 == RAM:ff8000`; the returned
  bytes match the immutable runtime-monitor capture and stock fixed data. The
  Full 512-KiB captures of `RAM:f00000..f7ffff` and
  `RAM:f80000..ffffff` are byte-identical with SHA-256
  `c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`.
  The execution view exactly equals immutable stock 1.04 through
  `RAM:feffff` plus the immutable runtime-monitor capture at
  `RAM:ff0000..ffffff`. Exact successful CDBs, lengths, SCSI status, bytes,
  and backups are retained in
  `out/agent-tests/windows-readonly-w5224a-full-backup-20260925/`. This
  establishes normal-command CPU data-read aliasing and F5 semantics on
  that drive, not write aliasing or the mapping after firmware changes
  `BITS:0580/0581` for a programmer handoff.

- In stock firmware 1.04, `ProgramAndVerifyDriveConfiguration` at
  `RAM:fd643b` reads configuration word `0x000f` and, if nonzero, clears
  `BITS:0580` and sets `BITS:0581` before calling runtime
  `RAM:ff4500` (`RAM:fd647b..fd649d`). After a successful monitor return
  it sets `BITS:0580` and makes `BITS:0581` follow `RAM:000fae`
  (`RAM:fd64b7..fd64c9`). The failure return is before those later writes.
  Read-only canonical listing: `out/agent-tests/configuration_p3_headless.log`.
  These are CPU-visible bit transitions; LC898095 board wiring and any
  address-map effect remain unknown.

- Stock firmware 1.04 `HandleWriteBufferCommand` at `RAM:fd2741`
  clears `BITS:0580` and `BITS:0581` at `RAM:fd2b79..fd2b7b` before
  calling runtime `RAM:ff4000` with operation 3 or 5
  (`RAM:fd2b83..fd2bbb`; read-only canonical listing at
  `out/agent-tests/write_buffer_p3_headless.log`). The C reconstruction
  performs the same clears. The physical function of those P3 pins is
  unmeasured, so a normal-command F5 read alias cannot establish the
  programming-state flash mapping.

- Plextor’s PX-W5224TA installation manual calls the retail model
  `PX-W5224TA` and specifies `PX-W5224A` as its ATAPI INQUIRY string.
  The immutable 1.04 firmware image contains `PX-W5224A`, so the model
  naming in the PX-W5224TA board inspection is consistent with this
  firmware target. The teardown establishes components for the inspected
  board, not every production unit or the LC898095 bus wiring. See
  `docs/HARDWARE_PLATFORM.md` for the manual and teardown sources.

- Firmware 1.04 captured runtime monitor dispatches SCSI WRITE BUFFER
  opcode `0x3b` through table entry `RAM:ff7aec` to
  `HandleMonitorWriteBufferCommand` at `RAM:ff210d`, and READ BUFFER
  opcode `0x3c` through `RAM:ff7af0` to `RAM:ff25ae`. Mode 4 branches to
  `RAM:ff23cd` and calls DATA OUT worker `RAM:ff28a6` at `RAM:ff2427`;
  mode 5 branches to `RAM:ff243c` and can call the same worker at
  `RAM:ff249f` for a nonzero payload. Its mode-5 write path then calls
  staged-checksum validation at `RAM:ff2508`/`RAM:ff2526`,
  clears `BITS:0580/0581` at `RAM:ff2542..ff2544`, and calls the
  runtime `RAM:ff4000` programmer with operation 3 or 5 at
  `RAM:ff2554`/`RAM:ff2566`. The read-only canonical validator enforces
  the dispatch and loader flows;
  `out/agent-tests/monitor_buffer_commands_headless.log` retains the
  instruction listing. Physical monitor-mode transfer and reflashing
  remain unmeasured.

- Native Windows uVision is launched with each generated project directory as
  its working directory. Differential-run output roots must therefore be
  resolved to absolute paths before project and initialization-file paths are
  constructed. A relative direct-CLI `--output` otherwise makes uVision open
  the wrong project path and wait until the 60-second wall timeout, while the
  same CMake target works because CMake supplies an absolute output directory.
- Native Windows uVision still needs Keil `-j0`, a
  `STARTF_USESHOWWINDOW` startup record whose show state is `SW_HIDE`, and
  `CREATE_NO_WINDOW`, but those launch flags do not provide durable focus
  isolation: a long run later created another top-level window. Do not
  continuously force-hide that simulator window because uVision owns and pumps
  it while command-file automation runs. Instead start the whole runner through
  `tools/run_on_private_desktop.py`, which supplies an explicit non-interactive
  `WinSta0` desktop in `STARTUPINFOW.lpDesktop`, below-normal priority, and a
  kill-on-close job for the process tree. A real differential case passed with
  this arrangement, while its uVision child remained absent from the normal
  desktop. Native-Windows CMake uVision targets must use this launcher by
  default and declare it as a build dependency; direct runner invocations must
  be prefixed explicitly.
- On native Windows, `tools/run_windows_generated_catalog.py` provides the
  unattended current-image direct-catalog policy: a resumable 10-second broad
  traversal followed by 30-, 60-, and 120-second passes over errors only. It
  reads `results.json` after every pass, stops on a comparison mismatch, and
  keeps `simulator.py` unchanged so the existing run identity remains valid.
- A Windows `subprocess.run(..., timeout=...)` around uVision is not a complete
  hang boundary: one generated case left a responsive, CPU-consuming UV4 child
  alive for more than two minutes under a 10-second runner limit. The private
  launcher can therefore enumerate only the processes assigned to its own job,
  match a named descendant executable, and terminate it after a configured age.
  The generated-catalog driver watches `UV4.exe` at the selected runner timeout
  plus 30 seconds; a live short-limit child test verified the job-scoped kill.
  In the real generated catalog it later terminated an overdue UV4 child for
  `auto-fa1ac5` at exactly 40.0 seconds and the unchanged runner continued to
  subsequent cases, demonstrating that this boundary recovers the catalog
  without an orphan or resume-identity change.
- Keil C251 V5.60 does not accept the C99 `inline` keyword in target C. Small
  reconstruction-only helpers written as ordinary `static` functions become
  real `LCALL`/`RET` bodies and add stack boundaries absent from canonical
  code. When direct canonical instructions must remain at the public function's
  call site, use carefully scoped expression or `do { ... } while(0)` macros
  and confirm the target listing contains no helper function bodies. The
  `auto-fa56c5` reconciliation case is the regression example.
- The retained PX-W5224A updater `PXFirm3.exe` (SHA-256
  `a564786456d9cd1b16ab9c5e0177ab768d949ec923a78be91b996647a4dd50cd`)
  submits ten-byte CDBs through Windows
  `IOCTL_SCSI_PASS_THROUGH_DIRECT` (`0x0004d014`). With its hash-pinned
  `PlexFirm.ini` (`FirmwareFileSize=524288`, `BufferSize=16384`,
  `Use Update After Cmd=Yes`), the loop at VA `0x00406320` sends 31
  buffer-ID-zero WRITE BUFFER mode-4 chunks at offsets `0..0x78000` and the
  final chunk at offset `0x7c000` as mode 5. The pass-through wrapper uses a
  600-second timeout for WRITE BUFFER mode 5 versus ten seconds otherwise.
  This is host-command evidence only; it does not prove physical flash safety.
- In the same hash-pinned PXFirm3.exe, the update eligibility function at VA
  `0x00405cb0` checks the candidate revision's first byte at `0x00405dd7` and
  the selected drive revision's first byte at `0x00405de7`. If either is ASCII
  `0`, it reaches accepted successor `0x00405ead` without the later-version
  floating-point comparison. The protected monitor's `0.00` revision is
  therefore not rejected by this version gate. This does not prove live
  recovery enumeration or transfer. The offline update-plan generator enforces
  this evidence by mapping those addresses through the PE32 section table and
  checking the exact branch and success-return instruction bytes after the
  whole-file updater hash check; a byte mutation is rejected in focused tests.
- The SST39LF400A datasheet gives manufacturer ID word `0x00bf`, x16 unlock
  word addresses `0x5555`/`0x2aaa`, and command data in `DQ7..DQ0`. On the live
  PX-W5224A, the flash-identification loader's CPU-visible selector is `0xbf00`.
  Its byte swap agrees with the programmer's CPU words `0xaa00`/`0x5500`, while
  CPU byte offsets `0xaaaa`/`0x5554` divide by two to the documented x16 word
  addresses. Together with complete live `RAM:f00000..f7ffff` equals
  `RAM:f80000..ffffff` captures, this strongly establishes the stock command
  address and byte ordering. It does not itself observe a flash write effect.
- Do not confuse SCSI WRITE BUFFER mode 5 with the runtime programmer's internal
  operation number. Buffer ID zero—the stock full-image path—finalizes through
  internal operation 3. Buffer ID `0xff` selects internal operation 5 for the
  auxiliary/recovery/configuration path. An operation-5 erase-range concern is
  not evidence against the normal buffer-ID-zero operation-3 update.
- A live PX-W5224A updater must never retry an indeterminate final WRITE BUFFER
  mode-5 command: that command is self-activating and the drive may disappear
  while the host observes an error. Persist the exact CDB and candidate hash
  before submission, then use protected recovery and read-only identity/readback
  to determine state.
- A live redumper update of candidate `f3740520...a5f6` proved the recovered
  internal-operation-3 physical mapping: recovery-mode F5 readback matched the
  candidate exactly across `RAM:f80000..feffff` and `RAM:ff8000..ff9fff`, both
  CPU views were identical, and the live stored/computed checksum was `0xda36`.
  The protected monitor remained usable, but the reconstructed main firmware
  failed before host enumeration. This separates a semantic/boot defect in the
  reconstruction from transfer corruption or an incorrect physical flash map.
- Restoring immutable stock 1.04 with redumper after that failed candidate
  returns the same drive to normal revision-`1.04` operation. Its full
  DATA-IN-only CPU views match each other and stock main through `RAM:feffff`.
  The persistent tail values `RAM:ffa077=03` and `RAM:ffa07f=08` are present in
  this working stock state, so those two post-flash differences are not the
  reconstructed-image boot blocker
  (`out/agent-tests/windows-stock-restore-full-backup-20260925/report.json`).
- In stock firmware 1.04 cold-start initialization, `RAM:fdc529..fdc534`
  constructs and publishes `0x00ff5000` at `RAM:00130e`, `RAM:fdc535` reloads
  that four-byte value into `DR8`, and `RAM:fdc539` executes `ECALL @DR8`.
  A direct call reaches the same runtime loader but does not reproduce the
  inherited `DR8` state. The reconstruction's addressless flash-identification
  adapter therefore loads `DR8` from `EDATA 130EH` and uses the indirect call;
  the A251 listing emits `7e 2f 13 0e 99 28 aa`.
- On the physical PX-W5224A, diagnostic transitions on `BITS:0580/0581`
  produce separately visible yellow and green front-panel indications. The
  first-pass diagnostic image SHA-256
  `452eeb7a5f02a001891c7816ed0f056fa60f8b009df6f198f32d1c4c93699b3f`
  produced a complete 11-count final frame. Because that call is immediately
  after `ConfigureInitialCoroutines`, the observation proves physical return
  from the indirect `RAM:ff5000` flash-identification loader, the successful
  cold-initialization path, and initial coroutine configuration. It does not
  prove that any coroutine subsequently returns/yields or that host enumeration
  succeeds.
- The native firmware-1.04 coroutine boundary does not preserve a scheduler C
  local allocated in `DR12` (`R12..R15`) across `ResumeCoroutineN`.
  `ResumeCoroutineN` saves scheduler SPX, loads coroutine SPX, and pops the
  coroutine's `DR12`; `YieldCoroutineN` pushes coroutine `DR12`, restores
  scheduler SPX, and returns without restoring the scheduler's old `DR12`.
  A diagnostic local allocated to `R15` was therefore overwritten and caused
  checkpoint sequences `1..6`, `2..6`, and `2,5,6` on physical hardware.
  Persistent instrumentation state spanning resume/yield must live in an
  explicitly initialized memory byte/bitmask, not an automatic/register local.
  The first uninterrupted `1..6` remains valid proof that all four first-pass
  resume gates and one complete scheduler pass returned.
- With the PX-W5224A behind USB bridge `093b:0042`, successful USB descriptor,
  configuration, and GET MAX LUN traffic identifies the bridge only. A Windows
  device sound can be the bridge resetting and re-enumerating while the ATAPI
  target remains unresponsive. In the diagnostic-06 Bus Hound trace, repeated
  36-byte INQUIRY CBWs are followed by canceled bulk-IN requests and USB resets
  with no DATA-IN payload or CSW. Do not use bridge enumeration, the sound, or
  a front-panel LED state as evidence that the Plextor command completed.
- On the physical drive, diagnostic 07 (image SHA-256
  `ae4b11fca20ecc6480d87381561e53878b180df4d831b8e5831d8792b0973374`)
  produced its one-yellow/two-green breakpoint frame after the final DATA-IN
  setup write corresponding to stock 1.04 RAM:fdde3b. This proves physical
  execution of the shared DATA-IN setup path, not correct payload/address/
  length or completion. ATA IDENTIFY also calls this worker; an unqualified
  breakpoint there cannot independently identify INQUIRY as its caller.

- Stock 1.04 SCSI coroutine0 replaces the initial coroutine1 entry: at
  RAM:fdbfa7..fdbfcc, when RAM:00faef bit7 is set and RAM:001009 is zero,
  it calls InitializeCoroutine1 with RAM:f84b0b before yielding. Therefore
  the tested mode0 reset-to-scheduler seed with faef=80 first resumes drive initialization,
  even though ConfigureInitialCoroutines initially installs RAM:f84c30.
  Live canonical instructions and the paired continuation traces confirm
  this ordering; see docs/DIAG17_STARTUP_CONTINUATION_ANALYSIS.md.
  Qualification: this replacement occurs after host startup initialization.
  With RAM:000c79=1 and (RAM:0000fd & f9)!=99, fd9f6e calls the mode1
  handshake fda757, which yields C0 at fda832 before fdbe7d writes c32=1 or
  replaces C1. Thus the initial f84c30 entry can legitimately run with c32=0.
  Live canonical instructions and paired stock/rebuilt mode1 continuation
  confirm this. Zero c32 at startup entry alone is not evidence of a fault.
  Physical diag25 (SHA256 2ef2fdeca8b97c7958ec862da3b4af4c77a68b5b1e6dbd7b60361440aabdc925)
  reported1: tagged initial C1 entry, packet0, C0 saved return at the host
  handshake yield. This identifies that route under diagnostic timing, not
  subsequent handshake completion. See docs/DIAG25_STARTUP_ENTRY_ROUTE.md.

- S251 debugger SP is eight bits; assigning SP=0520 actually selects low SP20.
  Set and capture DR60 for the complete native SPX. Diag26 silent-return probes
  verify DR60=00000520 across calls. Corrected diag25 documentation retains its
  passing blink frame but withdraws the previous high-stack claim.

- Stock1.04 DelayTimerUnits (RAM:fdbda2) is shared beyond the host handshake:
  InitializeControllerAsic calls it with12 at RAM:fdb87c and9 at RAM:fdb895;
  configuration/input checks call it with2470 at RAM:fdc75c/fdc95c. A global
  validator accepting only59 traps legitimate cold initialization before the
  handshake. Live canonical xrefs/instructions confirm these callers. Scope
  handshake probes by call site, not an assumed universal delay argument.

- Physical diag27 (SHA256
  7bf565ba1f98082adc146621fce3149975d5382e2d8794df2cce5617de2f6f92)
  reports4: the actual host-handshake delay59 returned and its cleanup ran.
  This bounds progress under diagnostic timing, not later handshake completion
  or timer frequency. Stock1.04 uses separate software countdowns RAM:00121e
  and RAM:001204 in subsequent handshake phases; live TimerDispatchPhase2
  instructions decrement them at RAM:fd16a2..fd16a7 and fd192c..fd1931.

- Stock 1.04 f8b58c writes controller RAM:018002/018046/01804e/01806a;
  MOV @DPX,R12/R13 at f8b5d8/f8b5de inherits controller bank01. Its table
  words are at RAM:ff84fe + 2*profile and RAM:ff850a + 2*profile, emitted
  low byte first. Paired S251 marker cases distinguish both bank and index
  from the old near-RAM/minus-one reconstruction. Tests and actual early
  scheduler continuation exposed a surviving startup value2 at RAM:018046
  before the correction. See docs/DIAG18_CONTROLLER_RESET_ADDRESSING.md.

- Physical diag20 (SHA256 5eca42d41aaf065c7823ebabedd8b5465d0d9b3d7f5bbc5c0c6cb9b84004309a)
  reaches ProcessDriveStateMachine entry, evidenced by its literal eight-green
  frame; Windows detection still fails and tray stays closed. This bounds
  reachability under diagnostic timing, not a stopping location. Stock 1.04
  RAM:f84fb5..f84fbd yields C1 while RAM:00faeb is zero. Host ISR fdd3c3 sets
  that byte, but main fdcebd..fdced9 can preset it too: its nonzero value alone
  is not proof of an interrupt. Paired synthetic latch-ready/two-yield cases
  reach recognition fb9d58 with matching captured state (DIAG20/21 docs).

- In the standalone C4 project, C251 `REGPARMS` calls pass a controller
  port as WR6, while stock 1.04 RAM:f8a7dc/RAM:f8a803/RAM:f8a934 expect
  DR0=RAM:018xxx (WR0=1, WR2=port). C251 passes `u16,u16` in WR6,WR4;
  stock RAM:f8a828 expects WR4,WR6. C251 passes `u8,u8` in ACC,R7;
  stock RAM:f8f784 expects R7,ACC. The A251 bridges in `stock_calls.a51`
  adapt these registers. Verified against live canonical 1.04 call-site
  disassembly, Keil movement listing, and linked bridge bytes. These are
  function-specific contracts; do not generalize them to other stock calls.
  Stock 1.04 queue reset RAM:f927bd ends in ERET and can be reached by a
  far tail bridge. Alternate capture setup RAM:f92b52 ends in RET and can
  be reached through an LCALL/ERET bridge. Both calls replace unnecessary
  unchanged code copies in the C4 payload.
- Live PX-W5224A 1.04 H: observations on 2026-09-27 show Timer0 at about
  1 ms/tick and the stock RAM:001216 byte countdown at about 10 ms/decrement.
  The stock RAM:fb084e HOME has two 120-count motor phases and shorter settle
  waits; absent a sensor break it takes about 2.6 s. In the C4 adaptation,
  checking debounced RAM:0011ed bit 2 only after those settle waits can lose
  an in-loop sensor hit. HOME from D8-acquired lead-in finished in 156 ms yet
  returned `05/24/00`, while HOME from D8 LBA 120,000 took 2,609 ms and
  returned the same status; the short run suggested a sensor break, but two
  full-pass attempts with the later latched-result build from both lead-in
  and LBA 120,000 again returned `05/24/00`, so the physical sensor behavior
  and HOME motor-path integration remain unresolved.
  The stock HOME routine publishes `0x0855` even
  after both phases expire, so that estimate is not proof of sensor arrival.
  A C4 +50 controller-unit SEEK returned GOOD and immediately left the stock
  successful cache RAM:00fc2b..fc2f=`00 32 00 01 00`. Its unchanged radial
  estimate is expected for this disc: stock RAM:fd07de divides calibration
  RAM:000cf9=`0x062f` by 12000 before multiplying by distance, yielding zero.
  Live records: `out/agent-tests/gofw-c4-live-20260927/`.
- Stock F5 selector zero can read one-byte mapped controller addresses
  CPU:01849f, CPU:01849d, and CPU:0180f8 on the flashed PX-W5224A 1.04.
  Port 01849f changed after small C4 SEEKs and stock D8 reads, but also
  changed substantially without an explicit position command; its absolute
  physical meaning is unresolved. Do not use one post-command sample as a
  calibrated pickup-position reading. The read-only CDBs and observations are
  recorded in `out/agent-tests/gofw-c4-homefix-live-20260927/`.

- In stock PX-W5224A 1.04 D8 alternate raw read, RAM:fd6cf2 explicitly
  initializes coroutine2 at RAM:f929ab before yielding. That coroutine calls
  RAM:f92a69, whose acquisition request at RAM:fdccc1 dispatches through
  RAM:fdca07, setting RAM:000faf=2. RAM:f92d0b requires that value plus
  RAM:001300=1 before it enqueues captured frames. RAM:f92b52 by itself only
  programs the controller and enters state5; it does not perform the
  acquisition request or initialize coroutine2. RAM:fc683c's empty-queue
  fallback can initialize coroutine2 only when RAM:000f46=0. Copying the
  stock coroutine's RAM:000f46=1 prologue into a command without starting
  coroutine2 therefore disables that fallback. This is a static control-flow
  finding; it does not identify the exact live hang location.
- In the standalone C4 baseline compiled with Keil C251 5.60, a C call with
  `(s32 start, s32 exclusive_end, u8 format)` places `start` in DR4,
  `exclusive_end` in DR0, and `format` in ACC immediately before ECALL.
  This matches the stock PX-W5224A 1.04 call from RAM:fd6c64 to the raw-range
  function RAM:fd6cf2, whose tail at RAM:fd6fdf is ERET. A far EJMP bridge
  therefore preserves this particular call ABI. The generated-image alias
  at the three stock D8 opcode reads returns D8 only for C4 operation 2 and
  returns the actual opcode otherwise; physical baseline behavior is recorded
  below.
- Stock PX-W5224A 1.04 acquisition loop RAM:fc818b calls the near position
  routine RAM:fc858f from RAM:fc83c7 and consumes ACC. Its `LCALL` is **three
  bytes**, followed by the two-byte `MOV R14,ACC` at RAM:fc83ca. Replacing
  four bytes at that call site splits the result-store instruction and produced
  a physically hung image (SHA-256 `405b0ce8...a17c2`). The sentinel
  `0x10000000` inward 200-unit movement occurs inside RAM:fc858f at
  RAM:fc85f6 or RAM:fc8669. Its first four bytes are complete instructions
  (`PUSH R15; XRL B,B`), and its epilogue RAM:fc86c4 is `POP R15; RET`.
  The corrected offline hook replaces that four-byte entry with `EJMP`,
  replays the prologue on the stock path, and enters the FC-bank epilogue on
  the C4 path so the original near return can finish correctly. Do not use
  an `LCALL` from the FD payload bank to RAM:fc858f: its 16-bit encoding
  cannot retain the FC bank. The corrected hook remains untested on hardware.
- Physical PX-W5224A 1.04 test of the flashed standalone C4 baseline image
  SHA-256 `3da82d83587b6b361981ab6a4c904526fc6f958627f99ffac96398db543df6b3`
  on H: confirmed an independent negative-range acquisition: after a GOOD
  D8 LBA 120000 read, C4 READ (one frame, format 8 internally) returned GOOD
  with 2742 bytes in 3.032 s; the same repositioning followed by direct D8
  LBA -20149 format 8 returned GOOD with 2742 bytes in 3.031 s. Both frames
  had zero main-data and C2 regions and nonzero subchannel bytes. This
  validates command completion and stock-style acquisition on this disc,
  not corrected C2, unaddressed continuous capture, or enforced timeout.
  Exact CDBs and reports are under `out/agent-tests/gofw-d8-baseline-live/`.
- In stock PX-W5224A 1.04, the special raw-read controller event at
  RAM:f912cc routes to packed-subchannel processing once RAM:001598 is greater
  than -1000. `ProcessPackedSubchannelRecordBatch` RAM:f933e8..f93421 sets
  RAM:001598=150 and enqueues a `05/24/00` descriptor after its RAM:000f4e
  miss counter exceeds 32. `ValidateRawSectorDescriptorStatus` RAM:fc5364
  rejects a nonzero-status descriptor when RAM:000c2a=3. The stock raw-range
  failure path RAM:fd6f65..fd6fd0 can return zero without setting sense for
  an in-range position, then resets the queue and installs idle coroutine2.
  A caller must check its return and invalidate stream reuse after failure.
  The 2026-09-27 stalled C4 capture has RAM:000f4e=39, RAM:000f2e=5 and
  RAM:001598=150, matching that packed path. Its first failed descriptor was
  not captured, so causation of the initial short transfer remains inferred.
- Stock PX-W5224A 1.04 Timer0 at RAM:fd12af cycles RAM:001231 through ten phases and increments one-byte RAM:001232 each interrupt. Phase 2 at RAM:fd1693 runs every ten Timer0 ticks; its RAM:001212 divider and the subsequent RAM:00120d divider make RAM:0011f4 a nominal 1000-tick, 32-bit incrementing counter (RAM:fd19b5..fd19bb). RAM:fd1e66 resets RAM:0011f4 when controller RAM:0180f8 bit0 becomes debounced active and accumulates it into RAM:00fbed when bit0 becomes debounced inactive. RAM:0011f0 increments on the same slow path only if RAM:000faf=1 (RAM:fd19e2..fd19e8) and has other stock writers. The 32-bit RAM:0011f8 and RAM:0011fc values are interrupt-decremented, caller-initialized countdowns. None of these is established as a free-running millisecond clock for an independent command deadline.
- Stock 1.04 uses Timer0 divider cascades for longer waits: RAM:fd1693 runs every tenth Timer0 tick; RAM:001212 gates its divided service every tenth phase-2 call, decrementing RAM:001205 about every 100 Timer0 ticks. Canonical RAM:fcf345 reloads RAM:001205 to 0x1e (30) for a controller Q-packet capture; RAM:fcf36f returns timeout when its signed value is <=0, giving a nominal three-second window at ~1 ms Timer0 cadence. The same phase-2 worker decrements shared 16-bit RAM:001200 every tenth Timer0 tick; stock operating-code transition paths initialize it to values including 80, 500, and 1000. Such timer-owned countdowns show how stock times longer intervals without a universal monotonic clock. The alternate raw producer at RAM:f92a69 also uses a separate RAM:000f76 timer and local retries, and RAM:f90b91 throttles producer activity when its ring approaches capacity; these paths should not be conflated with a single D8 command deadline.
- On physical H: with SpiralDrive 1.04 running, stock 16-bit RAM:001200 decreased from 30967 to 30647 over 3.219 wall-clock seconds while RAM:0011f4 increased from 2566 to 2569; this independently supports about 10 ms per RAM:001200 decrement and about 1 s per RAM:0011f4 increment in the idle state. Canonical Timer0 phase2 uses one 16-bit read/decrement/write at RAM:fd16fc..fd1702. The sole non-timer writer of 32-bit RAM:0011f8, stock RAM:f8e2b0, is called by RAM:fd0ff2 (`WriteControllerPort80E4Value03AndWait`), which C4's movement routine uses. Thus RAM:0011f8 is specifically unsafe as C4's read-only SEEK clock even though it decrements every odd Timer0 phase. RAM:001200 is also shared and may be reloaded by stock operating-code, calibration, or scaled-radial paths; short live SEEKs alone do not prove exclusive ownership during a long C4 move.

- In PX-W5224A stock 1.04, `EnqueueRawSectorDescriptor` at RAM:feefdb silently returns when RAM:001292 queued count reaches RAM:00129c high-water; callers that require continuous capture must detect this before enqueue. The mode-3 host poll RAM:fd6fe0 waits inside its near call for RAM:000f47 to clear after starting a batch, so an outer inactivity timer cannot run during that wait. The host ISR completion at RAM:fde972 clears RAM:000f47 and advances RAM:001459. These points were checked against canonical Ghidra disassembly and the reconstructed C sources.
- In stock 1.04 `ExecuteControllerDataInTransfer` RAM:fdddad, a timed-out DATA IN sets RAM:000c33 to 4, then stops the host-transfer controller through RAM:01801f=0, RAM:018020=0, RAM:018021=0, RAM:018047=0x14. This is evidence for the controller-port stop sequence, not proof that clearing RAM:000f47 and returning CHECK during a C4 batched transfer works on physical hardware; that C4 adaptation still needs a drive test.
- In stock PX-W5224A 1.04, `ProcessRawSectorBufferEvent` at RAM:f92d0b bypasses its decoded-position matching branches when BITS:0013 (`SpecialRawReadMode`) is set, but still stores RAM:001598 into each descriptor via `BuildRawSectorDescriptorScratch` at RAM:fe0000 and advances RAM:001598 after enqueue. `CountValidatedRawReadDescriptorsForHostTransfer` at RAM:fc65f4 only compares descriptor position with the mapped host range when BITS:000c is set. The C4 raw-range code clears BITS:000c, so RAM:001598 can serve as a descriptor sequence without making host transfer LBA dependent. The C4 event hook must continue bypassing the stock RAM:001598 > -1000 packed-subchannel branch for that sequence to work. These facts follow from the canonical 1.04 Ghidra control flow; hardware behavior of the count-only C4 revision is not yet verified.
- In stock PX-W5224A 1.04, BITS:0046 is set around the D8/D9 and standard READ command bodies, cleared before their return, and tested at RAM:fdcf8b in `FirmwareMainLoop` to decide whether to call RAM:fee51e. It represents command execution, not persistent raw capture; a custom stream hook must not keep it set between SCSI commands. Stock `ProcessControllerReadEvent` at RAM:f912cc enters the raw descriptor path for `SpecialRawReadMode == 0` or signed RAM:001598 < -999; otherwise it enters packed-subchannel processing. Thus a C4 stream with BITS:0013 set and a zero-based RAM:001598 sequence requires its custom state-7 event route to remain active between successful READ commands. This is canonical 1.04 control-flow evidence, not proof of a particular physical failure.
- In the 2026-09-29 user-flashed SpiralDrive 1.04 capture `E:/ISO/dump/spiraltest.subcode` (97,609 file slots; 52,801 written frames starting at synthetic LBA -342), 44 CRC-valid adjacent Sub-Q absolute-address jumps all land on synthetic LBA modulo 16 = 11. The first 16-frame C4 READ begins at -149, also modulo 16 = 11; thus all observed jumps land on its first frame. Their skipped-position sum is 6,530, while 50,015 other adjacent valid-Q pairs increase by one. Redumper's `dump::spiral` reports `shift = Q absolute LBA - synthetic received-frame index`; its logged changes are therefore command-boundary Q discontinuities. Redumper writes main, C2-state, and Sub-Q separately for every frame via `write_entry`, which seeks and flushes each file every call. The capture alone cannot identify whether host throughput, firmware queue pressure/reacquisition, or both caused the discontinuities; no queue occupancy/timing trace was taken during this run.
- The follow-up 2026-09-29 4× SpiralDrive capture in the same output path (51,928 file slots, 7,409 written frames from synthetic LBA -631) has 15 adjacent CRC-valid Sub-Q jumps, each 39..42 physical positions, totaling 600 omitted Q positions. All jumps land on synthetic LBA modulo 16 = 10, the last frame of each affected 16-frame READ; consecutive jumps are 256 or 272 delivered frames apart. Thus lowering selected speed to 4× did not remove command-boundary discontinuities. This observation does not prove that a specific queue, timer, or host I/O path caused them.
- In canonical PX-W5224A 1.04, the coroutine-1 ProcessReadAcquisitionState near-returning function calls ResetDriveControlStateToEight by a complete four-byte ECALL at RAM:f86835..f86838 after a capture failure, then enters calibration. Its near-return epilogue is at RAM:f86890 (DEC SPX,#2; POP DR12; RET). The fixed 80251 language models an ECALL/ERET return address as three stack bytes. The normal raw-read speed initializer RAM:fbf67c..fbf735 is far-returning and limits speed to 4x when BITS:0013 is set; clearing that bit lets C4 use its stock selected-speed path. The stock empty-queue helper RAM:fc683c can install the stock coroutine when available descriptors are exhausted, BITS:0008 is clear, and RAM:000f46 is zero; a C4 range wrapper must guard that condition before invoking the stock poll.
- In the 2026-09-30 SpiralDrive C4T2 4x physical probe, an unrelated SCSI opcode `0x00` TEST UNIT READY was admitted while C4 capture was active (RAM:000c2a=3, RAM:000f06=4) after 114 READ attempts. The custom `GofwStopC4ForOtherCommand` then stopped capture, leaving mode0 for the next C4 READ. That READ rearmed the queue, returned GOOD/full, and omitted seven valid Sub-Q positions across its command boundary. The seven unrelated admissions match seven periodic long READs in the bounded probe. Evidence: `out/agent-tests/gofw-trace-live/spiraltest_buffer-c4-trace-cause.{bin,csv,json}` and `E:/ISO/dump/spiraltest_buffer.spiral-probe.csv`. The issuing process and safety of allowing stock TEST UNIT READY during active capture remain unverified.
- Bus Hound export `E:/ISO/dump/asdf.txt` for the same SpiralDrive 4x probe independently shows seven USB mass-storage CBWs containing six-byte TEST UNIT READY CDBs interleaved with C4 READs, each with a successful CSW. These are host-originated commands, not controller-internal CDBs; the export does not identify the sending process. A repeated TEST UNIT READY row after final C4 SEEK establishes polling outside the READ loop. This corroborates the firmware C4T2 admission count and explains its periodic capture resets.
- In canonical PX-W5224A 1.04, stock TEST UNIT READY RAM:fde75a validates the six-byte CDB and writes only SCSI sense fields from the status table; it does not reset the raw queue or RAM:000c2a capture mode. On the next ordinary addressed D8 READ, RAM:fd6cf2 computes an LBA-derived pipeline target. With mode3 and matching format, RAM:fc5af5 either selects a descriptor cursor inside the buffered span (no pickup movement), waits at the active producer cursor under its target-match condition, or resets the queue, installs coroutine2, and publishes a new pipeline target when that target cannot be served from the queue. The queue reset itself does not move the pickup. In ordinary mode the producer RAM:f92a69 requests acquisition at `ReadPipelineTargetAddress - 10` via RAM:fdccc1; the acquisition positioner RAM:fc858f calls a pickup-move routine only when its distance checks require it. The actual physical branch taken after a TUR has not been measured on a stock D8 run. Evidence: canonical Ghidra decompilation cross-checked with `src/platform/status_mapping.c`, `src/scsi/raw_sector_range.c`, `src/scsi/raw_sector_host_transfer_poll.c`, `src/media/raw_read_position_probe.c`, and `src/media/read_acquisition_distance.c`.
