#include "type2_leadin_toc_sequence.h"
#include "encoding.h"
#include "firmware_memory.h"

static u32 ReadType2LeadInU32LowWordFirst(fw_addr_t address)
{
    u16 low = (u16)((u16)FirmwareRead8(address + 2u) << 8);
    u16 high;

    low |= FirmwareRead8(address + 3u);
    high = (u16)((u16)FirmwareRead8(address) << 8);
    high |= FirmwareRead8(address + 1u);
    return ((u32)high << 16) | low;
}

static fw_addr_t BeginType2LeadInRecord(u8 write_full_toc_descriptors, u8 record_number)
{
    if(write_full_toc_descriptors != 0u)
    {
        fw_addr_t descriptor = FW_HUGE_UNKNOWN_2B8000 + (u16)record_number * 11u;

        FirmwareWrite8(descriptor, 1u);
        return descriptor + 1u;
    }
    return 0x00008012ul + (u16)(record_number - 1u) * 10u;
}

static void WriteType2LeadInRecordPrefix(fw_addr_t record, u8 control_adr, u8 point)
{
    FirmwareWrite8(record, control_adr);
    FirmwareWrite8(record + 1u, 0u);
    FirmwareWrite8(record + 2u, point);
}

static void WriteZeroType2LeadInMsf(fw_addr_t record)
{
    FirmwareWrite8(record + 3u, 0u);
    FirmwareWrite8(record + 4u, 0u);
    FirmwareWrite8(record + 5u, 0u);
    FirmwareWrite8(record + 6u, 0u);
}

static void WriteType2LeadInTuple(fw_addr_t record, fw_addr_t source, u8 convert_to_bcd)
{
    u8 value;

    value = FirmwareRead8(source);
    if(convert_to_bcd != 0u)
        value = BinaryByteToPackedBcd(value);
    FirmwareWrite8(record + 7u, value);

    value = FirmwareRead8(source + 1u);
    if(convert_to_bcd != 0u)
        value = BinaryByteToPackedBcd(value);
    FirmwareWrite8(record + 8u, value);

    value = FirmwareRead8(source + 2u);
    if(convert_to_bcd != 0u)
        value = BinaryByteToPackedBcd(value);
    FirmwareWrite8(record + 9u, value);
}

static void ConvertType2LeadInTupleToBcd(fw_addr_t record)
{
    u8 value = FirmwareRead8(record + 7u);

    FirmwareWrite8(record + 7u, BinaryByteToPackedBcd(value));
    value = FirmwareRead8(record + 8u);
    FirmwareWrite8(record + 8u, BinaryByteToPackedBcd(value));
    value = FirmwareRead8(record + 9u);
    FirmwareWrite8(record + 9u, BinaryByteToPackedBcd(value));
}

static void WriteType2LeadInWorkspaceWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u8 ReadShiftedOrPair(fw_addr_t right_address, fw_addr_t left_address)
{
    u8 right = FirmwareRead8(right_address);
    u8 left = FirmwareRead8(left_address);

    return (u8)((u8)(left << 1) | (u8)(right << 1));
}

/* RAM:fa90de */
void BuildType2LeadInTocRecordSequence(u8 write_full_toc_descriptors)
{
    fw_addr_t record;
    u8 clear_index;

    for(clear_index = 0u; clear_index < 60u; ++clear_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_8012 + clear_index, 0u);

    record = BeginType2LeadInRecord(write_full_toc_descriptors, 1u);
    WriteType2LeadInRecordPrefix(record, 0x71u, 0xa0u);
    WriteZeroType2LeadInMsf(record);
    FirmwareWrite8(record + 7u, 1u);
    FirmwareWrite8(record + 8u, 0x20u);
    FirmwareWrite8(record + 9u, 0u);

    record = BeginType2LeadInRecord(write_full_toc_descriptors, 2u);
    WriteType2LeadInRecordPrefix(record, 0x71u, 0xa1u);
    WriteZeroType2LeadInMsf(record);
    FirmwareWrite8(record + 7u, 1u);
    FirmwareWrite8(record + 8u, 0u);
    FirmwareWrite8(record + 9u, 0u);

    record = BeginType2LeadInRecord(write_full_toc_descriptors, 3u);
    WriteType2LeadInRecordPrefix(record, 0x71u, 0xa2u);
    WriteZeroType2LeadInMsf(record);
    ConvertFramePositionToBinaryMsfTuple((s32)ReadType2LeadInU32LowWordFirst(FW_HUGE_UNKNOWN_2BA58D), record + 7u);
    if(write_full_toc_descriptors == 0u)
        ConvertType2LeadInTupleToBcd(record);

    record = BeginType2LeadInRecord(write_full_toc_descriptors, 4u);
    WriteType2LeadInRecordPrefix(record, 0x71u, 1u);
    WriteZeroType2LeadInMsf(record);
    FirmwareWrite8(record + 7u, 0u);
    FirmwareWrite8(record + 8u, 2u);
    FirmwareWrite8(record + 9u, 0u);

    record = BeginType2LeadInRecord(write_full_toc_descriptors, 5u);
    WriteType2LeadInWorkspaceWord(FW_NEAR_UNKNOWN_8008, 3u);
    WriteType2LeadInRecordPrefix(record, 0x75u, 0xb0u);
    FirmwareWrite8(record + 3u, 0xffu);
    FirmwareWrite8(record + 4u, 0xffu);
    FirmwareWrite8(record + 5u, 0xffu);
    FirmwareWrite8(record + 6u, 3u);
    WriteType2LeadInTuple(record, FW_NEAR_UNKNOWN_113B, (u8)(write_full_toc_descriptors == 0u));

    record = BeginType2LeadInRecord(write_full_toc_descriptors, 6u);
    WriteType2LeadInRecordPrefix(record, 0x75u, 0xc0u);
    FirmwareWrite8(record + 3u, ReadShiftedOrPair(FW_NEAR_UNKNOWN_FB7B, FW_NEAR_UNKNOWN_FB7A));
    FirmwareWrite8(record + 4u, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) << 1));
    FirmwareWrite8(record + 5u, ReadShiftedOrPair(FW_NEAR_UNKNOWN_FB7E, FW_NEAR_UNKNOWN_FB7D));
    FirmwareWrite8(record + 6u, 0u);
    WriteType2LeadInTuple(record, FW_NEAR_UNKNOWN_113F, (u8)(write_full_toc_descriptors == 0u));

    record = BeginType2LeadInRecord(write_full_toc_descriptors, 7u);
    WriteType2LeadInRecordPrefix(record, 0x75u, 0xc1u);
    FirmwareWrite8(record + 3u, ReadShiftedOrPair(FW_NEAR_UNKNOWN_FB80, FW_NEAR_UNKNOWN_FB7F));
    FirmwareWrite8(record + 4u, ReadShiftedOrPair(FW_NEAR_UNKNOWN_FB82, FW_NEAR_UNKNOWN_FB81));
    FirmwareWrite8(record + 5u, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB83) << 1));
    FirmwareWrite8(record + 6u, 0u);
    FirmwareWrite8(record + 7u, 0u);
    FirmwareWrite8(record + 8u, 0u);
    FirmwareWrite8(record + 9u, 0u);

    WriteType2LeadInWorkspaceWord(FW_NEAR_UNKNOWN_800A, 6u);
    WriteType2LeadInWorkspaceWord(FW_NEAR_UNKNOWN_800C, 7u);
}
