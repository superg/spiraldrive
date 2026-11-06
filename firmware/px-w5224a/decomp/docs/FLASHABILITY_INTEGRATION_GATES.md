# Integrated gates before any flashability claim

Scope: Plextor PX-W5224A firmware 1.04. The comparison oracle is the immutable
stock image `data/W5224A104/MONT_104.bin`; the runtime monitor is the separate
`data/read_memory_ff0000.debug` capture. Cases must run stock and reconstructed
code from the same declared initial state and compare termination, observable
RAM/controller/SFR changes, hardware event order, and bounded progress. A case
passes only when both executions reach the intended checkpoint or return and
run a nonzero, specified minimum number of states. Exact instruction counts
need not match because the implementations are different.

## Evidence audit (2026-09-25)

A read-only audit compared the existing passing reports with the current case
catalogs and exact built artifacts. The integrated report has 24 unique passes:
all 22 cases in `tests/simulator/cases/integration.json`, plus the two unchanged
cases in `cases.json` and `feasibility.json`. The monitor catalog has 39 cases;
34 pass in the aggregate report and five later additions pass in separate
reports. All 63 result definitions match their current case SHA-256 fingerprints,
and all reports identify the current linked application and map. The simulator
runner, assembled monitor, and packaged image hashes also match the current
files. This verifies evidence identity, not completion of the four gates.

Further S251 runs are limited to a named, bounded question. The staged restart
continuation reached high-bank Mode Page 03 copy and then hit the debugger's
memory-model limit; one direct high-bank observation attempt exceeded its 45 s
wall limit before producing the stock snapshot. Repeating that run without a
new memory model will not establish physical bank behavior. The next priority
is to resolve the programming-state address/bus mapping, then run only targeted
differential cases needed to test a specific reconstruction correction. On
2026-09-25, the fixed DATA-IN helper identified a connected PX-W5224A revision
1.04 and completed all forty-four read-only commands, including two full
512-KiB window captures. The live `RAM:f80050` and
`RAM:f00050` reads both returned the immutable 16-byte stock reset sequence,
the Mode Page 03 vendor read matched MODE SENSE(10), and `RAM:0000f8` returned
`bf00`. Reads at `RAM:f70000`, `RAM:f72a3e`, and `RAM:f78000` matched
`RAM:ff0000`, `RAM:ff2a3e`, and `RAM:ff8000`, including protected-monitor and
fixed-data bytes. The complete `RAM:f00000..f7ffff` and
`RAM:f80000..ffffff` captures are byte-identical with SHA-256
`c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`;
the high view equals stock through `RAM:feffff` plus the immutable runtime
capture at `RAM:ff0000..ffffff`. Exact CDB, status, length, payload, and backup
evidence is retained under
`out/agent-tests/windows-readonly-w5224a-full-backup-20260925/`.
This closes the proposed normal-mode CPU data-read and selector observations.
The live `bf00` is the byte-swapped form of the SST39LF400A manufacturer word
`00bf`; the stock programmer's `aa00/5500` CPU words and doubled byte addresses
then match the manufacturer's x16 command byte and `5555/2aaa` word addresses.
That strongly resolves the intended stock address/lane mapping, while no flash
effect has yet been physically observed.

The user then entered the protected monitor by holding Eject at power-on. A
positive recovery-profile run identified revision `0.00`; all 41 commands
were DATA IN, all 15 checks passed, and both full 512-KiB reads were identical
to each other and to stock main firmware plus the immutable monitor capture
(SHA-256 `c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`).
The report is
`out/agent-tests/windows-recovery-full-backup-current-20260925/report.json`.
This closes live recovery entry and recovery-mode readback, not monitor DATA
OUT or physical erase/program effects.

### Flash-loader trace identity check (2026-09-25)

The retained current-image operation-5 traces at
`out/agent-tests/f2000checksumfix/results.json` and
`out/agent-tests/fbf00checksumfix/results.json` still match the current
`trace_runtime_flash.py` SHA-256 and packaged image SHA-256. For declared
`RAM:0000f8=0x2000`, stock and reconstruction have the same 14 ordered
`(CPU address, 16-bit value, post-store PC)` records through
`RAM:0008c5`; for declared `0xbf00`, they have the same six records through
`RAM:0008f9`. The first five stores in each trace are the
`0xaa00/0x5500/0x8000/0xaa00/0x5500` sequence at
`RAM:f0aaaa`, `RAM:f05554`, and `RAM:f0aaaa`; the sixth is
`0x3000` or `0x5000` at `RAM:f00000`. Thus the tested reconstruction agrees
on CPU stores through the first erase command. The earlier reports label the
SST command lane and `+0x80000` execution alias as assumptions. The later live
`bf00` manufacturer-word observation, full-window read aliases, doubled x16
unlock addresses, and exact command words now provide strong independent
evidence for both. A chip response has not been observed. The stock
configuration-programming caller conditionally changes
P3 bits `BITS:0580/0581` across its monitor call, and the stock WRITE
BUFFER path clears both bits before operation 3 or 5 at runtime
`RAM:ff4000`. Their external wiring is unknown. A normal-command F5
read alias alone would not prove a programming-state effect, but the full-image
plan reuses this exact factory operation-3 path. The `0x2000` replay flags the subsequent bare `0x3000` store as an
error under the SST model, so it cannot be promoted to an effective erase
sequence on that chip. No reconstruction correction follows from these traces.

## Current acceptance status (2026-09-25)

| Gate | Current evidence | Required next proof |
| --- | --- | --- |
| Reset and scheduler | Physical reset reaches the command coroutine and two complete 0/1/2/3 scheduler cycles. The second `ResumeCoroutine3` and third `ResumeCoroutine0` have exact saved-PC and SPX assertions. The coroutine-zero and coroutine-one yield stack defects and an extended SFR bit error were fixed. Under a declared flash-poll and direct-memory model, both images complete 32 configuration words, return through the caller, and yield coroutine one with saved SPX `0x507`. A paired trace shows the relocated programmer overwrites coroutine three’s `RAM:000900` return slot; the next `ResumeCoroutine3` returns to invalid `0x55547e` in both images. Declared `InitializeCoroutine3` RAM effects restore the fourth `ResumeCoroutine0` in both images. | Establish whether physical reset, natural coroutine-three reinitialization, or distinct RAM banking prevents that overlap before accepting a fourth natural `ResumeCoroutine0`; resolve physical flash busy/alias behavior and remaining active-scheduler interrupts. |
| Interrupts | Physical monitor routing and `RETI` pass for bounded sources; Timer0 while coroutine zero is yielded returns through both monitor and declared main handlers with scheduler progress. Main-mode INT0 no-pending and pending-bit branches return through the captured router, including yielded and active coroutine-zero contexts. Four main-mode host-interface pending causes pass through the captured router and `RETI`: transfer-count snapshot (`0x02`), data-completion state (`0x04`), packet-command admission (`0x41`), and host-status completion (`0x30`). Cause `0x30` has matching ordered controller writes. A fifth, cause `0x10`, matches the interrupt through `RestartFirmware` into initialization. A separate non-packet cause `0x01` enters the same captured vector, reads controller status `RAM:018057=0`, selects packet state `0x10` and protocol zero, and returns through `RETI` with matching ordered controller I/O. A staged continuation passes the finite runtime delay, two declared timer overflows, and two routed controller event-`0x40` completions for successive buffer-fill chunks before reaching a later timer wait on both images. A separate versioned event-`0x40` case passes through the captured INT0 router and `RETI`. Host cause-`0x02` passes both while coroutine zero is yielded (nonzero count copied, next `ResumeCoroutine0`) and while it actively reads configuration (zero count preserved, next `ResumeCoroutine1`), with saved contexts and host completion flags intact. In a conditional READ(10) transfer wait, cause `0x04` enters normal raw-read completion, clears the transfer latch, and returns to the scheduler in both images. | Test other host command/status branches, controller pending branches, other active-scheduler host causes, physical event delivery/order, and the restart continuation. |
| Commands | Packet ingress, transfer and completion pass for two-command sequences; READ CAPACITY(10) uses modeled media geometry. One-block READ(10) yields from the command coroutine to a captured scheduler context. A controlled real scheduler `ECALL ResumeCoroutine2` runs the media task to its first yield; both images return to the scheduler and naturally reach the next command resume. Supplying one declared synthetic descriptor at the empty ring lets both images configure the same 2048-byte controller transfer. A declared prefix-acquired state and cause-`0x04` DATA IN event then clear its latch; controlled scheduler resume reaches the READ(10) engine return and next command idle yield with matching state. A declared prebuffered `0xff` packet then reaches unsupported-command handling, writes `70/05/24/00` sense, and returns to idle. | Verify actual sector bytes and physical ingress of the next packet; establish unscripted controller timing and physical media behavior. |
| Update and recovery | Reset-to-rescue selection is accepted for this gate: the captured monitor checks forced-recovery inputs before main-image validation, and paired offline cases enter `RAM:ff0300` with an intact, corrupt, or checksum-valid but nonfunctional main image. The user reports that holding Eject during power-on selects this protected `RAM:ff0000` rescue firmware, which is never flashed. This is the recovery strategy premise for the drive, not a measured bus-mapping result. Monitor accepts intact images, rejects a bad checksum, and honors forced recovery. The canonical validator also checks the programmer's `RAM:0000fd=0x99` success marker, its jump to bootstrap `RAM:ff0200`, and the bootstrap call to the forced-recovery predicate (`out/agent-tests/reset_handoff_validation.log`). The captured monitor has statically verified WRITE BUFFER `0x3b` mode-4 DATA OUT staging and mode-5 validation/programmer routes to runtime `RAM:ff4000` operations 3/5, but no live monitor reflashing has been observed. After either modeled first 4-KiB sector erase or 64-KiB block erase, stock and reconstruction enter the captured recovery loop with ACC=0. Both operation-5 branches have matching bounded CPU stores on the current image, with conditional first-erase calculations. On the preceding image and corrected simulator harness, a valid mode-4 packet reaches command idle under declared DATA OUT payload/status, with matching controller event order and modeled buffer bytes. On a preceding image, mode-5 zero-length finalization completes revision and malformed-header rejection through written `70/04/40/F5` sense and command idle. On the current image, auxiliary mode-5 checksum rejection reaches written `70/04/40/F6` sense and command idle. A separate equal-byte auxiliary candidate passes checksum validation, copies 96 captured configuration bytes, and reaches the pre-flash operation-5 handoff under explicit debugger bank mapping; the native checksum-call ABI was corrected. The main-image passing-header path reaches checksum entry in both images, but the S251 checksum continuation stops at its bank-`0x02` read before a decision. The C checksum implementation passes a focused host test over both complete stock-defined staged ranges, including stored-word checks and mutated-byte rejection. | Establish staged-buffer behavior and main-image checksum rejection; resolve effective programming ranges, flash bus mapping, and activation. Reset-to-rescue selection is already accepted offline, and live rescue reflashing is not a gate prerequisite. |

