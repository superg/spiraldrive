#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
static u16 FirmwareReadBigEndianU16Near(u16 address);
#else
#define FirmwareReadBigEndianU16Near(address) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
static u32 FirmwareReadBigEndianU32(fw_addr_t address);
#else
#define FirmwareReadBigEndianU32(address) (*((volatile u32 huge *)(fw_addr_t)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
static u32 FirmwareReadBigEndianU32Near(u16 address);
#else
#define FirmwareReadBigEndianU32Near(address) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU16Near(u16 address, u16 value);
#else
#define FirmwareWriteBigEndianU16Near(address, value) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)) = (u16)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU32(fw_addr_t address, u32 value);
static void FirmwareWriteBigEndianU32Near(u16 address, u32 value);
#else
#define FirmwareWriteBigEndianU32(address, value) (*((volatile u32 huge *)(fw_addr_t)(address)) = (u32)(value))
#define FirmwareWriteBigEndianU32Near(address, value) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)) = (u32)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0547(0u);
#else
ClearBit0547Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0547((u8)(saved_ie >> 7));
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 8
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) & 0xfeu));
FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) & 0xfdu));
#else
ClearP3Bit0Atomic();
ClearP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 9
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRestartReadAcquisitionCoroutineForProducerWait(void);
#define RestartReadAcquisitionCoroutineForProducerWait ObserveRestartReadAcquisitionCoroutineForProducerWait
#else
extern u8 RestartReadAcquisitionCoroutineForProducerWait(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 10
#if defined(PLEXTOR_HOST_TEST)
extern u32 InterceptResetRawReadTransferState(void);
#define DispatchResetRawReadTransferState InterceptResetRawReadTransferState
#else
#define DispatchResetRawReadTransferState ResetRawReadTransferState
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 11
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveProcessMediaCoroutineState3UpdateMeasurement(void);
extern void ObserveProcessMediaCoroutineState3SetOperating(u8 enabled);
extern void ObserveProcessMediaCoroutineState3SelectOperatingCode(void);
extern s8 ObserveProcessMediaCoroutineState3AcquirePosition(void);
extern void ObserveProcessMediaCoroutineState3ServiceController(void);
extern u8 ObserveProcessMediaCoroutineState3CountdownExpired(void);
extern u8 ObserveProcessMediaCoroutineState3IndexedTransitions(void);
extern u8 ObserveProcessMediaCoroutineState3IndexedCompletion(void);
extern void ObserveProcessMediaCoroutineState3LoadTarget(void);
#define State3UpdateMeasurement() ObserveProcessMediaCoroutineState3UpdateMeasurement()
#define State3SetOperating(enabled) ObserveProcessMediaCoroutineState3SetOperating(enabled)
#define State3SelectOperatingCode() ObserveProcessMediaCoroutineState3SelectOperatingCode()
#define State3AcquirePosition() ObserveProcessMediaCoroutineState3AcquirePosition()
#define State3ServiceController() ObserveProcessMediaCoroutineState3ServiceController()
#define State3CountdownExpired() ObserveProcessMediaCoroutineState3CountdownExpired()
#define State3IndexedTransitions() ObserveProcessMediaCoroutineState3IndexedTransitions()
#define State3IndexedCompletion() ObserveProcessMediaCoroutineState3IndexedCompletion()
#define State3LoadTarget() ObserveProcessMediaCoroutineState3LoadTarget()
#else
#define State3UpdateMeasurement() UpdateReadAcquisitionMeasurementIfNeeded()
#define State3SetOperating(enabled) SetControllerOperatingEnableState(enabled)
#define State3SelectOperatingCode() SelectRequestedOperatingCodeForMediaState()
#define State3AcquirePosition() ((s8)AcquireDiscReadPosition())
#define State3ServiceController() ServiceMediaState3ControllerAndQCapture()
#define State3CountdownExpired() HasMediaStateCountdownExpired()
#define State3IndexedTransitions() ServiceMediaState3IndexedDriveTransitions()
#define State3IndexedCompletion() ProcessIndexedStateCompletion()
#define State3LoadTarget() LoadReadAcquisitionTargetForTrackNumber()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 12
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCopyControllerBufferPageToUpperBankedRegion(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker);
#define DispatchCopyControllerBufferPageToUpperBankedRegion ObserveCopyControllerBufferPageToUpperBankedRegion
#else
extern void CopyControllerBufferPageToUpperBankedRegionForC(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker);
#define DispatchCopyControllerBufferPageToUpperBankedRegion CopyControllerBufferPageToUpperBankedRegionForC
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 13
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveServiceMediaQPacketCaptureWithOperatingCodeBudget(void);
#define DispatchServiceMediaQPacketCaptureWithOperatingCodeBudget() ObserveServiceMediaQPacketCaptureWithOperatingCodeBudget()
#else
extern void PLEXTOR_NEAR_FUNCTION ServiceMediaQPacketCaptureWithOperatingCodeBudget(void);
#define DispatchServiceMediaQPacketCaptureWithOperatingCodeBudget() ServiceMediaQPacketCaptureWithOperatingCodeBudget()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 14
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerWord035FProgressUpdate(void);
extern void ObserveControllerParameterD1ModeBits(void);
extern void ObserveControllerPorts80D0D1D3Configuration(void);
#define UPDATE_CONTROLLER_WORD_035F_FOR_PORT_SETUP ObserveControllerWord035FProgressUpdate
#define APPLY_CONTROLLER_D1_FOR_PORT_SETUP ObserveControllerParameterD1ModeBits
#define CONFIGURE_CONTROLLER_D0_D1_D3_FOR_PORT_SETUP ObserveControllerPorts80D0D1D3Configuration
#else
#define UPDATE_CONTROLLER_WORD_035F_FOR_PORT_SETUP UpdateControllerWord035FFromProgressBucket
#define APPLY_CONTROLLER_D1_FOR_PORT_SETUP ApplyControllerParameterD1ModeBits
#define CONFIGURE_CONTROLLER_D0_D1_D3_FOR_PORT_SETUP ConfigureControllerPorts80D0D1D3ForCurrentState
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 15
#if defined(PLEXTOR_HOST_TEST)
extern u16 ObserveCalculateRadialModelDistanceForSpecialPositioning(void);
#define DispatchCalculateRadialModelDistanceForSpecialPositioning() ObserveCalculateRadialModelDistanceForSpecialPositioning()
#else
extern u16 PLEXTOR_NEAR_FUNCTION CalculateRadialModelDistanceToReadTarget(void);
#define DispatchCalculateRadialModelDistanceForSpecialPositioning() CalculateRadialModelDistanceToReadTarget()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 16
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveIssueReadAcquisitionPickupMove(void);
#define DispatchIssueReadAcquisitionPickupMove() ObserveIssueReadAcquisitionPickupMove()
#else
extern u8 PLEXTOR_NEAR_FUNCTION IssueReadAcquisitionPickupMove(void);
#define DispatchIssueReadAcquisitionPickupMove() IssueReadAcquisitionPickupMove()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 17
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidatePlextorPremiumDiscSignatureSector(void);
#define VALIDATE_PREMIUM_DISC_SIGNATURE() ObserveValidatePlextorPremiumDiscSignatureSector()
#else
extern u8 PLEXTOR_NEAR_FUNCTION ValidatePlextorPremiumDiscSignatureSector(void);
#define VALIDATE_PREMIUM_DISC_SIGNATURE() ValidatePlextorPremiumDiscSignatureSector()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 18
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveAcquirePlextorPremiumFirmwarePayloadFromDisc(void);
#define ACQUIRE_PREMIUM_FIRMWARE_PAYLOAD() ObserveAcquirePlextorPremiumFirmwarePayloadFromDisc()
#else
extern u8 PLEXTOR_NEAR_FUNCTION AcquirePlextorPremiumFirmwarePayloadFromDisc(void);
#define ACQUIRE_PREMIUM_FIRMWARE_PAYLOAD() AcquirePlextorPremiumFirmwarePayloadFromDisc()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 19
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateAndInvokeRuntimeMainFirmwareProgrammer(void);
#define INVOKE_RUNTIME_MAIN_PROGRAMMER_FOR_PREMIUM_UPDATE() ObserveValidateAndInvokeRuntimeMainFirmwareProgrammer()
#else
extern u8 PLEXTOR_NEAR_FUNCTION ValidateAndInvokeRuntimeMainFirmwareProgrammer(void);
#define INVOKE_RUNTIME_MAIN_PROGRAMMER_FOR_PREMIUM_UPDATE() ValidateAndInvokeRuntimeMainFirmwareProgrammer()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 20
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRunRecoveryWaitAndRestartForPremiumUpdate(void);
#define RUN_PREMIUM_UPDATE_RECOVERY() ObserveRunRecoveryWaitAndRestartForPremiumUpdate()
#else
extern void PLEXTOR_NEAR_FUNCTION RunRecoveryWaitAndRestartIfNeeded(void);
#define RUN_PREMIUM_UPDATE_RECOVERY() RunRecoveryWaitAndRestartIfNeeded()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 21
#if defined(PLEXTOR_HOST_TEST)
extern void DispatchClearControllerWord0252ForDriveControlInitialization(void);
#define CLEAR_CONTROLLER_WORD_0252_FOR_DRIVE_CONTROL_INITIALIZATION DispatchClearControllerWord0252ForDriveControlInitialization
#else
#define CLEAR_CONTROLLER_WORD_0252_FOR_DRIVE_CONTROL_INITIALIZATION ClearControllerWord0252AndResetChannel2
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 22
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetDriveControlStateToEight(void);
#define ResetDriveControlStateToEight ObserveResetDriveControlStateToEight
#else
extern void ResetDriveControlStateToEight(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 23
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveProbeControllerSignalAndInitializeDriveControl(void);
#define PROBE_CONTROLLER_SIGNAL_AND_INITIALIZE_DRIVE_CONTROL() ObserveProbeControllerSignalAndInitializeDriveControl()
#else
extern void PLEXTOR_NEAR_FUNCTION ProbeControllerSignalAndInitializeDriveControl(void);
#define PROBE_CONTROLLER_SIGNAL_AND_INITIALIZE_DRIVE_CONTROL() ProbeControllerSignalAndInitializeDriveControl()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 24
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawSectorHeaderAndTdiPayloadRangePopulation(fw_addr_t controller_buffer_address, u32 sector_count);
#define POPULATE_RAW_TDI_RANGE_FOR_ACQUIRE_WRAPPER ObserveRawSectorHeaderAndTdiPayloadRangePopulation
#else
#define POPULATE_RAW_TDI_RANGE_FOR_ACQUIRE_WRAPPER PopulateRawSectorHeaderAndTdiPayloadRange
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 25
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartFormat2RawSectorReadAtFramePosition(s32 frame_position);
#define START_PLAY_AUDIO_RAW_READ(position) ObserveStartFormat2RawSectorReadAtFramePosition(position)
#else
extern void StartFormat2RawSectorReadAtFramePosition(s32 frame_position);
#define START_PLAY_AUDIO_RAW_READ(position) StartFormat2RawSectorReadAtFramePosition(position)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 26
#if defined(PLEXTOR_HOST_TEST)
extern u8 FirmwareReadBit000D(void);
extern void FirmwareWriteBit000D(u8 value);
extern void FirmwareWriteBit000E(u8 value);
extern void FirmwareWriteBit0018(u8 value);
extern void FirmwareWriteBit0046(u8 value);
extern u8 FirmwareReadBit0014(void);
extern u8 FirmwareReadBit0015(void);
extern void FirmwareWriteBit0014(u8 value);
extern void FirmwareWriteBit0015(u8 value);
extern void FirmwareWriteBit0008(u8 value);
extern void FirmwareWriteBit000F(u8 value);
extern void FirmwareWriteBit0022(u8 value);
extern void StartControllerMeasurementEvaluation(void);
extern u8 PollControllerMeasurementEvaluation(void);
extern u8 CompareReadWindowBounds(u32 upper_bound, u32 lower_bound);
#define CLEAR_PLAY_AUDIO_BIT0018() FirmwareWriteBit0018(0u)
#else
extern void ClearBit0018Atomic(void);
#define CLEAR_PLAY_AUDIO_BIT0018() ClearBit0018Atomic()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 27
#if defined(PLEXTOR_HOST_TEST)
StartControllerMeasurementEvaluation();
do
{
    packet_result = PollControllerMeasurementEvaluation();
} while(packet_result == 0xfeu);
#else
ArmControllerThreeBytePacketCapture();
do
{
    packet_result = ParseControllerThreeBytePacket();
} while(packet_result == 0xfeu);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 28
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 29
#if defined(PLEXTOR_HOST_TEST)
u8 use_command_address = FirmwareReadBit000D();
#else
u8 use_command_address = ReadBit000DForWriteRecordQuery();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 30
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) | 1u));
FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) & 0xfdu));
#else
SetP3Bit0Atomic();
ClearP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 31
#if defined(PLEXTOR_HOST_TEST)
static u16 FirmwareReadBigEndianU16Near(u16 address)
{
    return FirmwareReadNearU16(address);
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 32
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU16Near(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 33
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0014(0u);
FirmwareWriteBit0015(0u);
FirmwareWriteBit0012(0u);
#else
ClearBit0012Atomic();
ClearBit0014Atomic();
ClearBit0015Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 34
#if defined(PLEXTOR_HOST_TEST)
special_mode = FirmwareReadBit000E();
#else
special_mode = ReadBit000EForBuild();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 35
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) & 0xfdu));
FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 1u));
#else
ClearExternalInterrupt0PendingAndEnableAtomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 36
#if defined(PLEXTOR_HOST_TEST)
if(FirmwareReadBit0027() != 0u)
{
    FirmwareWriteBit0027(0u);
#else
if(ReadBit0027ForBuild() != 0u)
{
    ClearBit0027Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 37
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0027(1u);
#else
    SetBit0027Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 38
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0026(0u);
#else
    ClearBit0026Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 39
#if defined(PLEXTOR_HOST_TEST)
    s32 position = (s32)FirmwareReadBigEndianU32Near(0x1598u);
    u32 unsigned_position;
    u32 frame_quotient;
    u16 frames;
    u16 seconds;
    u32 minutes;

    if(position < 0)
        position += 450000l;
    unsigned_position = (u32)position;
    frame_quotient = DivideU32QuotientWithoutRuntimeLibrary(unsigned_position, 75ul);
    frames = (u16)(unsigned_position - MultiplyU32Low(frame_quotient, 75ul));
    minutes = DivideU32QuotientWithoutRuntimeLibrary(frame_quotient, 60ul);
    seconds = (u16)(frame_quotient - MultiplyU32Low(minutes, 60ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, BinaryByteToPackedBcd((u8)minutes));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, BinaryByteToPackedBcd((u8)seconds));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, BinaryByteToPackedBcd((u8)frames));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, 2u);
