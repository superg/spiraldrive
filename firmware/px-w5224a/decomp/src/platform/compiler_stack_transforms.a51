; Native compiler register-save stack transforms.
$CASE
NAME COMPILER_STACK_TRANSFORMS

PUBLIC SaveDataRegistersAndDuplicateReturnAddress?
PUBLIC RestoreDataRegistersAndRelocateReturnAddress?

COMPILER_STACK_TRANSFORMS SEGMENT ECODE
RSEG COMPILER_STACK_TRANSFORMS

; RAM:fee571
SaveDataRegistersAndDuplicateReturnAddress?:
    PUSH DR0
    PUSH DR4
    PUSH DR8
    PUSH DR12
    PUSH DR16
    PUSH DR20
    PUSH DR24
    MOV WR24,@SPX-021H
    MOV WR26,@SPX-023H
    PUSH DR24
    RET

; RAM:fee58a
RestoreDataRegistersAndRelocateReturnAddress?:
    MOV WR24,@SPX
    MOV WR26,@SPX+02H
    DEC SPX,#04H
    MOV @SPX-01CH,WR24
    MOV @SPX-01EH,WR26
    POP DR24
    POP DR20
    POP DR16
    POP DR12
    POP DR8
    POP DR4
    POP DR0
    RET

END
