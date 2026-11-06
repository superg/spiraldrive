; Addressless register-ABI and atomic boundaries retained in assembly.
$CASE
NAME ARITHMETIC_BOUNDARIES

RUNTIME_HARDWARE_INITIALIZATION_ENTRY EQU 0FF2000H
RUNTIME_FLASH_IDENTIFICATION_ENTRY EQU 0FF5000H

PUBLIC InitializeDriveControlSequenceReturningAccumulator?
PUBLIC ClearRxdatBit3Atomic?
PUBLIC SetRxdatBit3Atomic?
PUBLIC SetBit0719Atomic?
PUBLIC ClearBit0719Atomic?
PUBLIC SetBit070FAtomic?
PUBLIC ClearBit070FAtomic?
PUBLIC CaptureInheritedR15ForAtipDecode?
PUBLIC CaptureInheritedR15ForMeasurementEvaluation?
PUBLIC EvaluateControllerMeasurementSeriesForCurrentModeWithFallback?
PUBLIC CaptureInheritedR15ForD8D9MeasurementSeries?
PUBLIC CaptureInheritedR15ForWorkingTrackProbeState?
PUBLIC CaptureInheritedR15ForIndexedStateGroup?
PUBLIC CaptureInheritedR15ForResetAndAcquireRawReadRange?
PUBLIC CaptureInheritedR15ForResolveDriveControlState?
PUBLIC CaptureInheritedR7ForActiveControllerRecordBoundaries?
PUBLIC CaptureR3ForFourByteBitConsensusState?
PUBLIC CaptureR3ForOneHotByteMapping?
PUBLIC PublishR7ForOneHotByteMapping?
PUBLIC CaptureInheritedDR0ForReadTrackInformation?
PUBLIC CaptureInheritedDR12ForReadTrackNextWritable?
PUBLIC CaptureInheritedBForReadTrackInformation?
PUBLIC CaptureInheritedBForCloseTrackSession?
PUBLIC CaptureR7ForControllerBufferPageCopy?
PUBLIC CaptureR6ForControllerBufferPageCopy?
PUBLIC CaptureDR0ForControllerBufferPageCopy?
PUBLIC CaptureRawSectorToControllerBufferAndWaitForC?
PUBLIC CaptureBForRawReadHostTransferDescriptorCompletion?
PUBLIC CaptureDR4ForRawReadHostTransferDescriptorCompletion?
PUBLIC CaptureWR2ForRawReadHostTransferDescriptorCompletion?
PUBLIC CaptureControllerProfileSelectorInputs?
PUBLIC CaptureInheritedR9ForControllerProfileSelector?
PUBLIC CaptureInheritedR15ForSecondaryControllerProfileUpdate?
PUBLIC SelectControllerProfileForOperatingCodeFromC?
PUBLIC CaptureInheritedR15ForMediaState5ProfileUpdate?
PUBLIC CaptureInheritedR15ForVendorF3MeasurementResponse?
PUBLIC CaptureInheritedR15ForPrimaryRawReadPipelineStep?
PUBLIC CallPrimaryRawReadPipelineStepWithDescriptorPointer?
PUBLIC CaptureInheritedDR12ForVendorF5MediaStrategyResponse?
PUBLIC PollReadTrackInformationRawDescriptorFlagsFromC?
PUBLIC CaptureInheritedR7ForRepairTrack?
PUBLIC CaptureInheritedDR12ForProgressBucket?
PUBLIC CaptureControllerReadPathOperatingSampleAtomic?
PUBLIC ResetAndAcquireRawReadRangeForLeadInOut?
PUBLIC ResetAndAcquireRawReadRangeForBlankFormat?
PUBLIC ProgramVendorF3CalibrationHeaderPair?
PUBLIC CaptureInheritedDR12ForEventBit7Transfer?
PUBLIC RestoreInheritedDR12ForEventBit7Transfer?
PUBLIC CaptureInheritedDR12ForControllerRecordWorkspacePageCopy?
PUBLIC RestoreInheritedDR12ForControllerRecordWorkspacePageCopy?
PUBLIC CaptureInheritedR14ForAlternateRawReadStep?
PUBLIC ResetFirmwareStackAndEnterMainContinuation?
PUBLIC RestartFirmwareAfterInterrupt?
PUBLIC InvokeRuntimeHardwareInitializationEntry?
PUBLIC RunStartupMemoryProbe20_36?
PUBLIC RunStartupMemoryProbe300_540?
PUBLIC InvokeRuntimeFlashIdentificationLoader?
PUBLIC CaptureInheritedDR0ForControllerStrobe?
PUBLIC CaptureInheritedDR0ForControllerF8Wait?
PUBLIC WaitControllerPort80F8Bit7ClearWithoutTimeout?
PUBLIC WaitControllerPort80F8Bit7ClearZeroFalseFalse?
PUBLIC WaitControllerPort80F8Bit7Clear500AndYield?
PUBLIC WaitControllerPort80F8Bit7Clear10AndYield?
PUBLIC WaitControllerPort80F8Bit7Clear2000AndYield?
PUBLIC InvokeRuntimeMainFlashProgrammerOperation3?
PUBLIC InvokeRuntimeMainFlashProgrammerOperation5?
PUBLIC SquareRootFloat32ViaFloat64?
PUBLIC StoreR3AtSegmentRelativeE789PlusSelf?
PUBLIC ReadControllerBufferByteAtomic?
PUBLIC CopyWordRangeAtOneMiBOffsets?
PUBLIC ReadSfrD1Bit5ForMsfAdjustment?
PUBLIC SetSfrA9Bit2Extended?
PUBLIC ClearSfrA9Bit2Extended?

