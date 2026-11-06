#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/format_unit_code11_start.h"

static u8 memory[0x1000000ul];
static u8 events[32];
static unsigned event_count;
static u8 opc_result;
static u8 prepare_result;
static u8 initialize_result;
static u8 sense_key;
static u16 sense_code;

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

static void Record(u8 event)
{
    assert(event_count < sizeof(events));
    events[event_count++] = event;
}

void ObserveFormat11StartResetRawRead(void)
{
    Record(1u);
}

void ObserveFormat11StartInitializeCoroutine2(u32 entry_point)
{
    assert(entry_point == 0x00f84a00ul);
    Record(2u);
}

void ObserveFormat11StartYield(void)
{
    Record(3u);
}

u8 ObserveFormat11StartEnsureOpc(void)
{
    Record(4u);
    return opc_result;
}

void ObserveFormat11StartConfigureRing(u8 geometry_mode)
{
    assert(geometry_mode == 1u);
    Record(5u);
}

u8 ObserveFormat11StartPrepareType2(void)
{
    Record(6u);
    return prepare_result;
}

void ObserveFormat11StartSetSense(u8 key, u16 code)
{
    sense_key = key;
    sense_code = code;
    Record(7u);
}

void ObserveFormat11StartCompleteNoSense(void)
{
    Record(8u);
}

u8 ObserveFormat11StartInitializeType2(u8 mode)
{
    assert(mode == 0u);
    Record(9u);
    return initialize_result;
}

void ObserveFormat11StartDispatchCompletion(void)
{
    Record(10u);
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    opc_result = prepare_result = initialize_result = 0u;
    sense_key = 0xffu;
    sense_code = 0xffffu;
}

static void AssertEvents(const u8 *expected, unsigned count)
{
    assert(event_count == count);
    assert(memcmp(events, expected, count) == 0);
}

static void TestVariantZeroOpcFailure(void)
{
    static const u8 expected[] = { 1u, 2u, 3u, 4u };

    ResetFixture();
    memory[0x000c84u] = 0xffu;
    memory[0x000c8bu] = 0xffu;
    StartFormatUnitCode11Operation(0ul);
    AssertEvents(expected, sizeof(expected));
    assert(memory[0x000c8bu] == 0xfeu);
    assert(memory[0x000c84u] == 0xf7u);
}

static void TestVariantZeroPreparationFailure(void)
{
    static const u8 expected[] = { 1u, 2u, 3u, 4u, 5u, 6u, 7u };

    ResetFixture();
    opc_result = 1u;
    memory[0x000c84u] = 0xffu;
    memory[0x000c85u] = 0xffu;
    memory[0x000c86u] = 0xffu;
    memory[0x000c88u] = 0xffu;
    memory[0x000c89u] = 0xffu;
    memory[0x000c8au] = 0xffu;
    StartFormatUnitCode11Operation(0ul);
    AssertEvents(expected, sizeof(expected));
    assert(sense_key == 3u && sense_code == 0x0c07u);
    assert(memory[0x000c84u] == 0xf7u && memory[0x000c85u] == 0xfeu);
    assert(memory[0x000c86u] == 0xbfu && memory[0x000c88u] == 0x3fu);
    assert(memory[0x000c89u] == 0xfdu && memory[0x000c8au] == 0xf7u);
    assert(memory[0x0088b7u] == 0u);
}

static void TestVariantZeroSuccessSchedulesCountdown(void)
{
    static const u8 expected[] = { 1u, 2u, 3u, 4u, 5u, 6u };

    ResetFixture();
    opc_result = prepare_result = 1u;
    memory[0x000c84u] = 8u;
    memory[0x000c85u] = 1u;
    memory[0x000c86u] = 0x40u;
    memory[0x000c88u] = 0xc0u;
    memory[0x000c89u] = 0u;
    memory[0x000c8au] = 8u;
    StartFormatUnitCode11Operation(0ul);
    AssertEvents(expected, sizeof(expected));
    assert(memory[0x000c84u] == 0u && memory[0x000c85u] == 0u);
    assert(memory[0x000c86u] == 0u && memory[0x000c88u] == 0u);
    assert(memory[0x000c89u] == 8u && memory[0x000c8au] == 0u);
    assert(memory[0x0088b7u] == 0u && memory[0x00122bu] == 2u);
}

static void TestNonzeroVariantOpcFailure(void)
{
    static const u8 expected[] = { 1u, 2u, 3u, 4u };

    ResetFixture();
    memory[0x000c84u] = 0x80u;
    StartFormatUnitCode11Operation(7ul);
    AssertEvents(expected, sizeof(expected));
    assert(memory[0x000c84u] == 0x80u);
}

static void TestNonzeroVariantPipelineFailure(void)
{
    static const u8 expected[] = { 1u, 2u, 3u, 4u, 8u, 1u, 2u, 3u, 5u, 9u };

    ResetFixture();
    opc_result = 1u;
    StartFormatUnitCode11Operation(1ul);
    AssertEvents(expected, sizeof(expected));
    assert(memory[0x000c9eu] == 2u && memory[0x001586u] == 1u);
    assert(memory[0x000c88u] == 0x40u && memory[0x000c89u] == 2u);
    assert(memory[0x000c8au] == 8u);
}

static void TestNonzeroVariantSuccessDispatchesCompletion(void)
{
    static const u8 expected[] = { 1u, 2u, 3u, 4u, 1u, 2u, 3u, 5u, 9u, 10u };

    ResetFixture();
    opc_result = initialize_result = 1u;
    memory[0x000c85u] = 1u;
    memory[0x0088b7u] = 1u;
    StartFormatUnitCode11Operation(0x10000ul);
    AssertEvents(expected, sizeof(expected));
    assert(memory[0x000c85u] == 0u && memory[0x0088b7u] == 0u);
    assert(memory[0x000c88u] == 0x40u && memory[0x000c89u] == 0x10u);
    assert(memory[0x000c8au] == 0u);
}

int main(void)
{
    TestVariantZeroOpcFailure();
    TestVariantZeroPreparationFailure();
    TestVariantZeroSuccessSchedulesCountdown();
    TestNonzeroVariantOpcFailure();
    TestNonzeroVariantPipelineFailure();
    TestNonzeroVariantSuccessDispatchesCompletion();
    puts("format unit code-11 start tests passed");
    return 0;
}
