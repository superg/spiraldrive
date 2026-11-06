#include "status_mapping.h"
#include "../boot/led_boot_diagnostics.h"
#include "../media/decoder.h"
#include "../media/raw_read_frame_sequence.h"
#include "../media/raw_read_q_record_sector_acquisition.h"
#include "../media/raw_read_q_record_table_population.h"
#include "../media/state3_indexed_transitions.h"
#include "../scsi/raw_sector_range.h"
#include "../startup/drive_control_state10.h"
#include "../startup/status_routing.h"
#include "arithmetic.h"
#include "controller_measurement_transition.h"
#include "controller_operating_code_interpolation_tables.h"
#include "controller_parameters.h"
#include "controller_record_workspace_page.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "media_interrupt_leaves.h"
#include "memory.h"
#include "timing.h"
#include "track_state.h"

#define PLEXTOR_BINDING_SECTION 53
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern void PLEXTOR_NEAR_FUNCTION UpdateRawReadQPacketControlAndIdentificationForBuild(void);
#define PLEXTOR_BINDING_SECTION 54
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static s32 DivideS32ByU16TruncatingTowardZero(s32 dividend, u16 divisor)
{
    u32 remainder = 0u;
    u32 quotient = 0u;
    u32 magnitude = (u32)dividend;
    u8 bit_index;
    u8 negative = (u8)(dividend < 0);

    if(negative != 0u)
        magnitude = (u32)(0u - magnitude);
    for(bit_index = 0u; bit_index < 32u; ++bit_index)
    {
        remainder = (u32)((remainder << 1) | (magnitude >> 31));
        magnitude <<= 1;
        quotient <<= 1;
        if(remainder >= (u32)divisor)
        {
            remainder -= (u32)divisor;
            quotient |= 1u;
        }
    }
    return negative != 0u ? (s32)(0u - quotient) : (s32)quotient;
}
#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 FirmwareReadBigEndianU32LowWordFirst(fw_addr_t address)
{
    u16 low_word = FirmwareReadU16(FirmwareAddressIncrement24(address, 2u));
    u16 high_word = FirmwareReadU16(address);

    return ((u32)high_word << 16) | low_word;
}
static u16 RawReadRecordAddress(u16 record_index);

/* The RAM:f9b6e0 entry invokes RAM:fd95b1 directly after constructing this quotient.
 * Retain that native divider on the target: its zero-divisor route executes a
 * processor-defined DIV instruction and cannot be represented faithfully by C.
 */
#define PLEXTOR_BINDING_SECTION 55
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; C-facing equivalent of the RAM:feea1c controller
 * buffer reader. The native address in DR4 is an offset in the 0x100000 bank.
 */
static u8 ReadControllerBufferByteAtomicForChecksum(fw_addr_t buffer_offset)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 value;

#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    value = FirmwareRead8(FirmwareAddressIncrement24(FW_HUGE_BUFFER_BANK_BASE, buffer_offset));
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
#define PLEXTOR_BINDING_SECTION 7
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    return value;
}

/* Reconstruction helper; no direct Ghidra function address.  The native
 * P1.7 branch at RAM:fdd38c clears P3.0 then P3.1 as separate bit writes. */
void ClearP3Bit0AndBit1Atomic(void)
{
#define PLEXTOR_BINDING_SECTION 8
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:fbd456 */
u8 ValidateMediaState2ControllerCodeForWrite(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) > 2u)
    {
        SetSenseData(2u, 0x3005u);
        return 0u;
    }
    return 1u;
}

/* RAM:fbd476 */
u8 ValidateMediaState2ControllerFlagsForWrite(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB89) & 3u) != 0u)
    {
        SetSenseData(2u, 0x3005u);
        return 0u;
    }
    return 1u;
}

extern u8 FirmwareReadBit000B(void);
extern u8 FirmwareReadBit000E(void);
extern u8 FirmwareReadBit0031(void);
extern u8 FirmwareReadBit0028(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0013ForReadTargetAdjustment(void);
#define PLEXTOR_BINDING_SECTION 9
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ServicePendingHostCommandWhenOperationFlagsPermit(void);
extern void ApplyFixedControllerParameterTable(u8 table_index);
#define PLEXTOR_BINDING_SECTION 10
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 AcquireDiscReadPosition(void);
#define PLEXTOR_BINDING_SECTION 11
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 ResetDriveControlAndInitializeControllerCalibration(void);
#define PLEXTOR_BINDING_SECTION 12
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ExecuteControllerDataInTransferFromBufferOffset(u32 transfer_length, u32 buffer_offset);
#define PLEXTOR_BINDING_SECTION 13
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ServiceConditionalControllerOperatingDisable(void);
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearWithoutTimeout(void);
extern u8 PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7Clear500AndYield(void);
extern void UpdateControllerWord035FFromProgressBucket(void);
#define PLEXTOR_BINDING_SECTION 14
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ProcessModeSelectParameterList(u8 opcode, u16 parameter_list_length);
extern void HandlePlayTrackRelative12Command(void);
extern void ResetAllPmaQRecordTables(void);
extern void ResetDriveControlStateToEightForState1Calibration(void);
extern u8 InitializeControllerOperatingStateAndRunCalibrationForState1(void);
#define PLEXTOR_BINDING_SECTION 15
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 16
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern u8 AcquireRawSectorSequenceForRangeAndWait(u32 frame_count);
#define PLEXTOR_BINDING_SECTION 17
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 18
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 19
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 20
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 21
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 22
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 23
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PrepareReadVerifyCommandState(void);
extern void ExecuteReadVerifyRange(void);
extern void FinalizeReadVerifyCommandState(void);
extern u8 ExecuteRawReadQRecordSequence(u8 first_record_index);
extern void HandlePlayAudio12Command(void);
#define PLEXTOR_BINDING_SECTION 24
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 56
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void YieldCoroutine0(void);
extern void YieldCoroutine1(void);
extern void ResumeCoroutine1(void);
extern void ResumeCoroutine2(void);
extern void ApplyControllerRequestModeOne(void);
extern void ApplyControllerRequestModeTwo(void);
extern void RefreshInquiryResponseFromDriveConfiguration(void);
extern void FinalizeControllerOperatingState(void);
extern void FinalizeCloseTrackState(void);
extern void CaptureControllerPacketFlag1(void);
extern void CaptureControllerPacketFlag2(void);
extern void WaitControllerTimingUnits(u16 units);
extern u16 MeasureControllerTimingValue(u8 selector);
#define PLEXTOR_BINDING_SECTION 25
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 26
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd4ec5 */
void StartPlayAudioFrameRange(s32 start_frame_position, s32 end_frame_position, u8 playback_mode)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 4u));
    if(start_frame_position == end_frame_position)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 3u);
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_0C8E, (u32)end_frame_position);
        (void)DispatchAndWaitForReadAcquisitionResult();
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, playback_mode != 0u ? 1u : 0u);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_0F36, (u32)end_frame_position - (u32)start_frame_position);
    CLEAR_PLAY_AUDIO_BIT0018();
    START_PLAY_AUDIO_RAW_READ(start_frame_position);
}

/* RAM:fc2460 */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage0EWithTimingValue(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 0x0eu)
        return 0u;
    if(ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A22), FW_NEAR_UNKNOWN_0B2A) == 0u)
        return 0u;
    return ScaleMediaStateTimingFromWord(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D)) != 0u ? 1u : 0u;
}

/* RAM:fb9c19 */
void PLEXTOR_NEAR_FUNCTION UpdateControllerTimingValueUnlessFlag4Set(void)
{
    u16 value;

    if(((u8)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5) & 0x10u) != 0u)
        return;
    WaitControllerTimingUnits(10u);
    value = MeasureControllerTimingValue(0x40u);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0FD5, value);
    value = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FD5);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0FD5, (u16)(value + 0x78u));
}

/* RAM:fb9ad3 */
void PLEXTOR_NEAR_FUNCTION CaptureControllerPacketsFromFlagWord(void)
{
    if((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5) & 2u) != 0u)
        CaptureControllerPacketFlag1();
    if((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5) & 4u) != 0u)
        CaptureControllerPacketFlag2();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FED, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED) + 1u));
}

/* RAM:fb5192 */
void FinalizeCloseTrackSessionState(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 1u));
        CompleteScsiCommandWithNoSense();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    }
    FinalizeCloseTrackState();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}

/* RAM:fb22ba */
u8 PLEXTOR_NEAR_FUNCTION ResetControllerOperatingStateTail(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D8, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018093, 0x36u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 4u);
    FinalizeControllerOperatingState();
    return 1u;
}

/* RAM:f95af2 */
void PLEXTOR_NEAR_FUNCTION RetryControllerMeasurementEvaluation(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, 0u);
    do
    {
        u32 position;
        u32 current;
        u8 packet_result;

#define PLEXTOR_BINDING_SECTION 27
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 4u);
            return;
        }
        position = FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_103D);
        current = FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_1235);
        if(current == position - 1ul)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 8u));
        }
        if((s32)current < (s32)(position + 15ul))
            continue;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 1u);
        return;
    } while(1);
}

/* Reconstruction helper; no direct Ghidra function address.
 * Restoring unsigned divide without C251's runtime-library division helper.
 * The native packet-progress path produces quotient zero for 0 / 0. */
static u32 DivideU32QuotientWithoutRuntimeLibrary(u32 dividend, u32 divisor)
{
    u32 quotient = 0;
    u32 remainder = 0;
    u8 bit_count = 32u;

    if(divisor == 0ul)
        return 0ul;

    do
    {
        u8 high_bit = (u8)(dividend >> 31);
        u8 remainder_overflow = (u8)(remainder >> 31);

        dividend <<= 1;
        remainder = (remainder << 1) | (u32)high_bit;
        quotient <<= 1;
        if(remainder_overflow != 0u || remainder >= divisor)
        {
            remainder -= divisor;
            quotient |= 1ul;
        }
        bit_count--;
    } while(bit_count != 0u);
    return quotient;
}

#define PLEXTOR_BINDING_SECTION 57
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static void EncodeFramePositionAsPackedBcdMsf(s32 frame_position, u8 output[3])
{
    u32 position;
    u32 frame_quotient;
    u32 minutes;
    u16 seconds;
    u16 frames;

    if(frame_position < 0)
        frame_position += 450000l;
    position = (u32)frame_position;
    frame_quotient = DivideU32QuotientWithoutRuntimeLibrary(position, 75ul);
    frames = (u16)(position - MultiplyU32Low(frame_quotient, 75ul));
    minutes = DivideU32QuotientWithoutRuntimeLibrary(frame_quotient, 60ul);
    seconds = (u16)(frame_quotient - MultiplyU32Low(minutes, 60ul));
    output[0] = BinaryByteToPackedBcd((u8)minutes);
    output[1] = BinaryByteToPackedBcd((u8)seconds);
    output[2] = BinaryByteToPackedBcd((u8)frames);
}

/* Reconstruction helper; no direct Ghidra function address.
 * Mirrors the signed divider's remainder result without importing the C251
 * runtime divider. The source divisor is the nonzero literal 100. */
static s32 SignedRemainderBy100(s32 dividend)
{
    u32 magnitude = (u32)dividend;
    u32 quotient;
    u32 remainder;

    if(dividend < 0)
        magnitude = (u32)(0ul - magnitude);
    quotient = DivideU32QuotientWithoutRuntimeLibrary(magnitude, 100ul);
    remainder = magnitude - MultiplyU32Low(quotient, 100ul);
    return dividend < 0 ? (s32)(0ul - remainder) : (s32)remainder;
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 28
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fba432 */
u8 AreAuxiliaryDecodedTocPayloadsComplete(void)
{
    u8 table_index;
    u8 present_byte_count = 0;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F566) == 0)
        return 0;

    /* The first payload has one required point byte per ten-byte record. */
    table_index = 1;
    while(table_index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_F56B))
    {
        if(FirmwareReadNear8((u16)(0xf7c6u + (u16)table_index * 10u)) == 0)
            return 0;
        table_index++;
    }

    /* The source deliberately leaves this loop at the first absent byte and
     * compares the accumulated count rather than requiring all three records.
     * Keep that behavior, including its individual volatile reads. */
    table_index = 1;
    do
    {
        u16 record = (u16)(0xf79eu + (u16)table_index * 10u);

        if(FirmwareRead8(record) == 0)
            break;
        present_byte_count++;
        if(FirmwareReadNear8((u16)(record + 1u)) == 0)
            break;
        present_byte_count++;
        if(FirmwareReadNear8((u16)(record + 2u)) == 0)
            break;
        present_byte_count++;
        if(FirmwareReadNear8((u16)(record + 3u)) == 0)
            break;
        present_byte_count++;
        if(FirmwareReadNear8((u16)(record + 4u)) == 0)
            break;
        present_byte_count++;
        if(FirmwareReadNear8((u16)(record + 5u)) == 0)
            break;
        present_byte_count++;
        if(FirmwareReadNear8((u16)(record + 6u)) == 0)
            break;
        present_byte_count++;
        table_index++;
    } while(table_index <= 3u);

    if(present_byte_count != FirmwareReadNear8(FW_NEAR_UNKNOWN_F56C))
        return 0;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x10u) == 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1E) + 1u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1E) <= 0x28u)
            return 0;
    }
    return 1;
}

/* RAM:fbacc5 */
u8 PLEXTOR_NEAR_FUNCTION IsCurrentSessionTocComplete(void)
{
    u8 point;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x01u) == 0)
        return 0;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x02u) == 0)
        return 0;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x04u) == 0)
        return 0;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x20u) != 0)
    {
        if(AreAuxiliaryDecodedTocPayloadsComplete() == 0)
            return 0;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 0x10u));
    }

    point = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC19);
    while(point <= FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A))
    {
        u8 summary_point = FirmwareReadNear8((u16)(0xf2b6u + (u16)point * 6u));

        if(summary_point == 0 || summary_point == 0xaau)
            return 0;
        point++;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1B, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A));
    return 1;
}

/* RAM:fba2d3 */
u8 PLEXTOR_NEAR_FUNCTION IsInitialDiscTocComplete(void)
{
    u8 retry_count;
    u8 point;
    u8 lead_out_point;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x01u) == 0)
        return 0;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x02u) == 0)
        return 0;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x04u) == 0)
        return 0;

    retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_F2B6);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F2B6, (u8)(retry_count + 1u));
    if((s8)retry_count <= 5)
        return 0;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F2B6, 6u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x80u) == 0)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x40u) == 0)
            goto skip_initial_payload_retry;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x10u) == 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1E) + 1u));
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1E) <= 0x14)
            return 0;
    }

skip_initial_payload_retry:
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x10u) != 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 0x08u));
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F550, FW_NEAR_UNKNOWN_F528, 10u);
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x20u) != 0)
    {
        if(AreAuxiliaryDecodedTocPayloadsComplete() == 0)
            return 0;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 0x10u));
    }

    point = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    while(point <= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
    {
        if(FirmwareReadNear8((u16)(0xf2b6u + (u16)point * 6u)) == 0)
            return 0;
        point++;
    }

    lead_out_point = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u);
    FirmwareWriteNear8((u16)(0xf2b6u + (u16)lead_out_point * 6u), 0xaau);
    FirmwareWriteNear8((u16)(0xf2b7u + (u16)lead_out_point * 6u), FirmwareReadNear8(FW_NEAR_UNKNOWN_F546));
    FirmwareWriteNear8((u16)(0xf2b8u + (u16)lead_out_point * 6u), 0);
    FirmwareWriteNear8((u16)(0xf2b9u + (u16)lead_out_point * 6u), FirmwareReadNear8(FW_NEAR_UNKNOWN_F54D));
    FirmwareWriteNear8((u16)(0xf2bau + (u16)lead_out_point * 6u), FirmwareReadNear8(FW_NEAR_UNKNOWN_F54E));
    FirmwareWriteNear8((u16)(0xf2bbu + (u16)lead_out_point * 6u), FirmwareReadNear8(FW_NEAR_UNKNOWN_F54F));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) == 1u && ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_F2BF) >= 0x8ca)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | 0x10u));
    return 1;
}

/* RAM:fc1b17 */
void PLEXTOR_NEAR_FUNCTION ClearTrackTableRecordAtIndex(u8 track_record_index)
{
    u16 record = (u16)(0xdd80u + (u16)track_record_index * 19u);

    FirmwareWriteNear8((u16)(record + 7u), 0);
    FirmwareWriteNear8((u16)(record + 9u), (u8)(FirmwareReadNear8((u16)(record + 9u)) & 0xfeu));
    FirmwareWriteNear8((u16)(record + 8u), 0);
    FirmwareWriteNear8((u16)(record + 6u), 0);
    FirmwareWriteNear8((u16)(record + 5u), 0);
    FirmwareWriteNear8((u16)(record + 4u), 0);
    FirmwareWriteNear8((u16)(record + 3u), 0);
    FirmwareWriteNear8((u16)(record + 2u), 0);
    FirmwareWriteNear8((u16)(record + 1u), 0);
    FirmwareWrite8(record, 0);
    FirmwareWriteBigEndianU32Near((u16)(record + 10u), 0u);
    FirmwareWriteBigEndianU32Near((u16)(record + 14u), 0u);
    FirmwareWriteNear8((u16)(record + 9u), (u8)(FirmwareReadNear8((u16)(record + 9u)) & 0xfdu));
    FirmwareWriteNear8((u16)(record + 18u), 0);
}

/* RAM:fb53c9 */
u8 FindFirstType1TrackRecordNumberOrOne(void)
{
    u16 record_index = 0;
    u16 last_record_index = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_B4BB);

    while(record_index <= last_record_index)
    {
        u16 record = (u16)(0xb4bdu + (u16)(record_index << 3));

        if((FirmwareRead8(record) & 0x0fu) == 1u)
        {
            u8 number = FirmwareReadNear8((u16)(record + 1u));

            if(number >= 1u && number <= 99u)
                return number;
        }
        record_index++;
    }
    return 1u;
}

/* RAM:fb556a */
void PLEXTOR_NEAR_FUNCTION UpdateTrackTableControlBit6Intersection(void)
{
    u8 record_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15F7, 0x40u);
    while(record_index < FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F))
    {
        if((FirmwareReadNear8((u16)(0xdd80u + (u16)record_index * 19u)) & 0x40u) != 0x40u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15F7, 0);
        record_index++;
    }
}

/* RAM:faefaf */
u8 FindTrackTableIndexForFramePosition(s32 frame_position)
{
    u8 record_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E);

    while(record_index < FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F))
    {
        s32 record_position = ConvertBinaryMsfToAdjustedFramePosition((u16)(0xdd84u + (u16)record_index * 19u));

        if(frame_position < record_position)
            return record_index;
        record_index++;
    }
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
}

/* RAM:faef6d */
u8 PLEXTOR_NEAR_FUNCTION FindTrackNumberForFramePositionFromTocTable(s32 frame_position)
{
    u8 track_number = 2u;

    while(track_number <= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
    {
        s32 start_position = ConvertBinaryMsfToAdjustedFramePosition((u16)(0xf2b9u + (u16)track_number * 6u));

        if(frame_position < start_position)
            return (u8)(track_number - 1u);
        track_number++;
    }
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
}

/* RAM:fbd5e5 */
u8 IsThirtySevenTrackLayoutSpacedBy9000Frames(void)
{
    u8 track_number;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) != 1u)
        return 0;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0)
        return 0;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) != 37u)
        return 0;

    for(track_number = 1u; track_number <= 37u; track_number++)
    {
        u16 record = (u16)(0xdd80u + (u16)track_number * 19u);
        s32 start_position;
        u32 end_position;

        if(FirmwareRead8(record) != 1u)
            return 0;
        start_position = ConvertBinaryMsfToAdjustedFramePosition((u16)(record + 1u));
        if(track_number == 37u)
            end_position = FirmwareReadNearU32(FW_NEAR_UNKNOWN_12C1);
        else
            end_position = (u32)ConvertBinaryMsfToAdjustedFramePosition((u16)(record + 20u));
        if((u32)(end_position - (u32)start_position) != 9000ul)
            return 0;
    }
    return 1;
}

/* RAM:faf1e3 */
s32 CompressWorkingTrackPacketFramePosition(s32 frame_position)
{
    u32 packet_span = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57) + 7ul;
    u32 start_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D);
    u32 quotient = DivideU32QuotientWithoutRuntimeLibrary((u32)frame_position - start_position, packet_span);

    return (s32)((u32)frame_position - MultiplyU32Low(7ul, quotient));
}

/* RAM:fd4d71 */
u8 GetEffectiveLastTrackNumber(void)
{
    u8 high_index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0)
        return FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);

    high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    if(high_index == 100u)
        return 99u;
    if(FirmwareReadNear8((u16)(0xdd87u + (u16)high_index * 19u)) == 0)
        high_index--;
    return high_index;
}

/* RAM:fc3c32 */
u8 ClassifyTrackInformationResultCode(u8 value)
{
    if(value == 8u || value == 0x10u || value == 0x20u)
        return 2u;
    if(value == 0x80u)
        return 0u;
    return 1u;
}

/* RAM:fb6125 */
void UpdateTrackRecordControlFlag000A(void)
{
    u8 first_track;
    u8 last_track;
    u8 track;

    FirmwareWriteBit000A(0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        return;

    last_track = GetEffectiveLastTrackNumber();
    first_track = GetEffectiveFirstTrackNumber();
    track = first_track;
    while((s8)track <= (s8)last_track)
    {
        u16 record_offset = (u16)((u16)track * 19u);

        if((FirmwareReadNear8((u16)(0xdd80u + record_offset)) & 0xd0u) == 0x50u && (FirmwareReadNear8((u16)(0xdd89u + record_offset)) & 0x01u) != 0u)
            FirmwareWriteBit000A(1u);
        track = (u8)(track + 1u);
    }
}

/* RAM:fe2d67 */
u8 FindAdjacentType1TrackRecordIndex(u8 search_forward, u16 start_index)
{
    u8 record_index = (u8)start_index;

    if(search_forward != 0u)
    {
        do
        {
            record_index++;
        } while((FirmwareReadNear8((u16)(0xb4bdu + (u16)record_index * 8u)) & 0x0fu) != 1u);
    }
    else
    {
        do
        {
            record_index--;
        } while((FirmwareReadNear8((u16)(0xb4bdu + (u16)record_index * 8u)) & 0x0fu) != 1u);
    }
    return record_index;
}

/* RAM:fb5ca6 */
void DivideWorkingTrackPacketProgress(fw_addr_t remainder_address, fw_addr_t quotient_address)
{
    u32 packet_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    u32 progress = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B);
    u32 quotient = DivideU32QuotientWithoutRuntimeLibrary(progress, packet_size);
    u32 remainder;

    FirmwareWriteBigEndianU32(quotient_address, quotient);
    quotient = FirmwareReadBigEndianU32(quotient_address);
    packet_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    remainder = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B) - MultiplyU32Low(packet_size, quotient);
    FirmwareWriteBigEndianU32(remainder_address, remainder);
}

/* RAM:faf0f9 */
s32 ComputeWorkingTrackPacketProgressFramePosition(u8 compress_packet_gaps)
{
    u32 packet_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    u32 progress = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B);
    u32 quotient = DivideU32QuotientWithoutRuntimeLibrary(progress, packet_size);
    u32 remainder;
    u32 base_position;
    u32 result;
    u8 use_alternate_base = 0;

    packet_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    progress = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B);
    remainder = progress - MultiplyU32Low(packet_size, quotient);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) != 0)
        use_alternate_base = 1u;

    packet_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    if(use_alternate_base != 0u)
        base_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD49);
    else
        base_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45);
    result = base_position + MultiplyU32Low(quotient, packet_size + 7ul) + remainder + 5ul;
    if(FirmwareReadBit000B() != 0u && compress_packet_gaps == 1u)
        result = (u32)CompressWorkingTrackPacketFramePosition((s32)result);
    return (s32)result;
}

/* RAM:faf086 */
s32 ComputeWorkingTrackNextWritableFramePosition(void)
{
    u8 track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);

    if(track_type == 1u || track_type == 3u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0)
            return (s32)((u32)ComputeWorkingTrackPacketProgressFramePosition(1u) - 150ul);
        return (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) - 145ul);
    }

    if(track_type == 5u || track_type == 6u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0 && FirmwareReadBit000B() != 0u)
            return (s32)((u32)CompressWorkingTrackPacketFramePosition((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD51)) - 150ul);
        return (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD51) - 150ul);
    }

    return (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) - 150ul);
}

/* RAM:fb5144 */
void PLEXTOR_NEAR_FUNCTION UpdateSelectedTrackTableTailFrameBounds(void)
{
    u8 selected_high_index;
    s32 tail_position;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0)
        selected_high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC);
    else
        selected_high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);

    selected_high_index--;
    tail_position = ConvertBinaryMsfToAdjustedFramePosition((u16)(0xdd84u + (u16)selected_high_index * 19u));
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD71, (u32)tail_position);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0)
        tail_position = (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD71) + 0x1a5eul);
    else
        tail_position = (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD71) + 0x8caul);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD75, (u32)tail_position);
}

/* RAM:fbcc86 */
void ResetTrackInformationTableState(void)
{
    u8 record_index;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, 0);
    for(record_index = 1u; record_index < 100u; record_index++)
        ClearTrackTableRecordAtIndex(record_index);
}

/* RAM:fc36a5 */
void InitializeTrackTableSessionFrameBounds(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) + 1u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) != 0));
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD69, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F));
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD6D, 0);
    }
    else
    {
        u32 reference_position;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 0);
        }
        else
        {
            u8 last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
            s32 session_position = ConvertBinaryMsfToAdjustedFramePosition(FW_HUGE_UNKNOWN_2BB042 + ((fw_addr_t)last_track << 3) + ((fw_addr_t)last_track << 1));

            reference_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, (u8)(session_position >= (s32)reference_position));
        }
        reference_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A);
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD6D, reference_position);
        reference_position -= 4500ul;
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD69, reference_position);
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD65, reference_position);
    }
}

/* RAM:fc3fce */
void PopulateTrackTableRecordFromRawQRecord(fw_addr_t raw_q_record_address)
{
    u8 track_number;
    u16 destination;
    u16 descriptor_index;
    u16 descriptor_count;

    track_number = FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, track_number);
    track_number = FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 2u));
    destination = (u16)(0xdd80u + (u16)track_number * 19u);
    FirmwareWrite8(destination, FirmwareRead8(raw_q_record_address));
    FirmwareWriteNear8((u16)(destination + 1u), FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 7u)));
    FirmwareWriteNear8((u16)(destination + 2u), FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 8u)));
    FirmwareWriteNear8((u16)(destination + 3u), FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 9u)));
    FirmwareWriteNear8((u16)(destination + 4u), FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 3u)));
    FirmwareWriteNear8((u16)(destination + 5u), FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 4u)));
    FirmwareWriteNear8((u16)(destination + 6u), FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 5u)));

    descriptor_count = (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_12CA) / 11u);
    for(descriptor_index = 1u; descriptor_index <= descriptor_count; descriptor_index++)
    {
        fw_addr_t descriptor_address = FW_HUGE_UNKNOWN_2B8000 + ((fw_addr_t)descriptor_index << 3) + ((fw_addr_t)descriptor_index << 1) + (fw_addr_t)descriptor_index;

        if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor_address, 3u)) == FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 2u)))
        {
            FirmwareWriteNear8((u16)(destination + 18u), FirmwareRead8(descriptor_address));
            break;
        }
    }
    if(FirmwareReadNear8((u16)(destination + 18u)) == 0)
        FirmwareWriteNear8((u16)(destination + 18u), FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64));
    FirmwareWriteNear8((u16)(destination + 9u), (u8)(FirmwareReadNear8((u16)(destination + 9u)) | 0x02u));
    FirmwareWriteNear8((u16)(destination + 7u), (FirmwareRead8(raw_q_record_address) & 0xd0u) == 0x50u ? 3u : 2u);
}

/* RAM:fbc6b0 */
s32 PLEXTOR_NEAR_FUNCTION FUN_RAM_fbc6b0(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 1u)
        return (s32)((u32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D) - 150ul);
    if(write_type == 2u)
        return 0;
    if(write_type == 3u)
        return (s32)((u32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD65) - 150ul);
    if(write_type == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
            return (s32)((u32)ComputeWorkingTrackPacketProgressFramePosition(0u) - 150ul);
        return (s32)((u32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D) - 150ul);
    }
    return 0;
}

