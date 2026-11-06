#include "format_unit_code11.h"
#include "../ata/host_command_dispatch.h"
#include "../media/active_write_pipeline.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/format_unit_code11_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

/* RAM:fa31f6 */
u8 ParseFormatUnitCode11ParameterList(void)
{
    u8 index;
    u8 layout_offset;
    u32 block_count;

    if(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B2E) == 0ul)
        layout_offset = 8u;
    else
        layout_offset = 4u;

    for(index = 0u; index < layout_offset; ++index)
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A + index) & FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E8F) + index)) != 0u)
            return 0xffu;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D) != 8u)
        return 0xffu;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2E + layout_offset) >> 2) != 0x24u)
        return 0xffu;
    if(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B2A + layout_offset) != 0xfffffffful)
        return 0xffu;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 0x40u));
    block_count = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2F + layout_offset) << 16;
    block_count |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0B30 + layout_offset) << 8;
    block_count |= FirmwareReadNear8(FW_NEAR_UNKNOWN_0B31 + layout_offset);

    if(block_count == 1ul)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1586, 1u);
    else if(block_count != 0ul)
        return 0xffu;

    FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 2u);
    return (u8)block_count;
}

/* RAM:fa360f */
void HandleExpiredFormatUnitCode11PrimaryCountdown(void)
{
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 3) & 1u) == 1u)
        FORMAT11_FINALIZE_ACTIVE_WRITE();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    FORMAT11_NORMALIZE_WRITE_SPEED();
    FORMAT11_RESET_RAW_READ();
    FORMAT11_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    FORMAT11_YIELD_COROUTINE0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));

    if(FORMAT11_ENSURE_OPC() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1586, 1u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 0x40u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 8u));
    FORMAT11_NORMALIZE_WRITE_SPEED();
    if(FORMAT11_INITIALIZE_TYPE2_PIPELINE(2u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xefu));
    FORMAT11_DISPATCH_COMPLETION();
}
