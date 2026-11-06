#include "raw_read_pattern_validation.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

extern void YieldCoroutine0(void);

static u16 ReadPatternWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadPatternDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static u8 PatternByte(fw_addr_t base, u16 offset)
{
    return FirmwareRead8(FirmwareAddressIncrement24(base, offset));
}

/* RAM:fbf806 */
u8 PLEXTOR_NEAR_FUNCTION ValidateRawReadDescriptorPayloadPatternBatch(u16 descriptor_count)
{
    u8 valid = 1u;
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_pattern_validation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1298, FirmwareReadNear8(FW_NEAR_UNKNOWN_1294));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1299, FirmwareReadNear8(FW_NEAR_UNKNOWN_1295));

    while(descriptor_count != 0u)
    {
        u16 descriptor_address;
        fw_addr_t payload;
        s32 frame_position;
        s32 relative_position;
        u16 offset;
        u8 repeated;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        YieldCoroutine0();

        descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
        frame_position = (s32)ReadPatternDword((u16)(descriptor_address + 2u));
        payload = FirmwareAddressIncrement24(ReadPatternDword((u16)(descriptor_address + 6u)), 0x10u);

        if(PatternByte(payload, 0u) != 0u)
            valid = 0u;
        for(offset = 1u; offset < 11u; ++offset)
            if(PatternByte(payload, offset) != 0xffu)
                valid = 0u;
        if(PatternByte(payload, 11u) != 0u)
            valid = 0u;

        ConvertFramePositionToBinaryMsfTuple(frame_position, tuple_address);
        ConvertThreeBinaryBytesToPackedBcd((u16)tuple_address);
        for(offset = 0u; offset < 3u; ++offset)
            if(PatternByte(payload, (u16)(12u + offset)) != FirmwareRead8(FirmwareAddressIncrement24(tuple_address, offset)))
                valid = 0u;
        if(PatternByte(payload, 15u) != 1u)
            valid = 0u;

        relative_position = (s32)((u32)frame_position - 150ul);
        if(PatternByte(payload, 16u) != (u8)((u32)relative_position >> 24))
            valid = 0u;
        if(PatternByte(payload, 17u) != (u8)((u32)relative_position >> 16))
            valid = 0u;
        if(PatternByte(payload, 18u) != (u8)((u32)relative_position >> 8))
            valid = 0u;
        if(PatternByte(payload, 19u) != (u8)relative_position)
            valid = 0u;

        for(offset = 0u; offset < 3u; ++offset)
            if(PatternByte(payload, (u16)(20u + offset)) != FirmwareRead8(FirmwareAddressIncrement24(tuple_address, offset)))
                valid = 0u;
        if(PatternByte(payload, 23u) != 1u)
            valid = 0u;

        repeated = (u8)relative_position;
        for(offset = 24u; offset < 0x810u; offset += 4u)
            if(PatternByte(payload, offset) != repeated || PatternByte(payload, (u16)(offset + 1u)) != repeated || PatternByte(payload, (u16)(offset + 2u)) != repeated
                || PatternByte(payload, (u16)(offset + 3u)) != repeated)
                valid = 0u;

        if(valid == 0u)
            return 0u;

        {
            u16 cursor = ReadPatternWord(FW_NEAR_UNKNOWN_1298);
            u16 mask = ReadPatternWord(FW_NEAR_UNKNOWN_129A);

            cursor = (u16)((cursor + 1u) & mask);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1298, (u8)(cursor >> 8));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1299, (u8)cursor);
        }
        --descriptor_count;
    }
    return valid;
}
