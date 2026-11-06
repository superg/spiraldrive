#include "raw_sector_transfer.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_sector_transfer_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRawSectorTransferU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadRawSectorTransferU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRawSectorTransferU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRawSectorTransferU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void WriteControllerTransferLength(u16 byte_count)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)byte_count);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(byte_count >> 8));
}

static void WriteControllerTransferAddress(u32 address)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(address >> 16));
}

static void WriteControllerTransferCommand(u8 command)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, command);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
}

static u32 BuildInvalidLayoutSenseInformation(void)
{
    u16 indirect_address = ReadRawSectorTransferU16(FW_NEAR_UNKNOWN_6402);
    u32 value = (u32)ReadRawSectorTransferU16(indirect_address) << 16;

    value |= 0x6400ul;
    value -= ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_0F0C);
    return value - 150ul;
}

static u8 RejectRawSectorLayout(void)
{
    RAW_SECTOR_TRANSFER_SET_SENSE(5u, 0x6400u);
    /* The canonical post-call register flow reads RAM:6402 indirectly. */
    RAW_SECTOR_TRANSFER_SET_INFORMATION(BuildInvalidLayoutSenseInformation());
    return 0u;
}

static u8 IsNormalRawSectorLayoutInvalid(u8 descriptor_type, u16 block_size)
{
    if(descriptor_type == 1u || descriptor_type == 3u)
        return (u8)(block_size < 0x0914u && block_size != 0x0800u);
    if(descriptor_type == 2u)
        return (u8)(block_size == 0x0914u);
    if(descriptor_type == 0u)
        return (u8)(block_size == 0x0808u || block_size == 0x0914u);
    return 0u;
}

static u32 AdjustNormalRawSectorAddress(u32 address, u16 block_size, u8 descriptor_type)
{
    if(block_size == 0x0800u)
        return address + (descriptor_type == 0u ? 0x10ul : 0x18ul);
    if(block_size == 0x0808u || block_size == 0x0920u)
        return address + 0x10ul;
    if(block_size == 0x0804u || block_size == 0x080cu || block_size == 0x0924u)
        return address + 0x0cul;
    if(block_size == 0x0914u || block_size == 0x0918u)
        return address + 0x18ul;
    return address;
}

static u16 ConfigureNormalRawSectorTransfer(u16 descriptor_address, u8 descriptor_type)
{
    u16 block_size = ReadRawSectorTransferU16(FW_NEAR_UNKNOWN_145D);
    u32 address;

    RAW_SECTOR_TRANSFER_CONFIGURE_ATA();
    WriteControllerTransferCommand(block_size == 0x0a56u ? 0x8au : 0x88u);
    address = ReadRawSectorTransferU32((fw_addr_t)(u16)(descriptor_address + 4u));
    WriteControllerTransferLength(block_size == 0x0a56u ? 0x0930u : block_size);
    address = AdjustNormalRawSectorAddress(address, block_size, descriptor_type);
    if(block_size == 0x0a56u)
    {
        WriteControllerTransferAddress(address);
        WriteControllerTransferLength(0x0126u);
        address = ReadRawSectorTransferU32((fw_addr_t)(u16)(descriptor_address + 10u));
    }
    WriteControllerTransferAddress(address);
    return block_size;
}

static u8 ValidateRawSectorSequence(u8 descriptor_type)
{
    u8 expected_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0A);

    if(expected_type != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F09, 0u);
        if(expected_type == 1u || descriptor_type != (u8)(expected_type - 2u))
            return 0u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F09) == 1u)
    {
        s32 difference = (s32)(ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_0B02) + 150ul - ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_0F14));
        if(difference >= -4l && difference <= 4l && difference != 0l)
        {
            RAW_SECTOR_TRANSFER_SET_SENSE(3u, 0x1500u);
            return 2u;
        }
    }
    return 1u;
}

