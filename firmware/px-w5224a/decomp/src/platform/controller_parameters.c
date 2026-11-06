#include "controller_parameters.h"
#include "../boot/startup_progress_diagnostics.h"
#include "arithmetic.h"
#include "controller_mapping.h"
#include "controller_operating_interpolation.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern void PLEXTOR_NEAR_FUNCTION SetBit0719Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0719Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit070FAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit070FAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit054AAtomic(void);
extern void ServiceControllerCommand0EMeasurementCapture(void);
extern void CompleteControllerMeasurementSequence(void);
extern void YieldCoroutine1(void);
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearWithoutTimeout(void);
extern u8 PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7Clear500AndYield(void);

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:faa88f */
void ReadControllerPort30Bytes(u16 byte_count, fw_addr_t destination_address)
{
    u16 index = 0u;

    while(index < byte_count)
    {
        while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x80u) == 0u)
        {
        }
        FirmwareWrite8(destination_address, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018030));
        destination_address = FirmwareAddressIncrementLow16(destination_address, 1u);
        index = (u16)(index + 1u);
    }
}

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadBigEndianU16ControllerRecord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ReadBigEndianS32ControllerRecord(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianS32ControllerRecord(fw_addr_t address, s32 value)
{
    u32 bits = (u32)value;
    FirmwareWrite8(address, (u8)(bits >> 24));
    FirmwareWrite8(address + 1u, (u8)(bits >> 16));
    FirmwareWrite8(address + 2u, (u8)(bits >> 8));
    FirmwareWrite8(address + 3u, (u8)bits);
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 DivideS32By39TruncatingTowardZero(s32 dividend)
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
        if(remainder >= 39ul)
        {
            remainder -= 39ul;
            quotient |= 1u;
        }
    }
    return negative != 0u ? (s32)(0u - quotient) : (s32)quotient;
}

/* RAM:fc8141 */
/* Source DR4 input/result, RET; DR28 holds the wrapping biased input. */
s32 PLEXTOR_NEAR_FUNCTION CompressThirtyNineFramePositionToLogicalBlockAddress(s32 frame_position)
{
    u32 biased = (u32)frame_position - 150ul;
    u32 quotient = (u32)DivideS32By39TruncatingTowardZero((s32)biased);
    u32 seven_quotients = (quotient << 3) - quotient;
    return (s32)(biased - seven_quotients);
}

/* RAM:fc8166 */
/* Restore the sign after shifting magnitude so division truncates toward
 * zero, including negative inputs that are not multiples of 32. */
s32 PLEXTOR_NEAR_FUNCTION ExpandLogicalBlockAddressToThirtyNineFramePosition(s32 logical_address)
{
    u32 magnitude = (u32)logical_address;
    u32 quotient;
    if(logical_address < 0)
        magnitude = 0ul - magnitude;
    quotient = magnitude >> 5;
    if(logical_address < 0)
        quotient = 0ul - quotient;
    return (s32)((u32)logical_address + (quotient << 3) - quotient + 150ul);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void PublishReadMeasurementRangeQueueState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5D8, 2u);
}

/* RAM:fed112 */
void PrepareReadMeasurementControllerRecordRangeAndQueue(void)
{
    if(FirmwareReadBit0028() != 0u)
    {
        s32 failure_position;
        s32 first_position;
        u16 last_index;
        u16 first_index;
        u16 span;
        u16 consumer;
        u16 queued;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
        LatchFirstReadFailureContext();
        failure_position = ReadBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_1484);
        if(failure_position >= 145l)
        {
            last_index = (u16)DivideS32By39TruncatingTowardZero((s32)((u32)failure_position - 145ul));
            first_position = (s32)((u32)ReadBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_FBA6) + 3ul);
            if(first_position < 145l)
                first_position = 145l;
            first_index = (u16)DivideS32By39TruncatingTowardZero((s32)((u32)first_position - 145ul));
            MarkControllerRecordIndexRangeAndEndpoint(first_index, last_index, 1u);
            if(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5D8) != 0u)
            {
                u32 rounded_position = MultiplyU32Low((u32)last_index, 39ul) + 145ul;

                span = (u16)(39u - (u16)((u32)failure_position - rounded_position));
                consumer = (u16)((ReadBigEndianU16ControllerRecord(FW_NEAR_UNKNOWN_1294) + span) & ReadBigEndianU16ControllerRecord(FW_NEAR_UNKNOWN_129A));
                queued = (u16)(ReadBigEndianU16ControllerRecord(FW_NEAR_UNKNOWN_1292) - span);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1294, (u8)(consumer >> 8));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1295, (u8)consumer);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1292, (u8)(queued >> 8));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1293, (u8)queued);
                PublishReadMeasurementRangeQueueState();
                return;
            }
        }
    }
    else
    {
        s32 descriptor_position = ReadBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_FBA2);

        if(descriptor_position >= 145l)
        {
            u16 index = (u16)DivideS32By39TruncatingTowardZero((s32)((u32)descriptor_position - 145ul));

            MarkControllerRecordIndexRangeAndEndpoint(index, index, 1u);
            if(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5D8) != 0u)
            {
                u16 consumer = (u16)((ReadBigEndianU16ControllerRecord(FW_NEAR_UNKNOWN_1294) + 39u) & ReadBigEndianU16ControllerRecord(FW_NEAR_UNKNOWN_129A));
                u16 queued = (u16)(ReadBigEndianU16ControllerRecord(FW_NEAR_UNKNOWN_1292) - 39u);

                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1294, (u8)(consumer >> 8));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1295, (u8)consumer);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1292, (u8)(queued >> 8));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1293, (u8)queued);
                PublishReadMeasurementRangeQueueState();
                return;
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 1u));
}

/* RAM:fed262 */
void MarkControllerRecordIndexRangeAndEndpoint(u16 first_index, u16 last_index, u8 mark_endpoint)
{
    fw_addr_t table_address = ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAFF) << 24) | ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB00) << 16)
                            | ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB01) << 8) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB02);
    u8 mark = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5D8) == 0u ? 2u : 1u;

    while((s16)first_index < (s16)last_index)
    {
        FirmwareWrite8(FirmwareAddressIncrementLow16(table_address, first_index), mark);
        first_index = (u16)(first_index + 1u);
    }
    FirmwareWrite8(FirmwareAddressIncrementLow16(table_address, last_index), mark_endpoint != 0u ? (u8)(mark + 2u) : mark);

    if(ReadBigEndianU16ControllerRecord(FW_HUGE_UNKNOWN_2BA5DB) < last_index)
        FirmwareWriteU16(FW_HUGE_UNKNOWN_2BA5DB, last_index);
}

/* RAM:fed2c3 */
void MarkReadAcquisitionControllerRecordRange(void)
{
    s32 last_input = ReadBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_14DE);

    last_input = (s32)((u32)last_input - 1ul);
    if(last_input >= 145l)
    {
        s32 first_input = (s32)((u32)ReadBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_FBA6) + 3ul);
        u16 first_index;
        u16 last_index;

        if(first_input < 145l)
            first_input = 145l;
        first_index = (u16)DivideS32By39TruncatingTowardZero((s32)((u32)first_input - 145ul));
        last_index = (u16)DivideS32By39TruncatingTowardZero((s32)((u32)last_input - 145ul));
        MarkControllerRecordIndexRangeAndEndpoint(first_index, last_index, 0u);
    }
}

/* RAM:faa8c7 */
void RecalculateType2ControllerRecordGeometryWindow(void)
{
    u16 highest_index = ReadBigEndianU16ControllerRecord(FW_HUGE_UNKNOWN_2BA5DB);
    u16 record_count = ReadBigEndianU16ControllerRecord(FW_HUGE_UNKNOWN_2BA5D9);
    s32 stride_position = (s32)((u32)highest_index * 39ul);
    s32 middle;
    s32 table_value;
    u8 group_index;
    u8 group_count;

    if((s16)record_count > (s16)highest_index)
    {
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA599, (s32)(stride_position + 184));
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA595, (s32)(stride_position + 496));
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA58D, (s32)(stride_position + 535));
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA57B, (s32)((u32)(highest_index + 1u) << 5));
    }
    else
    {
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA58D, (s32)(stride_position + 184));
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA595, (s32)(stride_position + 145));
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA599, (s32)(stride_position - 167));
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA57B, (s32)(s16)(highest_index - 8u) * 32l);
    }

    middle = ReadBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA595);
    table_value = DivideS32By39TruncatingTowardZero((s32)(((u32)middle - 145ul) << 5));
    WriteBigEndianS32ControllerRecord(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION, table_value);

    table_value = DivideS32By39TruncatingTowardZero((s32)(((u32)middle - 145ul) << 5));
    group_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA536);
    for(group_index = 0u; group_index < group_count; ++group_index)
    {
        table_value = (s32)(table_value - 32);
        WriteBigEndianS32ControllerRecord(FW_HUGE_UNKNOWN_2BA557 + (fw_addr_t)group_index * 4ul, table_value);
    }
}

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Fixed bytes from RAM:ff9633.  Written as executable selection logic until
 * HCONST has a reviewed below-ff0000 placement rule. */
