#include "raw_sector_batched_host_transfer.h"
#include "../media/raw_read_header_normalization.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/raw_sector_batched_host_transfer_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBatchU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadBatchU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
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

/* RAM:fd72d9 */
void PLEXTOR_NEAR_FUNCTION StartBatchedRawReadHostTransferFromSecondaryCursor(void)
{
    u16 descriptor_address;
    u16 slot_index;
    u8 format;
    u8 c2_selector;
    u32 transfer_address = 0ul;
    u32 transfer_value;

    descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
    c2_selector = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) >> 1) * 7u);
    ConfigureAtaDataTransferStatus();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    format = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);

    if(READ_BATCH_BIT_0045() == 1u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, c2_selector == 0u || format == 0u ? 0x80u : 0x82u);
    }
    else
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF95E2) + format + c2_selector));
        if(format != 3u)
        {
            if(format != 0u || c2_selector != 0u)
                WriteControllerTransferLength(0x0930u);
            transfer_address = ReadBatchU32((fw_addr_t)(u16)(descriptor_address + 4u)) + 0x110ul;
            WriteControllerTransferAddress(transfer_address);
        }
    }

    if(c2_selector != 0u)
    {
        WriteControllerTransferLength(0x0126u);
        transfer_address = ReadBatchU32((fw_addr_t)(u16)(descriptor_address + 10u)) + 0x22ul;
        WriteControllerTransferAddress(transfer_address);
    }

    if(format != 0u)
    {
        slot_index = ReadBatchU16((fw_addr_t)(u16)(descriptor_address + 8u));
        if(format == 1u)
        {
            transfer_address = ReadBatchU32(FW_NEAR_UNKNOWN_156E) + (u32)slot_index * 0x10ul;
            if(READ_BATCH_BIT_000D() != 0u)
            {
                CopyAndNormalizeRawReadHeaderToLinkedBuffer(transfer_address, 0x1au);
                transfer_address = ReadBatchU32(FW_NEAR_UNKNOWN_1556) - 0x20ul;
            }
            WriteControllerTransferLength(0x0010u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1463, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1464, 1u);
        }
        else if(format == 2u || format == 4u)
        {
            WriteControllerTransferLength(0x0060u);
            transfer_address = ReadBatchU32(FW_NEAR_UNKNOWN_1566) + (u32)slot_index * 0x60ul;
        }
        else if(format == 3u)
        {
            if(c2_selector != 0u)
                WriteControllerTransferLength(0x0060u);
            transfer_address = ReadBatchU32(FW_NEAR_UNKNOWN_1566) + (u32)slot_index * 0x60ul;
        }
        WriteControllerTransferAddress(transfer_address);
    }

    transfer_value = FUN_RAM_fde9cf(ReadBatchU16(FW_NEAR_UNKNOWN_145D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F47, 1u);
    StartRawReadHostTransferFromStackAddress(transfer_value);
}
