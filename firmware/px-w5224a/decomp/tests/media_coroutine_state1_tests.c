#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/media_coroutine_state1.h"

#define RESULT_SLOTS 8u
#define EVENT_SLOTS 128u

struct OperationEvent
{
    u8 operation;
    s32 argument;
};

static u8 memory_bytes[0x14000ul];
static s32 operation_results[MEDIA_STATE1_PROCESS_INDEXED_COMPLETION + 1u][RESULT_SLOTS];
static s32 default_results[MEDIA_STATE1_PROCESS_INDEXED_COMPLETION + 1u];
static unsigned result_lengths[MEDIA_STATE1_PROCESS_INDEXED_COMPLETION + 1u];
static unsigned operation_counts[MEDIA_STATE1_PROCESS_INDEXED_COMPLETION + 1u];
static struct OperationEvent events[EVENT_SLOTS];
static unsigned event_count;
static u8 p3_bit0;
static u8 p3_bit1;
static unsigned p3_write_count;
static u8 fba_writes[8u];
static unsigned fba_write_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
    if(address == 0x000fbaul && fba_write_count < sizeof(fba_writes))
        fba_writes[fba_write_count++] = value;
}

s32 ObserveMediaState1Operation(u8 operation, s32 argument)
{
    unsigned call_index;

    assert(operation <= MEDIA_STATE1_PROCESS_INDEXED_COMPLETION);
    assert(event_count < EVENT_SLOTS);
    events[event_count].operation = operation;
    events[event_count].argument = argument;
    ++event_count;
    call_index = operation_counts[operation]++;
    if(call_index < result_lengths[operation])
        return operation_results[operation][call_index];
    return default_results[operation];
}

void ObserveMediaState1WriteP3(u8 bit0, u8 bit1)
{
    p3_bit0 = (u8)(bit0 != 0u);
    p3_bit1 = (u8)(bit1 != 0u);
    ++p3_write_count;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static void SetResult(u8 operation, unsigned index, s32 result)
{
    assert(index < RESULT_SLOTS);
    operation_results[operation][index] = result;
    if(result_lengths[operation] <= index)
        result_lengths[operation] = index + 1u;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(operation_results, 0, sizeof(operation_results));
    memset(default_results, 0, sizeof(default_results));
    memset(result_lengths, 0, sizeof(result_lengths));
    memset(operation_counts, 0, sizeof(operation_counts));
    memset(events, 0, sizeof(events));
    memset(fba_writes, 0, sizeof(fba_writes));
    event_count = p3_write_count = fba_write_count = 0u;
    p3_bit0 = p3_bit1 = 0u;
    default_results[MEDIA_STATE1_HANDLE_ACQUISITION_STATUS] = 1;
    default_results[MEDIA_STATE1_PROCESS_DRIVE_FLAGS] = 2;
    WriteDword(0x000e3bul, 1000ul);
}

static void TestEarlyAcquisitionExitAndOrderedFlags(void)
{
    ResetFixture();
    memory_bytes[0x0fbau] = 2u;
    default_results[MEDIA_STATE1_ACQUIRE_POSITION] = 7;
    ProcessMediaCoroutineState1();

    assert(memory_bytes[0x1007u] == 3u);
    assert(fba_write_count == 2u);
    assert(fba_writes[0] == 3u && fba_writes[1] == 1u);
    assert(ReadDword(0x0e3bu) == 994ul);
    assert(memory_bytes[0x0e40u] == 1u);
    assert(memory_bytes[0x0ed6u] == 1u);
    assert(operation_counts[MEDIA_STATE1_HANDLE_ACQUISITION_STATUS] == 1u);
    assert(events[event_count - 1u].argument == 7);
    assert(operation_counts[MEDIA_STATE1_CONFIGURE_PORTS_8085_8086] == 0u);
}

static void TestTargetVariantsAndRetryPublication(void)
{
    ResetFixture();
    memory_bytes[0x1006u] = 1u;
    ProcessMediaCoroutineState1();
    assert(ReadDword(0x0e3bu) == 1000ul);

    ResetFixture();
    memory_bytes[0x0fbau] = 0x10u;
    ProcessMediaCoroutineState1();
    assert(ReadDword(0x0e3bu) == 999ul);

    ResetFixture();
    memory_bytes[0x1304u] = 0x40u;
    memory_bytes[0xfbf1u] = 1u;
    memory_bytes[0xfafdu] = 9u;
    ProcessMediaCoroutineState1();
    assert(ReadDword(0x0e3bu) == 984ul);
    assert(memory_bytes[0xfafdu] == 10u);
    assert(memory_bytes[0x1009u] == 1u);
    assert(memory_bytes[0xfae7u] == 0x52u);
    assert(operation_counts[MEDIA_STATE1_RESTART_COROUTINE] == 1u);
}

static void TestFullSetupAndCompletionSignal(void)
{
    ResetFixture();
    default_results[MEDIA_STATE1_HANDLE_ACQUISITION_STATUS] = 0;
    memory_bytes[0x0e3fu] = 1u;
    ProcessMediaCoroutineState1();

    assert(operation_counts[MEDIA_STATE1_CONFIGURE_PORTS_8085_8086] == 1u);
    assert(operation_counts[MEDIA_STATE1_APPLY_OPERATING_CHANNELS] == 1u);
    assert(operation_counts[MEDIA_STATE1_BUILD_PORT_SHADOWS] == 1u);
    assert(operation_counts[MEDIA_STATE1_APPLY_PORT_SHADOWS] == 1u);
    assert(operation_counts[MEDIA_STATE1_SET_P1_BIT4] == 1u);
    assert(operation_counts[MEDIA_STATE1_REARM_Q_CAPTURE] == 1u);
    assert(memory_bytes[0x1218u] == 100u);
    assert(memory_bytes[0x121bu] == 0xffu);
    assert(memory_bytes[0x0e3fu] == 0u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);
}

static void TestActiveIndexedRoute(void)
{
    ResetFixture();
    default_results[MEDIA_STATE1_HANDLE_ACQUISITION_STATUS] = 0;
    default_results[MEDIA_STATE1_PROCESS_DRIVE_FLAGS] = 0;
    default_results[MEDIA_STATE1_TEST_INDEXED_STATE] = 1;
    default_results[MEDIA_STATE1_SELECT_NARROW_TARGET] = 1;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x130au] = 6u;
    WriteDword(0x1235u, 1000ul);
    ProcessMediaCoroutineState1();

    assert(operation_counts[MEDIA_STATE1_SET_OPERATING_ENABLE] == 1u);
    assert(operation_counts[MEDIA_STATE1_TEST_INDEXED_STATE] == 1u);
    assert(operation_counts[MEDIA_STATE1_CONFIGURE_PORTS_8079_809D_809F] == 1u);
    assert(operation_counts[MEDIA_STATE1_SELECT_NARROW_TARGET] == 1u);
    assert(operation_counts[MEDIA_STATE1_SET_TARGET_FROM_Q] == 0u);
    assert(memory_bytes[0x130bu] == 10u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);
}

