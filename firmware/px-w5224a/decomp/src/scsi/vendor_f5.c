#include "vendor_f5.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "vendor_f5_media_strategy_response.h"
#include "vendor_f5_response.h"

#define VENDOR_F5_CDB FW_NEAR_HOST_COMMAND_OPCODE
#define VENDOR_F5_RESPONSE FW_NEAR_UNKNOWN_0B2A

static u16 ReadVendorF5U16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadVendorF5U32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteVendorF5U16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static fw_addr_t GetVendorF5RawDescriptorPayloadAddress(void)
{
    u16 descriptor = GetRawReadConsumerDescriptorAddressOrZero();
    u16 bank_pointer = ReadVendorF5U16((fw_addr_t)descriptor + 2u);
    u16 high_word = ReadVendorF5U16(bank_pointer);

    return (((u32)high_word << 16) | descriptor) + 4ul;
}

static void ConsumeVendorF5RetainedResponseIfRequested(u8 selector)
{
    if(selector == 8u && (FirmwareRead8(VENDOR_F5_CDB + 4u) & 1u) != 0u)
    {
        WriteVendorF5U16(FW_NEAR_UNKNOWN_FBD7, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD9, 0u);
    }
}

/* RAM:fcb61f */
void HandleScsiOpcodeF5(void)
{
    u8 selector = FirmwareRead8(VENDOR_F5_CDB + 3u);
    fw_addr_t source_address = 0ul;
    u16 response_length = 0u;
    u16 transfer_length;
    u8 transfer_status;
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f5_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    switch(selector)
    {
    case 0x00u:
        source_address = ReadVendorF5U32(VENDOR_F5_CDB + 4u);
        response_length = ReadVendorF5U16(VENDOR_F5_CDB + 8u);
        break;
    case 0x01u:
        source_address = FW_NEAR_UNKNOWN_FB6A;
        response_length = 12u;
        break;
    case 0x02u:
        HandleUnsupportedScsiCommand();
        return;
    case 0x03u:
        FirmwareWrite8(local_response_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A));
        FirmwareWrite8(local_response_address + 1u, 0u);
        source_address = local_response_address;
        response_length = 2u;
        break;
    case 0x04u:
        source_address = GetVendorF5RawDescriptorPayloadAddress();
        response_length = 4u;
        break;
    case 0x05u:
        source_address = FW_NEAR_UNKNOWN_DD2F;
        response_length = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) * 8u + 63u);
        break;
    case 0x06u:
        source_address = FW_NEAR_UNKNOWN_FB7A;
        response_length = 18u;
        break;
    case 0x07u:
        source_address = FW_NEAR_UNKNOWN_116E;
        response_length = 92u;
        break;
    case 0x08u:
        if((FirmwareRead8(VENDOR_F5_CDB + 4u) & 2u) != 0u)
        {
            response_length = 4u;
            WriteVendorF5U16(VENDOR_F5_RESPONSE, ReadVendorF5U16(FW_NEAR_UNKNOWN_FBD7));
            WriteVendorF5U16(VENDOR_F5_RESPONSE + 2u, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD9));
        }
        break;
    case 0x09u:
        BuildVendorF5Selector09ControllerStateResponse();
        return;
    case 0x0au:
        BuildVendorF5MediaStrategyResponse();
        return;
    case 0x0bu:
        TransferFixedResponse52Bytes();
        return;
    case 0x80u:
        source_address = FW_NEAR_UNKNOWN_E806;
        response_length = 0x06a4u;
        break;
    case 0x81u:
        source_address = FW_NEAR_UNKNOWN_F157;
        response_length = 0x0060u;
        break;
    default:
        HandleUnsupportedScsiCommand();
        return;
    }

    if((0x001affcdul - ReadVendorF5U32(FW_NEAR_UNKNOWN_155A)) < (u32)response_length)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    transfer_length = MinU16(ReadVendorF5U16(VENDOR_F5_CDB + 8u), response_length);
    if(transfer_length == 0u)
    {
        ConsumeVendorF5RetainedResponseIfRequested(selector);
        SetNoSenseData();
        return;
    }

    transfer_status = CopyResponseToControllerBufferAndExecuteDataIn(selector == 8u ? VENDOR_F5_RESPONSE : source_address, (u32)transfer_length);
    if(transfer_status != 4u)
    {
        ConsumeVendorF5RetainedResponseIfRequested(selector);
        SetNoSenseData();
    }
}
