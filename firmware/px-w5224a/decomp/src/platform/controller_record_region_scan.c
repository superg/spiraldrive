#include "controller_record_region_scan.h"
#include "arithmetic.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_region_scan_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRegionScanU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRegionScanU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static s32 DivideRegionScanBy32TowardZero(s32 value)
{
    if(value >= 0l)
        return value / 32l;
    return -(s32)(((u32)(-(value + 1l)) + 1ul) / 32ul);
}

static s32 ConvertSecondaryRegionScanPosition(s32 position)
{
    s32 product = (s32)MultiplyU32Low(39ul, (u32)position);

    return DivideRegionScanBy32TowardZero(product) + 5l;
}

static u8 IsRetryControllerRecordStatus(u8 status)
{
    return (u8)(status == 0x11u || status == 0xfeu || status == 0xffu || status == 0x0fu);
}

/* RAM:fcdc24 */
u8 PLEXTOR_NEAR_FUNCTION ScanControllerRecordRegionGridAndCachePages(u8 fallback_scan)
{
    u8 maximum_bank;
    u8 maximum_page;
    u8 accepted_segment_count = 0u;
    u8 bank;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA588) == 0x40u)
    {
        maximum_bank = 7u;
        maximum_page = 7u;
    }
    else
    {
        maximum_bank = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA588) >> 3);
        maximum_page = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA588) & 7u) - 1u);
    }

    for(bank = 0u; bank != 8u; ++bank)
    {
        u8 scan_offset = 0u;
        u8 page = 0u;
        u8 bank_complete = 0u;
        u8 use_secondary_position = 0u;
        u8 probe_limit = 2u;
        u8 copy_marker = 0u;

        for(;;)
        {
            s32 base_position;
            u8 probe_index = 0u;
            u8 page_accepted = 0u;

            if(fallback_scan == 1u)
                base_position = (s32)(-214l - (s32)((u16)bank << 5));
            else if(use_secondary_position != 0u)
                base_position = ConvertSecondaryRegionScanPosition((s32)ReadRegionScanU32(FW_NEAR_UNKNOWN_FB2C + (u16)bank * 5u));
            else
                base_position = (s32)ReadRegionScanU32(FW_NEAR_UNKNOWN_FB04 + (u16)bank * 5u);

            if(base_position == -1l)
                break;
            if(scan_offset > 0x20u)
            {
                if(use_secondary_position != 0u || fallback_scan == 1u)
                    return 0u;
                base_position = ConvertSecondaryRegionScanPosition((s32)ReadRegionScanU32(FW_NEAR_UNKNOWN_FB2C + (u16)bank * 5u));
                scan_offset = 0u;
                use_secondary_position = 1u;
                probe_limit = 2u;
                copy_marker = 1u;
            }
            if(bank_complete == 1u)
                break;

            while(page_accepted != 1u)
            {
                s32 frame_position;
                u8 status;
                u8 returned_region;

                if(probe_index > probe_limit)
                    break;
                frame_position = (s32)(ReadRegionScanU32(FW_NEAR_UNKNOWN_DD3D) + (u32)base_position + (u32)scan_offset);
                status = ProbeControllerRecordAtFrameAndMapStatus(frame_position);
                if(status == 5u)
                    return 0u;
                if(IsRetryControllerRecordStatus(status) != 0u)
                {
                    probe_index = (u8)(probe_limit + 1u);
                    if(use_secondary_position == 0u && status == 0xffu)
                        FirmwareWrite8(FW_HUGE_UNKNOWN_00FB03 + (u16)bank * 5u, 1u);
                    if(scan_offset > 8u && page == 0u)
                        scan_offset = 0x20u;
                    continue;
                }

                returned_region = FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB91C);
                if((u8)(returned_region >> 4) != bank || (u8)(returned_region & 0x0fu) != page)
                {
                    ++probe_index;
                    continue;
                }

                if((bank < maximum_bank || (bank == maximum_bank && page <= maximum_page)) && FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB923) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB924) == 0u)
                    ++accepted_segment_count;

                if(page == 7u)
                {
                    bank_complete = 1u;
                    if(FirmwareRead8(FW_HUGE_UNKNOWN_00FB03 + (u16)bank * 5u) != 0u)
                    {
                        WriteRegionScanU32(FW_NEAR_UNKNOWN_FB04 + (u16)bank * 5u, ReadRegionScanU32(FW_HUGE_UNKNOWN_2BA577) - 0x20ul);
                        WriteRegionScanU32(FW_HUGE_UNKNOWN_2BA577, ReadRegionScanU32(FW_NEAR_UNKNOWN_FB04 + (u16)bank * 5u));
                        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 1u));
                    }
                }
                page_accepted = 1u;
            }

            if(page_accepted != 1u)
            {
                ++scan_offset;
                continue;
            }
            COPY_CONTROLLER_RECORD_REGION_PAGE(FW_HUGE_UNKNOWN_2BB919, bank, page, copy_marker);
            ++page;
            if(page == 4u && scan_offset == 3u)
                scan_offset = 7u;
            ++scan_offset;
        }
    }

    if(accepted_segment_count < FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA588))
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfdu));
    else
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 2u));
    return 1u;
}
