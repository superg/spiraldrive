#include <assert.h>
#include <string.h>
#include "media/toc_session_removal.h"
#include "platform/firmware_memory.h"

static u8 low_memory[1u << 16];
static u8 far_2b_memory[1u << 16];
static unsigned reset_all_count;
static unsigned reset_summaries_count;
static fw_addr_t copy_ten_source[8];
static fw_addr_t copy_ten_destination[8];
static unsigned copy_ten_count;
static fw_addr_t copy_payload_source[8];
static fw_addr_t copy_payload_destination[8];
static unsigned copy_payload_count;
static unsigned tuple_convert_count;
static u8 tuple_minute;
static u8 tuple_second;
static u8 tuple_frame;
static s32 tuple_result;
static unsigned address_convert_count;
static fw_addr_t address_convert_source;
static s32 address_result;

u8 FirmwareRead8(fw_addr_t address)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        return far_2b_memory[(u16)address];
    return low_memory[(u16)address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        far_2b_memory[(u16)address] = value;
    else
        low_memory[(u16)address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

void ObserveRemoveTocResetAll(void)
{
    reset_all_count++;
}

void ObserveRemoveTocResetSummaries(void)
{
    reset_summaries_count++;
    memset(&low_memory[0xf2b6u], 0, 606u);
    memset(&low_memory[0xf79cu], 0, 450u);
    low_memory[0x12beu] = 0u;
    low_memory[0xfc1cu] = 0u;
}

void ObserveRemoveTocCopyTen(fw_addr_t source, fw_addr_t destination)
{
    unsigned index;

    copy_ten_source[copy_ten_count] = source;
    copy_ten_destination[copy_ten_count] = destination;
    copy_ten_count++;
    for(index = 0u; index < 10u; ++index)
        FirmwareWrite8(destination + index, FirmwareRead8(source + index));
}

void ObserveRemoveTocCopyPayload(fw_addr_t source, fw_addr_t destination)
{
    copy_payload_source[copy_payload_count] = source;
    copy_payload_destination[copy_payload_count] = destination;
    copy_payload_count++;
}

s32 ObserveRemoveTocConvertTuple(u8 minute, u8 second, u8 frame)
{
    tuple_convert_count++;
    tuple_minute = minute;
    tuple_second = second;
    tuple_frame = frame;
    return tuple_result;
}

s32 ObserveRemoveTocConvertAddress(fw_addr_t tuple_address)
{
    address_convert_count++;
    address_convert_source = tuple_address;
    return address_result;
}

u8 ObserveRemoveTocPackedBcdToBinary(u8 value)
{
    return (u8)((value >> 4) * 10u + (value & 0x0fu));
}

static fw_addr_t DescriptorAddress(u16 index)
{
    return 0x2b8000ul + (fw_addr_t)(index * 11u);
}

static void SetDescriptor(u16 index, u8 session, u8 control, u8 point, u8 pminute, u8 psecond, u8 pframe)
{
    fw_addr_t address = DescriptorAddress(index);

    FirmwareWrite8(address, session);
    FirmwareWrite8(address + 1u, control);
    FirmwareWrite8(address + 3u, point);
    FirmwareWrite8(address + 8u, pminute);
    FirmwareWrite8(address + 9u, psecond);
    FirmwareWrite8(address + 10u, pframe);
}

static u16 ReadWord(u16 address)
{
    return (u16)(((u16)low_memory[address] << 8) | low_memory[(u16)(address + 1u)]);
}

static u32 ReadDword(u16 address)
{
    return ((u32)low_memory[address] << 24) | ((u32)low_memory[(u16)(address + 1u)] << 16) | ((u32)low_memory[(u16)(address + 2u)] << 8) | low_memory[(u16)(address + 3u)];
}

static void ResetFixture(void)
{
    memset(low_memory, 0, sizeof(low_memory));
    memset(far_2b_memory, 0, sizeof(far_2b_memory));
    reset_all_count = 0u;
    reset_summaries_count = 0u;
    copy_ten_count = 0u;
    copy_payload_count = 0u;
    tuple_convert_count = 0u;
    tuple_minute = 0u;
    tuple_second = 0u;
    tuple_frame = 0u;
    tuple_result = 123456l;
    address_convert_count = 0u;
    address_convert_source = 0ul;
    address_result = 2250l;
}

static void TestSingleSessionUsesCompleteReset(void)
{
    ResetFixture();
    low_memory[0x12c9u] = 1u;
    RemoveLatestFullTocSessionAndRebuildState();
    assert(reset_all_count == 1u);
    assert(reset_summaries_count == 0u);
}

static void TestRebuildsPriorSessionsAndClearsNewest(void)
{
    fw_addr_t lead_out_summary = 0x00f2c2ul;

    ResetFixture();
    low_memory[0x12c9u] = 3u;
    SetDescriptor(1u, 1u, 0x01u, 0xa0u, 0u, 0x10u, 0u);
    SetDescriptor(2u, 1u, 0x41u, 0xa1u, 5u, 0u, 0u);
    SetDescriptor(3u, 2u, 0x01u, 0xa2u, 1u, 2u, 3u);
    SetDescriptor(4u, 2u, 0x01u, 1u, 4u, 5u, 6u);
    SetDescriptor(5u, 2u, 0x05u, 0xb2u, 0u, 0u, 0u);
    SetDescriptor(6u, 2u, 0x05u, 0x12u, 0u, 0u, 0u);
    SetDescriptor(7u, 3u, 0x01u, 1u, 7u, 8u, 9u);

    RemoveLatestFullTocSessionAndRebuildState();

    assert(reset_summaries_count == 1u && reset_all_count == 0u);
    assert(ReadWord(0x12dcu) == 7u && ReadWord(0x12cau) == 66u);
    assert(low_memory[0x12c9u] == 2u);
    assert(copy_ten_count == 4u);
    assert(copy_ten_source[0] == DescriptorAddress(1u) + 1u && copy_ten_destination[0] == 0x00f532ul);
    assert(copy_payload_count == 2u);
    assert(copy_payload_source[0] == 0x0014e4ul && copy_payload_destination[0] == 0x00f7a6ul);
    assert(copy_payload_source[1] == DescriptorAddress(6u) + 1u && copy_payload_destination[1] == 0x00f814ul);
    assert(tuple_convert_count == 1u);
    assert(tuple_minute == 1u && tuple_second == 2u && tuple_frame == 3u);
    assert(ReadDword(0x12c1u) == 123456ul);
    assert(low_memory[0xfc1cu] == 1u);
    assert(low_memory[0x12beu] == 0x0fu);
    assert(low_memory[0x12c0u] == 1u);
    assert(low_memory[0xf2bcu] == 1u);
    assert(low_memory[0xf2bdu] == 0x01u);
    assert(low_memory[0xf2bfu] == 4u && low_memory[0xf2c0u] == 5u && low_memory[0xf2c1u] == 6u);
    assert(FirmwareRead8(lead_out_summary) == 0xaau);
    assert(FirmwareRead8(lead_out_summary + 1u) == 0x01u);
    assert(FirmwareRead8(lead_out_summary + 3u) == 1u);
    assert(FirmwareRead8(lead_out_summary + 4u) == 2u);
    assert(FirmwareRead8(lead_out_summary + 5u) == 3u);
    assert(FirmwareRead8(DescriptorAddress(6u)) == 2u);
    assert(FirmwareRead8(DescriptorAddress(7u)) == 0u);
    assert(FirmwareRead8(DescriptorAddress(499u) + 10u) == 0u);
}

static void TestSingleControlClassSetsLongLeadInFlag(void)
{
    ResetFixture();
    low_memory[0x12c9u] = 2u;
    low_memory[0x12c0u] = 1u;
    low_memory[0xf546u] = 0x41u;
    low_memory[0xf54du] = 1u;
    low_memory[0xf54eu] = 2u;
    low_memory[0xf54fu] = 3u;
    SetDescriptor(1u, 1u, 0x01u, 1u, 0u, 30u, 0u);
    SetDescriptor(2u, 2u, 0x01u, 1u, 0u, 0u, 0u);

    RemoveLatestFullTocSessionAndRebuildState();

    assert(address_convert_count == 1u);
    assert(address_convert_source == 0x00f2bful);
    assert(low_memory[0x12beu] == 0x11u);
}

int main(void)
{
    TestSingleSessionUsesCompleteReset();
    TestRebuildsPriorSessionsAndClearsNewest();
    TestSingleControlClassSetsLongLeadInFlag();
    return 0;
}
