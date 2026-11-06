#include "controller_read_event.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "packed_subchannel_record_batch.h"
#include "parsed_q_raw_read_producer_position.h"
#include "primary_raw_read_producer_position.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_position_tracker.h"
#include "raw_read_position_validation.h"
#include "raw_read_ring_alignment.h"
#include "raw_sector_buffer_event.h"
#include "raw_sector_completion.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/controller_read_event_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/controller_read_event_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadControllerEventWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadControllerEventDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteControllerEventDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u16 ReadControllerEventPortWord(fw_addr_t low_byte_port, fw_addr_t high_byte_port)
{
    return (u16)(FirmwareRead8(low_byte_port) | ((u16)FirmwareRead8(high_byte_port) << 8));
}

static u32 ReadControllerEventPortCursor(fw_addr_t low_byte_port)
{
    return (u32)FirmwareRead8(low_byte_port) | ((u32)FirmwareRead8(low_byte_port + 1u) << 8) | ((u32)FirmwareRead8(low_byte_port + 2u) << 16);
}

static void ServiceControllerReadEventCountdown(void)
{
    ServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x0200u);
}

static void HandleControllerReadEventState2(void)
{
    u8 metadata_mode;
    u8 operating_code;

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F20) > 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F20, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F20) - 1u));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
    WRITE_CONTROLLER_EVENT_BIT_0006(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1F, 0u);
    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    if(ReadControllerEventWord(FW_NEAR_UNKNOWN_0B00) == 0x0a56u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u)
    {
        if(operating_code >= 0x18u)
            metadata_mode = 0x2cu;
        else if(operating_code >= 8u)
            metadata_mode = 0x4cu;
        else
            metadata_mode = 0x6cu;
    }
    else
    {
        if(operating_code >= 0x18u)
            metadata_mode = 0x28u;
        else if(operating_code >= 8u)
            metadata_mode = 0x48u;
        else
            metadata_mode = 0x68u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, metadata_mode);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 2u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F27) & 3u) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u)
    {
        WRITE_CONTROLLER_EVENT_BIT_0006(1u);
        if(READ_CONTROLLER_EVENT_BIT_000C() == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFB) & 0x8fu) | 0x10u));
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFB));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 3u);
}

static void HandleControllerReadEventState3(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F21, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 4u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 9u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, 4u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F75, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 7) & 1u));
}

