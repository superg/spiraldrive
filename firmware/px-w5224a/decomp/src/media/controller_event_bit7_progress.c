#include "controller_event_bit7_progress.h"
#include "platform/firmware_memory.h"
#include "platform/mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/controller_event_bit7_progress_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadEventBit7U16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadEventBit7U32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteEventBit7U16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteEventBit7U32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:f93946 */
void HandleControllerEventBit7TransferProgress(void)
{
    u16 current_progress;
    u16 previous_progress;
    u16 descriptor_count;
    u16 descriptor_address;
    u16 high_water;
    s32 countdown;

    if(FirmwareReadBit000E() == 0u)
    {
        ResetControllerEventBit7TransferState();
        return;
    }

    high_water = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01807A);
    current_progress = (u16)((high_water << 8) | FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018079));
    if(ReadEventBit7U16(FW_NEAR_UNKNOWN_1292) == 0u)
    {
        countdown = (s32)(ReadEventBit7U32(FW_NEAR_UNKNOWN_0F36) - 1ul);
        WriteEventBit7U32(FW_NEAR_UNKNOWN_0F36, (u32)countdown);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807C, 2u);
        FirmwareWriteBit001A(1u);
        return;
    }

    if(FirmwareReadBit001A() != 0u && ReadEventBit7U16(FW_NEAR_UNKNOWN_1292) != 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807C, 0u);
        FirmwareWriteBit001A(0u);
    }

    if(ReadEventBit7U16(FW_NEAR_UNKNOWN_1292) == 0u)
        return;

    previous_progress = ReadEventBit7U16(FW_NEAR_UNKNOWN_0F5A);
    if(current_progress == previous_progress)
        return;
    previous_progress = ReadEventBit7U16(FW_NEAR_UNKNOWN_0F5A);
    if(current_progress < previous_progress)
        descriptor_count = (u16)((u16)ReadEventBit7U32(FW_NEAR_UNKNOWN_156A) + current_progress - previous_progress);
    else
        descriptor_count = (u16)(current_progress - previous_progress);
    WriteEventBit7U16(FW_NEAR_UNKNOWN_0F5A, current_progress);

    if(ReadEventBit7U16(FW_NEAR_UNKNOWN_0F4E) < ReadEventBit7U16(FW_NEAR_UNKNOWN_129C))
        WriteEventBit7U16(FW_NEAR_UNKNOWN_0F4E, (u16)(ReadEventBit7U16(FW_NEAR_UNKNOWN_0F4E) + 1u));

    descriptor_address = GetRawReadConsumerDescriptorAddressOrZero();
    WriteEventBit7U32(FW_NEAR_UNKNOWN_0F32, ReadEventBit7U32(descriptor_address) + 3ul);
    WriteEventBit7U16(FW_NEAR_UNKNOWN_0F5C, ReadEventBit7U16((fw_addr_t)descriptor_address + 8u));
    ConsumeRawReadDescriptors(descriptor_count);

    if(FirmwareRead8((fw_addr_t)descriptor_address + 17u) != 0u)
    {
        ResetControllerPort8087And808A808BUnlessFlag0E();
        ClearControllerPorts8088And8089UnlessFlag0E();
    }

    if(FirmwareRead8((fw_addr_t)descriptor_address + 16u) == 0u)
    {
        countdown = (s32)ReadEventBit7U32(FW_NEAR_UNKNOWN_0F36);
        WriteEventBit7U32(FW_NEAR_UNKNOWN_0F36, (u32)countdown - 1ul);
        if(countdown > 0l)
            return;
        ResetControllerEventBit7TransferState();
        WriteEventBit7U16(FW_NEAR_UNKNOWN_0F4E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
        return;
    }

    ResetControllerEventBit7TransferState();
    WriteEventBit7U16(FW_NEAR_UNKNOWN_0F4E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
}
