# LED diagnostics

The `PLEXTOR_LED_BOOT_DIAGNOSTICS` CMake option builds a separately named,
flashable diagnostic variant of the reconstructed firmware. It uses
`BITS:0580/0581` (P3.0/P3.1) as the two front-panel LED outputs. Physical
testing of the first-pass image showed both the yellow and green indications
and a complete checkpoint-11 frame.

The normal build compiles every checkpoint away. Diagnostic packaging writes
`MONT_reconstructed_led_diag.bin`; it never reuses the ordinary firmware image
name.

## Diagnostic 08: DATA-IN outcome

Current artifact:
`out/agent-tests/diag08-data-in-outcome-20260925/PXW5224A_diag08_data_in_outcome.bin`

SHA-256: `8e7270a2dfa6509985f7f1517aac87369aabae27ce37de7017909de9abe78ae2`
Checksum: `0x3352`; 524,288 bytes; fixed data matches stock; zero Keil diagnostics.

Diagnostic 07's setup stop is removed. A volatile XDATA byte is cleared on
entry to FirmwareMainLoop and set on entry to ExecuteControllerDataInTransfer.
There is no blink or delay before an outcome. The first reached outcome stops
execution, disables global interrupts, and repeats one long yellow plus N green:

| N | Exact observed boundary |
|---|---|
| 1 | Host ISR selected the `cause & 4` completion branch while DATA-IN was active, before CompleteHostDataTransfer. |
| 2 | DATA-IN's existing timer timeout branch reached SetSenseData(4, 0x1b00) and returned. |
| 3 | The configuration/operation abort predicate in the DATA-IN wait evaluated true. |
| 4 | The DATA-IN wait observed bit 0x10 in RAM:000c83, its host-completion abort flag. |
| 5 | The host ISR reached its tail with a firmware or coroutine restart request while DATA-IN was active. |
| 6 | The DATA-IN wait exited without first reaching any of these stops. |

This is one repeating result, not a progression through all six stages. Normal
drive operation intentionally stops at that result. No extra Bus Hound capture
is needed. No repeating frame is inconclusive: the core may have stopped/yielded
elsewhere or a different restart path may have discarded the observation.
The worker is shared by ATA IDENTIFY and SCSI responses; neither this result nor
diagnostic 07 alone proves the opcode or correct transferred bytes. Count 1
proves the completion branch was selected, not that INQUIRY completed on USB.

Generated listings verify all six branches, initialization of the active byte
at linked RAM:011a34, removal of the setup stop, and CLR EA in the reporter.
The focused host DATA-IN transfer and host-interface-interrupt tests pass with
diagnostics compiled out. An inadvertently launched full foundation test did
not finish promptly and was stopped; no full-suite pass is claimed. The build
option was restored to OFF after archiving. Flashing remains user-operated via
redumper; use the already established Eject-at-power-on recovery route.

## Completed diagnostic 07: DATA-IN-programmed breakpoint

Retained artifact (superseded by diagnostic 08):

`out/agent-tests/diag07-data-in-programmed-breakpoint-20260925/PXW5224A_diag07_data_in_programmed_breakpoint.bin`

SHA-256: `ae4b11fca20ecc6480d87381561e53878b180df4d831b8e5831d8792b0973374`  
Firmware checksum: `0x74e3`

This image has no earlier command-path LED markers. Immediately after the
INQUIRY response has been copied and the DATA-IN controller registers through
`0x805e=0x97` have been programmed, it deliberately stops and continuously
repeats one long yellow pulse followed by two short green pulses. Seeing that
exact repeating frame proves the command reached the programmed DATA-IN
boundary. Any other stable or transient LED state is ordinary firmware output
and must not be interpreted. This image intentionally cannot complete INQUIRY;
do not make another Bus Hound capture for it.

The image is 524,288 bytes, has stock-identical fixed data, and passed firmware
format/checksum verification. C251/L251 completed with zero warnings and zero
errors. The generated listing confirms the breakpoint call is after the final
DATA-IN setup store and that its reporter loops forever.

