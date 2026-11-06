# Diag17 startup continuation investigation

2026-09-26, stock firmware 1.04 versus the unchanged diag17 build.

The user reports flashing diag17 with "nothing good". A present-device query
again found no Plextor bridge or optical child. This is not a readback of the
installed image, and exact tray behavior was not reported. The timer-input
correction remains evidence-backed, but did not restore operation.

## Conditional comparison

`tests/simulator/cases/p1-startup-continuation.json` has three passing paired
S251 cases. Evidence: `out/agent-tests/p1-startup-continuation-checked/`.
The scratch Windows runner uses the hidden desktop:

```powershell
python out/agent-tests/run_p1_startup_continuation.py <new-output-directory-name>
```

The cases execute the real scheduler and native coroutine yield/restart
boundaries. They reach stock RAM:fb01d0 (P1.7 predicate), RAM:f852eb
(initialization-completion coroutine), and RAM:f84f6a (drive state machine).
Stock/rebuild state counts are respectively 395549/62736, 396120/63442,
397558/65190. These counts are simulator states, not measured hardware time.

Declared assumptions and exclusions:

- RAM:0000fb=1 skips cold hardware initialization; RAM:0011ed/0011ee=80
  selects asserted debounced P1.7; RAM:0011ef=08 skips calibration loading.
- A timed write releases initial RAM:001214 countdown. Controller
  RAM:018015 reads return a0; SFR:d7 reads return zero. Responses are bounded
  and traced, using the appropriate stock RAM24 and rebuilt MOVX watches.
- RAM:fb01de is an explicit result-2 stub; RAM:f85583 is a return stub.
  Thus drive-control and startup-media initialization are outside this check.
- Coroutine stack RAM:000500..0009ff contains compiler-specific saved
  registers/return addresses and is excluded from byte equality. Actual
  checkpoint execution, saved coroutine1 SPX=00000507, and all other captured
  firmware/controller RAM deltas are compared. Coroutine1 restart pointers
  are normalized and compared at the latter two checkpoints.
- Expected RAM:001300/001301 pairs are 00/10, 00/30, and 08/30.
  Dependency and hardware traces also agree.

The initial broad comparison reported mismatches solely from stack contents
and unnormalized interior return addresses. Inspection found no differing
non-stack firmware/controller RAM delta before defining the explicit stack
exclusion. Intermediate scratch runs are not accepted passing cases.

## Earlier path that still needs investigation

Live canonical Ghidra confirms stock RAM:fdbfc4 initializes coroutine1 to
RAM:f84b0b when RAM:00faef bit7 is set and RAM:001009 is zero. This happens
inside the initial SCSI coroutine0 execution, before coroutine1 first resumes.
Consequently, the initial f84c30 entry is replaced; the trace first runs
RunDriveInitializationCoroutine1 and InitializeDriveControlSequence fb01de.

Earlier scratch traces without these stubs stalled in controller handshakes
and pickup homing inside that earlier initialization. They did not establish
a stock/rebuilt divergence, nor execution after the startup P1.7 decision.
An early claim that stock had reached the checkpoint was corrected during
the investigation: the runner executes reconstruction first and skips stock
when reconstruction fails its required stop.

Next inspect fb01de's real calls to fb07ff, fb00ee, and fb034e; the last has
its own P1.7 call at fb0517. Preserve real coroutine yields and supply only
explicit timer/sensor responses. The passing downstream cases do not clear
these earlier paths, startup-media initialization, interrupt timing, or
physical memory backing. No production change or new flash image resulted
from this investigation.
