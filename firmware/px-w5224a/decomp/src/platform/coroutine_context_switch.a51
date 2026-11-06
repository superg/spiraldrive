; Native coroutine stack-switch primitives.
;
; C251 cannot express MOV to/from SPX followed by ERET without introducing
; interrupt masking, temporary registers, and ordinary stack accesses.  These
; functions therefore retain the firmware's atomic architectural boundary.
$CASE
NAME COROUTINE_CONTEXT_SWITCH

PUBLIC ResumeCoroutine0?
PUBLIC ResumeCoroutine1?
PUBLIC ResumeCoroutine2?
PUBLIC ResumeCoroutine3?
PUBLIC YieldCoroutine0?
PUBLIC YieldCoroutine1?
PUBLIC YieldCoroutine2?
PUBLIC YieldCoroutine3?
PUBLIC RestartCoroutine1AndYieldImpl?

COROUTINE_CONTEXT_SWITCH SEGMENT ECODE
RSEG COROUTINE_CONTEXT_SWITCH

; RAM:fee4fd
ResumeCoroutine0?:
    MOV EDATA 115AH,SPX
    MOV SPX,EDATA 115EH
    POP DR12
    ERET

; RAM:fee508
ResumeCoroutine1?:
    MOV EDATA 115AH,SPX
    MOV SPX,EDATA 1162H
    POP DR12
    ERET

; RAM:fee513
ResumeCoroutine2?:
    MOV EDATA 115AH,SPX
    MOV SPX,EDATA 1166H
    POP DR12
    ERET

; RAM:fee51e
ResumeCoroutine3?:
    MOV EDATA 115AH,SPX
    MOV SPX,EDATA 116AH
    POP DR12
    ERET

; RAM:fee529
YieldCoroutine0?:
    PUSH DR12
    MOV EDATA 115EH,SPX
    MOV SPX,EDATA 115AH
    ERET

; RAM:fee534
YieldCoroutine1?:
    PUSH DR12
    MOV EDATA 1162H,SPX
    MOV SPX,EDATA 115AH
    ERET

; RAM:fee53f
YieldCoroutine2?:
    PUSH DR12
    MOV EDATA 1166H,SPX
    MOV SPX,EDATA 115AH
    ERET

; RAM:fee54a
YieldCoroutine3?:
    PUSH DR12
    MOV EDATA 116AH,SPX
    MOV SPX,EDATA 115AH
    ERET

; RAM:fee559
; RestartCoroutine1AndYield? moves the C argument from DR4 to DR0 before
; entering this non-returning native primitive.
RestartCoroutine1AndYieldImpl?:
    MOV WR4,#0500H
    MOV @WR4,WR0
    MOV @WR4+02H,WR2
    MOV DR0,#0507H
    MOV EDATA 1162H,DR0
    MOV SPX,EDATA 115AH
    ERET

END