static u16 ConfigureFormattedRawSectorTransfer(u16 descriptor_address, u8 descriptor_type, u8 *terminal_result)
{
    u8 option_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) >> 3);
    u8 layout_index = (u8)(((u8)(descriptor_type & 0xfeu) << 4) + option_index);
    u8 c2_option = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) * 5u);
    u8 sequence_status;
    u8 main_present;
    u8 format_index;
    u16 byte_count;
    u32 address;

    if(FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8EBA) + layout_index) == 0u)
    {
        *terminal_result = RejectRawSectorLayout();
        return 0u;
    }

    sequence_status = ValidateRawSectorSequence(descriptor_type);
    if(sequence_status == 0u)
    {
        *terminal_result = RejectRawSectorLayout();
        return 0u;
    }
    if(sequence_status == 2u)
    {
        *terminal_result = 1u;
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F09, (u8)(descriptor_type + 2u));
    WriteRawSectorTransferU32(FW_NEAR_UNKNOWN_0F14, ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_0B02) + ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_0B06) + 150ul);
    address = ReadRawSectorTransferU32((fw_addr_t)(u16)(descriptor_address + 4u));
    address += FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8EFA) + layout_index);
    byte_count = ReadRawSectorTransferU16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8F3A) + (fw_addr_t)descriptor_type * 64ul + (fw_addr_t)option_index * 2ul);
    format_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);
    if(byte_count == 0u)
    {
        if(format_index == 0u && c2_option == 0u)
        {
            RAW_SECTOR_TRANSFER_NO_SENSE();
            *terminal_result = 0u;
            return 0u;
        }
        main_present = 0u;
    }
    else
    {
        if(byte_count < 6u)
            WriteRawSectorTransferU16(FW_NEAR_UNKNOWN_1463, 1u);
        main_present = 5u;
    }

    RAW_SECTOR_TRANSFER_CONFIGURE_ATA();
    WriteControllerTransferCommand(FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF903A) + format_index + main_present + c2_option));
    if(byte_count != 0u)
    {
        WriteControllerTransferLength(byte_count);
        WriteControllerTransferAddress(address);
    }
    if(c2_option != 0u)
    {
        WriteControllerTransferLength(0x0126u);
        address = ReadRawSectorTransferU32((fw_addr_t)(u16)(descriptor_address + 10u));
        WriteControllerTransferAddress(address);
        byte_count = (u16)(byte_count + 0x0126u);
    }
    if(format_index != 0u)
    {
        if(format_index == 2u)
        {
            byte_count = (u16)(byte_count + 0x10u);
            RAW_SECTOR_TRANSFER_NORMALIZE(ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_156E) + (u32)option_index * 0x10ul, 0x1au);
            address = ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_1556) - 0x20ul;
            if(main_present != 0u || c2_option != 0u)
                WriteControllerTransferLength(0x10u);
            WriteRawSectorTransferU16(FW_NEAR_UNKNOWN_1463, 1u);
        }
        else
        {
            byte_count = (u16)(byte_count + 0x60u);
            address = ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_1566) + (u32)option_index * 0x60ul;
            if(main_present != 0u || c2_option != 0u)
                WriteControllerTransferLength(0x60u);
        }
        WriteControllerTransferAddress(address);
    }
    return byte_count;
}

static u8 StartConfiguredRawSectorTransfer(u16 byte_count)
{
    u32 transfer_value = RAW_SECTOR_TRANSFER_PREPARE_SIZE(byte_count);
    u16 block_count = ReadRawSectorTransferU16(FW_NEAR_UNKNOWN_1463);

    WriteRawSectorTransferU32(FW_NEAR_UNKNOWN_1459, ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_1459) + block_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1462, 1u);
    RAW_SECTOR_TRANSFER_START(transfer_value);
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) != 0u)
        RAW_SECTOR_TRANSFER_YIELD();
    return 1u;
}

/* RAM:fc549f */
u8 ConfigureAndTransferRawReadSector(void)
{
    u16 descriptor_address = RAW_SECTOR_TRANSFER_GET_DESCRIPTOR();
    u8 descriptor_type = FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x13u));
    u16 byte_count;
    u8 terminal_result = 0xffu;

    if(RAW_SECTOR_TRANSFER_BIT000D() == 0u)
    {
        WriteRawSectorTransferU32(FW_NEAR_UNKNOWN_0F14, ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_0B02) + ReadRawSectorTransferU32(FW_NEAR_UNKNOWN_0B06) + 150ul);
        byte_count = ReadRawSectorTransferU16(FW_NEAR_UNKNOWN_145D);
        if(IsNormalRawSectorLayoutInvalid(descriptor_type, byte_count) != 0u)
            return RejectRawSectorLayout();
        byte_count = ConfigureNormalRawSectorTransfer(descriptor_address, descriptor_type);
    }
    else
    {
        byte_count = ConfigureFormattedRawSectorTransfer(descriptor_address, descriptor_type, &terminal_result);
        if(terminal_result != 0xffu)
            return terminal_result;
    }
    return StartConfiguredRawSectorTransfer(byte_count);
}
