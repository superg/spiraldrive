#include <assert.h>
#include <string.h>
#include "media/toc_session_append.h"
#include "platform/firmware_memory.h"

static u8 low_memory[1u << 16];
static u8 far_2b_memory[1u << 16];
static unsigned copy_ten_count;
static unsigned copy_decode_count;
static unsigned copy_binary_count;
static unsigned bcd_count;
static unsigned convert_address_count;
static unsigned convert_tuple_count;

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

u8 ObserveAppendTocPackedBcdToBinary(u8 value)
{
    bcd_count++;
    return (u8)((value >> 4) * 10u + (value & 0x0fu));
}

static s32 ConvertTuple(u8 minute, u8 second, u8 frame)
{
    return (s32)((u32)minute * 60ul * 75ul + (u32)second * 75ul + frame) - 150l;
}

s32 ObserveAppendTocConvertAddress(fw_addr_t tuple_address)
{
    convert_address_count++;
    return ConvertTuple(FirmwareRead8(tuple_address), FirmwareRead8(tuple_address + 1u), FirmwareRead8(tuple_address + 2u));
}

s32 ObserveAppendTocConvertTuple(u8 minute, u8 second, u8 frame)
{
    convert_tuple_count++;
    return ConvertTuple(minute, second, frame);
}

void ObserveAppendTocCopyTen(fw_addr_t source, fw_addr_t destination)
{
    unsigned index;

    copy_ten_count++;
    for(index = 0u; index < 10u; ++index)
        FirmwareWrite8(destination + index, FirmwareRead8(source + index));
}

void ObserveAppendTocCopyDecode(fw_addr_t source, fw_addr_t destination)
{
    unsigned index;

    copy_decode_count++;
    FirmwareWrite8(destination, FirmwareRead8(source));
    FirmwareWrite8(destination + 1u, 0u);
    FirmwareWrite8(destination + 2u, FirmwareRead8(source + 2u));
    for(index = 3u; index < 10u; ++index)
        FirmwareWrite8(destination + index, ObserveAppendTocPackedBcdToBinary(FirmwareRead8(source + index)));
}

void ObserveAppendTocCopyBinary(fw_addr_t source, fw_addr_t destination)
{
    unsigned index;

    copy_binary_count++;
    FirmwareWrite8(destination, FirmwareRead8(source));
    FirmwareWrite8(destination + 1u, 0u);
    for(index = 2u; index < 10u; ++index)
        FirmwareWrite8(destination + index, FirmwareRead8(source + index));
}

static void WriteWord(u16 address, u16 value)
{
    low_memory[address] = (u8)(value >> 8);
    low_memory[(u16)(address + 1u)] = (u8)value;
}

static u16 ReadWord(u16 address)
{
    return (u16)(((u16)low_memory[address] << 8) | low_memory[(u16)(address + 1u)]);
}

static u32 ReadDword(u16 address)
{
    return ((u32)low_memory[address] << 24) | ((u32)low_memory[(u16)(address + 1u)] << 16) | ((u32)low_memory[(u16)(address + 2u)] << 8) | low_memory[(u16)(address + 3u)];
}

static fw_addr_t DescriptorAddress(u16 cursor)
{
    return 0x2b8000ul + (fw_addr_t)cursor * 11u;
}

static void SetRecord(u16 index, u8 control_adr, u8 point, u8 pminute, u8 psecond, u8 pframe)
{
    u16 address = (u16)(0x8012u + index * 10u);

    low_memory[address] = control_adr;
    low_memory[(u16)(address + 2u)] = point;
    low_memory[(u16)(address + 7u)] = pminute;
    low_memory[(u16)(address + 8u)] = psecond;
    low_memory[(u16)(address + 9u)] = pframe;
}

static void ResetFixture(void)
{
    memset(low_memory, 0, sizeof(low_memory));
    memset(far_2b_memory, 0, sizeof(far_2b_memory));
    copy_ten_count = 0u;
    copy_decode_count = 0u;
    copy_binary_count = 0u;
    bcd_count = 0u;
    convert_address_count = 0u;
    convert_tuple_count = 0u;
}