The user observed one yellow and two green. This positively establishes that
the shared DATA-IN worker reached its post-programming stop. The worker also
serves ATA commands, so the observation alone does not establish INQUIRY as
the caller or validate the response bytes/address/length on the physical bus.

## Retired static INQUIRY diagnostic

The current diagnostic performs no blinking and no delay. It latches these
states while the first host command is processed:

| Visible state | Reached state |
|---|---|
| Off | `ReceiveScsiCommandPacket` returned, but INQUIRY has not entered. |
| Yellow | `HandleInquiryCommand` entered. |
| Green | The DATA-IN controller registers were programmed and its completion wait began. |
| Yellow and green | The DATA-IN wait returned and controller cleanup completed. |

Physical output was `both -> off -> both`, followed by the Windows device
sound. This does not prove that the instrumented DATA-IN worker returned:
ordinary foreground, coroutine, timer, and interrupt code has many writers to
the same P3 bits and can overwrite any one-time state. Treat only the Bus Hound
DATA-IN/CSW record as authoritative. Do not flash this image again. If a later
test needs an LED milestone, it must continuously reassert an unmistakable
pattern after one selected breakpoint and intentionally stop there.

The simultaneous Bus Hound trace shows repeated 36-byte standard INQUIRY CBWs,
but every observed bulk-IN is canceled with no payload and no CSW, followed by
a USB bus reset and retry. A full descriptor/configuration enumeration in the
middle is the bridge recovering, so the Windows device sound and final two-LED
state are not evidence that the Plextor command completed. The remaining live
boundary is DATA-IN reachability/completion: use a persistent breakpoint first
after controller programming and, in a later image if necessary, at host
interrupt cause bit 2 (`cause & 4`).

The current artifact is
`out/agent-tests/diag06-inquiry-static-led-20260925/PXW5224A_diag06_inquiry_static_led.bin`,
SHA-256 `9d96274d2578282310076f26615db3c7a1a2a61a0b000a876630dcee37fecb27`,
checksum `0x3b41`. It is 524,288 bytes, has stock-identical fixed data, linked
with zero diagnostics, and contains no call site to the slow checkpoint
reporter.

## Retired pulse format

Each frame contains:

1. one long indication on the marker LED (`01`);
2. a separator with both diagnostic lines idle (`11`);
3. `N` short indications on the other LED (`10`), where `N` is the stage;
4. a longer idle separator before the next frame.

The count color no longer alternates. Count only the short indications after
the long marker. Every timing unit is four times as long as in the first-pass
image: the delay worker runs four times per unit. The reporter uses a CPU busy
wait rather than a peripheral timer.

The current image stores its emitted-stage bitmask in linked XDATA and clears
it after initial coroutine configuration. Each stage therefore appears at most
once even when its source location is inside a loop or spans a coroutine yield.

## Retired host-command stage map

| Stage | Reached state |
|---:|---|
| 1 | SCSI command-processor coroutine 0 entered. |
| 2 | `InitializeScsiCommandProcessor` returned. |
| 3 | The optional initial drive-startup handoff yielded and returned, or that handoff was not required. |
| 4 | The main command-processing loop was entered. |
| 5 | The processor prepared its first idle yield while waiting for a host packet. |
| 6 | That first idle yield returned and idle continuation completed. |
| 7 | `ReceiveScsiCommandPacket` returned with an actual host command cycle. |
| 8 | The first received command reached a completion-handling path. |

The frames must be monotonic and nonrepeating. Stage 6 without 7 means the
processor is alive and repeatedly yielding but no complete host packet reaches
the foreground command path. Stage 7 without 8 places the failure in protocol
dispatch or command execution. Stage 8 proves that one command cycle reached
completion handling, although the inserted LED delay can still perturb host
timeouts.

## Retired diagnostic-04 artifact

The current host-command Windows build is retained at
`out/agent-tests/diag04-hostcmd-slow4x-20260925/`:

