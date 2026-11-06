#include "raw_read_position_validation.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_sector_descriptor_scratch.h"
#include "../platform/status_mapping.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_ring_alignment.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_position_validation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPositionValidationU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WritePositionValidationU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadPositionValidationU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePositionValidationU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void FinishPositionValidationWithStatus(u16 status, u8 mode)
{
    POSITION_ENQUEUE_TIMING(status, mode);
    WritePositionValidationU32(FW_NEAR_UNKNOWN_0F41, ReadPositionValidationU32(FW_NEAR_UNKNOWN_1598));
}

/* RAM:f937fa */
void PLEXTOR_NEAR_FUNCTION ServiceRawReadPositionValidationAndDescriptorEnqueue(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1F) != 0u)
    {
        if(POSITION_CHECK_P1() == 0u)
        {
            u8 old_secondary_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24);
            s16 secondary_threshold = (s16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 2u);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, (u8)(old_secondary_count - 1u));
            if((s16)(u16)old_secondary_count <= secondary_threshold)
            {
                FinishPositionValidationWithStatus(0x800bu, 9u);
                return;
            }
            POSITION_INITIALIZE_CURSORS();
            return;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1F) - 1u));
    }
    else
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 0x40u) != 0u && POSITION_CHECK_P1() == 0u)
        {
            FinishPositionValidationWithStatus(0x800bu, 9u);
            return;
        }

        if(POSITION_DECODE(0u) != 0u)
        {
            s32 target = (s32)ReadPositionValidationU32(FW_NEAR_UNKNOWN_1598);
            s32 decoded = (s32)ReadPositionValidationU32(FW_NEAR_UNKNOWN_0F6E);
            s32 delta = (s32)((u32)target - (u32)decoded);

            if(delta >= -30L && delta <= 30L)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);
                WritePositionValidationU32(FW_NEAR_UNKNOWN_1598, (u32)decoded);
            }
            else
            {
                u8 old_retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E);

                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, (u8)(old_retry_count - 1u));
                if((s8)old_retry_count <= 0)
                {
                    FinishPositionValidationWithStatus(0x0082u, 4u);
                    return;
                }
            }
        }
        else
        {
            u8 old_retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, (u8)(old_retry_count - 1u));
            if((s8)old_retry_count <= 0)
            {
                FinishPositionValidationWithStatus(0x0082u, 4u);
                return;
            }
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 2u)
    {
        s32 limit = (s32)ReadPositionValidationU32(FW_NEAR_UNKNOWN_0F36);
        s32 target = (s32)ReadPositionValidationU32(FW_NEAR_UNKNOWN_1598);

        if(target >= limit)
        {
            FinishPositionValidationWithStatus(0u, 0u);
            return;
        }
        WritePositionValidationU16(FW_NEAR_UNKNOWN_1298, (u16)((ReadPositionValidationU16(FW_NEAR_UNKNOWN_1298) + 1u) & ReadPositionValidationU16(FW_NEAR_UNKNOWN_129A)));
        POSITION_LIMIT_BACKLOG();
    }

    POSITION_BUILD_DESCRIPTOR(0u, 0u);
    POSITION_ENQUEUE_DESCRIPTOR();
    WritePositionValidationU32(FW_NEAR_UNKNOWN_1598, ReadPositionValidationU32(FW_NEAR_UNKNOWN_1598) + 1ul);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 1u && ReadPositionValidationU16(FW_NEAR_UNKNOWN_1292) > 0x001eu)
    {
        POSITION_INITIALIZE_CURSORS();
        POSITION_SET_TERMINAL();
    }
}
