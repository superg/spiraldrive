#include "write_and_verify.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/write_and_verify_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fcbcde */
void WriteAndVerifyFunction(void)
{
    u32 block_count;
    u32 start_address;

    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x20u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xbfu));
    SET_COMMAND_ACTIVE();

    if(SELECT_TRANSFER_LENGTH_FORM() == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13) & 0xfcu) != 0u)
        {
            HANDLE_UNSUPPORTED();
            return;
        }
        block_count = ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11);
    }
    else
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B15) & 0xfcu) != 0u)
        {
            HANDLE_UNSUPPORTED();
            return;
        }
        block_count = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B10);
    }

    start_address = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B0C);
    PREPARE_WRITE_STATE(start_address);
    if(START_OR_CONTINUE(start_address, block_count) == 0u)
        return;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x40u) != 0u)
    {
        SET_NO_SENSE();
        CLEAR_COMMAND_ACTIVE();
        return;
    }

    if(ReadBigEndianU16(FW_NEAR_UNKNOWN_1292) < 0x0249u)
    {
        SET_NO_SENSE();
        CLEAR_COMMAND_ACTIVE();
        return;
    }

    if(EXECUTE_BLOCK_RANGE(block_count) != 0u)
    {
        EXECUTE_READBACK();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    }
    CLEAR_COMMAND_ACTIVE();
}