#else
    u8 tuple[4];

    ConvertFramePositionToBinaryMsfTuple((s32)FirmwareReadBigEndianU32Near(0x1598u), (fw_addr_t)tuple);
    ConvertThreeBinaryBytesToPackedBcd((u16)tuple);
    tuple[3] = 2u;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, tuple[0]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, tuple[1]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, tuple[2]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, tuple[3]);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 40
#if defined(PLEXTOR_HOST_TEST)
    /* RAM:fe29d2 */
    u8 RestoreRawReadDescriptorsToQueueHead(u16 descriptor_count)
    {
        u16 queued_count;
        u16 consumer_index;
        u16 index_mask;
        u8 accepted;

        FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) & 0x7fu));
        queued_count = ReadBigEndianU16(FW_NEAR_UNKNOWN_1292);
        accepted = (u8)((u16)(descriptor_count + queued_count) <= ReadBigEndianU16(FW_NEAR_UNKNOWN_129C));
        if(accepted != 0u)
        {
            queued_count = (u16)(descriptor_count + ReadBigEndianU16(FW_NEAR_UNKNOWN_1292));
            WriteBigEndianU16(FW_NEAR_UNKNOWN_1292, queued_count);
            consumer_index = ReadBigEndianU16(FW_NEAR_UNKNOWN_1294);
            index_mask = ReadBigEndianU16(FW_NEAR_UNKNOWN_129A);
            WriteBigEndianU16(FW_NEAR_UNKNOWN_1294, (u16)(consumer_index - descriptor_count) & index_mask);
        }
        FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 0x80u));
        FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 0x40u));
        FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 0x04u));
        return accepted;
    }

    /* RAM:fe2a0e */
    u8 PLEXTOR_NEAR_FUNCTION DiscardRawReadDescriptorsFromQueueTail(u16 descriptor_count)
    {
        u16 queued_count;
        u16 producer_index;
        u16 index_mask;
        u8 accepted;

        FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) & 0x7fu));
        queued_count = ReadBigEndianU16(FW_NEAR_UNKNOWN_1292);
        accepted = (u8)(descriptor_count <= queued_count);
        if(accepted != 0u)
        {
            queued_count = ReadBigEndianU16(FW_NEAR_UNKNOWN_1292);
            WriteBigEndianU16(FW_NEAR_UNKNOWN_1292, (u16)(queued_count - descriptor_count));
            producer_index = ReadBigEndianU16(FW_NEAR_UNKNOWN_1296);
            index_mask = ReadBigEndianU16(FW_NEAR_UNKNOWN_129A);
            WriteBigEndianU16(FW_NEAR_UNKNOWN_1296, (u16)(producer_index - descriptor_count) & index_mask);
        }
        FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 0x80u));
        FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 0x40u));
        FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 0x04u));
        return accepted;
    }
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 41
#if defined(PLEXTOR_HOST_TEST)
    static u32 FirmwareReadBigEndianU32Near(u16 address)
    {
        return FirmwareReadNearU32(address);
    }
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 42
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0547(0u);
#else
    ClearBit0547Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 43
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0547(1u);
    FirmwareWriteBit0546(1u);
    FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 4u));
