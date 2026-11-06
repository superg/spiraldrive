#include "vendor_ec_record_list.h"
#include "../platform/firmware_memory.h"

#define VENDOR_EC_RESPONSE FW_NEAR_UNKNOWN_0B2A
#define VENDOR_EC_PRIMARY_TABLE PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8C54)
#define VENDOR_EC_ALTERNATE_TABLE PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8C86)

static u32 ReadVendorEcU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteVendorEcU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void AppendVendorEcRecordPayload(fw_addr_t record, u8 eligibility_mask, u8 transform_record_type, u16 *response_length)
{
    u8 payload_index;

    for(payload_index = 1u; payload_index < 5u; ++payload_index)
    {
        u8 value = FirmwareRead8(record + payload_index);

        if(payload_index == 2u && transform_record_type != 0u)
            value |= eligibility_mask;
        FirmwareWrite8(VENDOR_EC_RESPONSE + *response_length, value);
        ++*response_length;
    }
}

static u8 GetVendorEcPrimaryEligibility(fw_addr_t record, u8 eligibility_mask, u8 media_type_mask, u8 capacity_override, u8 include_all_records)
{
    u8 record_flags = FirmwareRead8(record);
    u8 eligibility = 0u;
    u8 selected;

    if((record_flags & 0x80u) != 0u)
        eligibility |= 2u;
    if((record_flags & 0x40u) != 0u)
        eligibility |= 4u;
    if((FirmwareRead8(record + 2u) & eligibility_mask) != 0u)
        eligibility |= 1u;
    if((eligibility & 1u) != 0u && eligibility_mask == 0x40u)
    {
        eligibility &= 0xfeu;
        if((media_type_mask & record_flags) != 0u)
            eligibility |= 1u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        selected = (u8)((eligibility >> 1) & 1u);
        eligibility = (u8)((eligibility & 0xf6u) | selected | (u8)(selected << 3));
    }
    else if(capacity_override != 0u)
    {
        selected = (u8)((eligibility >> 2) & 1u);
        eligibility = (u8)((eligibility & 0xf6u) | selected | (u8)(selected << 3));
    }

    if(include_all_records != 0u)
        eligibility |= 1u;
    return eligibility;
}

/* RAM:fcace5 */
u16 PLEXTOR_NEAR_FUNCTION BuildVendorEcRecordListResponse(u8 include_all_records, u8 use_alternate_table_format)
{
    fw_addr_t table = use_alternate_table_format != 0u ? VENDOR_EC_ALTERNATE_TABLE : VENDOR_EC_PRIMARY_TABLE;
    u8 capacity_override = 0u;
    u8 media_type_mask = 0u;
    u8 eligibility_mask = 0u;
    u16 response_length = 4u;
    u8 record_index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        FirmwareWrite8(VENDOR_EC_RESPONSE + 2u, 0u);
        include_all_records = 1u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        FirmwareWrite8(VENDOR_EC_RESPONSE + 2u, 2u);
        eligibility_mask = 0x80u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u && ReadVendorEcU32(FW_NEAR_CURRENT_FRAME_POSITION) > 0x00020f58ul && ReadVendorEcU32(FW_NEAR_CURRENT_FRAME_POSITION) < 0x0004ce78ul)
            capacity_override = 0x40u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWrite8(VENDOR_EC_RESPONSE + 2u, 3u);
        eligibility_mask = 0x40u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
        {
            media_type_mask = 4u;
        }
        else
        {
            media_type_mask = (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) != 0u ? 2u : 1u;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 2u)
                media_type_mask = 0u;
        }
    }
    else
    {
        FirmwareWrite8(VENDOR_EC_RESPONSE + 2u, 1u);
        eligibility_mask = 0x80u;
        if(use_alternate_table_format == 0u)
            include_all_records = 1u;
    }

    include_all_records = (u8)(include_all_records != 0u);
    FirmwareWrite8(VENDOR_EC_RESPONSE + 3u, include_all_records);
    for(record_index = 0u; record_index < FirmwareRead8(table); ++record_index)
    {
        fw_addr_t record = table + 5u + (u16)record_index * 5u;

        if(use_alternate_table_format != 0u)
        {
            if(include_all_records != 0u || (FirmwareRead8(record + 2u) & eligibility_mask) != 0u)
                AppendVendorEcRecordPayload(record, eligibility_mask, 0u, &response_length);
        }
        else
        {
            u8 eligibility = GetVendorEcPrimaryEligibility(record, eligibility_mask, media_type_mask, capacity_override, include_all_records);

            if((eligibility & 1u) != 0u)
                AppendVendorEcRecordPayload(record, eligibility_mask, (u8)((eligibility & 8u) != 0u), &response_length);
        }
    }

    WriteVendorEcU16(VENDOR_EC_RESPONSE, (u16)(response_length - 2u));
    return response_length;
}
