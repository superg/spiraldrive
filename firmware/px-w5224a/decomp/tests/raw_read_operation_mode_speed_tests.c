#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_operation_mode_speed.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x20000];
static u8 indexed_results[4];
static u8 opc_results[4];
static u8 repeat_results[4];
static u32 repeat_spacings[4];
static u8 prepare_result;
static u8 bit0406;
static u8 bit0402;
static u8 indexed_calls;
static u8 opc_calls;
static u8 repeat_calls;
static u8 yield_calls;
static u8 noop_calls;
static u8 normalize_calls;
static u8 tdi_calls;
static u8 initialize_calls;
static u32 initialized_entry;

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

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
    memset(memory_image, 0xa5, sizeof(memory_image));
    memset(indexed_results, 0, sizeof(indexed_results));
    memset(opc_results, 1, sizeof(opc_results));
    memset(repeat_results, 0, sizeof(repeat_results));
    memset(repeat_spacings, 0, sizeof(repeat_spacings));
    prepare_result = 1u;
    bit0406 = 0u;
    bit0402 = 0u;
    indexed_calls = 0u;
    opc_calls = 0u;
    repeat_calls = 0u;
    yield_calls = 0u;
    noop_calls = 0u;
    normalize_calls = 0u;
    tdi_calls = 0u;
    initialize_calls = 0u;
    initialized_entry = 0ul;
    WriteU32(0x156au, 0x200ul);
    WriteU32(0x1562u, 0x126100ul);
    memory_image[0xfafu] = 3u;
    memory_image[0x120au] = 0u;
    memory_image[0x1008u] = 1u;
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

u8 ObserveRawReadOperationIndexed(u8 state_code)
{
    assert(state_code == 0x1fu && indexed_calls < 4u);
    return indexed_results[indexed_calls++];
}

void ObserveRawReadOperationYield(void)
{
    ++yield_calls;
    if(memory_image[0x120au] != 0u)
        --memory_image[0x120au];
    else
    {
        memory_image[0xfafu] = 3u;
        memory_image[0x1302u] |= 0x04u;
    }
}

void ObserveRawReadOperationNoOp(void)
{
    ++noop_calls;
}

u8 ObserveRawReadOperationBit0406(void)
{
    return bit0406;
}

u8 ObserveRawReadOperationBit0402(void)
{
    return bit0402;
}

void ObserveRawReadOperationNormalize(void)
{
    ++normalize_calls;
}

u8 ObserveRawReadOperationOpc(void)
{
    assert(opc_calls < 4u);
    return opc_results[opc_calls++];
}

void ObserveRawReadOperationBuildTdi(fw_addr_t destination)
{
    ++tdi_calls;
    assert(destination == 0x143cul);
}

u8 ObserveRawReadOperationPrepareSynthetic(void)
{
    return prepare_result;
}

u8 ObserveRawReadOperationAcquireRepeated(u32 frame_spacing)
{
    u8 index = repeat_calls++;

    assert(index < 4u);
    repeat_spacings[index] = frame_spacing;
    return repeat_results[index];
}

void ObserveRawReadOperationInitializeCoroutine1(u32 entry_point)
{
    ++initialize_calls;
    initialized_entry = entry_point;
}

static void AssertResetImage(void)
{
    assert(ReadU16(0x1298u) == 0u && ReadU16(0x1294u) == 0u && ReadU16(0x1296u) == 0u && ReadU16(0x1292u) == 0u);
    assert(ReadU16(0x129au) == 0x1ffu && ReadU16(0x129cu) == 0x1fau);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x1262u) == 0ul && ReadU32(0x1266u) == 0ul);
    assert(ReadU32(0x126au) == 0x126100ul && ReadU32(0x126eu) == 0ul);
    assert(memory_image[0x14cbu] == 0u && memory_image[0x127du] == 0u);
    assert(memory_image[0x120au] == 0u && noop_calls == 1u);
}

static void TestOrdinaryReadinessAndPassThroughSpeed(void)
{
    ResetHarness();
    memory_image[0xfafu] = 1u;
    assert(ExecuteRawReadOperationForModeAndSpeed(2u, 12u) == 1u);
    AssertResetImage();
    assert(yield_calls == 11u);
    assert(memory_image[0xc38u] == 12u && memory_image[0xe47u] == 12u);
    assert(normalize_calls == 1u && opc_calls == 1u && tdi_calls == 1u);
}

