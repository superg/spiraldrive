#include "read_subchannel_current_position.h"
#include "../platform/controller_buffer_atomic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

extern void YieldCoroutine0(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_subchannel_current_position_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static fw_addr_t CurrentPositionAddress(fw_addr_t base, u16 offset)
{
    return FirmwareAddressIncrementLow16(base, offset);
}

static u16 ReadCurrentPositionWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteCurrentPositionDword(fw_addr_t destination, u32 value)
{
    FirmwareWriteU32(destination, value);
}

static void WriteUnavailableCurrentPosition(fw_addr_t destination)
{
    u8 index;

    FirmwareWrite8(destination, 1u);
    for(index = 1u; index < 12u; ++index)
        FirmwareWrite8(CurrentPositionAddress(destination, index), 0u);
}

static void WriteCurrentPositionFields(fw_addr_t destination, u8 control_adr, u8 track_number, u8 index_number, fw_addr_t absolute_tuple, fw_addr_t relative_tuple)
{
    s32 frame_position;

    FirmwareWrite8(destination, 1u);
    FirmwareWrite8(CurrentPositionAddress(destination, 1u), (u8)((control_adr << 4) | (control_adr >> 4)));
    FirmwareWrite8(CurrentPositionAddress(destination, 2u), track_number);
    FirmwareWrite8(CurrentPositionAddress(destination, 3u), index_number);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u)
    {
        FirmwareWrite8(CurrentPositionAddress(destination, 4u), 0u);
        FirmwareWrite8(CurrentPositionAddress(destination, 5u), FirmwareRead8(absolute_tuple));
        FirmwareWrite8(CurrentPositionAddress(destination, 6u), FirmwareRead8(absolute_tuple + 1u));
        FirmwareWrite8(CurrentPositionAddress(destination, 7u), FirmwareRead8(absolute_tuple + 2u));
        FirmwareWrite8(CurrentPositionAddress(destination, 8u), 0u);
        FirmwareWrite8(CurrentPositionAddress(destination, 9u), FirmwareRead8(relative_tuple));
        FirmwareWrite8(CurrentPositionAddress(destination, 10u), FirmwareRead8(relative_tuple + 1u));
        FirmwareWrite8(CurrentPositionAddress(destination, 11u), FirmwareRead8(relative_tuple + 2u));
    }
    else
    {
        frame_position = ConvertBinaryMsfToAdjustedFramePosition(absolute_tuple) - 150l;
        WriteCurrentPositionDword(CurrentPositionAddress(destination, 4u), (u32)frame_position);
        frame_position = ConvertBinaryMsfToAdjustedFramePosition(relative_tuple);
        if(index_number == 0u)
            frame_position = -frame_position;
        WriteCurrentPositionDword(CurrentPositionAddress(destination, 8u), (u32)frame_position);
    }
}

static u8 EnsureCurrentPositionAvailable(void)
{
    s8 countdown;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1224, 50u);
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1224) > 0)
            YieldCoroutine0();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) | 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1224, 10u);
        do
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 4u) != 0u)
                break;
            countdown = (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1224);
            if(countdown < 0)
                break;
            YieldCoroutine0();
        } while(1);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfdu));
    }
    return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 4u) != 0u);
}

static u8 LoadQueuedCurrentPositionPacket(void)
{
    u16 descriptor = GetRawReadConsumerDescriptorAddressOrZero();
    u16 slot_index;
    fw_addr_t controller_offset;
    u8 index;
    u8 value;

    if(FirmwareRead8((fw_addr_t)descriptor + 0x10u) != 0u)
        return 0u;
    slot_index = ReadCurrentPositionWord((fw_addr_t)descriptor + 8u);
    controller_offset = FW_NEAR_UNKNOWN_156E + ((fw_addr_t)slot_index << 4);
    DisableExternalInterrupt0IfPipelineModeNonzero();
    value = ReadControllerBufferByteAtomic(controller_offset);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1245, value);
    if((value & 0x0fu) != 1u)
    {
        EnableExternalInterrupt0IfPipelineModeNonzero();
        return 0u;
    }
    for(index = 1u; index < 10u; ++index)
    {
        value = ReadControllerBufferByteAtomic(controller_offset + index);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1245 + index, PackedBcdByteToBinary(value));
    }
    EnableExternalInterrupt0IfPipelineModeNonzero();
    return 1u;
}

/* RAM:f98eea */
void PLEXTOR_NEAR_FUNCTION BuildReadSubChannelCurrentPositionPayloadC(fw_addr_t destination_address)
{
    if(READ_SUBCHANNEL_CAPTURE_BIT() == 0u || ReadCurrentPositionWord(FW_NEAR_UNKNOWN_1292) <= 8u)
    {
        if(EnsureCurrentPositionAvailable() == 0u)
        {
            WriteUnavailableCurrentPosition(destination_address);
            return;
        }
    }
    else if(LoadQueuedCurrentPositionPacket() == 0u)
    {
        WriteUnavailableCurrentPosition(destination_address);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C8C) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBC) == 0u)
        WriteCurrentPositionFields(destination_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAF2), FirmwareReadNear8(FW_NEAR_UNKNOWN_FAF3), FirmwareReadNear8(FW_NEAR_UNKNOWN_FAF4), FW_NEAR_UNKNOWN_FAF9,
            FW_NEAR_UNKNOWN_FAF5);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1246) == 0xaau)
        WriteCurrentPositionFields(destination_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_F578), FirmwareReadNear8(FW_NEAR_UNKNOWN_F579), FirmwareReadNear8(FW_NEAR_UNKNOWN_F57A), FW_NEAR_UNKNOWN_F57F,
            FW_NEAR_UNKNOWN_F57B);
    else
        WriteCurrentPositionFields(destination_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_1245), FirmwareReadNear8(FW_NEAR_UNKNOWN_1246), FirmwareReadNear8(FW_NEAR_UNKNOWN_1247), FW_NEAR_UNKNOWN_124C,
            FW_NEAR_UNKNOWN_1248);
}
