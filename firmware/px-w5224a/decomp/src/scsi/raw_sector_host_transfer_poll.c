#include "raw_sector_host_transfer_poll.h"
#include "../platform/firmware_memory.h"
#include "raw_sector_batched_host_transfer.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/raw_sector_host_transfer_poll_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteBigEndianU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fa439b */
u8 PollRawReadTransferBatchStatus(void)
{
    u16 descriptor_count;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0xffu;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        return 0u;

    if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH) == 1u || (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 2u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfdu));
        return 0xffu;
    }
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_122E) <= 0 && ReadBigEndianU16(FW_NEAR_UNKNOWN_122F) >= 3u)
        return 0u;

    WriteBigEndianU16(FW_NEAR_UNKNOWN_145F, ReadBigEndianU16(FW_NEAR_UNKNOWN_0C3F));
    if(PREPARE_BATCH() == 0u)
        return 0xfeu;

    descriptor_count = COUNT_VALIDATED();
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1463, descriptor_count);
    if(descriptor_count == 0u)
    {
        if(READ_BATCH_MAPPING_BIT() != 0u)
        {
            u16 descriptor_address = GET_BATCH_SECONDARY_DESCRIPTOR();

            if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x13u)) == 10u)
            {
                WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, (u16)((ReadBigEndianU16(FW_NEAR_UNKNOWN_1298) + 1u) & ReadBigEndianU16(FW_NEAR_UNKNOWN_129A)));
                return 0xfeu;
            }
            if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) == 0u)
            {
                s32 logical_block = (s32)(ReadBigEndianU32(FW_NEAR_UNKNOWN_15DC) + ReadBigEndianU32(FW_NEAR_UNKNOWN_1459));

                RESET_BATCH_QUEUE();
                INITIALIZE_BATCH_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
                WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)MAP_BATCH_LOGICAL_BLOCK(logical_block));
                return 0xfeu;
            }
        }
        return 0u;
    }

    WriteBigEndianU32(FW_NEAR_UNKNOWN_1459, ReadBigEndianU32(FW_NEAR_UNKNOWN_1459) + (u32)descriptor_count);
    if(ReadBigEndianU32(FW_NEAR_UNKNOWN_1459) != ReadBigEndianU32(FW_NEAR_UNKNOWN_1455))
        return 0xfeu;

    WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, (u16)((ReadBigEndianU16(FW_NEAR_UNKNOWN_1298) + descriptor_count) & ReadBigEndianU16(FW_NEAR_UNKNOWN_129A)));
    LIMIT_POSITION_BACKLOG();
    return ADVANCE_BATCH_MATCHED_TARGET() == 1u ? 1u : 2u;
}

/* RAM:fd6fe0 */
u8 PLEXTOR_NEAR_FUNCTION PollRawSectorReadHostTransferStatus(void)
{
    u8 producer_error;
    u16 validated_count;
    u32 total_blocks;
    u32 completed_blocks;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0u;

    producer_error = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    if(producer_error != 0u)
    {
        u8 sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        u16 sense_code = ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (u16)sense_index * 2u);
        u8 key_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        u8 sense_key = FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + key_index);
        SET_SENSE(sense_key, sense_code);
        completed_blocks = ReadBigEndianU32(FW_NEAR_UNKNOWN_1459);
        SET_INFORMATION(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02) + completed_blocks);
        return 0u;
    }

    WriteBigEndianU16(FW_NEAR_UNKNOWN_145F, ReadBigEndianU16(FW_NEAR_UNKNOWN_0C3F));
    if(PREPARE_BATCH() == 0u)
        return 0xfeu;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) != 3u)
        return VALIDATE_DESCRIPTOR(GET_CONSUMER_DESCRIPTOR()) != 0u ? 1u : 0u;

    validated_count = COUNT_VALIDATED();
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1463, validated_count);
    if(validated_count == 0u)
        return 0u;

    PREPARE_BLOCK_SIZE(ReadBigEndianU16(FW_NEAR_UNKNOWN_145D));
    START_BATCH_TRANSFER();
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F47) != 0u)
        YIELD_COROUTINE0();

    total_blocks = ReadBigEndianU32(FW_NEAR_UNKNOWN_1455);
    completed_blocks = ReadBigEndianU32(FW_NEAR_UNKNOWN_1459);
    return completed_blocks == total_blocks ? 1u : 0xfeu;
}

