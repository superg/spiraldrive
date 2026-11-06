#include "get_performance.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "spc.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/get_performance_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadGetPerformanceCommandWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadGetPerformanceCommandDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteGetPerformanceCommandDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u16 AppendFilteredGetPerformanceTable(u16 descriptor_limit, u8 filter_kind, u8 filter_mask)
{
    const fw_addr_t table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8C54);
    u16 response_length = 8u;
    u16 selected_count = 0u;
    u8 record_count = FirmwareRead8(table);
    u8 index;

    for(index = 0u; index < record_count; ++index)
    {
        u8 record_offset = (u8)(index * 5u + 5u);
        fw_addr_t record = table + record_offset;
        u8 selected = 0u;

        if(filter_kind == 0u)
            selected = (u8)((FirmwareRead8(record + 2u) & 0x80u) != 0u);
        else if(filter_kind == 1u)
            selected = (u8)((FirmwareRead8(record) & filter_mask) != 0u);

        if(selected != 0u)
        {
            u16 inherited_wr14 = (u16)(((u16)record_offset << 8) | index);

            response_length = GET_PERFORMANCE_APPEND_TABLE(record, response_length, inherited_wr14);
            ++selected_count;
        }
        if(selected_count == descriptor_limit)
            break;
    }
    return response_length;
}

/* RAM:fc036c */
void HandleGetPerformanceCommand(void)
{
    u16 response_length = 8u;
    u16 descriptor_limit;
    u8 response_type = 0u;
    u8 unsupported = 0u;
    u8 media_available;
    u8 request_type;

    if(GET_PERFORMANCE_VALIDATE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8EAD)) == 0u)
    {
        GET_PERFORMANCE_UNSUPPORTED();
        return;
    }

    media_available = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u);
    descriptor_limit = ReadGetPerformanceCommandWord(FW_NEAR_UNKNOWN_0B12);
    request_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14);

    if(request_type == 3u)
    {
        if(descriptor_limit != 0u)
        {
            u8 media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);

            if(media_available == 0u || media_state == 0u || media_state > 2u)
            {
                response_length = AppendFilteredGetPerformanceTable(descriptor_limit, 0u, 0u);
            }
            else if(media_state == 1u)
            {
                u8 filter_kind;
                u8 filter_mask;

                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u)
                {
                    filter_kind = 0u;
                    filter_mask = 0u;
                }
                else
                {
                    s32 lead_out = (s32)ReadGetPerformanceCommandDword(FW_NEAR_CURRENT_FRAME_POSITION);

                    if(lead_out >= 0x20f59l && lead_out <= 0x4ce77l)
                    {
                        filter_kind = 1u;
                        filter_mask = 0x40u;
                    }
                    else
                    {
                        filter_kind = 0u;
                        filter_mask = 0u;
                    }
                }
                response_length = AppendFilteredGetPerformanceTable(descriptor_limit, filter_kind, filter_mask);
            }
            else
            {
                u8 filter_mask;

                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
                    filter_mask = 4u;
                else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 2u)
                    filter_mask = 0u;
                else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 4u)
                    filter_mask = 1u;
                else
                    filter_mask = 2u;
                response_length = AppendFilteredGetPerformanceTable(descriptor_limit, 1u, filter_mask);
            }
        }
    }
    else if(request_type == 0u)
    {
        u8 selector = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 7u);

        if(selector == 0u)
        {
            u8 speed = media_available != 0u ? FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F) : FirmwareReadNear8(FW_NEAR_UNKNOWN_0C43);

            response_length = GET_PERFORMANCE_APPEND_NOMINAL(speed, descriptor_limit, response_length);
        }
        else if(selector == 4u)
        {
            u8 speed;

            response_type = 2u;
            if(media_available == 0u)
            {
                speed = 52u;
            }
            else
            {
                u8 media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);

                if(media_state == 1u)
                    speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u ? 52u : 8u;
                else if(media_state == 2u)
                {
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
                        speed = 24u;
                    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) != 0u)
                        speed = 10u;
                    else
                        speed = 4u;
                }
                else
                    speed = 52u;
            }
            response_length = GET_PERFORMANCE_APPEND_WRITE(speed, response_length, descriptor_limit);
        }
        else
        {
            unsupported = 5u;
        }
    }
    else
    {
        unsupported = 5u;
    }

    if(unsupported != 0u)
    {
        GET_PERFORMANCE_UNSUPPORTED();
        return;
    }

    WriteGetPerformanceCommandDword(FW_NEAR_UNKNOWN_0B2A, (u32)(u16)(response_length - 4u));
    WriteGetPerformanceCommandDword(FW_NEAR_UNKNOWN_0B2E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, response_type);
    if(GET_PERFORMANCE_TRANSFER(FW_NEAR_UNKNOWN_0B2A, response_length) != 4u)
        GET_PERFORMANCE_NO_SENSE();
}