ACC DATA 0E0H
INTERRUPT_ENABLE_REGISTER DATA 0A8H
INTERRUPT_CONTROL_REGISTER DATA 0A9H
GLOBAL_INTERRUPT_ENABLE BIT 0AFH
EXTRN ECODE (WaitControllerPort80F8Bit7Clear?, ConvertFloat32ToFloat64?, SquareRootFloat64?, ConvertFloat64ToFloat32?)
EXTRN ECODE (UpdateAndProgramDriveConfigurationRange?)
EXTRN ECODE (CaptureRawSectorToControllerBufferAndWait?)
EXTRN ECODE (PollReadTrackInformationRawDescriptorFlags?, SelectControllerProfileForOperatingCode?, EvaluateControllerMeasurementSeriesForCurrentMode?)
EXTRN ECODE (ProcessPrimaryRawReadPipelineStep?)
EXTRN ECODE (InitializeDriveControlSequence?)
EXTRN ECODE (InitializeAndEnterFirmwareMainAfterStackReset?)
EXTRN ECODE (RestartFirmware?)
EXTRN ECODE (ResetAndAcquireRawReadRange?)


ARITHMETIC_BOUNDARIES SEGMENT ECODE
RSEG ARITHMETIC_BOUNDARIES

; RAM:fee780.  The byte value and index arrive in inherited R3; clearing
; R0..R2 forms the 24-bit index in DR0 before the signed displacement.
StoreR3AtSegmentRelativeE789PlusSelf?:
    SUB R2,R2
    SUB WR0,WR0
    MOV @DR0-01877H,R3
    RET

; RAM:feea1c.  Preserve both interrupt SFR bytes across the controller-window
; read while retaining the canonical disabled-interrupt access window.
ReadControllerBufferByteAtomic?:
    MOV R8,INTERRUPT_ENABLE_REGISTER
    MOV R9,INTERRUPT_CONTROL_REGISTER
    CLR GLOBAL_INTERRUPT_ENABLE
    ADD WR4,#010H
    MOV R11,@DR4
    MOV INTERRUPT_ENABLE_REGISTER,R8
    MOV INTERRUPT_CONTROL_REGISTER,R9
    ERET

; RAM:fef131.  The C251 private stack supplies the 32-bit word count at
; SPX-6; this ABI and its one-bit byte-offset scaling are not expressible as
; an ordinary C function signature.
CopyWordRangeAtOneMiBOffsets?:
    ADD WR4,#010H
    ADD WR0,#010H
    SUB DR28,DR28
    SJMP CopyWordRangeAtOneMiBOffsetsTest
CopyWordRangeAtOneMiBOffsetsBody:
    MOV A,#01H
    MOV DR24,DR28
CopyWordRangeAtOneMiBOffsetsSourceScale:
    ADD DR24,DR24
    DEC A
    JNE CopyWordRangeAtOneMiBOffsetsSourceScale
    MOV DR8,DR4
    ADD DR8,DR24
    MOV WR26,@DR8
    MOV A,#01H
    MOV DR20,DR28
