; Addressless diagnostic checker for the diag23 count5 interval.
; u8 point in R11. Points1..6 report only when RAM:000c32 is zero.
; Point7 always reports:7 for zero,8 for nonzero. No persistent state.
; Silent return restores IE, PSW/PSW1 and DR8. Other registers untouched.
$CASE
NAME STARTUP_HOST_INTERVAL_DIAGNOSTICS
PUBLIC StartupHostIntervalCheckpoint?
EXTRN ECODE (StartupForegroundCheckpoint?)

STARTUP_HOST_INTERVAL_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_HOST_INTERVAL_DIAGNOSTICS

StartupHostIntervalCheckpoint?:
    PUSH 0A8H
    CLR 0AFH
    PUSH 0D0H
    PUSH 0D1H
    PUSH DR8
    MOV R10,EDATA 0C32H
    ORL R10,R10
    JE HostIntervalStop
    CMP R11,#7
    JE HostIntervalNonzero
    POP DR8
    POP 0D1H
    POP 0D0H
    POP 0A8H
    ERET
HostIntervalNonzero:
    MOV R11,#8
HostIntervalStop:
    MOV R14,R11
HostIntervalReport:
    MOV R11,R14
    ECALL StartupForegroundCheckpoint?
    SJMP HostIntervalReport
END