/* RAM:fc5226 */
s8 PLEXTOR_NEAR_FUNCTION ServiceReadCommandRawSectorTransferBatch(void)
{
    u8 producer_error;
    u16 descriptor_count;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0;

    producer_error = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    if(producer_error != 0u)
    {
        u8 sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        u16 sense_code = ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (fw_addr_t)((u16)sense_index * 2u));
        u8 key_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        u32 adjustment;
        u32 command_start;
        u32 completed_blocks;

        SET_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + key_index), sense_code);
        adjustment = ReadBigEndianU32(FW_NEAR_UNKNOWN_0F0C);
        command_start = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02);
        completed_blocks = ReadBigEndianU32(FW_NEAR_UNKNOWN_1459);
        SET_INFORMATION(command_start - adjustment + completed_blocks);
        return 0;
    }

    WriteBigEndianU16(FW_NEAR_UNKNOWN_145F, ReadBigEndianU16(FW_NEAR_UNKNOWN_0C3F));
    if(PREPARE_BATCH() == 0u)
        return (s8)0xfe;

    descriptor_count = COUNT_VALIDATED();
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1463, descriptor_count);
    if(descriptor_count == 0u)
    {
        if(READ_BATCH_MAPPING_BIT() != 0u)
        {
            u16 descriptor_address = GET_BATCH_SECONDARY_DESCRIPTOR();

            if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x13u)) == 10u)
            {
                u16 secondary_cursor = (u16)((ReadBigEndianU16(FW_NEAR_UNKNOWN_1298) + 1u) & ReadBigEndianU16(FW_NEAR_UNKNOWN_129A));
                WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, secondary_cursor);
                return (s8)0xfe;
            }
            if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) == 0u)
            {
                u32 logical_block;
                s32 target_position;

                RESET_BATCH_QUEUE();
                INITIALIZE_BATCH_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
                logical_block = ReadBigEndianU32(FW_NEAR_UNKNOWN_15DC) + ReadBigEndianU32(FW_NEAR_UNKNOWN_1459);
                if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xb9u)
                    target_position = (s32)(logical_block + 150ul);
                else
                    target_position = MAP_BATCH_LOGICAL_BLOCK((s32)logical_block);
                WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)target_position);
                return (s8)0xfe;
            }
        }
        return 0;
    }

    SET_BATCH_TRANSFER_BIT(1u);
    if(CONFIGURE_BATCH_TRANSFER() == 0u)
    {
        SET_BATCH_TRANSFER_BIT(0u);
        return (s8)0xff;
    }
    SET_BATCH_TRANSFER_BIT(0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1468) != 0u)
        return 0;
    if(ReadBigEndianU32(FW_NEAR_UNKNOWN_1459) != ReadBigEndianU32(FW_NEAR_UNKNOWN_1455))
        return (s8)0xfe;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1467) | FirmwareReadNear8(FW_NEAR_UNKNOWN_1466)) != 0u)
        return 0;
    return 1;
}

/* RAM:fc5107 */
void ExecuteReadCommandRawSectorTransferFunction(void)
{
    s8 batch_status;

    WriteBigEndianU32(FW_NEAR_UNKNOWN_0F0C, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F09, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F08, 0x32u);
    INITIALIZE_READ_COMMAND_BITS();

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x40u) != 0u)
    {
        HANDLE_COMPLETED_READ_COMMAND();
        return;
    }

    CLEAR_READ_COMMAND_VALIDATION_BITS();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x13u) == 1u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 5u) == 5u && VALIDATE_READ_COMMAND_MEDIA() == 0u))
    {
        SET_SENSE(5u, 0x6400u);
        SET_INFORMATION(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02));
        return;
    }

    if(READ_READ_COMMAND_MAPPING_ENABLE() == 0u)
    {
        CLEAR_READ_COMMAND_MAPPING_BIT();
    }
    else
    {
        u32 command_start = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02);
        s32 mapped_position;

        WriteBigEndianU32(FW_NEAR_UNKNOWN_15DC, command_start);
        mapped_position = MAP_READ_COMMAND_LBA((s32)command_start);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_0B02, (u32)mapped_position - 150ul);
    }

    if(VALIDATE_READ_COMMAND_RANGE() == 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    SET_READ_COMMAND_HANDSHAKE_PINS(0u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0FB0, ReadBigEndianU16(FW_NEAR_UNKNOWN_0FB4));
    UPDATE_READ_COMMAND_PROGRESS();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 2u)
    {
        POSITION_READ_COMMAND_PIPELINE((s32)RESET_READ_COMMAND_TRANSFER());
    }
    else
    {
        RESET_READ_COMMAND_QUEUE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
        INITIALIZE_READ_COMMAND_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, RESET_READ_COMMAND_TRANSFER());
        YIELD_READ_COMMAND_COROUTINE0();
    }

    CLEAR_READ_COMMAND_PENDING_BIT();
    do
    {
        batch_status = SERVICE_READ_COMMAND_BATCH();
        if(batch_status == (s8)-2)
            YIELD_READ_COMMAND_COROUTINE0();
    } while(batch_status == (s8)-2);

    if(batch_status == 0)
    {
        RESET_READ_COMMAND_QUEUE();
        INITIALIZE_READ_COMMAND_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        DISPATCH_READ_COMMAND_REQUEST5();
        YIELD_READ_COMMAND_COROUTINE0();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_READ_COMMAND_HANDSHAKE_PINS(1u);
}

