#include "mip_controller_record_snapshot.h"
#include "firmware_memory.h"
#include "mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/mip_controller_record_snapshot_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadSnapshotU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteSnapshotU32LowWordFirst(fw_addr_t address, u32 value)
{
    FirmwareWriteU16(FirmwareAddressIncrement24(address, 2u), (u16)value);
    FirmwareWriteU16(address, (u16)(value >> 16));
}

static void WriteFreshU32Bytes(fw_addr_t destination, fw_addr_t source)
{
    FirmwareWrite8(destination, (u8)(ReadSnapshotU32(source) >> 24));
    FirmwareWrite8(destination + 1u, (u8)(ReadSnapshotU32(source) >> 16));
    FirmwareWrite8(destination + 2u, (u8)(ReadSnapshotU32(source) >> 8));
    FirmwareWrite8(destination + 3u, (u8)ReadSnapshotU32(source));
}

static void WriteFreshLow24(fw_addr_t destination, fw_addr_t source)
{
    FirmwareWrite8(destination, (u8)(ReadSnapshotU32(source) >> 16));
    FirmwareWrite8(destination + 1u, (u8)(ReadSnapshotU32(source) >> 8));
    FirmwareWrite8(destination + 2u, (u8)ReadSnapshotU32(source));
}

static void WriteFreshPacked20(fw_addr_t destination, fw_addr_t source)
{
    FirmwareWrite8(destination, (u8)((ReadSnapshotU32(source) >> 16) & 0x0fu));
    FirmwareWrite8(destination + 1u, (u8)(ReadSnapshotU32(source) >> 8));
    FirmwareWrite8(destination + 2u, (u8)ReadSnapshotU32(source));
}

/* RAM:fa9a16 */
void BuildMipControllerRecordSnapshotBlock(void)
{
    const fw_addr_t snapshot = FW_HUGE_UNKNOWN_2BA5DF;
    u32 derived_position;
    u32 mapped_position;
    u32 source_position;
    u8 index;

    for(index = 0u; index < 0x50u; ++index)
        FirmwareWrite8(snapshot + index, 0u);

    FirmwareWrite8(snapshot, 0x4du);
    FirmwareWrite8(snapshot + 1u, 0x49u);
    FirmwareWrite8(snapshot + 2u, 0x50u);
    FirmwareWrite8(snapshot + 3u, 1u);

    WriteFreshU32Bytes(snapshot + 8u, FW_HUGE_UNKNOWN_2BA525);
    WriteFreshLow24(snapshot + 13u, FW_HUGE_UNKNOWN_2BA529);

    FirmwareWrite8(snapshot + 17u, 0x20u);
    FirmwareWrite8(snapshot + 19u, 8u);
    FirmwareWrite8(snapshot + 21u, 0x88u);

    if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u)
        FirmwareWrite8(snapshot + 24u, 0x40u);
    else if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u)
        FirmwareWrite8(snapshot + 24u, 0x80u);
    else
        FirmwareWrite8(snapshot + 24u, 0x40u);

    derived_position = ReadSnapshotU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY);
    if(derived_position != 0ul)
    {
        mapped_position = ReadSnapshotU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION);
        derived_position = ReadSnapshotU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY);
        if(mapped_position != derived_position)
            FirmwareWrite8(snapshot + 24u, (u8)(FirmwareRead8(snapshot + 24u) | 0x10u));
    }

    WriteFreshPacked20(snapshot + 25u, FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION);

    if(ReadSnapshotU32(FW_HUGE_UNKNOWN_2BA5A3) == 0ul)
    {
        WriteSnapshotU32LowWordFirst(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, 0ul);
    }
    else
    {
        source_position = ReadSnapshotU32(FW_HUGE_UNKNOWN_2BA5A3);
        source_position = (source_position + 0xa7ul) << 5;
        WriteSnapshotU32LowWordFirst(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, (u32)DIVIDE_SNAPSHOT_POSITION((s32)source_position, 39l));
    }

    WriteFreshPacked20(snapshot + 28u, FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY);
    FirmwareWrite8(snapshot + 31u, FirmwareRead8(FW_HUGE_UNKNOWN_2BA536));

    for(index = 0u; index < 8u; ++index)
        WriteFreshPacked20(snapshot + 32u + (u16)index * 3u, FW_HUGE_UNKNOWN_2BA537 + (u16)index * 4u);
    for(index = 0u; index < 8u; ++index)
        WriteFreshPacked20(snapshot + 56u + (u16)index * 3u, FW_HUGE_UNKNOWN_2BA557 + (u16)index * 4u);
}
