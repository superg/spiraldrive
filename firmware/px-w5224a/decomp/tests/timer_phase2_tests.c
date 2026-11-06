#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/timer_phase2.h"

static u8 memory[0x200000];
static u8 bit003d;
static u8 bit003e;
static u8 p3_bit0;
static u8 p3_bit1;
static unsigned transition_calls;

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

u8 ObserveTimerPhase2Bit003E(void)
{
    return bit003e;
}
void FirmwareWriteBit003D(u8 value)
{
    bit003d = (u8)(value != 0u);
}
void FirmwareWriteBit003E(u8 value)
{
    bit003e = (u8)(value != 0u);
}
u8 FirmwareReadBit0580(void)
{
    return p3_bit0;
}
u8 FirmwareReadBit0581(void)
{
    return p3_bit1;
}
void FirmwareWriteBit0580(u8 value)
{
    p3_bit0 = (u8)(value != 0u);
}
void FirmwareWriteBit0581(u8 value)
{
    p3_bit1 = (u8)(value != 0u);
}

void ServiceP1Bit6DriveStateTransition(void)
{
    ++transition_calls;
}

static void WriteU16(u16 address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(u16 address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void WriteU32(u16 address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(u16 address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    bit003d = 0u;
    bit003e = 0u;
    p3_bit0 = 0u;
    p3_bit1 = 0u;
    transition_calls = 0u;
}

static void TestFastCountdowns(void)
{
    static const u16 countdowns[] = { 0x121e, 0x1216, 0x1217, 0x1215, 0x1213, 0x1214, 0x121a, 0x0fd1, 0x0fbd, 0x1219, 0x121b, 0x121c };
    unsigned index;

    ResetState();
    for(index = 0u; index < sizeof(countdowns) / sizeof(countdowns[0]); ++index)
        memory[countdowns[index]] = 1u;
    memory[0x1218] = 1u;
    memory[0x1212] = 1u;
    WriteU16(0x1200u, 0u);
    WriteU16(0x1034u, 0u);

    TimerDispatchPhase2();

    assert(memory[0x121d] == 1u);
    for(index = 0u; index < sizeof(countdowns) / sizeof(countdowns[0]); ++index)
        assert(memory[countdowns[index]] == 0u);
    assert(ReadU16(0x1200u) == 0xffffu);
    assert(ReadU16(0x1034u) == 0xffffu);
    assert(memory[0x1218] == 0u);
    assert(memory[0xfbd5] == 0xffu);
    assert(memory[0x1212] == 0u);
    assert(memory[0x120f] == 0u);
}

static void TestDividedAndSlowerCountdowns(void)
{
    ResetState();
    memory[0x1212] = 0u;
    memory[0x120d] = 0u;
    memory[0x120e] = 0u;
    memory[0x1204] = 5u;
    memory[0x1203] = 5u;
    memory[0x1202] = 5u;
    memory[0x0fb8] = 5u;
    memory[0x0c89] = 9u;
    memory[0x0c8a] = 1u;
    memory[0x122e] = 2u;
    memory[0x122b] = 5u;
    memory[0x122c] = 5u;
    memory[0x122d] = 0u;
    memory[0x0c8b] = 0x10u;
    memory[0x0faf] = 1u;
    memory[0x1226] = 1u;
    memory[0x1210] = 4u;
    memory[0x1211] = 4u;
    WriteU16(0x1004u, 1u);
    WriteU16(0x11dau, 0u);
    WriteU32(0x11f4u, 0xfffffffful);
    WriteU32(0x11f0u, 7ul);

    TimerDispatchPhase2();

    assert(memory[0x1212] == 9u);
    assert(memory[0x120f] == 0xffu && memory[0x130b] == 0xffu);
    assert(memory[0x122e] == 1u && ReadU16(0x1004u) == 0u);
    assert(memory[0x120d] == 9u && memory[0x120e] == 4u);
    assert(memory[0x1204] == 4u && memory[0x1203] == 4u && memory[0x1202] == 4u && memory[0x0fb8] == 4u);
    assert(bit003d == 1u && bit003e == 0u);
    assert(memory[0x122b] == 4u && memory[0x122c] == 4u);
    assert(memory[0x1588] == 4u && memory[0x0c8b] == 0u);
    assert(ReadU32(0x11f4u) == 0u && ReadU32(0x11f0u) == 8u);
    assert(ReadU16(0x11dau) == 0xffffu);
    assert(memory[0x1226] == 0u && p3_bit0 == 1u && p3_bit1 == 0u);
}

static void TestSerialHandshakes(void)
{
    ResetState();
    memory[0x1212] = 0u;
    memory[0x120d] = 1u;
    memory[0x120e] = 1u;
    memory[0x1210] = 0u;
    memory[0x1211] = 20u;
    memory[0xfaed] = 0x2cu;
    memory[0xfaee] = 5u;
    p3_bit1 = 1u;

    TimerDispatchPhase2();

    assert(memory[0x1210] == 2u && memory[0xfaee] == 6u);
    assert((memory[0xfaed] & 0x10u) != 0u);
    assert((memory[0xfaef] & 0x10u) != 0u && memory[0x1211] == 29u);
    assert(transition_calls == 1u && p3_bit0 == 1u && p3_bit1 == 0u);

    ResetState();
    memory[0x1212] = 0u;
    memory[0x120d] = 1u;
    memory[0x120e] = 1u;
    memory[0x1210] = 5u;
    memory[0x1211] = 0u;
    memory[0xfaed] = 0x20u;
    memory[0xfaef] = 0x21u;
    memory[0xfaf0] = 5u;
    p3_bit0 = 1u;

    TimerDispatchPhase2();

    assert(memory[0x1211] == 2u && memory[0xfaf0] == 6u);
    assert((memory[0xfaef] & 0x40u) != 0u);
    assert(transition_calls == 1u && p3_bit0 == 0u && p3_bit1 == 1u);
}

static void TestPeriodicPatterns(void)
{
    ResetState();
    memory[0x1212] = 1u;
    memory[0x1302] = 0x10u;
    memory[0xfb88] = 1u;
    memory[0x121d] = 7u;
    TimerDispatchPhase2();
    assert(memory[0x121d] == 0u && p3_bit0 == 1u && p3_bit1 == 1u);

    memory[0x1212] = 1u;
    memory[0xfb88] = 2u;
    memory[0x121d] = 3u;
    TimerDispatchPhase2();
    assert(memory[0x121d] == 0u && p3_bit0 == 0u && p3_bit1 == 1u);

    memory[0x1212] = 1u;
    memory[0xfc21] = 8u;
    memory[0x121d] = 14u;
    p3_bit0 = 1u;
    TimerDispatchPhase2();
    assert(memory[0x121d] == 0u && p3_bit0 == 0u && p3_bit1 == 0u);

    ResetState();
    memory[0x1212] = 1u;
    memory[0x1009] = 0u;
    memory[0xfbf1] = 1u;
    memory[0x121d] = 14u;
    TimerDispatchPhase2();
    assert(memory[0x121d] == 0u && p3_bit0 == 1u && p3_bit1 == 0u);
}

static void TestFinalHandshake(void)
{
    ResetState();
    memory[0x1212] = 10u;
    memory[0x11dc] = 1u;
    p3_bit1 = 1u;
    TimerDispatchPhase2();
    assert(memory[0x1212] == 9u && p3_bit0 == 1u && p3_bit1 == 0u);

    ResetState();
    memory[0x1212] = 5u;
    memory[0x11dc] = 1u;
    TimerDispatchPhase2();
    assert(memory[0x1212] == 4u && p3_bit0 == 0u && p3_bit1 == 1u);
}

int main(void)
{
    TestFastCountdowns();
    TestDividedAndSlowerCountdowns();
    TestSerialHandshakes();
    TestPeriodicPatterns();
    TestFinalHandshake();
    puts("timer phase-two tests passed");
    return 0;
}