/* RAM:fc4881 */
void DecodeWorkingTrackModeFromTdiRecord(fw_addr_t track_mode_address, fw_addr_t record_class_address)
{
    u8 pairs_match = 1u;

    FirmwareWrite8(record_class_address, FirmwareRead8(FW_HUGE_UNKNOWN_2BB91C) & 0x03u);
    if(FirmwareRead8(record_class_address) != 2u)
    {
        FirmwareWrite8(track_mode_address, 0x40u);
        return;
    }

    {
        u8 index;
        for(index = 0u; index < 4u; ++index)
        {
            u8 duplicate = FirmwareRead8(FW_HUGE_UNKNOWN_2BB921 + index);
            u8 original = FirmwareRead8(FW_HUGE_UNKNOWN_2BB91D + index);
            if(duplicate != original)
                pairs_match = 0u;
        }
    }

    if(pairs_match == 0u)
        FirmwareWrite8(track_mode_address, 0x20u);
    else if((FirmwareRead8(FW_HUGE_UNKNOWN_2BB91F) & 0x20u) != 0u)
        FirmwareWrite8(track_mode_address, 0x08u);
    else
        FirmwareWrite8(track_mode_address, 0x10u);
}

/* RAM:fc4923 */
u8 ParseWorkingTrackTdiRecordMetadata(void)
{
    u8 record_offset = 4u;
    fw_addr_t record_address;
    fw_addr_t marker_address;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) == 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38) != 0x20u)
            record_offset = 12u;
    }
    record_address = FW_HUGE_UNKNOWN_2BB919 + (fw_addr_t)record_offset;
    if(FirmwareRead8(record_address) != 0x54u || FirmwareRead8(FirmwareAddressIncrementLow16(record_address, 1u)) != 0x44u || FirmwareRead8(FirmwareAddressIncrementLow16(record_address, 2u)) != 0x49u)
        return 0u;

    marker_address = FirmwareAddressIncrementLow16(record_address, 9u);
    if(FirmwareRead8(marker_address) != 0x90u && FirmwareRead8(marker_address) != 0x91u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD55, 1u);
    if(FirmwareRead8(marker_address) == 0x90u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 0u);
    }
    else
    {
        u32 packed_bcd_value;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 1u);
        {
            u8 byte2 = FirmwareRead8(marker_address + 2ul);
            u8 byte3 = FirmwareRead8(marker_address + 3ul);
            u8 byte0 = FirmwareRead8(marker_address);
            u8 byte1 = FirmwareRead8(marker_address + 1ul);
            packed_bcd_value = ((u32)byte0 << 24) | ((u32)byte1 << 16) | ((u32)byte2 << 8) | byte3;
        }
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD57, DecodeThreeBytePackedBcdBase100Value(packed_bcd_value & 0x00fffffful));
    }
    return 1u;
}

/* RAM:fb5b58 */
void UpdateTrackTableReferenceFromCurrentSessionPointB0(void)
{
    u16 descriptor_index;
    u16 descriptor_count;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 0);
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD7A, 0);
        return;
    }

    descriptor_count = (u16)DivideU32QuotientWithoutRuntimeLibrary(((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_12CA) << 8) | FirmwareReadNear8(FW_NEAR_UNKNOWN_12CB), 11ul);
    for(descriptor_index = 1u; descriptor_index <= descriptor_count; descriptor_index++)
    {
        fw_addr_t descriptor_address = FW_HUGE_UNKNOWN_2B8000 + ((fw_addr_t)descriptor_index << 3) + ((fw_addr_t)descriptor_index << 1) + (fw_addr_t)descriptor_index;

        if(FirmwareRead8(descriptor_address) != FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) || FirmwareRead8(FirmwareAddressIncrementLow16(descriptor_address, 3u)) != 0xb0u)
            continue;

        if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor_address, 4u)) == 0xffu)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 1u);
            return;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 0);
        /* The original copies this three-byte tuple to its private stack before
         * conversion. Reading it directly retains the same source read order;
         * the stack copy has no firmware-memory side effect. */
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_DD7A, (u32)ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(descriptor_address, 4u)));
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 1u);
}

/* Reconstruction helper; no direct Ghidra function address.
 * The original keeps the 0x2b bank fixed while its 16-bit descriptor offset
 * wraps.  Callers establish that a matching descriptor is reachable. */
#define FullTocDescriptorAddress(descriptor_index) (FW_HUGE_UNKNOWN_2B0000 | (fw_addr_t)(u16)(0x8000u + ((u16)(descriptor_index) << 3) + ((u16)(descriptor_index) << 1) + (u16)(descriptor_index)))

/* RAM:fbad3e */
/* Descriptor zero is deliberately retained. The native loops use signed
 * word comparisons over the nonnegative ranges 1..499 and 0..10. */
void InitializeFullTocDescriptorWorkspaceForSession1(void)
{
    u16 descriptor_index;

    for(descriptor_index = 1u; descriptor_index < 500u; ++descriptor_index)
    {
        fw_addr_t descriptor_address = FullTocDescriptorAddress(descriptor_index);
        u16 byte_index;

        for(byte_index = 0u; byte_index < 11u; ++byte_index)
            FirmwareWrite8(descriptor_address + byte_index, 0u);
    }
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_12DC, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, 1u);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_12CA, 0u);
}

/* Reconstruction helper for RAM:fcd09f; the native MOV word sequence writes
 * the low half at address+2 before the high half at address. */
static void FirmwareWriteBigEndianU32LowWordFirst(fw_addr_t address, u32 value)
{
    FirmwareWriteU16(FirmwareAddressIncrement24(address, 2u), (u16)value);
    FirmwareWriteU16(address, (u16)(value >> 16));
}

/* RAM:fcd09f */
/* All 64 canonical RAM:f84690 table values were checked: entry i is
 * 0x000006ae + i * 0x15f0. */
void InitializeControllerRecordBoundaryTableFromDefaultsFromIndex(u8 first_index)
{
    while(first_index <= 63u)
    {
        u32 default_boundary = 0x000006aeul + (u32)first_index * 0x15f0ul;
        fw_addr_t destination = FW_HUGE_UNKNOWN_2BAE33 + (u16)((u16)first_index << 2);

        FirmwareWriteBigEndianU32LowWordFirst(destination, default_boundary - 5ul);
        first_index++;
    }
    FirmwareWriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BAE2F, 145ul);
}

/* RAM:fcd067 */
/* Native XRL B,B then SJMP RAM:fcd09f. */
void InitializeControllerRecordBoundaryTableFromDefaults(void)
{
    InitializeControllerRecordBoundaryTableFromDefaultsFromIndex(0u);
}

/* RAM:fcbbc9 */
void InitializeReadVerifyWorkingTrackState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD9C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD9C) | 1u));
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD9D, 0x20ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD94, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD95, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD96, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD97, 0xffu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD98, 0xffu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD99, 0xffu);
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 3u)
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_12C1, FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION));
    FirmwareWriteBit000C(1u);
    FirmwareWriteBit000A(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 1u);
}

/* RAM:fd7d65 */
u8 HandleIndexedState09(void)
{
    s32 target = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);

    if(target > 225l)
        target = 150l;
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u)
        target = (s32)((u32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1) - 5ul);
    else if((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC) & 0x0800u) == 0u)
        target = (s32)((u32)FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION) - 5ul);
    else
        target = 300l;
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)target);
    return 1u;
}

/* RAM:fd7d01 */
/* ACC holds mode_selector and B holds inherited_track_code. */
void ConfigureIndexedStateParameters(u8 mode_selector, u8 inherited_track_code)
{
    u8 track_code = inherited_track_code;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) == 15u)
    {
        u8 cursor_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);

        if(cursor_track == 9u)
        {
            track_code = 15u;
            FirmwareWriteBigEndianU32LowWordFirst(FW_NEAR_UNKNOWN_0E3B, 0x0002d3b1ul);
        }
        else if(cursor_track == 15u && mode_selector == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            return;
        }
        else if(cursor_track == 4u)
        {
            track_code = 9u;
            FirmwareWriteBigEndianU32LowWordFirst(FW_NEAR_UNKNOWN_0E3B, 0x00019497ul);
        }
        else
        {
            track_code = 4u;
            FirmwareWriteBigEndianU32LowWordFirst(FW_NEAR_UNKNOWN_0E3B, 0x0000be6eul);
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_code);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_code);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
}

/* RAM:fb40e9 */
u8 ValidateIndexedState0FReferencePayloads(void)
{
    u8 index;

    for(index = 0u; index < 44u; ++index)
    {
        u8 reference_byte = FirmwareRead8(PLEXTOR_INDEXED_STATE_REFERENCE_ADDRESS(FW_ROM_UNKNOWN_F894E3) + index);
        u8 payload_byte = FirmwareRead8(FW_HUGE_UNKNOWN_2B800B + index);

        if(payload_byte != reference_byte)
            return 0u;
    }
    for(index = 0u; index < 10u; ++index)
    {
        u8 reference_byte = FirmwareRead8(PLEXTOR_INDEXED_STATE_REFERENCE_ADDRESS(FW_ROM_UNKNOWN_F8950F) + index);
        u8 payload_byte = FirmwareRead8(FW_HUGE_UNKNOWN_2BB045 + index);

        if(payload_byte != reference_byte)
            return 0u;
    }
    for(index = 0u; index < 19u; ++index)
    {
        u8 reference_byte = FirmwareRead8(PLEXTOR_INDEXED_STATE_REFERENCE_ADDRESS(FW_ROM_UNKNOWN_F89519) + index);
        u8 payload_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD93 + index);

        if(payload_byte != reference_byte)
            return 0u;
    }
    return 1u;
}

/* Reconstruction helper; no direct Ghidra function address. */
static void ClearMutableTrackInformationRecordFields(u8 record_index)
{
    u16 record = (u16)(0xdd80u + (u16)((u16)record_index * 19u));
    u8 flags;

    FirmwareWriteNear8((u16)(record + 7u), 0u);
    flags = FirmwareReadNear8((u16)(record + 9u));
    FirmwareWriteNear8((u16)(record + 9u), (u8)(flags & 0xfeu));
    FirmwareWriteNear8((u16)(record + 8u), 0u);
    FirmwareWriteNear8((u16)(record + 6u), 0u);
    FirmwareWriteNear8((u16)(record + 5u), 0u);
    FirmwareWriteNear8((u16)(record + 4u), 0u);
    FirmwareWriteNear8((u16)(record + 3u), 0u);
    FirmwareWriteNear8((u16)(record + 2u), 0u);
    FirmwareWriteNear8((u16)(record + 1u), 0u);
    FirmwareWrite8(record, 0u);
    FirmwareWriteNear8((u16)(record + 12u), 0u);
    FirmwareWriteNear8((u16)(record + 13u), 0u);
    FirmwareWriteNear8((u16)(record + 10u), 0u);
    FirmwareWriteNear8((u16)(record + 11u), 0u);
    FirmwareWriteNear8((u16)(record + 16u), 0u);
    FirmwareWriteNear8((u16)(record + 17u), 0u);
    FirmwareWriteNear8((u16)(record + 14u), 0u);
    FirmwareWriteNear8((u16)(record + 15u), 0u);
    flags = FirmwareReadNear8((u16)(record + 9u));
    FirmwareWriteNear8((u16)(record + 9u), (u8)(flags & 0xfdu));
    FirmwareWriteNear8((u16)(record + 18u), 0u);
}

/* RAM:fc1a71 */
/* then shared
 * RAM:fc1b0e bound check. B is explicit in the C interface. */
void ResetTrackInformationRecordAndContinue(u8 first_record_index)
{
    do
    {
        ClearMutableTrackInformationRecordFields(first_record_index);
        first_record_index = (u8)(first_record_index + 1u);
    } while(first_record_index < 100u);
}

/* RAM:fc1b0e */
void ContinueResetTrackInformationRecordsFromIndex(u8 first_record_index)
{
    if(first_record_index < 100u)
        ResetTrackInformationRecordAndContinue(first_record_index);
}

/* RAM:fc1a6c */
/* The RAM:fc1b0e ACC input is explicit in the C interface. */
void ResetTrackInformationRecordsFromIndex(u8 first_record_index)
{
    ContinueResetTrackInformationRecordsFromIndex(first_record_index);
}

/* RAM:fba58d */
void ResetDecodedTocPayloadTablesFromIndex(u8 first_payload_index)
{
    u8 byte_index;

    while(first_payload_index <= 40u)
    {
        fw_addr_t payload = FW_NEAR_UNKNOWN_F7C4 + (u16)(first_payload_index * 10u);

        for(byte_index = 0u; byte_index < 10u; ++byte_index)
            FirmwareWrite8(payload + byte_index, 0u);
        ++first_payload_index;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1C, 0u);
    for(first_payload_index = 0u; first_payload_index <= 3u; ++first_payload_index)
    {
        fw_addr_t payload = FW_NEAR_UNKNOWN_F79C + (u16)(first_payload_index * 10u);

        for(byte_index = 0u; byte_index < 10u; ++byte_index)
            FirmwareWrite8(payload + byte_index, 0u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1D, 0u);
}

/* RAM:fba567 */
/* Native XRL B,B then SJMP RAM:fba58d. */
void ResetDecodedTocPayloadTables(void)
{
    ResetDecodedTocPayloadTablesFromIndex(0u);
}

/* RAM:fba51d */
/* B supplies the direct-entry point index. */
void ResetTocAcquisitionStateFromPointIndexAndFinalize(u8 first_point_index)
{
    while((s8)first_point_index <= 100)
    {
        u16 record_address = (u16)(0xf2b6u + (u16)((u16)first_point_index * 6u));
        u8 byte_index;

        for(byte_index = 0u; byte_index < 6u; ++byte_index)
            FirmwareWriteNear8((u16)(record_address + byte_index), 0u);
        first_point_index++;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BF, 0u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_12D4, 0ul);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_12D8, 0ul);
    ResetDecodedTocPayloadTables();
    InitializeFullTocDescriptorWorkspaceForSession1();
}

/* RAM:fba4f7 */
void PLEXTOR_NEAR_FUNCTION ResetAllTocAcquisitionState(void)
{
    ResetTocAcquisitionStateFromPointIndexAndFinalize(0u);
}

/* RAM:fd05b8 */
/* The native body reads each input before preparing the next call argument.
 * Keep that order explicit because the firmware RAM is externally observable. */
void UpdateControllerMeasurementReference(void)
{
    u32 radial_model_scale = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5);
    u16 radial_scale_quotient = (u16)DivideU32QuotientNative(radial_model_scale, 65ul);
    u32 read_acquisition_target = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);
    u8 secondary_operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
    u32 reference = ComputeControllerMeasurementReferenceForOperatingCode(read_acquisition_target, radial_scale_quotient, secondary_operating_code);

    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FC27, (u16)reference);
}

/* RAM:f86895 */
/* Position and speed rereads are instruction-visible and may observe
 * asynchronously updated firmware state, so do not merge them. */
u16 ComputeReadAcquisitionDynamicToleranceFrames(void)
{
    s32 current_position;
    s32 target_position;
    s32 comparison_target;
    s32 comparison_current;
    s32 distance;
    s32 tolerance;
    u8 speed;

    UpdateControllerMeasurementReference();
    current_position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);
    target_position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    distance = (s32)((u32)target_position - (u32)current_position);
    comparison_target = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    comparison_current = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);
    if(comparison_current >= comparison_target)
    {
        target_position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
        current_position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);
        distance = (s32)((u32)current_position - (u32)target_position);
    }

    tolerance = DivideS32QuotientNative(distance, 3000l);
    speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
    if(speed >= 0x20u)
        return 250u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) == 0x18u)
        return 180u;
    if(tolerance <= 15l)
        tolerance = 15l;
    if(tolerance >= 120l)
        tolerance = 120l;
    return (u16)tolerance;
}

/* RAM:f8f30f */
/* ACC supplies restart_timeout and receives the Boolean result. */
u8 CheckControllerMeasurementToleranceOrTimeout(u8 restart_timeout)
{
    UpdateControllerMeasurementReference();
    if(IsControllerPort80A8A9WithinScaledTolerance(5u, 0u) != 0u)
        return 1u;
    if(restart_timeout != 0u)
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_1034, 500u);
    return (u8)((s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1034) <= 0);
}

/* RAM:fd0ca8 */
/* The native R7/B selections are overwritten before use. The reads that feed
 * those discarded selections remain explicit because RAM is externally
 * observable firmware state. */
void PLEXTOR_NEAR_FUNCTION ApplyFixedControllerParameterTables1And2(void)
{
    volatile u8 discarded_table_selection;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 4u)
        discarded_table_selection = FirmwareReadNear8(FW_NEAR_UNKNOWN_1006);
    ApplyFixedControllerParameterTable(1u);
    ApplyFixedControllerParameterTable(2u);
}

/* RAM:fee334 */
u8 IsCurrentWriteControllerRecordIndexMarked(void)
{
    u32 write_address;
    s32 quotient;
    fw_addr_t table_address;
#define PLEXTOR_BINDING_SECTION 29
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(use_command_address != 0u)
        write_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02) + 150ul;
    else
        write_address = DispatchResetRawReadTransferState();
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, write_address);

    quotient = DivideS32ByU16TruncatingTowardZero((s32)(write_address - 145ul), 39u);
    table_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_FAFF);
    return (u8)(FirmwareRead8(FirmwareAddressIncrementLow16(table_address, (u16)quotient)) != 0u);
}

/* RAM:fe6363 */
u8 PLEXTOR_NEAR_FUNCTION TryReleaseRawReadProducerCapacityWait(void)
{
    if(FirmwareReadBit0036() == 0u && FirmwareReadBit0032() != 0u)
    {
        u16 high_water = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129C);
        u16 safety_margin = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1552);
        u32 transferred = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1455);
        u16 queued = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292);
        u32 release_threshold = (u32)(u16)(high_water - safety_margin);

        release_threshold -= transferred;
        if((u32)queued >= release_threshold)
            RestartReadAcquisitionCoroutineForProducerWait();
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B4) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B4, 0u);
        return 1u;
    }
    {
        u16 release_limit = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0C41);
        u16 queued = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292);

        if(release_limit >= queued)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B4, 0u);
            return 1u;
        }
    }
    return 0u;
}

/* RAM:fce2ae */
u8 PLEXTOR_NEAR_FUNCTION AcquireDiscPositionAtTargetWithRetryBudget(s32 target_frame_position)
{
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)target_frame_position);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 1u);
        if(AcquireDiscReadPosition() == 1u)
            return 1u;
        if(ResetDriveControlAndInitializeControllerCalibration() == 0u)
            return 0u;
    }
    return 0u;
}

/* RAM:fa4741 */
void BuildAllControllerRecordWorkspaceBankPages(void)
{
    u8 bank_index;

    for(bank_index = 0u; bank_index < 8u; ++bank_index)
    {
        u8 page_index;

        for(page_index = 0u; page_index < 8u; ++page_index)
        {
            BuildControllerRecordWorkspacePage(bank_index, page_index);
            DispatchCopyControllerBufferPageToUpperBankedRegion(FW_HUGE_UNKNOWN_2BA62F, bank_index, page_index, 0u);
        }
    }
}

/* RAM:fddd42 */
/* The value at RAM:00155a is deliberately reread after the copy. */
u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source_address, u32 byte_count)
{
    u32 buffer_offset;

    if(byte_count == 0ul)
        return 5u;
    buffer_offset = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    (void)CopyToControllerBufferWindow(source_address, buffer_offset, byte_count);
    buffer_offset = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    ExecuteControllerDataInTransferFromBufferOffset(byte_count, buffer_offset);
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 13u ? 5u : 4u;
}

/* RAM:f86a7f */
void PLEXTOR_NEAR_FUNCTION ServiceMediaState3ControllerAndQCapture(void)
{
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) >> 2) & 1u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u))
        DispatchServiceMediaQPacketCaptureWithOperatingCodeBudget();

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) >> 6) & 1u) != 0u)
        ServiceConditionalControllerOperatingDisable();
    if(FirmwareReadBit000E() != 0u)
        AttemptMediaState3QCaptureAndControllerReinitialization();
}

/* Reconstruction helper; no direct Ghidra function address. */
static void CompleteMediaCoroutineState3PortLines(void)
{
#define PLEXTOR_BINDING_SECTION 30
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:f86951 */
void PLEXTOR_NEAR_FUNCTION ProcessMediaCoroutineState3(void)
{
    s8 acquisition_status;
    u8 first_track;

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FB0, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB8, 10u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB8, 10u);
        if((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5) & 0x0040u) != 0u)
        {
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5) & 0xffbfu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB8, 60u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfbu));
    }

    State3UpdateMeasurement();
    for(;;)
    {
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) >> 2) & 1u) == 1u)
            State3SetOperating(1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        State3SelectOperatingCode();
        acquisition_status = State3AcquirePosition();

        if(acquisition_status == 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            CompleteMediaCoroutineState3PortLines();
            return;
        }
        if(acquisition_status == -1)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            return;
        }
        if(acquisition_status != 1)
        {
            for(;;)
            {
            }
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            return;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        YieldCoroutine1();
        State3UpdateMeasurement();
        State3ServiceController();

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            return;
        }
        if(State3CountdownExpired() == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC) | 0x0010u));
            CompleteMediaCoroutineState3PortLines();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) | 2u));
            return;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && State3IndexedTransitions() != 0u)
            return;
        if(State3IndexedCompletion() == 1u)
        {
            first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, first_track);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, first_track);
            State3LoadTarget();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
            return;
        }
    }
}

/* RAM:f8d8fd */
void AbortControllerCommandAndClearWords032A033A(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180B0, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
    WaitControllerPort80F8Bit7ClearWithoutTimeout();
    SubmitControllerWordPair(0u, 0x032au);
    SubmitControllerWordPair(0u, 0x033au);
}

/* RAM:f8d2e8 */
u8 PLEXTOR_NEAR_FUNCTION RunControllerCommand17AndReadPort80C7(void)
{
    u8 result = 0u;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 2u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0x17u);
    if(WaitControllerPort80F8Bit7Clear500AndYield() == 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        WaitControllerPort80F8Bit7ClearWithoutTimeout();
        result = 0x7fu;
    }
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
    if(result == 0u)
        result = WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180C7);
    return result;
}

/* RAM:f8c3f3 */
void ConfigureControllerPorts8085And8086Value80(void)
{
    u8 port_8082_value = 0xb0u;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018085, 0x80u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 1u)
        port_8082_value = 0x90u;
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018082, port_8082_value);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x18u)
        UPDATE_CONTROLLER_WORD_035F_FOR_PORT_SETUP();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018086, 0x80u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x80u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, 0x43u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    }
    APPLY_CONTROLLER_D1_FOR_PORT_SETUP();
    CONFIGURE_CONTROLLER_D0_D1_D3_FOR_PORT_SETUP();
}

/* RAM:f8c6ef */
void ConfigureControllerPort8085Value80And8086Value00(void)
{
    u8 setup_flags;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018085, 0x80u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x18u)
        UPDATE_CONTROLLER_WORD_035F_FOR_PORT_SETUP();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018086, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018082, 0xb0u);
    setup_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0);
    if((setup_flags & 0x80u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, 0x41u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    }
    setup_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0);
    if((setup_flags & 0x20u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6) | 0x10u));
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    }
    APPLY_CONTROLLER_D1_FOR_PORT_SETUP();
    CONFIGURE_CONTROLLER_D0_D1_D3_FOR_PORT_SETUP();
}

/* RAM:f8c778 */
void ConfigureControllerPorts8085And8086Value80AndYield(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018085, 0x80u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x18u)
        UPDATE_CONTROLLER_WORD_035F_FOR_PORT_SETUP();
    YieldCoroutine1();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018086, 0x80u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x80u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, 0x43u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    }
    CONFIGURE_CONTROLLER_D0_D1_D3_FOR_PORT_SETUP();
}

/* RAM:f8c887 */
s16 ComputeControllerWord0255TargetFromCalibrationByte(void)
{
    s16 target = (s16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC2) - 0x80u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 4u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC2) != 0xffu)
    {
        s16 step = (s16)(4 / (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));

        if(target <= (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8))
        {
            target = (s16)(target + step);
            if(target >= (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8))
                target = (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8);
        }
        else
        {
            target = (s16)(target - step);
            if(target <= (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8))
                target = (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8);
        }
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC2) == 0xffu)
        target = (s16)0x8000u;
    return target;
}

/* RAM:fbb22d */
void PLEXTOR_NEAR_FUNCTION FinalizeDecodedTocLeadOutRecord(void)
{
    u8 last_track;
    fw_addr_t record;

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_12C1, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12D4));
    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, last_track);
    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A);
    record = (u16)(0xf2b6u + (u16)((u8)(last_track + 1u) * 6u));
    FirmwareWrite8(record, 0xaau);
    FirmwareWrite8(record + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F546));
    FirmwareWrite8(record + 2u, 0u);
    FirmwareWrite8(record + 3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F54D));
    FirmwareWrite8(record + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F54E));
    FirmwareWrite8(record + 5u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F54F));
}

/* Reconstruction helper; no direct Ghidra function address. */
static void ClearElevenByteRuntimeStateRecord(u8 record_index)
{
    u16 record = (u16)(0x0ccbu + (u16)(record_index * 11u));

    FirmwareWriteNear8((u16)(record + 7u), 0u);
    FirmwareWriteNear8((u16)(record + 9u), (u8)(FirmwareReadNear8((u16)(record + 9u)) & 0xfeu));
    FirmwareWriteNear8((u16)(record + 10u), 0u);
    FirmwareWriteNear8((u16)(record + 8u), 0u);
    FirmwareWriteNear8((u16)(record + 6u), 0u);
    FirmwareWriteNear8((u16)(record + 5u), 0u);
    FirmwareWriteNear8((u16)(record + 4u), 0u);
    FirmwareWriteNear8((u16)(record + 3u), 0u);
    FirmwareWriteNear8((u16)(record + 2u), 0u);
    FirmwareWriteNear8((u16)(record + 1u), 0u);
    FirmwareWrite8(record, 0u);
}

/* RAM:fccfdd */
void ClearElevenByteRuntimeStateRecordAndAdvanceIndex(u8 first_record_index)
{
    do
    {
        ClearElevenByteRuntimeStateRecord(first_record_index);
        ++first_record_index;
    } while(first_record_index <= 1u);
}

/* RAM:fcd05e */
void ClearTwoElevenByteRuntimeStateRecordsFromIndex(u8 first_record_index)
{
    while(first_record_index <= 1u)
    {
        ClearElevenByteRuntimeStateRecord(first_record_index);
        ++first_record_index;
    }
}

/* RAM:fccfd8 */
/* Native XRL B,B then LJMP RAM:fcd05e. */
void ClearTwoElevenByteRuntimeStateRecords(void)
{
    ClearTwoElevenByteRuntimeStateRecordsFromIndex(0u);
}

static u8 FinalizeType2MipRecordComparisonStateBody(u8 matched_record_count, u8 residual_count)
{
    if(matched_record_count == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x80u));
    }
    else if(matched_record_count == 0u)
    {
        u32 first_payload = FirmwareReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA5A3);

        residual_count = (u8)first_payload;

        if(first_payload == 0ul)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x80u));
        }
        else
        {
            u32 second_payload = FirmwareReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA59F);

            first_payload = FirmwareReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA5A3);
            if(first_payload == second_payload)
                FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x7fu));
            else
                FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x80u));
        }
    }
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfbu));
    return residual_count;
}

/* RAM:fcd912 */
void PLEXTOR_NEAR_FUNCTION FinalizeType2MipRecordComparisonState(u8 matched_record_count)
{
    (void)FinalizeType2MipRecordComparisonStateBody(matched_record_count, 0u);
}

/* Reconstruction helper; no direct Ghidra function address. */
u8 FinalizeType2MipRecordComparisonStateForBoundaryBuild(u8 matched_record_count, u8 unmatched_residual_count)
{
    return FinalizeType2MipRecordComparisonStateBody(matched_record_count, unmatched_residual_count);
}

/* RAM:fc94ad */
s32 FindLastFullTocPointB0FrameAtOrBeforeTargetPlus40(s32 target_frame_position)
{
    u16 descriptor_index;
    s32 last_frame_position = 0;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 1u)
        return 0;

    for(descriptor_index = 1u; descriptor_index < FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_12DC); descriptor_index++)
    {
        fw_addr_t descriptor_address = FullTocDescriptorAddress(descriptor_index);
        s32 frame_position;

        if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor_address, 3u)) != 0xb0u)
            continue;
        frame_position = ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(descriptor_address, 4u));
        if(frame_position > (s32)(target_frame_position + (s32)40))
            return last_frame_position;
        last_frame_position = frame_position;
    }
    return last_frame_position;
}

/* RAM:fc93db */
void PLEXTOR_NEAR_FUNCTION AdjustReadAcquisitionTargetNearFullTocPointB0(void)
{
    s32 target_frame_position;
    s32 point_b0_frame_position;
    s32 distance;
    u16 tolerance;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        return;
    target_frame_position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);
    point_b0_frame_position = FindLastFullTocPointB0FrameAtOrBeforeTargetPlus40(target_frame_position);
    if(point_b0_frame_position > target_frame_position)
        distance = (s32)(point_b0_frame_position - target_frame_position);
    else
        distance = (s32)(target_frame_position - point_b0_frame_position);

    tolerance = (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4);
    if(distance <= (s32)(tolerance * 2u))
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)(target_frame_position + (s32)(tolerance * 2u)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFA, 2u);
    }
    else if(distance <= (s32)(tolerance * 5u) && point_b0_frame_position > target_frame_position)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)(target_frame_position + (s32)(tolerance * 5u)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFA, 1u);
    }
}