static void HandleControllerReadEventState4(void)
{
    u32 main_cursor = ReadControllerEventPortCursor(FW_CONTROLLER_UNKNOWN_018008);
    u8 status_nibble;
    u8 format;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) != 0u)
    {
        s32 cursor_delta = (s32)((u32)ReadControllerEventWord(FW_NEAR_UNKNOWN_1296) * 0x0930ul + 0x2df0ul - main_cursor);

        WriteControllerEventDword(FW_NEAR_UNKNOWN_0F7F, (u32)cursor_delta);
        if(cursor_delta != 0l)
        {
            if(cursor_delta < -5l || cursor_delta > 5l)
                WriteControllerEventDword(FW_NEAR_UNKNOWN_0F7F, 0ul);
            InitializeControllerBufferCursorRegistersIfNeeded();
            return;
        }
    }

    if(READ_CONTROLLER_EVENT_BIT_000C() != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1F) > 0)
    {
        AdvanceMainDataRingAndCheckControllerBufferAlignment(main_cursor);
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u
        && ReadControllerEventDword(FW_NEAR_UNKNOWN_1598) == ReadControllerEventDword(FW_NEAR_UNKNOWN_0F00))
    {
        InitializeControllerBufferCursorRegistersIfNeeded();
        return;
    }

    WriteControllerEventDword(FW_NEAR_UNKNOWN_126E, main_cursor);
    status_nibble = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F30) & 0x0fu);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F21) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F21, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F25, status_nibble);
    }
    if(!((status_nibble == FirmwareReadNear8(FW_NEAR_UNKNOWN_0F25) && (READ_CONTROLLER_EVENT_BIT_0006() == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 2u) == 0u))
           || READ_CONTROLLER_EVENT_BIT_000C() != 0u))
    {
        ServiceControllerReadEventCountdown();
        if((s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= (s16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 10u))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 4u);
        }
        WRITE_CONTROLLER_EVENT_BIT_0004(0u);
        return;
    }

    format = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);
    if((s8)format > 0)
    {
        u32 slot_index;

        if(format == 2u)
            slot_index = ReadControllerEventPortWord(FW_CONTROLLER_UNKNOWN_018020, FW_CONTROLLER_UNKNOWN_01801F);
        else
            slot_index = ReadControllerEventPortWord(FW_CONTROLLER_UNKNOWN_01801D, FW_CONTROLLER_UNKNOWN_01801C);
        WriteControllerEventDword(FW_NEAR_UNKNOWN_1262, slot_index);
        if(slot_index == 0x0000fffful)
        {
            if(format != 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F25, status_nibble);
                ServiceControllerReadEventCountdown();
                return;
            }
        }
        else
        {
            slot_index -= ReadControllerEventDword(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF958D) + (u16)format * 4u);
            if(slot_index >= ReadControllerEventDword(FW_NEAR_UNKNOWN_156A))
                slot_index += ReadControllerEventDword(FW_NEAR_UNKNOWN_156A);
            WriteControllerEventDword(FW_NEAR_UNKNOWN_1262, slot_index);
        }

        if(format >= 2u)
        {
            u8 command_bit = (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018022) & 1u);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F74, command_bit);
            if(command_bit == 0u || (FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018023) & 0x40u) == 0u)
            {
                u8 progress = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22) + 1u);

                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, progress);
                if((s8)progress < 5)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F25, status_nibble);
                    ServiceControllerReadEventCountdown();
                    return;
                }
            }
        }
    }

    if(ReadControllerEventWord(FW_NEAR_UNKNOWN_0B00) == 0x0a56u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u)
    {
        u32 auxiliary_cursor;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F25, status_nibble);
        auxiliary_cursor = ReadControllerEventPortCursor(FW_CONTROLLER_UNKNOWN_018019) - ReadControllerEventDword(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF95A1) + (u16)format * 4u);
        if(auxiliary_cursor < ReadControllerEventDword(FW_NEAR_UNKNOWN_1562))
            auxiliary_cursor += MultiplyU32Low(0x0126ul, ReadControllerEventDword(FW_NEAR_UNKNOWN_156A));
        WriteControllerEventDword(FW_NEAR_UNKNOWN_126A, auxiliary_cursor);
    }
    ProcessControllerReadSectorCompletion();
}

static void HandleControllerReadEventState6(void)
{
    if(READ_CONTROLLER_EVENT_BIT_0013() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x87u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x87u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F75, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 7) & 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 7u);
}

static u8 LoadControllerReadEventState7Slot(void)
{
    u8 format = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);
    u32 slot_index;

    if((s8)format <= 1)
    {
        slot_index = ReadControllerEventPortWord(FW_CONTROLLER_UNKNOWN_018020, FW_CONTROLLER_UNKNOWN_01801F);
    }
    else
    {
        slot_index = ReadControllerEventPortWord(FW_CONTROLLER_UNKNOWN_01801D, FW_CONTROLLER_UNKNOWN_01801C);
        WriteControllerEventDword(FW_NEAR_UNKNOWN_1266, ReadControllerEventPortWord(FW_CONTROLLER_UNKNOWN_018020, FW_CONTROLLER_UNKNOWN_01801F));
    }
    WriteControllerEventDword(FW_NEAR_UNKNOWN_1262, slot_index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F62, 0u);

    if(slot_index == 0x0000fffful)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F63) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 1u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F62, 1u);
        if((s8)format > 0 && (s8)format <= 3)
        {
            if(READ_CONTROLLER_EVENT_BIT_000E() != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22) < 3)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, 3u);
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22) < 6 && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F62) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22) + 1u));
                ServiceParsedQDescriptorCountdownAndPositionAdvance();
                return 0u;
            }
            slot_index = (u16)(ReadControllerEventWord((fw_addr_t)(u16)(GetLatestQueuedRawSectorDescriptorAddress() + 8u)) + 1u);
            if(slot_index >= ReadControllerEventDword(FW_NEAR_UNKNOWN_156A))
                slot_index += ReadControllerEventDword(FW_NEAR_UNKNOWN_156A);
            WriteControllerEventDword(FW_NEAR_UNKNOWN_1262, slot_index);
        }
    }
    else if((s8)format > 0)
    {
        slot_index -= 1ul;
        if(slot_index >= ReadControllerEventDword(FW_NEAR_UNKNOWN_156A))
            slot_index += ReadControllerEventDword(FW_NEAR_UNKNOWN_156A);
        WriteControllerEventDword(FW_NEAR_UNKNOWN_1262, slot_index);
    }
    return 1u;
}

