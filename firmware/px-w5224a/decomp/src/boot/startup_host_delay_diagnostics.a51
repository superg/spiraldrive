; Diagnostic-only bound on the real DelayTimerUnits overflow polling.
; Register/stack counter, no persistent scratch RAM or synthetic timer flag.
; No IRQ masking on the normal wait path. Slow terminal frames are unchanged.
$CASE
NAME STARTUP_HOST_DELAY_DIAGNOSTICS
PUBLIC StartupDelayOverflowWait?
PUBLIC StartupHandshakeDelayCompleted?
PUBLIC StartupDelayDeadlineSnapshot?
EXTRN ECODE (StartupForegroundCheckpoint?)

STARTUP_HOST_DELAY_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_HOST_DELAY_DIAGNOSTICS

StartupDelayOverflowWait?:
    PUSH 0D0H
    PUSH 0D1H
    PUSH DR4
    PUSH DR8
    MOV R7,#32
DelayPollOuter:
    MOV WR4,#0FFFFH
DelayPoll:
    JB 091H.7,DelayReady
    DEC WR4,#1
    JNE DelayPoll
    DEC R7,#1
    JNE DelayPollOuter
    ; Freeze current interrupt enable and timer status at the poll deadline.
StartupDelayDeadlineSnapshot?:
    MOV R9,0A8H
    CLR 0AFH
    MOV R10,091H
    MOV R11,R10
    JB 0E7H,DelayLateOverflow
    MOV R14,#1
    JNB 0E6H,DelayReport
    MOV R14,#2
    MOV R11,R9
    JNB 0E7H,DelayReport
    MOV R14,#3
    SJMP DelayReport
DelayLateOverflow:
    MOV R14,#6
    SJMP DelayReport
DelayReady:
    POP DR8
    POP DR4
    POP 0D1H
    POP 0D0H
    ERET

StartupHandshakeDelayCompleted?:
    CLR 0AFH
    MOV R14,#4
DelayReport:
    MOV R11,R14
    ECALL StartupForegroundCheckpoint?
    SJMP DelayReport
END
