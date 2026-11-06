#include "raw_sector_descriptor_scratch.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/raw_sector_descriptor_scratch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRawSectorDescriptorU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRawSectorDescriptorU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteRawSectorDescriptorU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildSpecialRawSectorDescriptor(u16 descriptor_status, u8 descriptor_mode)
{
    WriteRawSectorDescriptorU16(FW_NEAR_UNKNOWN_12AA, (u16)ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_1262));
    WriteRawSectorDescriptorU32(FW_NEAR_UNKNOWN_12AC, ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_126A));
    WriteRawSectorDescriptorU16(FW_NEAR_UNKNOWN_12B0, descriptor_status);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B2, descriptor_mode);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B3, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F30));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B4, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A3));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B5, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2E));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B7, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31));
}

static void CopyCurrentQPacketToRawSectorDescriptor(void)
{
    u8 index;

    for(index = 0u; index < 10u; ++index)
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_12AC + index), FirmwareReadNear8((fw_addr_t)(FW_NEAR_UNKNOWN_1272 + index)));
}

static void RefreshSecondarySlotDescriptorFlag(void)
{
    u32 slot = ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_1266);
    u32 base = ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_156E);
    u8 metadata = READ_DESCRIPTOR_CONTROLLER_BYTE((u32)(base + (slot << 4) - 1ul));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, (u8)((metadata & 0x80u) != 0u));
}

static void RefreshMode2DescriptorFlag(void)
{
    u32 cursor = ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_126E);
    u8 metadata = READ_DESCRIPTOR_CONTROLLER_BYTE(cursor + 0x12ul);
    u8 flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_12B6);

    if((metadata & 0x20u) != 0u)
        flags |= 0x40u;
    else
        flags &= 0xbfu;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, flags);
}

static u8 MustSetRawSectorDescriptorTerminalFlag(void)
{
    u32 threshold;
    u32 target;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14D0) == 1u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14CF) != 1u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1281) != 2u)
        return 0u;
    threshold = ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_14D1);
    target = ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_1598);
    return (u8)((s32)target > (s32)threshold);
}

static void ApplyRawSectorDescriptorTerminalFlag(void)
{
    u8 latched_flag;
    u8 flags;

    if(MustSetRawSectorDescriptorTerminalFlag() != 0u)
    {
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_12B6);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, (u8)(flags | 0x80u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14D0, 0u);
        return;
    }

    latched_flag = FirmwareReadNear8(FW_NEAR_UNKNOWN_127C);
    flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_12B6);
    if(latched_flag != 0u)
        flags |= 0x80u;
    else
        flags &= 0x7fu;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, flags);
}

/* RAM:fe0000 */
void BuildRawSectorDescriptorScratch(u16 descriptor_status, u8 descriptor_mode)
{
    u32 virtual_position = ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_1598);
    u8 q_type;

    WriteRawSectorDescriptorU32(FW_NEAR_UNKNOWN_12A2, virtual_position);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) == 1u && READ_DESCRIPTOR_BIT_001F() != 0u)
        WriteRawSectorDescriptorU32(FW_NEAR_UNKNOWN_12A2, ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_0F00));
    WriteRawSectorDescriptorU32(FW_NEAR_UNKNOWN_12A6, ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_126E));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) == 1u)
    {
        BuildSpecialRawSectorDescriptor(descriptor_status, descriptor_mode);
        return;
    }

    q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);
    if(q_type == 1u)
        WriteRawSectorDescriptorU16(FW_NEAR_UNKNOWN_12AA, (u16)ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_1266));
    else
        WriteRawSectorDescriptorU16(FW_NEAR_UNKNOWN_12AA, (u16)ReadRawSectorDescriptorU32(FW_NEAR_UNKNOWN_1262));
    CopyCurrentQPacketToRawSectorDescriptor();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 1u)
    {
        RefreshSecondarySlotDescriptorFlag();
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) != 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 0x0du)
            RefreshMode2DescriptorFlag();
    }

    ApplyRawSectorDescriptorTerminalFlag();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B7, FirmwareReadNear8(FW_NEAR_UNKNOWN_1285));
}