static void HandleControllerReadEventState7(void)
{
    u32 retained_cursor = 0ul;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F31, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F28));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F28, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F29));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F29, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2A));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2A, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2B, (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018022) & 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F73));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F73, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018023));

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F20) > 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F20, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F20) - 1u));
        return;
    }
    if(LoadControllerReadEventState7Slot() == 0u)
        return;

    if(READ_CONTROLLER_EVENT_BIT_0013() != 0u && (s32)ReadControllerEventDword(FW_NEAR_UNKNOWN_1598) > -1000l)
    {
        ProcessPackedSubchannelRecordBatch();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u)
    {
        retained_cursor = ReadControllerEventPortCursor(FW_CONTROLLER_UNKNOWN_018019);
        WriteControllerEventDword(FW_NEAR_UNKNOWN_126A, retained_cursor);
    }
    if(READ_CONTROLLER_EVENT_BIT_0013() == 0u)
    {
        retained_cursor = ReadControllerEventPortCursor(FW_CONTROLLER_UNKNOWN_01800B);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < 0x34u && retained_cursor % 0x0930ul != 0ul && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F62) == 0u)
        {
            ServiceParsedQDescriptorCountdownAndPositionAdvance();
            return;
        }
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) != 4u)
        retained_cursor = ReadControllerEventPortCursor(FW_CONTROLLER_UNKNOWN_018008);
    WriteControllerEventDword(FW_NEAR_UNKNOWN_126E, retained_cursor);
    ProcessRawSectorBufferEvent();
}

static void HandleControllerReadEventState9(void)
{
    u8 status_nibble;

    WriteControllerEventDword(FW_NEAR_UNKNOWN_126E, ReadControllerEventPortCursor(FW_CONTROLLER_UNKNOWN_018008));
    status_nibble = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F30) & 0x0fu);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1F) == 0u && (status_nibble == 9u || status_nibble == 10u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_15A3) & 0xf0u) == 0xe0u))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1F, 7u);
    ServiceRawReadPositionValidationAndDescriptorEnqueue();
}

/* RAM:f90d22 */
void ProcessControllerReadEvent(void)
{
    u8 state;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F76, 0x7du);
    if(READ_CONTROLLER_EVENT_BIT_0015() != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
        WRITE_CONTROLLER_EVENT_BIT_0015(0u);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2F, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01800E));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F31, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018011));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F30, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F26));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F26, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F27));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F27, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018010));

    state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04);
    switch(state)
    {
    case 1u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u && READ_CONTROLLER_EVENT_BIT_001B() == 0u)
            ProcessParsedQRawReadProducerPosition();
        else
            ProcessPrimaryRawReadProducerPosition();
        break;
    case 2u:
        HandleControllerReadEventState2();
        break;
    case 3u:
        HandleControllerReadEventState3();
        break;
    case 4u:
        HandleControllerReadEventState4();
        break;
    case 5u:
        TrackRawReadPositionAndArmTargetCapture();
        break;
    case 6u:
        HandleControllerReadEventState6();
        break;
    case 7u:
        HandleControllerReadEventState7();
        break;
    case 9u:
        HandleControllerReadEventState9();
        break;
    default:
        break;
    }
}