/* RAM:fe96e2 */
u8 SelectRawReadQTrackNumberForCurrentOperation(void)
{
    u16 descriptor_index;
    fw_addr_t descriptor_address;
    u8 result;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u)
        {
            result = 0;
        }
        else
        {
            descriptor_index = 0;
            do
            {
                descriptor_address = FullTocDescriptorAddress(descriptor_index);
                if(FirmwareRead8(descriptor_address) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9))
                    break;
                descriptor_index++;
            } while(1);
            do
            {
                descriptor_address = FullTocDescriptorAddress(descriptor_index);
                if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor_address, 3u)) <= 0x99u)
                    break;
                descriptor_index++;
            } while(1);
            result = FirmwareRead8(FirmwareAddressIncrementLow16(FullTocDescriptorAddress(descriptor_index), 3u));
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 0u)
    {
        result = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35);
    }
    else
    {
        u8 operation_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_E600);

        if(operation_type == 5u)
        {
            result = 0xffu;
        }
        else if(operation_type != 6u)
        {
            result = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35);
        }
        else
        {
            descriptor_index = 0;
            do
            {
                descriptor_address = FullTocDescriptorAddress(descriptor_index);
                if(FirmwareRead8(descriptor_address) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9))
                    break;
                descriptor_index++;
            } while(1);
            do
            {
                descriptor_address = FullTocDescriptorAddress(descriptor_index);
                if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor_address, 3u)) <= 99u)
                    break;
                descriptor_index++;
            } while(1);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD63) == 2u)
            {
                result = FirmwareRead8(FirmwareAddressIncrementLow16(FullTocDescriptorAddress(descriptor_index), 3u));
            }
            else
            {
                do
                {
                    descriptor_index++;
                    descriptor_address = FullTocDescriptorAddress(descriptor_index);
                } while(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor_address, 3u)) <= 99u);
                result = (u8)(FirmwareRead8(FirmwareAddressIncrementLow16(FullTocDescriptorAddress((u16)(descriptor_index - 1u)), 3u)) + 1u);
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CC, result);
    return result;
}

/* RAM:fc1a28 */
u8 PLEXTOR_NEAR_FUNCTION FindLastTrackTableIndexForSessionByte(u8 session_byte)
{
    u8 record_index;

    if(session_byte == 0u)
        return 1u;

    record_index = 1u;
    while(record_index <= 99u)
    {
        u16 offset = (u16)((record_index << 4) + (record_index << 1) + record_index);

        if(FirmwareReadNear8((u16)(0xdd92u + offset)) == session_byte)
        {
            do
            {
                u8 next_index;
                u16 next_offset;

                record_index++;
                next_index = record_index;
                next_offset = (u16)((next_index << 4) + (next_index << 1) + next_index);
                if(FirmwareReadNear8((u16)(0xdd7fu + next_offset)) != FirmwareReadNear8((u16)(0xdd92u + next_offset)))
                    break;
            } while(1);
            return (u8)(record_index - 1u);
        }
        record_index++;
    }
    return (u8)(record_index - 1u);
}

/* RAM:fc4f18 */
void SnapshotTrackTableTailState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E4EC, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E4ED, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E));
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_E4EE, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A));
}

/* RAM:fb5240 */
void StoreWorkingTrackAtEmptyTrackTableTail(void)
{
    u8 working_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35);
    u8 tail_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);

    if(working_track == tail_index)
    {
        SnapshotTrackTableTailState();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E4EC, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) + 1u));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E4EC, (u8)(working_track + 1u));
    }

    if(tail_index != 100u && FirmwareReadNear8((u16)(0xdd87u + (u16)(tail_index << 4) + (u16)(tail_index << 1) + tail_index)) == 0u)
    {
        u16 record = (u16)(0xdd80u + (u16)(working_track << 4) + (u16)(working_track << 1) + working_track);

        FirmwareWrite8(record, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36));
        ConvertFramePositionToBinaryMsfTupleFar((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D), (u16)(record + 1u));
        ConvertFramePositionToBinaryMsfTupleFar((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41), (u16)(record + 4u));
        FirmwareWriteNear8((u16)(record + 8u), FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38));
    }
}

/* RAM:fc627c */
u8 PLEXTOR_NEAR_FUNCTION ReadMainDataRingByteAtCursorPlus16(void)
{
    u32 ring_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_155E);
    u32 cursor = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_15D8);
    u8 value;

    if(cursor > ring_size)
    {
        cursor -= ring_size;
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_15D8, cursor);
    }
    value = FirmwareRead8(cursor + 0x10ul);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_15D8, cursor + 1ul);
    return value;
}

/* RAM:f93aaf */
void SetRawReadTransferCountdownToOneNearLimit(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        s16 limit = (s16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 1u);
        s16 countdown = (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24);

        if(countdown <= limit)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, 1u);
    }
}

/* RAM:fc94a7 */
void PLEXTOR_NEAR_FUNCTION ClearPendingReadAcquisitionTargetNudge(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFA, 0);
}

/* RAM:fc90e5 */
u8 PLEXTOR_NEAR_FUNCTION IsReadAcquisitionFrameDistanceAccepted(void)
{
    s32 distance = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE3) != 0u)
        return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE2) == 1u && distance <= 3);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED6) != 0u)
        return (u8)(distance == 0);
    return (u8)(distance <= (s32)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4));
}

/* RAM:fc912a */
u8 PLEXTOR_NEAR_FUNCTION IsReadAcquisitionFrameDistanceWithinFineTolerance(void)
{
    s32 distance = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE3) != 0u)
        return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE2) == 1u && distance <= 3);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED6) != 0u)
        return (u8)(distance == 0);
    return (u8)(distance <= 9);
}

/* RAM:fc9194 */
u8 PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionNearTargetWithinTwelveFrames(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE3) == 0u && (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5) <= 12)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE2, 1u);
        return 1u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE2, 0);
    return 0;
}

/* RAM:fc9169 */
u8 PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionNearTargetFromDynamicTolerance(void)
{
    s32 tolerance = (s32)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4) + 3;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE3) == 0u && (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5) <= tolerance)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE2, 1u);
        return 1u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE2, 0);
    return 0;
}

/* RAM:fc91b5 */
/* The native unsigned-divider
 * result is held in DR4; this routine deliberately returns only WR6, its
 * low sixteen bits. The source paths establish a nonzero radial-model value. */
u16 PLEXTOR_NEAR_FUNCTION CalculateFineReadAcquisitionMoveMagnitude(void)
{
    u32 distance = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5);
    u16 radial_model_value = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0EE0);
    u8 move_direction = FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE3);
    u16 magnitude = (u16)DivideU32QuotientWithoutRuntimeLibrary(distance << 8, (u32)radial_model_value);

    if(magnitude < 21u)
    {
        if(move_direction == 0u)
        {
            if(distance > 3u)
                magnitude = (u16)DivideU32QuotientWithoutRuntimeLibrary((distance - 3u) << 8, (u32)radial_model_value);
        }
        else
        {
            magnitude = (u16)DivideU32QuotientWithoutRuntimeLibrary((distance + 3u) << 8, (u32)radial_model_value);
        }
    }
    return move_direction == 0u ? magnitude : (u16)(magnitude + 1u);
}

/* RAM:fc9226 */
u32 PLEXTOR_NEAR_FUNCTION CalculateReadAcquisitionAbsoluteFrameDistance(void)
{
    s32 target = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    s32 current = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);

    if(current < target)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE3, 1u);
        return (u32)(target - current);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE3, 0);
    return (u32)(current - target);
}

/* RAM:fc86c7 */
u8 PLEXTOR_NEAR_FUNCTION ProcessSpecialReadAcquisitionPositioningStep(void)
{
    u16 move_distance;

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5, CalculateReadAcquisitionAbsoluteFrameDistance());
    if(IsReadAcquisitionFrameDistanceWithinFineTolerance() != 0u)
        return 1u;

    if(UpdateReadAcquisitionNearTargetWithinTwelveFrames() != 0u)
    {
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0EDC, 0u);
        return 0u;
    }

    move_distance = DispatchCalculateRadialModelDistanceForSpecialPositioning();
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0EDC, move_distance);
    YieldCoroutine1();
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0EDE, FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0EDC));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3F) + 1u));
    return DispatchIssueReadAcquisitionPickupMove();
}

/* RAM:fcec77 */
u16 CalculateRadialModelSpanFromMeasurement(u16 measurement)
{
    u32 normalized;
    u32 scaled;

    normalized = MultiplyU32Low(250ul, (u32)measurement);
    normalized = DivideU32QuotientNative(normalized, (u32)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F99C));
    scaled = MultiplyU32Low(normalized, normalized);
    scaled -= 62500ul;
    scaled = MultiplyU32Low(0x4305ul, scaled);
    scaled = DivideU32QuotientNative(scaled, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5));
    scaled = DivideU32QuotientNative(scaled, 10ul);
    return (u16)scaled;
}

/* RAM:fcedd7 */
u16 MapLeadInSecondsMagnitudeToRadialModelValue(u16 lead_in_seconds_magnitude)
{
    u32 product = MultiplyU32Low(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5), (u32)lead_in_seconds_magnitude);
    u32 root = IntegerSquareRootU32(0x04cb0000ul - product);
    u32 scaled = MultiplyU32Low((u32)(u16)root, 0x01beul) >> 8;
    u16 result = (u16)(0x3d09u - (u16)scaled);

    if((s16)result < 0)
        result = 0u;
    return result;
}

/* RAM:fced65 */
u16 MapFrameSecondsToRadialModelValue(u16 frame_seconds)
{
    u32 scaled = MultiplyU32Low(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5), 10000ul) >> 16;
    u16 radial_calibration_span;
    u16 root_delta;

    scaled = MultiplyU32Low(scaled, 0x2c24ul);
    scaled = DivideU32QuotientNative(scaled, 0x0163ul);
    scaled = MultiplyU32Low(scaled, (u32)frame_seconds);
    scaled = DivideU32QuotientNative(scaled, 1000ul);
    radial_calibration_span = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9);
    scaled = MultiplyU32Low(scaled, (u32)radial_calibration_span);
    scaled = IntegerSquareRootU32(scaled + 0x2540be40ul);
    root_delta = (u16)((u16)scaled - 25000u);
    scaled = MultiplyU32Low((u32)root_delta, 1000ul);
    return (u16)DivideU32QuotientNative(scaled, (u32)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9));
}

/* RAM:fc9254 */
u16 PLEXTOR_NEAR_FUNCTION CalculatePositiveRadialModelDistanceToReadTarget(void)
{
    u16 current_value = MapFrameSecondsToRadialModelValue((u16)DivideS32QuotientNative((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235), 75l));
    u16 target_value;

    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0ED3, current_value);
    target_value = MapFrameSecondsToRadialModelValue((u16)DivideS32QuotientNative((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B), 75l));
    return target_value < current_value ? (u16)(current_value - target_value) : (u16)(target_value - current_value);
}

/* RAM:feb097 */
u8 AcquireRawReadFramesAndPopulateTdiPayload(s32 frame_count)
{
    fw_addr_t controller_buffer_address;

    if(frame_count < 0)
        return 1u;
    controller_buffer_address = ResumeMainDataRingCursorFromLatestDescriptor();
    if(AcquireRawReadFrameSequenceWithCurrentQPacket(frame_count, 6u) == 0u)
        return 0u;
    POPULATE_RAW_TDI_RANGE_FOR_ACQUIRE_WRAPPER(controller_buffer_address, (u32)frame_count);
    return 1u;
}

/* RAM:fc909c */
void PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionFrameDistanceFromMsfTuple(void)
{
    s32 packet_frame = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_1241);
    s32 previous_frame = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0EF1);

    if(packet_frame > previous_frame)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0EF1, (u32)packet_frame);
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5, FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u ? 400ul : 2400ul);
    }
    else
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0EE5, (u32)(previous_frame - packet_frame));
    }
}

/* RAM:faad79 */
u32 SetMainDataRingCursorAtSectorOffset(u8 sector_offset, u32 base_cursor)
{
    u32 offset = MultiplyU32Low(0x930ul, (u32)sector_offset);
    u32 cursor = base_cursor + offset;
    u32 ring_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_155E);

    if(cursor >= ring_size)
        cursor -= ring_size;
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_15D8, cursor);
    return cursor;
}

/* RAM:faad66 */
u8 GetReadPipelineTargetThirtyNineFrameRemainder(void)
{
    s32 dividend = (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598) - 150ul);
    u32 magnitude = dividend < 0 ? 0ul - (u32)dividend : (u32)dividend;
    u32 quotient = DivideU32QuotientWithoutRuntimeLibrary(magnitude, 39ul);
    u32 remainder = magnitude - MultiplyU32Low(quotient, 39ul);

    if(dividend < 0)
        return (u8)(0ul - remainder);
    return (u8)remainder;
}

/* RAM:f8f1d2 */
u8 IsMediaState2AlternateRawReadPipelineActive(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 1u);
}

/* RAM:f8f1e6 */
u8 IsReadAcquisitionPacketAcceptedForCurrentTocState(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == 0xffu || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u))
        return 1u;
    return FirmwareReadBit0020();
}

/* Reconstruction helper; no direct Ghidra function address. */
static void MaskMediaStateFlagsWord(u16 mask)
{
    u16 value = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_0FF5);

    value &= mask;
    FirmwareWriteNearU16(FW_NEAR_UNKNOWN_0FF5, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 31
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fbff86 */
u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u)
            return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u);
        if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) == 0u)
            return 1u;
        RestartReadAcquisitionCoroutineForProducerWait();
    }

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u)
    {
        ServicePendingHostCommandWhenOperationFlagsPermit();
        YieldCoroutine0();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u);
}

/* RAM:fe8ae5 */
u8 AcquireRawReadRangeAndWait(s32 start_frame_address, u32 frame_count)
{
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, (u32)start_frame_address);
    if(AcquireRawSectorSequenceForRangeAndWait(frame_count + 1ul) == 0u)
        return 0u;

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u)
    {
        ServicePendingHostCommandWhenOperationFlagsPermit();
        YieldCoroutine0();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        return 1u;

    ReportReadPipelineFailure(7u);
    return 0u;
}

/* RAM:fe8b25 */
u8 PLEXTOR_NEAR_FUNCTION BuildAndAcquireRawReadQRecordTable(u8 first_record_index)
{
    if(PopulateRawReadQRecordTableFromIndex(first_record_index) != 0u)
    {
        u32 expected_sector_count = (u32)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_8010);
        u16 q_record_count = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_800E);

        if(AcquireRawSectorsFromQRecordTable(expected_sector_count, q_record_count) != 0u)
            return 1u;
    }

    SetSenseData(3u, 0x7304u);
    SetSenseInformation(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598));
    return 0u;
}

/* RAM:fbf67c */
void InitializeRawReadSpeedForCurrentDriveState(void)
{
    u8 speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) != 1u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 24)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 24u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) > 0 && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 8)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
    }
    else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) > 0 && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 32)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 32u);
    }

    if(FirmwareReadBit000E() != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
    if(ReadBit0013ForReadTargetAdjustment() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) >= 4u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) >> 1) & 1u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) >= 10u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < 8u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
    }

    speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F3A, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
}

/* RAM:fbf2d8 */
void DispatchReadAcquisitionRequest5ForPipelineTarget(void)
{
    u32 target;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) != 0u)
        InitializeRawReadSpeedForCurrentDriveState();
    target = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_159C);
    target += FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598);
    target += FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0F10);
    if((s32)target <= 0)
        target = 1ul;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 5u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E, target);
    DispatchPendingReadAcquisitionRequest();
}

/* RAM:fbfe07 */
u8 CheckQueuedRawReadDescriptorBatchStatus(void)
{
    u16 queued_count = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292);
    u32 requested_count = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06);

    if((u32)queued_count < requested_count)
    {
        u16 descriptor_address;

        if(FirmwareReadBit0008() == 0u)
            return 0u;
        if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) == 0u)
            return 0xfeu;
        descriptor_address = GetLatestQueuedRawSectorDescriptorAddress();
        return FirmwareReadNear8((u16)(descriptor_address + 0x10u)) == 0u ? 0xfeu : 0u;
    }

    DispatchReadAcquisitionRequest5ForPipelineTarget();
    YieldCoroutine0();
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_1298, FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1294));
    {
        u16 descriptor_index = 0u;

        while(descriptor_index < FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292))
        {
            u16 descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
            u16 cursor;

            if(FirmwareReadNear8((u16)(descriptor_address + 0x10u)) != 0u)
                return 0u;
            cursor = (u16)((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1298) + 1u) & FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A));
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_1298, cursor);
            descriptor_index++;
        }
    }
    return 1u;
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 32
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fe2328 */
void UpdateRawReadQPacketControlBit(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 0u || write_type == 1u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 0x04u) == 0u)
            return;
    }
    else
    {
        u16 table_index;
        u16 record_address;

        if(write_type != 2u)
            return;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1273) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1273) == 0xaau)
            return;

        table_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_128E);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x02u) == 0u)
        {
            u8 track_number = PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1273));

            do
            {
                record_address = (u16)(0xb4bdu + (u16)(table_index << 3));
                if((FirmwareRead8(record_address) & 0x0fu) == 1u && FirmwareReadNear8((u16)(record_address + 1u)) == track_number)
                    break;
                table_index = (u16)(table_index + 1u);
                FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_128E, table_index);
            } while(1);
        }

        record_address = (u16)(0xb4bdu + (u16)(table_index << 3));
        if(FirmwareReadNear8((u16)(record_address + 4u)) == 0u)
            return;
        if((FirmwareRead8(record_address) & 0x20u) != 0u)
            return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1289) != 0u)
        return;

    /* The original signed divide only uses a zero remainder test.  A divisor
     * of four makes the low two bits identical for all signed 32-bit inputs. */
    if((FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598) & 3ul) != 0ul)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1288, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1288) != 1u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1288) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) | 0x20u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0xdfu));
}

/* RAM:fe2498 */
void ReportReadPipelineFailure(u8 failure_code)
{
    LatchFirstReadFailureContext();
    MapReadPipelineFailureCodeToSenseData(failure_code);
    SetSenseInformation(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598));
}

/* RAM:fe24b0 */
#define PLEXTOR_BINDING_SECTION 59
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

void MapReadPipelineFailureCodeToSenseData(u8 failure_code)
{
    u8 preparation_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0);

    if(preparation_status != 4u || !READ_PIPELINE_FAILURE_BIT0028)
    {
        fw_addr_t sense_table_address = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9259) + ((fw_addr_t)preparation_status << 1);

        if(READ_PIPELINE_FAILURE_BIT0031)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 1u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
        SetSenseData(FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9251) + (fw_addr_t)preparation_status),
            (u16)(((u16)FirmwareRead8(sense_table_address) << 8) | FirmwareRead8(FirmwareAddressIncrement24(sense_table_address, 1u))));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 1u);
    if(failure_code == 0u)
        failure_code = 1u;

    switch(failure_code)
    {
    case 1u:
    case 2u:
    case 6u:
    case 10u:
        SetSenseData(3u, 0x0c07u);
        return;
    case 3u:
        SetSenseData(3u, 0x7304u);
        return;
    case 4u:
        SetSenseData(3u, 0x7201u);
        return;
    case 5u:
        SetSenseData(3u, 0x7202u);
        return;
    case 11u:
        SetSenseData(3u, 0x0c08u);
        return;
    default:
        break;
    }

    SetSenseData(FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9255)),
        (u16)(((u16)FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9261)) << 8) | FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9262))));
}

/* RAM:fe17b1 */
void WriteControllerRegisterBlock25FromMemory(fw_addr_t source_address)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, FirmwareRead8(source_address));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, FirmwareRead8(FirmwareAddressIncrementLow16(source_address, 1u)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, FirmwareRead8(FirmwareAddressIncrementLow16(source_address, 2u)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, FirmwareRead8(FirmwareAddressIncrementLow16(source_address, 3u)));
}

/* RAM:fe0b15 */
void CopyFirstTrackStartMsfFromRawReadQRecordTable(u16 destination)
{
    u16 a0_record_index = 0u;
    u16 first_track_record_index = 0u;
    u8 first_track_selector;

    while(FirmwareReadNear8((u16)(0x8014u + (u16)(a0_record_index * 10u))) != 0xa0u)
        a0_record_index = (u16)(a0_record_index + 1u);

    first_track_selector = FirmwareRead8((u16)(0x8019u + (u16)(a0_record_index * 10u)));
    while(FirmwareReadNear8((u16)(0x8014u + (u16)(first_track_record_index * 10u))) != first_track_selector)
        first_track_record_index = (u16)(first_track_record_index + 1u);

    CopyThreeBytes16(destination, (u16)(0x8019u + (u16)(first_track_record_index * 10u)));
}

/* Reconstruction helper; no direct Ghidra function address. */
static u16 FirmwareReadBigEndianU16FarLow16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void FirmwareWriteBigEndianU16NearRawQ(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:fe0b57 */
void MergeStagedRawReadQRecordTablesIntoRawReadTable(void)
{
    u16 primary_count = FirmwareReadBigEndianU16FarLow16(FW_HUGE_UNKNOWN_2BC119);
    u16 secondary_count = FirmwareReadBigEndianU16FarLow16(FW_HUGE_UNKNOWN_2BC517);
    u16 record_index;

    for(record_index = 0u; record_index < primary_count; record_index++)
    {
        fw_addr_t source = FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_2BC11B, (u16)(record_index * 10u));
        u16 destination = (u16)(0x8012u + (u16)(record_index * 10u));
        u8 field_index;

        for(field_index = 0u; field_index < 10u; field_index++)
            FirmwareWriteNear8((u16)(destination + field_index), FirmwareRead8(FirmwareAddressIncrementLow16(source, field_index)));
    }

    for(record_index = 0u; record_index < secondary_count; record_index++)
    {
        u16 destination_index = (u16)(primary_count + record_index);
        fw_addr_t source = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2BC519, (u16)(record_index * 10u));
        u16 destination = (u16)(0x8012u + (u16)(destination_index * 10u));
        u8 field_index;

        for(field_index = 0u; field_index < 10u; field_index++)
            FirmwareWriteNear8((u16)(destination + field_index), FirmwareRead8(FirmwareAddressIncrementLow16(source, field_index)));
    }

    FirmwareWriteBigEndianU16NearRawQ(FW_NEAR_UNKNOWN_8008, primary_count);
    FirmwareWriteBigEndianU16NearRawQ(FW_NEAR_UNKNOWN_800C, (u16)(primary_count + secondary_count));
    FirmwareWriteBigEndianU16NearRawQ(FW_NEAR_UNKNOWN_800A, (u16)(primary_count + secondary_count));
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteMode2SubheaderWords(fw_addr_t destination, u32 value)
{
    FirmwareWriteU32(destination, value);
}

/* RAM:fe16a8 */
void PopulateConfiguredMode2SubheaderWhenRequired(fw_addr_t raw_sector_address, u8 raw_sector_source_kind)
{
    u8 type_code;
    fw_addr_t destination;
    u32 mode2_subheader;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u) && raw_sector_source_kind == 5u)
        type_code = MapWorkingTrackFieldsToStatusCode();
    else
        type_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);

    if(type_code < 10u || (s8)type_code > (s8)13)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1280) != 0u && raw_sector_source_kind == 4u)
        return;

    destination = FirmwareAddressIncrement24(raw_sector_address, 0x10u);
    mode2_subheader = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C71);
    WriteMode2SubheaderWords(destination, mode2_subheader);
    mode2_subheader = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C71);
    WriteMode2SubheaderWords(FirmwareAddressIncrement24(destination, 4u), mode2_subheader);
}

/* RAM:fa44a8 */
void CountRawReadTargetPositionMatch(void)
{
    s32 target_frame_position;
    s32 current_frame_position;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u)
        return;
    if((WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180F9) & 4u) == 0u)
        return;

    /* The controller port is deliberately read again after the capture flag. */
    (void)WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180F9);
    target_frame_position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598);
    current_frame_position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    if(current_frame_position < target_frame_position)
        return;
    if(current_frame_position > target_frame_position)
        return;
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_122F, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_122F) + 1u));
}

/* RAM:fa4517 */
u8 AdvanceRawReadMatchedTargetCursorIfAvailable(void)
{
    u16 cursor_high;
    u16 cursor_low;

    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_122F) == 0u)
        return 1u;

    /*
     * DR0 is set to RAM:2bc6ff.  The native code reads its low word before
     * its high word, increments only that low word, then writes low and high
     * words in the same order.  Keep those volatile accesses explicit.
     */
    cursor_low = (u16)(FirmwareReadU16(FW_HUGE_UNKNOWN_2BC701));
    cursor_high = (u16)(FirmwareReadU16(FW_HUGE_UNKNOWN_2BC6FF));
    cursor_low = (u16)(cursor_low + 1u);
    FirmwareWriteU16(FW_HUGE_UNKNOWN_2BC701, cursor_low);
    FirmwareWriteU16(FW_HUGE_UNKNOWN_2BC6FF, cursor_high);
    return 0u;
}

/* RAM:fa44f3 */
void ClearControllerPorts80F8And80FAIfRawReadEnabled(void)
{
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u)
        return;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180FA, 4u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180F8, 0u);
}

/* RAM:fddbed */
void AppendMeasurementSnapshotRecord(void)
{
    fw_addr_t output_address;
    u32 snapshot_cursor;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 2u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_151F, FirmwareReadNear8(FW_NEAR_UNKNOWN_123D));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1520) == FirmwareReadNear8(FW_NEAR_UNKNOWN_151F))
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1520, FirmwareReadNear8(FW_NEAR_UNKNOWN_151F));

    snapshot_cursor = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1517);
    /* ADD WR4,#0x1c changes the upper word only; WR6 remains the low word. */
    output_address = ((fw_addr_t)(u16)((u16)(snapshot_cursor >> 16) + 0x1cu) << 16) | (fw_addr_t)(u16)snapshot_cursor;
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);

    FirmwareWrite8(output_address, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_123D)));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 1u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);
    FirmwareWrite8(output_address, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_123E)));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 1u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);
    FirmwareWrite8(output_address, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_123F)));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 3u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);

    FirmwareWrite8(output_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD3));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 2u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);
    FirmwareWrite8(output_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD0));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 1u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);

    FirmwareWrite8(output_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_1521));
    FirmwareWrite8(FirmwareAddressIncrement24(output_address, 1u), FirmwareReadNear8(FW_NEAR_UNKNOWN_1522));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 2u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);
    FirmwareWrite8(output_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_1523));
    FirmwareWrite8(FirmwareAddressIncrement24(output_address, 1u), FirmwareReadNear8(FW_NEAR_UNKNOWN_1524));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 3u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);
    FirmwareWrite8(output_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD1));
    output_address = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_151B);
    output_address = FirmwareAddressIncrement24(output_address, 1u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_151B, (u32)output_address);
    FirmwareWrite8(output_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBC3));
    FirmwareWrite8(FirmwareAddressIncrement24(output_address, 1u), FirmwareReadNear8(FW_NEAR_UNKNOWN_FBC4));

    /* ADD DR4,#0x10 followed by ANL WR4,#0 preserves only the low word. */
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1517, (u32)(u16)(snapshot_cursor + 0x10ul));
}

/* RAM:fe02f5 */
u8 PLEXTOR_NEAR_FUNCTION FindAdjacentRawReadQRecordClass(u16 record_index, u8 scan_forward)
{
    u16 record_address;
    u8 record_type;

    for(;;)
    {
        record_address = (u16)(0xb4bdu + (u16)(record_index << 3));
        record_type = FirmwareRead8(record_address);
        if((record_type & 0x0fu) == 1u)
        {
            switch(FirmwareReadNear8((u16)(record_address + 3u)) & 0x30u)
            {
            case 0x10u:
                return 1u;
            case 0x20u:
            case 0x30u:
                return 2u;
            default:
                return 0u;
            }
        }
        if(scan_forward != 0u)
        {
            record_index = (u16)(record_index + 1u);
        }
        else
        {
            record_index = (u16)(record_index - 1u);
            if(record_index == 0xffffu)
                return 0u;
        }
    }
}

