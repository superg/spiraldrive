; Addressless diagnostic: localize a zero host-packet state without persistent
; diagnostic RAM or delays on passing checks. u8 point arrives in R11.
; 1 always reports the host-init short branch; 2..6 report only when c32==0;
; 7 is final media entry: zero reports7, nonzero reports8.
; Silent returns restore IE, PSW/PSW1 and DR8 (including incoming R11).
$CASE
NAME STARTUP_HOST_ZERO_DIAGNOSTICS
PUBLIC StartupHostZeroCheckpoint?
EXTRN ECODE (StartupForegroundCheckpoint?)

STARTUP_HOST_ZERO_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_HOST_ZERO_DIAGNOSTICS

StartupHostZeroCheckpoint?:
    PUSH 0A8H
    CLR 0AFH
    PUSH 0D0H
    PUSH 0D1H
    PUSH DR8
    MOV R10,EDATA 0C32H
    CMP R11,#1
    JE HostZeroStop
    ORL R10,R10
    JE HostZeroStop
    CMP R11,#7
    JE HostZeroAbsent
    POP DR8
    POP 0D1H
    POP 0D0H
    POP 0A8H
    ERET
HostZeroAbsent:
    MOV R11,#8
HostZeroStop:
    MOV R14,R11
HostZeroReport:
    MOV R11,R14
    ECALL StartupForegroundCheckpoint?
    SJMP HostZeroReport
END
