; Reconstruction helper; no direct Ghidra function address.
; C-facing preservation of the main-to-monitor configuration programming call.
; C251 passes and receives the one-byte Boolean in ACC, matching RAM:fd6493.
$CASE
NAME CONFIGURATION_RUNTIME

CONFIGURATION_FLASH_PROGRAMMER_ENTRY EQU 0FF4500H

PUBLIC InvokeRecoveryConfigurationProgram?

CONFIGURATION_RUNTIME SEGMENT ECODE
RSEG CONFIGURATION_RUNTIME

InvokeRecoveryConfigurationProgram?:
    ECALL CONFIGURATION_FLASH_PROGRAMMER_ENTRY
    ERET

END