Update to the table: recovery entry is now proved live at revision `0.00`, and
the SST address/command lane is strongly resolved by the live manufacturer word
plus exact x16 command addresses. The normal full-image plan selects internal
operation 3, not the auxiliary internal operation 5 discussed in the modeled
first-erase warning. The remaining update observation is an explicitly
authorized DATA-OUT/program/activation run followed by candidate-range readback.

The preceding reconstructed image is SHA-256
`2c6e774a886c791cce180dbf57174c6fc68b136c9582d045a56026e7f2b3e1ef`
with stored checksum `0x9a8e`; C251, L251, the absolute-DATA audit, OH251,
packaging, and strict format validation pass. Canonical 1.04
`RAM:fd661f`/`RAM:fd6629` established that
`ProgramAndVerifyDriveConfiguration` must read the full 32-bit ring count at
`RAM:00156a`. The corrected target listing loads both words, and the focused
host `configuration_queue_count_tests` passes with `0x00000200`.

The preceding image passes all 17 then-versioned integrated cases at
`out/agent-tests/integration-gates/config-queue-fix-integration-seventeen-run/results.json`
and all 25 then-versioned monitor cases at
`out/agent-tests/integration-gates/config-queue-fix-monitor-twentyfive-run/results.json`.
Four newly added INT0 monitor cases pass separately at
`out/agent-tests/integration-gates/controller-event-main-no-pending-versioned-run/results.json`,
`out/agent-tests/integration-gates/controller-event-main-bit1-pending-versioned-run/results.json`,
`out/agent-tests/integration-gates/boot-main-int0-bit1-while-coroutine0-yielded-versioned-run/results.json`,
and `out/agent-tests/integration-gates/boot-coroutine0-main-int0-bit1-active-versioned-run/results.json`.
The newly added READ(10) first-yield case passes separately at
`out/agent-tests/r10yieldv2/results.json`. A valid mode-4 WRITE BUFFER
first-yield case passes separately at `out/agent-tests/m4yieldv/results.json`;
the integrated suite now has 19 cases. A controlled continuation from that
case reaches the DATA OUT helper with transfer status 8, then a declared
four-byte payload and completion status reaches its return in both images at
`out/agent-tests/m4resume-complete-v4/results.json`.
The versioned monitor suite has 29 cases: 25 passed together on the preceding image,
and four new cases passed individually. All eight differential case reports identify the same
linked application and map. These tests cover
bounded scenarios; they do not complete any of the four gates.

The preceding image, after correcting the `CopyFromControllerBufferWindow` argument order, is SHA-256 `2b965412b1e539d0b2cf0da1b27e78b901e4b1e87c950344b10014eed39e1979` with checksum `0x99bc`. C251/L251, absolute-DATA audit, OH251, packaging, strict format validation, and the focused host controller-copy test pass. The versioned mode-4 first yield and controlled command completion pass on this image under the corrected harness at `out/agent-tests/integration-hdatafix-correct-xvfb/results.json` and `out/agent-tests/m4-final-complete-xvfb/results.json`. Mode-5 first yield and first revision-copy read pass at `out/agent-tests/integration-hdatafix-correct-xvfb/results.json` and `out/agent-tests/m5-final-copy-entry-xvfb/results.json`. A controlled continuation with an explicit S251-only source-bank relocation reaches `SetSenseData(4, 0x40f5)` in both images after copying staged revision `00` (`out/agent-tests/m5-revision-rejection-final-xvfb/results.json`); `out/agent-tests/m5-rejection-completion-final-xvfb/results.json` continues both images through the `70/04/40/F5` sense write and command idle. A separate revision-passing `09` scenario with declared media state 1 reaches the main-image header validator, rejects a staged zero identity against installed `P`, writes the same sense, and returns to idle (`out/agent-tests/m5-header-rejection-final-xvfb/results.json`). All 20 versioned integrated cases and all 29 monitor cases now pass together on this image and corrected harness at `out/agent-tests/integration-hdatafix-correct-xvfb/results.json` and `out/agent-tests/monitor-hdatafix-correct-xvfb/results.json`. Both reports record linked application SHA-256 `2eb00ff6152b40a843422d289542eefb42fa263d717f7a411685e6b1441da0bd`, map SHA-256 `f22b8b84fba69f5235d7fecb21aab327f92de73f431e75548f417cb382ae357c`, and assembled monitor SHA-256 `51570409be33d1d9b01dfbd960aec2fb31e136a04fab0e8ceba84148f2a4d486`.

The preceding image after removing the extra coroutine-one yield call frame is
SHA-256 `061ed2a1722b64030f362d537e6d8c1ffdc31c6619f0c961826db2f2c1ca4e76`
with checksum `0xa12f`; application SHA-256
`7a848a669a864d8ebea33bb14149e3bdbf2a4f0c5a90c57a99d73948c74ffbfa`
and map SHA-256
`c8691cd007fc0407fd1ec53c46c4acf9b30bd03c8985dee70297f7ca3d91ad71`.
C251/L251, absolute-DATA audit, OH251, packaging, format validation, and the
focused startup-status host test pass. All 33 versioned monitor cases and all
23 integrated cases pass on this image at `out/agent-tests/m33fix/results.json`
and `out/agent-tests/i21fix/results.json`. The monitor and integrated runs
were resumed from retained partial results after their per-command time limits;
no completed case was rerun by `--resume`. The longer READ(10), mode-4, and mode-5 continuations above remain evidence from the older linked image; only their first checkpoints were rerun on this image.