CopyWordRangeAtOneMiBOffsetsDestinationScale:
    ADD DR20,DR20
    DEC A
    JNE CopyWordRangeAtOneMiBOffsetsDestinationScale
    MOV DR8,DR0
    ADD DR8,DR20
    MOV @DR8,WR26
    INC DR28,#01H
CopyWordRangeAtOneMiBOffsetsTest:
    DB 069H,05FH,0FFH,0FCH ; MOV AB,@SPX-04H
    MOV WR8,@SPX-06H
    CMP DR28,DR8
    JC CopyWordRangeAtOneMiBOffsetsBody
    ADD SPX,#00H
    ERET

; Addressless atomic SFR:A9.2 setter. The legacy bit address 0AAH selects
; SFR:A8.2, so this operation requires an extended bit operand.
SetSfrA9Bit2Extended?:
    SETB 0A9H.2
    ERET

ClearSfrA9Bit2Extended?:
    CLR 0A9H.2
    ERET

; Addressless C-facing reader for the 80251 extended PSW1.5 bit operand used
; by RAM:fee841.  Reading the whole byte through the classic SFR form is not
; equivalent in S251 source mode.
ReadSfrD1Bit5ForMsfAdjustment?:
    CLR A
    DB 0A9H,025H,0D1H,001H ; JB SFR:D1.5,+1
    ERET
    INC A
    ERET

; First-operation capture for RAM:fccd59's no-threshold-match edge.
CaptureInheritedR7ForActiveControllerRecordBoundaries?:
    MOV A,R7
    ERET

; Addressless C-facing boundary for the extended RXDAT.3 atomic clear.
ClearRxdatBit3Atomic?:
    DB 0A9H,0C3H,0E3H,012H
    ERET

; Addressless C-facing boundary for the extended RXDAT.3 atomic set.
SetRxdatBit3Atomic?:
    DB 0A9H,0D3H,0E3H,012H
    ERET

; Addressless C-facing boundaries for the extended RXDAT.1 and SFR:E1.7
; atomic bit operations. Classic two-byte SETB/CLR bit encodings address a
; different architectural bit space on the 80251.
SetBit0719Atomic?:
    DB 0A9H,0D1H,0E3H
    ERET

ClearBit0719Atomic?:
    DB 0A9H,0C1H,0E3H
    ERET

SetBit070FAtomic?:
    DB 0A9H,0D7H,0E1H
    ERET

ClearBit070FAtomic?:
    DB 0A9H,0C7H,0E1H
    ERET

; Preserve RAM:fb8319's inherited R15 value across intervening C calls.
CaptureInheritedR15ForAtipDecode?:
    DB 0CAH,0F8H ; PUSH R15 (A251 does not accept the extended byte-register spelling)
    POP ACC
    ERET

; Addressless C-facing adapter for RAM:f95c1c's inherited R15 fallback.
CaptureInheritedR15ForMeasurementEvaluation?:
    DB 0CAH,0F8H ; PUSH R15
    POP ACC
    ERET

; Addressless C-facing adapter: ACC fallback -> inherited R15 near evaluator.
EvaluateControllerMeasurementSeriesForCurrentModeWithFallback?:
        MOV R15,ACC
        ECALL EvaluateControllerMeasurementSeriesForCurrentMode?
        ERET
; Addressless C-facing adapter for RAM:fa78fd's unsupported-code fallback.
CaptureInheritedR15ForD8D9MeasurementSeries?:
    DB 0CAH,0F8H ; PUSH R15
    POP ACC
    ERET

; Addressless entry capture for RAM:fee5aa's explicit R3 second argument.
CaptureR3ForFourByteBitConsensusState?:
    MOV A,R3
    ERET

; Addressless near-entry/result adapters for RAM:fee617's R3/R7 ABI.
CaptureR3ForOneHotByteMapping?:
    MOV A,R3
    ERET

PublishR7ForOneHotByteMapping?:
    MOV R7,A
    ERET

; Addressless C-facing adapter for RAM:faeef2's inherited DR0 output pointer.
CaptureInheritedDR0ForReadTrackInformation?:
    PUSH DR0
    POP DR4
    ERET

; Addressless entry capture for RAM:faeffa's unrecognized-type fallback.
CaptureInheritedDR12ForReadTrackNextWritable?:
    PUSH DR12
    POP DR4
    ERET

; Addressless entry capture for RAM:fae689's inherited-B loader paths.
CaptureInheritedBForReadTrackInformation?:
    DB 0A5H,0EAH ; MOV ACC,B (A251 lacks the extended register spelling)
    ERET

