#include "controller_buffer_transfer.h"
#include "arithmetic.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_buffer_transfer_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteControllerTransferAddress(u8 selector, u32 address)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806F, selector);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018070, (u8)address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018071, (u8)(address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018072, (u8)(address >> 16));
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32Far(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32Far(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:faaa8c */
void TransferControllerBufferSectorsAndWait(u32 first_controller_address, u32 second_controller_address, u8 sector_count)
{
    u8 controller_control = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) | 0x04u);
    u32 byte_count;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, controller_control);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807E, controller_control);
    WriteControllerTransferAddress(0u, first_controller_address);
    WriteControllerTransferAddress(1u, second_controller_address);

    byte_count = MultiplyU32Low(0x0498ul, (u32)sector_count);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018073, (u8)byte_count);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018074, (u8)(byte_count >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018075, (u8)(byte_count >> 16));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C0, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806F, 0x80u);
    while(CONTROLLER_TRANSFER_COMPLETE() == 0u)
    {
    }
}

/* RAM:faab68 */
void TransferControllerBufferWordsAndWait(u32 first_controller_address, u32 second_controller_address, u32 controller_word_count)
{
    u8 controller_control = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) | 0x04u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, controller_control);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807E, controller_control);
    WriteControllerTransferAddress(0u, first_controller_address);
    WriteControllerTransferAddress(1u, second_controller_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018073, (u8)controller_word_count);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018074, (u8)(controller_word_count >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018075, (u8)(controller_word_count >> 16));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C0, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806F, 0x80u);
    while(CONTROLLER_TRANSFER_COMPLETE() == 0u)
    {
    }
}

/* RAM:fc766a */
void CopyControllerBufferRecordPageToScratch(void)
{
    u16 low_word = FirmwareReadU16(FW_HUGE_UNKNOWN_2BA51E);
    u16 high_word = FirmwareReadU16(FW_HUGE_UNKNOWN_2BA51C);
    fw_addr_t source_page;
    u16 record_index;

    high_word = (u16)(high_word + 0x10u);
    source_page = (((fw_addr_t)high_word << 16) | low_word) & FW_24BIT_ADDRESS_MASK;

    for(record_index = 0u; record_index < 256u; ++record_index)
    {
        u16 record_offset = (u16)(record_index * 6u);
        u8 field;

        for(field = 0u; field < 6u; ++field)
        {
            u16 byte_offset = (u16)(record_offset + field);

            FirmwareWrite8(FirmwareAddressIncrement24(FW_HUGE_UNKNOWN_2B971C, byte_offset), FirmwareRead8(FirmwareAddressIncrement24(source_page, byte_offset)));
        }
    }
}

/* RAM:fcfe05 */
void StartWriteControllerBufferTransfer(u8 block_count, u32 controller_buffer_address)
{
    u32 byte_count = (u32)block_count << 11;

    WriteBigEndianU32Far(FW_HUGE_UNKNOWN_2BA5D4, ReadBigEndianU32Far(FW_HUGE_UNKNOWN_2BA5D4) + (u32)block_count);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C6, (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C6) + block_count));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C7, FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C6));

    InitializeWriteControllerBufferTransferRegisters();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)controller_buffer_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(controller_buffer_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(controller_buffer_address >> 16));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)byte_count);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(byte_count >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1462, 1u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);

    if((FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 0x01u) == 0u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x88u);
    else if(block_count < 0x20u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0xd8u);
    else
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x8au);

    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805E, 0x87u);
}