static void TestIndexedReadinessAndModeZeroSpeed(void)
{
    ResetHarness();
    indexed_results[0] = 1u;
    memory_image[0xfafu] = 1u;
    memory_image[0x1008u] = 2u;
    memory_image[0xfb88u] = 0u;
    memory_image[0x130au] = 3u;
    memory_image[0xfb87u] = 0x40u;
    assert(ExecuteRawReadOperationForModeAndSpeed(0u, 52u) == 1u);
    assert(yield_calls == 11u);
    assert(memory_image[0xc38u] == 10u && memory_image[0xe47u] == 10u);
    assert((memory_image[0xc84u] & 0x20u) == 0u);

    ResetHarness();
    memory_image[0x1008u] = 2u;
    memory_image[0xfb88u] = 2u;
    memory_image[0xfb87u] = 0u;
    assert(ExecuteRawReadOperationForModeAndSpeed(0u, 4u) == 1u);
    assert(memory_image[0xc38u] == 4u);
}

static void TestModeFourPortSpeedMatrix(void)
{
    ResetHarness();
    memory_image[0xfb88u] = 1u;
    bit0406 = 1u;
    repeat_results[0] = 1u;
    assert(ExecuteRawReadOperationForModeAndSpeed(4u, 1u) == 1u);
    assert(memory_image[0xc38u] == 10u);
    assert(repeat_calls == 1u && repeat_spacings[0] == 30ul);
    assert(initialize_calls == 1u && initialized_entry == 0xf84b0bul);

    ResetHarness();
    memory_image[0xfb88u] = 1u;
    bit0402 = 1u;
    repeat_results[0] = 0u;
    assert(ExecuteRawReadOperationForModeAndSpeed(4u, 1u) == 0u);
    assert(memory_image[0xc38u] == 8u && initialize_calls == 0u);
}

static void TestIndexedOpcFailureMapping(void)
{
    ResetHarness();
    indexed_results[1] = 1u;
    memory_image[0x1008u] = 2u;
    memory_image[0xef72u] = 0x80u;
    opc_results[0] = 0u;
    WriteU16(0xfad7u, 0x0008u);
    assert(ExecuteRawReadOperationForModeAndSpeed(2u, 4u) == 0u);
    assert(opc_calls == 1u && memory_image[0xfba0u] == 7u);
    assert(memory_image[0xef72u] == 0x80u && tdi_calls == 0u);

    ResetHarness();
    opc_results[0] = 0u;
    WriteU16(0xfad7u, 0u);
    assert(ExecuteRawReadOperationForModeAndSpeed(2u, 4u) == 0u);
    assert(memory_image[0xfba0u] == 8u);
}

static void TestModeZeroPreparationResult(void)
{
    ResetHarness();
    prepare_result = 0u;
    assert(ExecuteRawReadOperationForModeAndSpeed(0u, 4u) == 0u);
    assert(tdi_calls == 1u);
}

static void TestRepeatedWindowModes(void)
{
    ResetHarness();
    WriteU16(0xf9dfu, 123u);
    repeat_results[0] = 1u;
    repeat_results[1] = 0u;
    assert(ExecuteRawReadOperationForModeAndSpeed(3u, 4u) == 0u);
    assert(repeat_calls == 2u && repeat_spacings[0] == 123ul && repeat_spacings[1] == 123ul);

    ResetHarness();
    repeat_results[0] = 1u;
    repeat_results[1] = 0u;
    assert(ExecuteRawReadOperationForModeAndSpeed(0x1fu, 4u) == 0u);
    assert(repeat_calls == 2u && repeat_spacings[0] == 10ul && repeat_spacings[1] == 10ul);
}

int main(void)
{
    TestOrdinaryReadinessAndPassThroughSpeed();
    TestIndexedReadinessAndModeZeroSpeed();
    TestModeFourPortSpeedMatrix();
    TestIndexedOpcFailureMapping();
    TestModeZeroPreparationResult();
    TestRepeatedWindowModes();
    puts("raw-read operation mode/speed tests passed");
    return 0;
}