/* Reconstruction helper; no direct Ghidra function address. */
static u8 ControllerE8ProfileAdjustment(u8 profile_index)
{
    if(profile_index == 3u)
        return 6u;
    if(profile_index == 4u || profile_index == 5u)
        return 12u;
    if(profile_index == 6u)
        return 20u;
    if(profile_index == 7u)
        return 8u;
    if(profile_index >= 8u && profile_index <= 12u)
        return 16u;
    return 0;
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 7
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fa8811 */
void PLEXTOR_NEAR_FUNCTION ResetControllerCommand0EMeasurementCaptureState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xbfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfeu));
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_10A4, 0ul);
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_109E, 0ul);
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_1099, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_109D, 0u);
    FirmwareWriteBigEndianU16ControllerState(FW_NEAR_UNKNOWN_10AC, 0u);
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_10A8, 0ul);
    FirmwareWriteBigEndianU16ControllerState(FW_NEAR_UNKNOWN_10B2, 0u);
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_10AE, 0ul);
}

/* RAM:fa8891 */
void PrepareControllerCommand0EMeasurementCapture(void)
{
    u8 enabled;

    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_10A4, 0ul);
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_109E, 0ul);
    FirmwareWriteBigEndianU16ControllerState(FW_NEAR_UNKNOWN_10AC, 0u);
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_10A8, 0ul);
    FirmwareWriteBigEndianU16ControllerState(FW_NEAR_UNKNOWN_10B2, 0u);
    FirmwareWriteBigEndianU32ControllerState(FW_NEAR_UNKNOWN_10AE, 0ul);
    enabled = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0x03u) != 0u);
    if(enabled != 0u)
    {
        WriteControllerParametersD4AndE2ForCode(0x74u);
        WriteControllerParameterWord(0xf028u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018097, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB7));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 4u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
    }
}

/* RAM:f8e41c */
void Port8097CalibrationFunction(void)
{
    u8 candidate = 0x18u;
    u8 accepted = 1u;
    u8 feedback;

    WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C0, 2u);
    while(accepted != 0u)
    {
        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_018097, candidate);
        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C0, 6u);
        WAIT_PORT8097_CALIBRATION(4000u, 1u);
        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C5, 0u);
        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C4, 8u);
        if(WaitControllerPort80F8Bit7Clear500AndYield() == 0u)
        {
            WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
            WaitControllerPort80F8Bit7ClearWithoutTimeout();
            accepted = 0u;
            break;
        }

        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C4, 8u);
        if(WaitControllerPort80F8Bit7Clear500AndYield() == 0u)
        {
            WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
            WaitControllerPort80F8Bit7ClearWithoutTimeout();
            accepted = 0u;
            break;
        }

        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        feedback = READ_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C7);
        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C0, 2u);
        if((s8)feedback >= (s8)0x9fu)
            break;
        if(candidate == 9u)
        {
            accepted = 0u;
            break;
        }
        candidate = (u8)((candidate + 1u) & 0x1fu);
    }

    WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
    if(accepted != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB7, candidate);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB7, 0xc0u);
        WRITE_PORT8097_CALIBRATION(FW_CONTROLLER_UNKNOWN_018097, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB7));
    }
}

/* RAM:f8f62b */
void WriteControllerRegister01Clamped(u8 value)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018001, value < 0x14u ? (u8)(value & 0x3fu) : 0x18u);
}

/* RAM:fca810 */
void ClearControllerParameterEEShadow(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC17, 0u);
    WriteControllerParameterEEFromShadow();
}

/* RAM:fd0156 */
void ClearControllerParameterD1ShadowBit6(void)
{
    u8 value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13) & 0xbfu);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC13, value);
    WriteControllerParameterWord((u16)(0xd100u | value));
}

/* RAM:fd00f5 */
void ClearControllerChannel86ForOperatingThresholds(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x20u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        SetIndexedControllerChannelValue(0u, 0x86u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) >= 0x10u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        SetIndexedControllerChannelValue(0u, 0x86u);
}

/* Fixed profile bytes from /validated/MONT_104.bin RAM:ff908e. */
static const u8 kControllerPortProfiles[14][8] = {
    { 0x23u, 0x80u, 0xffu, 0xc0u, 0x72u, 0x00u, 0xf6u, 0x4eu },
    { 0x23u, 0x80u, 0xffu, 0xc0u, 0x72u, 0x00u, 0xf6u, 0x4eu },
    { 0x13u, 0xc0u, 0xffu, 0xc0u, 0x73u, 0x00u, 0xf6u, 0x6au },
    { 0x01u, 0x84u, 0xc8u, 0xc0u, 0xb5u, 0x00u, 0xf6u, 0x6eu },
    { 0x01u, 0x84u, 0xc8u, 0xc0u, 0xb5u, 0x00u, 0xf6u, 0x6eu },
    { 0x01u, 0x84u, 0xa8u, 0xc0u, 0xb7u, 0x00u, 0xf6u, 0x72u },
    { 0x00u, 0x84u, 0xc7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x8eu },
    { 0x00u, 0x84u, 0xa7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x6eu },
    { 0x00u, 0x84u, 0xc7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x8eu },
    { 0x00u, 0x84u, 0xa7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x6eu },
    { 0x00u, 0x84u, 0xc7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x8eu },
    { 0x00u, 0x84u, 0xa7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x6eu },
    { 0x00u, 0x84u, 0xc7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x8eu },
    { 0x00u, 0x84u, 0xa7u, 0xc0u, 0x9fu, 0x00u, 0xf6u, 0x6eu }
};

/* Reconstruction helper; no direct Ghidra function address. */
static u16 DivideU16WithoutRuntimeLibrary(u16 dividend, u16 divisor, u16 *remainder_out)
{
    u16 quotient = 0;
    u16 remainder = 0;
    u8 bits = 16u;

    do
    {
        remainder = (u16)((u16)(remainder << 1) | (u16)(dividend >> 15));
        dividend = (u16)(dividend << 1);
        quotient = (u16)(quotient << 1);
        if(remainder >= divisor)
        {
            remainder = (u16)(remainder - divisor);
            quotient = (u16)(quotient | 1u);
        }
        bits--;
    } while(bits != 0);
    *remainder_out = remainder;
    return quotient;
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 8
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Fixed bytes from RAM:ff9640, addressed by the profile mapper. */
/* Reconstruction helper; no direct Ghidra function address. */
static u8 ControllerEfProfileValue(u8 profile_index)
{
    return profile_index < 3u ? 0u : 0xbcu;
}

/* RAM:f8a964 */
void WriteControllerParameterWord(u16 parameter_and_value)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xefu));
    FirmwareWriteDirectSfr8(D2, 0x80u);
    FirmwareWriteDirectSfr8(D7, 0x11u);
    FirmwareWriteDirectSfr8(D5, (u8)(parameter_and_value >> 8));
    FirmwareWriteDirectSfr8(D6, (u8)parameter_and_value);
    while((FirmwareReadDirectSfr8(D7) & 0x0fu) != 0)
    {
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 0x10u));
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:fc9ef3 */
void PLEXTOR_NEAR_FUNCTION WriteCalibratedControllerParameterForCode(u8 controller_parameter_code, u8 value)
{
    u16 parameter_and_value;

    switch(controller_parameter_code)
    {
    case 0x00u:
        parameter_and_value = (u16)(0xf200u | value);
        break;
    case 0x04u:
        parameter_and_value = (u16)(0xf300u | value);
        break;
    case 0x1cu:
        WriteControllerParameterWord((u16)(0xf800u | value));
        parameter_and_value = (u16)(0xf900u | value);
        break;
    case 0x24u:
        parameter_and_value = (u16)(0xfa00u | value);
        break;
    case 0x2cu:
        parameter_and_value = (u16)(0xf500u | value);
        break;
    case 0x3cu:
        parameter_and_value = (u16)(0xfe00u | (value & 0xfeu));
        break;
    default:
        return;
    }
    WriteControllerParameterWord(parameter_and_value);
}

/* RAM:fca623 */
void WriteControllerParameterE8Value(u8 value)
{
    WriteControllerParameterWord((u16)(0xe800u | value));
}

/* RAM:fca594 */
void WriteControllerParametersE8E9ForValue(u8 value)
{
    u8 adjustment = 0;

    if(value == 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1307, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) & 0xfeu));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && value != 0)
    {
        u8 profile_index = MapOperatingCodeToProfileIndex(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F));

        adjustment = ControllerE8ProfileAdjustment(profile_index);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) > 0x28u)
        {
            adjustment = (u8)(adjustment + 8u);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0)
                adjustment = FirmwareReadNear8(FW_NEAR_UNKNOWN_FA04);
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
            adjustment = (u8)(adjustment + 8u);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0 && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && value != 0)
        adjustment = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u ? 4u : 16u;
    WriteControllerParameterE8Value((u8)(value + adjustment));
    WriteControllerParameterWord(0xe900u);
}

/* RAM:f8b738 */
void ApplyControllerParametersE8E9ForScsiF3(u8 value)
{
    WriteControllerParametersE8E9ForValue(value);
}

