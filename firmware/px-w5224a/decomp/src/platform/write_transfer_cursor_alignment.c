#include "write_transfer_cursor_alignment.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/write_transfer_cursor_alignment_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* Reconstruction helper; equivalent to the native signed divide by 32. */
static s32 DivideBy32TowardZero(s32 value)
{
    if(value >= 0l)
        return (s32)((u32)value >> 5);
    return -(s32)(((u32)(-(value + 1l)) + 1ul) >> 5);
}

/* RAM:fedd6d */
u32 AlignWriteTransferCursorForBlockCount(u32 block_count)
{
    s32 logical_address = (s32)ReadBigEndianU32(FW_HUGE_UNKNOWN_2BC703);
    s32 group = DivideBy32TowardZero(logical_address);
    s32 aligned_address = (s32)((u32)group << 5);
    u32 mapped_address;
    u32 pipeline_target;

    if(aligned_address == logical_address)
    {
        if(block_count > 0x20ul)
        {
            block_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C6);
            group = (s32)((u32)group + 1ul);
        }
    }
    else
    {
        if(DoesTransferCrossNextThirtyTwoBoundary((u32)logical_address) != 0u)
            group = (s32)((u32)group + 1ul);
        block_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C6);
    }

    aligned_address = (s32)((u32)group << 5);
    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BC703, (u32)aligned_address);
    if(((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 2) & 1u) != 0u)
        mapped_address = MAP_FIXED_PACKET((u32)aligned_address);
    else
        mapped_address = (u32)EXPAND_RECORD_OVERHEAD(aligned_address);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0B02, mapped_address);

    pipeline_target = INITIALIZE_HOST_RANGE();
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, pipeline_target);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_8000, pipeline_target);
    return block_count;
}