/* RAM:fe036a */
u16 FindNextType1RawTrackRecordWithIndex1(u16 record_index)
{
    u16 record_address;

    do
    {
        do
        {
            record_index = (u16)(record_index + 1u);
            record_address = (u16)(0xb4bdu + (u16)(record_index << 3));
        } while(FirmwareReadNear8((u16)(record_address + 2u)) != 1u);
    } while((FirmwareRead8(record_address) & 0x0fu) != 1u);
    return record_index;
}

/* RAM:fe0475 */
u16 PLEXTOR_NEAR_FUNCTION FindType1RawTrackRecordIndexByTrackNumberOrLeadOut(u8 track_number)
{
    u16 record_index = 0u;

    for(;;)
    {
        u16 record_address = (u16)(0xb4bdu + (u16)(record_index << 3));
        u8 candidate_track = FirmwareReadNear8((u16)(record_address + 1u));

        if(candidate_track == 0xaau)
            return record_index;
        if(candidate_track == track_number && FirmwareReadNear8((u16)(record_address + 2u)) == 1u && (FirmwareRead8(record_address) & 0x0fu) == 1u)
            return record_index;
        record_index = (u16)(record_index + 1u);
    }
}

/* RAM:fe0ff8 */
u8 PLEXTOR_NEAR_FUNCTION IsIsrcAvailableForCurrentTrack(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    u16 record_index;

    if(write_type == 0u || write_type == 1u)
        return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C61) & 0x80u) != 0u);
    if(write_type != 2u)
        return 0u;

    record_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_14A3);
    if(record_index < FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_14B0))
        record_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_14B0);
    if(record_index == 0u)
        return 0u;

    for(;;)
    {
        u16 record_address;
        u8 candidate_track;

        if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_B4BB) <= record_index)
            return 0u;
        record_address = (u16)(0xb4bdu + (u16)(record_index << 3));
        candidate_track = FirmwareReadNear8((u16)(record_address + 1u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14A8) < candidate_track)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14A8) == candidate_track && (FirmwareRead8(record_address) & 0x0fu) == 3u)
        {
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_14B0, record_index);
            return 1u;
        }
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_14B0, record_index);
        record_index = (u16)(record_index + 1u);
    }
}

/* RAM:fe1319 */
u8 PLEXTOR_NEAR_FUNCTION SetRawReadQPacketMsfTuples(s32 relative_frame_position, s32 absolute_frame_position)
{
    ConvertFramePositionToBinaryMsfTuple(absolute_frame_position, 0x1279ul);
    ConvertThreeBinaryBytesToPackedBcd(0x1279u);
    ConvertFramePositionToBinaryMsfTuple(relative_frame_position, 0x1275ul);
    ConvertThreeBinaryBytesToPackedBcd(0x1275u);
    return 1u;
}

/* RAM:fe0e0a */
void LoadMediaCatalogNumberCharacters(void)
{
    LoadMediaCatalogNumberCharactersFromBIndex(0u);
}

/* RAM:fe0e17 */
void LoadMediaCatalogNumberCharactersFromBIndex(u8 first_clear_index)
{
    u8 index;
    u8 write_type;

    /* B is a caller-provided loop index in the canonical body. */
    for(index = first_clear_index; index < 13u; index++)
        FirmwareWriteNear8((u16)(0x141au + index), 0u);

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 0u || write_type == 1u)
    {
        for(index = 0u; index < 13u; index++)
            FirmwareWriteNear8((u16)(0x141au + index), FirmwareReadNear8((u16)(0x0c51u + index)));
        return;
    }
    if(write_type != 2u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_141A, FirmwareReadNear8(FW_NEAR_UNKNOWN_B4BD));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1421, FirmwareReadNear8(FW_NEAR_UNKNOWN_B4C5));
    for(index = 1u; index < 7u; index++)
        FirmwareWriteNear8((u16)(0x141au + index), FirmwareReadNear8((u16)(0xb4beu + index)));
    /* The second raw-record slice intentionally writes workspace byte thirteen. */
    for(index = 8u; index < 14u; index++)
        FirmwareWriteNear8((u16)(0x141au + index), FirmwareReadNear8((u16)(0xb4bfu + index)));
}

/* RAM:fe0e9c */
u8 BuildMediaCatalogNumberQPacket(void)
{
    u32 raw_q_tuple;
    s32 target_remainder;
    u16 source_address;
    u16 destination_address;
    u8 character_offset;

    target_remainder = SignedRemainderBy100((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598));
    if(target_remainder != (s32)FirmwareReadNear8(FW_NEAR_UNKNOWN_14CD))
        return 0u;

    raw_q_tuple = ((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1275) << 16) | ((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1276) << 8) | FirmwareReadNear8(FW_NEAR_UNKNOWN_1277);
    if(raw_q_tuple < 5ul && FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_14CD) + 5u));
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CD, 0x21u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0xf0u) + 2u));
    source_address = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u ? 0x141au : 0x0c52u;
    destination_address = 0x1273u;

    for(character_offset = 0u; character_offset < 13u; character_offset = (u8)(character_offset + 2u))
    {
        u8 high_character = FirmwareRead8(source_address);
        u8 low_character = FirmwareReadNear8((u16)(source_address + 1u));

        FirmwareWrite8(destination_address, (u8)(((high_character << 4) & 0xf0u) | (low_character & 0x0fu)));
        source_address = (u16)(source_address + 2u);
        destination_address = (u16)(destination_address + 1u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127A, 0u);
    return 1u;
}

/* Reconstruction helper; no direct Ghidra function address. */
static void NormalizeIsrcLetterCharacter(u16 source_address)
{
    if(FirmwareRead8(source_address) >= 0x41u)
    {
        if((s8)FirmwareRead8(source_address) <= (s8)0x5au)
        {
            FirmwareWrite8(source_address, (u8)(FirmwareRead8(source_address) + 0xd0u));
            return;
        }
    }
    if(FirmwareRead8(source_address) >= 0x61u)
    {
        if((s8)FirmwareRead8(source_address) <= (s8)0x7au)
        {
            FirmwareWrite8(source_address, (u8)(FirmwareRead8(source_address) + 0xb0u));
            return;
        }
    }
    if((s8)FirmwareRead8(source_address) > (s8)0x7au)
        FirmwareWrite8(source_address, 0u);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void NormalizeIsrcDigitCharacter(u16 source_address)
{
    if(FirmwareRead8(source_address) >= 0x30u && (s8)FirmwareRead8(source_address) <= (s8)0x39u)
        FirmwareWrite8(source_address, (u8)(FirmwareRead8(source_address) + 0xd0u));
}

/* RAM:fe108d */
u8 BuildIsrcQPacket(void)
{
    u32 raw_q_tuple;
    s32 target_remainder;
    u16 source_address;
    u16 destination_address;
    u8 character_index;

    target_remainder = SignedRemainderBy100((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598));
    if(target_remainder != (s32)FirmwareReadNear8(FW_NEAR_UNKNOWN_14CE))
        return 0u;

    raw_q_tuple = ((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1275) << 16) | ((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1276) << 8) | FirmwareReadNear8(FW_NEAR_UNKNOWN_1277);
    if(raw_q_tuple < 5ul && FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_14CE) + 5u));
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CE, 0x42u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0xf0u) + 3u));
    source_address = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u ? 0x140eu : 0x0c62u;

    for(character_index = 0u; character_index < 5u; character_index++)
        NormalizeIsrcLetterCharacter((u16)(source_address + character_index));
    for(character_index = 0u; character_index < 13u; character_index++)
        NormalizeIsrcDigitCharacter((u16)(source_address + character_index));

    destination_address = 0x1273u;
    for(character_index = 0u; character_index < 13u; character_index++)
    {
        u8 value;
        u16 prior_destination;

        switch(character_index)
        {
        case 0u:
        case 4u:
            value = FirmwareRead8(source_address);
            source_address = (u16)(source_address + 1u);
            FirmwareWrite8(destination_address, (u8)((value << 2) & 0xfcu));
            if(character_index == 4u)
                destination_address = (u16)(destination_address + 1u);
            break;
        case 1u:
            value = FirmwareRead8(source_address);
            prior_destination = destination_address;
            destination_address = (u16)(destination_address + 1u);
            FirmwareWriteNear8(prior_destination, (u8)(FirmwareReadNear8(prior_destination) | ((value >> 4) & 0x03u)));
            FirmwareWrite8(destination_address, (u8)((FirmwareRead8(source_address) << 4) & 0xf0u));
            source_address = (u16)(source_address + 1u);
            break;
        case 2u:
            value = FirmwareRead8(source_address);
            prior_destination = destination_address;
            destination_address = (u16)(destination_address + 1u);
            FirmwareWriteNear8(prior_destination, (u8)(FirmwareReadNear8(prior_destination) | ((value >> 2) & 0x0fu)));
            FirmwareWrite8(destination_address, (u8)((FirmwareRead8(source_address) << 6) & 0xc0u));
            source_address = (u16)(source_address + 1u);
            break;
        case 3u:
            value = FirmwareRead8(source_address);
            source_address = (u16)(source_address + 1u);
            prior_destination = destination_address;
            destination_address = (u16)(destination_address + 1u);
            FirmwareWriteNear8(prior_destination, (u8)(FirmwareReadNear8(prior_destination) | (value & 0x3fu)));
            break;
        case 7u:
        case 9u:
        case 11u:
            value = FirmwareRead8(source_address);
            source_address = (u16)(source_address + 1u);
            prior_destination = destination_address;
            destination_address = (u16)(destination_address + 1u);
            FirmwareWriteNear8(prior_destination, (u8)(FirmwareReadNear8(prior_destination) | (value & 0x0fu)));
            break;
        default:
            value = FirmwareRead8(source_address);
            source_address = (u16)(source_address + 1u);
            FirmwareWrite8(destination_address, (u8)((value << 4) & 0xf0u));
            break;
        }
    }
    return 1u;
}

/* RAM:fe0f77 */
void PLEXTOR_NEAR_FUNCTION LoadIsrcCharactersForCurrentTrack(void)
{
    u8 index;
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_14A8) == 0u)
        return;

    for(index = 0u; index < 12u; index++)
        FirmwareWriteNear8((u16)(0x140eu + index), 0u);

    if(write_type == 2u)
    {
        u16 record_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_14A3);
        u16 first_address = (u16)(0xb4bfu + (u16)(record_index << 3));
        u16 second_address = (u16)(0xb4bfu + (u16)((u16)(record_index + 1u) << 3));

        for(index = 0u; index < 6u; index++)
            FirmwareWriteNear8((u16)(0x140eu + index), FirmwareReadNear8((u16)(first_address + index)));
        for(index = 6u; index < 12u; index++)
            FirmwareWriteNear8((u16)(0x140eu + index), FirmwareReadNear8((u16)(second_address + (u16)(index - 6u))));
    }
}

/* RAM:f8f6b2 */
u8 SelectLowestEnabledWriteSpeed(void)
{
    u16 flags = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7);

    if((flags & 0x1000u) != 0u)
        return 8u;
    if((flags & 0x0800u) != 0u)
        return 10u;
    if((flags & 0x0400u) != 0u)
        return 12u;
    if((flags & 0x0200u) != 0u)
        return 16u;
    if((flags & 0x0100u) != 0u)
        return 20u;
    if((flags & 0x0080u) != 0u)
        return 24u;
    if((flags & 0x0040u) != 0u)
        return 32u;
    if((flags & 0x0020u) != 0u)
        return 40u;
    return 52u;
}

/* RAM:f8fdcd */
void AccumulateWriteSpeedFlagsFromPositionAndOperatingCode(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    s32 position;
    u16 selected_flag = 0u;

    if(operating_code < 0x14u)
        return;
    position = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    if(position >= (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9F6))
    {
        if(operating_code == 0x34u)
            selected_flag = 0x0020u;
        else if(operating_code == 0x28u)
            selected_flag = 0x0040u;
        else if(operating_code == 0x20u)
            selected_flag = 0x0080u;
        else if(operating_code == 0x18u)
            selected_flag = 0x0100u;
        else if(operating_code == 0x14u)
            selected_flag = 0x0200u;
    }
    else if(position >= (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9F2))
    {
        if(operating_code >= 0x28u)
            selected_flag = 0x0040u;
        else if(operating_code == 0x20u)
            selected_flag = 0x0080u;
        else if(operating_code == 0x18u)
            selected_flag = 0x0100u;
        else if(operating_code == 0x14u)
            selected_flag = 0x0200u;
    }
    else if(position >= (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9EE))
    {
        if(operating_code >= 0x20u)
            selected_flag = 0x0080u;
        else if(operating_code == 0x18u)
            selected_flag = 0x0100u;
        else if(operating_code == 0x14u)
            selected_flag = 0x0200u;
    }
    else
    {
        if(operating_code >= 0x18u)
            selected_flag = 0x0100u;
        else if(operating_code == 0x14u)
            selected_flag = 0x0200u;
    }
    if(selected_flag != 0u)
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | selected_flag));
}

/* RAM:f8f5ae */
u8 TryAccumulateReadMeasurementWriteSpeedFlags(void)
{
    u16 retained_flags;

    if((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD9) & 0x0001u) != 0u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 0x01u) != 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        return 0u;

    retained_flags = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7);
    if((retained_flags & 0x0003u) == 0x0003u)
    {
        /* The source accepts this pair without consulting the profile bit. */
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u || (FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x1000u) == 0u)
            return 0u;
    }

    AccumulateWriteSpeedFlagsFromPositionAndOperatingCode();
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD9, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD9) | 0x0001u));
    if(FirmwareReadBit0031() != 0u)
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD9, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD9) | 0x0002u));
    return 1u;
}

/* RAM:f8c7ce */
void RampControllerWord0255TowardFeedbackValue(void)
{
    s16 current_value;
    s16 feedback_value;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x80u) != 0u)
    {
        current_value = (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F994);
        feedback_value = (s16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8);
        while(current_value != feedback_value)
        {
            if(current_value < feedback_value)
                current_value = (s16)(current_value + 1);
            else
                current_value = (s16)(current_value - 1);
            SubmitControllerByteAndWord((u8)current_value, 0x0255u);
            WaitTimer1ScaledDuration(200u, 1u);
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x7fu));
}

/* RAM:f8ff3e */
void ConstrainDiscReadSpeedLimitFromCalibration(void)
{
    u8 calibration;
    u8 speed_limit;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0x04u) == 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 0x04u));
    calibration = FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E8);
    speed_limit = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F);
    if(calibration >= 0x83u)
    {
        if(speed_limit >= 0x18u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 0x18u);
    }
    else if(calibration >= 0x79u)
    {
        if(speed_limit >= 0x20u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 0x20u);
    }
    else if(calibration >= 0x6fu)
    {
        if(speed_limit >= 0x28u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 0x28u);
    }
    else if(calibration > 0x28u && speed_limit >= 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 0x20u);
    }
}

/* RAM:f8f51a */
void ClearMediaStateFlags7Through12(void)
{
    MaskMediaStateFlagsWord(0xefffu);
    MaskMediaStateFlagsWord(0xf7ffu);
    MaskMediaStateFlagsWord(0xfbffu);
    MaskMediaStateFlagsWord(0xfdffu);
    MaskMediaStateFlagsWord(0xfeffu);
    MaskMediaStateFlagsWord(0xff7fu);
}

/* RAM:f8ce0c */
void ReplaceWriteSpeedSelectionFlagForValue(u8 write_speed_x)
{
    /* The original uses nine individual word read-modify-writes.  These
     * writes are observable to concurrent interrupt code, so retain their
     * order instead of combining their masks. */
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xffdfu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xffbfu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xff7fu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xfeffu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xfdffu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xfbffu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xf7ffu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xefffu));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xdfffu));
    if(write_speed_x < 0x35u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD4, write_speed_x);
        if(write_speed_x == 4u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x2000u));
        if(write_speed_x == 8u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x1000u));
        if(write_speed_x == 10u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0800u));
        if(write_speed_x == 12u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0400u));
        if(write_speed_x == 16u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0200u));
        if(write_speed_x == 20u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0100u));
        if(write_speed_x == 24u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0080u));
        if(write_speed_x == 32u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0040u));
        if(write_speed_x == 40u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0020u));
        if((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0010u) != 0u)
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x1000u));
    }
}

/* RAM:f89f42 */
void PLEXTOR_NEAR_FUNCTION ClearTemporaryWriteSpeedReductionState(void)
{
    if((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0004u) != 0u)
    {
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0xfffbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD4, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD3));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD2, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD3));
    }
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x3ff7u));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD9, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD9) & 0xfffcu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD6, 0u);
    ReplaceWriteSpeedSelectionFlagForValue(0xffu);
}

/* RAM:fd811d */
u8 IsIndexedDriveStateActive(u8 state_code)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && state_code == FirmwareReadNear8(FW_NEAR_UNKNOWN_130A));
}

/* RAM:fb9d2b */
void ClearPositionSnapshotFlagsAndCaptureCurrentFrame(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) & 0xfeu));
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_FB72, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) & 0xfdu));
}

/* RAM:fd8132 */
u8 IsCurrentDriveOperationCodeInTocAcquisitionSet(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 4u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0xffu
                || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 7u
                || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x14u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0xf2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x15u
                || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x16u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0xfdu || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x17u);
}

/* RAM:fe19de */
/* C251 source-order ABI: flags in ACC, class in R7, detail in R6. */
u8 PLEXTOR_NEAR_FUNCTION MapThreeFieldStatusCode(u8 flags, u8 field_r7, u8 field_r6)
{
    if((flags & 0x40u) != 0u)
    {
        switch(field_r7)
        {
        case 0u:
            return 0xffu;
        case 1u:
            return 8u;
        case 2u:
            switch(field_r6)
            {
            case 8u:
                return 12u;
            case 16u:
                return 10u;
            case 32u:
                return 9u;
            }
            break;
        }
    }
    return 0u;
}

/* RAM:fe19ce */
u8 MapWorkingTrackFieldsToStatusCode(void)
{
    u8 field_r6 = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD33);
    u8 field_r7 = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD32);
    u8 flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31);

    return MapThreeFieldStatusCode(flags, field_r7, field_r6);
}

/* Reconstruction helper; no direct Ghidra function address.
 * RAM:fe2109 copies the three noncontiguous record bytes to private stack
 * storage before calling RAM:fee841.  This preserves the source-byte and
 * adjustment-gate read order without exposing a compiler-owned stack address
 * through the C interface. */
static s32 ConvertRawTrackRecordMsfValuesToAdjustedFramePosition(u8 minute, u8 second, u8 frame)
{
    u16 total_seconds = (u16)((u16)minute * 60u + second);
    s32 frame_position = (s32)(((u32)total_seconds << 6) + ((u32)total_seconds << 3) + ((u32)total_seconds << 1) + total_seconds + frame);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_102F) == 1u && (FirmwareReadDirectSfr8(D1) & 0x20u) == 0u)
    {
        frame_position -= 450000l;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) != 0u)
        {
            u32 lead_in_start = FirmwareReadNearU32(FW_NEAR_UNKNOWN_114F);

            if((s32)lead_in_start != 0 && frame_position < (s32)lead_in_start - 0x47el)
                frame_position += 0x8cal;
        }
    }
    return frame_position;
}

/* RAM:fe2109 */
s32 GetType1TrackRecordFramePositionAtOrAfterIndex(u16 starting_record_index)
{
    u16 record_address;
    u8 minute;
    u8 second;
    u8 frame;

    do
    {
        record_address = (u16)(0xb4bdu + (u16)(starting_record_index << 3));
        if((FirmwareRead8(record_address) & 0x0fu) == 1u)
            break;
        starting_record_index++;
    } while(1);

    /* The canonical stack tuple receives record offsets 4, 6, and 7. */
    minute = FirmwareReadNear8((u16)(record_address + 4u));
    second = FirmwareReadNear8((u16)(record_address + 6u));
    frame = FirmwareReadNear8((u16)(record_address + 7u));
    return ConvertRawTrackRecordMsfValuesToAdjustedFramePosition(minute, second, frame);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void PublishRawReadPreparationSegmentsFromTrackRecordDistance(u16 record_index, u32 first_count, u8 first_mode, u8 second_mode)
{
    u32 next_position;
    u32 current_position;

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1430, first_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, first_mode);
    next_position = (u32)GetType1TrackRecordFramePositionAtOrAfterIndex((u16)(record_index + 1u));
    current_position = (u32)GetType1TrackRecordFramePositionAtOrAfterIndex(record_index);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1434, (u32)(next_position - current_position - first_count));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, second_mode);
}

/* RAM:fe1f57 */
void InitializeRawReadPreparationSegmentsFromTrackRecordIndex(u16 record_index)
{
    u8 previous_flags = FirmwareRead8(RawReadRecordAddress((u16)(record_index - 1u)));
    u8 current_flags = FirmwareRead8(RawReadRecordAddress(record_index));
    u8 previous_class = FindAdjacentRawReadQRecordClass((u16)(record_index - 1u), 0u);
    u8 next_class = FindAdjacentRawReadQRecordClass(record_index, 1u);

    if(record_index == (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149D) + 1u))
        PublishRawReadPreparationSegmentsFromTrackRecordDistance(record_index, 0u, 0u, next_class == 0u ? 1u : 2u);
    else if((previous_flags & 0x40u) == 0u)
        if((current_flags & 0x40u) != 0u)
            PublishRawReadPreparationSegmentsFromTrackRecordDistance(record_index, 75u, 1u, 2u);
        else
            PublishRawReadPreparationSegmentsFromTrackRecordDistance(record_index, 0u, 0u, 1u);
    else if(previous_class == next_class)
        PublishRawReadPreparationSegmentsFromTrackRecordDistance(record_index, 0u, 0u, 2u);
    else
        PublishRawReadPreparationSegmentsFromTrackRecordDistance(record_index, 75u, 3u, 2u);
}

/* RAM:fe1c61 */
void BuildRawSectorTdiPayloadPrefix(fw_addr_t output_address)
{
    u8 selected_track;
    u8 offset;
    u32 encoded_packet_size;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        u16 lead_in_record_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149D);
        u16 current_record_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B);
        u16 record_index;

        if((u16)(lead_in_record_index + 2u) > current_record_index)
            return;
        record_index = (u16)(current_record_index - 1u);
        while((FirmwareRead8(RawReadRecordAddress(record_index)) & 0x0fu) != 1u)
            record_index = (u16)(record_index - 1u);
        InitializeRawReadPreparationSegmentsFromTrackRecordIndex(record_index);
    }

    for(offset = 0u; offset != 24u; ++offset)
        FirmwareWrite8(FirmwareAddressIncrementLow16(output_address, offset), 0u);
    FirmwareWrite8(output_address, 0x54u);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, 0x44u);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, 0x49u);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, 1u);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, 0x50u);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, 1u);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_128B) == 1u)
    {
        selected_track = BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35));
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 1u)
            selected_track = BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149));
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 1u)
            selected_track = BinaryByteToPackedBcd((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) + 1u));
        else
            selected_track = 1u;
    }
    else
    {
        selected_track = BinaryByteToPackedBcd((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) + 1u));
    }
    FirmwareWrite8(output_address, selected_track);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, selected_track);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, selected_track);
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, SelectRawSectorTdiControlByteFromWriteState());
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    encoded_packet_size = EncodeTdiPacketSizeFromWriteParameters();
    FirmwareWrite8(output_address, (u8)(encoded_packet_size >> 16));
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, (u8)(encoded_packet_size >> 8));
    output_address = FirmwareAddressIncrementLow16(output_address, 1u);
    FirmwareWrite8(output_address, (u8)encoded_packet_size);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1454, 24u);
}

/* RAM:fe1dfd */
void PopulateRawSectorHeaderAndTdiPayloadRange(fw_addr_t controller_buffer_address, u32 sector_count)
{
    u8 type2_special_subheader = 0u;
    fw_addr_t raw_sector_address = controller_buffer_address;
    fw_addr_t payload_address;
    u32 sector_index = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        u16 record_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B);

        do
        {
            record_index = (u16)(record_index - 1u);
        } while((FirmwareRead8(RawReadRecordAddress(record_index)) & 0x0fu) != 1u);
        if(FirmwareReadNear8((u16)(RawReadRecordAddress(record_index) + 3u)) == 0x31u)
            type2_special_subheader = 1u;
    }

    payload_address = FirmwareAddressIncrement24(controller_buffer_address, ComputeRawSectorPayloadOffsetFromWriteState());
    while(sector_index < sector_count)
    {
        if(type2_special_subheader != 0u)
        {
            CopyBytesToAddressPlus1MiB(FirmwareAddressIncrement24(raw_sector_address, 0x10u), PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF92C9), 8u);
            raw_sector_address = FirmwareAddressIncrement24(raw_sector_address, 0x930u);
        }
        else
        {
            PopulateConfiguredMode2SubheaderWhenRequired(raw_sector_address, 6u);
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u)
            CopyBytesToAddressPlus1MiB(payload_address, FW_NEAR_UNKNOWN_143C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1454));
        payload_address = FirmwareAddressIncrement24(payload_address, 0x930u);
        raw_sector_address = FirmwareAddressIncrement24(raw_sector_address, 0x930u);
        sector_index++;
    }
}

/* RAM:fe243a */
s32 SelectCurrentReadFailureFramePosition(void)
{
    if(FirmwareReadBit0028() != 0u)
    {
        u16 descriptor_address;
        u16 source_address;
        u32 frame_position;
        u8 restored = RestoreRawReadDescriptorsToQueueHead(1u);

        descriptor_address = GetRawReadConsumerDescriptorAddressOrZero();
        source_address = FirmwareReadBigEndianU16Near((u16)(descriptor_address + 2u));
        frame_position = FirmwareReadBigEndianU32Near(source_address);
        return restored != 0u ? (s32)frame_position : (s32)(frame_position - 1ul);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1281) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1281) == 3u)
        return (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_148C);
    return (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_88BB);
}

/* RAM:fe2426 */
void LatchFirstReadFailureContext(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1490) == 0u)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1484, (u32)SelectCurrentReadFailureFramePosition());
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 1u);
    }
}

/* RAM:fe2ce9 */
s32 ComputeFrameDistanceToType1TrackRecordAtOrAfterIndex(u16 starting_record_index)
{
    u16 record_address;
    u8 minute;
    u8 second;
    u8 frame;
    s32 record_frame_position;

    do
    {
        record_address = (u16)(0xb4bdu + (u16)(starting_record_index << 3));
        if((FirmwareRead8(record_address) & 0x0fu) == 1u)
            break;
        starting_record_index++;
    } while(1);

    /* The original copies these noncontiguous bytes into private stack storage
     * before calling RAM:fee841.  Keep their source read order and calculate
     * from the same byte tuple without exposing a C compiler stack address. */
    minute = FirmwareReadNear8((u16)(record_address + 4u));
    second = FirmwareReadNear8((u16)(record_address + 6u));
    frame = FirmwareReadNear8((u16)(record_address + 7u));
    record_frame_position = ConvertRawTrackRecordMsfValuesToAdjustedFramePosition(minute, second, frame);
    return record_frame_position - (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598);
}

/* RAM:fe6262 */
/* The two controller-port cases use different comparison divisors, but both
 * commit the same full-block limit when the comparison requires a cap. */
u16 PLEXTOR_NEAR_FUNCTION ComputeRawReadTransferBatchBlockCount(void)
{
    u16 queued = (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1296) - FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1294));
    u16 batch = (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129C) - (queued & FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A)));
    u32 remaining = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1455) - FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1459);
    u16 transfer_bytes;

    if(remaining < 0x100ul && (u16)remaining < batch)
        batch = (u16)remaining;
    else if(remaining >= 0x100ul && 0xffu < batch)
        batch = 0xffu;

    if(batch == 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 0x02u) != 0u)
        {
            u32 packet_remaining = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B) - FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_14B8);
            if(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B) < FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_14B8) + (u32)batch)
                batch = (u16)packet_remaining;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        s32 distance = ComputeFrameDistanceToType1TrackRecordAtOrAfterIndex(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B));
        if(distance == 0)
            return 0u;
        if(distance > 0 && (u32)distance < (u32)batch)
            batch = (u16)distance;
    }

    transfer_bytes = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_146D);
    if((FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 1u) == 0u)
    {
        if(transfer_bytes != 0u && (u16)(0xffffu / transfer_bytes) < batch)
            batch = (u16)(0xffffu / transfer_bytes);
    }
    else
    {
        u16 comparison_divisor = (u16)(transfer_bytes >> 1);
        if(comparison_divisor != 0u && (u16)(0x8000u / comparison_divisor) < batch)
        {
            /* The second DIV intentionally uses the unshifted divisor. */
            if(transfer_bytes != 0u)
                batch = (u16)(0xffffu / transfer_bytes);
        }
    }
    return batch;
}

/* RAM:fe7ddd */
#define PLEXTOR_BINDING_SECTION 58
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