/* RAM:f8a803 */
u8 WaitControllerPortReadyAndReadByte(fw_addr_t port_address)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    STARTUP_DIAGNOSTIC_POLL_DECLARE();

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x20u) == 0)
    {
        STARTUP_DIAGNOSTIC_POLL();
    }
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
    return FirmwareReadController8(port_address);
}

/* RAM:f8a7dc */
void WaitControllerPortReadyAndWriteByte(fw_addr_t port_address, u8 value)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    STARTUP_DIAGNOSTIC_POLL_DECLARE();

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x20u) == 0)
    {
        STARTUP_DIAGNOSTIC_POLL();
    }
    FirmwareWriteController8(port_address, value);
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:fe01f2 */
u8 ReadControllerByteAtAddress(u32 controller_address)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 value;

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018014, (u8)controller_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018015, (u8)(controller_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018016, (u8)(controller_address >> 16));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018017, 0x80u);
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x80u) == 0u)
    {
    }
    value = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018030);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018017, 0u);
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
    return value;
}

/* RAM:f8d936 */
void ClearControllerPorts80E4Through80E6And849D(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E4, 0);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E5, 0);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E6, 0);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0);
}

/* RAM:fb0659 */
void ClearStateFlag11EFBit5(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xdfu));
}

/* RAM:fc96b7 */
void WriteControllerParameterD6Value00(void)
{
    WriteControllerParameterWord(0xd600u);
}

/* RAM:fca4b6 */
void WriteControllerParametersEAEBFromScaledValue(u16 scaled_value)
{
    u16 coarse;
    u16 remainder;
    u16 fine;

    if(scaled_value >= 0x5dc0u)
        scaled_value = 0x5dc0u;
    if(scaled_value >= 0x4e20u)
    {
        coarse = 0x1fu;
        remainder = (u16)(scaled_value - 0x4e20u);
    }
    else
    {
        coarse = DivideU16WithoutRuntimeLibrary(scaled_value, 0x0285u, &remainder);
    }
    fine = DivideU16WithoutRuntimeLibrary((u16)(remainder * 10u), 0x009du, &remainder);
    WriteControllerParameterWord((u16)(0xea00u | (u16)(coarse << 3) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC07)));
    WriteControllerParameterWord((u16)(0xeb00u | fine));
}

/* RAM:f8b6e3 */
void SetControllerFlagFC07Bit2AndPort8098Bit6(u8 enabled)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC07, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC07) & 0xfbu));
    if(enabled != 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC07, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC07) | 4u));
    WriteControllerParametersEAEBFromScaledValue(0);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018098, enabled != 0 ? 0x40u : 0u);
}

/* RAM:f8b74d */
void EnableControllerPort8098WritePath(void)
{
    (void)WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_018098);
    SetControllerFlagFC07Bit2AndPort8098Bit6(1u);
    WriteControllerParameterD6Value00();
}

/* RAM:f8b769 */
void DisableControllerPort8098WritePath(void)
{
    SetControllerFlagFC07Bit2AndPort8098Bit6(0u);
}

/* RAM:fca62e */
void WriteControllerParameterEF00AndConditionalD0Value(u8 value)
{
    WriteControllerParameterWord(0xef00u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        if(value < 0x40u)
            WriteControllerParameterWord((u16)(0xd003u | (u16)(value << 2)));
        else
            WriteControllerParameterWord((u16)(0xd001u | (u16)(((u8)(value + 1u) & 0x3fu) << 2)));
    }
}

/* RAM:fca66d */
void WriteControllerParametersD2AndD3FromState(void)
{
    u8 d3_value = 0xb0u;

    WriteControllerParameterWord(0xd279u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        d3_value = 0xacu;
        if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9) <= 0x0578u)
            d3_value = 0xa8u;
    }
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) >> 3) & 3u) != 0)
    {
        d3_value = 0x98u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            d3_value = 0xb8u;
    }
    WriteControllerParameterWord((u16)(0xd300u | d3_value));
}

/* RAM:fca6b4 */
void WriteControllerParametersD1D2D3FromConfiguration(void)
{
    u8 flags;

    WriteControllerParameterWord(0xd2cdu);
    WriteControllerParameterWord(0xd398u);
    flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13) & 0xf3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC13, flags);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0)
        flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13) | 4u);
    else
        flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13) | 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC13, flags);
    WriteControllerParameterWord((u16)(0xd100u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13)));
}

/* RAM:fca525 */
void WriteControllerParametersECEDFromScaledValue(u16 scaled_value)
{
    u16 coarse;
    u16 remainder;
    u16 fine;

    if(scaled_value >= 0x8ca0u)
        scaled_value = 0x8ca0u;
    if(scaled_value >= 0x7530u)
    {
        coarse = 0x1fu;
        remainder = (u16)(scaled_value - 0x7530u);
    }
    else
    {
        coarse = DivideU16WithoutRuntimeLibrary(scaled_value, 0x03c8u, &remainder);
    }
    fine = DivideU16WithoutRuntimeLibrary((u16)(remainder * 10u), 0x00ebu, &remainder);
    WriteControllerParameterWord((u16)(0xec00u | (u16)(coarse << 3) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC08)));
    WriteControllerParameterWord((u16)(0xed00u | fine));
}

/* RAM:fca76c */
void WriteControllerParameterF4WithDownAdjustment(void)
{
    u8 value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) - 0x24u);
    WriteControllerParameterWord((u16)(0xf400u | value));
}

/* RAM:fca789 */
void WriteControllerParameterF4WithUpAdjustment(void)
{
    u8 value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) < 0xfcu)
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) + 0x24u);
    WriteControllerParameterWord((u16)(0xf400u | value));
}

/* RAM:fca7e4 */
void WriteControllerParameterEFFromOperatingState(void)
{
    u8 profile_index = MapOperatingCodeToProfileIndex(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F));

    WriteControllerParameterWord((u16)(0xef00u | ControllerEfProfileValue(profile_index)));
}

/* RAM:fca3d9 */
void WriteControllerParameterFCValue(u8 value)
{
    WriteControllerParameterWord((u16)(0xfc00u | value));
}

/* RAM:fca3e4 */
void WriteControllerParameterFDValue(u8 value)
{
    WriteControllerParameterWord((u16)(0xfd00u | value));
}

/* RAM:fca819 */
void WriteControllerParameterEEFromShadow(void)
{
    u8 value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC17);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC17) | 1u);
    WriteControllerParameterWord((u16)(0xee00u | value));
}

/* RAM:fca405 */
void WriteControllerParameterE2ValueAndRefreshEE(u8 value)
{
    WriteControllerParameterWord((u16)(0xe202u | value));
    if(value == 0x50u)
        WriteControllerParameterWord(0xee04u);
    else
        WriteControllerParameterEEFromShadow();
}

/* RAM:fca3ef */
void WriteControllerParameterF6ValueWithBits0And1Set(u8 value)
{
    WriteControllerParameterWord((u16)(0xf603u | value));
}

/* RAM:fca3fa */
void WriteControllerParameterF7Value(u8 value)
{
    WriteControllerParameterWord((u16)(0xf700u | value));
}

/* RAM:fca7b1 */
u8 PLEXTOR_NEAR_FUNCTION ComputeControllerParametersFFE0Value(void)
{
    u8 value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) == 0)
    {
        value = 0xf0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            value = 0xc8u;
    }
    else
    {
        value = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) & 0xe0u) - 0x18u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            value = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) & 0xe0u) - 0x28u);
    }
    return value;
}

/* RAM:fca35e */
void SelectAndWriteControllerParameterFCForOperatingCode(u8 operating_code)
{
    u8 value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) == 0)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            value = 0x68u;
            if(operating_code >= 0x20u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) & 1u) != 0)
                value = 0x58u;
        }
        else
        {
            value = 0xa0u;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
                value = 0x68u;
        }
    }
    else
    {
        value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && operating_code >= 0x20u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) & 1u) != 0)
            value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) - 0x10u);
    }
    WriteControllerParameterFCValue(value);
}

/* RAM:fca3bb */
void SelectAndWriteControllerParameterF6ForOperatingCode(u8 operating_code)
{
    u8 value = 0xc0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        value = 0xb0u;
        if(operating_code >= 0x20u)
            value = 0xa0u;
    }
    WriteControllerParameterF6ValueWithBits0And1Set(value);
}

/* RAM:f8cb75 */
void RefreshControllerParametersFCAndF6ForCurrentOperatingCode(void)
{
    SelectAndWriteControllerParameterFCForOperatingCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    SelectAndWriteControllerParameterF6ForOperatingCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
}

/* RAM:fca34b */
void ConfigureControllerParametersFCAndF6ForOperatingCode(u8 operating_code)
{
    SelectAndWriteControllerParameterF6ForOperatingCode(operating_code);
    SelectAndWriteControllerParameterFCForOperatingCode(operating_code);
}

