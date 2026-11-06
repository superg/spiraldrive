#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_threshold_configuration.h"
#include "platform/firmware_memory.h"

enum
{
    EVENT_P1,
    EVENT_REGISTER_01,
    EVENT_MAP,
    EVENT_PROFILE,
    EVENT_HIGH_THRESHOLDS,
    EVENT_SELECT_SECONDARY,
    EVENT_SUBMIT,
    EVENT_STROBE,
    EVENT_COMMON_PORTS,
    EVENT_WRITE_PORT,
    EVENT_LOAD_PROFILE,
    EVENT_FINAL_PORTS,
    EVENT_FINALIZE,
    EVENT_UPDATE_103A
};

typedef struct
{
    u8 kind;
    u32 first;
    u32 second;
} Event;

static u8 low_memory[0x20000];
static u8 profile_memory[0x40];
static Event events[64];
static size_t event_count;
static u8 update_103a_sets_bit6;

static void Record(u8 kind, u32 first, u32 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    ++event_count;
}

static void SetWord(fw_addr_t address, u16 value)
{
    assert(address >= 0xff84c0ul && address + 1u < 0xff8500ul);
    profile_memory[address - 0xff84c0ul] = (u8)(value >> 8);
    profile_memory[address + 1u - 0xff84c0ul] = (u8)value;
}

static void SetLowWord(fw_addr_t address, u16 value)
{
    low_memory[address] = (u8)(value >> 8);
    low_memory[address + 1u] = (u8)value;
}

static u8 ExpectedProfile(u8 operating_code)
{
    if(operating_code == 0u)
        return 0u;
    if(operating_code < 2u)
        return 1u;
    if(operating_code < 4u)
        return 2u;
    if(operating_code < 8u)
        return 3u;
    if(operating_code < 10u)
        return 4u;
    if(operating_code < 12u)
        return 5u;
    if(operating_code < 16u)
        return 6u;
    if(operating_code < 20u)
        return 7u;
    if(operating_code < 24u)
        return 8u;
    if(operating_code < 32u)
        return 9u;
    if(operating_code < 40u)
        return 10u;
    if(operating_code < 48u)
        return 11u;
    return 12u;
}

static void Reset(u8 operating_code)
{
    memset(low_memory, 0, sizeof(low_memory));
    memset(profile_memory, 0, sizeof(profile_memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    update_103a_sets_bit6 = 0u;
    low_memory[0x0e43] = operating_code;
    low_memory[0x0e4d] = operating_code;
    low_memory[0xfc15] = 0xa5u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    if(address >= 0xff84c0ul && address < 0xff8500ul)
        return profile_memory[address - 0xff84c0ul];
    assert(address < sizeof(low_memory));
    return low_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(low_memory));
    low_memory[address] = value;
}

void ObserveThresholdConfigurationSetP1Bits(void)
{
    Record(EVENT_P1, 0u, 0u);
}

void ObserveThresholdConfigurationWriteRegister01(u8 value)
{
    Record(EVENT_REGISTER_01, value, 0u);
}

u8 ObserveThresholdConfigurationMapProfile(u8 operating_code)
{
    Record(EVENT_MAP, operating_code, 0u);
    return ExpectedProfile(operating_code);
}

void ObserveThresholdConfigurationProfile(u8 profile_code)
{
    Record(EVENT_PROFILE, profile_code, 0u);
}

void ObserveThresholdConfigurationSetHighThresholds(u8 operating_code)
{
    Record(EVENT_HIGH_THRESHOLDS, operating_code, 0u);
}

void ObserveThresholdConfigurationSelectSecondary(s32 threshold_value)
{
    Record(EVENT_SELECT_SECONDARY, (u32)threshold_value, 0u);
}

void ObserveThresholdConfigurationSubmitWord(u16 value, u16 selector)
{
    Record(EVENT_SUBMIT, value, selector);
}

void ObserveThresholdConfigurationStrobe(fw_addr_t address, u8 value)
{
    Record(EVENT_STROBE, address, value);
}

void ObserveThresholdConfigurationCommonPorts(u8 enabled)
{
    Record(EVENT_COMMON_PORTS, enabled, 0u);
}

void ObserveThresholdConfigurationWritePort(fw_addr_t address, u8 value)
{
    Record(EVENT_WRITE_PORT, address, value);
}

void ObserveThresholdConfigurationLoadProfile(u8 operating_code)
{
    Record(EVENT_LOAD_PROFILE, operating_code, 0u);
}

void ObserveThresholdConfigurationFinalPorts(u8 operating_code)
{
    Record(EVENT_FINAL_PORTS, operating_code, 0u);
}

void ObserveThresholdConfigurationFinalize(s32 threshold_value)
{
    Record(EVENT_FINALIZE, (u32)threshold_value, 0u);
}

void ObserveThresholdConfigurationUpdateState103A(void)
{
    Record(EVENT_UPDATE_103A, 0u, 0u);
    if(update_103a_sets_bit6 != 0u)
        low_memory[0x103a] |= 0x40u;
}

static size_t FindEvent(u8 kind, size_t start)
{
    while(start < event_count && events[start].kind != kind)
        ++start;
    assert(start < event_count);
    return start;
}

static void TestCode01FixedProfileAndBaseTableWords(void)
{
    size_t index;

    Reset(1u);
    SetWord(0xff84ceul, 0x1234u);
    SetWord(0xff84e6ul, 0x5678u);
    ConfigureControllerForCurrentOperatingCodeAndThresholdValue(77u);

    assert(low_memory[0xfc15] == (u8)(0xa5u & 0xbfu));
    assert(low_memory[0x125d] == 0xffu);
    index = FindEvent(EVENT_REGISTER_01, 0u);
    assert(events[index].first == 1u);
    index = FindEvent(EVENT_PROFILE, index + 1u);
    assert(events[index].first == 1u);
    index = FindEvent(EVENT_COMMON_PORTS, index + 1u);
    assert(events[index].first == 0u);
    index = FindEvent(EVENT_WRITE_PORT, index + 1u);
    assert(events[index].first == 0x180a8u && events[index].second == 0u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x1234u && events[index].second == 0x036fu);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x5678u && events[index].second == 0x037fu);
    assert(FindEvent(EVENT_FINALIZE, index + 1u) == event_count - 1u);
}