The current reconstructed image after correcting the staged-checksum call ABI is SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c` with stored checksum `0xa15d`. Its 32-KiB recovery hole and 24-KiB trailing padding are erased, and its 8,190 fixed-data bytes at `RAM:ff8000..ff9ffd` match stock exactly. The final image verifier now enforces stock fixed-data identity using the hash-pinned reference; the current image passes (`out/agent-tests/fixed_data_verification.json`), and a checksum-valid one-byte mutation fails (`tests/test_firmware_image.py`). This is packaging integrity, not programmer or chip behavior. Linked application SHA-256 `76069bba74cc771b329cbfa89dc9388759f7e00039c4c6faa2a3912c5653416e` and map SHA-256 `7902fad70049cf17f65802a746c3ff8b7da6800246a40d59488a9d7bc8c7983a`. C251/L251, absolute-DATA audit, OH251, packaging, strict format validation, and focused host checksum/write-buffer tests pass. All 24 integrated and 34 monitor cases pass on this image at `out/agent-tests/i24checksumfix/results.json` and `out/agent-tests/m34blockfull/results.json`. The monitor aggregate was resumed from retained partial results after two per-command time limits; its final report contains 34 unique passes with the same image, map, and monitor identities. The versioned monitor catalog now defines 35 cases; the new cause-`0x10` interrupt-to-initialization-entry case passes separately on those same identities at `out/agent-tests/hostrestart_init_versioned/results.json`. A paired auxiliary mode-5 trace reaches the checksum function with the stock register contract, then continues through checksum rejection, written `70/04/40/F6` sense, and command idle (`out/agent-tests/m5auxentry_final/results.json`, `out/agent-tests/m5auxreject_final/results.json`, `out/agent-tests/m5auxidle_final/results.json`). Its XDATA staging range overlaps S251 observation scratch, so this is conditional path evidence and does not validate a physical staged image or accepted checksum. A second case-local run relocates scratch outside the auxiliary checksum region, normalizes one implementation-specific staged byte, and applies the same debugger-only bank-`0x10` to bank-`0x01` substitution to three checksum reads on each image. Both sides accept the identical candidate (`SHA-256 2662469194e950652d4a755dcfc4594ce633ff629fc063012b9cd0f81c0be03a`, additive checksum `0x0001`) and enter `StagePrimaryDriveConfigurationForAuxiliaryUpdate` with `ACC=1` (`out/agent-tests/m5auxcleanscratch_final/results.json`, `out/agent-tests/m5auxcleanaccept_final/results.json`). Seeding the 96-byte primary configuration from the immutable runtime capture at `RAM:ffc000` then changes 87 staged bytes at `RAM:10c000`; all 96 staged bytes equal the declared capture slice in both images. Both reach `RAM:ff4000` with `DR0=0x00050000`, DR4=0, and DR8=`RAM:ff4000` before executing a flash-loader instruction (`out/agent-tests/m5auxcleanhandoff_final2/results.json`). This proves conditional validator, configuration-copy, and handoff behavior only; actual controller staging, flash operations, activation, and physical recovery remain unverified.

The simulator HDATA relocation pass previously scanned entire compiler listings per reachable function, changing the copy helper’s bank-add immediate from `0x0010` to `0x0001` in the debugger while the image and both compiled binaries retained `0x0010`. The pass now scans only the verified function body; targeted unit tests and the current mode-4/mode-5 traces pass under this correction. Earlier differential reports involving HDATA patching require rerun before being used as current-harness evidence.

**Decision:** the reconstructed image is structurally valid and has substantial
bounded behavioral evidence, but none of these four broad integration gates is
fully accepted. The former byte-lane/address assumption is now strongly resolved
by live and manufacturer evidence. The conditional operation-5 erase warning
applies to the auxiliary path, not the stock full-image operation-3 plan. The
remaining decisive observation for that plan is an explicitly authorized,
logged physical update followed by readback; these tests did not flash a drive.

### Simulator boundary and next evidence

S251 has no LC898095 peripheral model. The READ(10) traces can compare CPU
register writes, a declared descriptor, interrupt handling, and command state,
but S251 cannot produce a real sector from the media path or prove that the
external-buffer controller transfers its 2,048 bytes to the host. Adding a
synthetic sector to the debugger would test only the declared data and CPU
control path. Actual sector contents and physical packet ingress therefore
remain hardware integration checks; they are not pending longer emulator runs.

The offline priority is to fix any demonstrated reconstruction mismatch and
retain a passing bounded regression, then stop that probe. The hardware
priority is read-only characterization of the LC898095 buffer and flash address
mapping and drive identity before specifying a controlled write experiment.
The reset/coroutine-three overlap remains a scheduler acceptance blocker.
The modeled first erase can blank the main-image reset veneer; that effect
needs correct flash mapping for a programming claim, while the separate
protected rescue bootstrap remains available according to the user-provided
hardware behavior.
A firmware-1.04 vendor F5 selector-zero DATA IN command offers a bounded,
read-only CPU data-address comparison at `RAM:f00050` and `RAM:f80050`; the
exact CDBs, reference bytes, and limits are in `docs/HARDWARE_PLATFORM.md`.
That normal-mode comparison and full read-only backup now pass on the connected
identified drive, as recorded in the evidence audit above. Even those matching
reads do not prove flash write routing or the SST command byte lane. The
captured monitor's
F5 dispatch and memory-read worker have also been checked directly; a separate
read-only self-read checkpoint is specified for a drive already in recovery mode.
That checkpoint would test monitor DATA IN, not reflashing or post-erase survival.

## 1. Reset, startup, and scheduler

Start at the physical main reset entry `RAM:f80050`, with the runtime monitor
loaded when testing its bootstrap. Exercise both the normal and forced recovery
boot decisions. For normal boot, trace the reset veneer, firmware initialization,
interrupt selector handoff, coroutine creation, and at least one full scheduler
turn that yields and resumes each active coroutine. Inject timer or controller
events needed to advance waits. Compare coroutine contexts, SPX, interrupt-enable
bits, the firmware-active selector, controller initialization writes, and a
bounded liveness checkpoint after repeated turns. A stop at first coroutine
configuration proves only the initial part of this gate.

Current evidence: `tests/simulator/cases/integration.json` passes reset to
`ConfigureInitialCoroutines` (72 stock / 63 reconstructed states), reset through
four coroutine setups to the first `ResumeCoroutine0` call (443 / 476), and
coroutine-0 startup, CRC initialization, yield, and the next `ResumeCoroutine1`
call (391,572 / 57,512). A separate monitor-loaded reset case stops at
`ScsiCommandProcessorCoroutine0` itself after the first scheduler resume
(471 stock / 502 reconstructed states). Both images have `SP=0`, saved
scheduler SPX `RAM:00115a..00115d = 00 00 00 0a`, and initial coroutine-zero
saved SPX `RAM:00115e..001161 = 00 00 03 07`. The versioned case and passing
result are `tests/simulator/monitor-cases/integration.json` and
`out/agent-tests/integration-gates/reset-to-scsi-coroutine-entry-versioned-run/results.json`.
This confirms the first scheduler handoff into the real command coroutine,
not packet admission or a full scheduler turn. The latter cases use a declared timed countdown
release, controller-ready initial state, and semantic normalization of
relocated coroutine entry pointers; their RAM deltas match. Coroutine 1's startup path now reaches the third `ResumeCoroutine2` call in
both images under a declared offline response model. The model supplies
controller-ready `RAM:018015=0x20`, SFR:D7 completion zero, one zero
`RAM:0180ac` response, and timer-owned countdown releases at
`RAM:001221`, `RAM:001208`, and `RAM:001214`. Stock's 24-bit `@DR`
controller reads require a plain `rwatch(0x018015)` in S251; the linked
reconstruction uses the `X:0x8015` watch. The run reaches the checkpoint
in 464,275 stock / 136,165 reconstructed states and records the same 164
ordered modeled events, including 108 ready reads and 48 D7 reads. Retained
result: `out/agent-tests/integration-gates/reset-third-resume-dual-watch-timers-run/results.json`.
This historical probe is a **mismatch**, not an accepted gate case. Its
raw comparison included relocated live return PCs, bytes above the saved
coroutine-one SPX `RAM:000508`, and controller `RAM:018013`/`RAM:018046`
views. The later fourth-resume work below proves the return-site
relationships and fixes the source byte order responsible for the differing
`RAM:018013` write. The `RAM:018046` final snapshot remains an S251 view
limitation despite matching ordered writes; the older result is retained
only as a diagnostic from before the correction.
A fourth-resume probe exposed and verified two source defects.
`ResetRawReadQueueAndTransferState` (`RAM:f927bd`) must read the full 32-bit
slot count at `RAM:00156a`; the corrected C matches stock ring limits
`01 ff 01 fa` for count `0x00000200`. Canonical
`ConfigureMainDataRingGeometry` (`RAM:fdbdff`, instructions
`RAM:fdbe42..fdbe64`) writes ring size low/middle/high at controller
`RAM:018013..018015`; the previous C wrote high/middle/low. The corrected
mode-one and mode-two C passes focused host tests and direct differential
cases.

Two versioned monitor cases passed on an earlier image. The first reaches
`ResumeCoroutine3` after reset (468,297 stock / 294,939 reconstructed states). The second stops first at stock
`ResumeCoroutine0` `RAM:fee4fd` and its linked counterpart, then reaches their
second occurrence after coroutine 0, 1, 2, and 3 each run. That full-cycle
case takes 468,465 stock / 295,091 reconstructed states. Both cases compare
166 ordered modeled events, including writes `RAM:018046=0x02` then
`RAM:018013=0x00`; matching RAM changes; and exact live saved code pointers.
The saved coroutine-one PC is the instruction immediately after
`YieldCoroutine1` in each image (`RAM:f84bf9` / `RAM:fb2e5c`), and coroutine-two
likewise resumes immediately after `YieldCoroutine2` (`RAM:f84a12` /
`RAM:fce693`). At the full-cycle checkpoint, coroutine-three has yielded
and its saved PC is the immediate successor of `YieldCoroutine3`
(`RAM:f84b09` / `RAM:fce6a4`). Saved scheduler and coroutine SPX values are
asserted in both images. The cases use exact physical pointer expectations;
a wrong pointer cannot normalize into a pass. They exclude only bytes above
saved coroutine stack pointers and the `RAM:018046` final XDATA snapshot,
whose value differs under S251 despite the identical ordered write watches.
Versioned definitions are in `tests/simulator/monitor-cases/integration.json`;
passing results are retained at
`out/agent-tests/integration-gates/reset-fourth-resume-exact-context-run/results.json`
and
`out/agent-tests/integration-gates/reset-second-loop-resume0-corrected-run/results.json`.
The final controller snapshot remains a simulator-view limitation, not an
independent physical bus measurement.

The former second-occurrence `ResumeCoroutine3` mismatch identified a real
source-stack defect: coroutine zero called an idle helper that called
`YieldCoroutine0`, retaining a six-byte helper frame. Its direct yield now
saves `SPX=RAM:000307` in both images and the correct immediate successor PC.
The versioned second-`ResumeCoroutine3` case and third-`ResumeCoroutine0`
case pass, proving two complete scheduler cycles with 166 matching modeled
events. At the fourth `ResumeCoroutine3`, a
new SFR observation exposed a separate bug: C251's legacy `sbit=0xaa`
accessed `SFR:A8.2`, whereas stock `RAM:fee815` sets extended `SFR:A9.2`.
The corrected A251 instruction emits stock bytes `A9 D2 A9`. The four
deep scheduler cases passed on the preceding image with refreshed exact linked
PCs, matching SPX, and `SFR:A8=0xc2` / `SFR:A9=0x04` assertions at
the fourth resume. The final two-SFR assertion passes separately at
`out/agent-tests/integration-gates/monitor-fourth-sfr-a9-assertion-run/results.json`.
The earlier 19 versioned monitor cases also pass on this image; results are
`out/agent-tests/integration-gates/monitor-four-scheduler-extended-bit-run/results.json`
and `out/agent-tests/integration-gates/monitor-twentythree-extended-bit-run/results.json`
(the latter retains the four pre-refresh pointer-only mismatches).
Monitor and declared main-mode Timer0 requests while coroutine zero is yielded now pass; other active-scheduler sources remain open.

A three-cycle extension was attempted with an exact fourth
`ResumeCoroutine0` checkpoint and intermediate PC/SP assertions. Both stock
and reconstruction instead stop at `RAM:000818` after the third checkpoint.
A full `0x28e`-byte code-space snapshot at `RAM:000800` matches the captured
configuration flash programmer source `RAM:ff3c8a..ff3f17` byte for byte,
not the flash-identification routine. Controlled stock resumes stop
successively at `RAM:00081f`, `RAM:000826`, `RAM:00082d`, `RAM:000834`,
`RAM:000848`, then repeat at `RAM:000872`. Canonical 1.04 runtime
`configuration_flash_relocated::00086f` reads the programmed flash address,
and `configuration_flash_relocated::000872` tests bit 7 before looping.
The retained stock register capture at the repeated poll has `DR28=RAM:f7a000`;
`RAM:00086d` copies that address to `DR16`, so the read at `RAM:00086f`
addresses the programming aperture at `RAM:f7a000`. A preceding store at
`RAM:000845` sends `0x3000` to that address on the observed path. With the
previously inferred `+0x80000` execution alias, the corresponding image
sector would be `RAM:ffa000..ffafff`, which is all `0xff` in both downloadable
images. The alias and flash response remain unmeasured; an `0xff` poll reply
is therefore a conditional test input, not an observed hardware result.
This is a shared S251 flash-response modeling boundary, not a
reconstruction-only mismatch or proof that a physical flash write occurred.
A reproducible paired continuation at
`out/agent-tests/integration-gates/config-flash-poll-paired-recorded-run/results.json`
starts from the passing current-image versioned scheduler case. Only after
both traces reach `RAM:000872`, it maps `RAM:f7a000..f7afff` as simulator
read/write memory and declares `0xff` at `RAM:f7a000`. Both then reach
`RAM:00090e`, with matching recorded stops from `RAM:00081f` onward,
`0xff` readback before and after the resume, and `ACC=0xff` there. Both
654-byte relocated bodies match each other and the immutable 1.04 monitor
capture (`SHA-256 4c917db0e694b97f0f73ea9f1fb5c00f3f12bf7a9989a42092b0991460aa346e`).
The stock trace captures the earlier `RAM:000818` stop; the reconstructed
trace records zero for that one debugger PC sample, although a separate
manual reconstructed trace captured `RAM:000818`. The paired tool explicitly
reports that capture gap. This is conditional S251 continuation through the
erase poll, not observed flash behavior, completed programming, or the fourth
scheduler checkpoint. A second reproducible paired trace at
`out/agent-tests/integration-gates/config-flash-first-batch-corrected-run/results.json`
continues under a declared direct-memory map of `RAM:f70000..f7ffff`.
With the canonical `B != 0` branch (`DR0=RAM:001680`, count 32), both images
complete 31 word iterations: each reaches the next unlock at `RAM:00090e`,
advances target and source by two bytes, and decrements the remaining count.
The 62 written bytes at programming `RAM:f7a060..f7a09d` match the source
`RAM:001680..0016bd` byte for byte in each image and between images.
The last recorded stop is `RAM:00091c`, preparing the 32nd word at
`RAM:f7a09e` with count one. The debugger occasionally records zero for an
otherwise matched PC sample; the report identifies those indices.
A paired instruction-step trace at
`out/agent-tests/integration-gates/config-flash-final-word-return-run/results.json`
resolves the former final-word uncertainty. Both images write and verify the
32nd word, decrement the count to zero, and follow the same 53 relocated
instruction PCs through `RAM:000a8d ERET`. All 64 modeled target bytes at
programming `RAM:f7a060..f7a09f` equal the source
`RAM:001680..0016bf` in each image. `ACC=1` is retained across the return.
Stock resumes at `RAM:fd649f`; reconstruction resumes through the
addressless adapter at `RAM:f800d9` and then its C caller at `RAM:f83f59`.
The earlier one-shot `G` result at `0x55547e` was therefore not a final-word
failure. A current-image return trace (`tests/simulator/trace_configuration_flash_caller_return.py`,
`out/agent-tests/ccrfix/results.json`) proves both enclosing configuration
callers return with `ACC=1`, matching observed RAM state, and valid immediate
parent-call successors. Before the source correction, stock entered the later
status yield with `SP=3` and saved coroutine-one SPX `0x507`, while C251 kept
an extra helper frame (`SP=6`, SPX `0x50a`). The source now places all
nonreturning status loops directly in `RunStartupStatusCoroutine1`; its O5/SIZE
listing calls `YieldCoroutine1` directly. The new paired trace
(`tests/simulator/trace_configuration_flash_status_yield.py`,
`out/agent-tests/csyfix/results.json`) reaches the same status-three yield and
scheduler `ResumeCoroutine2` call in both images with `SP=3`, saved
coroutine-one SPX `0x507`, and scheduler SPX 10.

The next natural scheduler call is `ResumeCoroutine3`. In the declared S251
memory model, the relocated configuration programmer at `RAM:000800..000a8d`
overwrites coroutine three's saved return slot at `RAM:000900` while its saved
SPX remains `0x907`. The new paired
`tests/simulator/trace_configuration_flash_stack_overlap.py` trace records
`RAM:000900..000907 = b4 55 54 7e a4 00 f0 7e` and the resulting
`0x55547e` return in both images (`out/agent-tests/csofix/results.json`).
The earlier PC-zero diagnostic is thus explained by a shared modeled stack
and programmer-code overlap, not by the final configuration word. It does not
establish that a physical drive follows this invalid return: a reset,
coroutine-three reinitialization, or different physical RAM banking may
intervene. A controlled paired continuation applies the exact four saved return bytes and
SPX `0x907` written by firmware 1.04 `InitializeCoroutine3` at the next
scheduler `ResumeCoroutine3` call. Both images then reach the fourth
`ResumeCoroutine0` with `SP=10`, saved coroutine-one SPX `0x507`, saved
coroutine-three SPX `0x907`, and status `RAM:001009=3`. The source and retained
result are `tests/simulator/trace_configuration_flash_reinitialized_coroutine3.py`
and `out/agent-tests/c3reinit_final/results.json`. This injected RAM state
isolates the immediate simulator liveness effect of the overwritten return;
the current path does not naturally call `InitializeCoroutine3` before that
scheduler resume. The fourth natural `ResumeCoroutine0`, physical flash
busy/toggle behavior, and flash address alias remain unverified.

A new current-image, versioned non-packet host-command case is
`host-interface-main-vector-nonpacket-command-cause01-reti-via-timer-source`
in `tests/simulator/monitor-cases/integration.json`. Stock and reconstruction
enter the captured host vector with declared cause `0x01`, read controller
`RAM:018057=0`, clear host-command flag bit five, set packet state `0x10`
and protocol zero, match four ordered acknowledgement writes, and execute
`RETI` to the interrupted reset PC with `SP=7` (414 / 417 S251 states).
Its case SHA-256 matches the passing report at
`out/agent-tests/hostcause01_probe_xdata/results.json` on the current
application/map/monitor hashes. The first probe's missing reconstructed read
was an observation error: C251 emits `MOVX A,@DPTR` at linked function offset
`0x244`, so the read must be watched as XDATA `X:8057`, not as the simulator's
24-bit RAM view. The corrected case records the same read on both sides.
The monitor catalog now defines 37 cases: the previous 34-case aggregate and
three separately passing additions. This checks one declared controller
response and the routed interrupt; electrical delivery and other command
states remain open.

## 2. Interrupt delivery and return

Load the `RAM:ff0000..ffffff` monitor capture and select monitor or main handlers
through `RAM:0000fc`. Inject controller, host, packet, neutral SFR-E1, and timer
sources at defined execution states. For each, prove the physical monitor router
selects the right handler, the handler acknowledges the source and updates the
right shared state, and authentic `RETI` restores the interrupted context. Test
an interrupt while a coroutine is waiting and while one is active; verify
subsequent scheduler and command progress. Compare ordered hardware accesses
and contexts after return. Directly starting an ISR and stopping before `RETI`
checks only its body and return opcode, not interrupt entry or restoration.

Current evidence: four optional interrupt cases pass. Timer0 runs from a
state-30 SFR request through the captured router and authentic `RETI` to the
interrupted reset PC in both selector modes. With `RAM:0000fc=1`, the main
handler increments its timer state (507 stock / 541 reconstructed states).
With selector zero, the monitor handler at `RAM:ff03a6` increments
`RAM:0011ce` (332 / 333 states). The captured `INT0` vector at `RAM:ff0003`
and `INT1` vector at `RAM:ff0013` also deliver controller-event and
controller-packet sources through their monitor-mode handlers, execute `RETI`,
and restore `SP=7` at the interrupted reset PC (329 / 330 and 228 / 229
states). The controller-event case observes `RAM:0011cd` increment and the
controller acknowledgement registers; the packet case establishes routing
and context return but no packet-processing effect. The monitor is linked
from the immutable 64-KiB capture as a separate ABS; S251's generic Timer0
vector uses a declared four-byte alias to the physical monitor router.
A versioned case on the preceding image starts at physical reset, injects a single pending
Timer0 flag at state 1,000 while `ReadDriveConfigurationBytes` is executing
inside coroutine-zero startup (stock `RAM:fd66d9`, linked `RAM:f83c0b` in a
separate state-1,000 probe), then reaches the next `ResumeCoroutine1` call.
`SFR:88` starts with TR0 clear and is set to `0x20` once, so S251 does not
free-run the timer. Both images show exactly one main-handler phase/tick
(`RAM:001231=1`, `RAM:001232=1`), `RAM:0000fc=1`, restored `SP=10`,
`SFR:A8=0x82`, `SFR:88=0`, `SFR:A9=0`, and matching RAM and timed-event
transcripts (395,761 stock / 215,924 reconstructed states). These values
are required by `expected_observations`, so a matched failure to deliver
the interrupt cannot pass. The versioned case is in
`tests/simulator/monitor-cases/integration.json`; its exact passing result is
`out/agent-tests/integration-gates/boot-coroutine0-single-timer0-expected-run/results.json`.
This proves one interrupt return and subsequent scheduler progress under
the declared S251 timer model. A second versioned case injects `SFR:88=0x20`
at the second `ResumeCoroutine0` breakpoint, when coroutine zero has yielded
with saved `SPX=RAM:000307` and the exact saved return PC. The natural
selector is `RAM:0000fc=0`, so the captured monitor Timer0 handler runs.
Both stock and reconstruction reach the third `ResumeCoroutine0` checkpoint
(469,238 / 295,995 states), have one monitor tick at `RAM:0011ce`, cleared
`SFR:88`, `SP=10`, the same saved coroutine-zero context, and 166 matching
modeled events. The passing versioned result is
`out/agent-tests/integration-gates/monitor-waiting-timer0-versioned-run/results.json`.
This proves monitor-mode delivery and return while coroutine zero is yielded.
A paired versioned case explicitly writes `RAM:0000fc=1` at the same paused
checkpoint before raising Timer0. It reaches the third `ResumeCoroutine0`
(469,413 / 296,205 states) with one main phase/tick at
`RAM:001231..001232`, no monitor tick, cleared `SFR:88`, `SP=10`, matching
saved coroutine-zero PC/SPX, and 166 matching modeled events. Its result is
`out/agent-tests/integration-gates/main-timer0-while-coroutine0-versioned-run/results.json`.
This establishes main-handler behavior under a declared selector handoff;
the ordinary offline reset path still has selector zero at that checkpoint.
Other sources during active scheduling remain open.
A fifth optional interrupt case routes a declared SFR-E1 bit-7 request
through the captured `RAM:ff3fc9` router and main handler, clears `SFR:E1`
bit 7, executes authentic `RETI`, and restores `SP=7` at the interrupted
reset PC (259 stock / 320 reconstructed states). S251 uses an explicit
Timer0-vector alias to inject this source, so actual E1 interrupt delivery
remains untested. Two more optional cases enter the captured host-interface
vector `RAM:ff000b` through S251's generic Timer0 request. In monitor mode,
the handler writes controller `RAM:018059=0x17`, `RAM:01805b=0x37`, then
`RAM:018059=0x97` and executes `RETI` (376 / 377 states). In main mode with
a zero declared cause, the main handler records `RAM:00faeb=1`, writes
controller `RAM:018059=0x17`, `RAM:018002=0`, `RAM:01805b=0x37`, then
`RAM:018059=0x97`, and executes `RETI` (370 / 353 states). Both restore
`SP=7` at the interrupted reset PC with matching ordered transcripts.
These verify the vector/router and bounded handlers. Two additional versioned main-mode host cases now declare controller cause `0x02` or `0x04` at `RAM:01805a`, using side-specific S251 read watches. Cause `0x02` copies four source count bytes `12 34 56 78` from `RAM:001455..58` to `RAM:001459..5c`, sets `RAM:001596=1`, clears `RAM:001462`, and sets host completion flags `RAM:000c83=0x10` (388 stock / 379 reconstructed states). Cause `0x04` changes active transfer state `RAM:000c33` from 8 to `0x0d` (416 / 440 states). Both enter through captured `RAM:ff000b` and router `RAM:ff3f9b`, execute authentic `RETI` to the interrupted reset PC with `SP=7`, and match the ordered controller read and four acknowledgement writes at `RAM:018059`, `RAM:018002`, `RAM:01805b`, and `RAM:018059`. Results: `out/agent-tests/host-interface-transfer-cause2-versioned-v2-xvfb/results.json` and `out/agent-tests/host-interface-data-completion-cause4-versioned-xvfb/results.json`. These two cases were individually verified on the preceding image. Two further versioned cases use the same physical monitor vector and preceding image. Cause `0x41` moves packet state `RAM:000c32` from 1 to `0x11`, sets protocol `RAM:000c78=1`, clears host command flag bit five, and returns through `RETI` (401 / 405 states; `out/agent-tests/host-interface-command-cause41-versioned-xvfb/results.json`). Cause `0x30` sets completion flags `RAM:000c83=0x18`, operation flags `RAM:001588=0x21`, copies status/error `0x12/0x34` to `RAM:000e46`/`RAM:000e47`, and matches all 15 ordered controller writes, including recovery-buffer setup, before `RETI` (558 / 621 states; `out/agent-tests/host-interface-status-cause30-versioned-xvfb/results.json`). Those four branches are now included in the latest passing 33-case monitor aggregate. S251 raises its generic Timer0 source to enter the captured host vector, so actual electrical host IRQ delivery and other host command/status branches remain untested. A fifth versioned host case on the current image declares cause `0x10` with command flags `RAM:000c86=0` and queue activity `RAM:000f46=1`. Stock and reconstruction enter physical `RAM:ff000b`, record cause `RAM:00131b=0x10`, write controller `RAM:018059=0x17`, `RAM:018002=0`, and `RAM:01805b=0x37` in order, then set `RAM:0000fd=0x98` and `RAM:0000fe=0xff`. The interrupt wrapper clears the restart-request byte `RAM:0000fa`; both images execute `RestartFirmware`, clear `SFR:A8` and `SFR:A9`, and reach `InitializeAndEnterFirmwareMain` at stock `RAM:fee7ed` / linked symbol (340 / 397 states). The result is `out/agent-tests/hostrestart_init_versioned/results.json`. The versioned checkpoint proves the bounded interrupt-to-initialization handoff under a declared S251 source. Read-only canonical runtime disassembly shows `RAM:ff2004` is the body of a finite `WR6 < 0x893` delay, explaining the earlier 1,500-state stop. The repeatable staged trace (`tests/simulator/trace_host_restart_initialization.py`; `out/agent-tests/hostrestart_initialization_trace/host-interface-main-vector-status-cause10-initialization-entry-via-timer-source/results.json`) lets both images complete that delay, supplies two declared `SFR:91` overflow events, and reaches the first controller-buffer fill at `RAM:1b0000` with `0x8000` bytes. A separately versioned physical-INT0 event-`0x40` case passes with mask `0xbe`, status `0xff`, `RAM:14da=1`, and authentic `RETI` (423 / 452 states; `out/agent-tests/controller_event40_versioned/results.json`). Applying the same declared event source at the first wait advances both restart traces to a second `0x8000`-byte fill at `RAM:1b8000`; applying it again advances both to a later `DelayTimerUnits` poll at 780,015 / 780,014 states, with `RAM:14da=1` and `SFR:A8=0`. The two events are modeled S251 inputs. An exploratory 5,000-state timer-overflow model then reaches the third mode-page defaults copy: stock `RAM:fc2a73` calls `CopyBytes8` with DR0=`RAM:2bc6ef`, DR4=`RAM:ff8a5a`, ACC=8; the reconstructed call at `RAM:fd89a0` has the same source/destination/count in its local register layout. Both stop inside this high-bank copy in the unmodified S251 address model (`out/agent-tests/hostrestart_flash_id_entry_probe/`). A debugger-only destination substitution to `RAM:01c6ef` makes both write the immutable source bytes `03 06 00 00 00 00 00 00` (`out/agent-tests/hostrestart_modepage_copy_probe/host-interface-main-vector-status-cause10-initialization-entry-via-timer-source/results.json`). The stock after-call breakpoint produces an unusable PC/state snapshot, so that experiment proves matching copied bytes under the substitution, not a stock caller return. Restart completion, a post-restart scheduler turn, high-bank access, controller timing, and physical interrupt delivery remain unverified. The monitor catalog now has 36 cases; the preceding 34-case aggregate remains valid for its unchanged cases, while the restart and event-`0x40` additions passed separately. Two main-mode packet cases now
pass through physical `INT1` at `RAM:ff0013`, the captured `RAM:ff3fb2`
router, and authentic `RETI` to the interrupted reset PC with `SP=7`.
With declared `RAM:01808e=0x40` and ready `RAM:018015=0x20`, the short-response
branch publishes `RAM:01808c=0x87` and `RAM:00125a=0xc0` (351 stock / 389
reconstructed states). With `RAM:01808e=0`, the same ready bit and declared
`RAM:01808d=0xa5`, the other branch captures ten `0xa5` bytes at
`RAM:00124f..001258`, sets `RAM:0011d9=1` and `RAM:00125a=0x80`, then
returns (680 / 740 states). Retained results are
`out/agent-tests/integration-gates/controller-packet-main-ready-versioned-run/results.json`
and `out/agent-tests/integration-gates/controller-packet-main-ten-byte-versioned-run/results.json`.
S251 far-controller read/write watches yielded no events for the short-response
case even though the final RAM delta proves the write. These cases therefore
compare declared controller inputs, captured RAM and return context, but do
not establish an ordered controller bus transcript. A new versioned main-mode
`INT0` case supplies all-high controller status and enters through the physical
`RAM:ff0003` vector and captured router. Stock and reconstruction both execute
the event dispatcher and authentic `RETI`, reach the interrupted reset PC with
`SP=7`, preserve `RAM:0000fc=1`, record event mask `RAM:00122a=0xfe` and source
status `RAM:001097=0xff`, and log the same two controller reads (361 / 407
states). Its result is
`out/agent-tests/integration-gates/controller-event-main-no-pending-versioned-run/results.json`.
The stock `@DR` reads require S251's default `X:` read watch here, whereas the
linked C route requires `watch: "RAM24"`; the earlier probe's one-size watch
placement missed one side and stopped inside the port strobe. A second versioned
case supplies active-low event bit 1 (`RAM:018003=0xfd`) and a pending bit at
`RAM:000fad=0x02`. Both images clear that bit, set `RAM:001302=0x20`, and
return through `RETI` with `SP=7`, matching event mask `RAM:00122a=0xfc`
and both declared controller reads (381 / 427 states). Its result is
`out/agent-tests/integration-gates/controller-event-main-bit1-pending-versioned-run/results.json`.
The same pending-bit event now passes in two longer physical-reset scenarios.
At the second `ResumeCoroutine0` checkpoint, with coroutine zero yielded,
a declared main selector and INT0 request return through the handler and reach
the third `ResumeCoroutine0` checkpoint. Both images preserve the exact saved
scheduler/coroutine contexts and `SP=10`, clear `RAM:000fad` bit 1, set
`RAM:001302=0x20`, record mask/status `0xfc/0xff`, and match 168 modeled
events (469,287 stock / 296,089 reconstructed states). Result:
`out/agent-tests/integration-gates/boot-main-int0-bit1-while-coroutine0-yielded-versioned-run/results.json`.
A second case injects INT0 at state 1,000 while coroutine zero is executing
`ReadDriveConfigurationBytes` during startup, then reaches the next
`ResumeCoroutine1` with restored `SP=10`. Both images have mask/status
`0xfc/0xff`, cleared pending bit, completion flag `0x20`, no Timer0 tick,
`SFR:A8=0x83`, `SFR:88=0x01`, and the same five modeled events (395,635 /
215,820 states). Result:
`out/agent-tests/integration-gates/boot-coroutine0-main-int0-bit1-active-versioned-run/results.json`.
These cases establish one pending event branch and scheduler continuation
under declared S251 controller inputs. Ordered controller writes for INT0, other pending event branches, remaining host command/status causes, active-scheduler host interrupt delivery, and physical interrupt timing remain open.

An exploratory active-coroutine-zero host cause-`0x01` fixture was **not**
accepted. It injected the generic S251 Timer0 source at state 1,000 and
seeded command state immediately beforehand. With an ongoing Timer0 source,
the reconstructed run exhausted its single declared cause read and stopped
at linked `HandleHostInterfaceInterrupt` `RAM:fb99fe`. Disabling that generic
source at state 1,100 avoided that stop, but the run later stopped at linked
`RunFullMediaReadVerifyRecoveryCoroutine0` `RAM:fcdeec` instead of the required
`ResumeCoroutine1` checkpoint. Reports are
`out/agent-tests/hostcause01_active_probe_v3/results.json` and
`out/agent-tests/hostcause01_active_probe_oneshot/results.json`;
`out/agent-tests/hostcause01_active_case.json` is disposable. This does not
establish a stock/reconstruction mismatch: the runner stopped on the
reconstructed side before a paired result, and the injected command state
may have changed the startup path. A future active-host case needs a
checkpointed scheduler context and a one-shot interrupt source that does not
alter the command state outside the handler.

A separate current-image case,
`boot-main-host-cause02-while-coroutine0-yielded`, starts from physical reset,
reaches the second `ResumeCoroutine0` with coroutine zero yielded, then uses a
declared one-shot S251 Timer0 source to enter the captured host vector with
cause `0x02`. At that scheduler checkpoint it seeds source count
`12 34 56 78`; stock and reconstruction copy all four bytes to
`RAM:001459..5c`, set `RAM:001596=1` and completion flags `RAM:000c83=0x10`,
and reach the third `ResumeCoroutine0` after `RETI` (469,294 / 296,041 states).
Both preserve `SP=10`, scheduler SPX `0x0a`, coroutine-zero SPX `0x307`,
and the exact other saved coroutine pointers from the established scheduler
fixture. The versioned definition is in the monitor catalog and its matching
case-hash report is `out/agent-tests/hostcause02_yielded_nonzero/results.json`
on the current application/map/monitor identities. This proves bounded host
interrupt service and scheduler continuation while coroutine zero is yielded.
The generic Timer0 vector is an S251 injection route; physical host IRQ
source delivery remains unverified.

A separate current-image case, `boot-coroutine0-main-host-cause02-active`,
starts at physical reset and raises the one-shot generic S251 Timer0 vector
while coroutine zero actively executes `ReadDriveConfigurationBytes`. It does
not seed packet state. With declared host cause `0x02` and a zero source
count, both images reach the next `ResumeCoroutine1` after the captured host
router returns (395,642 / 215,772 states). Both record cause `0x02`, leave
`RAM:001596=0`, set completion flags `RAM:000c83=0x10`, preserve the zero
four-byte count, and reach `SP=10` with the timer stopped and priority zero.
The case-hash-matching report is
`out/agent-tests/hostcause02_active_probe_corrected/results.json`; the
versioned definition is in the 39-case monitor catalog. The first scratch
run asserted `RAM:001596=1` from the yielded-context case and failed on both
images; correcting that fixture expectation produced a pass. This tests one
active-coroutine interrupt branch under declared S251 event delivery, not
physical IRQ timing or other host causes.

## 3. Complete host commands

Start from packet reception or the command processor coroutine, not solely a
handler. Drive a deterministic host request through admission, CDB parsing,
opcode dispatch, data transfer where applicable, sense preparation, controller
status publication, and return to an idle state ready for another command.
Cover at least: unsupported opcode, INQUIRY with a short allocation length,
REQUEST SENSE after an error, a read-only media-independent command, and a
representative media command with modeled controller responses. Compare payload
bytes, transfer lengths, sense key/ASC/ASCQ, status registers, event order,
completion count, and a second command to catch stale state.

Current evidence: `tests/simulator/cases/integration.json` passes a complete
unsupported-opcode dispatch from `RAM:fa34b2` through sense and controller
completion (307 stock / 340 reconstructed states; ten matching RAM/controller
changes). It also passes a four-byte INQUIRY from the same dispatch point
through response copy, modeled data-in completion, and return (1,259 / 1,369
states). The first four response bytes match `[05, 80, 00, 21]`; transfer
register deltas and balanced stacks match. S251 exposes the copied controller
buffer at `RAM:010000` in this model. A two-call trampoline now runs an
unsupported opcode and then REQUEST SENSE in one execution. The first call
returns to the exact intermediate PC with its stack restored; after a declared
CDB replacement and state-2000 data-in completion signal, the second call
returns normally. The 18-byte response at `RAM:010000` matches in both images:
`[70, 00, 05, 00, 00, 00, 00, 00, 00, 00, 00, 00, 24, 00, 00, 00, 00, 00]`
(hex), establishing sense `05/24/00` after the first error. Stock executes
2,265 states and reconstruction 2,377, with matching RAM/controller deltas
and balanced final stacks. A separate command-coroutine case begins at
`RAM:fdbe7d`, takes a twelve-byte INQUIRY packet from the modeled controller
window, passes admission and dispatch, transfers four response bytes, publishes
status, and reaches the idle-loop edge. Stock and reconstruction execute 3,772
and 3,805 states, respectively; both produce `[05, 80, 00, 21]` at
`RAM:010000`, reset packet state to one, and match 20 RAM/controller changes
and 17 modeled hardware events, including command-code writes `0x80`
then `0xc0`. Two more cases resume the same coroutine after that idle-loop
edge with a second packet. INQUIRY then unsupported opcode reaches two
completion edges (4,271 / 4,629 states), retains the first four-byte payload,
and records command-code writes `[0x80, 0xc0, 0xc0]`. Unsupported opcode then
REQUEST SENSE reaches two completion edges (5,298 / 5,328 states) and yields
the full `70 00 05 ... 24 00` response. In both, the first edge PC and stack
are verified before the second packet arrives; packet state returns to one.
A further packet-ingress case executes READ BUFFER mode 3 with a declared
`0x1000`-byte staging capacity. Both images return `[00, 00, 10, 00]` in
the descriptor and controller payload, publish `0x80` then `0xc0`, reset
packet state to one, and reach the idle-loop edge (4,839 / 4,895 states).
The data-in completion is declared at state 4,500, after transfer setup in
both implementations; its 17-event hardware transcript matches. A new
packet-ingress READ CAPACITY(10) case declares ready media, capacity 4,096
blocks at `RAM:0012c1`, and a 2,048-byte logical block at `RAM:000b00`.
The real admission, dispatch, eight-byte DATA IN and completion paths yield
`[00, 00, 0f, 69, 00, 00, 08, 00]` in both the response and controller
payload. The same coroutine then consumes unsupported opcode `0xff`, retains
the first payload, publishes `05/24/00` sense, and returns to the idle edge
with packet state one. Stock/reconstruction execute 5,310 / 5,648 states,
with matching 30-event hardware transcripts including controller command
codes `[0x80, 0xc0, 0xc0]`. The full fourteen-case run passes at
`out/agent-tests/integration-gates/fourteen-bounded-read-regression-run/results.json`.
This closes a media-dependent geometry-response scenario with modeled host
DATA IN completion. A new versioned packet-ingress READ(10) case supplies a
one-block CDB for LBA zero and stops at the first authentic `YieldCoroutine0`
inside the shared raw-sector engine. Both images decode one block, select
raw-read mode two, initialize coroutine two at canonical `RAM:f90000` or its
linked counterpart, set target frame `RAM:001598=150`, and retain command
completion state zero (2,200 stock / 4,546 reconstructed states). The case
sets `SP=0x40` for this direct launch, keeping nested calls above persistent
bit-addressable `RAM:0020..0028`. Both finish with byte `RAM:0022=0x10`,
identical controller snapshots at `RAM:018004=0x80` and `RAM:01802d=0x20`,
and matching ordered writes `0x8004=[0,0x80]`, `0x802d=[0x20]` with no
controller exclusions. Its passing result is
`out/agent-tests/r10yieldv2/results.json`. A separate entry probe at
`out/agent-tests/integration-gates/host-command-packet-ingress-read10-raw-engine-entry-probe-run/results.json`
confirms real packet dispatch to canonical `RAM:fc5107` (1,492 / 2,736
states). An earlier direct launch used the default shallow stack: stock
reached `SP=0x15`, reconstruction reached `SP=0x21` and left
`RAM:0022=0xf8` before the cursor helper, falsely suppressing linked
port writes. Raising the fixture stack base resolved that issue without a
firmware source change. A new versioned current-image case seeds the captured scheduler context and verified 512-slot ring count. From the same one-block READ(10) packet, both images execute `YieldCoroutine0` and return to the first `ResumeCoroutine1` scheduler checkpoint with `SP=10`, scheduler SPX `RAM:00115a=0x0000000a`, coroutine-two SPX `RAM:001166=0x00000707`, target frame 150, and ring limits `RAM:00129a..00129d = 01 ff 01 fa` (2,239 stock / 4,587 reconstructed states). Result: `out/agent-tests/read10-scheduler-yield-versioned-xvfb/results.json`. The corrected `tests/simulator/trace_read10_media_first_yield.py` continuation selects the real `FirmwareMainLoop` `ECALL ResumeCoroutine2` site (`RAM:fdcf87` stock / linked `RAM:fdcf6a`), so the scheduler call frame is preserved. Both images enter the primary raw-read coroutine at `RAM:f90000` / linked `RAM:fde81b` with SPX `0x700`, then reach their first `YieldCoroutine2` at `RAM:fee53f` / linked `RAM:feaa94`. Target frame 150, pipeline state `RAM:000f05=2`, `RAM:000e44=1`, `RAM:00127e=1`, ring count, and limits match (`out/agent-tests/read10-media-first-yield-ecall-final-xvfb/results.json`). The reproducible `tests/simulator/trace_read10_wait_cycle.py` continues through that yield: both return to the exact scheduler ECALL successor (`RAM:fdcf8b` / linked `RAM:fdcf6e`), naturally reach the next `ResumeCoroutine0` call site, enter `ServiceReadCommandRawSectorTransferBatch`, and reach a second command `YieldCoroutine0` with completed/requested blocks `0/1`, completion state zero and no media error (`out/agent-tests/read10-wait-cycle-final-xvfb/results.json`). A prior disposable direct jump to the `ResumeCoroutine2` function produced invalid later PCs because it omitted the ECALL return frame; the corrected call-site trace supersedes that diagnostic. This is a bounded command/media wait cycle with no sector descriptor or payload. At that earlier checkpoint, no descriptor or payload was supplied.
A further paired continuation (`tests/simulator/trace_read10_transfer_start.py`,
`out/agent-tests/read10-transfer-start-final-xvfb/results.json`) starts
from the same scheduler checkpoint and keeps the natural scheduler and command
resume path. At `ServiceReadCommandRawSectorTransferBatch` entry, both images
have an empty ring at `RAM:0088bb`, a 512-slot mask, and block size `0x0800`.
The trace declares one status-zero descriptor with controller source address
`0x002000` and increments the queue/producer counts to one. Both images enter
`ConfigureAndTransferRawReadSector`, then
`StartRawReadHostTransferFromStackAddress`: `RAM:001459=1`,
`RAM:001462=1`, controller command `0x88`, length `0x0800`, and address
`0x002010`. Both issue trigger `0x50`, control `0x80`, argument `2`, and status
`0x97`, then reach a command yield with the transfer latch still set. The
controller register bytes are read through S251's documented `X:0x80xx` view;
this trace verifies final register values, not write order. The descriptor is a
conditional test input, not an observed media producer result. No sector data or completion interrupt is supplied by this transfer-start
trace. A separate continuation checks those later control transitions.
The paired `tests/simulator/trace_read10_data_in_interrupt.py` continuation
also declares the prefix-acquired state `RAM:00127e=0`, matching the store in
canonical `AcquireRawReadPrefixAtCurrentPipelineTarget` (`RAM:fe3b86`). With a
declared controller cause `0x04`, stock and reconstruction enter physical
`RAM:ff000b`, their main host handlers, and normal raw-read completion
(`RAM:fed307` / linked `RAM:f83b81`). In both, `RAM:001462` changes 1 to 0
at the fourth stepped instruction in that worker; both return through the
interrupt entry and `YieldCoroutine0` to the scheduler's `ResumeCoroutine1`
call site with SP 10 and `RAM:001459=1`.
`out/agent-tests/read10-data-in-interrupt-final-xvfb/results.json` records the
fixed-length paired trace. The follow-on
`tests/simulator/trace_read10_command_idle.py` selects the scheduler's real
`ResumeCoroutine0` ECALL to skip unrelated coroutine turns, then single-steps
both images to the READ(10) engine return and next command idle yield. Both
have packet state `RAM:000c32=1`, command state `RAM:001226=4`, latch
`RAM:001462=0`, and issued/requested blocks `1/1`; result:
`out/agent-tests/read10-command-idle-final-xvfb/results.json`. These are
conditional control-flow results with a synthetic descriptor and no sector
payload. The completion interrupt was declared, and the post-handler path was
single-stepped. An unmodeled coroutine-one context prevents continuing the
whole scheduler turn directly; earlier open-ended breakpoints aimed at the
yield entry missed the actual interrupt return inside the yield helper and
were discarded. The versioned
`tests/simulator/trace_read10_second_command.py` continuation declares a
prebuffered unsupported opcode `0xff` at the idle scheduler checkpoint and
selects the real `ResumeCoroutine0` ECALL. Both images enter
`HandleUnsupportedScsiCommand`, write `70/05/24/00` sense, and reach another
idle yield with packet state one and completion flags zero. Result:
`out/agent-tests/read10-second-command-final-xvfb/results.json`. Actual
sector bytes, physical ingress of the second packet, controller event
ordering, and physical timing remain open.

## 4. Update and recovery

In an offline flash/controller model, exercise WRITE BUFFER mode 4 staging and
mode 5 validation. Reject invalid buffer ID, header, revision, length, and
checksum without any program/erase event. For valid main and auxiliary images,
trap and compare the handoff to the runtime loader `RAM:ff4000`, the relocated
`RAM:000800` programmer, its exact source/destination ranges, survival of
the main fixed-data region, and the copied primary configuration bytes during
auxiliary operation 5. The captured code issues an erase command at programming
address `RAM:f78000` even though operation 5 does not byte-program that region;
resolve its physical effect before accepting operation 5. Model both
successful activation (`RAM:0000fd=0x99`, jump to `RAM:ff0200`) and interrupted
or corrupt programming. Show that monitor bootstrap accepts a valid image,
rejects a bad checksum to `RAM:ff0300`, and honors forced recovery even for a
checksummed but nonfunctional main image. These reset-selection scenarios have
passing stock/reconstruction results. The user confirms the protected rescue
firmware at `RAM:ff0000` is entered by holding Eject during power-on and is
never flashed; the remaining update gate concerns the main-image programmer,
its mapping and completion, and its reset handoff to the rescue bootstrap.
A failed attempt to execute the `RAM:fd6b13` disc-update rejection path is not evidence of recovery behavior.

Current evidence: firmware/runtime disassembly documents the update contract
in `docs/FIRMWARE_UPDATE_ANALYSIS.md`, and host unit tests cover several
handoff branches. Two stock-versus-reconstructed mode-5 WRITE BUFFER
rejections pass through the real handler: invalid buffer ID 3 returns matching command-sequence-error
sense (587 / 918 states), and a main-image request while format state is busy
(`RAM:000c9e=1`, media status `RAM:000fae=0`) returns matching `04/40/F6`
sense (575 / 885 states). Both cases set forbidden execution breakpoints at
the runtime loader `RAM:ff4000` and relocated programmer `RAM:000800` and
return at the intended call boundary. A disposable positive control redirected
the handler to `RAM:ff4000`; the debugger stopped there and failed the case,
confirming the trap is live. A mode-4 staging request with offset `0x0ff0` and length `0x20` against
a declared `0x1000`-byte capacity also passes through the real handler. It
returns `05/24/00` sense and leaves transfer state zero (612 / 873 states),
with the same loader/programmer exclusions. This closes one bounds-rejection
case. A positive mode-4 packet with offset `0x10` and length 4 now uses the
authentic scheduler SPX and internal stack captured at physical reset. The
versioned `write-buffer-mode4-valid-yield-returns-scheduler` case passes in
`out/agent-tests/m4yieldv/results.json`: both images admit the packet,
prepare the transfer, and yield through `ResumeCoroutine1` to scheduler
`SPX=0x0000000a`, without entering `RAM:ff4000` or `RAM:000800`. The
reproducible `tests/simulator/trace_mode4_staging_resume.py` continuation
then invokes `ResumeCoroutine0` with the saved context and reaches the DATA
OUT transport (`ACC=0x3b`, length `DR0=4`, offset `DR4=0x10`) and the DATA
OUT helper (`RAM:fddf0b` stock; linked `RAM:fdb7be`) with transfer status
`RAM:000c33=8` on both sides. The same trace injects declared payload
`11 22 33 44` at the controller window `RAM:100010..100013` and completion status `0x0d` at
`RAM:000c33` after both images configure controller length 4 and destination
`0x10`. Both reach their DATA OUT helper return with that status, those
modeled bytes and controller cleanup `RAM:018047=0x14`. The corrected result is
`out/agent-tests/m4resume-staging-e/results.json`. A further controlled
continuation reaches the WRITE BUFFER handler return and command idle, with
packet state 1, transfer state 5, no sense, the four modeled bytes intact,
and identical ordered controller events including command-code writes
`[0x80, 0xc0]`: `out/agent-tests/m4complete-staging-e/results.json`.
The preceding-image results do not model physical DMA. The same mode-4 first yield, controlled DATA OUT return, and command idle pass on the preceding image under the corrected relocation harness at `out/agent-tests/integration-hdatafix-correct-xvfb/results.json`, `out/agent-tests/m4-final-resume-xvfb/results.json`, and `out/agent-tests/m4-final-complete-xvfb/results.json`.
The preceding image passes the versioned mode-5 zero-length first-yield case at
`out/agent-tests/integration-hdatafix-correct-xvfb/results.json`. At its controller-copy call, stock
firmware requires destination in DR0 and controller offset in DR4; the prior
C reconstruction passed these reversed. The corrected source and C251 listing
now match that call-entry contract. The reproducible `tests/simulator/trace_mode5_copy_entry.py` continuation reaches the first revision-copy read in both images with destination `RAM:000b2a`, source `RAM:100000`, end-offset `0x30`, declared minimum revision 2 and staged ASCII `00`, and the unmodified bank-add immediate `0x10`. Result: `out/agent-tests/m5-final-copy-entry-xvfb/results.json`. The unmodified S251 run still stops immediately after that high-bank read. A CPU microprobe showed that the debugger aliases `E:100000` to `E:010000` while native register-indirect access at `RAM:100000` returns zero and stops; the same instruction completes at simulator-safe `RAM:010000`. The reproducible `tests/simulator/trace_mode5_revision_rejection.py` applies an explicit debugger-only `0x10` to `0x01` bank substitution to the verified copy instruction in each image, seeds `X:001a..001b` with ASCII `00`, and asserts that both images complete the 48-byte copy and enter `SetSenseData` with arguments `4` and `0x40f5`. Result: `out/agent-tests/m5-revision-rejection-final-xvfb/results.json`. `tests/simulator/trace_mode5_rejection_completion.py` continues from the accepted sense-call trace and asserts the `70/04/40/F5` write, handler return, and command idle on both sides (`out/agent-tests/m5-rejection-completion-final-xvfb/results.json`). `tests/simulator/trace_mode5_header_rejection.py` seeds passing staged revision `09`, declares media state 1 after the first yield to avoid an unmodeled controller wait, and verifies that a staged zero header byte versus installed `P` selects header rejection, writes the same sense, and returns to idle (`out/agent-tests/m5-header-rejection-final-xvfb/results.json`). These preceding-image conditional offline results do not establish main-image checksum rejection, accepted-image programming, activation, or physical controller-window alias. An earlier bounded stock-only continuation with the installed identity staged reached `ValidateStagedFirmwareChecksum` at `RAM:fd2c93`, then stopped at `RAM:fd2cb8` with source cursor DR4=`RAM:020000` after 3,807,024 states, before the checksum sense call (`out/agent-tests/m5-checksum-reject-stock-probe/`). On the current image, the paired passing-header continuation reaches the stock `RAM:fd2c93` and reconstructed `RAM:f84fb4` checksum entries with DR0=0 and DR4=`0x79ffe` (`out/agent-tests/m5mainchecksum_entry_probe/results.json`). A bounded continuation then reaches the same S251 bank-`0x02` data-read limit on both sides: stock DR4 and reconstructed DR0 each hold `RAM:020000` after 3,807,024 and 3,418,023 states respectively (`out/agent-tests/m5mainchecksum_boundary_final3/results.json`). The reconstructed debugger may report PC zero at this invalid-bank halt. No main-image checksum decision or sense result is accepted from these traces. The preceding-image fourteen-case regression run is retained at
`out/agent-tests/integration-gates/fourteen-bounded-read-regression-run/results.json` with
stock SHA-256 `622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2`
and reconstructed SHA-256
`bcf3f8b21c0bbb3c4fc84db28bf6fc44a97fec77fa040855eb553e3a7fde03c8`.
These are loader-entry exclusions, not a flash-event log or proof of all
rejection branches. Main-image staged-checksum rejection, effective programming ranges, and activation remain open. The current-image auxiliary path conditionally reaches `SetSenseData(4, 0x40f6)` and returns to command idle with written `70/04/40/F6` sense (`tests/simulator/trace_mode5_aux_checksum_entry.py`, `tests/simulator/trace_mode5_aux_checksum_rejection.py`, `tests/simulator/trace_mode5_aux_rejection_completion.py`). Stock `ValidateStagedFirmwareChecksum` at `RAM:fd2c93` takes start offset in DR0 and end offset in DR4; the prior C formal order reversed these registers. The corrected C251 call and paired entry trace both use DR0=0, DR4=`0xfffe`. The original S251 observation scratch occupies `X:6000..7fff` inside the auxiliary checksum candidate `X:0000..7fff` (`out/agent-tests/m5auxview_final/results.json`), so that original harness layout cannot support a clean accepted-image control. Relocating that scratch and applying the checksum-read bank substitution yields an identical-byte auxiliary acceptance and pre-flash handoff control (`out/agent-tests/m5auxcleanaccept_final/results.json`, `out/agent-tests/m5auxcleanhandoff_final2/results.json`). No flash loader is entered in the rejection trace. A separate optional monitor
case launches the captured `RAM:ff4000` loader with declared operation 5 and
a recognized `0x2000` flash ID. Both images copy the `0x40e`-byte programmer
to `RAM:000800` and reach `RAM:00082b` after the first unlock-store instruction
at `RAM:000828` (12,166 states each, matching registers and RAM changes). The
declared target register is `DR16=0xf0aaaa` with `WR24=0xaa00` at this edge.
A single `G` to the later `RAM:0008b1` checkpoint had stopped at
`RAM:00082b` in both images. The reproducible
`tests/simulator/trace_runtime_flash.py` now resumes S251 after each
flash-window store from that passing first-unlock case. The 2026-09-24 trace
at `out/agent-tests/integration-gates/operation5-2000-cpu-store-ring-order-pc-pair/results.json`
records 14 matching post-store PCs, target registers, CPU addresses and words
for stock and reconstruction. It confirms the first five unlock/setup stores
at `RAM:f0aaaa`/`RAM:f05554`, then `0x3000` stores at programming
`RAM:f00000`, `RAM:f10000`, ... `RAM:f60000`, `RAM:f78000`, and
`RAM:f7c000`. The final post-store checkpoint is `RAM:0008c5`.
The 32-byte trace record keeps the 32-bit state counter separate
from SP and captures PC twice, requiring at least one exact checkpoint PC. Both counters increase from 12,166 to 12,292. The old 24-byte
record overwrote a counter byte with SP and falsely suggested a final drop.
This is a CPU-store trace, not a flash-chip write/effect log. A disposable
attempt to extend the trace toward status-poll `RAM:000906` did not yield a
validated checkpoint. Therefore the accepted trace does not establish erase
completion or subsequent programming.

A second optional monitor case declares `RAM:0000f8=0xbf00` and reaches
the alternate programmer branch's first unlock-store successor
`RAM:0008d6` in both images (12,176 states each, matching registers and
RAM changes). The same controlled-resume tool records six matching stores
through post-store `RAM:0008f9`: the five unlock/setup words followed by
`0x5000` at programming `RAM:f00000`. Its retained result is
`out/agent-tests/integration-gates/operation5-bf00-cpu-store-ring-order-pc-pair/results.json`.
The declaration selects a firmware branch; the physical drive's normal-mode
selector was measured as `bf00`, so the retained trace declares that same value.

The inspected board's SST39LF400A has uniform 4-KiB erase sectors and 64-KiB
blocks; the [manufacturer datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST39LFX00A-SST39VFX00A-2-Mbit-4-Mbit-8-Mbit-x16-Multi-Purpose-Flash-DS20005001.pdf)
requires a six-write `0x30` sector-erase or `0x50` block-erase command and
states that commands during an internal erase are ignored. The physical
`RAM:0000f8` selector is `bf00`. Its byte-swapped correspondence to SST
manufacturer word `00bf`, the exact doubled x16 unlock addresses, and the live
full-window read aliases strongly establish the address and command lane.
Under the resulting high-byte command-lane and
`+0x80000` execution-alias model, the trace's first complete six-store
sequence erases programming `RAM:f00000..f00fff`, corresponding to executable
`RAM:f80000..f80fff`. That would replace the `RAM:f80050` reset veneer with
`ff ff ff ff`, change 3,682 non-FF stock bytes, and leave computed checksum
`0xf55a` against stored `0xb3bd` (earlier reconstruction with checksum
`0x782d`: 4,010 bytes and computed `0x34fe`). The alternate `0xbf00` trace's `0x5000` command instead
conditionally erases programming `RAM:f00000..f0ffff` (execution
`RAM:f80000..f8ffff`): 63,469 stock bytes, reset veneer `ff ff ff ff`, and
computed checksum `0x481a` versus stored `0xb3bd` (earlier reconstruction:
65,136 bytes, `0x1fc9` versus `0x782d`). On the preceding packaged image
(SHA-256 `2c6e774a886c791cce180dbf57174c6fc68b136c9582d045a56026e7f2b3e1ef`),
the same conditional first sector erase changes 4,010 bytes and computes
`0x5282` versus stored `0x9a8e`; the block erase changes 65,139 bytes and
computes `0x3fbc` versus stored `0x9a8e`. Both leave the reset veneer
`ff ff ff ff`. The reproducible disposable report is
`out/agent-tests/integration-gates/config-queue-fix-conditional-first-erase.json`.
The operation-5 allowed-range guard
rejects the first effect in both declared branches. The `0x2000` branch's immediately following
bare `0x3000` store at `RAM:f10000` does not form another complete SST erase command
in the strict sequential model. None of these model results proves a physical
erase, nor does the raw `RAM:f78000` store prove it became an effective chip
command. Resolving actual device-ID selection, bus lane, alias and flash
response remains necessary before accepting either update operation.

The same first-unlock cases were rerun on the preceding image and its
application/map/monitor identities. The `0x2000` branch again yields 14
matching CPU flash-window stores through `RAM:0008c5`
(`out/agent-tests/flash-first-unlock-current-xvfb/results.json`,
`out/agent-tests/flash-first-stores-current-xvfb/results.json`); the `0xbf00`
branch yields six matching stores through `RAM:0008f9`
(`out/agent-tests/flash-bf00-first-unlock-current-xvfb/results.json`,
`out/agent-tests/flash-bf00-stores-current-xvfb/results.json`). Under the
explicit high-byte command-lane and `+0x80000` alias assumptions, the first
sector erase changes 4,010 bytes of that reconstructed image
`2b965412b1e539d0b2cf0da1b27e78b901e4b1e87c950344b10014eed39e1979`,
leaves reset veneer `ff ff ff ff`, and changes its computed checksum to
`0x51b0` versus stored `0x99bc`. The first block erase changes 65,139 bytes,
leaves the same erased veneer, and computes `0x3fd0` versus `0x99bc`.
The corresponding stock-image calculations are 3,682 bytes / `0xf55a`
for sector erase and 63,469 bytes / `0x481a` for block erase, versus stored
`0xb3bd`. Both modeled effects violate the operation-5 allowed-range guard.
These calculations use disposable image copies; CPU stores do not establish
physical chip effects or the LC898095 flash alias.

On the preceding reconstructed image SHA-256
`061ed2a1722b64030f362d537e6d8c1ffdc31c6619f0c961826db2f2c1ca4e76`,
`out/agent-tests/f2000fix/results.json` records 14 matching CPU stores in the
`0x2000` branch and `out/agent-tests/fbf00fix/results.json` records six in
the `0xbf00` branch. Under the same explicit high-byte command-lane and
`+0x80000` alias assumptions, the first sector erase changes 4,010 bytes
and produces checksum `0x5846` versus stored `0xa12f`; the first block erase
changes 65,139 bytes and produces `0x4922` versus `0xa12f`. Both modeled
erases blank the reset veneer. The corresponding stock model results remain
3,682 bytes / `0xf55a` and 63,469 bytes / `0x481a`, respectively, versus
stored `0xb3bd`. These are disposable image calculations, not observed chip
erases or a validated physical address/lane mapping.

On the current reconstructed image SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c`, both operation-5 branches again match the bounded stock CPU stores (`out/agent-tests/f2000checksumfix/results.json`, `out/agent-tests/fbf00checksumfix/results.json`). Under the same explicit flash-bus assumptions, the first sector erase changes 4,010 bytes and computes `0x5874` versus stored `0xa15d`; the first block erase changes 65,139 bytes and computes `0x4950` versus `0xa15d`. Both modeled erases blank the reset veneer. These calculations remain conditional and are not chip observations.

