; Native interrupt-enable operations.
$CASE
NAME INTERRUPT_CONTROL

PUBLIC DisableGlobalInterrupts?
PUBLIC EnableFirmwareInterruptSources?

GLOBAL_INTERRUPT_ENABLE BIT 0AFH
CONTROLLER_INTERRUPT_ENABLE BIT 0AEH

; Share the established native-boundary segment so these ordinary helpers are
; not packed into the fixed hardware-vector window at RAM:f800xx.
ARITHMETIC_BOUNDARIES SEGMENT ECODE
RSEG ARITHMETIC_BOUNDARIES

; RAM:fee80e
DisableGlobalInterrupts?:
    CLR GLOBAL_INTERRUPT_ENABLE
    ERET

; RAM:fee811
EnableFirmwareInterruptSources?:
    SETB GLOBAL_INTERRUPT_ENABLE
    SETB CONTROLLER_INTERRUPT_ENABLE
    SETB 0A9H.2
    ERET

END
