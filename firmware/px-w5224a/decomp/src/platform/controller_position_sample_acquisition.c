#include "controller_position_sample_acquisition.h"
#include "../media/decoder.h"
#include "controller_position_sample_evaluation.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_position_sample_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPositionSampleU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WritePositionSampleU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadPositionSampleU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePositionSampleU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ClearPositionSampleActiveBit(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
}

static void ClearPositionSampleWindowBit(void)
{
    WritePositionSampleU16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadPositionSampleU16(FW_NEAR_UNKNOWN_0FF5) & 0xfff7u));
}

static void SelectPositionSampleTargetWord(void)
{
    u8 speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
    fw_addr_t source = 0ul;

    if(speed == 1u)
        source = FW_NEAR_UNKNOWN_FB61;
    else if(speed == 4u)
        source = FW_NEAR_UNKNOWN_FB63;
    else if(speed == 8u)
        source = FW_NEAR_UNKNOWN_FB65;
    else if(speed == 16u || speed == 20u || speed == 24u || speed == 32u || speed == 40u || speed == 52u)
        source = FW_NEAR_UNKNOWN_FB67;

    if(source != 0ul)
        WritePositionSampleU16(FW_NEAR_UNKNOWN_FB5F, ReadPositionSampleU16(source));
}

/* RAM:f89fc3 */
u8 PLEXTOR_NEAR_FUNCTION AcquireControllerPositionSamplesAndUpdateCalibration(void)
{
    s32 target_frame_position;
    u8 index;

    WritePositionSampleU32(FW_NEAR_UNKNOWN_0FE5, 0x10000000ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF0, 0u);
    for(index = 0u; index < 5u; ++index)
        WritePositionSampleU16((fw_addr_t)(FW_NEAR_UNKNOWN_0FDB + (u16)index * 2u), 0u);

    SelectPositionSampleTargetWord();
    target_frame_position = (s32)(ReadPositionSampleU32(FW_NEAR_UNKNOWN_0FE9) - 0x489ul - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) * 15u)
                                  + (u32)(s32)(s16)((s16)(3 - (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF)) * 5));
    WritePositionSampleU16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadPositionSampleU16(FW_NEAR_UNKNOWN_0FF5) | 0x0010u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 6u);
    POSITION_SAMPLE_ARM();

    for(;;)
    {
        s8 capture_status = (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203);

        if(capture_status <= 0)
        {
            if(capture_status < 0)
                ClearPositionSampleActiveBit();
            return 0u;
        }

        if(POSITION_SAMPLE_PARSE() != 0xfeu)
        {
            s32 current_position;

            POSITION_SAMPLE_ARM();
            current_position = (s32)ReadPositionSampleU32(FW_NEAR_UNKNOWN_1235);
            if(current_position == target_frame_position)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FED, 0u);
                WritePositionSampleU16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadPositionSampleU16(FW_NEAR_UNKNOWN_0FF5) | 8u));
            }
            else if(current_position == (s32)((u32)target_frame_position + 1ul) && (ReadPositionSampleU16(FW_NEAR_UNKNOWN_0FF5) & 8u) == 0u)
            {
                ClearPositionSampleActiveBit();
                return 0u;
            }
            else if(current_position == (s32)((u32)target_frame_position + 4ul) && FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A4) <= 0x10u)
            {
                ClearPositionSampleWindowBit();
                ClearPositionSampleActiveBit();
                return FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF1) == 0u ? 11u : 3u;
            }
            else if(current_position >= (s32)((u32)target_frame_position + 5ul))
            {
                ClearPositionSampleWindowBit();
                ClearPositionSampleActiveBit();
                return POSITION_SAMPLE_EVALUATE((s16)(((u32)target_frame_position + 5ul) >> 16));
            }
        }
        POSITION_SAMPLE_YIELD();
    }
}
