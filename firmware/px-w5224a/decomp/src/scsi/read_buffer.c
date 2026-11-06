#include "read_buffer.h"
#include "../platform/arithmetic.h"
#include "../platform/buffer_transfer_setup.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "read_buffer_snapshot.h"

/* Reconstruction helpers; no direct Ghidra addresses. */
static u32 ReadReadBufferDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u32 ReadReadBufferU24(fw_addr_t address)
{
    u32 value = (u32)FirmwareRead8(address) << 16;

    value |= (u32)FirmwareRead8(address + 1u) << 8;
    value |= FirmwareRead8(address + 2u);
    return value;
}

static void WriteReadBufferDescriptor(u32 capacity)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, (u8)(capacity >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, (u8)(capacity >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, (u8)capacity);
}

static u8 TransferReadBufferDescriptor(u32 byte_count)
{
    return CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, byte_count) == 4u;
}

static u8 HandleReadBufferMode0(u8 buffer_id, u32 offset, u32 byte_count)
{
    u32 transfer_length;

    if(buffer_id != 0u || offset != 0ul)
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }

    WriteReadBufferDescriptor(ReadReadBufferDword(FW_NEAR_UNKNOWN_1556) + 0x00050000ul);
    if(byte_count <= 4ul)
        return TransferReadBufferDescriptor(byte_count);
    if(TransferReadBufferDescriptor(4ul) != 0u)
        return 1u;

    transfer_length = MinU32(byte_count - 4ul, ReadReadBufferDword(FW_NEAR_UNKNOWN_1556));
    return ExecuteBufferDataTransfer(0ul, transfer_length, 0x3cu) == 4u;
}

static u8 HandleReadBufferMode1Snapshot(u32 offset, u32 byte_count)
{
    u16 snapshot_length;
    u32 transfer_length;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        SetCommandSequenceErrorSense();
        return 1u;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
    {
        SetCommandSequenceErrorSense();
        return 1u;
    }
    if(offset != 0ul)
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }

    PrepareBufferDataTransfer();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 4u);
    snapshot_length = BuildReadBufferMode1Id2Snapshot();
    transfer_length = MinU32(byte_count, snapshot_length);
    return ExecuteBufferDataTransfer(ReadReadBufferDword(FW_NEAR_UNKNOWN_155A), transfer_length, 0x3cu) == 4u;
}

static u8 HandleReadBufferMode1(u8 buffer_id, u32 offset, u32 byte_count)
{
    u32 capacity;
    u32 transfer_length;

    if(buffer_id == 2u)
        return HandleReadBufferMode1Snapshot(offset, byte_count);

    capacity = ReadReadBufferDword(FW_NEAR_UNKNOWN_1556);
    if(buffer_id != 0u || offset >= capacity)
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }

    WriteReadBufferDescriptor(ReadReadBufferDword(FW_NEAR_UNKNOWN_1556) - offset);
    if(byte_count <= 4ul)
        return TransferReadBufferDescriptor(byte_count);
    if(TransferReadBufferDescriptor(4ul) != 0u)
        return 1u;

    transfer_length = MinU32(byte_count - 4ul, ReadReadBufferDword(FW_NEAR_UNKNOWN_1556));
    return ExecuteBufferDataTransfer(offset, transfer_length, 0x3cu) == 4u;
}

static u8 HandleReadBufferMode2(u8 buffer_id, u32 offset, u32 byte_count)
{
    u32 capacity = ReadReadBufferDword(FW_NEAR_UNKNOWN_1556);
    u32 transfer_length;

    if(buffer_id != 0u || offset >= capacity)
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }

    transfer_length = MinU32(byte_count, ReadReadBufferDword(FW_NEAR_UNKNOWN_1556) - offset);
    return ExecuteBufferDataTransfer(offset, transfer_length, 0x3cu) == 4u;
}

static u8 HandleReadBufferMode3(u8 buffer_id, u32 offset, u32 byte_count)
{
    if(offset != 0ul)
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }

    if(buffer_id == 0u)
        WriteReadBufferDescriptor(ReadReadBufferDword(FW_NEAR_UNKNOWN_1556));
    else
        WriteReadBufferDescriptor(0ul);
    return TransferReadBufferDescriptor(MinU32(byte_count, 4ul));
}

/* RAM:fd2e21 */
void HandleReadBufferCommand(void)
{
    u8 mode;
    u8 buffer_id;
    u32 offset;
    u32 byte_count;
    u8 failed;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D9C)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 7u);
    buffer_id = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C);
    offset = ReadReadBufferU24(FW_NEAR_UNKNOWN_0B0D);
    byte_count = ReadReadBufferU24(FW_NEAR_UNKNOWN_0B10);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x20u));

    if(mode == 0u)
        failed = HandleReadBufferMode0(buffer_id, offset, byte_count);
    else if(mode == 1u)
        failed = HandleReadBufferMode1(buffer_id, offset, byte_count);
    else if(mode == 2u)
        failed = HandleReadBufferMode2(buffer_id, offset, byte_count);
    else if(mode == 3u)
        failed = HandleReadBufferMode3(buffer_id, offset, byte_count);
    else
    {
        HandleUnsupportedScsiCommand();
        failed = 1u;
    }

    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xdfu));
    if(failed == 0u)
        SetNoSenseData();
}
