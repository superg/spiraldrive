#include "write_buffer.h"
#include "../platform/buffer_transfer_setup.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "write_buffer_snapshot_validation.h"

extern void YieldCoroutine0(void);
extern u8 InvokeRuntimeMainFlashProgrammerOperation3(void);
extern u8 InvokeRuntimeMainFlashProgrammerOperation5(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/write_buffer_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra addresses. */
static u32 ReadWriteBufferDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u32 ReadWriteBufferU24(fw_addr_t address)
{
    u32 value = (u32)FirmwareRead8(address) << 16;

    value |= (u32)FirmwareRead8(address + 1u) << 8;
    value |= FirmwareRead8(address + 2u);
    return value;
}

static void WriteRuntimeProgrammerEntry(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1312, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1313, 0xffu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1314, 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1315, 0u);
}

static void RestoreProgrammingInterrupts(void)
{
    SetWriteBufferBit(FW_BIT_UNKNOWN_0547, 1u);
    SetWriteBufferBit(FW_BIT_UNKNOWN_0546, 1u);
    WRITE_BUFFER_SET_A9_BIT2();
}

static u8 ReceiveShortWriteBufferPayload(u32 byte_count)
{
    if(byte_count == 0ul)
        return 0u;
    return ExecuteControllerDataOutAndMapStatus(byte_count) == 4u;
}

static u8 ReceiveWriteBufferPayload(u32 buffer_address, u32 byte_count)
{
    if(byte_count == 0ul)
        return 0u;
    PrepareBufferDataTransfer();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 4u);
    return ExecuteBufferDataTransfer(buffer_address, byte_count, 0x3bu) == 4u;
}

static u8 HandleWriteBufferMode0(u8 buffer_id, u32 offset, u32 byte_count)
{
    if(buffer_id != 0u || offset != 0ul || byte_count > ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556) + 4ul)
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }
    if(byte_count == 0ul)
        return 0u;
    if(byte_count <= 4ul)
        return ReceiveShortWriteBufferPayload(byte_count);
    PrepareBufferDataTransfer();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 4u);
    if(ExecuteControllerDataOutAndMapStatus(4ul) == 4u)
        return 1u;
    return ExecuteBufferDataTransfer(0ul, byte_count - 4ul, 0x3bu) == 4u;
}

static u8 HandleWriteBufferMode1(u8 buffer_id, u32 offset, u32 byte_count)
{
    u32 remaining;

    if(buffer_id != 0u || offset >= ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556) || offset + byte_count - 4ul > ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556))
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }
    if(byte_count == 0ul)
        return 0u;
    if(byte_count <= 4ul)
        return ReceiveShortWriteBufferPayload(byte_count);
    PrepareBufferDataTransfer();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 4u);
    if(ExecuteControllerDataOutAndMapStatus(4ul) == 4u)
        return 1u;
    remaining = byte_count - 4ul;
    if(ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556) - offset <= remaining)
        remaining = ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556) - offset;
    return ExecuteBufferDataTransfer(offset, remaining, 0x3bu) == 4u;
}

static u8 HandleWriteBufferMode2Or4(u8 buffer_id, u32 offset, u32 byte_count)
{
    if(buffer_id != 0u || offset >= ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556) || offset + byte_count > ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556))
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }
    return ReceiveWriteBufferPayload(offset, byte_count);
}

static u8 ReceiveProgrammingPayload(u32 offset, u32 byte_count)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    InitializeCoroutine3(FW_ENTRY_IDLE_COROUTINE3);
    YieldCoroutine0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 4u);
    return ExecuteBufferDataTransfer(offset, byte_count, 0x3bu) == 4u;
}

static void QuiesceMediaForProgramming(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
            YieldCoroutine0();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) | 2u));
        ResetMediaProcessingCoroutines();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C3) == 0u)
        {
            YieldCoroutine0();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 1u);
        }
    }
}

static u8 ValidateProgrammingPayload(u8 buffer_id)
{
    if(buffer_id == 0u)
    {
        if(ValidateStagedMainFirmwareHeader() == 0u)
        {
            SetSenseData(4u, 0x40f5u);
            RestoreProgrammingInterrupts();
            return 0u;
        }
        if(ValidateStagedFirmwareChecksum(0x00079ffeul, 0ul) == 0u)
        {
            SetSenseData(4u, 0x40f6u);
            RestoreProgrammingInterrupts();
            return 0u;
        }
    }
    else
    {
        if(ValidateStagedFirmwareChecksum(0x0000fffeul, 0ul) == 0u)
        {
            SetSenseData(4u, 0x40f6u);
            RestoreProgrammingInterrupts();
            return 0u;
        }
        StagePrimaryDriveConfigurationForAuxiliaryUpdate();
    }
    return 1u;
}

