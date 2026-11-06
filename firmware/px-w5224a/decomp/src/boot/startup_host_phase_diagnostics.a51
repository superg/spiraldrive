; Diagnostic terminal only. C supplies a literal result after the observed
; branch; preserve that count across the shared, slow frame reporter.
$CASE
NAME STARTUP_HOST_PHASE_DIAGNOSTICS
PUBLIC StartupHandshakePhaseHalt?
EXTRN ECODE (StartupForegroundCheckpoint?)

STARTUP_HOST_PHASE_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_HOST_PHASE_DIAGNOSTICS

StartupHandshakePhaseHalt?:
    CLR 0AFH
    MOV R14,R11
PhaseReport:
    MOV R11,R14
    ECALL StartupForegroundCheckpoint?
    SJMP PhaseReport
END