; Addressless entry capture for RAM:fb4a22's inherited-B track-loader call.
CaptureInheritedBForCloseTrackSession?:
    DB 0A5H,0EAH ; MOV ACC,B
    ERET

; Addressless native-entry captures for RAM:fcd87e.
CaptureR7ForControllerBufferPageCopy?:
    MOV A,R7
    ERET

CaptureR6ForControllerBufferPageCopy?:
    MOV A,R6
    ERET

CaptureDR0ForControllerBufferPageCopy?:
    PUSH DR0
    POP DR4
    ERET

; Addressless bridge for leaf_workers.c's static forwarding ABI.  Keeping the
; byte argument first makes C251 place it in ACC and the following huge address
; in DR4, which is already the canonical RAM:feb1ed entry contract.  The former
; address-first bridge lost the address low byte because ACC aliases R11/DR8.
CaptureRawSectorToControllerBufferAndWaitForC?:
    ECALL CaptureRawSectorToControllerBufferAndWait?
    ERET

; Addressless entry captures for RAM:fde8d8. Capture order in C is B, DR4,
; then WR2 because returning WR2 through WR6 would overwrite half of DR4.
CaptureBForRawReadHostTransferDescriptorCompletion?:
    DB 0A5H,0EAH ; MOV ACC,B (A251 lacks the extended register spelling)
    ERET

CaptureDR4ForRawReadHostTransferDescriptorCompletion?:
    ERET

CaptureWR2ForRawReadHostTransferDescriptorCompletion?:
    PUSH WR2
    POP WR6
    ERET

; Addressless entry capture for RAM:f9de74. Return inherited R15 in the high
; byte of WR6 and its explicit ACC coordinate in the low byte.
CaptureControllerProfileSelectorInputs?:
    MOV R7,A
    MOV R6,R15
    ERET

; Addressless near-entry capture for RAM:f9dec5's inherited auxiliary record
; index. The C worker invokes this before any other helper call.
CaptureInheritedR9ForControllerProfileSelector?:
    MOV ACC,R9
    ERET

; Addressless entry capture for RAM:f9de23's inherited low-speed selector.
CaptureInheritedR15ForSecondaryControllerProfileUpdate?:
    DB 0CAH,0F8H ; PUSH R15
    POP ACC
    ERET

; Addressless C-facing wrapper for RAM:f9de23. C251 supplies the coordinate in
; ACC and the captured inherited selector in R7; preserve the caller's R15.
SelectControllerProfileForOperatingCodeFromC?:
    DB 0CAH,0F8H ; PUSH R15
    MOV R15,R7
    ECALL SelectControllerProfileForOperatingCode?
    DB 0DAH,0F8H ; POP R15
    ERET

; Addressless ERET-class reader for RAM:f9de23's direct BITS:0038 gate.
CaptureInheritedR15ForMediaState5ProfileUpdate?:
    DB 0CAH,0F8H ; PUSH R15
    POP ACC
    ERET

; Addressless entry capture for RAM:fbe744's unsupported-selector transfer cap.
CaptureInheritedR15ForVendorF3MeasurementResponse?:
    DB 0CAH,0F8H ; PUSH R15
    POP ACC
    ERET

; Addressless entry capture for RAM:f9019b's inherited retry counter.
CaptureInheritedR15ForPrimaryRawReadPipelineStep?:
    MOV ACC,R15
    ERET

; Addressless near adapter preserving RAM:f90000's WR14=0x0f04 call state.
CallPrimaryRawReadPipelineStepWithDescriptorPointer?:
    MOV WR14,#00F04H
    ECALL ProcessPrimaryRawReadPipelineStep?
    ERET

; Addressless entry capture for RAM:fbeb4c's inherited record-table pointer.
CaptureInheritedDR12ForVendorF5MediaStrategyResponse?:
    MOV DR4,DR12
    ERET

; Addressless C-facing adapter for RAM:faee30's explicit local output byte.
PollReadTrackInformationRawDescriptorFlagsFromC?:
    MOV DR0,DR4
    ECALL PollReadTrackInformationRawDescriptorFlags?
    ERET

; Preserve RAM:fafe24's inherited R7 value across intervening C calls.
CaptureInheritedR7ForRepairTrack?:
    MOV A,R7
    ERET