static u8 InvokeWriteBufferProgrammer(u8 buffer_id)
{
    u8 result;

    SetWriteBufferBit(FW_BIT_UNKNOWN_0580, 0u);
    SetWriteBufferBit(FW_BIT_UNKNOWN_0581, 0u);
    WriteRuntimeProgrammerEntry();
    if(buffer_id == 0u)
        result = InvokeRuntimeMainFlashProgrammerOperation3();
    else
        result = InvokeRuntimeMainFlashProgrammerOperation5();
    if(result != 0u)
    {
        SetSenseData(4u, 0x40f3u);
        return 1u;
    }
    return 0u;
}

static u8 HandlePrimaryOrAuxiliaryProgramming(u8 buffer_id, u32 offset, u32 byte_count)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        SetSenseData(4u, 0x40f6u);
        return 1u;
    }
    if(ReceiveProgrammingPayload(offset, byte_count) != 0u)
        return 1u;
    if(ValidateStagedFirmwareRevisionPolicy(buffer_id) == 0u)
    {
        SetSenseData(4u, 0x40f5u);
        return 1u;
    }
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xdfu));
    QuiesceMediaForProgramming();
    SetWriteBufferBit(FW_BIT_UNKNOWN_0481, 0u);
    SetWriteBufferBit(FW_BIT_UNKNOWN_0480, 0u);
    SetWriteBufferBit(FW_BIT_UNKNOWN_0547, 0u);
    if(ValidateProgrammingPayload(buffer_id) == 0u)
        return 1u;
    return InvokeWriteBufferProgrammer(buffer_id);
}

static u8 HandleWriteBufferSnapshot(u8 buffer_id, u32 offset, u32 byte_count)
{
    u8 failed = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 1u)
    {
        SetCommandSequenceErrorSense();
        return 1u;
    }
    if(offset != 0ul || byte_count > ReadWriteBufferDword(FW_NEAR_UNKNOWN_1556) || buffer_id != 2u)
    {
        HandleUnsupportedScsiCommand();
        return 1u;
    }
    if(byte_count == 0ul)
        return 0u;
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 4u);
    if(ExecuteBufferDataTransfer(0ul, byte_count, 0x3bu) == 4u)
        failed = 1u;
    if(ValidateWriteBufferSnapshot(byte_count) == 0u)
    {
        SetSenseData(5u, 0x2600u);
        failed = 1u;
    }
    return failed;
}

static u8 HandleWriteBufferMode5(u8 buffer_id, u32 offset, u32 byte_count)
{
    if(buffer_id == 0u || buffer_id == 0xffu)
        return HandlePrimaryOrAuxiliaryProgramming(buffer_id, offset, byte_count);
    return HandleWriteBufferSnapshot(buffer_id, offset, byte_count);
}

/* RAM:fd2741 */
void HandleWriteBufferCommand(void)
{
    u8 mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 7u);
    u8 buffer_id = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C);
    u32 offset = ReadWriteBufferU24(FW_NEAR_UNKNOWN_0B0D);
    u32 byte_count = ReadWriteBufferU24(FW_NEAR_UNKNOWN_0B10);
    u8 failed;

    if(mode <= 2u)
    {
        u8 media_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);

        if(media_status != 0u)
        {
            SetSenseDataFromStatusCode(media_status);
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
        {
            SetSenseData(2u, 0x0401u);
            return;
        }
    }
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E3F)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x20u));
    if(mode == 0u)
        failed = HandleWriteBufferMode0(buffer_id, offset, byte_count);
    else if(mode == 1u)
        failed = HandleWriteBufferMode1(buffer_id, offset, byte_count);
    else if(mode == 2u || mode == 4u)
        failed = HandleWriteBufferMode2Or4(buffer_id, offset, byte_count);
    else if(mode == 5u)
        failed = HandleWriteBufferMode5(buffer_id, offset, byte_count);
    else
    {
        HandleUnsupportedScsiCommand();
        failed = 1u;
    }
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xdfu));
    if(failed == 0u)
        SetNoSenseData();
}