/* RAM:fca6fb */
void WriteControllerParametersD4AndE2ForCode(u8 selector_code)
{
    u8 d4_value;

    if(selector_code == 0x14u)
    {
        WriteControllerParameterWord(0xe212u);
        d4_value = 0x12u;
    }
    else if(selector_code == 0x1cu)
    {
        WriteControllerParameterWord(0xe21fu);
        d4_value = 0x12u;
    }
    else if(selector_code == 0x74u)
    {
        WriteControllerParameterE2ValueAndRefreshEE(0x74u);
        d4_value = 0x32u;
    }
    else if(selector_code == 0xffu)
    {
        WriteControllerParameterE2ValueAndRefreshEE(0x88u);
        d4_value = 1u;
    }
    else if(selector_code == 0x34u)
    {
        WriteControllerParameterWord(0xe237u);
        d4_value = 0x12u;
    }
    else
    {
        for(;;)
        {
        }
    }
    WriteControllerParameterWord((u16)(0xd400u | d4_value));
}

/* RAM:fca75d */
void WriteControllerParametersD412AndE2Value58(void)
{
    WriteControllerParameterE2ValueAndRefreshEE(0x58u);
    WriteControllerParameterWord(0xd412u);
}

/* RAM:fc96c0 */
void WriteControllerParameterD6Value7E(void)
{
    WriteControllerParameterWord(0xd67eu);
}

/* RAM:fc96c9 */
void WriteControllerParameterD6Value20(void)
{
    WriteControllerParameterWord(0xd620u);
}

/* RAM:f8e3cd */
void WriteControllerParameterFBValueWithBit0Set(u8 value)
{
    WriteControllerParameterWord((u16)(0xfb01u | value));
}

/* RAM:fb22e9 */
u8 SelectAndWriteControllerParameterFBValue(void)
{
    u8 value = 0x80u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x40u) == 0)
        value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB6);
    WriteControllerParameterFBValueWithBit0Set(value);
    return value;
}

/* RAM:fa8b0c */
void DispatchControllerCommand0EMeasurementCaptureForCurrentQ(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0x10u) != 0u)
    {
#define PLEXTOR_BINDING_SECTION 9
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    }
}

/* RAM:fb1d0d */
void RefreshControllerParameterFBIfSfrD8RangeWide(void)
{
    u8 range_is_wide;

#define PLEXTOR_BINDING_SECTION 10
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(range_is_wide != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, SelectAndWriteControllerParameterFBValue());
    WriteControllerParameterFBValueWithBit0Set(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));
}

/* RAM:fca806 */
void SetControllerParameterEEShadowBit7(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC17, 0x80u);
    WriteControllerParameterEEFromShadow();
}

#define ClearP1Bit4FromC ClearP1Bit4
#define SetP1Bit4FromC SetP1Bit4

/* RAM:f8e734 */
void BuildControllerPort8087And808BShadowsFromConfiguration(void)
{
    u8 upper_mode_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_F95E);
    u8 lower_mode_code;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CA, 0u);
    if(upper_mode_code == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CA, 4u);
    else if(upper_mode_code == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CA, 8u);
    else if(upper_mode_code == 3u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CA, 0x0cu);

    lower_mode_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_F960);
    if(lower_mode_code == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CA) | 1u));
    else if(lower_mode_code == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CA) | 2u));
    else if(lower_mode_code == 3u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CA) | 3u));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CB, FirmwareReadNear8(FW_NEAR_UNKNOWN_F95F));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F95E) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CB, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CC, FirmwareReadNear8(FW_NEAR_UNKNOWN_F961));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F960) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CC, 0u);
}

/* RAM:f8e834 */
void WriteControllerPorts8088ACAnd808900(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018088, 0xacu);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018089, 0u);
}

/* RAM:f8e86e */
void ApplyControllerPort8087And808A808BShadows(void)
{
    SetP1Bit4FromC();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808B, FirmwareReadNear8(FW_NEAR_UNKNOWN_11CB));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808A, 2u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808B, FirmwareReadNear8(FW_NEAR_UNKNOWN_11CC));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808A, 1u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018087, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CA) | 0x80u));
}

/* RAM:f8e82b */
void ResetControllerPorts8087Through808B(void)
{
    ResetControllerPort8087And808A808BUnlessFlag0E();
    ClearControllerPorts8088And8089UnlessFlag0E();
}

/* RAM:f8e8c1 */
void ResetControllerPort8087And808A808BUnlessFlag0E(void)
{
    if(FirmwareReadBit000E() == 0u)
    {
        ClearP1Bit4FromC();
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018087, 0x80u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808A, 2u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808B, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808A, 1u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808B, 0u);
    }
}

/* RAM:f8e850 */
void ClearControllerPorts8088And8089UnlessFlag0E(void)
{
    if(FirmwareReadBit000E() == 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018088, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018089, 0u);
    }
}

/* RAM:fcfb59 */
void PLEXTOR_NEAR_FUNCTION CaptureControllerQAdr3PacketIfEnabled(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B) & 1u) != 0u && IsControllerQAdr3SubmodeEnabled(FirmwareReadNear8(FW_NEAR_UNKNOWN_1253)) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 4u));
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_124F, FW_NEAR_UNKNOWN_F51E, 10u);
    }
}

/* RAM:fcfb2e */
void PLEXTOR_NEAR_FUNCTION CaptureControllerQAdr2PacketIfEnabled(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 2u));
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_124F, FW_NEAR_UNKNOWN_F514, 10u);
    }
}

/* RAM:f8e7c3 */
void ApplyControllerPortShadowsForModeFlag0E(void)
{
    if(FirmwareReadBit000E() == 0u)
    {
        ResetControllerPort8087And808A808BUnlessFlag0E();
        ClearControllerPorts8088And8089UnlessFlag0E();
        return;
    }
    ApplyControllerPort8087And808A808BShadows();
    WriteControllerPorts8088ACAnd808900();
}

/* RAM:f8e7e8 */
void ApplyControllerPortStateForParsedQStatus(void)
{
    u8 reset_ports = 0u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0x20u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121B, 5u);
        reset_ports = 1u;
    }
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121B) < 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121B, 0xffu);
        reset_ports = 1u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_121B) == 0u)
    {
        reset_ports = 1u;
    }
    if(reset_ports != 0u)
    {
        ResetControllerPort8087And808A808BUnlessFlag0E();
        ClearControllerPorts8088And8089UnlessFlag0E();
        return;
    }
    ApplyControllerPort8087And808A808BShadows();
    WriteControllerPorts8088ACAnd808900();
}

/* RAM:f8eb8a */
void InitializeControllerCalibrationByteFromFeedback(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC2, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8) + 0x80u));
}

/* RAM:f8f784 */
void ConfigureControllerDrivePorts8496Through849D(u8 drive_code, u8 enabled)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018498, 0x7fu);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018499, 0x81u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018496, (u8)(drive_code | 0xc0u));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018497, 0x88u);
    if(enabled == 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180EF, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0xa0u);
        return;
    }
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180EF, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0x90u);
}

/* RAM:f8f80e */
void ConfigureControllerPorts8496Through8499ForOperatingRange(void)
{
    u8 in_low_operating_range;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0184A2, 0x80u);
    in_low_operating_range = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) < 0x10u) && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x10u));
    if(in_low_operating_range != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1002, 0x50u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1003, 0xc0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018496, 0xc9u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018497, 0xb0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1002, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1003, 0xffu);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018496, 0xc6u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018497, 0x80u);
    }
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018498, FirmwareReadNear8(FW_NEAR_UNKNOWN_1002));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018499, FirmwareReadNear8(FW_NEAR_UNKNOWN_1003));
}

/* RAM:f8fd8a */
void ResetControllerPorts80C4And80C5State(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) | 6u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0xefu));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 0u);
}

/* RAM:fc976e */
void ConfigureControllerParameterProfileForCode0A(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x40u) != 0u)
    {
        WriteControllerParameterWord(0xf029u);
        WriteControllerParameterWord(0xf125u);
    }
    else
    {
        WriteControllerParameterWord(0xf059u);
        WriteControllerParameterWord(0xf121u);
    }
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x0au);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc97c8 */
void ConfigureControllerParameterProfileForCode0C(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    WriteControllerParameterWord(0xf059u);
    WriteControllerParameterWord(0xf121u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x0cu);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc97fc */
void ConfigureControllerParameterProfileForCode10(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    WriteControllerParameterWord(FW_NEAR_UNKNOWN_F06B);
    WriteControllerParameterWord(0xf121u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x10u);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc9830 */
void ConfigureControllerParameterProfileForCode14(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    WriteControllerParameterWord(0xf07bu);
    WriteControllerParameterWord(0xf121u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x14u);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc99a3 */
void ConfigureControllerParameterProfileCode28F125(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    WriteControllerParameterWord(FW_NEAR_UNKNOWN_F08B);
    WriteControllerParameterWord(0xf125u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x28u);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:f8f20c */
void ConfigureControllerPorts80D0D1D3ForCurrentState(void)
{
    u8 operating_code;
    u8 select_state_path = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 3u)
            select_state_path = 1u;
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u)
            select_state_path = 1u;
    }
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D1, 0u);
    if(select_state_path != 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D0, 0x0bu);
        return;
    }

    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    if(operating_code < 0x18u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D0, operating_code == 4u ? 0x0bu : 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D3, 0u);
        return;
    }
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D0, 0x40u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D3, 8u);
}