/* RAM:fc59df */
s8 ExecuteReadCdRawSectorTransferForFormatFunction(u8 raw_read_format_index)
{
    s8 batch_status;

    WriteBigEndianU32(FW_NEAR_UNKNOWN_1459, 0ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0F0C, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F08, 0x32u);
    INITIALIZE_READ_CD_BITS();

    if(READ_READ_COMMAND_MAPPING_ENABLE() == 0u)
    {
        CLEAR_READ_COMMAND_MAPPING_BIT();
    }
    else
    {
        u32 command_start = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02);

        WriteBigEndianU32(FW_NEAR_UNKNOWN_15DC, command_start);
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u || FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) != 0xb9u)
        {
            s32 mapped_position = MAP_READ_COMMAND_LBA((s32)command_start);
            WriteBigEndianU32(FW_NEAR_UNKNOWN_0B02, (u32)mapped_position - 150ul);
        }
    }

    if(VALIDATE_READ_COMMAND_RANGE() == 0u)
        return 1;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    SET_READ_COMMAND_HANDSHAKE_PINS(0u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0FB0, ReadBigEndianU16(FW_NEAR_UNKNOWN_0FB4));
    UPDATE_READ_COMMAND_PROGRESS();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 2u && raw_read_format_index == FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06))
    {
        POSITION_READ_COMMAND_PIPELINE((s32)(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02) + 150ul));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
        RESET_READ_COMMAND_QUEUE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
        INITIALIZE_READ_COMMAND_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02) + 150ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F06, raw_read_format_index);
        YIELD_READ_COMMAND_COROUTINE0();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1467, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1466, 0u);
    CLEAR_READ_COMMAND_PENDING_BIT();
    do
    {
        batch_status = SERVICE_READ_COMMAND_BATCH();
        if(batch_status == (s8)-2)
            YIELD_READ_COMMAND_COROUTINE0();
    } while(batch_status == (s8)-2);

    if(batch_status == 0 && READ_READ_CD_TERMINAL_BIT() != 1u)
    {
        u16 descriptor_address = GET_BATCH_SECONDARY_DESCRIPTOR();

        if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x13u)) != 4u)
        {
            RESET_READ_COMMAND_QUEUE();
            INITIALIZE_READ_COMMAND_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
            DISPATCH_READ_COMMAND_REQUEST5();
            YIELD_READ_COMMAND_COROUTINE0();
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_READ_COMMAND_HANDSHAKE_PINS(1u);
    return batch_status;
}

static void RestartRawReadPipelineAtFramePosition(s32 target_position)
{
    SET_POSITION_EXTERNAL_INTERRUPT0(1u);
    RESET_READ_COMMAND_QUEUE();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 2u)
        INITIALIZE_READ_COMMAND_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
    else
        INITIALIZE_READ_COMMAND_COROUTINE2(FW_ENTRY_RUN_ALTERNATE_RAW_READ_PIPELINE_COROUTINE2);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)target_position);
    YIELD_READ_COMMAND_COROUTINE0();
}

/* RAM:fc5af5 */
void PositionRawReadPipelineAtFramePositionFunction(s32 target_position)
{
    u16 latest_descriptor;

    SET_POSITION_EXTERNAL_INTERRUPT0(0u);
    latest_descriptor = GET_POSITION_LATEST_DESCRIPTOR();
    if(latest_descriptor == 0u)
    {
        if((READ_POSITION_BIT0008() == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) != 0u) && (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_1598) == target_position)
        {
            WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, ReadBigEndianU16(FW_NEAR_UNKNOWN_1294));
            SET_POSITION_EXTERNAL_INTERRUPT0(1u);
            return;
        }
        RestartRawReadPipelineAtFramePosition(target_position);
    }
    else
    {
        u16 latest_source = ReadBigEndianU16((fw_addr_t)(u16)(latest_descriptor + 2u));
        s32 latest_position = (s32)ReadBigEndianU32(latest_source);

        if(latest_position < target_position)
        {
            if((READ_POSITION_BIT0008() == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) != 0u) && (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_1598) == target_position)
            {
                WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, ReadBigEndianU16(FW_NEAR_UNKNOWN_1296));
                LIMIT_POSITION_BACKLOG();
                SET_POSITION_EXTERNAL_INTERRUPT0(1u);
                return;
            }
            RestartRawReadPipelineAtFramePosition(target_position);
        }
        else
        {
            u16 consumer_descriptor = GET_POSITION_CONSUMER_DESCRIPTOR();
            u16 consumer_source = ReadBigEndianU16((fw_addr_t)(u16)(consumer_descriptor + 2u));
            s32 consumer_position = (s32)ReadBigEndianU32(consumer_source);

            if(target_position < consumer_position)
            {
                RestartRawReadPipelineAtFramePosition(target_position);
            }
            else
            {
                u16 cursor = (u16)((u16)((u32)target_position - (u32)consumer_position) + ReadBigEndianU16(FW_NEAR_UNKNOWN_1294));
                cursor &= ReadBigEndianU16(FW_NEAR_UNKNOWN_129A);
                WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, cursor);
                LIMIT_POSITION_BACKLOG();
            }
        }
    }
    SET_POSITION_EXTERNAL_INTERRUPT0(1u);
}