void PLEXTOR_NEAR_FUNCTION InitializeRawReadRandomBytesFromDriveConfiguration(void)
{
    u8 configuration[4];
    u16 value;
    u16 seed;

    READ_RAW_READ_RANDOM_CONFIGURATION(configuration);
    seed = (u16)(((u16)configuration[2] << 8) | configuration[3]);
    ResetPseudoRandomGenerator(seed);
    value = NextPseudoRandom15Bit();
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5E0, (u8)(value % 100u));
    value = NextPseudoRandom15Bit();
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5DF, (u8)(value % 100u));
    value = NextPseudoRandom15Bit();
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5DE, (u8)(value % 100u));
}

/* RAM:fe9ef2 */
void PLEXTOR_NEAR_FUNCTION BuildRawReadQPacketForFramePosition(s32 frame_position, fw_addr_t destination)
{
    u8 tuple[3];
    u8 index;
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 2u)
    {
        u16 record_offset = (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B) << 3);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
        FirmwareWrite8(destination, FirmwareReadNear8((u16)(0xb4bdu + record_offset)));
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 1u), FirmwareReadNear8((u16)(0xb4beu + record_offset)));
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 2u), 0u);
    }
    else if(write_type == 0u || write_type == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1286) == 0u));
        FirmwareWrite8(destination, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && write_type == 0u)
            || FirmwareReadNear8(FW_NEAR_UNKNOWN_128B) == 1u)
            FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 1u), BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35)));
        else
            FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 1u), BinaryByteToPackedBcd((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) + 1u)));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u)
            FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 1u), 1u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_14B6) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_14B3) == 1u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_14B7) != 1u)
            FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 2u), 1u);
        else if(write_type == 0u && (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) < (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598))
            FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 2u), 1u);
        else
            FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 2u), 0u);
    }

    if(write_type == 0u && (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) < (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598))
        EncodeFramePositionAsPackedBcdMsf((s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598) - FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D)), tuple);
    else
        EncodeFramePositionAsPackedBcdMsf(frame_position, tuple);
    for(index = 0u; index < 3u; index++)
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination, (u16)(3u + index)), tuple[index]);
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 6u), 0u);
    EncodeFramePositionAsPackedBcdMsf((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598), tuple);
    for(index = 0u; index < 3u; index++)
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination, (u16)(7u + index)), tuple[index]);

    SnapshotAndAdjustRawReadQPacketMsf(1u);
    for(index = 0u; index < 10u; index++)
        FirmwareWriteNear8((u16)(0x88aau + index), FirmwareReadNear8((u16)(0x1272u + index)));
    UpdateRawReadQPacketControlAndIdentificationForBuild();
}

/* RAM:f92b52 */
void PLEXTOR_NEAR_FUNCTION InitializeAlternateRawReadControllerCaptureState(void)
{
    u8 port18;
    u8 special_mode;

    /* The native MOV at RAM:f92b54 stores zero through DR4, i.e. all four
     * bytes at RAM:0015ae..0015b1, rather than only the first byte. */
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_15AE, 0ul);
#define PLEXTOR_BINDING_SECTION 33
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFD, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x27u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x27u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0x60u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0u);
    port18 = FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) == 4u)
    {
        port18 = (u8)(port18 & 0x1fu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, port18);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, port18);
        port18 = (u8)(port18 | 0x86u);
    }
    else
        port18 = (u8)(port18 | 0xe6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, port18);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, port18);
#define PLEXTOR_BINDING_SECTION 34
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    port18 = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | (special_mode == 0u ? 0x20u : 0xa8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, port18);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, port18);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x29u);
#define PLEXTOR_BINDING_SECTION 35
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F76, 0x7du);
}

/* RAM:f9414a */
void PLEXTOR_NEAR_FUNCTION InitializeRawReadOperatingSpeedState(void)
{
    u8 selected_speed;
    u8 configuration_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA2);

    if(configuration_index == 0u)
    {
        selected_speed = 0u;
#define PLEXTOR_BINDING_SECTION 36
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F)
            && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9D))
            selected_speed = 0x28u;
    }
    if(selected_speed == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F)
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9D))
        selected_speed = 0x20u;
    if(selected_speed == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0)
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9D))
        selected_speed = 0x18u;
    if(selected_speed == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0)
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9D))
        selected_speed = 8u;
    if(selected_speed == 0u)
        selected_speed = 4u;
}
else
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

    if(operating_code == 8u)
        selected_speed = 0x18u;
    else if(operating_code == 0x18u)
        selected_speed = 0x20u;
    else if(operating_code == 0x20u)
        selected_speed = 0x28u;
    else if(operating_code == 4u)
        selected_speed = 8u;
    else if(configuration_index == 9u)
        selected_speed = 0x20u;
    else
        selected_speed = 4u;
#define PLEXTOR_BINDING_SECTION 37
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}
if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A) < selected_speed && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
    selected_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, selected_speed);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, selected_speed);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, selected_speed);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, selected_speed);
if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0F3B) < 0x1ce8u)
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0F3B, (u16)(9000u - ((u16)configuration_index << 5) - ((u16)selected_speed << 5)));
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0x19u);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F08, 0x32u);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, 0u);
#define PLEXTOR_BINDING_SECTION 38
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:f94872 */
void PLEXTOR_NEAR_FUNCTION AdvanceRawReadRingCursors(void)
{
    u32 cursor = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_126E) + 0x930ul;
    u32 ring_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_155E);

    if(ring_size <= cursor)
        cursor -= ring_size;
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_126E, cursor);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) != 0u)
    {
        u32 slot_index = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1262) + 1ul;
        u32 slot_count = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A);

        if(slot_count <= slot_index)
            slot_index -= slot_count;
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1262, slot_index);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u)
    {
        u32 slot_span = MultiplyU32Low(0x126ul, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A));
        u32 auxiliary_cursor = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_126A) + 0x126ul;

        if(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1562) + slot_span <= auxiliary_cursor)
            auxiliary_cursor -= slot_span;
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_126A, auxiliary_cursor);
    }
}

/* RAM:f9490c */
void PLEXTOR_NEAR_FUNCTION PackControllerSubchannelSymbolsTo72Bytes(fw_addr_t subchannel_slot_address)
{
    u8 group_index;
    fw_addr_t output_address = FW_HUGE_UNKNOWN_2BB919;

    for(group_index = 0u; group_index < 24u; group_index++)
    {
        u8 symbol0 = FirmwareRead8(subchannel_slot_address);
        u8 symbol1 = FirmwareRead8(FirmwareAddressIncrement24(subchannel_slot_address, 1u));
        u8 symbol2 = FirmwareRead8(FirmwareAddressIncrement24(subchannel_slot_address, 2u));
        u8 symbol3 = FirmwareRead8(FirmwareAddressIncrement24(subchannel_slot_address, 3u));

        FirmwareWrite8(output_address, (u8)((symbol0 << 2) | ((symbol1 & 0x30u) >> 4)));
        FirmwareWrite8(FirmwareAddressIncrement24(output_address, 1u), (u8)((symbol1 << 4) | ((symbol2 & 0x3cu) >> 2)));
        FirmwareWrite8(FirmwareAddressIncrement24(output_address, 2u), (u8)((symbol2 << 6) | (symbol3 & 0x3fu)));
        subchannel_slot_address = FirmwareAddressIncrement24(subchannel_slot_address, 4u);
        output_address = FirmwareAddressIncrement24(output_address, 3u);
    }
}

/* RAM:f94ed0 */
u8 PLEXTOR_NEAR_FUNCTION CaptureMediaState6LeadInMeasurementOffset(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 0x28u);
    ArmControllerThreeBytePacketCapture();
    for(;;)
    {
        u8 packet_result;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) == 0u)
            return 0u;
        packet_result = ParseControllerThreeBytePacket();
        if(packet_result != 0xfeu)
        {
            s32 lead_in = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_114F);
            s32 current = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_1235);

            ArmControllerThreeBytePacketCapture();
            if(lead_in - 0x406l <= current)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, 1u);
                return 1u;
            }
            if(lead_in - 0x46al <= current)
            {
                WaitTimer1ScaledDuration(100u, 0u);
                if(TestP1Cex0PulseSuccessThreshold(10u) != 0u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xf7u));
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, (u8)((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_114F) - (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1235) - 5));
                    return 1u;
                }
            }
        }
        YieldCoroutine1();
    }
}

/* RAM:f94f54 */
void PLEXTOR_NEAR_FUNCTION ComputeMediaState6MeasurementTargetFromLeadInOffset(void)
{
    s32 target;
    u8 flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72);
    u8 captured_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74);

    if((flags & 2u) != 0u)
    {
        target = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_114F) - 0x488l - (s32)MultiplyU32Low((u32)captured_offset, 15ul);
        if((flags & 4u) != 0u)
            target -= 15l;
    }
    else
    {
        target = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_114F) - 0x406l - (s32)captured_offset;
        if((flags & 4u) != 0u)
            target--;
    }
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_103D, (u32)target);
}

/* Reconstruction helper; no direct Ghidra function address.  The original
 * float64 path rounds downward, including for negative centered slopes. */
static s32 FloorCenteredSlopeTenths(u8 slope, s8 centered_index)
{
    s32 product = (s32)MultiplyU32Low((u32)(s32)centered_index, (u32)slope);
    s32 quotient = DivideS32ByU16TruncatingTowardZero(product, 10u);

    if(product < 0 && (s32)MultiplyU32Low((u32)quotient, 10ul) != product)
        quotient--;
    return quotient;
}

/* RAM:f9563a */
void PLEXTOR_NEAR_FUNCTION InitializeControllerMeasurementSeries00And01FromCenteredSlopes(void)
{
    fw_addr_t profile = FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_F1BB);
    u8 slope0 = FirmwareRead8(FirmwareAddressIncrement24(profile, 6u));
    s8 slope1 = (s8)FirmwareRead8(FirmwareAddressIncrement24(profile, 7u));
    s16 base0 = (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F115);
    s16 base1 = (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F117);
    u8 index;
    u8 mode6 = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 6u);

    for(index = 0u; index < 15u; index++)
    {
        s8 centered = (s8)((s8)index - 7);
        s32 value0 = (s32)base0 + FloorCenteredSlopeTenths(slope0, mode6 != 0u ? centered : (s8)-centered);
        s32 value1 = (s32)base1 + (s32)MultiplyU32Low((u32)(s32)centered, (u32)(s32)slope1);

        if(value0 < 0)
            value0 = 0;
        else if(value0 > 0x7el)
            value0 = 0x7el;
        if(value1 < 0)
            value1 = 0;
        else if(value1 > 24000l)
            value1 = 24000l;
        FirmwareWriteBigEndianU16Near((u16)(0xef89u + (u16)index * 2u), (u16)value0);
        FirmwareWriteBigEndianU16Near((u16)(0xefa7u + (u16)index * 2u), (u16)value1);
    }
    if(mode6 == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xbfu));
}

/* RAM:f95b53 */
void EvaluateControllerMeasurementTailResiduals(void)
{
    s16 residual[4];
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) != 1u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) < 15u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 3u);
        return;
    }

    for(index = 0u; index < 4u; index++)
    {
        fw_addr_t offset = (fw_addr_t)((u16)(index + 11u) * 2u);
        u16 series03_delta = (u16)(FirmwareReadBigEndianU16Near((u16)(0xefe3u + offset)) - FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F9A2));
        u16 series02_delta = (u16)(FirmwareReadBigEndianU16Near((u16)(0xefc5u + offset)) - FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F9A0));

        residual[index] = (s16)(series02_delta + series03_delta);
    }
    for(index = 1u; index < 4u; index++)
    {
        if((u16)residual[index] < 5u && (u16)residual[index - 1u] > 15u && (u16)((u16)residual[index - 1u] - (u16)residual[index]) > 30u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 3u);
            return;
        }
    }
}

/* RAM:f9673d */
/* The native entry
 * inherits its starting index in R9; the C parameter makes that local ABI
 * explicit. */
u8 ClassifyControllerMeasurementDifferenceFromIndex(u8 starting_index)
{
    u8 index = starting_index;
    u16 threshold = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_EF85);
    s16 difference;
    s16 quotient;
    u16 denominator;

    while(index < 15u && FirmwareReadBigEndianU16Near((u16)(0xef89u + (u16)index * 2u)) < threshold)
        index++;

    difference = (s16)((u16)(FirmwareReadBigEndianU16Near((u16)(0xefc5u + (u16)index * 2u)) - FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F9A0))
                       + (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F9A2) - FirmwareReadBigEndianU16Near((u16)(0xefe3u + (u16)index * 2u))));
    denominator = FirmwareReadBigEndianU16Near((u16)(0xefc5u + (u16)index * 2u));
    quotient = (s16)DivideS32ByU16TruncatingTowardZero((s32)MultiplyU32Low((u32)(s32)difference, 1000ul), denominator);

    if(quotient < 300)
        return 3u;
    if(quotient <= 700)
        return 1u;
    return 4u;
}

/* RAM:f96729 */
u8 ClassifyControllerMeasurementDifferenceFromIndex0(void)
{
    return ClassifyControllerMeasurementDifferenceFromIndex(0u);
}

/* RAM:f97e2a */
/* The native invalid
 * mode-clear path preserves incoming WR6, exposed here as inherited_wr6. */
u16 PLEXTOR_NEAR_FUNCTION SelectControllerMeasurementScalePercent(u16 inherited_wr6)
{
    fw_addr_t profile = FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_F1BB);
    u8 profile_override = FirmwareRead8(FirmwareAddressIncrement24(profile, 9u));
    u8 profile_class;

    if(profile_override != 0u)
        return (u16)profile_override;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) != 0u)
    {
        profile_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB81);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) != 8u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) == 4u)
                profile_class = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB86) & 7u) << 4);
            else
                profile_class = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB86) >> 3) & 7u) << 4);
        }
        switch((profile_class >> 4) & 7u)
        {
        case 0u:
            return 110u;
        case 1u:
            return 118u;
        case 2u:
            return 126u;
        case 3u:
            return 135u;
        case 4u:
            return 144u;
        case 5u:
            return 154u;
        case 6u:
            return 164u;
        default:
            return 175u;
        }
    }
    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB81))
    {
    case 0x00u:
        return 100u;
    case 0x10u:
        return 105u;
    case 0x20u:
        return 110u;
    case 0x30u:
        return 115u;
    case 0x40u:
        return 120u;
    case 0x50u:
        return 125u;
    case 0x60u:
        return 130u;
    case 0x70u:
        return 135u;
    default:
        return inherited_wr6;
    }
}

/* RAM:f97f45 */
u8 PLEXTOR_NEAR_FUNCTION ValidateControllerMeasurementSeriesOrderingAtIndex(u8 series_index)
{
    u16 offset = (u16)series_index * 2u;

    if(series_index == 14u)
        return 0u;
    if(series_index == 0u)
        return (u8)((s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F021) < (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F01F)
                    && (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F023) < (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F021));
    if(series_index == 13u)
        return (u8)((s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F039) < (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F037)
                    && (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F03B) < (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F039));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
        return (u8)((s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01du + offset))
                    && (s16)FirmwareReadBigEndianU16Near((u16)(0xf021u + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset)));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
    {
        if((s16)FirmwareReadBigEndianU16Near((u16)(0xf01du + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01bu + offset))
            && (s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01du + offset))
            && (s16)FirmwareReadBigEndianU16Near((u16)(0xf021u + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset)))
            return 1u;
        return (u8)((s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01du + offset))
                    && (s16)FirmwareReadBigEndianU16Near((u16)(0xf021u + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset))
                    && (s16)FirmwareReadBigEndianU16Near((u16)(0xf023u + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf021u + offset)));
    }
    return (u8)((s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01du + offset))
                && (s16)FirmwareReadBigEndianU16Near((u16)(0xf021u + offset)) > (s16)FirmwareReadBigEndianU16Near((u16)(0xf01fu + offset)));
}

/* RAM:f996f2 */
void BuildReadSubChannelMediaCatalogNumberField(fw_addr_t destination_address)
{
    u8 index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 2u) == 0u)
    {
        for(index = 0u; index < 15u; index++)
            FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, index), 0u);
        return;
    }

    FirmwareWrite8(destination_address, 0x80u);
    for(index = 0u; index < 6u; index++)
    {
        u8 packed = FirmwareReadNear8((u16)(0xf515u + index));
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, (u16)(index * 2u + 1u)), (u8)((packed >> 4) + 0x30u));
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, (u16)(index * 2u + 2u)), (u8)((packed & 0x0fu) + 0x30u));
    }
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, 13u), (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F51B) >> 4) + 0x30u));
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, 14u), 0u);
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, 15u), 0u);
}

/* RAM:f9407c */
u8 PLEXTOR_NEAR_FUNCTION SelectDriveNumericTable0IntervalForReadPipelineTarget(void)
{
    u32 target = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598);
    u8 index;
    u8 limit;

    if(target < FirmwareReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84510)))
    {
        if(target < FirmwareReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F844D0)))
        {
            index = 0u;
            limit = 16u;
        }
        else
        {
            index = 16u;
            limit = 32u;
        }
    }
    else if(target < FirmwareReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84550)))
    {
        index = 32u;
        limit = 48u;
    }
    else
    {
        index = 48u;
        limit = 64u;
    }
    while(index < limit)
    {
        s32 lower = (s32)FirmwareReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490) + (u32)index * 4ul);
        s32 upper = (s32)FirmwareReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84494) + (u32)index * 4ul);
        if((s32)target > lower && (s32)target < upper)
        {
            FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_159C, (u32)upper);
            return 0u;
        }
        if((s32)target == lower)
        {
            FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_159C, (u32)upper);
            return index;
        }
        index++;
    }
    return 0u;
}

/* RAM:fed498 */
void ProgramRawReadControllerTransferAddressRegisters(void)
{
    ConfigureAtaDataTransferStatus();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, FirmwareReadNear8(FW_NEAR_UNKNOWN_146A));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, FirmwareReadNear8(FW_NEAR_UNKNOWN_146B));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, FirmwareReadNear8(FW_NEAR_UNKNOWN_146C));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 0x0du)
    {
#define PLEXTOR_BINDING_SECTION 39
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    }
}

/* RAM:fe2181 */
s32 PLEXTOR_NEAR_FUNCTION ComputeFrameDistanceToNextType1TrackRecordBoundary(u16 record_index)
{
    u16 record_address = (u16)(0xb4bdu + (u16)(record_index << 3));
    u8 current_minute = FirmwareReadNear8((u16)(record_address + 4u));
    u8 current_second = FirmwareReadNear8((u16)(record_address + 6u));
    u8 current_frame = FirmwareReadNear8((u16)(record_address + 7u));
    u16 next_record_index = 1u;
    u8 require_nonzero_offset_two = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_B4C7) == 0u);
    u8 next_minute;
    u8 next_second;
    u8 next_frame;
    s32 current_position;
    s32 next_position;

    for(;;)
    {
        record_address = (u16)(0xb4bdu + (u16)(next_record_index << 3));
        if((FirmwareRead8(record_address) & 0x0fu) == 1u && (require_nonzero_offset_two == 0u || FirmwareReadNear8((u16)(record_address + 2u)) != 0u))
            break;
        next_record_index++;
    }

    next_minute = FirmwareReadNear8((u16)(record_address + 4u));
    next_second = FirmwareReadNear8((u16)(record_address + 6u));
    next_frame = FirmwareReadNear8((u16)(record_address + 7u));
    current_position = ConvertRawTrackRecordMsfValuesToAdjustedFramePosition(current_minute, current_second, current_frame);
    next_position = ConvertRawTrackRecordMsfValuesToAdjustedFramePosition(next_minute, next_second, next_frame);
    return next_position - current_position;
}

/* RAM:fe22e9 */
void SelectIsrcRawTrackRecordForCurrentTrack(void)
{
    u16 selected_index = ReadBigEndianU16(FW_NEAR_UNKNOWN_14A3);
    u16 record_index = 0u;

    while(record_index < ReadBigEndianU16(FW_NEAR_UNKNOWN_B4BB))
    {
        u16 record_address = (u16)(0xb4bdu + (u16)(record_index << 3));

        if(FirmwareReadNear8((u16)(record_address + 1u)) == FirmwareReadNear8(FW_NEAR_UNKNOWN_14A8) && (FirmwareRead8(record_address) & 0x0fu) == 3u)
        {
            selected_index = record_index;
            break;
        }
        record_index++;
    }
    WriteBigEndianU16(FW_NEAR_UNKNOWN_14A3, selected_index);
}

/* RAM:fe2292 */
void PLEXTOR_NEAR_FUNCTION RefreshIsrcForCurrentRawTrack(void)
{
    u16 selected_index = ReadBigEndianU16(FW_NEAR_UNKNOWN_14A3);
    u16 record_index = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u || selected_index == 0u || selected_index > ReadBigEndianU16(FW_NEAR_UNKNOWN_B4BB))
        return;
    while(record_index < ReadBigEndianU16(FW_NEAR_UNKNOWN_B4BB))
    {
        u16 record_address = (u16)(0xb4bdu + (u16)(record_index << 3));

        if((FirmwareRead8(record_address) & 0x0fu) == 3u && FirmwareReadNear8((u16)(record_address + 1u)) == FirmwareReadNear8(FW_NEAR_UNKNOWN_14A8))
        {
            LoadIsrcCharactersForCurrentTrack();
            return;
        }
        record_index++;
    }
}

/* RAM:fe2c81 */
void PLEXTOR_NEAR_FUNCTION InitializeSubchannelQIdentificationState(void)
{
    u8 index;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1400, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1401, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1402, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1404, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1405, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1403, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1406, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1407, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1408, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1409, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_140A, 0u);
    for(index = 0u; index < 12u; index++)
    {
        FirmwareWriteNear8((u16)(0x140eu + index), 0u);
        FirmwareWriteNear8((u16)(0x141au + index), 0u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1426, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1427, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CD, 0x21u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CE, 0x42u);
}

/* RAM:fe2e40 */
void PLEXTOR_NEAR_FUNCTION InitializeRawReadTrackSelectionState(void)
{
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_149B, 0u);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_149D, 0u);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_14A1, 0xffu);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_14A3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AA, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AB, 0u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_14AC, 0xfffffffful);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_14B0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14D0, 0u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_14D1, 0u);
}

/* RAM:fe3047 */
void SnapshotAndAdjustRawReadQPacketMsf(u8 increment_relative_msf)
{
    u8 index;
    for(index = 0u; index < 10u; index++)
        FirmwareWriteNear8((u16)(0x1491u + index), FirmwareReadNear8((u16)(0x1272u + index)));
    if(increment_relative_msf != 0u)
        IncrementPackedBcdMsf(FW_NEAR_UNKNOWN_1494);
    else
        DecrementPackedBcdMsf(0x1494u);
    DecrementPackedBcdMsf(0x1498u);
}

/* RAM:fe2c5a */
void InitializeRawReadSectorControlState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B2, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B8, 0u);
}

/* RAM:fda4de */
void InitializeReadRuntimeAddressC35(void)
{
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_15C5, 0x00000c35ul);
}

/* RAM:f84eda */
void InitializeReadRuntimePointersAndFlags(void)
{
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_15B3, 0x00000faeul);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_15B7, 0x0000faabul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF5) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F972, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F973, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F975, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F974, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F976, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) & 0xfeu));
}

/* RAM:f84add */
void InitializeReadRuntimeState(void)
{
    InitializeReadRuntimePointersAndFlags();
    InitializeReadRuntimeAddressC35();
}

/* RAM:fb840a */
void ResetAtipState(void)
{
    u8 index;

    for(index = 0u; index < 3u; index++)
    {
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_FB53 + index), 0u);
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_FB57 + index), 0u);
    }
    for(index = 0u; index < 18u; index++)
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_FB7A + index), 0u);
    FirmwareWriteBigEndianU32(FW_NEAR_CURRENT_FRAME_POSITION, 0ul);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_114F, 0ul);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_0FE9, 0ul);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_FB90, 0ul);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_FB8C, 0ul);
    for(index = 0u; index < 2u; index++)
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_FB5D + index), 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5B, 0u);
}

#define PLEXTOR_BINDING_SECTION 40
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
u8 IsRawSectorTdiControlFlagSet(void)
{
    return IsRawSectorTdiControlFlagSetFromB(0u);
}

/* RAM:fe189f */
u8 IsRawSectorTdiControlFlagSetFromB(u8 inherited_b)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 0 || write_type == 1u || write_type == 3u)
        return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0);

    /* Reviewed callers establish write_type == 2 on this path.  The original
     * decrements without a bounds check until it reaches a type-one record. */
    if(write_type == 2u)
    {
        u16 index = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_149B);
        u16 record_address;

        index = (u16)(index - 1u);
        for(;;)
        {
            record_address = (u16)(0xb4bdu + (u16)(index << 3));
            if((FirmwareRead8(record_address) & 0x0fu) == 1u)
                break;
            index = (u16)(index - 1u);
        }
        return (u8)((FirmwareRead8(record_address) & 0x40u) != 0);
    }

    return inherited_b;
}

/* Reconstruction helper; no direct Ghidra function address. */
u8 SelectRawSectorTdiControlByteFromWriteState(void)
{
    return SelectRawSectorTdiControlByteFromR15(0u);
}

/* RAM:fe185e */
u8 SelectRawSectorTdiControlByteFromR15(u8 inherited_r15)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 0)
        return (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 0x02u) != 0 ? 0x91u : 0x90u;
    if(write_type == 1u || write_type == 2u || write_type == 3u)
        return IsRawSectorTdiControlFlagSetFromB(0u) != 0 ? 0x80u : 0;

    return inherited_r15;
}

/* Reconstruction helper; no direct Ghidra function address. */
static u16 RawReadRecordAddress(u16 record_index)
{
    return (u16)(0xb4bdu + (u16)(record_index << 3));
}

/* Reconstruction helper; no direct Ghidra function address. */
static void SetRawReadPreparationSegments(s32 first_count, u8 first_mode, s32 second_count, u8 second_mode)
{
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1430, (u32)first_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, first_mode);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1434, (u32)second_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, second_mode);
}

/* RAM:fe1a7b */
void InitializeRawReadPreparationSegmentsFromPlanCode(u8 read_plan_code)
{
    s32 frame_distance;
    u8 plan_is_2_to_6_or_8;
    u8 plan_is_7_9_or_10;

    if(read_plan_code == 3u || read_plan_code == 4u)
        SetRawReadPreparationSegments(75l, 3u, 150l, 2u);
    else if(read_plan_code == 5u || read_plan_code == 6u || read_plan_code == 8u)
        SetRawReadPreparationSegments(0l, 0u, 150l, 2u);
    else if(read_plan_code == 7u)
        SetRawReadPreparationSegments(0l, 0u, 5l, 2u);
    else if(read_plan_code == 10u || read_plan_code == 11u)
        SetRawReadPreparationSegments(0l, 0u, (s32)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0C4F), 1u);
    else if(read_plan_code == 2u)
        SetRawReadPreparationSegments(75l, 1u, 150l, 2u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u
        && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_14B3) == 1u
            || FirmwareReadNear8(FW_NEAR_UNKNOWN_14B7) == 1u))
    {
        SetRawReadPreparationSegments(0l, 0u, 5l, (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) <= 4 ? 3u : 2u);
    }
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        SetRawReadPreparationSegments(0l, 0u, 5l, 2u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 2u)
        return;

    frame_distance = (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) - FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39));
    plan_is_2_to_6_or_8 = (u8)(read_plan_code == 2u || read_plan_code == 3u || read_plan_code == 4u || read_plan_code == 5u || read_plan_code == 6u || read_plan_code == 8u);
    plan_is_7_9_or_10 = (u8)(read_plan_code == 7u || read_plan_code == 9u || read_plan_code == 10u);

    if(frame_distance > 150l)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1430, (u32)(frame_distance - 150l));
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1434, 150ul);
        if(plan_is_2_to_6_or_8 != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, read_plan_code == 2u ? 1u : 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 2u);
        }
        else if(plan_is_7_9_or_10 != 0u)
        {
            FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1430, 0u);
            FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1434, (u32)frame_distance);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 1u);
        }
        return;
    }

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1430, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, 0u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1434, (u32)frame_distance);
    if(plan_is_2_to_6_or_8 != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 2u);
    else if(plan_is_7_9_or_10 != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 1u);
}

/* RAM:fe0398 */
u8 PLEXTOR_NEAR_FUNCTION DecodeRawReadQRecordTypeCode(u16 record_index)
{
    u16 record_address;
    u8 field;
    u8 category;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146F, 0);
    record_address = RawReadRecordAddress(record_index);
    if(FirmwareReadNear8((u16)(record_address + 1u)) == 0xaau)
    {
        do
        {
            record_index = (u16)(record_index - 1u);
            record_address = RawReadRecordAddress(record_index);
        } while((FirmwareRead8(record_address) & 0x0fu) != 1u);
    }

    if((FirmwareRead8(record_address) & 0x0fu) != 1u)
        return 0;
    field = FirmwareReadNear8((u16)(record_address + 3u));
    category = (u8)(field & 0x30u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) == 1u)
        return 0x41u;
    if(category == 0x10u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146F, (u8)(field & 0x0fu));
        return 8u;
    }
    if(category == 0x20u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146F, (u8)(field & 0x0fu));
        return 13u;
    }
    if(category == 0x30u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146F, (u8)(field & 0x0fu));
        return 9u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146F, 0u);
    if((field & 0xc0u) == 0xc0u)
        return 2u;
    if((field & 0xc0u) == 0x40u)
        return 3u;
    return 0;
}