static void TestCode0UsesPrecedingTableWordsAndHighPath(void)
{
    size_t index;

    Reset(0u);
    SetWord(0xff84ccul, 0x0a8cu);
    SetWord(0xff84e4ul, 0x222bu);
    SetLowWord(0x0cf1u, 0x3344u);
    ConfigureControllerForCurrentOperatingCodeAndThresholdValue(0x89abcdeful);

    index = FindEvent(EVENT_HIGH_THRESHOLDS, 0u);
    assert(events[index].first == 0u);
    index = FindEvent(EVENT_SELECT_SECONDARY, index + 1u);
    assert(events[index].first == 0x89abcdeful);
    index = FindEvent(EVENT_PROFILE, index + 1u);
    assert(events[index].first == 0x28u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x3344u && events[index].second == 0x035fu);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x0a8cu && events[index].second == 0x036fu);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x222bu && events[index].second == 0x037fu);
}

static void TestCode0ASpecialOverride(void)
{
    size_t index;

    Reset(10u);
    update_103a_sets_bit6 = 1u;
    low_memory[0x0e4d] = 8u;
    SetWord(0xff84d6ul, 0x1111u);
    SetWord(0xff84eeul, 0x2222u);
    SetLowWord(0x0ce9u, 0x9abcu);
    ConfigureControllerForCurrentOperatingCodeAndThresholdValue(91u);

    index = FindEvent(EVENT_UPDATE_103A, 0u);
    index = FindEvent(EVENT_REGISTER_01, index + 1u);
    assert(events[index].first == 4u);
    index = FindEvent(EVENT_PROFILE, index + 1u);
    assert(events[index].first == 10u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x9abcu && events[index].second == 0x035fu);
    index = FindEvent(EVENT_COMMON_PORTS, index + 1u);
    assert(events[index].first == 1u);
    index = FindEvent(EVENT_WRITE_PORT, index + 1u);
    assert(events[index].second == 0x12u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x77a3u && events[index].second == 0x036fu);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x445eu && events[index].second == 0x037fu);
    index = FindEvent(EVENT_LOAD_PROFILE, index + 1u);
    assert(events[index].first == 10u);
}

static void TestHighCodeUsesCf3AndClampedRegister(void)
{
    size_t index;

    Reset(0x34u);
    SetWord(0xff84e4ul, 0x3141u);
    SetWord(0xff84fcul, 0x5926u);
    SetLowWord(0x0cf3u, 0x7788u);
    ConfigureControllerForCurrentOperatingCodeAndThresholdValue(1234u);

    index = FindEvent(EVENT_REGISTER_01, 0u);
    assert(events[index].first == 0xffu);
    index = FindEvent(EVENT_HIGH_THRESHOLDS, index + 1u);
    assert(events[index].first == 0x34u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x7788u && events[index].second == 0x035fu);
    index = FindEvent(EVENT_COMMON_PORTS, index + 1u);
    assert(events[index].first == 1u);
    index = FindEvent(EVENT_WRITE_PORT, index + 1u);
    assert(events[index].second == 0x14u);
}

static void TestFixedProfileDispatch(void)
{
    static const u8 codes[] = { 4u, 8u, 12u, 16u };
    static const u8 ports[] = { 0x12u, 0x13u, 0x14u, 0x14u };
    size_t case_index;

    for(case_index = 0u; case_index < sizeof(codes); ++case_index)
    {
        size_t index;
        u8 profile = ExpectedProfile(codes[case_index]);
        Reset(codes[case_index]);
        SetWord(0xff84ceul + (u16)(profile - 1u) * 2u, 0x0102u);
        SetWord(0xff84e6ul + (u16)(profile - 1u) * 2u, 0x0304u);
        ConfigureControllerForCurrentOperatingCodeAndThresholdValue(5u);
        index = FindEvent(EVENT_PROFILE, 0u);
        assert(events[index].first == codes[case_index]);
        index = FindEvent(EVENT_WRITE_PORT, index + 1u);
        assert(events[index].second == ports[case_index]);
    }
}

int main(void)
{
    TestCode01FixedProfileAndBaseTableWords();
    TestCode0UsesPrecedingTableWordsAndHighPath();
    TestCode0ASpecialOverride();
    TestHighCodeUsesCf3AndClampedRegister();
    TestFixedProfileDispatch();
    puts("controller threshold configuration tests passed");
    return 0;
}
