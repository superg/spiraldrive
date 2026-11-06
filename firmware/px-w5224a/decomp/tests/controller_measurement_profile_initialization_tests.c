#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_profile_initialization.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void SetProfile(fw_addr_t address, u8 base00, u16 base01, u8 slope00, u16 published_word, s8 slope01)
{
    memory[address] = base00;
    WriteWord(address + 1u, base01);
    memory[address + 3u] = slope00;
    WriteWord(address + 4u, published_word);
    memory[address + 7u] = (u8)slope01;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memory[0x000e47ul] = 4u;
    memory[0x00f05ful] = 2u;
    memory[0x00fb80ul] = 2u;
}

static void CheckSelectedProfileAndOrdinarySeries(void)
{
    fw_addr_t profile = 0x00f83084ul + 22u;
    u8 index;

    Reset();
    SetProfile(profile, 5u, 1000u, 2u, 0x1234u, 0);
    memory[0x00f06aul] = 3u;
    memory[0x00ef77ul] = 4u;
    memory[0x00ef78ul] = 5u;
    WriteWord(0x00f074ul, 100u);
    memory[0x00ef72ul] = 0x40u;
    memory[0x00ef73ul] = 0u;

    InitializeControllerMeasurementSeries00And01FromProfile();
    assert(ReadDword(0x00f1bbul) == profile);
    assert(ReadWord(0x00f089ul) == 0x1234u);
    for(index = 0u; index < 15u; ++index)
    {
        assert(ReadWord(0x00ef89ul + 2u * index) == (u16)(12u + 2u * index));
        assert(ReadWord(0x00efa7ul + 2u * index) == 1105u);
    }
    assert(memory[0x00ef72ul] == 0x40u);
}

static void CheckProfileFamiliesAndOverride(void)
{
    static const u8 speeds[] = { 2u, 4u, 8u, 10u, 16u, 0x20u };
    static const fw_addr_t bases[] = { 0x00f831adul, 0x00f831adul, 0x00f833bdul, 0x00f832b5ul, 0x00f833bdul, 0x00f833bdul };
    u8 index;

    for(index = 0u; index < sizeof(speeds); ++index)
    {
        fw_addr_t profile = bases[index] + 33u;

        Reset();
        memory[0x000e47ul] = speeds[index];
        memory[0x00f05ful] = 3u;
        memory[0x00fb80ul] = 1u;
        SetProfile(profile, 1u, 2u, 0u, (u16)(0x2000u + index), 0);
        InitializeControllerMeasurementSeries00And01FromProfile();
        assert(ReadDword(0x00f1bbul) == profile);
        assert(ReadWord(0x00f089ul) == (u16)(0x2000u + index));
    }

    Reset();
    WriteDword(0x00f1bbul, 0x00200000ul);
    SetProfile(0x00200000ul, 1u, 2u, 0u, 0x3456u, 0);
    memory[0x000e47ul] = 3u;
    InitializeControllerMeasurementSeries00And01FromProfile();
    assert(ReadDword(0x00f1bbul) == 0x00200000ul);

    Reset();
    memory[0x00ef72ul] = 8u;
    SetProfile(0x00f8342bul, 1u, 2u, 0u, 0x4567u, 0);
    InitializeControllerMeasurementSeries00And01FromProfile();
    assert(ReadDword(0x00f1bbul) == 0x00f8342bul);
    assert(ReadWord(0x00f089ul) == 0x4567u);
}

static void CheckMode4UnclampedPath(void)
{
    fw_addr_t profile = 0x00210000ul;
    u8 index;

    Reset();
    memory[0x000e47ul] = 3u;
    WriteDword(0x00f1bbul, profile);
    SetProfile(profile, 250u, 0xfff0u, 20u, 0xabcd, 0);
    memory[0x00f06aul] = 200u;
    memory[0x00ef77ul] = 250u;
    memory[0x00ef78ul] = 200u;
    WriteWord(0x00f074ul, 0x20u);
    memory[0x00ef72ul] = 0x40u;
    memory[0x00ef73ul] = 4u;

    InitializeControllerMeasurementSeries00And01FromProfile();
    for(index = 0u; index < 15u; ++index)
    {
        assert(ReadWord(0x00ef89ul + 2u * index) == (u16)(500u + 20u * index));
        assert(ReadWord(0x00efa7ul + 2u * index) == 0x10u);
    }
    assert(memory[0x00ef72ul] == 0u);
}

static void CheckMode1SignedAdjustmentAndClamps(void)
{
    fw_addr_t profile = 0x00220000ul;
    u8 index;

    Reset();
    memory[0x000e47ul] = 3u;
    WriteDword(0x00f1bbul, profile);
    SetProfile(profile, 120u, 100u, 20u, 0u, -10);
    memory[0x00f06aul] = 10u;
    memory[0x00ef77ul] = 10u;
    memory[0x00ef73ul] = 1u;
    memory[0x00ef72ul] = 0x40u;

    InitializeControllerMeasurementSeries00And01FromProfile();
    for(index = 0u; index < 15u; ++index)
    {
        assert(ReadWord(0x00ef89ul + 2u * index) == 126u);
        assert(ReadWord(0x00efa7ul + 2u * index) == (u16)(100u - index));
    }
    assert(memory[0x00ef72ul] == 0u);

    SetProfile(profile, 0u, 23990u, 0u, 0u, 100);
    InitializeControllerMeasurementSeries00And01FromProfile();
    assert(ReadWord(0x00efa7ul) == 23990u);
    assert(ReadWord(0x00efa7ul + 28u) == 24000u);

    SetProfile(profile, 0u, 0xff00u, 0u, 0u, 0);
    InitializeControllerMeasurementSeries00And01FromProfile();
    assert(ReadWord(0x00efa7ul) == 0u);
}

static void CheckModeFlagMapping(void)
{
    fw_addr_t profile = 0x00230000ul;
    u8 mode;

    for(mode = 0u; mode < 8u; ++mode)
    {
        Reset();
        memory[0x000e47ul] = 3u;
        WriteDword(0x00f1bbul, profile);
        SetProfile(profile, 0u, 0u, 0u, 0u, 0);
        memory[0x00ef73ul] = mode;
        memory[0x00ef72ul] = mode == 1u || mode == 4u ? 0xffu : 0u;
        InitializeControllerMeasurementSeries00And01FromProfile();
        if(mode == 2u || mode == 3u || mode == 6u)
            assert((memory[0x00ef72ul] & 0x40u) != 0u);
        else if(mode == 1u || mode == 4u)
            assert((memory[0x00ef72ul] & 0x40u) == 0u);
        else
            assert(memory[0x00ef72ul] == 0u);
    }
}

int main(void)
{
    CheckSelectedProfileAndOrdinarySeries();
    CheckProfileFamiliesAndOverride();
    CheckMode4UnclampedPath();
    CheckMode1SignedAdjustmentAndClamps();
    CheckModeFlagMapping();
    return 0;
}