static void TestInactiveWideRoute(void)
{
    ResetFixture();
    default_results[MEDIA_STATE1_HANDLE_ACQUISITION_STATUS] = 0;
    default_results[MEDIA_STATE1_PROCESS_DRIVE_FLAGS] = 0;
    default_results[MEDIA_STATE1_PROCESS_INDEXED_COMPLETION] = 1;
    memory_bytes[0x0fbau] = 0x10u;
    ProcessMediaCoroutineState1();

    assert(operation_counts[MEDIA_STATE1_APPLY_PARSED_Q_PORT_STATE] == 1u);
    assert(operation_counts[MEDIA_STATE1_EXECUTE_DIRECTION_MOVE] == 1u);
    assert(operation_counts[MEDIA_STATE1_SET_TARGET_FROM_Q] == 1u);
    assert(operation_counts[MEDIA_STATE1_PROCESS_INDEXED_COMPLETION] == 1u);
    assert(operation_counts[MEDIA_STATE1_SERVICE_TARGET_WINDOW] == 0u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);
}

static void TestStateFiveCalibrationLoopMasksResult(void)
{
    ResetFixture();
    default_results[MEDIA_STATE1_HANDLE_ACQUISITION_STATUS] = 0;
    default_results[MEDIA_STATE1_PROCESS_DRIVE_FLAGS] = 0;
    default_results[MEDIA_STATE1_CALIBRATE_PORT_8097] = 3;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x130au] = 5u;
    ProcessMediaCoroutineState1();

    assert((memory_bytes[0x1306u] & 0x20u) != 0u);
    assert(operation_counts[MEDIA_STATE1_CALIBRATE_PORT_8097] == 1u);
    assert(operation_counts[MEDIA_STATE1_CALIBRATE_PARAMETER_F4] == 1u);
    assert(operation_counts[MEDIA_STATE1_INITIALIZE_CALIBRATION] == 1u);
    assert(ReadDword(0x0e3bu) == 0x00020f58ul);
    assert(memory_bytes[0x0fafu] == 1u);
    assert(memory_bytes[0x0e3fu] == 0u);
}

static void TestCalibrationRestartRetainsAcquisitionStatus(void)
{
    ResetFixture();
    default_results[MEDIA_STATE1_ACQUIRE_POSITION] = 1;
    default_results[MEDIA_STATE1_HANDLE_ACQUISITION_STATUS] = 0;
    memory_bytes[0xf79au] = 1u;
    memory_bytes[0xfbf1u] = 1u;
    ProcessMediaCoroutineState1();

    assert(operation_counts[MEDIA_STATE1_ACQUIRE_POSITION] == 1u);
    assert(operation_counts[MEDIA_STATE1_SERVICE_CALIBRATION] == 1u);
    assert(operation_counts[MEDIA_STATE1_RESTART_COROUTINE] == 1u);
    assert(memory_bytes[0x1009u] == 1u);
    assert(memory_bytes[0xfae7u] == 0x51u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);
}

static void TestOrdinaryTargetWindowRetriesOuterAcquisition(void)
{
    ResetFixture();
    SetResult(MEDIA_STATE1_HANDLE_ACQUISITION_STATUS, 0u, 0);
    SetResult(MEDIA_STATE1_HANDLE_ACQUISITION_STATUS, 1u, 1);
    default_results[MEDIA_STATE1_PROCESS_DRIVE_FLAGS] = 0;
    default_results[MEDIA_STATE1_SERVICE_TARGET_WINDOW] = 0;
    ProcessMediaCoroutineState1();

    assert(operation_counts[MEDIA_STATE1_ACQUIRE_POSITION] == 2u);
    assert(operation_counts[MEDIA_STATE1_SERVICE_TARGET_WINDOW] == 1u);
    assert((memory_bytes[0x0fbau] & 0x80u) == 0u);
}

int main(void)
{
    TestEarlyAcquisitionExitAndOrderedFlags();
    TestTargetVariantsAndRetryPublication();
    TestFullSetupAndCompletionSignal();
    TestActiveIndexedRoute();
    TestInactiveWideRoute();
    TestStateFiveCalibrationLoopMasksResult();
    TestCalibrationRestartRetainsAcquisitionStatus();
    TestOrdinaryTargetWindowRetriesOuterAcquisition();
    puts("media coroutine state-1 tests passed");
    return 0;
}