/* Reconstruction helper; no direct Ghidra function address. */
u8 SelectCurrentRawReadSectorClass(void)
{
    return SelectCurrentRawReadSectorClassFromR3(0u);
}

/* RAM:fe1a16 */
u8 PLEXTOR_NEAR_FUNCTION SelectCurrentRawReadSectorClassFromR3(u8 inherited_r3)
{
    u8 data_block_type;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        u16 record_index = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_149B);
        u16 record_address;

        record_index = (u16)(record_index - 1u);
        for(;;)
        {
            record_address = RawReadRecordAddress(record_index);
            if((FirmwareRead8(record_address) & 0x0fu) == 1u)
                break;
            record_index = (u16)(record_index - 1u);
        }
        data_block_type = DecodeRawReadQRecordTypeCode(record_index);
    }
    else
    {
        data_block_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    }

    if(data_block_type == 0 || data_block_type == 1u || data_block_type == 2u || data_block_type == 3u)
        return 0;
    if(data_block_type == 8u)
        return 1u;
    if(data_block_type == 9u || data_block_type == 10u || data_block_type == 11u || data_block_type == 12u || data_block_type == 13u)
        return 2u;

    return inherited_r3;
}

/* RAM:fe1640 */
void PLEXTOR_NEAR_FUNCTION UpdateRawReadQTypeStateFlag(void)
{
    u16 record_index;
    u16 record_address;
    u8 q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1280, (u8)(q_type == 11u || q_type == 13u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u)
        return;

    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149D) < FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B))
    {
        record_index = (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B) - 1u);
        for(;;)
        {
            record_address = RawReadRecordAddress(record_index);
            if((FirmwareRead8(record_address) & 0x0fu) == 1u)
                break;
            record_index = (u16)(record_index - 1u);
        }
    }
    else
    {
        record_index = 0;
    }
    record_address = RawReadRecordAddress(record_index);
    if(FirmwareReadNear8((u16)(record_address + 3u)) == 0x31u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1280, 1u);
}

/* Reconstruction helper; no direct Ghidra function address. */
u8 ComputeRawSectorPayloadOffsetFromWriteState(void)
{
    return ComputeRawSectorPayloadOffsetFromB(0u);
}

/* RAM:fe171d */
u8 ComputeRawSectorPayloadOffsetFromB(u8 inherited_b)
{
    u8 type_code = inherited_b;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u)
    {
        type_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, type_code);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        type_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, type_code);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        u16 record_index = (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B) - 1u);
        u16 record_address;

        for(;;)
        {
            record_address = RawReadRecordAddress(record_index);
            if((FirmwareRead8(record_address) & 0x0fu) == 1u)
                break;
            record_index = (u16)(record_index - 1u);
        }
        type_code = FirmwareReadNear8((u16)(record_address + 3u)) == 0x31u ? 13u : FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);
    }

    if(type_code <= 3u)
        return 0;
    if(type_code == 8u || type_code == 9u)
        return 0x10u;
    if(type_code >= 10u && type_code <= 13u)
        return 0x18u;
    return 0xffu;
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 41
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 DivideU32By100WithoutRuntimeLibrary(u32 dividend, u8 *remainder_out)
{
    u32 quotient = 0;
    u16 remainder = 0;
    u8 bit_count = 32;

    do
    {
        remainder = (u16)((u16)(remainder << 1) | (u16)(dividend >> 31));
        dividend <<= 1;
        quotient <<= 1;
        if(remainder >= 100u)
        {
            remainder = (u16)(remainder - 100u);
            quotient |= 1ul;
        }
        bit_count--;
    } while(bit_count != 0);
    *remainder_out = (u8)remainder;
    return quotient;
}

/* RAM:fe190e */
u32 EncodeTdiPacketSizeFromWriteParameters(void)
{
    u32 quotient;
    u32 result = 0;
    u16 digit_index = 0;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0 || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 0x02u) == 0)
        return 0xfffffffful;

    quotient = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B);
    while((s32)quotient > 0)
    {
        u8 remainder;
        u32 scaled_digit;
        u16 index = 0;

        quotient = DivideU32By100WithoutRuntimeLibrary(quotient, &remainder);
        scaled_digit = BinaryByteToPackedBcd(remainder);

        while((s16)index < (s16)digit_index)
        {
            u8 shift_count = 8;
            do
            {
                scaled_digit = (u32)(scaled_digit << 1);
                shift_count--;
            } while(shift_count != 0);
            index++;
        }
        result = (u32)(result + scaled_digit);
        digit_index++;
    }
    return result;
}

/* RAM:fddd02 */
u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_descriptor)
{
    u8 field_count = FirmwareRead8(mask_descriptor);
    fw_addr_t cursor = FirmwareAddressIncrementLow16(mask_descriptor, 2);
    u8 field_index = 1;

    while(field_index < field_count)
    {
        u8 field = FirmwareReadNear8((u16)(0x0b0au + field_index));
        u8 mask = FirmwareRead8(cursor);
        if((field & mask) != 0u)
            return 0;
        cursor = FirmwareAddressIncrementLow16(cursor, 1);
        field_index++;
    }
    return 1;
}

/* RAM:faff63 */
void SetSenseData(u8 sense_key, u16 asc_ascq)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B18, 0x70u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1A, sense_key);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B24, (u8)(asc_ascq >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B25, (u8)asc_ascq);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1B, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1C, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1D, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1E, 0);
}

/* RAM:faff7a */
void SetNoSenseData(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B18, 0x70u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1A, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B24, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B25, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1B, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1C, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1D, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1E, 0);
}

/* RAM:fcb96d */
void HandleScsiOpcodeEF(void)
{
    RefreshInquiryResponseFromDriveConfiguration();
    SetNoSenseData();
}

/* RAM:fa5355 */
void SetTrayOpenOrNoSenseForCurrentCommand(void)
{
    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x1bu && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E) & 0x02u) != 0u)
        SetNoSenseData();
    else if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x1eu && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E) & 0x01u) == 0u)
        SetNoSenseData();
    else
        SetSenseData(0x02u, 0x3a02u);
}

/* RAM:fdc22b */
void FinalizeScsiWriteCommandDispatchState(void)
{
    u8 opcode = FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE);

    if(opcode == 0x0au || opcode == 0x2au || opcode == 0xaau)
        return;
    if(opcode == 3u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1597) != 1u)
            return;
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x51u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 0xb0u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 3u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 5u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1597) == 1u)
    {
        SetSenseData(4u, 0x0803u);
    }
}

/* RAM:fdc9bd */
void CompleteReadAcquisitionRequest(u8 result)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, result);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
}

/* RAM:fdcb2e */
void PublishReadAcquisitionTarget(void)
{
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_0E3B, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
}

/* RAM:fdca07 */
void HandleReadAcquisitionRequest1(void)
{
    if(FirmwareReadBit000E() == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 2u);
    PublishReadAcquisitionTarget();
}

/* RAM:f927bd */
void ResetRawReadQueueAndTransferState(void)
{
    u32 slot_count;

    InitializeControllerBufferCursorRegistersIfNeeded();
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1292, 0u);
    slot_count = FirmwareReadNearU32(FW_NEAR_UNKNOWN_156A);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1ul));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6ul));
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_159C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F48, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F46, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F07, 0u);
    FirmwareWriteBit000F(0u);
    FirmwareWriteBit0008(0u);
    FirmwareWriteBit0022(0u);
}

/* RAM:fe2ac4 */
/* The separately accounted two-byte entry thunk is at RAM:fe2abe. */
u8 RestartReadAcquisitionCoroutineForProducerWaitBody(void)
{
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) == 1u)
        YieldCoroutine0();

    InitializeCoroutine3(FW_ENTRY_RUN_READ_ACQUISITION_COROUTINE3);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B4, 1u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_14BC, 0x80000000ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C0, 0u);
    YieldCoroutine0();
    return 1u;
}

/* RAM:fdd30c */
/* Native ERET entry; the C
 * translation delegates only the two already-linked native resume boundaries. */
void ApplyPendingDriveInitializationRestart(void)
{
    u8 operation_mode = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
    u8 action_kind;

    if(operation_mode == 2u)
        return;
    if(operation_mode != 1u && operation_mode != 3u)
        goto restart;

    action_kind = FirmwareReadNear8(FW_NEAR_FORMAT_OPERATION_RESULT);
    if((action_kind == 2u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u) || (action_kind == 1u && (s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >= 0)
        || (action_kind == 3u && (FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 1u) == 0u))
    {
restart:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0u);
        InitializeCoroutine1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
        InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        ResumeCoroutine1();
        ResumeCoroutine2();
    }
}

/* RAM:fdd374 */
/* Native ERET entry; physical
 * roles of the debounced P1 inputs remain intentionally unnamed. */
void ServiceP1Bit5AndBit7DriveStateTransition(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x10u) == 0u)
        return;
    if(IsDebouncedP1Bit7Low() != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
            ClearP3Bit0AndBit1Atomic();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 3u);
        return;
    }
    if((IsDebouncedP1Bit5Low() == 0u || TakeStateFlag11EFBit5() != 0u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) != 9u)
    {
        ResetControllerChannel2();
        InitializeCoroutine1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COMPLETION_COROUTINE1);
    }
}

/* RAM:fdda2f */
/* The
 * readiness wait on RAM:001302 bit five is intentional and has no timeout. */
void ServiceControllerRequestFlag11E5Bit2(void)
{
    u8 request_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5);
    u8 controller_flags;

    if((request_flags & 4u) == 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(request_flags & 0xfbu));
    controller_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_11D7);
    if((controller_flags & 0x20u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D7, (u8)(controller_flags | 0x20u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802A, 0x0bu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 2u));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x40u);
        while((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x20u) == 0u)
        {
        }
#define PLEXTOR_BINDING_SECTION 42
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0xc8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u));
#define PLEXTOR_BINDING_SECTION 43
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD));
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D7, (u8)(controller_flags & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802A, 3u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
#define PLEXTOR_BINDING_SECTION 44
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0xc0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u));
#define PLEXTOR_BINDING_SECTION 45
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
}

/* RAM:fde934 */
void CompleteSingleHostTransferBuffer(void)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
#define PLEXTOR_BINDING_SECTION 46
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 0x0fu)
        return;
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_1298, (u16)((FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1298) + 1u) & FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A)));
    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0F4E) != 0u)
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0F4E, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0F4E) - 1u));
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_1459, FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_1459) + 1ul);
}

/* RAM:fdcaac */
void HandleReadAcquisitionRequest5(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    PublishReadAcquisitionTarget();
}

/* RAM:fdccf2 */
void SetSenseDataFromStatusCode(u8 status_code)
{
    SetSenseData(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + status_code), ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + ((fw_addr_t)status_code << 1)));
}

/* RAM:fdc96f */
void DispatchPendingReadAcquisitionRequest(void)
{
    u8 request_kind;

    /* The source clears bit zero before testing the existing completion code. */
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfeu));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        CompleteReadAcquisitionRequest(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
        return;
    }

    /* These are intentionally separate volatile read-modify-write sequences. */
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xefu));
    request_kind = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C8C);

    /* The table at RAM:ff9617 contains exactly these seven 24-bit handler addresses.  The
     * original has no range check: values outside 0..6 are a caller contract
     * and fall through an unrelated table entry, so they are not assigned a
     * made-up C outcome here. */
    switch(request_kind)
    {
    case 0u:
        HandleReadAcquisitionRequest0();
        break;
    case 1u:
        HandleReadAcquisitionRequest1();
        break;
    case 2u:
        HandleReadAcquisitionRequest2();
        break;
    case 3u:
        HandleReadAcquisitionRequest3();
        break;
    case 4u:
        HandleReadAcquisitionRequest4();
        break;
    case 5u:
        HandleReadAcquisitionRequest5();
        break;
    case 6u:
        HandleReadAcquisitionRequest6();
        break;
    default:
        break;
    }
}

/* RAM:fdcc22 */
u8 DispatchAndWaitForReadAcquisitionResult(void)
{
    u8 result;

    FirmwareWriteBit0580(0u);
    FirmwareWriteBit0581(0u);
    FirmwareWriteBit000E(0u);
    DispatchPendingReadAcquisitionRequest();
    YieldCoroutine0();
    for(;;)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) == 0u)
        {
            FirmwareWriteBit0580(1u);
            FirmwareWriteBit0581(0u);
            result = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41);
            SetSenseDataFromStatusCode(result);
            return result == 0u ? 1u : 0u;
        }
        YieldCoroutine0();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
            FirmwareWriteBit0580(1u);
            FirmwareWriteBit0581(0u);
            return 0u;
        }
    }
}

/* RAM:fbcde5 */
u8 PLEXTOR_NEAR_FUNCTION FUN_RAM_fbcde5(u32 caller_target)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    u32 target = caller_target;
    u8 result;

    if(write_type == 0u)
        target = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45);
    else if(write_type == 1u)
        target = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39);
    else if(write_type == 2u || write_type == 3u)
        target = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD69);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
    FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_0C8E, target);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) | 0x04u));
    result = DispatchAndWaitForReadAcquisitionResult();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
    return result != 0u ? 1u : 0u;
}
/* RAM:fdafc1 */
void ClearScsiCompletionBits0And1IfBit4Set(void)
{
    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xefu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xfdu));
    }
}

/* RAM:fb01c2 */
u8 IsDebouncedP1Bit5Low(void)
{
    return (FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 0x20u) != 0 ? 1u : 0u;
}

/* RAM:fb0664 */
u8 IsDebouncedP1Bit6Low(void)
{
    return (FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 0x40u) != 0 ? 1u : 0u;
}

/* RAM:fb01d0 */
u8 IsDebouncedP1Bit7Low(void)
{
    return (FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 0x80u) != 0 ? 1u : 0u;
}

/* RAM:f8f63f */
void ApplyConfigurationFlagE750Bit0ToState1303Bit0(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) | 1u));
}

/* RAM:fb063c */
u8 TakeStateFlag11EFBit5(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x20u) == 0)
        return 0;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xdfu));
    return 1;
}

/* RAM:fbb2b4 */
void ClearDecodedTocPointSummariesFromIndex(u8 first_point_index)
{
    u8 point_index = first_point_index;

    while(point_index < 0x65u)
    {
        u8 byte_index = 0;
        u16 record_address = (u16)(0xf2b6u + (u16)point_index * 6u);

        while(byte_index < 6u)
        {
            FirmwareWriteNear8((u16)(record_address + byte_index), 0);
            byte_index++;
        }
        point_index++;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1A, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1B));
}

/* RAM:fcecb6 */
u8 PLEXTOR_NEAR_FUNCTION IsDecodedFramePositionWithinTwoFramesOfTarget(s32 target_frame_position)
{
    u32 target_bits = (u32)target_frame_position;
    u32 current_frame_position;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3F) > 2u)
        return 0;
    current_frame_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    if((s32)current_frame_position < (s32)target_bits)
        return 0;
    return (u8)((s32)current_frame_position <= (s32)(target_bits + 2ul));
}

/* RAM:fdaf0d */
u8 IsDriveOperationStatusScsiEligible(void)
{
    u8 operation_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_1009);

    return (u8)(operation_status != 1u && operation_status != 2u && operation_status != 6u && operation_status != 7u);
}

/* RAM:fbc504 */
u8 IsFixedPacketAddressExpansionEnabled(void)
{
    if(FirmwareReadBit000B() == 0u)
        return 0;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        return 0;
    return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) != 0u);
}

/* RAM:fef0f3 */
u8 IsRawReadQueueAtHighWater(void)
{
    return (u8)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129C) <= FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292));
}

/* RAM:fae444 */
u8 IsTrackTableTailEntryUninitialized(void)
{
    u8 high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);

    if(high_index != FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
        return 0;
    return (u8)(FirmwareReadNear8((u16)(0xdd87u + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E) * 0x13u)) == 0u);
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 47
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd0ce2 */
void ApplyFixedControllerParameterTable(u8 table_index)
{
    fw_addr_t pointer_entry = FirmwareAddressIncrement24(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF857C), (u32)table_index << 2);
    fw_addr_t record = FirmwareReadBigEndianU32(pointer_entry);
    u16 selector;

    while((selector = FirmwareReadBigEndianU16FarLow16(record)) != 0xffffu)
    {
        u16 value = FirmwareReadBigEndianU16FarLow16(FirmwareAddressIncrementLow16(record, 2u));

        SubmitControllerWordPair(value, selector);
        record = FirmwareAddressIncrementLow16(record, 4u);
    }
}

/* RAM:fc19fb */
u8 FindTrackTableEntryByNumber(u8 track_number)
{
    u8 entry_index;

    if(track_number == 0u)
        return 1u;

    for(entry_index = 1u; entry_index < 100u; entry_index++)
        if(FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD92, (u16)((u16)entry_index * 19u))) == track_number)
            return entry_index;
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u);
}

/* RAM:fdb3dc */
u8 PLEXTOR_NEAR_FUNCTION ApplyAtaSetFeaturesCommand(void)
{
    u8 feature = FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0);
    u8 mode;
    u8 mode_class;
    u8 index;

    if(feature == 0x5du || feature == 0x5eu || feature == 0x66u || feature == 0xccu || feature == 0xddu || feature == 0xdeu)
        return 1u;
    if(feature != 3u)
        return 0u;

    mode = FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT1);
    mode_class = (u8)(mode & 0xf8u);
    index = (u8)(mode & 7u);
    if(mode_class == 0u)
        return 1u;
    if(mode_class == 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2E, index <= 4u ? index : 4u);
        return 1u;
    }
    if(mode_class == 0x20u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2D, index <= 2u ? index : 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 3u);
    }
    else if(mode_class == 0x40u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 8u) == 0u || index > 2u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2C, index);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 7u);
    }
    else
    {
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FE, mode);
    return 1u;
}

/* RAM:fdafeb */
void ConfigureAtaDataTransferStatus(void)
{
    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PROTOCOL) == 0u)
        return;
    if((FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 1u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 1u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2C) + 1u));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805D, (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01805D) | 8u));
        return;
    }
    /* Separate source loads matter: opcode and protocol are reread. */
    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xbeu || FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x28u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) | 0x0cu));
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38) >= 0x10u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xaau || FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x2au))
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B));
    else
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) | 4u));
}

/* RAM:fdaf6f */
void PublishAtaCommandCompletion(void)
{
    u8 status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2F);

    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, status);
    if(FirmwareReadController8(FW_CONTROLLER_COMMAND_TRIGGER) == 0x50u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x20u) != 0u)
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C30));
    else
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C30) | 4u));

    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PROTOCOL) != 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 3u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
    }
    else
    {
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0x80u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 5u);
}

/* RAM:fdaf3e */
void PrepareAtaCompletionStatusFromSense(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B1A) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0x50u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0x51u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B1A) << 4) & 0xf0u));
    }
}

/* RAM:fe9cce */
u8 PLEXTOR_NEAR_FUNCTION IsRawReadPreparationStateEligible(u8 alternate_state)
{
    if(alternate_state != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1438) == 3u)
            return 1;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1438) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1439) == 2u)
            return 1;
    }
    return (u8)(alternate_state == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1439) == 2u);
}

/* RAM:fe12d9 */
u8 PLEXTOR_NEAR_FUNCTION IsMediaCatalogNumberAvailable(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 0u || write_type == 1u)
        return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C51) & 0x80u) != 0u);
    if(write_type != 2u)
        return 0;
    if((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598) < 0 || FirmwareReadNear8(FW_NEAR_UNKNOWN_14A8) == 0xaau)
        return 0;
    return (u8)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_14A1) == 0u);
}

/* RAM:fef07b */
u16 GetLatestQueuedRawSectorDescriptorAddress(void)
{
    u16 queued_count = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292);
    u16 producer_index;
    u16 index_mask;
    u16 descriptor_base;

    if(queued_count == 0u)
        return 0;
    producer_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1296);
    index_mask = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A);
    descriptor_base = (u16)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_129E);
    return (u16)(descriptor_base + (u16)(((u16)(producer_index - 1u) & index_mask) * 0x16u));
}

/* RAM:fef09e */
u16 GetRawReadSecondaryCursorDescriptorAddress(void)
{
    u16 cursor = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1298);
    u16 descriptor_base = (u16)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_129E);

    return (u16)(descriptor_base + (u16)(cursor * 0x16u));
}

/* RAM:fef0af */
u16 GetRawReadConsumerDescriptorAddressOrZero(void)
{
    u16 consumer_index;
    u16 descriptor_base;

    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) == 0u)
        return 0;
    consumer_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1294);
    descriptor_base = (u16)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_129E);
    return (u16)(descriptor_base + (u16)(consumer_index * 0x16u));
}

/* RAM:fc2335 */
u8 GetModePagePayloadLength(u8 page_code)
{
    switch(page_code)
    {
    case 0x01u:
        return 0x0au;
    case 0x02u:
        return 0x0eu;
    case 0x03u:
        return 0x06u;
    case 0x05u:
        return 0x32u;
    case 0x07u:
        return 0x06u;
    case 0x08u:
        return 0x0au;
    case 0x0du:
        return 0x06u;
    case 0x0eu:
        return 0x0eu;
    case 0x1au:
        return 0x0au;
    case 0x1du:
        return 0x08u;
    case 0x2au:
        return 0x3eu;
    default:
        return 0;
    }
}

/* RAM:f98ea1 */
u8 GetScsiAudioStatusCode(u8 acknowledge_terminal_status)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 2u) != 0u)
    {
        if(acknowledge_terminal_status != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfdu));
        return FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBC) == 0u ? 0x13u : 0x14u;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 4u) == 0u)
        return 0x15u;
    return FirmwareReadBit000E() != 0u ? 0x11u : 0x12u;
}

/* RAM:fdc8e3 */
u16 GetRawReadConsumerDescriptorAddressAtOffset(u16 relative_index)
{
    u16 consumer_index;
    u16 index_mask;
    u16 descriptor_base;

    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) == 0u)
        return 0;
    consumer_index = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1294);
    index_mask = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A);
    descriptor_base = (u16)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_129E);
    return (u16)(descriptor_base + (u16)(((u16)(relative_index + consumer_index) & index_mask) * 0x16u));
}

/* RAM:fc2322 */
u8 PLEXTOR_NEAR_FUNCTION DoesModePagePayloadLengthMatch(u8 page_code, u8 payload_length)
{
    return (u8)(GetModePagePayloadLength(page_code) == payload_length);
}

/* RAM:fd4d56 */
u8 GetEffectiveFirstTrackNumber(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u)
        return FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u)
        return 1u;
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
}

/* RAM:fb5077 */
u8 IsOpcRequiredForWriteSpeed(u8 write_speed_x)
{
    u8 calibration_speed_class;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E74E) == 0u)
        return 1;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(write_speed_x >= 0x10u)
        {
            calibration_speed_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74D);
            return (u8)(calibration_speed_class != 0x10u);
        }
        if(write_speed_x >= 8u)
        {
            calibration_speed_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74D);
            return (u8)(calibration_speed_class != 8u);
        }
        if(write_speed_x >= 4u)
        {
            calibration_speed_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74D);
            return (u8)(calibration_speed_class != 4u);
        }
        return 1;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
    {
        if(write_speed_x < 0x18u)
            return 1;
        calibration_speed_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74D);
        return (u8)(calibration_speed_class != 0x10u);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
    {
        if(write_speed_x >= 10u)
        {
            calibration_speed_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74D);
            return (u8)(calibration_speed_class != 10u);
        }
        if(write_speed_x >= 4u)
        {
            calibration_speed_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74D);
            return (u8)(calibration_speed_class != 4u);
        }
        return 1;
    }
    if(write_speed_x < 4u)
        return 1;
    calibration_speed_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74D);
    return (u8)(calibration_speed_class != 4u);
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ConvertRawReadPreparationBinaryMsfToAdjustedFramePosition(u8 minutes, u8 seconds, u8 frames)
{
    u16 total_seconds = (u16)((u16)minutes * 60u + seconds);
    s32 frame_position = (s32)(((u32)total_seconds << 6) + ((u32)total_seconds << 3) + ((u32)total_seconds << 1) + total_seconds + frames);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_102F) == 1u && (FirmwareReadDirectSfr8(D1) & 0x20u) == 0u)
        frame_position -= 450000l;
    return frame_position;
}

/* RAM:fe9df3 */
u8 PLEXTOR_NEAR_FUNCTION CompareFramePositionWithRawReadPreparationRecord(s32 frame_position, u16 record_index)
{
    u16 record = (u16)(0xb4bdu + (u16)((u32)record_index << 3));
    u8 minutes = FirmwareReadNear8((u16)(record + 4u));
    u8 seconds = FirmwareReadNear8((u16)(record + 6u));
    u8 frames = FirmwareReadNear8((u16)(record + 7u));
    s32 record_frame_position = ConvertRawReadPreparationBinaryMsfToAdjustedFramePosition(minutes, seconds, frames);

    if(record_frame_position < frame_position)
        return 2u;
    if(frame_position == record_frame_position)
        return 0u;
    return 1u;
}

/* RAM:faadc6 */
void InitializeDefaultTdiPayloadPrefixAtAddress(fw_addr_t output_address)
{
    static const u8 prefix[13] = { 0x54u, 0x44u, 0x49u, 0x01u, 0x50u, 0x01u, 0x01u, 0x01u, 0x01u, 0x91u, 0x00u, 0x00u, 0x32u };
    u8 offset;

    /* DR0's bank is held constant while WR6 advances the low word. */
    for(offset = 0u; offset < 24u; offset++)
        FirmwareWrite8(FirmwareAddressIncrementLow16(output_address, offset), 0u);
    for(offset = 0u; offset < sizeof(prefix); offset++)
        FirmwareWrite8(FirmwareAddressIncrementLow16(output_address, offset), prefix[offset]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1454, 24u);
}

/* RAM:fddb60 */
void MaskReadMeasurementStateWordToLow17Bits(void)
{
    u32 value = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1548) & 0x0001fffful;

    FirmwareWriteNearU32(FW_NEAR_UNKNOWN_1548, value);
}

/* RAM:fddb0c */
void FormatMeasurementStatusNibbles(void)
{
    u8 packed;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D0, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD3) >> 4));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD3) & 0x0fu));
    packed = BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_123D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D3, (u8)(packed >> 4));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D2, (u8)(packed & 0x0fu));
    packed = BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_123E));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D1, (u8)(packed >> 4));
}

/* RAM:fd4ff9 */
void ClearWriteBufferSnapshotValid(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1159, 0);
}

/* RAM:fd4fff */
void SetWriteBufferSnapshotValid(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1159, 1u);
}

/* RAM:fd5006 */
u8 IsWriteBufferSnapshotValid(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1159) != 0);
}

/* RAM:fd5d8c */
u16 CalculateControllerBufferChecksum28E0(fw_addr_t start_offset)
{
    u16 checksum = 0u;
    u16 index;

    for(index = 0u; index < 0x28e0u; ++index)
        checksum = (u16)(checksum + ReadControllerBufferByteAtomicForChecksum(FirmwareAddressIncrement24(start_offset, index)));
    return checksum;
}

/* RAM:fd4f9c */
u8 ValidateRetainedWriteBufferSnapshotChecksum(void)
{
    u16 expected_checksum;

    if(IsWriteBufferSnapshotValid() == 0u)
        return 0u;
    expected_checksum = (u16)((u16)ReadControllerBufferByteAtomicForChecksum(0x28e0ul) << 8);
    expected_checksum = (u16)(expected_checksum | ReadControllerBufferByteAtomicForChecksum(0x28e1ul));
    if(CalculateControllerBufferChecksum28E0(0u) == expected_checksum)
        return 1u;
    ClearWriteBufferSnapshotValid();
    return 0u;
}

/* RAM:fd4d9d */
s32 SelectWorkingTrackFramePosition(void)
{
    u8 track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    s32 start = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_DD3D);

    if(track_type == 1u)
        return (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_DD45);
    if(track_type == 3u)
    {
        s32 current = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_DD45);

        return current >= start ? current : start;
    }
    if(track_type == 4u)
        return (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_DD41);
    if(track_type == 5u || track_type == 6u)
        return (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_DD51);
    return start;
}

/* RAM:fd4e0e */
/* The C parameters expose the
 * verified native ACC/R6/R7/R5 register inputs without generalizing an ABI. */
