#include "write_record_prepare_predicate.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

static u32 ReadWriteRecordPrepareU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteWriteRecordPrepareU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static s32 DivideWriteRecordPrepareBy1100TowardZero(s32 value)
{
    if(value >= 0l)
        return value / 0x1100l;
    return -(s32)(((u32)(-(value + 1l)) + 1ul) / 0x1100ul);
}

static u8 IsMappedWriteStartBeforeBoundary(u32 boundary)
{
    u32 candidate = ReadWriteRecordPrepareU32(FW_NEAR_UNKNOWN_0B02) + 0x91ul;

    if(candidate < boundary)
        return 1u;
    candidate = ReadWriteRecordPrepareU32(FW_NEAR_UNKNOWN_0B02) + 0x91ul;
    if(candidate < ReadWriteRecordPrepareU32(FW_HUGE_UNKNOWN_2BA59F))
        return 1u;
    return (u8)(IsCurrentWriteControllerRecordIndexMarked() == 1u);
}

/* RAM:fcca94 */
u8 DoesWriteStartRequireControllerRecordPreparation(s32 start_address, u8 alternate_policy)
{
    s32 segment = DivideWriteRecordPrepareBy1100TowardZero(start_address);
    u32 boundary;

    if(alternate_policy == 1u)
    {
        if(((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 2) & 1u) == 1u)
            WriteWriteRecordPrepareU32(FW_NEAR_UNKNOWN_0B02, MapFixedPacketLogicalToPhysicalAddress((u32)start_address));
        else
            WriteWriteRecordPrepareU32(FW_NEAR_UNKNOWN_0B02, (u32)ExpandWriteLogicalAddressForRecordOverhead(start_address));
        if(((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 2) & 1u) == 1u)
            boundary = ReadWriteRecordPrepareU32(FW_HUGE_UNKNOWN_2BAE2F);
        else
            boundary = ReadWriteRecordPrepareU32(FW_HUGE_UNKNOWN_2BAE33 + (u32)(u8)segment * 4ul);
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
    {
        boundary = ReadWriteRecordPrepareU32(FW_HUGE_UNKNOWN_2BAE2F);
    }
    else
    {
        boundary = ReadWriteRecordPrepareU32(FW_HUGE_UNKNOWN_2BAE33 + (u32)(u8)segment * 4ul);
    }

    return IsMappedWriteStartBeforeBoundary(boundary);
}