/* RAM:f8c464 */
void ConfigureControllerPortsForDisabledOperatingPath(void)
{
    u8 strobe_value;

    DispatchWaitControllerPortForDisabledOperatingPath(FW_CONTROLLER_UNKNOWN_018085, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x18u)
        DispatchUpdateControllerWord035FForDisabledOperatingPath();
    DispatchWaitControllerPortForDisabledOperatingPath(FW_CONTROLLER_UNKNOWN_018086, 0u);
    DispatchWaitControllerPortForDisabledOperatingPath(FW_CONTROLLER_UNKNOWN_018082, 0xb0u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x80u) == 0u)
    {
        strobe_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u ? 0x94u : 0x14u;
        DispatchStrobeControllerPortForDisabledOperatingPath(FW_CONTROLLER_UNKNOWN_018079, strobe_value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, 0x41u);
        DispatchWaitControllerPortForDisabledOperatingPath(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
        DispatchWaitControllerPortForDisabledOperatingPath(FW_CONTROLLER_UNKNOWN_01809F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF7) | 0x80u));
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x20u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6) | 0x10u));
        DispatchWaitControllerPortForDisabledOperatingPath(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    }

    DispatchApplyControllerParameterD1ForDisabledOperatingPath();
    DispatchConfigureControllerD0D1D3ForDisabledOperatingPath();
}

/* RAM:f8c17b */
void SynchronizeLowOperatingCodeState(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAC) == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
}

/* RAM:f8b33d */
void ResetControllerChannelsAndApplyDriveCode5(u8 enabled)
{
    ResetFourIndexedControllerChannels();
    ConfigureControllerDrivePorts8496Through849D(5u, enabled);
}

/* RAM:fcf08f */
void WriteControllerPorts8090Through8097FromProfile(u8 profile_index)
{
    const u8 *profile;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 0x34u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        {
            if(profile_index == 8u)
                profile_index = 10u;
            else if(profile_index == 9u)
                profile_index = 11u;
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            if(profile_index == 8u)
                profile_index = 12u;
            else if(profile_index == 9u)
                profile_index = 13u;
        }
    }

    profile = kControllerPortProfiles[profile_index];
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018094, profile[0]);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018095, profile[1]);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018096, profile[2]);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018097, profile_index < 6u ? profile[3] : FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB7));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018092, profile[4]);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018091, profile[5]);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018093, profile[6]);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018090, profile[7]);
}

/* RAM:fcfbbf */
void PLEXTOR_NEAR_FUNCTION CopyControllerQAdr5PointB2ThroughB4Record(void)
{
    u8 point_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1247) - 0xb1u);
    u16 offset = (u16)((u16)point_index * 10u);
    fw_addr_t destination = FW_NEAR_UNKNOWN_F79C + offset;

    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, destination, 10u);
}

/* RAM:fcfbe4 */
void PLEXTOR_NEAR_FUNCTION CopyControllerQAdr5TrackPointRecord(void)
{
    u16 offset = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1247) * 10u);
    fw_addr_t destination = FW_NEAR_UNKNOWN_F7C4 + offset;

    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, destination, 10u);
}

/* RAM:fcfc06 */
void PLEXTOR_NEAR_FUNCTION ParseControllerQPacket(void)
{
    u8 track_number;
    u8 control_adr;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, 0u);
    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1250);
    if(track_number == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 8u));
    if(track_number == 0xaau)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 4u));

    control_adr = FirmwareReadNear8(FW_NEAR_UNKNOWN_124F);
    if((control_adr & 0x40u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 0x20u));
    else if((control_adr & 0x10u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 0x10u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1251) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 2u));
}

/* RAM:fb9d48 */
u8 IsRadialPositionEstimateAtMost3542(void)
{
    u16 radial_position = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1031);

    return (u8)((s16)radial_position <= (s16)0x0dd6);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianFirmwareU32(fw_addr_t address);

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianFirmwareU32(fw_addr_t address, u32 value);

/* RAM:fcfda0 */
void UpdateDecodedFramePositionAndRadialEstimate(u8 radial_table_index, fw_addr_t tuple_address)
{
    u8 radial_position_is_low = IsRadialPositionEstimateAtMost3542();
    s32 decoded_position;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1030, radial_position_is_low);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, radial_position_is_low == 1u ? 1u : 0u);
    decoded_position = ConvertBinaryMsfToAdjustedFramePosition(tuple_address);
    WriteBigEndianFirmwareU32(FW_NEAR_UNKNOWN_1235, (u32)decoded_position);
    WriteBigEndianFirmwareU32(FW_NEAR_UNKNOWN_0EF5, (u32)decoded_position);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);

    if(decoded_position < 0 || radial_table_index >= 0xa0u)
    {
        u16 radial_value = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0CFB);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1031, (u8)(radial_value >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1032, (u8)radial_value);
    }
    else
    {
        u16 radial_value = FirmwareReadBigEndianU16Near((u16)(0x0cfbu + ((u16)radial_table_index << 1)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1031, (u8)(radial_value >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1032, (u8)radial_value);
    }
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianFirmwareU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianFirmwareU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fcfc63 */
u8 PLEXTOR_NEAR_FUNCTION AdvanceDecodedFramePositionOutsideLeadInLeadOut(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 8u) != 0u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 4u) != 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
    {
        u32 previous_position = ReadBigEndianFirmwareU32(FW_NEAR_UNKNOWN_1235);
        u32 current_position = previous_position + 1u;
        WriteBigEndianFirmwareU32(FW_NEAR_UNKNOWN_1235, current_position);
        WriteBigEndianFirmwareU32(FW_NEAR_UNKNOWN_0EF5, previous_position);
    }
    return 1u;
}

/* RAM:fcfb8d */
u8 IsControllerQAdr3SubmodeEnabled(u8 submode)
{
    submode = (u8)(submode & 3u);
    if(submode == 0u)
        return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_124F) >> 2) & 1u);
    if(submode == 3u)
        return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_124F) >> 3) & 1u);
    return 0u;
}

/* RAM:fcfc9d */
void PLEXTOR_NEAR_FUNCTION DecodeControllerQPacketBcdTuples(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1248, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1252)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1249, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1253)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124A, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1254)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124C, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1256)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124D, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1257)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124E, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1258)));
}

/* RAM:f9c48c */
void InitializeControllerRegisterBlock81ThroughBD(void)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 offset;

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    for(offset = 0u; offset < 0x30u; ++offset)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018081 + offset, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B1, 9u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B6, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B7, 4u);
    for(offset = 0u; offset < 4u; ++offset)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9 + offset, 0u);
    for(offset = 0u; offset < 4u; ++offset)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9 + offset, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 9u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:fd0125 */
void ApplyControllerParameterD1ModeBits(void)
{
    u8 mode_bits = (FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0x20u) == 0u ? 0xc0u : 0xa0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC13, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13) | mode_bits));
    WriteControllerParameterWord((u16)(0xd100u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13)));
}

/* RAM:fc994a */
void ConfigureControllerParameterProfileForCode30(u8 alternate_profile)
{
    if(alternate_profile == 1u)
    {
        WriteControllerParameterWord(FW_NEAR_UNKNOWN_F08B);
        WriteControllerParameterWord(0xf121u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x20u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x20u);
        WriteControllerParameterWord(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u ? 0xf09du : 0xf09bu);
        WriteControllerParameterWord(0xf124u);
    }
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x30u);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc9901 */
void ConfigureControllerParameterProfileForCode28(u8 alternate_profile)
{
    if(alternate_profile == 1u)
    {
        WriteControllerParameterWord(FW_NEAR_UNKNOWN_F08B);
        WriteControllerParameterWord(0xf121u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x20u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x20u);
        WriteControllerParameterWord(0xf09bu);
        WriteControllerParameterWord(0xf124u);
    }
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x28u);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc98b2 */
void ConfigureControllerParameterProfileForCode20(u8 alternate_profile)
{
    u8 is_alternate = alternate_profile == 1u ? 1u : 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, is_alternate != 0u ? 0x30u : 0x20u);
    WriteControllerParameterWord(FW_NEAR_UNKNOWN_F08B);
    WriteControllerParameterWord(is_alternate != 0u ? 0xf120u : 0xf124u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF7));
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x20u);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc9864 */
void ConfigureControllerParameterProfileForCode18(u8 alternate_profile)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, alternate_profile != 0u ? 0x30u : 0x20u);
    WriteControllerParameterWord(0xf07bu);
    WriteControllerParameterWord(alternate_profile != 0u ? 0xf121u : 0xf124u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF7));
    ConfigureControllerParametersFCAndF6ForOperatingCode(0x18u);
    WriteControllerParameterWord(0xe4ddu);
}

