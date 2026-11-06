#include "raw_q_position_target_probe.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_q_position_target_probe_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadTargetWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadTargetDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteTargetDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fc5e81 */
u8 ProbeRawReadQPositionAtTarget(void)
{
    s32 target_position = (s32)ReadTargetDword(FW_NEAR_UNKNOWN_1598);
    u16 descriptor_address;
    u16 descriptor_status;
    u8 result;

    WriteTargetDword(FW_NEAR_UNKNOWN_0F41, (u32)target_position);
    if(target_position >= 0 && target_position <= 450)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x10u) != 0u)
            return 7u;
        if(READ_TARGET_BIT0017() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) != 0u)
            return 7u;
        CLEAR_TARGET_BIT0017();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    SET_TARGET_HANDSHAKE_PINS(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 0x0bu);
    RESET_TARGET_QUEUE();
    INITIALIZE_TARGET_COROUTINE2(FW_ENTRY_RUN_RAW_READ_POSITION_PROBE_COROUTINE2);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F07, 2u);
    do
    {
        YIELD_TARGET_COROUTINE3();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
            return 4u;
    } while(READ_TARGET_BIT0008() == 1u);

    descriptor_address = GET_TARGET_LATEST_DESCRIPTOR();
    descriptor_status = ReadTargetWord((fw_addr_t)(u16)(descriptor_address + 0x0eu));
    if(descriptor_status == 0x0281u || descriptor_status == 0x2100u)
    {
        result = 7u;
    }
    else if(descriptor_status == 0u)
    {
        result = 1u;
    }
    else if(descriptor_status == 0x800bu)
    {
        u32 cursor = ReadTargetDword((fw_addr_t)(u16)(descriptor_address + 4u));
        u8 tuple_index;

        result = 7u;
        WriteTargetDword(FW_NEAR_UNKNOWN_15D8, cursor);
        if(cursor < 0x930ul)
        {
            cursor += ReadTargetDword(FW_NEAR_UNKNOWN_155E);
            WriteTargetDword(FW_NEAR_UNKNOWN_15D8, cursor);
        }
        WriteTargetDword(FW_NEAR_UNKNOWN_15D8, cursor - 0x924ul);

        for(tuple_index = 0u; tuple_index < 4u; ++tuple_index)
        {
            u8 minute = READ_TARGET_RING_BYTE();
            u8 second = READ_TARGET_RING_BYTE();
            u8 frame = READ_TARGET_RING_BYTE();
            u8 marker = (u8)(READ_TARGET_RING_BYTE() & 0xe0u);

            if(marker >= 0xc0u)
            {
                s32 candidate = ConvertBinaryMsfValuesToAdjustedFramePosition(PackedBcdByteToBinary(minute), PackedBcdByteToBinary(second), PackedBcdByteToBinary(frame)) + 1;
                s32 tracked = (s32)ReadTargetDword(FW_NEAR_UNKNOWN_0F41);
                s32 delta;

                if(marker == 0xe0u)
                    candidate++;
                delta = tracked - candidate;
                if(delta > 150 || delta < -150)
                    candidate = tracked + 1;
                WriteTargetDword(FW_NEAR_UNKNOWN_0F41, (u32)candidate);
                result = 6u;
                break;
            }
            WriteTargetDword(FW_NEAR_UNKNOWN_15D8, ReadTargetDword(FW_NEAR_UNKNOWN_15D8) + 0x92cul);
        }
    }
    else
    {
        result = 4u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_TARGET_HANDSHAKE_PINS(1u);
    return result;
}