static void TestAdr1ScanAndMandatoryDescriptors(void)
{
    fw_addr_t descriptor;

    ResetFixture();
    low_memory[0x1132u] = 0xffu;
    WriteWord(0x800cu, 5u);
    SetRecord(0u, 0x01u, 0xa0u, 0x01u, 0x10u, 0x00u);
    SetRecord(1u, 0x01u, 0xa1u, 0x02u, 0x00u, 0x00u);
    SetRecord(2u, 0x01u, 0xa2u, 0x00u, 0x10u, 0x00u);
    SetRecord(3u, 0x01u, 0x01u, 0x00u, 0x02u, 0x00u);
    SetRecord(4u, 0x41u, 0x02u, 0x00u, 0x03u, 0x00u);

    AppendFullTocSessionFromRawReadQRecordTable();

    assert(low_memory[0x12c9u] == 1u);
    assert(low_memory[0x12beu] == 0x0bu);
    assert(low_memory[0x12bfu] == 1u && low_memory[0x12c0u] == 2u);
    assert(ReadDword(0x12c1u) == 600ul);
    assert(low_memory[0xf2bcu] == 1u && low_memory[0xf2bdu] == 0x01u);
    assert(low_memory[0xf2c2u] == 2u && low_memory[0xf2c3u] == 0x41u);
    assert(low_memory[0xf2c8u] == 0xaau);
    assert(low_memory[0xfc19u] == 1u && low_memory[0xfc1au] == 2u);
    assert(ReadWord(0x12dcu) == 6u && ReadWord(0x12cau) == 55u);
    descriptor = DescriptorAddress(1u);
    assert(FirmwareRead8(descriptor) == 1u && FirmwareRead8(descriptor + 3u) == 0xa0u);
    descriptor = DescriptorAddress(2u);
    assert(FirmwareRead8(descriptor + 3u) == 0xa1u);
    descriptor = DescriptorAddress(3u);
    assert(FirmwareRead8(descriptor + 3u) == 0xa2u);
    descriptor = DescriptorAddress(4u);
    assert(FirmwareRead8(descriptor + 3u) == 1u && FirmwareRead8(descriptor + 9u) == 2u);
    descriptor = DescriptorAddress(5u);
    assert(FirmwareRead8(descriptor + 3u) == 2u && FirmwareRead8(descriptor + 1u) == 0x41u);
    assert(low_memory[0x1132u] == 0xbfu);
    assert(copy_ten_count == 3u && copy_decode_count == 3u);
    assert(copy_binary_count == 0u && convert_tuple_count == 1u);
}

static void TestAdr5ScanAndOptionalDescriptors(void)
{
    static const u8 points[8] = { 0xb0u, 0xb1u, 0xb2u, 0xb3u, 0xb4u, 0x01u, 0xc0u, 0xc1u };
    unsigned index;

    ResetFixture();
    WriteWord(0x800cu, 8u);
    low_memory[0xf532u] = 0x01u;
    low_memory[0xf534u] = 0xa0u;
    low_memory[0xf539u] = 0x01u;
    low_memory[0xf53cu] = 0x01u;
    low_memory[0xf53eu] = 0xa1u;
    low_memory[0xf543u] = 0x00u;
    low_memory[0xf546u] = 0x01u;
    low_memory[0xf548u] = 0xa2u;
    low_memory[0x12c0u] = 0u;
    for(index = 0u; index < 8u; ++index)
    {
        u16 source;

        SetRecord((u16)index, 0x05u, points[index], 0x00u, (u8)(index + 1u), 0x00u);
        source = (u16)(0x8012u + index * 10u);
        low_memory[(u16)(source + 3u)] = 0x00u;
        low_memory[(u16)(source + 4u)] = 0x03u;
        low_memory[(u16)(source + 5u)] = 0x00u;
    }

    AppendFullTocSessionFromRawReadQRecordTable();

    assert(low_memory[0x14e2u] == 0u && low_memory[0x14e3u] == 0xffu);
    assert(low_memory[0xfc1cu] == 1u && low_memory[0x1502u] == 0u);
    assert((low_memory[0x12beu] & 4u) != 0u);
    assert(ReadWord(0x12dcu) == 12u && ReadWord(0x12cau) == 121u);
    assert(FirmwareRead8(DescriptorAddress(4u) + 3u) == 0xb0u);
    assert(FirmwareRead8(DescriptorAddress(5u) + 3u) == 0xb1u);
    assert(FirmwareRead8(DescriptorAddress(6u) + 3u) == 0xb2u);
    assert(FirmwareRead8(DescriptorAddress(7u) + 3u) == 0xb3u);
    assert(FirmwareRead8(DescriptorAddress(8u) + 3u) == 0xb4u);
    assert(FirmwareRead8(DescriptorAddress(9u) + 3u) == 1u);
    assert(FirmwareRead8(DescriptorAddress(10u) + 3u) == 0xc0u);
    assert(FirmwareRead8(DescriptorAddress(11u) + 3u) == 0xc1u);
    for(index = 1u; index <= 11u; ++index)
        assert(FirmwareRead8(DescriptorAddress((u16)index)) == 1u);
    assert(copy_ten_count == 7u);
    assert(copy_binary_count == 1u);
    assert(convert_address_count >= 1u);
}

static void TestLongLeadInThresholdAndExistingCursor(void)
{
    ResetFixture();
    low_memory[0x12c9u] = 1u;
    low_memory[0x12beu] = 1u;
    low_memory[0xf2bfu] = 0u;
    low_memory[0xf2c0u] = 32u;
    low_memory[0xf2c1u] = 0u;
    low_memory[0xf539u] = 1u;
    low_memory[0xf543u] = 0u;
    WriteWord(0x12dcu, 10u);
    WriteWord(0x12cau, 100u);

    AppendFullTocSessionFromRawReadQRecordTable();

    assert(low_memory[0x12c9u] == 2u);
    assert((low_memory[0x12beu] & 0x10u) != 0u);
    assert(ReadWord(0x12dcu) == 13u && ReadWord(0x12cau) == 133u);
    assert(FirmwareRead8(DescriptorAddress(10u)) == 2u);
}

int main(void)
{
    TestAdr1ScanAndMandatoryDescriptors();
    TestAdr5ScanAndOptionalDescriptors();
    TestLongLeadInThresholdAndExistingCursor();
    return 0;
}
