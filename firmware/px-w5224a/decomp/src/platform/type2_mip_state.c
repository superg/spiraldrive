#include "type2_mip_state.h"
#include "controller_record_boundaries.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/type2_mip_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadMipStateWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadMipStateDwordLowWordFirst(fw_addr_t address)
{
    u16 low = ReadMipStateWord(address + 2u);
    u16 high = ReadMipStateWord(address);

    return ((u32)high << 16) | low;
}

static void WriteMipStateWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteMipStateDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fcd2f0 */
void InitializeType2ControllerRecordStateFromMipPayload(void)
{
    u8 payload_flags;
    u8 layout_class;
    u8 matched_record_count;
    u8 boundary_segment_count;
    u16 highest_marked_index;
    u16 record_index;
    u16 record_count;

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA524, 1u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA525, FirmwareRead8(FW_HUGE_UNKNOWN_2BB921));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA526, FirmwareRead8(FW_HUGE_UNKNOWN_2BB922));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA527, FirmwareRead8(FW_HUGE_UNKNOWN_2BB923));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA528, FirmwareRead8(FW_HUGE_UNKNOWN_2BB924));

    payload_flags = FirmwareRead8(FW_HUGE_UNKNOWN_2BB931);
    layout_class = (u8)(payload_flags >> 6);
    matched_record_count = (u8)((payload_flags >> 4) & 1u);
    if(layout_class == 1u)
        FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 1u);
    else if(layout_class == 2u || layout_class == 3u)
        FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 3u);
    else
        FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 0u);

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E625, 0x80u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3C, FirmwareReadNear8(FW_NEAR_UNKNOWN_E625));
        FirmwareWrite8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x7fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xbfu));
    }

    MIP_STATE_DERIVE_GEOMETRY();
    WriteMipStateDword(FW_NEAR_UNKNOWN_FAFF, 0x0000b4bbul);
    highest_marked_index = (u16)MIP_STATE_DIVIDE((s32)(ReadMipStateDwordLowWordFirst(FW_HUGE_UNKNOWN_2BA59F) - 0x92ul), 39l);
    WriteMipStateWord(FW_HUGE_UNKNOWN_2BA5DB, highest_marked_index);

    record_index = 0u;
    do
    {
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BB4BB + record_index, 5u);
    } while(record_index++ < highest_marked_index);

    record_count = ReadMipStateWord(FW_HUGE_UNKNOWN_2BA5D9);
    while(record_index < record_count)
    {
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BB4BB + record_index, 0u);
        record_index++;
    }

    boundary_segment_count = MIP_STATE_FINALIZE_COMPARISON(matched_record_count, (u8)record_index);
    MIP_STATE_BUILD_BOUNDARIES(boundary_segment_count);
}