/* RAM:fc96d2 */
void ConfigureControllerParameterProfileForCode01(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    WriteControllerParameterWord(FW_NEAR_UNKNOWN_F001);
    WriteControllerParameterWord(0xf120u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(1u);
    WriteControllerParameterWord(0xe455u);
}

/* RAM:fc973a */
void ConfigureControllerParameterProfileForCode08(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    WriteControllerParameterWord(0xf049u);
    WriteControllerParameterWord(0xf121u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(8u);
    WriteControllerParameterWord(0xe4d5u);
}

/* RAM:fc9706 */
void ConfigureControllerParameterProfileForCode04(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, 0x30u);
    WriteControllerParameterWord(0xf025u);
    WriteControllerParameterWord(0xf120u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(4u);
    WriteControllerParameterWord(0xe455u);
}

/* RAM:fc9548 */
void ResetControllerParameterState(void)
{
    u8 fa_value;
    u8 ffe0_value;

    WriteControllerParameterD6Value00();
    WriteControllerParameterWord(FW_NEAR_UNKNOWN_F001);
    WriteControllerParameterWord(0xf100u);
    WriteControllerParameterWord(0xf120u);
    WriteControllerParameterWord((u16)(0xf200u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC09)));
    WriteControllerParameterWord((u16)(0xf300u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0A)));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC14, 0xd8u);
    WriteControllerParameterF4WithDownAdjustment();
    WriteControllerParameterWord((u16)(0xf500u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0B)));
    WriteControllerParameterWord((u16)(0xf800u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0C)));
    WriteControllerParameterWord((u16)(0xf900u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0C)));
    fa_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0D);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        fa_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0E);
    WriteControllerParameterWord((u16)(0xfa00u | fa_value));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 1u) != 0u)
        WriteControllerParameterWord(FW_NEAR_UNKNOWN_FB81);
    ConfigureControllerParametersFCAndF6ForOperatingCode(1u);
    WriteControllerParameterWord((u16)(0xfe00u | (FirmwareReadNear8(FW_NEAR_UNKNOWN_FC10) & 0xfeu)));
    ffe0_value = ComputeControllerParametersFFE0Value();
    WriteControllerParameterWord((u16)(0xff00u | ffe0_value));
    WriteControllerParameterWord((u16)(0xe000u | ffe0_value));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC13, 0x90u);
    WriteControllerParameterWord(0xd190u);
    WriteControllerParameterWord(0xe100u);
    WriteControllerParameterWord(0xe3b0u);
    WriteControllerParametersD412AndE2Value58();
    WriteControllerParameterWord(0xe455u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC12, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC11, 0u);
    WriteControllerParametersE8E9ForValue(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC07, 0u);
    WriteControllerParametersEAEBFromScaledValue(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC08, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC08, 2u);
    WriteControllerParametersECEDFromScaledValue(0u);
    WriteControllerParameterWord(0xef00u);
    WriteControllerParametersD1D2D3FromConfiguration();
    WriteControllerParameterWord(0xe680u);
    WriteControllerParameterWord(0xe700u);
    ConfigureP0Bit3AndControllerParametersE7E5(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u ? 1u : 0u);
    WriteControllerParameterWord(0xd083u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC17, 0u);
    WriteControllerParameterEEFromShadow();
}

/* RAM:fca832 */
void ConfigureP0Bit3AndControllerParametersE7E5(u8 set_p0_bit3)
{
    u8 e5_value = 0;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC15, 0xc0u);
    if(set_p0_bit3 != 0)
    {
        SetP0Bit3Atomic();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC15, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15) | 0x20u));
        e5_value = 0x81u;
    }
    else
    {
        ClearP0Bit3Atomic();
    }
    WriteControllerParameterWord((u16)(0xe700u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15)));
    WriteControllerParameterWord((u16)(0xe500u | e5_value));
}

/* RAM:fca428 */
void ConfigureControllerForParameterState2(void)
{
    u8 ffe0_value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, 2u);
    StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_01803B, 0xaau);
    StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018079, 0x90u);
    ConfigureP0Bit3AndControllerParametersE7E5(1u);
    WriteControllerParameterEEFromShadow();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC1) == 0xffu)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC1));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) <= 0x18u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, 0u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) + 0xe8u));
    }

    WriteControllerParameterF4WithDownAdjustment();
    ffe0_value = ComputeControllerParametersFFE0Value();
    WriteControllerParameterWord((u16)(0xff00u | ffe0_value));
    WriteControllerParameterWord((u16)(0xe000u | ffe0_value));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC08, 2u);
    WriteControllerParametersECEDFromScaledValue(0u);
    ConfigureControllerParametersFCAndF6ForOperatingCode(1u);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void EnableIndexedControllerChannelIfNeeded(u16 shadow_address, u8 enable_value)
{
    if(FirmwareReadNear8(shadow_address) == 0)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180AC, enable_value);
        do
        {
        } while((WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180AC) & 0x7fu) != 0);
    }
}

/* RAM:f8b147 */
void SetIndexedControllerChannelValue(u8 channel_index, u8 value)
{
    if(channel_index == 0)
    {
        EnableIndexedControllerChannelIfNeeded(FW_NEAR_UNKNOWN_F98C, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98C, value);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A0, value);
    }
    else if(channel_index == 1u)
    {
        EnableIndexedControllerChannelIfNeeded(FW_NEAR_UNKNOWN_F98D, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98D, value);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A2, value);
    }
    else if(channel_index == 2u)
    {
        EnableIndexedControllerChannelIfNeeded(FW_NEAR_UNKNOWN_F98E, 4u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0x80u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98E, value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, value);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, value);
    }
    else if(channel_index == 3u)
    {
        EnableIndexedControllerChannelIfNeeded(FW_NEAR_UNKNOWN_F98F, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98F, value);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A6, value);
        SetP1Bit1Atomic();
        SetP1Bit0Atomic();
    }
}

/* RAM:f8b287 */
void SetControllerChannel0ValueForOperatingCode(u8 operating_code)
{
    SetIndexedControllerChannelValue(0u, (operating_code == 1u || operating_code == 2u) ? 0x9du : 0x85u);
}

/* RAM:f8b258 */
void ApplyCurrentOperatingCodeToControllerChannels0Through3(void)
{
    SetControllerChannel0ValueForOperatingCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    ApplyCurrentOperatingCodeToControllerChannels1Through3();
}

/* RAM:f8b265 */
void ApplyCurrentOperatingCodeToControllerChannels1Through3(void)
{
    SetIndexedControllerChannelValue(1u, 0x94u);
    SetIndexedControllerChannelValue(2u, 0xc1u);
    SetIndexedControllerChannelValue(3u, MapOperatingCodeToControllerChannel3Value(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43)));
}

/* RAM:f8a828 */
void SubmitControllerWordPair(u16 word0, u16 word1)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x20u) == 0)
    {
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F0, (u8)word0);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F1, (u8)(word0 >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F2, (u8)word1);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F3, (u8)(word1 >> 8));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 0x80u) != 0)
    {
    }
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:f8c8f1 */
void SubmitControllerWord0204WithOptionalHighCodeOffset(u8 subtract_high_code_offset)
{
    u16 calibrated_value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F977) == 2u)
        return;

    calibrated_value = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAB8);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x34u && subtract_high_code_offset != 0u)
        calibrated_value = (u16)(calibrated_value - 0x0400u);
    SubmitControllerWordPair(calibrated_value, 0x0204u);
}

#define PLEXTOR_BINDING_SECTION 11
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void StrobeControllerPortWriteByteAt(fw_addr_t port_address, u8 value)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 controller_shadow;

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    FirmwareWriteController8(port_address, value);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:f8a934 */
/* ACC supplies value and inherited DR0 supplies port_address. */
void StrobeControllerPortWriteByte(u8 value)
{
    fw_addr_t port_address = CAPTURE_CONTROLLER_STROBE_ADDRESS();

    StrobeControllerPortWriteByteAt(port_address, value);
}

/* C-facing form for ordinary typed callers. */
void StrobeControllerPortWriteByteFromC(fw_addr_t port_address, u8 value)
{
    StrobeControllerPortWriteByteAt(port_address, value);
}

#define PLEXTOR_BINDING_SECTION 12
#include "bindings/platform/controller_parameters_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f8e2b0 */
/* ACC supplies yield_mode, R7 supplies enforce_timeout, and inherited DR0
 * supplies the signed timer-owned timeout published at RAM:0011f8. */
u8 WaitControllerPort80F8Bit7Clear(u8 yield_mode, u8 enforce_timeout)
{
    s32 timeout = CAPTURE_CONTROLLER_F8_TIMEOUT();

    WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_11F8, timeout);
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 0x80u) != 0u)
    {
        if(yield_mode != 0u)
            YieldCoroutine1();
        if(enforce_timeout != 0u && ReadBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_11F8) <= 0l)
            return 0u;
    }
    return 1u;
}

/* RAM:f8d441 */
void EnterControllerMeasurementConfiguration(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180AC, 0x02u);
    do
    {
    } while((WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180AC) & 0x7fu) != 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180B8, 0x12u);
    SubmitControllerWordPair(0x1500u, 0x033au);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D4, 0x80u);
}

/* RAM:f8d488 */
void LeaveControllerMeasurementConfiguration(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D4, 0u);
    SubmitControllerWordPair(0u, 0x033au);
}