#else
    SetBit0547Atomic();
    SetBit0546Atomic();
    FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 4u));
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 44
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0547(0u);
#else
    ClearBit0547Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 45
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0547(1u);
    FirmwareWriteBit0546(1u);
    FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 4u));
#else
    SetBit0547Atomic();
    SetBit0546Atomic();
    FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 4u));
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 46
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0023(0u);
#else
    ClearBit0023Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 47
#if defined(PLEXTOR_HOST_TEST)
    static u32 FirmwareReadBigEndianU32(fw_addr_t address)
    {
        return FirmwareReadU32(address);
    }
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 48
#if defined(PLEXTOR_HOST_TEST)
    static void FirmwareWriteBigEndianU32Near(u16 address, u32 value)
    {
        FirmwareWriteNearU32(address, value);
    }
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 49
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0580(0u);
    FirmwareWriteBit0581(1u);
#else
    ClearP3Bit0Atomic();
    SetP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 50
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0580(0u);
    FirmwareWriteBit0581(0u);
#else
    ClearP3Bit0Atomic();
    ClearP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 51
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0580(1u);
    FirmwareWriteBit0581(0u);
#else
    /* Preserve the target's bit-addressed P3 updates through the existing
     * A251 atomic boundaries. */
    SetP3Bit0Atomic();
    ClearP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 52
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0026(0u);
#else
    ClearBit0026Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 53
