#include "eight_channel_capture.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/eight_channel_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/eight_channel_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/eight_channel_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/eight_channel_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/eight_channel_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/eight_channel_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadEightChannelBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteEightChannelBigEndianU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:fcfce6 */
void HandleControllerPacketInterrupt(void)
{
    u8 index;

    CLEAR_CONTROLLER_PACKET_P1_BIT1();
    CaptureEightChannelMeasurementInputsAndStatus();
    if(((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01808E) >> 6) & 1u) == 1u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 1u) != 0u)
            WriteEightChannelBigEndianU16(FW_NEAR_UNKNOWN_FC22, (u16)(ReadEightChannelBigEndianU16(FW_NEAR_UNKNOWN_FC22) + 1u));
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01808C, 0x87u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 0x80u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 0x40u));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F77, 0x50u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) | 1u));
    {
        u8 saved_ie = FirmwareReadDirectSfr8(A8);
        u8 saved_a9 = FirmwareReadDirectSfr8(A9);

        CLEAR_CONTROLLER_PACKET_GLOBAL_INTERRUPT();
        for(index = 0u; index < 10u; ++index)
        {
            while(((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) >> 5) & 1u) == 0u)
            {
            }
            FirmwareWriteNear8((fw_addr_t)((u16)FW_NEAR_UNKNOWN_124F + index), FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01808D));
        }
        FirmwareWriteDirectSfr8(A8, saved_ie);
        FirmwareWriteDirectSfr8(A9, saved_a9);
#define PLEXTOR_BINDING_SECTION 7
#include "bindings/platform/eight_channel_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 0x80u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0xbfu));
    CLEAR_CONTROLLER_PACKET_EX1();
}

static u32 ReadEightChannelBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteEightChannelBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fa8382 */
void CaptureEightChannelMeasurementInputsAndStatus(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 1u) != 1u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1067, READ_EIGHT_CHANNEL_PORT_8083());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1068, READ_EIGHT_CHANNEL_PORT_8083());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1069, READ_EIGHT_CHANNEL_PORT_8083());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_106A, READ_EIGHT_CHANNEL_PORT_8083());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_106B, READ_EIGHT_CHANNEL_PORT_8083());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_106C, READ_EIGHT_CHANNEL_PORT_8084());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_106D, READ_EIGHT_CHANNEL_PORT_8084());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_106E, READ_EIGHT_CHANNEL_PORT_8084());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_106F, READ_EIGHT_CHANNEL_PORT_8084());

    if(((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01808E) >> 6) & 1u) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xefu));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) | 0x10u));
}

/* RAM:fa84c4 */
void PLEXTOR_NEAR_FUNCTION AccumulateEightChannelMeasurementCapture(void)
{
    u8 index;
    u32 window_start;
    u32 current_frame;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1062) == 0u)
        WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1063, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1235));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1062, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1062) + 1u));
    for(index = 0u; index < 8u; ++index)
    {
        u16 sum_address = (u16)(0x1070u + (u16)index * 2u);
        u8 input = FirmwareReadNear8((u16)(0x1067u + index));
        u16 sum = ReadEightChannelBigEndianU16(sum_address);

        WriteEightChannelBigEndianU16(sum_address, (u16)(sum + (u16)input));
    }

    window_start = ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1063);
    current_frame = ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1235);
    if((s32)(window_start + 0x4aul) > (s32)current_frame)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1080, FirmwareReadNear8(FW_NEAR_UNKNOWN_1062) == 75u ? 1u : 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1082, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1081, FirmwareReadNear8(FW_NEAR_UNKNOWN_1062));
    WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1083, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1063));
    for(index = 0u; index < 8u; ++index)
    {
        u16 source = (u16)(0x1070u + (u16)index * 2u);
        u16 destination = (u16)(0x1087u + (u16)index * 2u);

        WriteEightChannelBigEndianU16(destination, ReadEightChannelBigEndianU16(source));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1062, 0u);
    WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1063, 0ul);
    index = 8u;
    do
    {
        --index;
        WriteEightChannelBigEndianU16((u16)(0x1070u + (u16)index * 2u), 0u);
    } while(index != 0u);
}