The disposable `tests/simulator/flash_bus.py` model replays SST-style
six-write sector/block erase and four-write word-program sequences into a
copy of the 512-KiB image. Its seven model tests include the conditional
`RAM:f78000` sector-erasure checksum failure and operation-5 range rejection.
The model assumes a high-byte command lane and `+0x80000` alias, and omits
busy timing, status polling and protection. It is connected to the initial
six captured CPU stores in both declared branches through their trace reports,
but the later bare `0x3000` stores and full programming path are not modeled or validated.
The 134 simulator, flash-model, and trace-record Python tests pass. The preceding image
had SHA-256
`9f4dfef559f8c06c7bd607e052376a8b6cd092288c9d65da57eb27a3da619b9e`
and checksum `0x7698`. Its fourteen-case integration regression passes at
`out/agent-tests/integration-gates/fourteen-controller-xdata-fix-run/results.json`.
The current CPU-store traces and conditional erase calculations above use
the corrected trace record and then-current image. That image has SHA-256
`8178fc3c8f3189c9665fed5bfa33e234b3d56ec6da27715bbbada83bea23760a`
and checksum `0x782d`; all 21 versioned monitor cases pass at
`out/agent-tests/integration-gates/monitor-twentyone-scheduler-cycle-corrected-run/results.json`,
and all 17 versioned integrated cases pass at
`out/agent-tests/integration-gates/integration-seventeen-after-ring-order-fix/results.json`.
Its first-unlock checkpoint cases pass in the 21-case monitor suite. The
preceding-image first-unlock checkpoint cases pass at `out/agent-tests/integration-gates/runtime-operation5-2000-current-first-run/results.json`
and `out/agent-tests/integration-gates/runtime-operation5-bf00-current-first-run/results.json`.

