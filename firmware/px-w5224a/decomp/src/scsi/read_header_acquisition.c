#include "read_header_acquisition.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_header_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_header_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u16 ReadBigEndianU16ForReadHeader(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteBigEndianU16ForReadHeader(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fd3f70 */
u8 PollReadHeaderSectorAndTransferResponse(void)
{
    u8 controller_error;
    u16 descriptor_count;
    u16 descriptor_address;
    u16 frame_pointer;
    u16 allocation_length;
    s32 frame_position;

    if(((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) >> 4) & 1u) != 0u)
        return 0u;

    controller_error = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    if(controller_error != 0u)
    {
        SET_READ_HEADER_POLL_ERROR_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + controller_error),
            ReadBigEndianU16ForReadHeader(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (fw_addr_t)((u16)controller_error * 2u)));
        return 0u;
    }

    WriteBigEndianU16ForReadHeader(FW_NEAR_UNKNOWN_145F, ReadBigEndianU16ForReadHeader(FW_NEAR_UNKNOWN_0C3F));
    if(PREPARE_READ_HEADER_POLL_BATCH() == 0u)
        return 0xfeu;

    descriptor_count = COUNT_READ_HEADER_POLL_DESCRIPTORS();
    WriteBigEndianU16ForReadHeader(FW_NEAR_UNKNOWN_1463, descriptor_count);
    if(descriptor_count == 0u)
        return 0u;

    descriptor_address = GET_READ_HEADER_POLL_DESCRIPTOR();
    if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) != 0u && VALIDATE_READ_HEADER_POLL_DESCRIPTOR(descriptor_address) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x12u)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);

    frame_pointer = ReadBigEndianU16ForReadHeader(FW_NEAR_UNKNOWN_0B0D);
    frame_position = (s32)ReadBigEndianU32((fw_addr_t)frame_pointer);
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) >> 1) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
        CONVERT_READ_HEADER_POLL_FRAME(frame_position, FW_NEAR_UNKNOWN_0B2F);
    }
    else
    {
        WriteBigEndianU32(FW_NEAR_UNKNOWN_0B2E, (u32)frame_position - 150ul);
    }

    allocation_length = (u16)(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B10) >> 8);
    allocation_length = MIN_READ_HEADER_POLL_LENGTH(allocation_length, 8u);
    TRANSFER_READ_HEADER_POLL_RESPONSE(FW_NEAR_UNKNOWN_0B2A, (u32)allocation_length);
    SET_NO_SENSE_FOR_READ_HEADER_POLL();
    return 1u;
}

/* RAM:fd3eaf */
void ReadHeaderAcquisitionFunction(void)
{
    u8 status;
    s32 target_position;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x13u) == 1u)
    {
        SET_SENSE(5u, 0x6400u);
        SET_INFORMATION(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02));
        return;
    }

    WriteBigEndianU32(FW_NEAR_UNKNOWN_0F0C, 0ul);
    PREPARE_VALIDATION_BITS();
    if(VALIDATE_RANGE() == 0u)
        return;

    CLEAR_VALIDATION_BIT();
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0B06, 1ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    SET_HANDSHAKE_PINS(0u);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0FB0, ReadBigEndianU32(FW_NEAR_UNKNOWN_0FB4));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 2u)
    {
        target_position = (s32)RESET_TRANSFER_STATE();
        POSITION_PIPELINE(target_position);
    }
    else
    {
        RESET_QUEUE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
        INITIALIZE_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, RESET_TRANSFER_STATE());
        YIELD_COROUTINE0();
    }

    do
    {
        status = POLL_RESPONSE();
        if(status == 0xfeu)
            YIELD_COROUTINE0();
    } while(status == 0xfeu);

    if(status == 0u)
    {
        RESET_QUEUE();
        INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        DISPATCH_REQUEST5();
        YIELD_COROUTINE0();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_HANDSHAKE_PINS(1u);
}