; Addressless inherited-register adapter for RAM:fcee18's C reconstruction.
CaptureInheritedDR12ForProgressBucket?:
    MOV DR4,DR12
    ERET
; C-facing adapter for RAM:fe516f's R15/ACC argument tuple.
ResetAndAcquireRawReadRangeForLeadInOut?:
    MOV R15,ACC
    MOV A,#02H
    ECALL ResetAndAcquireRawReadRange?
    ERET

; C-facing adapter for RAM:fe7e4b. C251 supplies start in DR4, count in DR0
; and Q-metadata mode in ACC. The canonical caller leaves the start's high
; byte in R15 through its DR12-to-DR4 move, so reproduce that hidden input.
ResetAndAcquireRawReadRangeForBlankFormat?:
    MOV R15,R7
    ECALL ResetAndAcquireRawReadRange?
    ERET

; C-facing serialization adapter for RAM:f8b76f. C251 supplies the two-byte
; big-endian header in WR6; RAM:fd60f2 requires a far source pointer, logical
; destination offset in DR4 and count on the native stack.
ProgramVendorF3CalibrationHeaderPair?:
    INC SPX,#02H
    MOV @SPX-01H,WR6
    MOV WR6,#02H
    PUSH WR6
    MOV DR0,SPX
    ADD WR2,#0FFFDH
    XRL WR0,WR0
    MOV DR4,#020H
    ECALL UpdateAndProgramDriveConfigurationRange?
    DEC SPX,#02H
    DEC SPX,#02H
    ERET

; Reconstruction helper; no direct Ghidra function address.
CaptureInheritedDR12ForEventBit7Transfer?:
    MOV DR4,DR12
    ERET

RestoreInheritedDR12ForEventBit7Transfer?:
    MOV DR12,DR4
    ERET

; Addressless adapters for RAM:faa80c. The original body saves inherited DR12
; before selecting a bank and restores it after the controller-window copy.
CaptureInheritedDR12ForControllerRecordWorkspacePageCopy?:
    MOV DR4,DR12
    ERET

RestoreInheritedDR12ForControllerRecordWorkspacePageCopy?:
    MOV DR12,DR4
    ERET

; Addressless entry capture for RAM:fc46c4's unrecognized-status fallback.
CaptureInheritedR15ForWorkingTrackProbeState?:
    MOV ACC,R15
    ERET

; Addressless entry capture for RAM:fd7bf3's unsupported-layout fallback.
CaptureInheritedR15ForIndexedStateGroup?:
    MOV ACC,R15
    ERET

; Addressless entry capture for RAM:fe825b's nonzero-mode preserve-main flag.
CaptureInheritedR15ForResetAndAcquireRawReadRange?:
    MOV ACC,R15
    ERET

; Addressless entry capture for RAM:fb095f's state-two operating-code value.
CaptureInheritedR15ForResolveDriveControlState?:
    MOV ACC,R15
    ERET

; RAM:fde93a inherited-register boundary.
CaptureInheritedR14ForAlternateRawReadStep?:
    MOV ACC,R14
    ERET

CaptureInheritedDR0ForControllerStrobe?:
    MOV DR4,DR0
    ERET

; Addressless C adapter for RAM:f8e2b0's inherited signed DR0 timeout.
CaptureInheritedDR0ForControllerF8Wait?:
    MOV DR4,DR0
    ERET

; Binary32 operand/result around the native DR0:DR4 binary64 square-root ABI.
SquareRootFloat32ViaFloat64?:
    ECALL ConvertFloat32ToFloat64?
    ECALL SquareRootFloat64?
    ECALL ConvertFloat64ToFloat32?
    ERET

; Reconstruction helper for callers that use DR0=0, R7=0, ACC=1.
WaitControllerPort80F8Bit7ClearWithoutTimeout?:
    SUB DR0,DR0
    XRL R7,R7
    MOV A,#01H
    ECALL WaitControllerPort80F8Bit7Clear?
    ERET

; Reconstruction helper for RAM:fb1431's DR0=0, R7=0, ACC=0 tuple.
WaitControllerPort80F8Bit7ClearZeroFalseFalse?:
    SUB DR0,DR0
    XRL R7,R7
    CLR A
    ECALL WaitControllerPort80F8Bit7Clear?
    ERET

; Reconstruction helper for callers that use DR0=500, R7=1, ACC=1.
WaitControllerPort80F8Bit7Clear500AndYield?:
    MOV DR0,#01F4H
    MOV R7,#01H
    MOV A,#01H
    ECALL WaitControllerPort80F8Bit7Clear?
    ERET