The complete 21-case earlier-image monitor suite passes at
`out/agent-tests/integration-gates/monitor-twentyone-scheduler-cycle-corrected-run/results.json`.
Its cases include reset into the command coroutine, physical routed interrupt
returns, bounded update-loader checkpoints, and the boot decisions below.
The earlier-image optional monitor suite checks normal bootstrap handoff at `RAM:ff3f7d` (5,164,567 states in each run), a one-byte
checksum corruption reaching recovery entry `RAM:ff0300` (5,152,441 states
each), and forced recovery of the intact image at the same entry (15,839 states
each). Two more cases replace the reset veneer `8a fd ce a9` with a
four-byte `EJMP RAM:f80050` self-loop and adjust each image's stored checksum
by minus 300: the normal branch still reaches `RAM:ff3f7d` (5,164,567
states each), while forced recovery reaches `RAM:ff0300` (15,839 states
each). Disposable binaries with these bytes independently pass
`tools/firmware_image.py verify` (`0xb291` stock-derived; `0x756c`
reconstruction-derived). Current-image results are retained at
`out/agent-tests/integration-gates/monitor-current-boot-decisions-run/results.json`,
`out/agent-tests/integration-gates/monitor-forced-recovery-current-run/results.json`,
and `out/agent-tests/integration-gates/monitor-current-self-loop-decisions-run/results.json`.
A further versioned case models the conditional first 4-KiB SST erase by
filling main `RAM:f80000..f80fff` with `0xff` while preserving each image's
stored checksum word. Both images reach recovery entry `RAM:ff0300` with
`ACC=0` after 5,152,441 states; the case requires that value explicitly.
Its result is retained at
`out/agent-tests/integration-gates/monitor-erased-reset-sector-versioned-run/results.json`.
This tests the monitor decision after one modeled interruption point. A new versioned alternate-device case fills `RAM:f80000..f8ffff` with `0xff` to model the first 64-KiB block erase. Both current stock and reconstructed images likewise reach recovery entry `RAM:ff0300` with `ACC=0` after 5,152,441 states; the individual result is `out/agent-tests/m34blockerase/results.json`, and all 34 monitor cases pass together at `out/agent-tests/m34blockfull/results.json`. Both erase cases require the explicit ACC result. These tests do not prove a physical erase, subsequent programming, or monitor-mode reflashing.
The forced case pauses at the captured `RAM:ff4e72` predicate entry,
verifies the pause PC, applies declared `SFR:P0/P1` latch and `PORT0/PORT1`
pin values, then resumes. Both images return `ACC=1` and enter the recovery
loop before any checksum scan. The monitor computes the expected image-specific
full-image sum on its normal path:
`0xb3bd` stock and `0x782d` reconstructed. S251 initially exposed the stock
stored checksum word at `RAM:ff9ffe` to both runs, despite computing different
sums from their main-image bytes. The cases map each tested image's two stored
checksum bytes into that code view before launch. Their reset-window sums
(`0x14d3e` stock, `0xfff5` reconstructed) differ, so the cases compare the
resulting monitor decision and shared RAM state rather than those raw sums.
This validates the monitor's offline decision path under the declared mapping;
it does not model flash programming or prove physical bank selection.

