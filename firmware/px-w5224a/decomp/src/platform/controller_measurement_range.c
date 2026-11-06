#include "controller_measurement_range.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadAcquireRangeDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteAcquireRangeDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 AcquireRangeMeasurementActive(void)
{
    return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) >> 3) & 1u);
}

static u8 FailAcquireRange(u8 result_code)
{
    if(AcquireRangeMeasurementActive() == 1u)
        ACQUIRE_RANGE_END();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, result_code);
    return 0u;
}

/* RAM:fa7317 */
u8 PLEXTOR_NEAR_FUNCTION AcquireConfiguredReadMeasurementRange(void)
{
    s32 current_position;
    s32 previous_position;
    s32 range_start;
    s32 range_end;
    u8 packet_result;

    ACQUIRE_RANGE_WRITE_INITIAL(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) & 0xf7u));
    WriteAcquireRangeDword(FW_NEAR_UNKNOWN_F099, ReadAcquireRangeDword(FW_NEAR_UNKNOWN_1235));
    ACQUIRE_RANGE_ARM();

    for(;;)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 14u);
        do
        {
            packet_result = ACQUIRE_RANGE_PARSE();
            if(packet_result == 0xfeu)
            {
                ACQUIRE_RANGE_YIELD();
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
                {
                    if(AcquireRangeMeasurementActive() == 1u)
                        ACQUIRE_RANGE_END();
                    return 1u;
                }
                if(AcquireRangeMeasurementActive() == 1u && (FirmwareReadBit0407() == 0u || (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) < 0))
                {
                    ACQUIRE_RANGE_END();
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, 8u);
                    return 0u;
                }
            }
        } while(packet_result == 0xfeu);

        ACQUIRE_RANGE_ARM();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
            return FailAcquireRange(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
        if(FirmwareReadBit0407() != 1u)
            return FailAcquireRange(8u);

        previous_position = (s32)ReadAcquireRangeDword(FW_NEAR_UNKNOWN_F099);
        current_position = (s32)ReadAcquireRangeDword(FW_NEAR_UNKNOWN_1235);
        if(current_position < previous_position || (s32)((u32)previous_position + 1ul) < current_position)
            return FailAcquireRange(6u);

        WriteAcquireRangeDword(FW_NEAR_UNKNOWN_F099, (u32)current_position);
        range_start = (s32)ReadAcquireRangeDword(FW_NEAR_UNKNOWN_F091);
        if(current_position == (s32)((u32)range_start - 1ul))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) | 8u));
            ACQUIRE_RANGE_BEGIN();
        }

        if(AcquireRangeMeasurementActive() == 0u)
        {
            range_start = (s32)ReadAcquireRangeDword(FW_NEAR_UNKNOWN_F091);
            current_position = (s32)ReadAcquireRangeDword(FW_NEAR_UNKNOWN_1235);
            if(range_start < current_position)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, 8u);
                return 0u;
            }
        }

        range_end = (s32)ReadAcquireRangeDword(FW_NEAR_UNKNOWN_F095);
        current_position = (s32)ReadAcquireRangeDword(FW_NEAR_UNKNOWN_1235);
        if(current_position >= range_end)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) & 0xf7u));
            ACQUIRE_RANGE_END();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, 0u);
            return 1u;
        }

        ACQUIRE_RANGE_ADVANCE();
    }
}