- image: `PXW5224A_diag04_hostcmd_slow4x.bin`
- SHA-256: `ba284d0dfc092812dc1bcf32633212256cf68b775f47f01de2fb1c85b3c782ba`
- firmware checksum: `0x2560`
- size: 524,288 bytes
- stock fixed-data match: true
- linked code size: 439,312 bytes
- linked XDATA size: 6,823 bytes
- Keil diagnostics: zero warnings and zero errors

The retained map and listing prove that the bitmask uses `MOVX` memory access,
is explicitly armed after `ConfigureInitialCoroutines`, and is not retained in
`DR12` across a coroutine switch.

Every later hardware-test image must be copied under a new descriptive filename
before it is handed to the user; do not reuse this name or an earlier generic
diagnostic filename.

## Retired scheduler-section evidence

Only the first scheduler pass was intended to be reported. Physical testing
showed that the register-backed one-shot guard is not preserved by the native
coroutine switch, so later partial repetitions can occur. The first complete
`1..6` sequence remains valid; do not use later repetitions as control-flow
ordering evidence.

### Scheduler-section stage map

All eleven already-proven cold-initialization checkpoints were removed. This
section restarts numbering at one:

| Stage | Reached state |
|---:|---|
| 1 | Scheduler pre-resume work completed; coroutine 0 is about to resume. |
| 2 | Coroutine 0 returned/yielded to the scheduler; coroutine 1 is next. |
| 3 | Coroutine 1 returned/yielded to the scheduler. |
| 4 | The coroutine-2 gate completed; if coroutine 2 was resumed, it returned/yielded. |
| 5 | The coroutine-3 gate completed; if coroutine 3 was resumed, it returned/yielded. |
| 6 | The post-resume coroutine-0 reinitialization gate and the first complete scheduler pass finished. |

Interpret the last complete frame as follows:

- no frame: execution did not reach the first scheduler resume after the
  previously proven cold initializer;
- stage 1 only: the first `ResumeCoroutine0` did not return to the scheduler;
- stage 2 only: the first `ResumeCoroutine1` did not return;
- stage 3 only: the coroutine-2 gate or active coroutine 2 did not complete;
- stage 4 only: the coroutine-3 gate or active coroutine 3 did not complete;
- stage 5 only: the post-resume reinitialization/loop-end path did not complete;
- stage 6: the entire first scheduler pass completed, so the next pass must
  instrument the failing subsystem more deeply.

Record from before reset or power-on through the final steady LED state. Report
the complete stage sequence and any tray/motor sound aligned with it.

### Retired scheduler artifact

The scheduler-section Windows build is retained at
`out/agent-tests/led-scheduler-diagnostic-20260925/`:

- image: `MONT_reconstructed_led_diag.bin`
- SHA-256: `482bcb030871c8ed74bc4b8add8475f769e6c6716905b32da6729a6287a488af`
- firmware checksum: `0x61ae`
- size: 524,288 bytes
- stock fixed-data match: true
- linked code size: 439,244 bytes
- Keil diagnostics: zero warnings and zero errors

The retained directory also contains the exact map, HEX file, three relevant
listings, validation report, and generated update plan.

Physical output was `1,2,3,4,5,6,2,3,4,5,6,2,5,6`. The first sequence proves
one complete scheduler pass. The later repetitions result from C251 allocating
the diagnostic guard in `R15`: the coroutine switch replaces `DR12` and does
not restore the scheduler's old `R15`. Do not flash this image again. Future
repeating-path diagnostics must store their one-shot mask in explicitly
initialized memory.

## Retained first-pass evidence

The first-pass cold-initialization image remains unchanged at
`out/agent-tests/led-boot-diagnostic-20260925/` with SHA-256
`452eeb7a5f02a001891c7816ed0f056fa60f8b009df6f198f32d1c4c93699b3f`.
It emitted eleven cold-start frames with alternating count colors. Physical
testing reached frame 11, proving that `InitializeDriveFirmware`, the indirect
runtime flash-identification loader, and `ConfigureInitialCoroutines` returned.

These images are diagnostics, not release candidates. The user owns flashing
and retains the verified Eject-at-power-on protected recovery route and the
immutable stock 1.04 image.