/* RAM:f8a882 */
u16 ReadControllerWord(u16 request_word)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 low;
    u8 high;

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x20u) == 0)
    {
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F2, (u8)request_word);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F3, (u8)((request_word >> 8) | 0x80u));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 0x80u) != 0)
    {
    }
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
    low = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F0);
    high = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F1);
    return (u16)(((u16)high << 8) | low);
}

/* RAM:f8a8e0 */
void SubmitControllerByteAndWord(u8 byte1, u16 word1)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x20u) == 0)
    {
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F1, byte1);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F2, (u8)word1);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180F3, (u8)(word1 >> 8));
    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 0x80u) != 0)
    {
    }
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:f8b58c */
void ConfigureControllerPorts8046And804EAnd80C9Through80CB(u8 enabled)
{
    u8 controller_8046 = 0u;
    u8 controller_804e = 0u;
    u8 controller_80c9 = 0u;
    u8 raw_q_profile_index = 0u;
    u16 raw_q_offset;
    u8 controller_shadow;

    if(enabled != 0u)
    {
        controller_8046 = 0x94u;
        controller_804e = 0x01u;
        controller_80c9 = 0xa3u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 10u)
        {
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806A, 0u);
            controller_8046 = 0xd4u;
            controller_804e = 0x04u;
            controller_80c9 = 0u;
        }
        else
        {
            raw_q_profile_index = MapOperatingCodeToProfileIndexMinusSeven(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
        }
    }

    ClearBit0547Atomic();
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, controller_8046);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01804E, controller_804e);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    SetBit0547Atomic();
    SetBit0546Atomic();
    FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 0x04u));

    /* Stock builds bank ff plus 2*index; MOV AB reads the low byte first. */
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C9, controller_80c9);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CA, 0x08u);
    raw_q_offset = (u16)((u16)raw_q_profile_index << 1);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CB, FirmwareRead8(FirmwareAddressIncrementLow16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF84FE), (u16)(raw_q_offset + 1u))));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CA, 0x04u);
    raw_q_offset = (u16)((u16)raw_q_profile_index << 1);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CB, FirmwareRead8(FirmwareAddressIncrementLow16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF84FE), raw_q_offset)));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CA, 0x02u);
    raw_q_offset = (u16)((u16)raw_q_profile_index << 1);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CB, FirmwareRead8(FirmwareAddressIncrementLow16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF850A), (u16)(raw_q_offset + 1u))));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CA, 0x01u);
    raw_q_offset = (u16)((u16)raw_q_profile_index << 1);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CB, FirmwareRead8(FirmwareAddressIncrementLow16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF850A), raw_q_offset)));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CA, 0u);
}

/* RAM:f8b49e */
void ConfigureControllerWords035F036F037FForOperatingCode(u8 operating_code)
{
    u8 port_a8;
    u8 port_a9;
    u16 word_35f;
    u16 word_36f;
    u16 word_37f;

    if(operating_code == 4u || operating_code == 8u)
    {
        port_a8 = operating_code == 4u ? 0x12u : 0x13u;
        port_a9 = 1u;
        word_35f = 0x06e4u;
        word_36f = 0x1880u;
        word_37f = 0x6000u;
    }
    else if(operating_code == 0x18u || operating_code == 0x20u || operating_code == 0x28u || operating_code == 0x34u)
    {
        port_a8 = 0x14u;
        port_a9 = 1u;
        word_36f = 0x0620u;
        word_37f = 0x1800u;
        word_35f = FirmwareReadBigEndianU16Near(operating_code == 0x18u ? 0x0cebu : operating_code == 0x20u ? 0x0cedu : operating_code == 0x28u ? 0x0cefu : 0x0cf3u);
    }
    else if(operating_code == 1u)
    {
        port_a8 = 0;
        port_a9 = 0x71u;
        word_35f = 0x1b90u;
        word_36f = 0x1880u;
        word_37f = 0x4580u;
    }
    else
    {
        for(;;)
        {
        }
    }
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A8, port_a8);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A9, port_a9);
    SubmitControllerWordPair(word_35f, 0x035fu);
    SubmitControllerWordPair(word_36f, 0x036fu);
    SubmitControllerWordPair(word_37f, 0x037fu);
}

/* RAM:f8b2ca */
/* The original ACC input and ERET exit are function-local native ABI
 * properties. This C entry models only the verified zero/one input states. */
void SetControllerOperatingEnableState(u8 enabled)
{
    if(enabled == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
            return;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xfbu));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x08u));
        SelectAndApplyOperatingCodeForMediaStateEnabled();
        ConfigureControllerPort8006AndChannel3(0u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 1u);
    ClearBit0542Atomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x04u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD));
    ConfigureControllerPort8006AndChannel3(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x08u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D) > 0x18u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x10u));
}

/* RAM:f8b3b7 */
void ConfigureControllerPort8006AndChannel3(u8 enabled)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

    if(enabled != 0u)
    {
        u8 profile_index = MapOperatingCodeToProfileIndex(operating_code);
        u16 table_word_address = (u16)(0x84cdu + (u16)((u16)(profile_index - 1u) << 1));

        SubmitControllerWordPair(FirmwareReadBigEndianU16Near(table_word_address), 0x036fu);
        SubmitControllerWordPair(0x1878u, 0x037fu);
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018006, 0xc0u);
    }
    else
    {
        ConfigureControllerWords035F036F037FForOperatingCode(operating_code);
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018006, 0x80u);
    }
    SetIndexedControllerChannelValue(3u, MapOperatingCodeToControllerChannel3Value(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43)));
}

/* RAM:f8c3d4 */
void ApplyOperatingCodeAndFixedControllerChannels0Through2(void)
{
    SetControllerChannel0ValueForOperatingCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    SetIndexedControllerChannelValue(1u, 0x94u);
    SetIndexedControllerChannelValue(2u, 0xc1u);
    ConfigureControllerPorts8496Through8499ForOperatingRange();
}

/* RAM:fc8d16 */
void SetCurrentOperatingCodeFromBoundedSelector(u8 selector_value, u8 upper_bound)
{
    if(upper_bound < selector_value)
        selector_value = upper_bound;

    if(selector_value == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
    }
    else if(selector_value == 4u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 4u);
    }
    else if(selector_value == 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 8u);
    }
    else if(selector_value == 10u || selector_value == 0x18u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 0x18u);
    }
    else if(selector_value == 0x20u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 0x20u);
    }
    else if(selector_value == 0x28u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 0x28u);
    }
    else if(selector_value == 0x34u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 0x34u);
    }
    else
    {
        for(;;)
        {
        }
    }
}

/* RAM:fd06cd */
void SetP1Bits0And1(void)
{
    SetP1Bit1Atomic();
    SetP1Bit0Atomic();
}

/* RAM:fd06ee */
u8 IsControllerPort80A8A9WithinScaledTolerance(u8 tolerance_percent, u8 lower_bound_only)
{
    u8 measurement_high;
    u16 measurement;
    u16 reference;
    u16 divisor;
    u16 scaled_tolerance;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180AA, 2u);
    measurement_high = WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180A9);
    measurement = (u16)(((u16)measurement_high << 8) | WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180A8));
    reference = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FC27);
    divisor = (u16)(100u / tolerance_percent);
    scaled_tolerance = (u16)(reference / divisor);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 2u || lower_bound_only != 0u)
    {
        if(measurement >= (u16)(reference - scaled_tolerance))
            return 1u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 2u)
        {
            if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180CC) & 0x10u) == 0)
            {
                WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CC, 0x12u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1221, 0x7du);
            }
            else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1221) < 0)
            {
                return 1u;
            }
        }
        return 0u;
    }
    return measurement >= (u16)(reference - scaled_tolerance) && measurement <= (u16)(reference + scaled_tolerance);
}

/* RAM:f8b0b3 */
void ResetIndexedControllerChannel(u8 channel_index)
{
    if(channel_index == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98C, 0u);
        SetControllerFlagFC07Bit2AndPort8098Bit6(0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A0, 0u);
        SubmitControllerWordPair(0u, 0x0250u);
    }
    else if(channel_index == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98D, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A2, 0u);
        SubmitControllerWordPair(0u, 0x0251u);
    }
    else if(channel_index == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    }
    else if(channel_index == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F98F, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A6, 0u);
        SubmitControllerWordPair(0u, 0x0253u);
    }
}

/* RAM:fd07d7 */
void ResetControllerChannel2(void)
{
    ResetIndexedControllerChannel(2u);
}

/* RAM:fb2214 */
void ConfigureControllerPorts8086And809DThrough80AE(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018086, 0x80u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, 0x43u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x20u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6) | 0x10u));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x80u) == 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809E, 0x40u);
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_01807A, 3u);
    }
    else
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809E, 0x80u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF7, 0x30u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF7));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018099, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180AE, 0x2bu);
}

/* RAM:fb21aa */
void PLEXTOR_NEAR_FUNCTION InitializeControllerRegistersD4D5AndWord23F(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D4, 0x20u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D5, 0x0fu);
    SubmitControllerWordPair(0x2000u, 0x023fu);
}