/* RAM:fa861d */
void FinalizeEightChannelMeasurementCapture(void)
{
    u8 index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 1u) == 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1080) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1080, FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) == 0u ? 2u : 4u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1082) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1082, 1u);
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) >> 2) & 1u) == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1081, FirmwareReadNear8(FW_NEAR_UNKNOWN_1062));
                WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1083, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1063));
                for(index = 0u; index < 8u; ++index)
                {
                    u16 source = (u16)(0x1070u + (u16)index * 2u);
                    u16 destination = (u16)(0x1087u + (u16)index * 2u);

                    WriteEightChannelBigEndianU16(destination, ReadEightChannelBigEndianU16(source));
                }
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1081, 0u);
                WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1083, 0x96ul);
                index = 8u;
                do
                {
                    --index;
                    WriteEightChannelBigEndianU16((u16)(0x1087u + (u16)index * 2u), 0u);
                } while(index != 0u);
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1062, 0u);
}

static fw_addr_t FullTocFieldAddress(u16 descriptor_index, u16 field_offset)
{
    u16 low_address = (u16)(0x8003u + (u16)(descriptor_index * 11u) + field_offset);

    return FW_HUGE_UNKNOWN_2B0000 | (fw_addr_t)low_address;
}

/* RAM:fa870a */
u8 PLEXTOR_NEAR_FUNCTION DoesCurrentDecodedFrameMatchFullTocPointA2(void)
{
    u16 descriptor_index;
    u8 cached_count;
    s32 descriptor_frame;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 1u) == 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) < 1u)
        return 0u;

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) >> 5) & 1u) == 0u)
    {
        cached_count = 0u;
        descriptor_index = 1u;
        while(descriptor_index <= ReadEightChannelBigEndianU16(FW_NEAR_UNKNOWN_12DC))
        {
            if(FirmwareRead8(FullTocFieldAddress(descriptor_index, 0u)) == 0xa2u)
            {
                if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) >> 5) & 1u) == 0u)
                {
                    WriteEightChannelBigEndianU16((u16)(0xf1f0u + (u16)cached_count * 2u), descriptor_index);
                    cached_count++;
                }
                descriptor_frame = CONVERT_FULL_TOC_MSF_FOR_A2_MATCHER(FullTocFieldAddress(descriptor_index, 5u));
                if((s32)ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1235) == descriptor_frame)
                    return 1u;
            }
            descriptor_index++;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) | 0x20u));
    }
    else
    {
        u16 cache_index = 0u;

        while(cache_index <= 98u)
        {
            descriptor_index = ReadEightChannelBigEndianU16((u16)(0xf1f0u + cache_index * 2u));
            if(FirmwareRead8(FullTocFieldAddress(descriptor_index, 0u)) != 0xa2u)
                return 0u;
            descriptor_frame = CONVERT_FULL_TOC_MSF_FOR_A2_MATCHER(FullTocFieldAddress(descriptor_index, 5u));
            if((s32)ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1235) == descriptor_frame)
                return 1u;
            cache_index++;
        }
    }
    return 0u;
}

/* RAM:fa8909 */
void FinalizeControllerCommand0EMeasurementCapture(void)
{
    u8 sample_count;
    s32 signed_quotient;
    u32 unsigned_quotient;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 1u) == 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) >> 1) & 1u) == 0u)
        return;

    RESTORE_COMMAND0E_CONTROLLER_PARAMETERS();
    WRITE_COMMAND0E_CONTROLLER_PARAMETER(0xf025u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_10A2) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A2, FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) == 0u ? 2u : 4u);
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) >> 5) & 1u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) | 0x20u));
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) >> 3) & 1u) == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A3, FirmwareReadNear8(FW_NEAR_UNKNOWN_109D));
                WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A4, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_109E));

                sample_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_109D);
                signed_quotient = DIVIDE_COMMAND0E_SIGNED((s32)ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A8), (s32)(u32)sample_count);
                WriteEightChannelBigEndianU16(FW_NEAR_UNKNOWN_10AC, (u16)(u32)signed_quotient);

                sample_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_109D);
                unsigned_quotient = DIVIDE_COMMAND0E_UNSIGNED(ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10AE), (u32)sample_count);
                WriteEightChannelBigEndianU16(FW_NEAR_UNKNOWN_10B2, (u16)unsigned_quotient);
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A3, 0u);
                WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A4, 0x96ul);
                WriteEightChannelBigEndianU16(FW_NEAR_UNKNOWN_10AC, 0u);
                WriteEightChannelBigEndianU16(FW_NEAR_UNKNOWN_10B2, 0u);
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_109D, 0u);
}

