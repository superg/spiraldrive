#include "raw_read_toc_summary.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_toc_summary_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 RawReadTocRecordAddress(u16 record_index)
{
    return (u16)(0xb4bdu + (u16)(record_index << 3));
}

static void PublishRawReadTocLeadOut(u16 record_index)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_toc_summary_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    u16 record_address = RawReadTocRecordAddress(record_index);

    FirmwareWriteNear8(tuple_address, FirmwareReadNear8((u16)(record_address + 4u)));
    FirmwareWriteNear8((u16)(tuple_address + 1u), FirmwareReadNear8((u16)(record_address + 6u)));
    FirmwareWriteNear8((u16)(tuple_address + 2u), FirmwareReadNear8((u16)(record_address + 7u)));
    RAW_TOC_CONVERT(tuple_address);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1406, FirmwareReadNear8(tuple_address));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1407, FirmwareReadNear8((u16)(tuple_address + 1u)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1408, FirmwareReadNear8((u16)(tuple_address + 2u)));
}

/* RAM:fe13eb */
void PLEXTOR_NEAR_FUNCTION InitializeRawReadTocSummaryFromTrackTable(void)
{
    u16 record_index = 0u;
    u16 record_address;
    u8 track_number;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u)
        return;

    for(;;)
    {
        record_address = RawReadTocRecordAddress(record_index);
        track_number = FirmwareReadNear8((u16)(record_address + 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1400, track_number);
        if(track_number != 0u && (FirmwareReadNear8(record_address) & 0x0fu) == 1u)
            break;
        record_index = (u16)(record_index + 1u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A) == 0x10u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1402, 0x10u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1400, 2u);
        while(track_number != 0xaau)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1401, track_number);
            if((FirmwareReadNear8(record_address) & 0x40u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1400, (u8)(track_number + 1u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1401, (u8)(track_number + 1u));
            }
            record_index = (u16)(record_index + 1u);
            record_address = RawReadTocRecordAddress(record_index);
            track_number = FirmwareReadNear8((u16)(record_address + 1u));
        }
    }
    else
    {
        if((FirmwareReadNear8((u16)(record_address + 3u)) & 0x30u) == 0x20u)
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A) == 0x10u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1402, 0x10u);
            else
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1402, 0x20u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1402, 0u);
        while(track_number != 0xaau)
        {
            record_index = (u16)(record_index + 1u);
            record_address = RawReadTocRecordAddress(record_index);
            track_number = FirmwareReadNear8((u16)(record_address + 1u));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1401, FirmwareReadNear8((u16)(RawReadTocRecordAddress((u16)(record_index - 1u)) + 1u)));
    }

    PublishRawReadTocLeadOut(record_index);
}
