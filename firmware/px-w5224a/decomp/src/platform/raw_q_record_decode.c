#include "raw_q_record_decode.h"
#include "encoding.h"
#include "firmware_memory.h"

/* Reconstruction helper; no direct Ghidra function address. */
static void CopyDecodedRawQByte(fw_addr_t source_record_address, fw_addr_t destination_record_address, u8 byte_offset)
{
    u8 packed_bcd = FirmwareRead8(FirmwareAddressIncrementLow16(source_record_address, byte_offset));

    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_record_address, byte_offset), PackedBcdByteToBinary(packed_bcd));
}

/* RAM:fabc8e */
void PLEXTOR_NEAR_FUNCTION CopyAndDecodeRawReadQRecordToFarTable(fw_addr_t source_record_address, fw_addr_t destination_record_address)
{
    FirmwareWrite8(destination_record_address, FirmwareRead8(source_record_address));
    CopyDecodedRawQByte(source_record_address, destination_record_address, 2u);
    CopyDecodedRawQByte(source_record_address, destination_record_address, 3u);
    CopyDecodedRawQByte(source_record_address, destination_record_address, 4u);
    CopyDecodedRawQByte(source_record_address, destination_record_address, 5u);
    CopyDecodedRawQByte(source_record_address, destination_record_address, 7u);
    CopyDecodedRawQByte(source_record_address, destination_record_address, 8u);
    CopyDecodedRawQByte(source_record_address, destination_record_address, 9u);
}

/* RAM:facae1 */
void PLEXTOR_NEAR_FUNCTION CopyAndDecodeRawTocPayloadToFullTocDescriptorBody(fw_addr_t source_payload_address, fw_addr_t destination_descriptor_body_address)
{
    FirmwareWrite8(destination_descriptor_body_address, FirmwareRead8(source_payload_address));
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_descriptor_body_address, 1u), 0u);
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_descriptor_body_address, 2u), FirmwareRead8(FirmwareAddressIncrementLow16(source_payload_address, 2u)));
    CopyDecodedRawQByte(source_payload_address, destination_descriptor_body_address, 3u);
    CopyDecodedRawQByte(source_payload_address, destination_descriptor_body_address, 4u);
    CopyDecodedRawQByte(source_payload_address, destination_descriptor_body_address, 5u);
    CopyDecodedRawQByte(source_payload_address, destination_descriptor_body_address, 6u);
    CopyDecodedRawQByte(source_payload_address, destination_descriptor_body_address, 7u);
    CopyDecodedRawQByte(source_payload_address, destination_descriptor_body_address, 8u);
    CopyDecodedRawQByte(source_payload_address, destination_descriptor_body_address, 9u);
}
