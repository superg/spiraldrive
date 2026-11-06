#include "toc_session_removal.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/toc_session_removal_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRemovedTocWord(u16 address)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/toc_session_removal_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteRemovedTocWord(u16 address, u16 value)
{
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/toc_session_removal_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteRemovedTocDword(u16 address, u32 value)
{
#define PLEXTOR_BINDING_SECTION 4
#include "../platform/bindings/media/toc_session_removal_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static fw_addr_t RemovedTocDescriptorAddress(u16 descriptor_index)
{
    return FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)(descriptor_index * 11u);
}

static fw_addr_t RemovedTocSummaryAddress(u8 point)
{
    return FW_NEAR_UNKNOWN_F2B6 + (fw_addr_t)((u16)point * 6u);
}

static void AddRemovedTocSummaryFlags(u8 flags)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | flags));
}

static void CopyRemovedTocTrackSummary(fw_addr_t descriptor, u8 point)
{
    fw_addr_t summary = RemovedTocSummaryAddress(point);

    FirmwareWrite8(summary, point);
    FirmwareWrite8(summary + 1u, FirmwareRead8(descriptor + 1u));
    FirmwareWrite8(summary + 2u, 0u);
    FirmwareWrite8(summary + 3u, FirmwareRead8(descriptor + 8u));
    FirmwareWrite8(summary + 4u, FirmwareRead8(descriptor + 9u));
    FirmwareWrite8(summary + 5u, FirmwareRead8(descriptor + 10u));
    AddRemovedTocSummaryFlags((FirmwareRead8(descriptor + 1u) & 0x40u) == 0u ? 1u : 2u);
}

static void RebuildRemovedTocAdr1Descriptor(fw_addr_t descriptor)
{
    u8 point = FirmwareRead8(descriptor + 3u);

    if(point == 0xa0u)
    {
        REMOVE_TOC_COPY_TEN(descriptor + 1u, FW_NEAR_UNKNOWN_F532);
        if(FirmwareRead8(descriptor + 9u) == 0x10u)
        {
            fw_addr_t summary = RemovedTocSummaryAddress(1u);

            FirmwareWrite8(summary, 1u);
            FirmwareWrite8(summary + 1u, 0x41u);
            FirmwareWrite8(summary + 3u, 0u);
            FirmwareWrite8(summary + 4u, 2u);
            FirmwareWrite8(summary + 5u, 0u);
            AddRemovedTocSummaryFlags(2u);
            AddRemovedTocSummaryFlags(8u);
        }
    }
    if(point == 0xa1u)
    {
        REMOVE_TOC_COPY_TEN(descriptor + 1u, FW_NEAR_UNKNOWN_F53C);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, FirmwareReadNear8(FW_NEAR_UNKNOWN_F543));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F53A) == 0x10u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F53C) & 0x40u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, 1u);
    }
    if(point == 0xa2u)
    {
        s32 lead_out = REMOVE_TOC_CONVERT_TUPLE(FirmwareRead8(descriptor + 8u), FirmwareRead8(descriptor + 9u), FirmwareRead8(descriptor + 10u));

        WriteRemovedTocDword(FW_NEAR_UNKNOWN_12C1, (u32)lead_out);
        REMOVE_TOC_COPY_TEN(descriptor + 1u, FW_NEAR_UNKNOWN_F546);
    }
    if(point < 100u)
        CopyRemovedTocTrackSummary(descriptor, point);
}

static void RebuildRemovedTocAdr5Descriptor(fw_addr_t descriptor)
{
    u8 point = FirmwareRead8(descriptor + 3u);

    if(point == 0xb2u)
    {
        REMOVE_TOC_COPY_TEN(descriptor + 1u, FW_NEAR_UNKNOWN_14E4);
        REMOVE_TOC_COPY_PAYLOAD(FW_NEAR_UNKNOWN_14E4, FW_NEAR_UNKNOWN_F7A6);
    }
    if(point == 0xb3u)
    {
        REMOVE_TOC_COPY_TEN(descriptor + 1u, FW_NEAR_UNKNOWN_14EE);
        REMOVE_TOC_COPY_PAYLOAD(FW_NEAR_UNKNOWN_14EE, FW_NEAR_UNKNOWN_F7B0);
    }
    if(point == 0xb4u)
    {
        REMOVE_TOC_COPY_TEN(descriptor + 1u, FW_NEAR_UNKNOWN_14F8);
        REMOVE_TOC_COPY_PAYLOAD(FW_NEAR_UNKNOWN_14F8, FW_NEAR_UNKNOWN_F7BA);
    }
    if(point != 0u && point <= 0x40u)
    {
        u8 payload_index = REMOVE_TOC_BCD_TO_BINARY(point);

        REMOVE_TOC_COPY_PAYLOAD(descriptor + 1u, FW_NEAR_UNKNOWN_F79C + (fw_addr_t)((u16)payload_index * 10u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1C) + 1u));
    }
}

static void WriteRemovedTocLeadOutSummary(void)
{
    u8 point_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u);
    fw_addr_t summary = RemovedTocSummaryAddress(point_index);

    FirmwareWrite8(summary, 0xaau);
    FirmwareWrite8(summary + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F546));
    FirmwareWrite8(summary + 2u, 0u);
    FirmwareWrite8(summary + 3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F54D));
    FirmwareWrite8(summary + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F54E));
    FirmwareWrite8(summary + 5u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F54F));
}

/* RAM:fad340 */
void RemoveLatestFullTocSessionAndRebuildState(void)
{
    u16 descriptor_index;
    u16 bytes_used;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) < 2u)
    {
        REMOVE_TOC_RESET_ALL();
        return;
    }

    REMOVE_TOC_RESET_SUMMARIES();
    descriptor_index = 1u;
    bytes_used = 0u;
    WriteRemovedTocWord(FW_NEAR_UNKNOWN_12DC, descriptor_index);
    WriteRemovedTocWord(FW_NEAR_UNKNOWN_12CA, bytes_used);

    while(FirmwareRead8(RemovedTocDescriptorAddress(descriptor_index)) != FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9))
    {
        fw_addr_t descriptor = RemovedTocDescriptorAddress(descriptor_index);
        u8 adr;

        if(FirmwareRead8(descriptor) > 1u)
            AddRemovedTocSummaryFlags(4u);
        adr = (u8)(FirmwareRead8(descriptor + 1u) & 0x0fu);
        if(adr == 1u)
            RebuildRemovedTocAdr1Descriptor(descriptor);
        if(adr == 5u)
            RebuildRemovedTocAdr5Descriptor(descriptor);

        descriptor_index = (u16)(descriptor_index + 1u);
        bytes_used = (u16)(bytes_used + 11u);
        WriteRemovedTocWord(FW_NEAR_UNKNOWN_12DC, descriptor_index);
        WriteRemovedTocWord(FW_NEAR_UNKNOWN_12CA, bytes_used);
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) == 1u && REMOVE_TOC_CONVERT_ADDRESS(FW_NEAR_UNKNOWN_F2BF) >= 2250l)
        AddRemovedTocSummaryFlags(0x10u);

    WriteRemovedTocLeadOutSummary();
    for(descriptor_index = ReadRemovedTocWord(FW_NEAR_UNKNOWN_12DC); descriptor_index < 500u; ++descriptor_index)
    {
        fw_addr_t descriptor = RemovedTocDescriptorAddress(descriptor_index);
        u8 field;

        for(field = 0u; field < 11u; ++field)
            FirmwareWrite8(descriptor + field, 0u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) - 1u));
}
