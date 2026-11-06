; Addressless diagnostic-only terminal snapshot, reached at media-state entry.
; No prior frames, timers or compiler XDATA. Snapshot IE before clearing EA.
; Count = 1 + 4*(EA enabled) + 2*(RAM:faeb nonzero)
;           + 1*(RAM:0c32 is not the idle packet state 1).
; faeb may be preset by FirmwareMainLoop; nonzero alone does not prove an IRQ.
; The preserved R14 holds the frozen count across each complete LED frame.
$CASE
NAME STARTUP_HOST_GATE_DIAGNOSTICS
PUBLIC StartupHostGateCheckpoint?
EXTRN ECODE (StartupForegroundCheckpoint?)

STARTUP_HOST_GATE_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_HOST_GATE_DIAGNOSTICS

StartupHostGateCheckpoint?:
    MOV R10,0A8H
    CLR 0AFH
    MOV R14,#1
    MOV R11,R10
    JNB 0E7H,HostGateInterruptsCaptured
    ADD R14,#4
HostGateInterruptsCaptured:
    MOV R11,EDATA 0FAEBH
    ORL R11,R11
    JE HostGateLatchCaptured
    ADD R14,#2
HostGateLatchCaptured:
    MOV R11,EDATA 0C32H
    CMP R11,#1
    JE HostGateReport
    INC R14,#1
HostGateReport:
    MOV R11,R14
    ECALL StartupForegroundCheckpoint?
    SJMP HostGateReport
END