; Reconstruction helper for callers that use DR0=10, R7=1, ACC=1.
WaitControllerPort80F8Bit7Clear10AndYield?:
    MOV DR0,#000AH
    MOV R7,#01H
    MOV A,#01H
    ECALL WaitControllerPort80F8Bit7Clear?
    ERET

; Reconstruction helper for callers that use DR0=2000, R7=1, ACC=1.
WaitControllerPort80F8Bit7Clear2000AndYield?:
    MOV DR0,#07D0H
    MOV R7,#01H
    MOV A,#01H
    ECALL WaitControllerPort80F8Bit7Clear?
    ERET

; Reconstruction helper; no direct Ghidra function address. Preserve the
; interrupt-protected controller sample and shadow/port write order used by
; RAM:fb9624. Return the signed sample in WR6 for C251.
CaptureControllerReadPathOperatingSampleAtomic?:
    DB 07EH,071H,0A8H,07EH,0A1H,0A9H,0C2H,0AFH
    DB 07EH,0B3H,00FH,0CAH,044H,010H,090H,080H,002H,0F0H
    DB 090H,080H,072H,0E0H,01AH,0DBH,07DH,02DH,07CH,045H
    DB 06CH,055H,07DH,0D2H,090H,080H,071H,0E0H,01AH,0EBH
    DB 04DH,0DEH,07EH,0B3H,00FH,0CAH,054H,0E7H,07AH,0B3H
    DB 00FH,0CAH,090H,080H,002H,0F0H,07AH,071H,0A8H,07AH
    DB 0A1H,0A9H
    MOV WR6,WR26
    ERET

; Addressless C adapter for the runtime loader's operation-three register tuple.
InvokeRuntimeMainFlashProgrammerOperation3?:
    DB 09FH,011H,07EH,004H,000H,003H,06DH,011H
    DB 07EH,02FH,013H,012H,099H,028H,0AAH

; Addressless C adapter for the runtime loader's operation-five register tuple.
InvokeRuntimeMainFlashProgrammerOperation5?:
    SUB DR4,DR4
    MOV WR0,#0005H
    XRL WR2,WR2
    MOV DR8,EDATA 1312H
    ECALL @DR8
    ERET

; Preserve the accumulator result of the drive-control initialization call.
InitializeDriveControlSequenceReturningAccumulator?:
    ECALL InitializeDriveControlSequence?
    ERET

ResetFirmwareStackAndEnterMainContinuation?:
    MOV SPX,#0FFH
    EJMP InitializeAndEnterFirmwareMainAfterStackReset?

; Addressless nonreturning interrupt-exit adapter. An ordinary call to
; RestartFirmware abandons the ISR without releasing its in-service latch.
; Build an INTR=1 frame: PSW1, PC bank, PC low, PC high. ECALL supplies
; bank/high/low; swap its last two bytes before RETI. The continuation enters
; the C restart worker, which resets SPX, so no abandoned ISR frame is read.
; Keep EA clear until initialization deliberately enables interrupt sources.
RestartFirmwareAfterInterrupt?:
    CLR GLOBAL_INTERRUPT_ENABLE
    PUSH #00H
    ECALL ReleaseHostInterruptForRestart
    EJMP RestartFirmware?
ReleaseHostInterruptForRestart:
    POP WR0
    PUSH R1
    PUSH R0
    RETI

InvokeRuntimeHardwareInitializationEntry?:
    ECALL RUNTIME_HARDWARE_INITIALIZATION_ENTRY
    ERET

; Addressless adapters for RAM:fdc43b. The RAM:ff1f00 startup probe uses
; function-local DR4/WR2 tuples, while the flash loader entry is published in
; RAM:00130e before the indirect call.
RunStartupMemoryProbe20_36?:
    MOV DR4,#020H
    MOV WR2,#036H
    MOV WR10,#01F00H
    MOV WR8,#0FFH
    ECALL @DR8
    ERET

RunStartupMemoryProbe300_540?:
    MOV DR4,#0300H
    MOV WR2,#0540H
    MOV WR10,#01F00H
    MOV WR8,#0FFH
    ECALL @DR8
    ERET

InvokeRuntimeFlashIdentificationLoader?:
    MOV DR8,EDATA 130EH
    ECALL @DR8
    ERET

END