u8 ValidatePlayAudioTrackIndexRange(u8 start_track_number, u8 end_track_number, u8 start_index_number, u8 end_index_number)
{
    u8 last_track_number;

    if(start_track_number == 0u || end_track_number == 0u || end_track_number < start_track_number || start_index_number > 1u || end_index_number > 1u)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        last_track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
        if(start_track_number > last_track_number)
        {
            HandleUnsupportedScsiCommand();
            return 0u;
        }
        if(end_track_number <= last_track_number && (FirmwareReadNear8((fw_addr_t)(FW_NEAR_UNKNOWN_F2B7 + (u16)start_track_number * 6u)) & 0x40u) != 0u)
        {
            SetSenseData(5u, 0x6400u);
            return 0u;
        }
        return 1u;
    }
    last_track_number = GetEffectiveLastTrackNumber();
    if(last_track_number == 0u)
    {
        SetSenseData(5u, 0x2100u);
        return 0u;
    }
    if(start_track_number > last_track_number)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }
    if(end_track_number <= last_track_number)
    {
        (void)LoadTrackInformationByNumberFromInheritedB(start_track_number, (u8)(((u16)start_track_number * 19u) >> 8));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u)
        {
            SetSenseData(5u, 0x6400u);
            return 0u;
        }
    }
    return 1u;
}

/* RAM:fdaea2 */
/* Native ERET returns its
 * Boolean result in ACC; this C value return models only that local exit. */
u8 CheckScsiCommandAllowedForDriveOperationState(void)
{
    u8 drive_operation_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C80);
    u8 gate_address;

    if(drive_operation_state == 5u)
    {
        gate_address = 0x81u;
    }
    else if(drive_operation_state == 6u)
    {
        u8 opcode = FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE);

        gate_address = (opcode == 0x16u || opcode == 0x17u || opcode == 0x56u || opcode == 0x57u) ? 0x81u : 0x82u;
    }
    else
    {
        return 1u;
    }
    if(FirmwareReadNear8((fw_addr_t)(FW_NEAR_UNKNOWN_0C00 + gate_address)) == 0u)
        return 1u;
    SetSenseData(5u, 0x8100u);
    return 0u;
}

/* RAM:fe25a8 */
void UpdateRawReadStateFlagFromModeAndCurrentQ(void)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1281);

    if(mode == 0 || mode == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0);
    else if(mode == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) == 0));
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 48
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:faffb4 */
void SetCommandSequenceErrorSense(void)
{
    SetSenseData(5u, 0x2c00u);
}

/* RAM:faffd2 */
void SetSenseInformation(u32 information)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B18) | 0x80u));
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B1B, information);
}

/* RAM:fe2a80 */
void PLEXTOR_NEAR_FUNCTION SetRawReadPositionCommandSequenceError(void)
{
    SetCommandSequenceErrorSense();
    SetSenseInformation(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598));
}

/* RAM:fe2a8d */
void PLEXTOR_NEAR_FUNCTION ReportSense030C08AtRawReadTarget(void)
{
    SetSenseData(3u, 0x0c08u);
    SetSenseInformation(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598));
}

/* RAM:fe2a44 */
void ReportSense030C09FromRawReadQueue(void)
{
    u16 queued_count;
    u32 information;

    LatchFirstReadFailureContext();
    SetSenseData(3u, 0x0c09u);
    queued_count = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292);
    if((s16)queued_count > 0)
    {
        u16 descriptor_address = GetLatestQueuedRawSectorDescriptorAddress();
        u16 descriptor_self = FirmwareReadBigEndianU16Near((u16)(descriptor_address + 2u));
        u16 position_high = FirmwareReadBigEndianU16Near(descriptor_self);

        queued_count = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292);
        information = ((u32)position_high << 16) | descriptor_address;
        information -= queued_count;
    }
    else
    {
        information = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598);
    }
    SetSenseInformation(information);
}

/* RAM:f84998 */
void AppendConditionalInquiryExtensionBytes(void)
{
    u8 offset;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7F) != 0x81u)
        return;

    offset = 0x2eu;
    do
    {
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_DCC5 + (fw_addr_t)offset), 0x20u);
        offset++;
    } while(offset < 0x32u);

    offset = 0u;
    do
    {
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_DCF7 + (fw_addr_t)offset), FirmwareReadNear8((fw_addr_t)(FW_NEAR_UNKNOWN_DD25 + (fw_addr_t)offset)));
        offset++;
    } while(offset < 6u);
}

/* RAM:f89587 */
void PLEXTOR_NEAR_FUNCTION CopyCurrentDecodedFramePositionToReadAcquisitionTarget(void)
{
    u32 frame_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, frame_position);
}

/* RAM:f89590 */
void StoreWriteSpeedCalibrationOffset(void)
{
    u8 write_speed_x = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);

    if(write_speed_x == 8u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FABE, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADE));
    else if(write_speed_x == 4u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FABD, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADD));
    else if(write_speed_x == 16u || write_speed_x == 20u || write_speed_x == 24u || write_speed_x == 32u || write_speed_x == 40u || write_speed_x == 52u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FABF, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADF));
}

/* RAM:f8858b */
u8 ReturnMediaState6CalibrationSuccess(void)
{
    return 1u;
}

/* RAM:f8f652 */
void ApplyFourSpeedWriteCalibrationOffset(void)
{
    u8 count = FirmwareReadNear8(FW_NEAR_UNKNOWN_158B);
    u16 multiplier;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) != 4u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) == 0u)
        return;

    multiplier = (s8)count < (s8)1 ? 0x009cu : 0x00a0u;
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FACF, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1589) + (u16)((u16)count * multiplier)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) | 4u));
}

/* RAM:f8f693 */
void RestoreFourSpeedWriteCalibrationOffset(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 4u) == 0u)
        return;

    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_FACF, FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1589));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfbu));
}

/* RAM:f955e4 */
u8 IsControllerMeasurementSeriesRetryEligible(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 1u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 3u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 4u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x20u) != 0u)
        return 0u;
    if((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7A) | FirmwareReadNear8(FW_NEAR_UNKNOWN_EF79)) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) == 1u)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) | 0x10u));
    return 0u;
}

/* RAM:f96141 */
void PLEXTOR_NEAR_FUNCTION SetControllerMeasurementTerminalStatus0E02(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F05D, 0x0eu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F05E, 2u);
}

/* RAM:fb0b8e */
void SetStartupMaintenanceFlagForActiveDriveState(void)
{
    if(IsIndexedDriveStateActive(0x1fu) != 1u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB1) != 1u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC4, 1u);
}

/* RAM:fb9ceb */
void SnapshotMeasurementDescriptorTargetPositionIfEnabled(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) & 1u) == 0u)
        return;
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_FB6E, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_FBA2));
}

/* RAM:fb9cfc */
void UpdateMeasurementPositionSnapshotState(void)
{
    u32 snapshot_position;
    u32 current_position;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) & 1u) == 0u)
        return;

    snapshot_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_FB6E);
    current_position = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    if((s32)current_position > (s32)(snapshot_position - 1ul))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) | 2u));
        return;
    }
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_FB76, 0ul);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_FB6A, 0ul);
}

/* RAM:fbb2c2 */
void PLEXTOR_NEAR_FUNCTION ReturnFromFullTocAppendNoOp(void) {}

/* RAM:fbc2ec */
u8 ValidatePmaStartBeforeAtipLeadInForWrite(void)
{
    u8 write_type;
    u8 working_track_type;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E74C) == 0u)
        return 1u;

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type == 2u || write_type == 3u)
        return 1u;

    working_track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    if(working_track_type != 0u && working_track_type != 1u && working_track_type != 6u)
        return 1u;

    if((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1143) < (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F))
        return 1u;

    SetSenseData(3u, 0x7305u);
    return 0u;
}

/* RAM:fbc33d */
void PLEXTOR_NEAR_FUNCTION SelectWorkingTrackWriteCursorFramePosition(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39));
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39));
        return;
    }
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45));
}

/* RAM:fbd098 */
void PLEXTOR_NEAR_FUNCTION SelectAlternateWorkingTrackWriteCursorFramePosition(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39));
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39));
        return;
    }
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD49));
}

/* RAM:fbe54c */
void ApplyVendorF3ControllerWordInRange(u16 value)
{
    if(value > 0x5dc0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_F087, value);
    WriteControllerParametersEAEBFromScaledValue(value);
    SetNoSenseData();
}

/* RAM:fbe11f */
void CopyVendorF2MeasurementResponsePrefix(void)
{
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0BB0, FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_F06B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0BB2, FirmwareReadNear8(FW_NEAR_UNKNOWN_F06D));
}

/* RAM:fbe98f */
void ClearVendorF3MeasurementStateAndSense(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfeu));
    SetNoSenseData();
}

/* RAM:fbe56d */
void ExecuteVendorF3MeasurementParameterSetup(u8 value)
{
    WriteControllerParameterWord(0xee01u);
    WriteControllerParameterEF00AndConditionalD0Value(value);
    SetNoSenseData();
}
/* RAM:fb46ea */
void CompleteScsiCommandWithNoSense(void)
{
    SetNoSenseData();
    PrepareAtaCompletionStatusFromSense();
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
    PublishAtaCommandCompletion();
}

/* RAM:fde0fb */
void SetSenseDataFromPendingResultCode(void)
{
    u8 result_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAAB);

    SetSenseData(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F843F7) + (fw_addr_t)result_code), FirmwareReadBigEndianU16Near((u16)(0x4416u + ((u16)result_code << 1))));
}

/* RAM:fc22f9 */
u8 ValidateModeParameterBytesAgainstMaskRecord(fw_addr_t mask_record_address, fw_addr_t parameter_address)
{
    u8 byte_count = FirmwareRead8(FirmwareAddressIncrementLow16(parameter_address, 1u));
    u8 index = 0u;
    fw_addr_t parameter_cursor = FirmwareAddressIncrementLow16(parameter_address, 2u);
    fw_addr_t mask_cursor = FirmwareAddressIncrementLow16(mask_record_address, 2u);

    while(index < byte_count)
    {
        u8 mask = FirmwareRead8(mask_cursor);
        u8 value = FirmwareRead8(parameter_cursor);

        if((value & mask) != 0u)
            return 0u;
        parameter_cursor = FirmwareAddressIncrementLow16(parameter_cursor, 1u);
        mask_cursor = FirmwareAddressIncrementLow16(mask_cursor, 1u);
        index++;
    }
    return 1u;
}

/* RAM:fc24ac */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage03ParameterList(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 3u)
        return 0u;
    return ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A52), FW_NEAR_UNKNOWN_0B2A);
}

/* RAM:fc27b4 */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage07ParameterList(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 7u)
        return 0u;
    return ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B1E), FW_NEAR_UNKNOWN_0B2A);
}

/* RAM:fc2999 */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage1AParameterList(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 0x1au)
        return 0u;
    return ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B8A), FW_NEAR_UNKNOWN_0B2A);
}

/* RAM:fc29f1 */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage1DParameterList(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 0x1du)
        return 0u;
    return ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8BAE), FW_NEAR_UNKNOWN_0B2A);
}

/* RAM:fc27db */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage06ParameterList(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 6u)
        return 0u;
    if(ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B42), FW_NEAR_UNKNOWN_0B2A) == 0u)
        return 0u;
    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B2E) != 0x003cu)
        return 0u;
    return (u8)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B30) == 0x004bu);
}

/* RAM:fc286d */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage0EParameterList(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 0x0eu)
        return 0u;
    if(ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B5A), FW_NEAR_UNKNOWN_0B2A) == 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B32) > 3u)
        return 0u;
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B34) <= 3u);
}

/* RAM:fc257c */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage05ParameterList(void)
{
    u8 page_byte;
    u8 saved_runtime_bit;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 0x32u)
        return 0u;
    if(ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A6A), FW_NEAR_UNKNOWN_0B2A) == 0u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C) & 0x0fu) >= 4u)
        return 0u;

    /* The source records this bit in saved R15 then restores R15 before RET.
     * Keep the ordered volatile read in a value whose impossible 0xff state
     * cannot affect a source-reachable result. */
    saved_runtime_bit = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E642) & 0x10u);

    page_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D);
    if((page_byte & 0xc0u) == 0x80u)
        return 0u;
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D) & 0x0du) & 0x0cu) == 0x0cu)
        return 0u;
    page_byte = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2E) & 0x0fu);
    if((page_byte >= 4u && page_byte <= 7u) || (page_byte >= 0x0eu && page_byte <= 0x0fu))
        return 0u;
    page_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B32);
    if(saved_runtime_bit == 0xffu)
        return 0u;
    return (u8)(page_byte == 0u || page_byte == 0x10u || page_byte == 0x20u);
}

/* RAM:fc2ee7 */
fw_addr_t SelectModePageImageAddressForPageControl(u8 page_code, u8 page_control)
{
    switch(page_code)
    {
    case 0x01u:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E623;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A16);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A0A);
        break;
    case 0x02u:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E62F;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A42);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A32);
        break;
    case 0x03u:
        if(page_control == 0u)
            return FW_HUGE_UNKNOWN_2BC6EF;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A62);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A5A);
        break;
    case 0x05u:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E63F;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8AD2);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A9E);
        break;
    case 0x0au:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E687;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B52);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B4A);
        break;
    case 0x0bu:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E67B;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B36);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B2A);
        break;
    case 0x0du:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E673;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B16);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B0E);
        break;
    case 0x0eu:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E68F;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B7A);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B6A);
        break;
    case 0x1au:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E69F;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8BA2);
        if(page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B96);
        break;
    case 0x1du:
        if(page_control == 0u || page_control == 2u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8BB8);
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8BC2);
        break;
    case 0x2au:
        if(page_control == 0u)
            return FW_NEAR_UNKNOWN_E6AB;
        if(page_control == 1u)
            return PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8C10);
        if(page_control == 2u)
        {
            WriteCdSpeedKilobytesPerSecond(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C43), FW_NEAR_UNKNOWN_E6F3);
            return FW_NEAR_UNKNOWN_E6EB;
        }
        break;
    default:
        break;
    }
    return 0ul;
}

/* RAM:fc29c0 */
void PLEXTOR_NEAR_FUNCTION ApplyModePage1ASelectionState(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E6A2) & 0x02u) != 0u)
    {
        u32 countdown = FirmwareReadNearU32(FW_NEAR_UNKNOWN_E6A3);
        u32 scaled_countdown = MultiplyU32Low(countdown, 0x32ul);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB0, (u8)(scaled_countdown >> 24));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB1, (u8)(scaled_countdown >> 16));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB2, (u8)(scaled_countdown >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB3, (u8)scaled_countdown);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E6A2) & 0x01u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6A2, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E6A2) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6A7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6A8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6A9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6AA, 0u);
}

/* RAM:fc24d3 */
void PLEXTOR_NEAR_FUNCTION ApplyModeSelectPage03RuntimeFlags(void)
{
    u8 mode_page_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D);

    if(FirmwareReadBit0008() != 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 3) & 1u) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xbfu) | ((mode_page_value & 1u) << 6)));
            return;
        }
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >> 7) & 1u) != mode_page_value)
        {
            ResetRawReadQueueAndTransferState();
            InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
            YieldCoroutine0();
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x7fu) | ((mode_page_value & 1u) << 7)));
}

/* RAM:fb2a66 */
void HandlePreventAllowMediumRemovalCommand(void)
{
    u8 control;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D95)) == 0u)
    {
        /* The RAM:faff96 helper writes this fixed sense record. */
        SetSenseData(5u, 0x2400u);
        return;
    }

    control = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E);
    if((control & 1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C35, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C36, (u8)((control >> 1) & 1u));
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            u8 status_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);

            SetSenseData(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + status_code),
                ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + ((fw_addr_t)status_code << 1)));
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
        {
            SetSenseData(2u, 0x0401u);
            return;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C35, 1u);
        FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C36, (u8)((control >> 1) & 1u));
    }
    SetNoSenseData();
}

/* RAM:fde75a */
void HandleTestUnitReadyCommand(void)
{
    u8 status_code;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E13)) == 0u)
    {
        SetSenseData(5u, 0x2400u);
        return;
    }
    status_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    SetSenseData(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + status_code), ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + ((fw_addr_t)status_code << 1)));
}

/* Reconstruction helper; no direct Ghidra function address.
 * Exact RAM:ff964d table bytes consumed by RAM:fc238c and RAM:fc273a. */
static const u8 mode_select_page06_allowed_codes[16] = { 0x00u, 0x01u, 0x05u, 0x06u, 0x07u, 0x20u, 0x21u, 0x24u, 0x25u, 0x26u, 0x27u, 0x10u, 0x11u, 0x14u, 0x15u, 0x40u };

/* RAM:fc238c */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage01ParameterList(void)
{
    u8 six_byte_form;
    u8 index;
    u8 match = 0u;

    six_byte_form = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) == 6u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 10u && six_byte_form == 0u)
        return 0u;
    if(ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF89FE), FW_NEAR_UNKNOWN_0B2A) == 0u)
        return 0u;

    for(index = 0u; index < 16u; index++)
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C) & 0x7fu) == mode_select_page06_allowed_codes[index])
            match = 1u;
    if(match == 0u)
        return 0u;

    if(six_byte_form != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E62B, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E62C, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E62D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E62E, 0u);
    }
    return 1u;
}

/* RAM:fc273a */
u8 PLEXTOR_NEAR_FUNCTION ValidateModeSelectPage06AllowedCode(void)
{
    u8 index;
    u8 match = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) != 6u)
        return 0u;
    if(ValidateModeParameterBytesAgainstMaskRecord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B06), FW_NEAR_UNKNOWN_0B2A) == 0u)
        return 0u;

    for(index = 0u; index < 16u; index++)
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C) & 0x7fu) == mode_select_page06_allowed_codes[index])
            match = 1u;
    return match;
}

/* RAM:fc4ef7 */
void InitializeWorkingTrackWritePacketState(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD55, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u));
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD57, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B));
}

/* RAM:fd692b */
void InitializePremiumDiscFirmwareUpdateState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
}

/* RAM:fd6b91 */
void ClearPremiumDiscOperationStatusFlags(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0xfeu));
}

/* RAM:fd68e9 */
void RunPremiumDiscFirmwareUpdateCoroutineStep(void)
{
#define PLEXTOR_BINDING_SECTION 49
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    InitializePremiumDiscFirmwareUpdateState();

    if(VALIDATE_PREMIUM_DISC_SIGNATURE() != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF1, 1u);
        RUN_PREMIUM_UPDATE_RECOVERY();
    }
    else if(ACQUIRE_PREMIUM_FIRMWARE_PAYLOAD() != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF1, 2u);
        RUN_PREMIUM_UPDATE_RECOVERY();
    }
    else if(INVOKE_RUNTIME_MAIN_PROGRAMMER_FOR_PREMIUM_UPDATE() != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF1, 2u);
        RUN_PREMIUM_UPDATE_RECOVERY();
    }

    ClearPremiumDiscOperationStatusFlags();
    YieldCoroutine0();
}

/* RAM:fb01de */
void InitializeDriveControlSequence(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xf7u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x80u) == 0u)
    {
#define PLEXTOR_BINDING_SECTION 50
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    }

    CLEAR_CONTROLLER_WORD_0252_FOR_DRIVE_CONTROL_INITIALIZATION();
    ResetDriveControlStateToEight();
    PROBE_CONTROLLER_SIGNAL_AND_INITIALIZE_DRIVE_CONTROL();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 1u);
    ClearStateFlag11EFBit5();
    RouteStartupStatusAndScheduleCoroutine0();
    RunDriveControlState10Sequence();
}
/* RAM:fecb47 */
u8 BeginWriteCommandTransferPipeline(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 1u);
    return 1u;
}

/* RAM:fedb6d */
u8 IsWriteAndVerifyLogicalBlockBelow256(void)
{
    return (u8)((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B10) <= 255l);
}

/* RAM:f8f563 */
void ApplyRequestedAndDiscReadSpeedLimits(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC32) & 0x02u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F) >= 0x20u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 0x20u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC20) < FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC20));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC20));
    }
}

/* RAM:f9b622 */
u32 PLEXTOR_NEAR_FUNCTION BuildEmptyExternalRequestEventStatusDescriptor(void)
{
    return 0ul;
}

/* RAM:f85fa3 */
void TryAdvanceMediaCoroutineState0ToState3(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) != 0x1fu)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB0) != 0u)
        return;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x04u) != 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x04u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
}

/* RAM:f890d3 */
void PLEXTOR_NEAR_FUNCTION SetReadAcquisitionTargetFromCurrentQPosition(void)
{
    u8 track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1233);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235));
}

/* RAM:fde86a */
void HandleInquiryCommand(void)
{
    u8 transfer_length;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CA9)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC5, 5u);
    transfer_length = MinU8(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E), 0x60u);
    if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_DCC5, (u32)transfer_length) != 4u)
        SetNoSenseData();
}

/* RAM:fde424 */
void AdvanceReadAcquisitionTrackCursor(void)
{
    u8 track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u)
        if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
            track_number++;
        else
            track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    else if(track_number == FirmwareReadNear8(FW_NEAR_UNKNOWN_1148))
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
    else
        track_number++;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
}

/* RAM:fde476 */
void RetreatReadAcquisitionTrackCursor(void)
{
    u8 track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u)
        if(track_number > FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF) && track_number != 0xaau)
            track_number--;
        else
            track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    else if(track_number == FirmwareReadNear8(FW_NEAR_UNKNOWN_1147))
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1148);
    else
        track_number--;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
}

/* RAM:f9ff1c */
u8 ValidateAtipMediaClassAgainstCurrentMediaState(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7D) & 0x40u) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB98, 2u);
            return 0u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB98, 1u);
        return 0u;
    }
    return 1u;
}

/* RAM:fa75e3 */
void PLEXTOR_NEAR_FUNCTION SelectMediaReadTransitionOperationStatus(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 4u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) | 0x40u));
}

/* RAM:f8792b */
void PLEXTOR_NEAR_FUNCTION ApplyStateF6ZeroScaledControllerParameters(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) != 0xf6u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1202, 0x14u);
    WriteControllerParametersEAEBFromScaledValue(0u);
}

/* RAM:f9f587 */
void UpdateControllerProfileDerivedValue(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA04, 0x10u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51) >= 0x30u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA04, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51) - 0x20u));
}

/* RAM:fb9b05 */
void StartControllerParameterEaEbCalibrationCapture(void)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x13u);
}

/* RAM:fb9b1b */
void StopControllerParameterEaEbCalibrationCapture(void)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
}

/* RAM:fba099 */
void PLEXTOR_NEAR_FUNCTION MarkInitialTocUnavailable(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x40u));
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_12C5, 0ul);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_12C1, 0ul);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_12DC, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, 0u);
}

/* RAM:fc8ec1 */
void PLEXTOR_NEAR_FUNCTION SetCurrentOperatingCodeWithHighCodeTransitionFloor(u8 requested_operating_code)
{
    if(requested_operating_code >= 0x14u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= (s8)8)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 0x10u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, requested_operating_code);
}

/* RAM:f95cb4 */
void PLEXTOR_NEAR_FUNCTION SelectControllerMeasurementSeries00AverageOffset(void)
{
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) != 1u)
        return;
    index = 14u;
    while(FirmwareReadBigEndianU16Near((u16)(0xef89u + (u16)(index << 1))) < FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_EF85))
        index = (u8)(index - 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F086, FirmwareReadNear8((u16)(0xf077u + (u16)index)));
}

/* RAM:f86909 */
void PLEXTOR_NEAR_FUNCTION ConfigureControllerPorts8079And809DAnd809F(void)
{
    u8 port_8079_value = 0x14u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        port_8079_value = 0x94u;
    StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018079, port_8079_value);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, 0x40u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0xb0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF7));
}

/* RAM:f868f8 */
void ApplyConditionalReadPathPortConfiguration(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x40u) == 0u)
        ConfigureControllerPorts8079And809DAnd809F();
}

/* RAM:f8e031 */
void IssueControllerCommand0E(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0x02u) == 0u)
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 7u);
    else
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 1u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0x0eu);
}


/* RAM:fb7ffe */
/* WR6 descriptor result is zero-extended before the indexed displacement.
 * Thus +0x15 and +8 can cross RAM:00ffff; do not truncate those sums. */
void LoadControllerBufferLookaheadFromNextRawDescriptor(void)
{
    u16 descriptor = GetRawReadConsumerDescriptorAddressAtOffset(1u);
    u8 high, low;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBBA, FirmwareReadNearDisplaced8(descriptor, 0x15u));
    high = FirmwareReadNearDisplaced8(descriptor, 8u);
    low = FirmwareReadNearDisplaced8(descriptor, 9u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBBB, high);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBBC, low);
}

/* RAM:fcff09 */
void InitializeWriteControllerBufferTransferRegisters(void)
{
    ConfigureAtaDataTransferStatus();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0xc0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0x43u);
}

/* RAM:fdab08 */
/* The caller supplies the incompatible-medium gate; this leaf tests opcode. */
u8 PLEXTOR_NEAR_FUNCTION RejectIncompatibleMediumWriteCommand(void)
{
    u8 opcode = FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE);
    if(opcode == 0x0au || opcode == 0x2au || opcode == 0x2eu || opcode == 0x35u || opcode == 0x5bu || opcode == 0xaau)
    {
        SetSenseData(5u, 0x3000u);
        return 0u;
    }
    return 1u;
}

/* RAM:fc41c8 */
u8 ValidateWorkingTrackRawQState(void)
{
    DecodeWorkingTrackModeFromTdiRecord(FW_NEAR_UNKNOWN_DD38, FW_NEAR_UNKNOWN_DD37);
    return (u8)(ParseWorkingTrackTdiRecordMetadata() != 0u);
}

/* RAM:fd4ae2 */
/* The coroutine reset remains a native A251 boundary on target. */
void HandleStopPlayScanCommand(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D8A)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 4u) != 0u)
        ResetMediaProcessingCoroutines();
    SetNoSenseData();
}

/* RAM:f9b3fc */
void HandleModeSelect10Command(void)
{
    u16 parameter_list_length;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CB7)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    parameter_list_length = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B11);
    if(parameter_list_length == 0u)
    {
        SetNoSenseData();
        return;
    }

    ProcessModeSelectParameterList(0x0au, parameter_list_length);
}

/* RAM:fd449b */
void HandlePlayTrackRelative10Command(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B15, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B14, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B13, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B12));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B12, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B11));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B10, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B11, 0u);
    HandlePlayTrackRelative12Command();
}

/* RAM:fd2702 */
void HandleVerify10Command(void)
{
    u32 transfer_fields;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E1A)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B02, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0C));
    transfer_fields = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B10);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B06, transfer_fields >> 8);
    PrepareReadVerifyCommandState();
    ExecuteReadVerifyRange();
    FinalizeReadVerifyCommandState();
}

/* RAM:fd40fc */
void HandlePlayAudio10Command(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D42)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B15, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B13, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B12));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B12, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B11));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B14, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B11, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B10, 0u);
    HandlePlayAudio12Command();
}


/* RAM:fb87a0 */
u8 ResetPmaAcquisitionAndOptionallyReinitializeController(u8 reinitialize_controller)
{
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F) - 0x3f7ul);
    ResetAllPmaQRecordTables();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));

    if(reinitialize_controller != 0u)
    {
        ResetDriveControlStateToEightForState1Calibration();
        if(InitializeControllerOperatingStateAndRunCalibrationForState1() == 0u)
            return 0u;
    }
    return 1u;
}

/* RAM:fba9a2 */
u8 PLEXTOR_NEAR_FUNCTION ReacquireNextSessionBoundaryPosition(void)
{
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12D0));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E42) + 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED9, 1u);
    result = AcquireDiscReadPosition();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED9, 0u);
    return (u8)(result != 0u);
}

/* RAM:fa1a85 */
u8 InitializeAndAcquireRawQSequenceFromIndex1(void)
{
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 1u);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_8010, 10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    result = ExecuteRawReadQRecordSequence(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 0u);
    if(result == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 0u);
        return 0u;
    }
    return 1u;
}

/* RAM:fdd8f9 */
void ServiceControllerRequestFlag11E5Bit1(void)
{
    u8 request;

    request = FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5);
    if((request & 2u) == 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xfdu));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11D7) & 0x40u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D7) | 0x40u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D7) & 0xbfu));
}

/* RAM:fe2b5d */
u8 PLEXTOR_NEAR_FUNCTION FailIfReadMeasurementPreparationActive(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        return 0u;

    ReportReadPipelineFailure(1u);
    return 1u;
}

