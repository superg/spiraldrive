#include "raw_sector_capture.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/raw_sector_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRawSectorCaptureU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void SelectCaptureForQType(u8 q_type, u8 *capture_mode, u16 *transfer_length)
{
    switch(q_type)
    {
    case 0u:
    case 1u:
    case 2u:
    case 3u:
        *capture_mode = 0xcbu;
        *transfer_length = 0x0930u;
        break;
    case 8u:
        *capture_mode = 0xc2u;
        *transfer_length = 0x0930u;
        break;
    case 9u:
        *capture_mode = 0xceu;
        *transfer_length = 0x0930u;
        break;
    case 10u:
    case 11u:
    case 12u:
    case 13u:
        *capture_mode = 0xc3u;
        *transfer_length = 0x0930u;
        break;
    default:
        break;
    }
}

static void SelectCaptureForTrackCode(u8 track_code, u8 *capture_mode, u16 *transfer_length)
{
    switch(track_code)
    {
    case 0x00u:
    case 0x01u:
        *capture_mode = 0xc2u;
        *transfer_length = 0x0930u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) != 0x41u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 0u);
        break;
    case 0x10u:
    case 0x12u:
        *capture_mode = 0xc2u;
        *transfer_length = 0x0800u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
        break;
    case 0x11u:
    case 0x13u:
        *capture_mode = 0xc2u;
        *transfer_length = 0x0930u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
        break;
    case 0x20u:
    case 0x22u:
        *capture_mode = 0xc3u;
        *transfer_length = 0x0920u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 13u);
        break;
    case 0x21u:
    case 0x23u:
        *capture_mode = 0xc3u;
        *transfer_length = 0x0930u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 13u);
        break;
    case 0x24u:
        *capture_mode = 0xc3u;
        *transfer_length = 0x0914u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 13u);
        break;
    case 0x30u:
    case 0x32u:
        *capture_mode = 0xceu;
        *transfer_length = 0x0920u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 9u);
        break;
    case 0x31u:
    case 0x33u:
    case 0x34u:
        *capture_mode = 0xceu;
        *transfer_length = 0x0930u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 9u);
        break;
    default:
        break;
    }
}

/* RAM:feb355 */
void PLEXTOR_NEAR_FUNCTION SelectRawSectorCaptureModeAndLength(u16 *transfer_length, u8 *capture_mode, u8 raw_sector_source_kind)
{
    u16 current_track_index;
    u16 selected_track_index;
    u8 q_type;
    u8 track_code;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1283) != 2u)
    {
        switch(raw_sector_source_kind)
        {
        case 7u:
            q_type = (u8)((FirmwareRead8(FW_HUGE_UNKNOWN_2BC139) & 0x40u) != 0u ? 8u : 0u);
            break;
        case 8u:
            q_type = (u8)((FirmwareRead8(FW_HUGE_UNKNOWN_2BC11B + (fw_addr_t)(FirmwareRead8(FW_HUGE_UNKNOWN_2BC119) - 1u) * 10u) & 0x40u) != 0u ? 8u : 0u);
            break;
        case 10u:
            q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);
            break;
        case 5u:
            q_type = MapWorkingTrackFieldsToStatusCode();
            break;
        default:
            q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
            break;
        }
        SelectCaptureForQType(q_type, capture_mode, transfer_length);
        return;
    }

    current_track_index = ReadRawSectorCaptureU16(FW_NEAR_UNKNOWN_149B);
    if(ReadRawSectorCaptureU16(FW_NEAR_UNKNOWN_149D) == current_track_index)
    {
        selected_track_index = (u16)(current_track_index + 1u);
        while((FirmwareRead8(FW_HUGE_UNKNOWN_00B4BD + (fw_addr_t)selected_track_index * 8u) & 0x0fu) != 1u)
            selected_track_index++;
    }
    else
    {
        selected_track_index = (u16)(current_track_index - 1u);
        while((FirmwareRead8(FW_HUGE_UNKNOWN_00B4BD + (fw_addr_t)selected_track_index * 8u) & 0x0fu) != 1u)
            selected_track_index--;
    }

    if(raw_sector_source_kind == 10u)
    {
        *capture_mode = 0xc2u;
        *transfer_length = 0x0800u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
        return;
    }

    track_code = (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_00B4C0 + (fw_addr_t)selected_track_index * 8u) & 0x3fu);
    SelectCaptureForTrackCode(track_code, capture_mode, transfer_length);
    if(ReadRawSectorCaptureU16(FW_NEAR_UNKNOWN_0C71) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 13u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C73, 0x20u);
}

/* RAM:feb1ed */
u8 CaptureRawSectorToControllerBufferAndWait(fw_addr_t controller_buffer_address, u8 raw_sector_source_kind)
{
    u8 capture_mode;
    u16 transfer_length;
    u8 control_shadow;

    SELECT_CAPTURE_MODE_AND_LENGTH(&capture_mode, &transfer_length, raw_sector_source_kind);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0xe0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, capture_mode);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)controller_buffer_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(controller_buffer_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(controller_buffer_address >> 16));

    control_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, control_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, control_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x80u);
    while(RAW_SECTOR_CAPTURE_COMPLETE() == 0u)
    {
    }

    control_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xbfu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, control_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, control_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    return 1u;
}
