#include "arithmetic.h"
#include "controller_buffer_transfer.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/write_controller_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadWriteRangeWord(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteWriteRangeWord(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fab26e */
u8 SelectWriteControllerBufferRangeEndIndex(u8 start_index, u8 record_count)
{
    u8 index = start_index;
    s32 first_position = (s32)ReadWriteRangeWord(FW_HUGE_UNKNOWN_2B9600 + (fw_addr_t)start_index * 8u);
    s32 block_start = DivideWriteRangePosition(first_position - 150l, 39l);

    block_start = (s32)MultiplyU32Low((u32)block_start, 39ul) + 150l;
    WriteWriteRangeWord(FW_HUGE_UNKNOWN_2BA5BC, (u32)block_start);

    for(;;)
    {
        s32 position = (s32)ReadWriteRangeWord(FW_HUGE_UNKNOWN_2B9600 + (fw_addr_t)index * 8u);

        if((u32)position >= (u32)block_start + 32ul)
        {
            FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CF, 1u);
            return (u8)(index - 1u);
        }
        if((s16)(u16)index >= (s16)((u16)record_count - 1u))
            return index;
        ++index;
    }
}

/* RAM:fab306 */
void SeedSelectedWriteControllerBufferRangeAnchors(void) PLEXTOR_REENTRANT
{
    u8 index = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CD);
    u16 exclusive_end = (u16)FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CE) + 1u;

    while((u16)index < exclusive_end)
    {
        fw_addr_t record_address = FW_HUGE_UNKNOWN_2B9600 + (fw_addr_t)index * 8u;
        u32 frame_position = ReadWriteRangeWord(record_address);
        u8 frame_offset = (u8)(frame_position - ReadWriteRangeWord(FW_HUGE_UNKNOWN_2BA5BC));
        u8 buffer_offset = (u8)(ReadWriteRangeWord(record_address + 4u) - ReadWriteRangeWord(FW_HUGE_UNKNOWN_2BA5B4));
        u32 anchor_controller_address = MultiplyU32Low((u32)frame_offset + 5ul, 0x0930ul);
        u32 source_controller_address = ReadWriteRangeWord(FW_NEAR_UNKNOWN_1572) + MultiplyU32Low((u32)buffer_offset, 0x0930ul);
        u8 anchor[4];

        TransferControllerBufferSectorsAndWait(source_controller_address, anchor_controller_address, 1u);
        ConvertFramePositionToBinaryMsfTuple((s32)frame_position, (fw_addr_t)(u16)anchor);
        ConvertThreeBinaryBytesToPackedBcd((u16)anchor);
        anchor[3] = 2u;
        CopyBytesToAddressPlus1MiB(anchor_controller_address + 12ul, (fw_addr_t)(u16)anchor, 4u);
        ++index;
    }
}
