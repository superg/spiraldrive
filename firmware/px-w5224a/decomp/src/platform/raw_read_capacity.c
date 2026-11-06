#include "raw_read_capacity.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/raw_read_capacity_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadRawReadCapacityU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadRawReadCapacityU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u8 SetRawReadProducerCapacityWait(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B4, 1u);
    return 1u;
}

/* RAM:fe2b6f */
u8 PLEXTOR_NEAR_FUNCTION ShouldWaitForRawReadProducerCapacity(void)
{
    u16 reserve_slots;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1596) != 0u)
        return SetRawReadProducerCapacityWait();

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        if(ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1292) < 0x00c3u)
            return 0u;
        return SetRawReadProducerCapacityWait();
    }

    if(FirmwareReadBit0032() != 0u)
    {
        u16 restart_threshold = (u16)(ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_129C) - ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1552));

        restart_threshold = (u16)(restart_threshold - ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1463));
        if(restart_threshold <= ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1292))
            RESTART_RAW_READ_ACQUISITION();
    }

    reserve_slots = ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1463);
    if(reserve_slots > 3u)
        reserve_slots = ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1463);
    else
        reserve_slots = 3u;

    if(ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1292) >= (u16)(ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_129C) - reserve_slots))
        return SetRawReadProducerCapacityWait();

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u
        && ReadRawReadCapacityU32(FW_NEAR_UNKNOWN_0C4B) < (u32)ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_129C)
        && (u32)ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_129C) < ReadRawReadCapacityU32(FW_NEAR_UNKNOWN_0C4B) + (u32)ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1292))
        return SetRawReadProducerCapacityWait();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) != 1u && ReadRawReadCapacityU32(FW_NEAR_UNKNOWN_1455) != 0ul
        && (u32)(u16)(ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_129C) - ReadRawReadCapacityU16(FW_NEAR_UNKNOWN_1292)) < ReadRawReadCapacityU32(FW_NEAR_UNKNOWN_1455))
        return SetRawReadProducerCapacityWait();

    return 0u;
}
