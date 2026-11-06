#include "raw_read_pattern_acquisition.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "raw_read_pattern_validation.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_pattern_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_pattern_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPatternAcquisitionWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadPatternAcquisitionDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePatternAcquisitionDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fbfc9f */
s8 PLEXTOR_NEAR_FUNCTION AcquireAndValidateRawReadPatternRange(s32 start_frame_position, u32 frame_count)
{
    s32 boundary;
    u8 retry_count = 0u;
    u8 status;

    WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_0F0C, 0ul);
    CLEAR_PATTERN_BIT_000C();
    CLEAR_PATTERN_BIT_000D();
    CLEAR_PATTERN_BIT_001D();
    CLEAR_PATTERN_BIT_001C();
    CLEAR_PATTERN_BIT_0013();
    WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_15DC, (u32)start_frame_position);
    WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_0B02, (u32)start_frame_position);
    WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_0B06, (u32)(u16)(ReadPatternAcquisitionWord(FW_NEAR_UNKNOWN_129C) - 16u));
    boundary = (s32)(ReadPatternAcquisitionDword(FW_NEAR_CURRENT_FRAME_POSITION) + 0x19c6ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, 40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3D, 15u);

    if((s32)((u32)start_frame_position + frame_count) >= boundary)
        return (s8)0xfdu;

    do
    {
        s32 command_start = (s32)ReadPatternAcquisitionDword(FW_NEAR_UNKNOWN_0B02);
        u32 batch_count = ReadPatternAcquisitionDword(FW_NEAR_UNKNOWN_0B06);

        if((s32)((u32)command_start + batch_count) >= boundary)
        {
            batch_count = (u32)((u32)boundary - (u32)command_start);
            WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_0B06, batch_count);
        }
        if(frame_count < batch_count)
        {
            batch_count = frame_count;
            WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_0B06, batch_count);
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
        RESET_PATTERN_QUEUE();
        INITIALIZE_PATTERN_COROUTINE(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
        WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_1598, RESET_PATTERN_TRANSFER());

        do
        {
            YIELD_PATTERN_COROUTINE();
            status = POLL_PATTERN_BATCH();
        } while(status == 0xfeu);

        if(status == 0u)
        {
            status = 0xffu;
            break;
        }

        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 4)
        {
            ++retry_count;
            if(retry_count >= 3u)
            {
                status = 0xffu;
                break;
            }
        }
        else
        {
            status = 1u;
            batch_count = ReadPatternAcquisitionDword(FW_NEAR_UNKNOWN_0B06);
            if(VALIDATE_PATTERN_BATCH((u16)batch_count) == 0u)
            {
                status = 0xfeu;
                break;
            }
            WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_0B02, ReadPatternAcquisitionDword(FW_NEAR_UNKNOWN_0B02) + batch_count);
            frame_count -= batch_count;
        }
    } while((s32)frame_count > 0);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    RESET_PATTERN_QUEUE();
    INITIALIZE_PATTERN_COROUTINE(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    WritePatternAcquisitionDword(FW_NEAR_UNKNOWN_1598, 150ul);
    DISPATCH_PATTERN_REQUEST5();
    YIELD_PATTERN_COROUTINE();
    return (s8)status;
}
