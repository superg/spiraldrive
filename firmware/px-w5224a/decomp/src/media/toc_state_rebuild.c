#include "toc_state_rebuild.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/toc_state_rebuild_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void ClearRebuildTocRecord(fw_addr_t address)
{
    FirmwareWrite8(address, 0u);
    FirmwareWrite8(address + 1u, 0u);
    FirmwareWrite8(address + 2u, 0u);
    FirmwareWrite8(address + 6u, 0u);
    FirmwareWrite8(address + 5u, 0u);
    FirmwareWrite8(address + 4u, 0u);
    FirmwareWrite8(address + 3u, 0u);
    FirmwareWrite8(address + 9u, 0u);
    FirmwareWrite8(address + 8u, 0u);
    FirmwareWrite8(address + 7u, 0u);
}

static u16 ReadRebuildTocWord(u16 address)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/toc_state_rebuild_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u32 ReadRebuildTocDword(u16 address)
{
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/toc_state_rebuild_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteRebuildTocDword(u16 address, u32 value)
{
#define PLEXTOR_BINDING_SECTION 4
#include "../platform/bindings/media/toc_state_rebuild_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:facd6e */
void ResetAndRebuildTocStateFromRawReadQRecords(void)
{
    u8 record_index;
    u32 lead_in_start;
    u16 descriptor_count;
    u8 state;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) != 0xc0u)
        return;

    for(record_index = 0u; record_index < 100u; ++record_index)
        ClearRebuildTocRecord(FW_HUGE_UNKNOWN_2BB03B + (fw_addr_t)record_index * 10u);

    ClearRebuildTocRecord(FW_NEAR_UNKNOWN_2BB5DB);

    for(record_index = 0u; record_index < 22u; ++record_index)
    {
        fw_addr_t offset = (fw_addr_t)record_index * 10u;

        ClearRebuildTocRecord(FW_NEAR_UNKNOWN_2BB423 + offset);
        ClearRebuildTocRecord(FW_NEAR_UNKNOWN_2BB4FF + offset);
    }

    for(record_index = 0u; record_index < 41u; ++record_index)
    {
        fw_addr_t offset = (fw_addr_t)record_index * 10u;

        ClearRebuildTocRecord(FW_NEAR_UNKNOWN_2BB5E5 + offset);
        ClearRebuildTocRecord(FW_NEAR_UNKNOWN_2BB77F + offset);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1148, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1149, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114E, 0u);

    REBUILD_TOC_PARSE_RAW_Q();

    lead_in_start = ReadRebuildTocDword(FW_NEAR_UNKNOWN_114F);
    descriptor_count = ReadRebuildTocWord(FW_NEAR_UNKNOWN_8010);
    WriteRebuildTocDword(FW_NEAR_UNKNOWN_1143, lead_in_start - 1000ul + (u32)descriptor_count);

    descriptor_count = ReadRebuildTocWord(FW_NEAR_UNKNOWN_8010);
    if(descriptor_count == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, 0xffu);
        state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x80u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, state);
        state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xdfu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, state);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, 1u);
        state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x7fu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, state);
        state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, state);
    }
}
