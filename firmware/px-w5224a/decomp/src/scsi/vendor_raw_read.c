#include "vendor_raw_read.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_sector_range.h"
#include "send_opc_information.h"
#include "vendor_measurement_response.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_raw_read_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/vendor_raw_read_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadBigEndianU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void OrBigEndianU16(fw_addr_t address, u16 mask)
{
    WriteBigEndianU16(address, (u16)(ReadBigEndianU16(address) | mask));
}

/* RAM:fbd67a */
void HandleScsiOpcodeF6(void)
{
    s32 start_frame_position;
    s32 end_frame_position;
    u8 command_flags;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 4u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            SetVendorF6SenseFromStatus(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
        {
            SetVendorF6Sense(2u, 0x0401u);
            return;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
    {
        SetVendorF6SequenceError();
        return;
    }

    command_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B);
    if((command_flags & 2u) == 0u)
    {
        start_frame_position = (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_0B0C);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) == 0xffu)
    {
        start_frame_position = (s32)(ReadBigEndianU32(FW_NEAR_UNKNOWN_114F) - 0x0a65ul);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 1u);
        start_frame_position = ConvertVendorF6Msf(FW_NEAR_UNKNOWN_0B0D);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
    }
    WriteBigEndianU32(FW_NEAR_UNKNOWN_F091, (u32)start_frame_position);

    if((command_flags & 2u) == 0u)
    {
        end_frame_position = (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_0B10);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B11) == 0xffu)
    {
        end_frame_position = (s32)(ReadBigEndianU32(FW_NEAR_CURRENT_FRAME_POSITION) + 0x1a5ful);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 1u);
        end_frame_position = ConvertVendorF6Msf(FW_NEAR_UNKNOWN_0B11);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
    }
    WriteBigEndianU32(FW_NEAR_UNKNOWN_F095, (u32)end_frame_position);

    if(end_frame_position <= start_frame_position)
    {
        RejectVendorF6Command();
        return;
    }

    if((command_flags & 4u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14));

    if((command_flags & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 1u));
        CompleteVendorF6Command();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    }

    ResetVendorF6RawReadState();
    InitializeVendorF6Coroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    YieldVendorF6Coroutine0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    ExecuteVendorF6Acquisition();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}

/* RAM:fbd873 */
void HandleScsiOpcodeF2(void)
{
    u8 command_flags;
    u8 transfer_status;
    u16 response_limit;
    u16 requested_count;
    u16 byte_count;

    if(ValidateVendorF2Media() == 0u)
        return;

    ClearVendorF2MediaFlags();
    command_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B);
    if((command_flags & 0x10u) != 0u)
    {
        OrBigEndianU16(FW_NEAR_UNKNOWN_0FF5, (u16)((command_flags & 4u) != 0u ? 0x0800u : 0x0400u));
        OrBigEndianU16(FW_NEAR_UNKNOWN_0FF5, 0x1000u);
    }
    else if((command_flags & 8u) != 0u)
    {
        OrBigEndianU16(FW_NEAR_UNKNOWN_0FF5, (u16)((command_flags & 4u) != 0u ? 0x0200u : 0x0100u));
        OrBigEndianU16(FW_NEAR_UNKNOWN_0FF5, 0x1000u);
    }
    else if((command_flags & 4u) != 0u)
    {
        OrBigEndianU16(FW_NEAR_UNKNOWN_0FF5, 0x0080u);
        OrBigEndianU16(FW_NEAR_UNKNOWN_0FF5, 0x1000u);
    }

    if((command_flags & 1u) != 0u)
    {
        NormalizeVendorF2WriteSpeed();
        ResetVendorF2RawReadState();
        InitializeVendorF2Coroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        YieldVendorF2Coroutine0();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
        if((command_flags & 2u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) | 8u));
        transfer_status = RunVendorF2Opc();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xf7u));
        if(transfer_status == 0u)
            return;
    }

    requested_count = ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        response_limit = 0x0176u;
        BuildVendorF2FullResponse();
    }
    else
    {
        response_limit = 0x008du;
        BuildVendorF2ShortResponse();
    }
    byte_count = MinU16(requested_count, response_limit);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 2u) || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 6u)))
        transfer_status = TransferVendorF2Direct((u32)byte_count);
    else
        transfer_status = TransferVendorF2Buffer(FW_NEAR_UNKNOWN_0B2A, (u32)byte_count);
    if(transfer_status != 4u)
        SetVendorF2NoSense();
}

/* RAM:fd6bfe */
void HandleScsiOpcodeD8(void)
{
    u8 invalid_fields = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B15) & 0xfcu);
    u8 format;
    u32 start_frame;
    u32 end_frame;

    invalid_fields = (u8)(invalid_fields | (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu));
    if(invalid_fields != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    format = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14);
    if(format > 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) != 8u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if(ReadBit000DForVendorD8() != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    start_frame = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B0C) + 150ul;
    end_frame = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B10) + start_frame;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) == 8u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, 2u);
    ClearVendorD8RawReadModeBitsAtomic();
    SetBit0046ForReadCommandAtomic();
    ExecuteRawSectorReadRange(start_frame, end_frame, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14));
    ClearBit0046ForReadCommandAtomic();
}

