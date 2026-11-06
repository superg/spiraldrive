#include "mode_select.h"
#include "../media/raw_read_runtime.h"
#include "../platform/firmware_memory.h"
#include "mode_page_initialization.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/mode_select_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadModeSelectU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadModeSelectU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteModeSelectU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteModeSelectU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 IsSupportedModeSelectPage(u8 page_code)
{
    switch(page_code)
    {
    case 0x01u:
    case 0x02u:
    case 0x03u:
    case 0x05u:
    case 0x07u:
    case 0x08u:
    case 0x0du:
    case 0x0eu:
    case 0x1au:
    case 0x1du:
        return 1u;
    default:
        return 0u;
    }
}

static u8 IsAcceptedModeSelectBlockSize(u32 block_size)
{
    switch(block_size)
    {
    case 0x0800ul:
    case 0x0804ul:
    case 0x0808ul:
    case 0x0914ul:
    case 0x091cul:
    case 0x0920ul:
    case 0x0924ul:
    case 0x0930ul:
    case 0x0940ul:
    case 0x0990ul:
    case 0x0a56ul:
        return 1u;
    default:
        return 0u;
    }
}

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/mode_select_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u8 ReadModeSelectHeader(u8 cdb_length, u16 *remaining, u32 *controller_cursor, u8 *has_block_descriptor)
{
    u16 header_length = cdb_length == 6u ? 4u : 8u;
    u16 descriptor_length;

    if(*remaining < header_length)
    {
        MODE_SELECT_NO_SENSE();
        return 0u;
    }
    *controller_cursor = MODE_SELECT_CONTROLLER_COPY(FW_NEAR_UNKNOWN_0B2A, *controller_cursor, header_length);
    *remaining = (u16)(*remaining - header_length);
    if(cdb_length == 6u)
        descriptor_length = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D);
    else
        descriptor_length = ReadModeSelectU16(FW_NEAR_UNKNOWN_0B30);
    if(descriptor_length != 0u && descriptor_length != 8u)
    {
        MODE_SELECT_SENSE(5u, 0x2600u);
        return 0u;
    }
    *has_block_descriptor = (u8)(descriptor_length == 8u);
    return 1u;
}

static u8 ValidateModeSelectPages(u16 remaining, u32 controller_cursor)
{
    u8 page_code;
    u8 payload_length;

    while(remaining != 0u)
    {
        if(remaining < 2u)
        {
            MODE_SELECT_SENSE(5u, 0x2600u);
            return 0u;
        }
        controller_cursor = MODE_SELECT_CONTROLLER_COPY(FW_NEAR_UNKNOWN_0B2A, controller_cursor, 2u);
        remaining = (u16)(remaining - 2u);
        page_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A);
        payload_length = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B);
        if(IsSupportedModeSelectPage(page_code) == 0u || ((page_code != 1u || payload_length != 6u) && MODE_SELECT_LENGTH_MATCH(page_code, payload_length) == 0u) || (u16)payload_length > remaining)
        {
            MODE_SELECT_SENSE(5u, 0x2600u);
            return 0u;
        }
        controller_cursor = MODE_SELECT_CONTROLLER_COPY(FW_NEAR_UNKNOWN_0B2C, controller_cursor, payload_length);
        remaining = (u16)(remaining - payload_length);
        if(MODE_SELECT_VALIDATE(page_code) == 0u)
        {
            MODE_SELECT_SENSE(5u, 0x2600u);
            return 0u;
        }
    }
    return 1u;
}

static void ApplyModeSelectPage(u8 page_code)
{
    switch(page_code)
    {
    case 0x01u:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E623, FW_NEAR_UNKNOWN_0B2A, 0x0cu);
        MODE_SELECT_APPLY(0x01u, ApplyModePage01RuntimeState());
        break;
    case 0x02u:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E62F, FW_NEAR_UNKNOWN_0B2A, 0x10u);
        MODE_SELECT_APPLY(0x02u, ApplyModePage02RuntimeTimingParameters());
        break;
    case 0x03u:
        MODE_SELECT_RUNTIME_COPY(FW_HUGE_UNKNOWN_2BC6EF, FW_NEAR_UNKNOWN_0B2A, 0x08u);
        MODE_SELECT_APPLY(0x03u, ApplyModeSelectPage03RuntimeFlags());
        break;
    case 0x05u:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E63F, FW_NEAR_UNKNOWN_0B2A, 0x34u);
        MODE_SELECT_APPLY(0x05u, ApplyModePage05RuntimeImageToWriteState());
        break;
    case 0x07u:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E673, FW_NEAR_UNKNOWN_0B2A, 0x08u);
        MODE_SELECT_APPLY(0x07u, ApplyModePage07RuntimeState());
        break;
    case 0x08u:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E67B, FW_NEAR_UNKNOWN_0B2A, 0x0cu);
        break;
    case 0x0du:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E687, FW_NEAR_UNKNOWN_0B2A, 0x08u);
        MODE_SELECT_APPLY(0x0du, UpdateMediaStateCountdownFromModePage0D());
        break;
    case 0x0eu:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E68F, FW_NEAR_UNKNOWN_0B2A, 0x10u);
        MODE_SELECT_APPLY(0x0eu, ApplyModePage0ERuntimeImageToRawReadState());
        break;
    case 0x1au:
        MODE_SELECT_RUNTIME_COPY(FW_NEAR_UNKNOWN_E69F, FW_NEAR_UNKNOWN_0B2A, 0x0cu);
        MODE_SELECT_APPLY(0x1au, ApplyModePage1ASelectionState());
        break;
    default:
        /* Page 1Dh is validation-only in the canonical apply pass. */
        break;
    }
}

