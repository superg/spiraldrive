; Addressless diagnostic-only LED adapter. C251 u8 parameter arrives in R11.
; Delays use registers and the current stack, with no timer or XDATA dependency.
; Preserve registers, flags and interrupt enables; pause interrupts only while
; emitting a frame so firmware LED updates cannot overwrite the pulses.
; Post-diag21 readability: 32 outer passes instead of 8 makes each pulse,
; dark interval and frame pause approximately four times longer at one clock.
; Physical seconds are uncalibrated. Keep future reports countable by eye.
$CASE
NAME STARTUP_FOREGROUND_DIAGNOSTICS
PUBLIC StartupForegroundCheckpoint?

STARTUP_FOREGROUND_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_FOREGROUND_DIAGNOSTICS

StartupForegroundCheckpoint?:
    PUSH 0A8H
    CLR 0AFH
    PUSH 0D0H
    PUSH 0D1H
    PUSH DR4
    PUSH DR8
    PUSH DR12
    MOV R15,R11
    CLR 0B0H
    SETB 0B1H
    ECALL ForegroundDelay
    ECALL ForegroundDelay
    ECALL ForegroundDelay
    SETB 0B0H
    ECALL ForegroundDelay
    ECALL ForegroundDelay
    CMP R15,#0
    JE ForegroundEndFrame
ForegroundGreenPulse:
    CLR 0B1H
    ECALL ForegroundDelay
    SETB 0B1H
    ECALL ForegroundDelay
    DEC R15,#1
    JNE ForegroundGreenPulse
ForegroundEndFrame:
    ECALL ForegroundDelay
    ECALL ForegroundDelay
    ECALL ForegroundDelay
    ECALL ForegroundDelay
    POP DR12
    POP DR8
    POP DR4
    POP 0D1H
    POP 0D0H
    POP 0A8H
    ERET

ForegroundDelay:
    MOV R7,#32
ForegroundDelayOuter:
    MOV WR4,#0FFFFH
ForegroundDelayInner:
    DEC WR4,#1
    JNE ForegroundDelayInner
    DEC R7,#1
    JNE ForegroundDelayOuter
    ERET
END