/* RAM:fd6c6b */
void HandleScsiOpcodeD9(void)
{
    u8 invalid_fields = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu);
    s32 start_frame;
    s32 end_frame;

    invalid_fields = (u8)(invalid_fields | FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C));
    invalid_fields = (u8)(invalid_fields | FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10));
    invalid_fields = (u8)(invalid_fields | (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) & 0xfcu));
    invalid_fields = (u8)(invalid_fields | (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B15) & 0xfcu));
    if(invalid_fields != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    if(IsBinaryMsfTupleValid(FW_NEAR_UNKNOWN_0B0D) == 0u || IsBinaryMsfTupleValid(FW_NEAR_UNKNOWN_0B11) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if(ReadBit000DForVendorD8() != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    start_frame = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_0B0D);
    end_frame = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_0B11);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, 0u);
    ClearVendorD8RawReadModeBitsAtomic();
    SetBit0046ForReadCommandAtomic();
    ExecuteRawSectorReadRange((u32)start_frame, (u32)end_frame, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14));
    ClearBit0046ForReadCommandAtomic();
}

/* RAM:fd709c */
u8 PLEXTOR_NEAR_FUNCTION ValidateRawSectorReadFrameRangeAgainstMediaBounds(s32 start_frame_position, s32 end_frame_position)
{
    s32 upper_bound;

    if(start_frame_position > end_frame_position)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) != 0xd8u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x13u) == 2u)
        {
            SetSenseData(5u, 0x6400u);
            return 0u;
        }
        upper_bound = (s32)(ReadBigEndianU32(FW_NEAR_UNKNOWN_12C1) + 100ul);
        if(upper_bound < 4500l && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) > 1u)
            upper_bound = 359999l;
    }
    else
    {
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) >> 4) & 1u) != 0u && start_frame_position <= 224l)
        {
            SetSenseData(5u, 0x2100u);
            SetSenseInformation(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B02));
            return 0u;
        }
        if((s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_12C1) > (s32)ReadBigEndianU32(FW_NEAR_CURRENT_FRAME_POSITION))
            upper_bound = (s32)(ReadBigEndianU32(FW_NEAR_UNKNOWN_12C1) + 100ul);
        else
            upper_bound = (s32)(ReadBigEndianU32(FW_NEAR_CURRENT_FRAME_POSITION) + 100ul);
    }

    if(end_frame_position > upper_bound)
    {
        SetSenseData(5u, 0x2100u);
        return 0u;
    }
    return 1u;
}

/* RAM:fd7199 */
void HandleScsiOpcodeDF(void)
{
    u32 transfer_block_count;
    u8 transfer_status;

    if(ValidateVendorDfCdb(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E4A)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 4u)
    {
        SetCommandSequenceErrorSense();
        return;
    }
    if(ReadVendorDfAudioStatus(0u) != 0x11u)
    {
        SetSenseData(5u, 0x6400u);
        return;
    }

    transfer_block_count = ReadBigEndianU32(FW_NEAR_UNKNOWN_0B10);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1455, transfer_block_count);
    if(transfer_block_count == 0ul)
    {
        SetNoSenseData();
        return;
    }

    WriteBigEndianU32(FW_NEAR_UNKNOWN_1459, 0ul);
    do
    {
        transfer_status = PollVendorDfTransfer();
        if(transfer_status == 0xfeu)
            YieldVendorDfCoroutine0();
    } while(transfer_status == 0xfeu);

    if(transfer_status == 1u)
    {
        SetNoSenseData();
        return;
    }

    ResetVendorDfRawReadState();
    InitializeVendorDfCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    YieldVendorDfCoroutine0();
}

/* RAM:fd721a */
u8 PLEXTOR_NEAR_FUNCTION PollVendorDfSubchannelTransfer(void)
{
    u8 producer_error;
    u16 descriptor_address;
    u16 subchannel_slot;
    u32 total_block_count;
    u32 completed_block_count;
    u32 controller_buffer_address;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        u16 asc_ascq;
        u8 sense_key;

        producer_error = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        asc_ascq = ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + ((u32)producer_error * 2ul));
        producer_error = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        sense_key = FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + (u32)producer_error);
        SetSenseData(sense_key, asc_ascq);
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 0u)
    {
        SetSenseData(5u, 0xbf00u);
        return 0u;
    }

    if(FirmwareReadBit000E() != 0u)
    {
        if(ReadBigEndianU16(FW_NEAR_UNKNOWN_0F4E) == 0u)
            return 0xfeu;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) == 2u && GetLatestQueuedRawSectorDescriptorAddress() == 0u)
    {
        SetSenseData(5u, 0x6400u);
        return 0u;
    }

    descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
    SetVendorDfSubchannelTransferBit();
    subchannel_slot = ReadBigEndianU16((fw_addr_t)descriptor_address + 8u);
    controller_buffer_address = ReadBigEndianU32(FW_NEAR_UNKNOWN_1566) + (u32)subchannel_slot * 0x60ul;
    DispatchExecuteControllerDataInTransfer(controller_buffer_address, 0x60ul);

    total_block_count = ReadBigEndianU32(FW_NEAR_UNKNOWN_1455);
    completed_block_count = ReadBigEndianU32(FW_NEAR_UNKNOWN_1459);
    return completed_block_count == total_block_count ? 1u : 0xfeu;
}
