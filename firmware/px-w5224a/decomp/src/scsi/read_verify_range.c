#include "read_verify_range.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_verify_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fd3d0a */
void ReadVerifyFunction(void)
{
    u8 status;
    s32 command_start;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x13u) == 1u)
    {
        SET_SENSE(5u, 0x6400u);
        SET_SENSE_INFORMATION(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02));
        return;
    }

    WriteBigEndianU32(FW_NEAR_UNKNOWN_0F0C, 0ul);
    if(READ_MAPPING_MODE_BIT() == 0u)
    {
        CLEAR_MAPPING_MODE_BIT();
    }
    else
    {
        command_start = (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_15DC, (u32)command_start);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_0B02, (u32)MAP_LOGICAL_BLOCK(command_start) - 150ul);
    }

    if(VALIDATE_RANGE() == 0u)
        return;

    CLEAR_RAW_MODE_BITS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    SET_HANDSHAKE_PINS(0u);
    RESET_RAW_READ_QUEUE();
    INITIALIZE_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, RESET_TRANSFER_STATE());

    do
    {
        status = POLL_DESCRIPTOR_STATUS();
        if(status == 0xfeu)
            YIELD_COROUTINE0();
    } while(status == 0xfeu);

    if(status == 1u)
        SET_NO_SENSE();

    RESET_RAW_READ_QUEUE();
    INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    DISPATCH_REQUEST5();
    YIELD_COROUTINE0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_HANDSHAKE_PINS(1u);
}

/* RAM:fd3dc1 */
u8 PollReadVerifyRawDescriptorStatus(void)
{
    u8 producer_error;
    u16 descriptor_address;
    u32 completed_blocks;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0u;

    producer_error = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    if(producer_error != 0u)
    {
        POLL_SET_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + producer_error),
            ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (fw_addr_t)((u16)producer_error * 2u)));
        POLL_SET_SENSE_INFORMATION(ReadBigEndianU32(FW_NEAR_UNKNOWN_1598) - 150ul);
        return 0u;
    }

    completed_blocks = ReadBigEndianU32(FW_NEAR_UNKNOWN_1459);
    if(completed_blocks == ReadBigEndianU32(FW_NEAR_UNKNOWN_1455))
        return 1u;
    if(ReadBigEndianU16(FW_NEAR_UNKNOWN_1292) == 0u)
        return 0xfeu;

    descriptor_address = POLL_GET_CONSUMER_DESCRIPTOR();
    if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) != 0u && POLL_VALIDATE_DESCRIPTOR(descriptor_address) == 0u)
    {
        if(POLL_MAPPING_MODE_BIT() != 0u)
        {
            if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x13u)) == 0x0au)
            {
                POLL_CONSUME_DESCRIPTORS(1u);
                return 0xfeu;
            }
            if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) == 0u)
            {
                POLL_RESET_RAW_READ_QUEUE();
                POLL_INITIALIZE_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
                WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)POLL_MAP_LOGICAL_BLOCK_ADDRESS((s32)(ReadBigEndianU32(FW_NEAR_UNKNOWN_15DC) + completed_blocks)));
                return 0xfeu;
            }
        }
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1468) != 0u)
        return 0u;
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1459, completed_blocks + 1ul);
    POLL_CONSUME_DESCRIPTORS(1u);
    return 0xfeu;
}

/* RAM:fbf750 */
void RunFullMediaReadVerifyRecoveryCoroutine0(void)
{
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0B02, 0ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0B06, ReadBigEndianU32(FW_NEAR_UNKNOWN_12C1) - 152ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, 0x1eu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfdu));
    RECOVERY_INITIALIZE_COROUTINE1(FW_ENTRY_PROCESS_DRIVE_STATE_MACHINE);

    for(;;)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x02u) != 0u)
        {
            RECOVERY_SET_NO_SENSE();
            RECOVERY_EXECUTE_RANGE();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B1A) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x53u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
                RECOVERY_INITIALIZE_COROUTINE1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, 3u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, 10u);
                WriteBigEndianU32(FW_NEAR_UNKNOWN_11F0, 0ul);
                WriteBigEndianU32(FW_NEAR_UNKNOWN_0E3B, 150ul);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
            }
            for(;;)
                RECOVERY_YIELD_COROUTINE0();
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x52u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
            RECOVERY_INITIALIZE_COROUTINE1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
            for(;;)
                RECOVERY_YIELD_COROUTINE0();
        }
        RECOVERY_YIELD_COROUTINE0();
    }
}
