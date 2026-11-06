#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/disc_control_state_initialization.h"

typedef struct HelperCall
{
    u8 event;
    s16 argument;
} HelperCall;

static u8 firmware_memory[0x20000];
static u8 configuration_byte_2e;
static u8 configuration_bytes_30[6];
static u8 indexed_results[2];
static unsigned indexed_result_index;
static s8 decoded_offset_result;
static HelperCall helper_calls[8];
static unsigned helper_call_count;
static unsigned memory_read_count;
static unsigned local_read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(firmware_memory));
    return firmware_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    firmware_memory[address] = value;
}

void ObserveDiscControlConfigurationReadToMemory(fw_addr_t offset, fw_addr_t destination, u16 count)
{
    assert(offset == 0x2eul);
    assert(destination == 0x1041ul);
    assert(count == 1u);
    ++memory_read_count;
    FirmwareWrite8(destination, configuration_byte_2e);
}

void ObserveDiscControlConfigurationReadToLocal(fw_addr_t offset, u8 *destination, u16 count)
{
    assert(offset == 0x30ul);
    assert(count == 6u);
    ++local_read_count;
    memcpy(destination, configuration_bytes_30, 6u);
}

s16 ObserveDiscControlInitializationHelper(u8 event, s16 argument)
{
    assert(helper_call_count < sizeof(helper_calls) / sizeof(helper_calls[0]));
    helper_calls[helper_call_count].event = event;
    helper_calls[helper_call_count].argument = argument;
    ++helper_call_count;

    if(event == DISC_CONTROL_DIVIDE_ADJUSTED_BYTE)
        return (s8)argument / 2;
    if(event == DISC_CONTROL_TEST_INDEXED_STATE)
    {
        assert(argument == 0x1f);
        assert(indexed_result_index < 2u);
        return indexed_results[indexed_result_index++];
    }
    if(event == DISC_CONTROL_DECODE_SIGNED_OFFSET)
        return decoded_offset_result;
    return 0;
}

static u16 ReadU16(u16 address)
{
    return (u16)(((u16)firmware_memory[address] << 8) | firmware_memory[address + 1u]);
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0xa5, sizeof(firmware_memory));
    memset(configuration_bytes_30, 0, sizeof(configuration_bytes_30));
    memset(indexed_results, 0, sizeof(indexed_results));
    memset(helper_calls, 0, sizeof(helper_calls));
    configuration_byte_2e = 0xffu;
    indexed_result_index = 0u;
    decoded_offset_result = 0;
    helper_call_count = 0u;
    memory_read_count = 0u;
    local_read_count = 0u;
    firmware_memory[0x105cu] = 0xffu;
    firmware_memory[0xef72u] = 0xffu;
    firmware_memory[0x1039u] = 0xffu;
    firmware_memory[0x103au] = 0xffu;
    firmware_memory[0x103bu] = 0xffu;
    firmware_memory[0x1302u] = 0xffu;
    firmware_memory[0xf1efu] = 0x5au;
    firmware_memory[0xf2b6u] = 0xc3u;
}

static void AssertCommonInitialization(void)
{
    u16 address;

    assert(firmware_memory[0x1082u] == 0u);
    assert(firmware_memory[0x1080u] == 0u);
    assert(firmware_memory[0x105cu] == 0xd0u);
    assert(firmware_memory[0xef72u] == 0xfeu);
    assert(firmware_memory[0x1039u] == 0x80u);
    assert(firmware_memory[0x103bu] == 0xcfu);
    assert(firmware_memory[0xf1efu] == 0x5au);
    for(address = 0xf1f0u; address < 0xf2b6u; ++address)
        assert(firmware_memory[address] == 0u);
    assert(firmware_memory[0xf2b6u] == 0xc3u);
    assert(memory_read_count == 1u);
    assert(local_read_count == 1u);
    assert(helper_calls[0].event == DISC_CONTROL_RESET_MEASUREMENT_CAPTURE);
}

static void TestNonTypeTwoDefaultsAndFreshIndexedCheck(void)
{
    ResetFixture();
    configuration_bytes_30[0] = 1u;
    configuration_bytes_30[3] = 0x81u;
    indexed_results[0] = 1u;
    decoded_offset_result = -7;

    InitializeDiscControlStateFromConfiguration();

    AssertCommonInitialization();
    assert(firmware_memory[0x103au] == 0xddu);
    assert(firmware_memory[0x1042u] == 0u);
    assert(firmware_memory[0x1043u] == 0u);
    assert(firmware_memory[0x1044u] == 0u);
    assert(firmware_memory[0x1302u] == 0xfbu);
    assert(firmware_memory[0xf06au] == (u8)-7);
    assert(ReadU16(0xf072u) == 0x0b80u);
    assert(ReadU16(0xf074u) == 0u);
    assert(indexed_result_index == 1u);
    assert(helper_calls[1].event == DISC_CONTROL_TEST_INDEXED_STATE);
    assert(helper_calls[2].event == DISC_CONTROL_DECODE_SIGNED_OFFSET);
}