## Physical readiness after the offline gates

The image format/checksum build, complete C function coverage, and direct
function smoke catalog do not establish drive safety. The protected rescue
firmware and Eject-at-power-on entry are a user-provided hardware property. The captured reset-selection cases establish the intended
route offline; live rescue reflashing is not a prerequisite for this gate.
Before a physical write, establish which flash cells the programmer actually
changes and retain the stock image and the Eject-at-power-on recovery procedure.
Flash writes, activation, resets, and destructive drive tests require explicit
authorization for the exact experiment and a stated recovery strategy.

## Execution order and acceptance record

1. Keep the accepted integrated cases green while adding inputs. Each new
   scenario records the input image hashes, initial RAM/SFR/controller state,
   timed events, exact checkpoint, minimum and maximum execution states, and
   stock/reconstructed observable deltas. A process exit or arrival at a PC
   without verified code bytes is not a pass.
2. Extend the passing mode-4 handler-to-idle and mode-5 revision/header and
   conditional auxiliary checksum rejection and acceptance-to-handoff paths to
   main-image checksum rejection, then effective programming and recovery with
   a flash-event log. Keep each emulator scenario bounded by an exact checkpoint and a per-case time limit. Stop expanding a scenario when debugger memory or controller modeling determines its outcome; record the limitation and switch to independent static evidence or a separately authorized hardware observation.
3. Resolve the relocated programmer/coroutine-three stack overlap using
   reset, reinitialization, and RAM-bank evidence before specifying the next
   natural scheduler checkpoint. Extend active-scheduler interrupts and
   READ(10) checks to actual sector bytes and physical next-packet ingress.
4. Extend the disposable flash model with an effective erase/program log and
   range checks. Compare valid handoff, interruption, recovery, and boot
   decisions against stock, then resolve physical alias and command-lane
   behavior before considering a drive write.
5. Record each gate as passing only with a committed scenario and a retained
   differential result. Keep device flashability separate: no physical write
   should be inferred from these offline results.
