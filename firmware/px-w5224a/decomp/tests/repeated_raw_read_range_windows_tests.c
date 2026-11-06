#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/repeated_raw_read_range_windows.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x300000];
static u8 prepare_results[1002];
static u8 prefix_results[1002];
static u16 prepare_calls;
static u16 prefix_calls;
static u16 opc_calls;
static u16 build_q_calls;
static u16 yield_calls;
static u16 initialize_calls;
static s32 starts[1002];
static s32 ends[1002];
static u32 initialized_entry;
static u8 opc_result;
static u8 build_q_result;

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(prepare_results, 1, sizeof(prepare_results));
    memset(prefix_results, 1, sizeof(prefix_results));
    memset(starts, 0, sizeof(starts));
    memset(ends, 0, sizeof(ends));
    prepare_calls = 0u;
    prefix_calls = 0u;
    opc_calls = 0u;
    build_q_calls = 0u;
    yield_calls = 0u;
    initialize_calls = 0u;
    initialized_entry = 0ul;
    opc_result = 1u;
    build_q_result = 1u;
    memory_image[0xfafu] = 3u;
    memory_image[0x1302u] = 4u;
    WriteU32(0x1153u, 60000ul);
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

void ObserveRepeatedRangeYield(void)
{
    ++yield_calls;
    memory_image[0xfafu] = 3u;
    memory_image[0x1302u] |= 4u;
}

void ObserveRepeatedRangeInitializeCoroutine1(u32 entry_point)
{
    ++initialize_calls;
    initialized_entry = entry_point;
}

s32 ObserveRepeatedRangeConvert(fw_addr_t tuple_address)
{
    assert(tuple_address == 0x002bb03eul + (fw_addr_t)memory_image[0x1149u] * 10ul);
    return 12345l;
}

u32 ObserveRepeatedRangeMultiply(u32 left, u32 right)
{
    assert(left == 75ul);
    return left * right;
}

u8 ObserveRepeatedRangePrepare(u8 use_plan, s32 start, s32 end)
{
    u16 index = prepare_calls++;
    assert(use_plan == 0u && index < 1002u);
    starts[index] = start;
    ends[index] = end;
    return prepare_results[index];
}

u8 ObserveRepeatedRangePrefix(u8 omit_final, u8 wait)
{
    u16 index = prefix_calls++;
    assert(omit_final == 0u && wait == 1u && index < 1002u);
    WriteU32(0x1598u, ReadU32(0x8000u));
    return prefix_results[index];
}

u16 ObserveRepeatedRangeRemainder20(u16 value)
{
    return (u16)(value % 20u);
}

u8 ObserveRepeatedRangeOpc(void)
{
    ++opc_calls;
    return opc_result;
}

u8 ObserveRepeatedRangeBuildQ(void)
{
    ++build_q_calls;
    return build_q_result;
}

static void TestSingleWindowFromZero(void)
{
    ResetHarness();
    memory_image[0x1008u] = 1u;
    assert(AcquireRepeatedRawReadRangeWindows(10ul) == 1u);
    assert(prepare_calls == 1u && starts[0] == 0l && ends[0] == 748l);
    assert(build_q_calls == 1u);
    assert((memory_image[0x1302u] & 4u) == 0u);
    assert(memory_image[0xc48u] == 8u && memory_image[0xc44u] == 1u && memory_image[0xdd31u] == 0x41u && memory_image[0xdd36u] == 0x41u);
}

static void TestTrackStartAndInvalidTrack(void)
{
    ResetHarness();
    memory_image[0x1149u] = 7u;
    assert(AcquireRepeatedRawReadRangeWindows(2ul) == 1u);
    assert(starts[0] == 12345l && ends[0] == 12493l);

    ResetHarness();
    memory_image[0x1149u] = 99u;
    assert(AcquireRepeatedRawReadRangeWindows(2ul) == 0u);
    assert(memory_image[0xfba0u] == 9u && initialize_calls == 1u && initialized_entry == 0xf84b0bul && prepare_calls == 0u);
}

static void TestState1fSingleSpecialWindow(void)
{
    ResetHarness();
    memory_image[0x1008u] = 2u;
    memory_image[0x130au] = 0x1fu;
    assert(AcquireRepeatedRawReadRangeWindows(4ul) == 1u);
    assert(prepare_calls == 1u && starts[0] == 45000l && ends[0] == 45298l);
}

static void TestState11PeriodicOpcFailure(void)
{
    ResetHarness();
    memory_image[0x1008u] = 2u;
    memory_image[0x130au] = 0x11u;
    opc_result = 0u;
    WriteU16(0xfad7u, 8u);
    assert(AcquireRepeatedRawReadRangeWindows(1ul) == 0u);
    assert(prepare_calls == 20u && opc_calls == 1u);
    assert(starts[0] == 750l && ends[0] == 59998l);
    assert(memory_image[0xe74eu] == 0u && memory_image[0xe74fu] == 1u && memory_image[0xfba0u] == 7u);
}

static void TestPreparationAndPrefixFailures(void)
{
    ResetHarness();
    prepare_results[0] = 0u;
    assert(AcquireRepeatedRawReadRangeWindows(1ul) == 0u);
    assert(prefix_calls == 0u);

    ResetHarness();
    prefix_results[0] = 0u;
    assert(AcquireRepeatedRawReadRangeWindows(1ul) == 0u);
    assert(prepare_calls == 1u && prefix_calls == 1u);
}

static void TestTerminalQueueFailure(void)
{
    ResetHarness();
    memory_image[0x1008u] = 1u;
    build_q_result = 0u;
    assert(AcquireRepeatedRawReadRangeWindows(1ul) == 0u);
    assert(build_q_calls == 1u && (memory_image[0x1302u] & 4u) != 0u);
}

int main(void)
{
    TestSingleWindowFromZero();
    TestTrackStartAndInvalidTrack();
    TestState1fSingleSpecialWindow();
    TestState11PeriodicOpcFailure();
    TestPreparationAndPrefixFailures();
    TestTerminalQueueFailure();
    puts("repeated raw-read range window tests passed");
    return 0;
}
