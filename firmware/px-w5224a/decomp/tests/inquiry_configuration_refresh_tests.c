#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/mcs251.h"
#include "scsi/inquiry_configuration_refresh.h"

static u8 memory[0x1000000ul];
static u8 configuration[32];
static u8 validity;
static fw_addr_t read_offsets[2];
static fw_addr_t read_destinations[2];
static u16 read_counts[2];
static unsigned read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveRefreshInquiryResponseEntry(void)
{
    return 1u;
}

void ObserveInquiryConfigurationRead(fw_addr_t offset, fw_addr_t destination, u16 count)
{
    unsigned call_index = read_count++;
    u16 index;

    assert(call_index < 2u);
    read_offsets[call_index] = offset;
    read_destinations[call_index] = destination;
    read_counts[call_index] = count;
    for(index = 0u; index < count; ++index)
        FirmwareWrite8(destination + index, offset == 0u ? configuration[index] : validity);
}

void InitializeInquiryResponseBuffer(void)
{
    memcpy(&memory[0xdccdu], &memory[0xf80000u], 8u);
    memcpy(&memory[0xdcd5u], &memory[0xf80008u], 16u);
}

void CopyBytesWith8BitCount(fw_addr_t destination, fw_addr_t source, u8 byte_count)
{
    while(byte_count-- != 0u)
        FirmwareWrite8(destination++, FirmwareRead8(source++));
}

u32 ParseUppercaseAsciiHexU32(u8 character_count, fw_addr_t source_address)
{
    u32 value = 0u;

    while(character_count-- != 0u)
    {
        u8 character = FirmwareRead8(source_address++);
        u8 digit = character >= (u8)'0' && character <= (u8)'9' ? (u8)(character - (u8)'0') : character >= (u8)'A' && character <= (u8)'F' ? (u8)(character - (u8)'A' + 10u) : 0u;
        value = (value << 4) + digit;
    }
    return value;
}

u32 PowerOfTenU32(u8 exponent)
{
    u32 value = 1u;

    while(exponent-- != 0u)
        value *= 10u;
    return value;
}

u32 DivideU32Quotient(u32 dividend, u32 divisor)
{
    return dividend / divisor;
}

u32 MultiplyU32Low(u32 left, u32 right)
{
    return left * right;
}

u8 CompareFullInquiryProductIdentification(void)
{
    static const u8 expected[16] = { 0x43u, 0x44u, 0x2du, 0x52u, 0x20u, 0x20u, 0x20u, 0x50u, 0x58u, 0x2du, 0x57u, 0x34u, 0x38u, 0x32u, 0x34u, 0x41u };

    return memcmp(&memory[0xdcd5u], expected, sizeof(expected)) == 0 ? 6u : 0u;
}

void AppendConditionalInquiryExtensionBytes(void) {}

static void ResetTestState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(configuration, 0, sizeof(configuration));
    memset(read_offsets, 0, sizeof(read_offsets));
    memset(read_destinations, 0, sizeof(read_destinations));
    memset(read_counts, 0, sizeof(read_counts));
    validity = 0u;
    read_count = 0u;
}

static void TestAcceptedFieldsAndSerialConversion(void)
{
    static const u8 expected_product[16] = { 0x43u, 0x44u, 0x2du, 0x52u, 0x20u, 0x20u, 0x20u, 0x50u, 0x58u, 0x2du, 0x57u, 0x34u, 0x38u, 0x32u, 0x34u, 0x41u };

    ResetTestState();
    memcpy(&configuration[2], "ZLEXTOR ", 8u);
    memcpy(&configuration[10], expected_product, sizeof(expected_product));
    memcpy(&configuration[26], "0002A", 5u);
    validity = 7u;
    memory[0x0c84u] = 0x81u;

    RefreshInquiryResponseFromDriveConfiguration();

    assert(
        read_count == 2u && read_offsets[0] == 0u && read_destinations[0] == 0x7000ul && read_counts[0] == 32u && read_offsets[1] == 0x46u && read_destinations[1] == 0x7020ul && read_counts[1] == 1u);
    assert(memcmp(&memory[0xdccdu], "ZLEXTOR ", 8u) == 0);
    assert(memcmp(&memory[0xdcd5u], expected_product, 16u) == 0);
    assert(memcmp(&memory[0xdd25u], "000042", 6u) == 0);
    assert(memory[0x0c84u] == 0x85u && memory[0x0c7fu] == 6u && memory[0x88bau] == 1u);
}

static void TestLeadingSpaceProductAndOverflowDigit(void)
{
    ResetTestState();
    memcpy(&memory[0xf80000u], "DEFAULT!", 8u);
    configuration[2] = '@';
    memcpy(&configuration[10], " PRODUCT-IDENTITY", 16u);
    memcpy(&configuration[26], "FFFFF", 5u);

    RefreshInquiryResponseFromDriveConfiguration();

    assert(memcmp(&memory[0xdccdu], "DEFAULT!", 8u) == 0);
    assert(memcmp(&memory[0xdcd5u], " PRODUCT-IDENTITY", 16u) == 0);
    assert(memcmp(&memory[0xdd25u], ":48575", 6u) == 0);
    assert(memory[0x88bau] == 0u);
}

static void TestRejectedFieldsAndErasedSerial(void)
{
    ResetTestState();
    memcpy(&memory[0xf80000u], "DEFAULT!", 8u);
    memcpy(&memory[0xf80008u], "DEFAULT PRODUCT!", 16u);
    configuration[2] = '[';
    configuration[10] = ' ';
    configuration[11] = '@';
    memcpy(&configuration[26], "12", 2u);
    configuration[28] = 0xffu;
    memcpy(&configuration[29], "34", 2u);

    RefreshInquiryResponseFromDriveConfiguration();

    assert(memcmp(&memory[0xdccdu], "DEFAULT!", 8u) == 0);
    assert(memcmp(&memory[0xdcd5u], "DEFAULT PRODUCT!", 16u) == 0);
    assert(memcmp(&memory[0xdd25u], "      ", 6u) == 0);
}

int main(void)
{
    TestAcceptedFieldsAndSerialConversion();
    TestLeadingSpaceProductAndOverflowDigit();
    TestRejectedFieldsAndErasedSerial();
    puts("inquiry configuration-refresh tests passed");
    return 0;
}
