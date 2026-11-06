; Diagnostic-only initial-entry tag and suspended C0 return-address classifier.
; No persistent scratch RAM. The handshake wrapper adds one four-byte frame.
; All reporting entries are terminal. Slow frame reporter is shared unchanged.
$CASE
NAME STARTUP_HOST_ROUTE_DIAGNOSTICS
PUBLIC StartupRouteInitialEntry?
PUBLIC StartupRouteOtherEntry?
PUBLIC StartupRouteHandshakeYield?
PUBLIC StartupRouteHandshakeResume?
EXTRN ECODE (StartupForegroundCheckpoint?, YieldCoroutine0?)

STARTUP_HOST_ROUTE_DIAGNOSTICS SEGMENT ECODE
RSEG STARTUP_HOST_ROUTE_DIAGNOSTICS

StartupRouteHandshakeYield?:
    ECALL YieldCoroutine0?
StartupRouteHandshakeResume?:
    ERET

StartupRouteInitialEntry?:
    CLR 0AFH
    MOV R14,#1
    SJMP RouteClassify
StartupRouteOtherEntry?:
    CLR 0AFH
    MOV R14,#4
RouteClassify:
    MOV R10,EDATA 0C32H
    ORL R10,R10
    JE RouteZero
    INC R14,#2
    SJMP RouteReport
RouteZero:
    MOV DR0,EDATA 115EH
    CMP WR0,#0
    JNE RouteInvalidStack
    CMP WR2,#0307H
    JC RouteInvalidStack
    CMP WR2,#0500H
    JNC RouteInvalidStack
    SUB DR0,#7
    MOV WR4,@DR0
    MOV WR6,@DR0+2
    ; Native ECALL's leading mode byte is not part of the 24-bit address.
    MOV R4,#0
    MOV WR8,#WORD2 StartupRouteHandshakeResume?
    MOV WR10,#WORD0 StartupRouteHandshakeResume?
    CMP WR4,WR8
    JNE RouteOtherYield
    CMP WR6,WR10
    JE RouteReport
RouteOtherYield:
    INC R14,#1
    SJMP RouteReport
RouteInvalidStack:
    MOV R14,#7
RouteReport:
    MOV R11,R14
    ECALL StartupForegroundCheckpoint?
    SJMP RouteReport
END