/* RAM:fa8b1d */
u16 PLEXTOR_NEAR_FUNCTION ReadControllerCommand0EMeasurementWordWithTimeout(void)
{
    u8 completed = 0u;
    u16 result = 0u;

    FirmwareWriteDirectSfr8(8D, 0xe8u);
    FirmwareWriteDirectSfr8(8B, 0xcbu);
    CLEAR_COMMAND0E_TIMER_OVERFLOW();
    SET_COMMAND0E_TIMER_RUN();
    for(;;)
    {
        if(READ_COMMAND0E_TIMER_OVERFLOW() != 0u)
            break;
        if((s8)READ_COMMAND0E_CONTROLLER_STATUS() >= 0)
        {
            completed = 1u;
            break;
        }
    }
    CLEAR_COMMAND0E_TIMER_RUN();
    CLEAR_COMMAND0E_TIMER_OVERFLOW();

    if(completed != 0u)
    {
        u8 high;
        u8 low;

        WRITE_COMMAND0E_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        high = READ_COMMAND0E_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180C7);
        low = READ_COMMAND0E_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180C6);
        result = (u16)(((u16)high << 8) | low);
        WRITE_COMMAND0E_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) | 0x40u));
        WRITE_COMMAND0E_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        WAIT_COMMAND0E_CONTROLLER_TIMEOUT();
    }
    return result;
}

/* RAM:fa8a40 */
void PLEXTOR_NEAR_FUNCTION AccumulateControllerCommand0EMeasurementCapture(void)
{
    s16 primary_sample;
    u8 sample_count;
    s32 signed_quotient;
    u32 unsigned_quotient;
    s32 window_end;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_109D) == 0u)
        WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_109E, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1235));

    ISSUE_COMMAND0E_FOR_ACCUMULATOR();
    primary_sample = CAPTURE_COMMAND0E_PRIMARY_SAMPLE();
    WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A8, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A8) + (u32)(s32)primary_sample);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0x40u) == 0u)
        WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10AE, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10AE) + (u32)READ_COMMAND0E_COMPANION_WORD());

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_109D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_109D) + 1u));
    window_end = (s32)(ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_109E) + 74ul);
    if(window_end > (s32)ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_1235))
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A2, FirmwareReadNear8(FW_NEAR_UNKNOWN_109D) == 75u ? 1u : 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) | 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A3, FirmwareReadNear8(FW_NEAR_UNKNOWN_109D));
    WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A4, ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_109E));

    sample_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_109D);
    signed_quotient = DIVIDE_COMMAND0E_SIGNED((s32)ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A8), (s32)(u32)sample_count);
    WriteEightChannelBigEndianU16(FW_NEAR_UNKNOWN_10AC, (u16)(u32)signed_quotient);

    sample_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_109D);
    unsigned_quotient = DIVIDE_COMMAND0E_UNSIGNED(ReadEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10AE), (u32)sample_count);
    WriteEightChannelBigEndianU16(FW_NEAR_UNKNOWN_10B2, (u16)unsigned_quotient);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_109D, 0u);
    WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_109E, 0u);
    WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10A8, 0u);
    WriteEightChannelBigEndianU32(FW_NEAR_UNKNOWN_10AE, 0u);
}