#if !defined(PLEXTOR_HOST_TEST)
    extern void SetP3Bit0Atomic(void);
    extern void ClearP3Bit0Atomic(void);
    extern void SetP3Bit1Atomic(void);
    extern void ClearP3Bit1Atomic(void);
    extern void ClearBit0026Atomic(void);
    extern void PLEXTOR_NEAR_FUNCTION ClearBit0027Atomic(void);
    extern void PLEXTOR_NEAR_FUNCTION SetBit0027Atomic(void);
    extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0027ForBuild(void);
    extern void ClearBit0547Atomic(void);
    extern void SetBit0547Atomic(void);
    extern void SetBit0546Atomic(void);
    extern void ClearBit0023Atomic(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 54
#if !defined(PLEXTOR_HOST_TEST)
    extern void PLEXTOR_NEAR_FUNCTION ClearBit0012Atomic(void);
    extern void PLEXTOR_NEAR_FUNCTION ClearBit0014Atomic(void);
    extern void PLEXTOR_NEAR_FUNCTION ClearBit0015Atomic(void);
    extern void PLEXTOR_NEAR_FUNCTION ClearExternalInterrupt0PendingAndEnableAtomic(void);
    extern u8 PLEXTOR_NEAR_FUNCTION ReadBit000EForBuild(void);
#else
    extern void FirmwareWriteBit0012(u8 value);
    extern u8 FirmwareReadBit000E(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 55
#if !defined(PLEXTOR_HOST_TEST)
    extern u16 ComputeDeviceBusyEventTimeValueNative(void);
#else
    static u16 ComputeDeviceBusyEventTimeValueNative(void)
    {
        u8 countdown_high = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBB4);
        u8 countdown_low = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBB5);
        u32 dividend = MultiplyU32Low((u32)(((u16)countdown_high << 8) | countdown_low), 0x5be0ul);
        u32 divisor = MultiplyU32Low((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47), 0x0002c000ul);
        u32 remainder = 0u;
        u32 quotient = 0u;
        u8 bit_index;

        /* Host vectors exercise the defined nonzero-divisor domain. The target
         * branch above retains the firmware instruction sequence for zero. */
        if(divisor == 0u)
            return 0u;
        for(bit_index = 0u; bit_index < 32u; ++bit_index)
        {
            remainder = (u32)((remainder << 1) | (dividend >> 31));
            dividend <<= 1;
            quotient <<= 1;
            if(remainder >= divisor)
            {
                remainder -= divisor;
                quotient |= 1u;
            }
        }
        return (u16)quotient;
    }
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 56
#if !defined(PLEXTOR_HOST_TEST)
    extern u8 PLEXTOR_NEAR_FUNCTION ReadBit000DForWriteRecordQuery(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 57
#if !defined(PLEXTOR_HOST_TEST)
    extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
    extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#else
    static u32 DivideU32QuotientNative(u32 dividend, u32 divisor)
    {
        return DivideU32QuotientWithoutRuntimeLibrary(dividend, divisor);
    }

    static s32 DivideS32QuotientNative(s32 dividend, s32 divisor)
    {
        return DivideS32ByU16TruncatingTowardZero(dividend, (u16)divisor);
    }
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 58
#if defined(PLEXTOR_HOST_TEST)
    static void ReadRawReadRandomConfiguration(u8 destination[4])
    {
        u8 index;

        for(index = 0u; index < 4u; ++index)
            destination[index] = FirmwareRead8(FirmwareAddressIncrement24(FW_ROM_UNKNOWN_FFA000, (fw_addr_t)(0x7aul + index)));
    }
#define READ_RAW_READ_RANDOM_CONFIGURATION(destination) ReadRawReadRandomConfiguration(destination)
#else
#define READ_RAW_READ_RANDOM_CONFIGURATION(destination) ReadDriveConfigurationBytes(0x7aul, (fw_addr_t)(destination), 4u)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 59
#if defined(PLEXTOR_HOST_TEST)
#define READ_PIPELINE_FAILURE_BIT0028 (FirmwareReadBit0028() != 0u)
#define READ_PIPELINE_FAILURE_BIT0031 (FirmwareReadBit0031() != 0u)
#else
    u8 bdata ReadPipelineFailureBitByte25 _at_ 0x25;
    u8 bdata ReadPipelineFailureBitByte26 _at_ 0x26;
    sbit ReadPipelineFailureBit0028 = ReadPipelineFailureBitByte25 ^ 0;
    sbit ReadPipelineFailureBit0031 = ReadPipelineFailureBitByte26 ^ 1;
#define READ_PIPELINE_FAILURE_BIT0028 ReadPipelineFailureBit0028
#define READ_PIPELINE_FAILURE_BIT0031 ReadPipelineFailureBit0031
#endif
#endif