/* RAM:f863f4 */
u8 PLEXTOR_NEAR_FUNCTION ProcessDriveStateFlags(void)
{
    u8 capture_options = FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B);
    u8 collected_kinds = FirmwareReadNear8(FW_NEAR_UNKNOWN_1259);

    if((capture_options & 1u) == 0u || (((capture_options & 2u) == 0u || (collected_kinds & 2u) == 0u) && (((capture_options & 0x0cu) == 0u || (collected_kinds & 4u) == 0u))))
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xf2u) | 2u));
#define PLEXTOR_BINDING_SECTION 51
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    return 2u;
}

/* RAM:f86fd5 */
/* Initializes the media-state-five flags and threshold before its controller
 * path begins.  BITS:0031 and the indexed state-four predicate independently
 * request a position snapshot (RAM:00fb69 bit zero). */
void PLEXTOR_NEAR_FUNCTION InitializeMediaState5ControlState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA00, 0xffu);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_F9E2, 10000u);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5, (u16)(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FF5) & 0xffbfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xddu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F978, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) & 0xf8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F970, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_102D) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD5, 0u);

    if(FirmwareReadBit0031() != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) | 1u));
    if(IsIndexedDriveStateActive(4u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) | 1u));
}

/* RAM:f93d45 */
/* Updates staged ADR-1 Q absolute and relative packed-BCD MSF positions by
 * one frame.  RAM:0015a6 selects decrement or increment for the relative
 * position; an underflow frame is deliberately restored as BCD 74. */
void PLEXTOR_NEAR_FUNCTION AdvanceRawSectorQPositionByOneFrame(void)
{
    u8 frame;
    u8 second;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_15A4) & 0x0fu) != 1u)
        return;

    frame = (u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15AD)) + 1u);
    if(frame > 74u)
    {
        frame = 0u;
        second = (u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15AC)) + 1u);
        if(second > 59u)
        {
            second = 0u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15AB, BinaryByteToPackedBcd((u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15AB)) + 1u)));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15AC, BinaryByteToPackedBcd(second));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15AD, BinaryByteToPackedBcd(frame));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A6) == 0u)
    {
        frame = (u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A9)) - 1u);
        if(frame > 74u)
        {
            frame = 74u;
            second = (u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A8)) - 1u);
            if(second > 59u)
            {
                second = 59u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A7, BinaryByteToPackedBcd((u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A7)) - 1u)));
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A8, BinaryByteToPackedBcd(second));
        }
    }
    else
    {
        frame = (u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A9)) + 1u);
        if(frame > 74u)
        {
            frame = 0u;
            second = (u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A8)) + 1u);
            if(second > 59u)
            {
                second = 0u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A7, BinaryByteToPackedBcd((u8)(PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A7)) + 1u)));
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A8, BinaryByteToPackedBcd(second));
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A9, BinaryByteToPackedBcd(frame));
}

/* RAM:f94427 */
/* Selects the raw-read operating speed from the acquisition phase and current
 * operating code, then applies the configured ceiling unless its override is
 * active. */
void PLEXTOR_NEAR_FUNCTION UpdateRawReadSpeedForAcquisitionPhase(void)
{
    u8 phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9C);
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 selected_speed;

    if(phase < 7u)
    {
        if(operating_code == 8u)
            selected_speed = 0x18u;
        else if(operating_code == 0x10u)
            selected_speed = 0x20u;
        else if(operating_code == 4u)
            selected_speed = 8u;
        else
            selected_speed = 0x28u;
    }
    else if(phase < 9u)
    {
        if(operating_code == 0x20u)
            selected_speed = 8u;
        else if(operating_code == 0x28u || operating_code == 0x34u)
            selected_speed = 0x20u;
        else
            selected_speed = 8u;
    }
    else if(phase < 11u)
    {
        selected_speed = operating_code == 8u ? 4u : 8u;
    }
    else if(phase < 13u)
    {
        selected_speed = operating_code == 8u ? 0x18u : 8u;
    }
    else
    {
        selected_speed = operating_code == 0x18u ? 0x20u : 0x18u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A) < selected_speed && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
        selected_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, selected_speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, selected_speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, selected_speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, selected_speed);
}

/* RAM:f94363 */
/* Selects the first saved probe strictly inside the ten-frame window before
 * the raw-read target.  Its final direct BITS:0026 clear is retained through
 * a narrow host/target bit adapter. */
void PLEXTOR_NEAR_FUNCTION SelectRecentReadAcquisitionProbeTarget(void)
{
    s32 target = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_1598);
    s32 candidate;
    u8 selected_code = 0u;

    candidate = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_0F83);
    if(target - 10l < candidate && candidate < target)
        selected_code = 5u;
    else
    {
        candidate = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_0F87);
        if(target - 10l < candidate && candidate < target)
            selected_code = 4u;
        else
        {
            candidate = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_0F8B);
            if(target - 10l < candidate && candidate < target)
                selected_code = 3u;
            else
            {
                candidate = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_0F8F);
                if(target - 10l < candidate && candidate < target)
                    selected_code = 2u;
            }
        }
    }
    if(selected_code != 0u)
    {
        FirmwareWriteBigEndianU32(FW_NEAR_UNKNOWN_0F97, (u32)candidate);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9B, selected_code);
    }

    if((s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_0F10) + target <= (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_0F97))
    {
#define PLEXTOR_BINDING_SECTION 52
#include "bindings/platform/status_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    }
}

/* RAM:f9b45c */
/* Builds the four-byte operational-change GET EVENT STATUS descriptor in
 * big-endian numeric form, consuming the corresponding pending flags. */
u32 PLEXTOR_NEAR_FUNCTION BuildOperationalChangeEventStatusDescriptor(void)
{
    u8 pending = FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS);
    u8 event_class = 0u;
    u8 event_status = 0u;

    if((pending & 0x18u) != 0u)
    {
        event_class = 2u;
        event_status = 1u;
        pending = (u8)(pending & 0xe7u);
    }
    if((pending & 0x20u) != 0u)
    {
        event_class = 2u;
        event_status = 3u;
        pending = (u8)(pending & 0xdfu);
    }
    FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, pending);
    return ((u32)event_class << 24) | ((u32)event_status << 8);
}

/* RAM:f9b4e7 */
/* The native stack event byte is not initialized when pending bit one is set
 * and RAM:000fae is outside 0..2, so the C adapter exposes that input. */
u32 PLEXTOR_NEAR_FUNCTION BuildGetEventStatusMediaEventDescriptor(u8 inherited_event)
{
    u8 pending = (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) & 0xfdu);
    u8 event = inherited_event;
    u8 media_status;

    if((pending & 1u) != 0u)
        pending = (u8)((pending & 0xfeu) | 2u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1584) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1584, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1584) & 0xfeu));
        pending = (u8)(pending | 2u);
    }
    FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, pending);

    if((pending & 2u) != 0u)
    {
        u8 drive_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);

        if(drive_state == 0u)
            event = 2u;
        else if(drive_state == 1u || drive_state == 2u)
            event = 3u;
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1584, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1584) | 1u));
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1586) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1586, 0u);
        event = 6u;
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x40u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1587) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1587, 0u);
        event = 5u;
    }
    else
        event = 0u;

    if((pending & 4u) != 0u)
    {
        event = 1u;
        FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) & 0xfbu));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x40u) == 0u)
            FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 0u);
    }

    media_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u ? 2u : FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 1u ? 1u : 0u;
    return ((u32)event << 24) | ((u32)media_status << 16);
}

/* RAM:f9b644 */
u32 BuildMediaEventStatusDescriptor(void)
{
    u8 media_status;
    u8 changed;
    u8 drive_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E751) != 0u)
        media_status = 4u;
    else if(drive_state == 2u)
        media_status = 3u;
    else if(drive_state == 3u || drive_state == 4u)
        media_status = 1u;
    else if(drive_state == 0x0fu || drive_state == 0x10u)
        media_status = 3u;
    else if(drive_state == 1u)
        media_status = 3u;
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 8u)
        media_status = 2u;
    else
        media_status = 1u;

    changed = (u8)((media_status != FirmwareReadNear8(FW_NEAR_UNKNOWN_1585) || FirmwareReadNear8(FW_NEAR_UNKNOWN_1585) == 5u) ? 1u : 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1585, media_status);
    return ((u32)changed << 24) | ((u32)media_status << 16);
}

/* RAM:f9b6e0 */
/* The active path's time conversion remains a narrow assembly boundary so the
 * firmware's unsigned-divider and divide-by-zero behavior stays intact. */
u32 PLEXTOR_NEAR_FUNCTION BuildDeviceBusyEventStatusDescriptor(void)
{
    u8 event_class;
    u8 event_status;
    u16 time_value;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) != 0u)
    {
        time_value = ComputeDeviceBusyEventTimeValueNative();
        event_class = 1u;
        event_status = 2u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
    {
        time_value = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1004);
        if(time_value == 0u)
        {
            time_value = 0x0014u;
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_1004, time_value);
        }
        event_class = 1u;
        event_status = 3u;
    }
    else
    {
        time_value = 0u;
        event_class = 0u;
        event_status = 0u;
    }

    return ((u32)event_class << 24) | ((u32)event_status << 16) | (u32)time_value;
}

/* RAM:fb1d22 */
/* The value at RAM:001215 is asynchronously decremented by the calibration coroutine/interrupt
 * owner; the two signed countdown waits are therefore deliberately not replaced by
 * fixed iteration counts. */
u8 MeasureSfrD8RangeAtLeast16(void)
{
    u8 sample;
    u8 maximum;
    u8 minimum;

    WriteControllerParametersD4AndE2ForCode(0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 10u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) > 0)
        YieldCoroutine1();

    FirmwareWriteDirectSfr8(E1, 0u);
    FirmwareWriteDirectSfr8(E1, 0x80u);
    while((FirmwareReadDirectSfr8(F1) & 1u) == 0u)
    {
    }
    sample = FirmwareReadDirectSfr8(D8);
    maximum = sample;
    minimum = sample;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 0x23u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) > 0)
    {
        while((FirmwareReadDirectSfr8(F1) & 1u) == 0u)
        {
        }
        sample = FirmwareReadDirectSfr8(D8);
        if(maximum < sample)
            maximum = sample;
        if(sample < minimum)
            minimum = sample;
    }

    FirmwareWriteDirectSfr8(E1, 0u);
    WriteControllerParametersD412AndE2Value58();
    return (u8)(maximum - minimum) >= 0x10u ? 1u : 0u;
}

/* RAM:fdc918 */
void ValidateStartupP0InputCombinationStable(u8 begin_enabled)
{
    u8 sampled_once = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, begin_enabled != 0u ? 1u : 0u);
    do
    {
        u8 p0 = FirmwareReadDirectSfr8(80);

        if((p0 & 0x40u) != 0u && (((p0 & 0x02u) == 0u && (p0 & 0x04u) != 0u) || ((p0 & 0x02u) != 0u && (p0 & 0x04u) == 0u)))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 0u);

        if(sampled_once != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u)
            break;
        for(p0 = 0u; p0 < 12u; ++p0)
            DelayTimerUnits(0x09a6u);
        sampled_once = 1u;
    } while(1);
}

/* RAM:fdd8b2 */
/* Mode transitions retain their independent native ERET/register boundaries. */
void ServiceControllerRequestFlag11E5Bit0(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 1u) == 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xfeu));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11D7) & 0x10u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D7) | 0x10u));
        ApplyControllerRequestModeOne();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D7) & 0xefu));
        ApplyControllerRequestModeTwo();
    }
}

/* RAM:fdd932 */
void ServiceControllerRequestFlag11E5Bit5(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0x20u) == 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xdfu));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) & 8u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) & 0xf7u));
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) | 8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_153E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_153E) + 1u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_153E) == 6u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_153E, 1u);
}

/* RAM:fdd983 */
void ConfigureControllerRequestModeOneRegisters(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9A, 1u);
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1525, 0x0cul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1531, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFD, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x30u);
    FirmwareWriteBit0043(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x10u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x13u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD));
    WriteControllerParameterWord((u16)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15) | 0xe720u));
}

/* RAM:fdd8ef */
void ApplyControllerRequestModeOne(void)
{
    ConfigureControllerRequestModeOneRegisters();
}

/* RAM:fdd9f2 */
void ConfigureControllerRequestModeTwoRegisters(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0028) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1531, 0u);
    WriteControllerParameterWord((u16)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15) | 0xe700u));
}

/* RAM:fdd8f4 */
void ApplyControllerRequestModeTwo(void)
{
    ConfigureControllerRequestModeTwoRegisters();
}

/* RAM:fb99f5 */
/* Chooses the nearest signed word in the five-entry controller-position
 * calibration series.  Equal distances choose the sample at or below the
 * target, matching the native WR comparison sequence. */
void SelectNearestControllerPositionSampleIndex(void)
{
    s16 target = (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FB5F);
    s16 best_sample = (s16)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0FDB);
    u8 best_index = 0u;
    u8 index;

    for(index = 1u; index < 5u; ++index)
    {
        s16 candidate = (s16)FirmwareReadBigEndianU16Near((u16)(0x0fdbu + (u16)(index << 1)));
        u16 best_distance = best_sample < target ? (u16)((s32)target - (s32)best_sample) : (u16)((s32)best_sample - (s32)target);
        u16 candidate_distance = candidate < target ? (u16)((s32)target - (s32)candidate) : (u16)((s32)candidate - (s32)target);

        if(candidate_distance < best_distance || (candidate_distance == best_distance && candidate <= target))
        {
            best_sample = candidate;
            best_index = index;
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF0, best_index);
}

/* RAM:fbc6f7 */
u8 PLEXTOR_NEAR_FUNCTION ValidateWriteTypeAgainstWorkingTrackState(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
        {
            SetCommandSequenceErrorSense();
            return 0u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u)
        {
            SetSenseData(5u, 0x7204u);
            return 0u;
        }
    }
    else if(write_type == 2u)
    {
        if(IsTrackTableTailEntryUninitialized() == 0u)
        {
            SetSenseData(5u, 0x2c03u);
            return 0u;
        }
    }
    else if(write_type == 3u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD63) == 1u)
        {
            SetSenseData(5u, 0x2c03u);
            return 0u;
        }
    }
    else if(write_type == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u)
    {
        SetSenseData(5u, 0x7204u);
        return 0u;
    }
    return 1u;
}

/* RAM:fbc191 */
u8 ValidateCurrentWriteParameterCombination(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    u8 data_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    u8 accepted = 1u;

    if(write_type == 2u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 4u) == 0u)
        {
            SetCommandSequenceErrorSense();
            return 0u;
        }
        return 1u;
    }
    if(write_type == 3u && data_type != 1u && data_type != 2u && data_type != 3u)
        accepted = 0u;
    if(write_type == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0xfdu) != 5u)
        accepted = 0u;
    if(write_type == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0xfdu) == 5u)
        accepted = 0u;
    if(data_type == 0u || data_type == 1u || data_type == 2u || data_type == 3u)
    {
        if(write_type == 0u)
            accepted = 0u;
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) == 0u)
    {
        accepted = 0u;
    }
    if((data_type == 0x0cu && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C73) & 0x20u) == 0u) || (data_type == 0x0au && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C73) & 0x20u) != 0u))
        accepted = 0u;
    if(accepted == 0u)
        SetCommandSequenceErrorSense();
    return accepted;
}

/* RAM:fbc247 */
u8 ValidateRecordableMediaCommandEligibility(void)
{
    u8 media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    u8 controller_code;

    if(media_state == 0u)
    {
        SetSenseData(2u, 0x3005u);
        return 0u;
    }
    if(media_state == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
    {
        SetSenseData(5u, 0x2700u);
        return 0u;
    }
    if(media_state == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) != 3u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 1u))
    {
        SetSenseData(2u, 0x3005u);
        return 0u;
    }
    if(ValidateMediaState2ControllerCodeForWrite() == 0u || ValidateMediaState2ControllerFlagsForWrite() == 0u)
        return 0u;

    controller_code = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) & 0x3fu);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) & 0x40u) == 0u)
    {
        if(controller_code != 0u && controller_code != FirmwareReadNear8(FW_NEAR_UNKNOWN_0C49))
        {
            SetSenseData(5u, 0x3008u);
            return 0u;
        }
    }
    else if(controller_code != 0u)
    {
        SetSenseData(5u, 0x3008u);
        return 0u;
    }
    return 1u;
}

/* RAM:fbc768 */
u8 ValidateWriteOperationTrackState(void)
{
    u8 accepted = 1u;
    u8 data_type;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) != 0u && FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B) == 0ul)
        {
            SetSenseData(5u, 0x6401u);
            return 0u;
        }
        return 1u;
    }

    data_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    if(data_type >= 9u && data_type <= 13u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) != 2u)
            accepted = 0u;
    }
    else if(data_type == 8u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) != 1u)
    {
        accepted = 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) == 0u || FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57) != FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B))
            accepted = 0u;
    }
    if(accepted == 0u)
        SetCommandSequenceErrorSense();
    return accepted;
}

/* RAM:fbc3dc */
u8 ValidateCandidateWriteStartFrameForCurrentParameters(s32 candidate_start_frame)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    u8 track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    u8 data_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    s32 working_track_start = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D);
    s32 working_track_reference = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39);

    if(write_type == 1u)
    {
        if(track_type == 1u || track_type == 3u)
            return 0u;
        if(data_type >= 8u && data_type <= 13u)
            return working_track_start == candidate_start_frame ? 1u : 0u;
        if(data_type != 0u && data_type != 1u && data_type != 2u && data_type != 3u)
            return 0u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
            return working_track_reference == candidate_start_frame ? 1u : 0u;
        return working_track_start == candidate_start_frame ? 1u : 0u;
    }
    if(write_type == 2u)
        return 0u;
    if(write_type == 3u)
        return (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD69) == candidate_start_frame ? 1u : 0u;
    if(write_type != 0u)
        return 0u;
    if(track_type == 0u)
        return working_track_start == candidate_start_frame ? 1u : 0u;
    if(IsFixedPacketAddressExpansionEnabled() != 0u)
        return ComputeWorkingTrackPacketProgressFramePosition(1u) == candidate_start_frame ? 1u : 0u;
    return (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) + 5ul) == candidate_start_frame ? 1u : 0u;
}

/* RAM:fbc36d */
u8 FindWritableTrackForCandidateFramePosition(s32 candidate_frame_position)
{
    s32 candidate_start_frame = (s32)((u32)candidate_frame_position + 0x96ul);
    u8 track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E);

    while(track_number <= FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F))
    {
        u16 track_offset = (u16)((u16)track_number * 19u);
        u8 track_type = FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD87, track_offset));

        if(track_type != 4u && track_type != 5u && track_type != 6u && track_type != 7u)
        {
            if(LoadTrackInformationByNumberFromInheritedB(track_number, (u8)(track_offset >> 8)) == 0u)
                return 0u;
            if(ValidateCandidateWriteStartFrameForCurrentParameters(candidate_start_frame) != 0u)
                return 1u;
        }
        track_number++;
    }
    HandleUnsupportedScsiCommand();
    return 0u;
}

/* RAM:fb9a54 */
u8 AreInteriorControllerPositionSamplesAboveReferenceBy30(s16 reference_value)
{
    u8 index;

    for(index = 1u; index < 4u; ++index)
    {
        s16 sample;

        reference_value = (s16)((u16)reference_value + 30u);
        sample = (s16)FirmwareReadBigEndianU16Near((u16)(0x0fdbu + (u16)(index << 1)));
        if(sample >= reference_value)
            return 0u;
    }
    return 1u;
}

/* RAM:f97dad */
/* WR6 u16 return; RET. */
u16 PLEXTOR_NEAR_FUNCTION SelectMode6MeasurementThresholdFromProfile(void)
{
    u8 profile_code = FirmwareRead8(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_F1BB) + 8ul);

    if(profile_code == 0u)
        return 500u;
    if(profile_code >= 1u && profile_code <= 10u)
        return (u16)(500u + (u16)profile_code * 100u);
    if(profile_code == 11u)
        return 950u;
    return 900u;
}

/* RAM:fbce3d */
u8 PLEXTOR_NEAR_FUNCTION ValidateWriteFramePositionAgainstTrackTable(s32 frame_position)
{
    s32 target_frame_position = (s32)((u32)frame_position + 0x96ul);
    u8 effective_last_track = GetEffectiveLastTrackNumber();
    u8 track_number = 1u;

    while(track_number <= (u8)(effective_last_track + 1u))
    {
        u16 track_offset = (u16)((u16)track_number * 19u);
        u8 track_type = FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD87, track_offset));

        if(track_type != 5u && track_type != 6u && track_type != 7u
            && (track_type != 4u || target_frame_position <= ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD84, track_offset))))
        {
            if(LoadTrackInformationByNumberFromInheritedB(track_number, (u8)(track_offset >> 8)) == 0u)
                return 0u;
            if(ValidateWriteFramePositionAgainstWorkingTrackState(target_frame_position) != 0u)
                return 1u;
        }
        track_number++;
    }
    HandleUnsupportedScsiCommand();
    return 0u;
}

/* Reconstruction helper: native DR arithmetic wraps before its signed compare. */
static u8 IsWriteTransferExtentWithinBoundary(s32 start_frame, u32 block_count, s32 end_boundary)
{
    return (s32)((u32)start_frame + block_count) <= (s32)((u32)end_boundary - 2ul) ? 1u : 0u;
}

/* RAM:fbc80f */
u8 PLEXTOR_NEAR_FUNCTION ValidateWriteTransferExtentAgainstTrackBoundary(u8 use_alternate_write_cursor, u32 transfer_block_count)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD61) == 2u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) < 4u)
            {
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) == 0u && use_alternate_write_cursor != 0u)
                    return IsWriteTransferExtentWithinBoundary((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D), transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41));
            }
            else if(use_alternate_write_cursor != 0u)
            {
                return IsWriteTransferExtentWithinBoundary((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D), transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41));
            }
            return IsWriteTransferExtentWithinBoundary((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D), transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41));
        }
        return IsWriteTransferExtentWithinBoundary((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D), transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION));
    }
    if(write_type != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
    {
        u8 track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);

        if(track_type == 3u)
            return IsWriteTransferExtentWithinBoundary(use_alternate_write_cursor != 0u ? (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) + 5ul)
                                                                                        : (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D),
                transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41));
        if(track_type == 1u)
            return IsWriteTransferExtentWithinBoundary(use_alternate_write_cursor != 0u ? (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) + 5ul)
                                                                                        : (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D),
                transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION));
        return IsWriteTransferExtentWithinBoundary(use_alternate_write_cursor != 0u ? (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) : (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D),
            transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION));
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u)
        return IsWriteTransferExtentWithinBoundary(use_alternate_write_cursor != 0u ? (s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) + 5ul)
                                                                                    : (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D),
            transfer_block_count, (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41));
    return 1u;
}

/* RAM:fbced3 */
u8 ValidateWriteFramePositionAgainstWorkingTrackState(s32 target_frame_position)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    u8 track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    u8 data_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    s32 working_track_start = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D);
    s32 working_track_reference = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39);

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD49, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45));
    if(write_type == 1u)
    {
        if(track_type == 1u || track_type == 3u)
            return 0u;
        if(data_type >= 8u && data_type <= 13u)
            return working_track_start == target_frame_position ? 1u : 0u;
        if(data_type != 0u && data_type != 1u && data_type != 2u && data_type != 3u)
            return 0u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
            return working_track_reference == target_frame_position ? 1u : 0u;
        return working_track_start == target_frame_position ? 1u : 0u;
    }
    if(write_type == 2u)
        return 0u;
    if(write_type == 3u)
        return (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD69) == target_frame_position ? 1u : 0u;
    if(write_type != 0u)
        return 0u;
    if(track_type == 0u)
        return working_track_start == target_frame_position ? 1u : 0u;
    if(IsFixedPacketAddressExpansionEnabled() != 0u)
    {
        if(ComputeWorkingTrackPacketProgressFramePosition(1u) == target_frame_position)
            return 1u;
    }
    else if((s32)(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD49) + 5ul) == target_frame_position)
    {
        return 1u;
    }
    if(track_type != 4u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
    {
        if(FUN_RAM_fbd046((u32)target_frame_position) == 0u)
            return 0u;
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD49, MapFixedPacketLogicalToPhysicalAddress((u32)target_frame_position - 0x96ul) + 0x91ul);
        return 1u;
    }
    return (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) == target_frame_position ? 1u : 0u;
}

/* Reconstruction helper: RAM:fbd328/RAM:fbd39d share this arithmetic. */
static void RecalculateWorkingTrackWriteCursorFromPacketProgress(u16 base_address)
{
    u32 packet_size = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    u32 progress = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B);
    u32 quotient = DivideU32QuotientWithoutRuntimeLibrary(progress, packet_size);
    u32 remainder = progress - MultiplyU32Low(packet_size, quotient);
    u32 cursor = FirmwareReadBigEndianU32Near(base_address) + MultiplyU32Low(quotient, packet_size + 7ul);

    if(remainder != 0u)
        cursor += remainder + 5ul;
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, cursor);
}

/* RAM:fbd328 */
void UpdateWorkingTrackWriteCursorForDefaultMediaState(void)
{
    RecalculateWorkingTrackWriteCursorFromPacketProgress(FW_NEAR_UNKNOWN_DD45);
}

/* RAM:fbd39d */
void UpdateWorkingTrackWriteCursorForMediaState2(void)
{
    RecalculateWorkingTrackWriteCursorFromPacketProgress(FW_NEAR_UNKNOWN_DD49);
}

/* RAM:fbd262 */
void PLEXTOR_NEAR_FUNCTION AdvanceWorkingTrackWriteCursorAfterTransfer(s32 start_lba, u32 transfer_block_count, u8 force_unit_access)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
    {
        if(force_unit_access == 0u)
        {
            if(start_lba == 0 || (s32)((u32)start_lba + 0x96ul) == (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D))
            {
                FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D) + transfer_block_count);
                FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B) + transfer_block_count);
            }
            else
            {
                FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D) + 7ul + transfer_block_count);
                FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B, transfer_block_count);
            }
        }
        else
        {
            u32 base = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u ? FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) : FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D) + 5ul;

            FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, base + transfer_block_count);
            FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B, transfer_block_count);
        }
    }
    else
    {
        u32 progress = force_unit_access != 0u ? transfer_block_count : FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B) + transfer_block_count;

        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B, progress);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            UpdateWorkingTrackWriteCursorForMediaState2();
        else
            UpdateWorkingTrackWriteCursorForDefaultMediaState();
    }
}

/* RAM:f87992 */
/* native DR4 input/output. */
s32 PLEXTOR_NEAR_FUNCTION UpdateMediaState5PositionTracking(s32 previous_frame_position)
{
    s32 current_position = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_1235);
    s32 difference;
    s32 track_end_position = 0x0002e248l;
    u16 track_offset;

    if(previous_frame_position == 0)
        previous_frame_position = current_position;
    if((previous_frame_position < 0 && current_position >= 0) || (previous_frame_position > 0 && current_position <= 0))
        difference = 0;
    else
        difference = previous_frame_position - current_position;

    if((difference >= 0x02ee || difference <= -0x02ee) && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) & 1u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F978, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) | 2u));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) != 0u)
        {
            track_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
            track_offset = (u16)((track_offset << 3) + (track_offset << 1));
            track_end_position = ConvertBinaryMsfToAdjustedFramePosition(FW_HUGE_UNKNOWN_2BB03E + (fw_addr_t)track_offset);
        }
        if(current_position > track_end_position + 0x2328l + 0x14l)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F978, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) | 2u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 8u);
        }
    }
    return current_position;
}

/* RAM:fccd1a */
s32 ExpandWriteLogicalAddressForRecordOverhead(s32 logical_address)
{
    s32 per_block_quotient = DivideS32ByU16TruncatingTowardZero(logical_address, 32u);
    s32 per_group_quotient = DivideS32ByU16TruncatingTowardZero(logical_address, 4352u);

    return (s32)((u32)logical_address + MultiplyU32Low((u32)per_block_quotient, 7u) + MultiplyU32Low((u32)per_group_quotient, 312u) + 1560u);
}

/* RAM:fedce8 */
u8 IsWritePipelineAtAlignedCursorOrDifferentRecord(void)
{
    s32 target_address;
    s32 write_cursor;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x10u) != 0u)
        return 0u;
    target_address = (s32)FirmwareReadBigEndianU32(FW_NEAR_UNKNOWN_8000);
    write_cursor = (s32)FirmwareReadBigEndianU32(FW_HUGE_UNKNOWN_2BC703);
    if(DivideS32ByU16TruncatingTowardZero(target_address - 145, 39u) != DivideS32ByU16TruncatingTowardZero(ExpandWriteLogicalAddressForRecordOverhead(write_cursor) + 5, 39u))
        return 1u;
    return (u8)(target_address == ExpandWriteLogicalAddressForRecordOverhead(DivideS32ByU16TruncatingTowardZero(write_cursor, 32u) << 5) + 150);
}