static void TestTypeTwoPositiveControlAndTimingUnderflow(void)
{
    ResetFixture();
    configuration_byte_2e = 0x12u;
    configuration_bytes_30[0] = 2u;
    configuration_bytes_30[1] = 10u;
    configuration_bytes_30[3] = 0x22u;
    configuration_bytes_30[4] = 0x0bu;
    configuration_bytes_30[5] = 0x00u;
    decoded_offset_result = 12;

    InitializeDiscControlStateFromConfiguration();

    AssertCommonInitialization();
    assert(firmware_memory[0x103au] == 0xdfu);
    assert(firmware_memory[0x1042u] == 10u);
    assert(firmware_memory[0x1043u] == 10u);
    assert(firmware_memory[0x1044u] == 10u);
    assert(firmware_memory[0xfac5u] == 10u);
    assert(firmware_memory[0x1302u] == 0xffu);
    assert(firmware_memory[0xf06au] == 12u);
    assert(ReadU16(0xf072u) == 0x0b00u);
    assert(ReadU16(0xf074u) == 0xff80u);
    assert(helper_calls[1].event == DISC_CONTROL_DIVIDE_ADJUSTED_BYTE);
    assert(helper_calls[1].argument == 21);
    assert(helper_calls[2].event == DISC_CONTROL_TEST_INDEXED_STATE);
    assert(helper_calls[3].event == DISC_CONTROL_TEST_INDEXED_STATE);
    assert(helper_calls[4].event == DISC_CONTROL_DECODE_SIGNED_OFFSET);
    assert(indexed_result_index == 2u);
}

static void TestTypeTwoNegativeControlAndIndexedSuppression(void)
{
    ResetFixture();
    configuration_byte_2e = 0u;
    configuration_bytes_30[0] = 2u;
    configuration_bytes_30[1] = (u8)-10;
    configuration_bytes_30[3] = 0x55u;
    configuration_bytes_30[4] = 0xffu;
    configuration_bytes_30[5] = 0xffu;
    indexed_results[0] = 1u;
    indexed_results[1] = 2u;
    decoded_offset_result = -1;

    InitializeDiscControlStateFromConfiguration();

    AssertCommonInitialization();
    assert(helper_calls[1].event == DISC_CONTROL_DIVIDE_ADJUSTED_BYTE);
    assert(helper_calls[1].argument == -21);
    assert(firmware_memory[0x103au] == 0xddu);
    assert(firmware_memory[0x1042u] == 0u);
    assert(firmware_memory[0x1043u] == 0u);
    assert(firmware_memory[0x1044u] == 0u);
    assert(firmware_memory[0xfac5u] == (u8)-10);
    assert(firmware_memory[0x1302u] == 0xffu);
    assert(ReadU16(0xf072u) == 0x0b80u);
    assert(ReadU16(0xf074u) == 0u);
    assert(indexed_result_index == 2u);
}

static void TestNegativeControlQuotientWithoutSuppression(void)
{
    ResetFixture();
    configuration_byte_2e = 1u;
    configuration_bytes_30[0] = 2u;
    configuration_bytes_30[1] = (u8)-10;
    configuration_bytes_30[4] = 0x12u;
    configuration_bytes_30[5] = 0x34u;

    InitializeDiscControlStateFromConfiguration();

    assert(firmware_memory[0x1042u] == (u8)-10);
    assert(firmware_memory[0x1043u] == (u8)-10);
    assert(firmware_memory[0x1044u] == (u8)-10);
    assert(ReadU16(0xf072u) == 0x1234u);
    assert(ReadU16(0xf074u) == 0x06b4u);
}

int main(void)
{
    TestNonTypeTwoDefaultsAndFreshIndexedCheck();
    TestTypeTwoPositiveControlAndTimingUnderflow();
    TestTypeTwoNegativeControlAndIndexedSuppression();
    TestNegativeControlQuotientWithoutSuppression();
    puts("disc-control state initialization tests passed");
    return 0;
}
