#include "read_toc_format3_pma.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

static u16 ReadPmaResponseWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadPmaControllerOffset(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePmaResponseWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u8 SwapPmaControlAdrNibbles(u8 value)
{
    return (u8)((value << 4) | (value >> 4));
}

static void ClearPmaResponseDescriptor(void)
{
    u8 index;

    for(index = 0u; index < 11u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A + index, 0u);
}

static u32 StagePmaResponseDescriptor(u32 controller_offset)
{
    return CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, 11ul);
}

static u32 StagePmaAdr2Descriptor(u32 controller_offset)
{
    ClearPmaResponseDescriptor();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, SwapPmaControlAdrNibbles(FirmwareRead8(FW_HUGE_UNKNOWN_2BB5DB)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, BinaryByteToPackedBcd(FirmwareRead8(FW_HUGE_UNKNOWN_2BB5DE)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, BinaryByteToPackedBcd(FirmwareRead8(FW_HUGE_UNKNOWN_2BB5DF)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, BinaryByteToPackedBcd(FirmwareRead8(FW_HUGE_UNKNOWN_2BB5E0)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, FirmwareRead8(FW_HUGE_UNKNOWN_2BB5E3));
    return StagePmaResponseDescriptor(controller_offset);
}

static u32 StagePmaAdr3Or4Descriptor(u32 controller_offset, fw_addr_t record)
{
    ClearPmaResponseDescriptor();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, FirmwareRead8(record + 3u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, SwapPmaControlAdrNibbles(FirmwareRead8(record)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareRead8(record + 2u));
    return StagePmaResponseDescriptor(controller_offset);
}

static u32 StagePmaAdr5Or6Or1Descriptor(u32 controller_offset, fw_addr_t record)
{
    ClearPmaResponseDescriptor();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, FirmwareRead8(record + 3u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, FirmwareRead8(record + 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareRead8(record + 5u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, FirmwareRead8(record + 7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, FirmwareRead8(record + 8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B34, FirmwareRead8(record + 9u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, SwapPmaControlAdrNibbles(FirmwareRead8(record)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareRead8(record + 2u));
    return StagePmaResponseDescriptor(controller_offset);
}

static u8 CountPmaResponseDescriptors(void)
{
    u8 count = 0u;
    u8 index = 1u;
    u8 record_count;

    while(index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_1149))
    {
        if(FirmwareRead8(FW_HUGE_UNKNOWN_2BB03D + (fw_addr_t)(index * 10u)) != 0u)
            count = (u8)(count + 1u);
        index = (u8)(index + 1u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) == 1u)
        count = (u8)(count + 1u);

    record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114A);
    if(record_count != 0u)
        count = (u8)(count + FirmwareReadNear8(FW_NEAR_UNKNOWN_114A));
    record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114B);
    if(record_count != 0u)
        count = (u8)(count + FirmwareReadNear8(FW_NEAR_UNKNOWN_114B));
    record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114C);
    if(record_count != 0u)
        count = (u8)(count + FirmwareReadNear8(FW_NEAR_UNKNOWN_114C));
    record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114D);
    if(record_count != 0u)
        count = (u8)(count + FirmwareReadNear8(FW_NEAR_UNKNOWN_114D));
    return count;
}

static u32 StagePmaRecordFamily(u32 controller_offset, fw_addr_t record_base, fw_addr_t count_address, u8 short_record)
{
    u8 index = 1u;

    while(index <= FirmwareRead8(count_address))
    {
        fw_addr_t record = record_base + (fw_addr_t)(index * 10u);

        if(short_record != 0u)
            controller_offset = StagePmaAdr3Or4Descriptor(controller_offset, record);
        else
            controller_offset = StagePmaAdr5Or6Or1Descriptor(controller_offset, record);
        index = (u8)(index + 1u);
    }
    return controller_offset;
}

/* RAM:f9ab19 */
void PLEXTOR_NEAR_FUNCTION HandleReadTocFormat3Pma(void)
{
    u16 allocation_length;
    u16 response_length;
    u8 descriptor_count;
    u8 index;
    u32 controller_offset;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    allocation_length = ReadPmaResponseWord(FW_NEAR_UNKNOWN_0B11);
    if(allocation_length == 0u)
    {
        SetNoSenseData();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);
        response_length = MinU16(allocation_length, 4u);
        if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, (u32)response_length) != 4u)
            SetNoSenseData();
        return;
    }

    descriptor_count = CountPmaResponseDescriptors();
    response_length = (u16)((u16)descriptor_count * 11u + 4u);
    WritePmaResponseWord(FW_NEAR_UNKNOWN_0B2A, (u16)(response_length - 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);

    controller_offset = ReadPmaControllerOffset(FW_NEAR_UNKNOWN_155A);
    controller_offset = CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, 4ul);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) == 1u)
        controller_offset = StagePmaAdr2Descriptor(controller_offset);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) != 0u)
        controller_offset = StagePmaRecordFamily(controller_offset, FW_NEAR_UNKNOWN_2BB423, FW_NEAR_UNKNOWN_114A, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114B) != 0u)
        controller_offset = StagePmaRecordFamily(controller_offset, FW_NEAR_UNKNOWN_2BB4FF, FW_NEAR_UNKNOWN_114B, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) != 0u)
        controller_offset = StagePmaRecordFamily(controller_offset, FW_NEAR_UNKNOWN_2BB5E5, FW_NEAR_UNKNOWN_114C, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114D) != 0u)
        controller_offset = StagePmaRecordFamily(controller_offset, FW_NEAR_UNKNOWN_2BB77F, FW_NEAR_UNKNOWN_114D, 0u);

    index = 1u;
    while(index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_1149))
    {
        fw_addr_t record = FW_HUGE_UNKNOWN_2BB03B + (fw_addr_t)(index * 10u);

        if(FirmwareRead8(record + 2u) != 0u)
            controller_offset = StagePmaAdr5Or6Or1Descriptor(controller_offset, record);
        index = (u8)(index + 1u);
    }

    response_length = MinU16(allocation_length, response_length);
    if(ExecuteControllerDataInAndMapStatus((u32)response_length) != 4u)
        SetNoSenseData();
}
