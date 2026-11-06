; Addressless terminal diagnostic at media-state entry (stock RAM:f84f6a).
; Freeze RAM:000c32 once with interrupts masked. One repeated count:
; 1 = idle 01; 2 = ready 10; 3 = packet ready 11; 4 = status 04;
; 5 = zero; 6 = any other byte. These are states, not command completions.
; No controller reads, timer or compiler XDATA. R14 survives the LED adapter.
$CASE
NAME STARTUP_HOST_PACKET_DIAGNOSTICS
PUBLIC StartupHostGateCheckpoint?
EXTRN ECODE (StartupForegroundCheckpoint?)

STARTUP_HOST_PACKET_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_HOST_PACKET_DIAGNOSTICS

StartupHostGateCheckpoint?:
    CLR 0AFH
    MOV R13,EDATA 0C32H
    MOV R14,#1
    CMP R13,#1
    JE HostPacketReport
    INC R14,#1
    CMP R13,#010H
    JE HostPacketReport
    INC R14,#1
    CMP R13,#011H
    JE HostPacketReport
    INC R14,#1
    CMP R13,#4
    JE HostPacketReport
    INC R14,#1
    CMP R13,#0
    JE HostPacketReport
    INC R14,#1
HostPacketReport:
    MOV R11,R14
    ECALL StartupForegroundCheckpoint?
    SJMP HostPacketReport
END
