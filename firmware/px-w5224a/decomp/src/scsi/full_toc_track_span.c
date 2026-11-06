#include "full_toc_track_span.h"
#include "platform/encoding.h"
#include "platform/firmware_memory.h"
#include "platform/memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/full_toc_track_span_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadFullTocDescriptorBytesUsed(void)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/full_toc_track_span_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static fw_addr_t FullTocSpanDescriptorAddress(u16 descriptor_index)
{
    return FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)(descriptor_index * 11u);
}

/* RAM:faf635 */
s32 PLEXTOR_NEAR_FUNCTION ComputeFullTocTrackSpanFrames(u8 track_number)
{
    u16 descriptor_count = (u16)(ReadFullTocDescriptorBytesUsed() / 11u);
    u16 descriptor_index;
    u8 session_number = 0u;
    u8 last_track_number = 0u;
    s32 lead_out_position = 0l;
    s32 start_position;
    s32 end_position;

    for(descriptor_index = 1u; descriptor_index <= descriptor_count; ++descriptor_index)
    {
        fw_addr_t descriptor_address = FullTocSpanDescriptorAddress(descriptor_index);

        if(FirmwareRead8(descriptor_address + 3u) == track_number)
        {
            session_number = FirmwareRead8(descriptor_address);
            break;
        }
    }

    for(descriptor_index = 1u; descriptor_index <= descriptor_count; ++descriptor_index)
    {
        fw_addr_t descriptor_address = FullTocSpanDescriptorAddress(descriptor_index);

        if(FirmwareRead8(descriptor_address) != session_number)
            continue;
        if(FirmwareRead8(descriptor_address + 3u) == 0xa1u)
        {
            last_track_number = FirmwareRead8(descriptor_address + 8u);
        }
        else if(FirmwareRead8(descriptor_address + 3u) == 0xa2u)
        {
            lead_out_position = CopyAndConvertFullTocLeadOut(descriptor_address + 8u);
            break;
        }
    }

    start_position = ConvertFullTocSpanMsf(FW_NEAR_UNKNOWN_F2B9 + (fw_addr_t)((u16)track_number * 6u));
    if(track_number == last_track_number)
        return (s32)((u32)lead_out_position - (u32)start_position);

    end_position = ConvertFullTocSpanMsf(FW_NEAR_UNKNOWN_F2BF + (fw_addr_t)((u16)track_number * 6u));
    return (s32)((u32)end_position - (u32)start_position);
}