/* RAM:fb215a */
void PLEXTOR_NEAR_FUNCTION InitializeControllerRegistersC2D0ThroughD3(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D1, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C2, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D0, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D3, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D2, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809B, 0x0cu);
}

/* RAM:fb21d3 */
void PLEXTOR_NEAR_FUNCTION InitializeControllerRegistersD8D9(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D9, 1u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D8, 0u);
}

/* RAM:fb21ef */
void PLEXTOR_NEAR_FUNCTION ClearControllerRegistersDCDDAndWord23E(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180DD, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180DC, 0u);
    SubmitControllerWordPair(0u, 0x023eu);
}

/* RAM:fb2142 */
void ResetFourIndexedControllerChannels(void)
{
    ResetIndexedControllerChannel(0u);
    ResetIndexedControllerChannel(1u);
    ResetIndexedControllerChannel(2u);
    ResetIndexedControllerChannel(3u);
}

/* RAM:fd0a5d */
u8 PLEXTOR_NEAR_FUNCTION DoesControllerModeOneClassificationDifferFromCache(u16 mode_word)
{
    return (u8)((mode_word == 1u) != (FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FC2B) == 1u));
}



/* RAM:fa69c3 */
/* Called by the controller event interrupt after testing RAM:001039 bit 7. */
void PublishControllerOperatingResponse(void)
{
    u8 response;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0x7fu));
    response = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1055) & 0x7fu);
    response = response < 10u ? (u8)(response + 0x58u) : (u8)(response - 10u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, (u8)(response | 0x80u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) | 0x08u));
}

/* RAM:fa69f4 */
void CompleteControllerOperatingResponse(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) & 0xf7u));
    ApplyControllerOperatingProfileAfterResponse();
}

/* RAM:fa757b */
void EndReadRangeControllerMeasurement(void)
{
    CompleteControllerMeasurementSequence();
    WriteControllerParameterWord((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0E)));
    WriteControllerParameterWord((u16)(0xe700u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15)));
}

/* RAM:fa7885 */
/* The table remains a runtime firmware-memory read at RAM:ff9244. */
void ConfigureControllerRegisters32Through35ForOperatingCode(void)
{
    u8 profile = MapOperatingCodeToProfileIndex(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9244) + profile));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x13u);
}

/* RAM:fde9a2 */
/* Both callers push four bytes of the DR4 result from RAM:fde9cf.
 * After the three-byte ECALL frame, SPX-3/-4 are the low/next argument
 * bytes. This internal C value interface assumes stable caller arguments;
 * the canonical stack signature is left unforced. */
void StartRawReadHostTransferFromStackAddress(u32 transfer_value)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)transfer_value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(transfer_value >> 8));
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805E, 0x97u);
}

/* RAM:fce9db */
/* ACC holds the operating code. */
void SetHighOperatingCodeThresholds(u8 operating_code)
{
    WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_F9EA, 0l);
    if(operating_code == 0x34u)
    {
        WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_F9EE, 9000l);
        WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_F9F2, 81000l);
        WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_F9F6, 175500l);
    }
    else
    {
        WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_F9EE, 27000l);
        WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_F9F2, 112500l);
        WriteBigEndianS32ControllerRecord(FW_NEAR_UNKNOWN_F9F6, 220500l);
    }
}

/* RAM:fc8d77 */
u8 ConstrainCurrentOperatingCodeForTransition(u8 inherited_seed)
{
    u8 current_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 applied_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D);
    u8 result = inherited_seed;

    if(applied_code < current_code)
    {
        if(applied_code == 2u || applied_code == 4u)
        {
            result = 8u;
        }
        else if(applied_code == 8u || applied_code == 10u || applied_code == 12u || applied_code == 16u || applied_code == 20u || applied_code == 24u || applied_code == 32u || applied_code == 40u
                || applied_code == 52u)
        {
            result = 52u;
        }
        else if(applied_code == 1u)
        {
            result = 4u;
        }

        if(current_code <= result && current_code <= FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46))
            result = current_code;
    }
    else
    {
        result = current_code;
        if(applied_code > 0x17u && current_code < 0x19u && current_code == 1u)
            result = 8u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) & 1u) == 1u)
        result = 0x18u;

    current_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F);
    if(current_code != 0x34u && current_code < result)
        result = current_code;
    return result;
}

/* RAM:fca2e5 */
u8 MeasureAveragedSfrD8MinusD9WithOffsets(void)
{
    u16 d8_sum = 0u;
    u16 d9_sum = 0u;
    u8 sample_index;
    u16 d8_adjusted;
    u16 d9_adjusted;
    u16 difference;

    ClearBit0547Atomic();
    FirmwareWriteDirectSfr8(E1, 0u);
    SetBit0719Atomic();
    SetBit070FAtomic();
    for(sample_index = 0u; sample_index < 8u; ++sample_index)
    {
        while((FirmwareReadDirectSfr8(F1) & 0x01u) == 0u || (FirmwareReadDirectSfr8(F1) & 0x02u) == 0u)
        {
        }
        d8_sum = (u16)(d8_sum + FirmwareReadDirectSfr8(D8));
        d9_sum = (u16)(d9_sum + FirmwareReadDirectSfr8(D9));
    }
    ClearBit070FAtomic();
    ClearBit0719Atomic();
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetBit054AAtomic();

    d8_adjusted = (u16)((d8_sum >> 3) + 0x78u);
    d9_adjusted = (u16)((d9_sum >> 3) + 0x11u);
    difference = d8_adjusted < d9_adjusted ? 0u : (u16)(d8_adjusted - d9_adjusted);
    return difference < 0x100u ? (u8)difference : 0xffu;
}

/* RAM:fcea44 */
void PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionMeasurementFrom8Samples(void)
{
    u32 sample_sum = 0ul;
    u32 average;
    u8 sample_index;
    u8 parameter_state;
    u8 flags;
    u8 threshold;

    if(DispatchReadAcquisitionEightSampleReadiness(10u) != 1u)
    {
        flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xe7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, flags);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F99E, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xe7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(flags | 8u));
        }
        return;
    }

    DispatchReadAcquisitionEightSampleControllerCode(0x74u);
    for(sample_index = 0u; sample_index < 8u; ++sample_index)
    {
        DispatchReadAcquisitionEightSampleWait(1000u, 0u);
        if(DispatchReadAcquisitionEightSamplePulseWindow() != 1u)
        {
            DispatchReadAcquisitionEightSampleRestore();
            return;
        }
        sample_sum += DispatchReadAcquisitionEightSampleMeasurement();
    }

    average = sample_sum >> 3;
    parameter_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    if(parameter_state != 0u)
    {
        threshold = parameter_state == 2u ? 0x50u : 0x32u;
        flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xe7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(flags | 8u));
        if(average >= threshold)
        {
            flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xe7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, flags);
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F99E, (u8)average);
    DispatchReadAcquisitionEightSampleRestore();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1307, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) | 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x20u));
}

/* RAM:fb0e72 */
void PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionMeasurementFrom16Samples(void)
{
    u32 sample_sum = 0ul;
    u8 sample_index;
    u32 average;

    WriteControllerParametersD4AndE2ForCode(0x74u);
    for(sample_index = 0u; sample_index < 16u; sample_index++)
    {
        WaitTimer1ScaledDuration(1000u, 0u);
        sample_sum += (u32)MeasureAveragedSfrD8MinusD9WithOffsets();
    }
    average = sample_sum >> 4;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xe7u) | (average < 0x5bul ? 8u : 0u)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F99E, (u8)average);
    WriteControllerParametersD412AndE2Value58();
}

/* RAM:fc9f60 */
/* All three reviewed callers pass a nonzero constant (8, 32, or 64). */
u16 MeasureAverageSfrDbSamples(u8 sample_count)
{
    u32 sample_sum = 0ul;
    u16 sample_index;

    FirmwareWriteDirectSfr8(E1, 1u);
    SetBit070FAtomic();
    for(sample_index = 0u; sample_index < sample_count; ++sample_index)
    {
        while((FirmwareReadDirectSfr8(F1) & 0x08u) == 0u)
        {
        }
        sample_sum += FirmwareReadDirectSfr8(DB);
    }
    ClearBit070FAtomic();
    return (u16)(sample_sum / sample_count);
}

/* RAM:f8d3f4 */
void PLEXTOR_NEAR_FUNCTION UpdateConfigurationWordFAACBit7FromMeasurement(void)
{
    u16 configuration_word;
    u8 measurement;

    if(IsIndexedDriveStateActive(5u) != 0u)
        return;

    ResetIndexedControllerChannel(2u);
    EnterControllerMeasurementConfiguration();
    WriteControllerParametersD4AndE2ForCode(0x34u);
    WaitTimer1ScaledDuration(4000u, 0u);
    measurement = MeasureAveragedSfrD8MinusD9WithOffsets();
    configuration_word = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC);
    if(measurement >= 0x66u)
        configuration_word |= 0x0080u;
    else
        configuration_word &= 0xff7fu;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAC, (u8)(configuration_word >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAD, (u8)configuration_word);
    LeaveControllerMeasurementConfiguration();
    WriteControllerParametersD412AndE2Value58();
}
