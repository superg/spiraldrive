#include "command_dispatch.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "blank.h"
#include "close_track_session.h"
#include "format_unit.h"
#include "get_configuration.h"
#include "get_event_status_notification.h"
#include "get_performance.h"
#include "opcode_f0_configuration_update.h"
#include "play_audio_track_index.h"
#include "play_cd.h"
#include "play_track_relative.h"
#include "read_buffer.h"
#include "read_buffer_capacity.h"
#include "read_disc_information.h"
#include "read_format_capacities.h"
#include "read_subchannel_command.h"
#include "read_toc_dispatch.h"
#include "repair_track.h"
#include "request_sense.h"
#include "reserve_track.h"
#include "scsi_opcode_ba_cd.h"
#include "send_cue_sheet.h"
#include "send_diagnostic.h"
#include "send_opc_information.h"
#include "set_cd_speed.h"
#include "spc.h"
#include "start_stop_unit.h"
#include "synchronize_cache10.h"
#include "vendor.h"
#include "vendor_e9.h"
#include "vendor_eb.h"
#include "vendor_ed.h"
#include "vendor_f3.h"
#include "vendor_f5.h"
#include "vendor_raw_read.h"
#include "write_and_verify.h"
#include "write_buffer.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/command_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static const u8 supported_opcode_bits[32] = { 0x1bu, 0x0du, 0x24u, 0x7cu, 0x28u, 0xcdu, 0x20u, 0x18u, 0xfcu, 0x4fu, 0x3eu, 0x3fu, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x22u, 0x97u,
    0x00u, 0x5eu, 0x00u, 0x20u, 0x00u, 0xc3u, 0x00u, 0xfau, 0xffu, 0x01u };

static const u8 media_required_opcode_bits[32] = { 0x13u, 0x0du, 0x00u, 0x30u, 0x20u, 0x8du, 0x30u, 0x00u, 0xbcu, 0x4bu, 0x1eu, 0x1bu, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x20u,
    0x87u, 0x00u, 0x56u, 0x00u, 0x20u, 0x00u, 0x83u, 0x00u, 0x00u, 0x54u, 0x06u };

u8 IsSupportedScsiOpcode(u8 opcode)
{
    return (u8)((supported_opcode_bits[opcode >> 3] & (u8)(1u << (opcode & 7u))) != 0u);
}

u8 ScsiCommandRequiresMedia(u8 opcode)
{
    return (u8)((media_required_opcode_bits[opcode >> 3] & (u8)(1u << (opcode & 7u))) != 0u);
}

void DispatchScsiOpcode(u8 opcode)
{
    switch(opcode)
    {
#define X(value, handler)                                                                                                                                                                              \
    case value:                                                                                                                                                                                        \
        handler();                                                                                                                                                                                     \
        break;
#include "command_dispatch_map.inc"
#undef X
    default:
        HandleUnsupportedScsiCommand();
        break;
    }
}

void DispatchCurrentScsiOpcode(void)
{
    DispatchScsiOpcode(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE));
}

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/command_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION
