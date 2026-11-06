#include "raw_read_controller_transfer_batch.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_controller_transfer_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fed36f */
void PLEXTOR_NEAR_FUNCTION ConfigureRawReadControllerTransferBatch(fw_addr_t fallback_address, u16 block_count)
{
    u8 address_count;
    u8 index;
    u32 total_length;

    WriteBigEndianU32(FW_NEAR_UNKNOWN_1459, ReadBigEndianU32(FW_NEAR_UNKNOWN_1459) + block_count);
    PROGRAM_TRANSFER_ADDRESS_REGISTERS();
    address_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_1469);
    for(index = 0u; index < address_count; ++index)
    {
        u32 component_length = ReadBigEndianU32(FW_NEAR_UNKNOWN_1470 + (fw_addr_t)index * 4ul);
        u32 buffer_address;

        if(address_count > 1u)
        {
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)component_length);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(component_length >> 8));
        }
        buffer_address = SELECT_BUFFER_ADDRESS(component_length, fallback_address);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)buffer_address);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(buffer_address >> 8));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(buffer_address >> 16));
    }

    total_length = (u32)ReadBigEndianU16(FW_NEAR_UNKNOWN_146D) * block_count;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)total_length);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(total_length >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1462, 1u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
    if((FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 1u) == 0u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x80u);
    else if(total_length == 0x10000ul)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x82u);
    else
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0xd0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805E, 0x87u);
}
