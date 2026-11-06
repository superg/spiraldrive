#include "raw_track_selection.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_track_selection_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadFirmwareU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteFirmwareU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static fw_addr_t RawTrackRecordFieldAddress(u16 index, u8 offset)
{
    return (fw_addr_t)(u16)(0xb4bcu + (u16)(index << 3) + offset);
}

/* RAM:fe28de */
void PLEXTOR_NEAR_FUNCTION InitializeRawReadTrackRecordSelectionAndIdentificationState(void)
{
    u16 index;
    u8 first_track_pending = 1u;
    u8 first_type2_pending = 1u;
    u8 first_type3_pending = 1u;

    InitializeTrackSelectionState();
    for(index = 0u; index < ReadFirmwareU16(FW_NEAR_UNKNOWN_B4BB); ++index)
    {
        u8 record_type = (u8)(FirmwareRead8(RawTrackRecordFieldAddress(index, 1u)) & 0x0fu);

        if(record_type == 1u)
        {
            u8 track_number = FirmwareRead8(RawTrackRecordFieldAddress(index, 2u));

            if(track_number == 0u)
                WriteFirmwareU16(FW_NEAR_UNKNOWN_149D, index);
            if(track_number == 0xaau)
                WriteFirmwareU16(FW_NEAR_UNKNOWN_149F, index);
            if(first_track_pending == 1u && track_number != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A5, track_number);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A8, track_number);
                first_track_pending = 0u;
            }
        }
        if(first_type2_pending == 1u && record_type == 2u)
        {
            WriteFirmwareU16(FW_NEAR_UNKNOWN_14A1, index);
            first_type2_pending = 0u;
        }
        if(first_type3_pending == 1u && record_type == 3u)
        {
            WriteFirmwareU16(FW_NEAR_UNKNOWN_14A3, index);
            first_type3_pending = 0u;
        }
        if((FirmwareRead8(RawTrackRecordFieldAddress(index, 4u)) & 0x40u) == 0x40u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A6, 1u);
    }

    if(ReadFirmwareU16(FW_NEAR_UNKNOWN_14A1) == 0u)
        LoadSelectedMediaCatalog();
    if(ReadFirmwareU16(FW_NEAR_UNKNOWN_14A3) != 0u)
        LoadSelectedIsrc();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AC, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AD, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AE, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A7, 0u);
}