/* RAM:fc1bb4 */
void ProcessModeSelectParameterList(u8 cdb_length, u16 parameter_list_length)
{
    u16 remaining;
    u32 controller_base;
    u32 controller_cursor;
    u32 block_size;
    u8 has_block_descriptor = 0u;
    u8 block_size_changed = 0u;
    u8 page_code;
    u8 payload_length;

    if(MODE_SELECT_DATA_OUT((u32)parameter_list_length) == 4u)
        return;

    controller_base = ReadModeSelectU32(FW_NEAR_UNKNOWN_155A);
    controller_cursor = controller_base;
    remaining = parameter_list_length;
    if(ReadModeSelectHeader(cdb_length, &remaining, &controller_cursor, &has_block_descriptor) == 0u)
        return;

    /* The native body consumes a block descriptor only for MODE SELECT(6). */
    if(has_block_descriptor != 0u && cdb_length == 6u)
    {
        controller_cursor = MODE_SELECT_CONTROLLER_COPY(FW_NEAR_UNKNOWN_0B2A, controller_cursor, 8u);
        remaining = (u16)(remaining - 8u);
        if(IsAcceptedModeSelectBlockSize(ReadModeSelectU32(FW_NEAR_UNKNOWN_0B2E)) == 0u)
        {
            MODE_SELECT_SENSE(5u, 0x2600u);
            return;
        }
    }

    if(ValidateModeSelectPages(remaining, controller_cursor) == 0u)
        return;

    controller_cursor = controller_base;
    remaining = parameter_list_length;
    if(ReadModeSelectHeader(cdb_length, &remaining, &controller_cursor, &has_block_descriptor) == 0u)
        return;

    if(has_block_descriptor != 0u && cdb_length == 6u)
    {
        controller_cursor = MODE_SELECT_CONTROLLER_COPY(FW_NEAR_UNKNOWN_0B2A, controller_cursor, 8u);
        remaining = (u16)(remaining - 8u);
        block_size = ReadModeSelectU32(FW_NEAR_UNKNOWN_0B2E);
        if((u32)ReadModeSelectU16(FW_NEAR_UNKNOWN_0B00) != block_size)
            block_size_changed = 1u;
        WriteModeSelectU16(FW_NEAR_UNKNOWN_0B00, (u16)block_size);
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xbfu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A) == 0x82u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A) == 0x84u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x40u));
            if(block_size == 0x0930ul)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C34, 0u);
            else if(block_size == 0x0990ul)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C34, 2u);
            else if(block_size == 0x0940ul)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C34, 1u);
        }
    }

    while((s16)remaining > 0)
    {
        controller_cursor = MODE_SELECT_CONTROLLER_COPY(FW_NEAR_UNKNOWN_0B2A, controller_cursor, 2u);
        remaining = (u16)(remaining - 2u);
        page_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A);
        payload_length = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B);
        controller_cursor = MODE_SELECT_CONTROLLER_COPY(FW_NEAR_UNKNOWN_0B2C, controller_cursor, payload_length);
        remaining = (u16)(remaining - payload_length);
        ApplyModeSelectPage(page_code);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfeu));
    WriteModeSelectU32(FW_NEAR_UNKNOWN_12C5, ReadModeSelectU32(FW_NEAR_UNKNOWN_12C1) - 150ul);
    WriteModeSelectU32(FW_NEAR_UNKNOWN_157E, ReadModeSelectU32(FW_NEAR_UNKNOWN_157A) - 150ul);
    MODE_SELECT_NO_SENSE();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) != 0u && block_size_changed != 0u)
    {
        MODE_SELECT_RESET_RAW_READ();
        MODE_SELECT_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        MODE_SELECT_DISPATCH_REQUEST5();
        MODE_SELECT_YIELD();
    }
}
