#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/drive_state_machine.h"

static u8 memory_bytes[0x20000ul];
static u8 completion_bit;
static u8 p3_bit0;
static u8 p3_bit1;
static u8 recognize_result;
static u8 indexed_state_result;
static unsigned p3_write_count;
static u8 p3_history[4][2];
static unsigned load_calibration_count;
static unsigned load_offsets_count;
static unsigned yield_count;
static unsigned recognize_count;
static unsigned indexed_transitions_count;
static unsigned configure_media_read_count;
static unsigned build_interpolation_count;
static unsigned initialize_media_read_count;
static unsigned indexed_state_count;
static u8 indexed_state_argument;
static unsigned restart_count;
static u32 restart_entries[3];
static u8 published_drive_state;
static u8 published_phase;
static u8 published_c85;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

void ObserveDriveStateMachineWriteP3(u8 bit0, u8 bit1)
{
    assert(p3_write_count < 4u);
    p3_history[p3_write_count][0] = bit0;
    p3_history[p3_write_count][1] = bit1;
    ++p3_write_count;
    p3_bit0 = bit0;
    p3_bit1 = bit1;
}

void ObserveDriveStateMachineClearCompletionBit(void)
{
    completion_bit = 0u;
}

void ObserveDriveStateMachineLoadCalibration(void)
{
    ++load_calibration_count;
}

void ObserveDriveStateMachineLoadOffsets(void)
{
    ++load_offsets_count;
}

void ObserveDriveStateMachineYield(void)
{
    ++yield_count;
    memory_bytes[0xfaebu] = 1u;
}

u8 ObserveDriveStateMachineRecognize(void)
{
    ++recognize_count;
    return recognize_result;
}

void ObserveDriveStateMachineIndexedTransitions(void)
{
    ++indexed_transitions_count;
    published_drive_state = memory_bytes[0x1009u];
    published_phase = memory_bytes[0xfaeu];
    published_c85 = memory_bytes[0xc85u];
}

void ObserveDriveStateMachineRestart(u32 entry_point)
{
    assert(restart_count < 3u);
    restart_entries[restart_count++] = entry_point;
}

void ObserveDriveStateMachineConfigureMediaRead(void)
{
    ++configure_media_read_count;
}

void ObserveDriveStateMachineBuildInterpolation(void)
{
    ++build_interpolation_count;
}

void ObserveDriveStateMachineInitializeMediaRead(void)
{
    ++initialize_media_read_count;
}

u8 ObserveDriveStateMachineIndexedState(u8 state_code)
{
    ++indexed_state_count;
    indexed_state_argument = state_code;
    return indexed_state_result;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(p3_history, 0, sizeof(p3_history));
    memset(restart_entries, 0, sizeof(restart_entries));
    completion_bit = 1u;
    p3_bit0 = p3_bit1 = 1u;
    recognize_result = indexed_state_result = 0u;
    p3_write_count = load_calibration_count = load_offsets_count = 0u;
    yield_count = recognize_count = indexed_transitions_count = 0u;
    configure_media_read_count = build_interpolation_count = 0u;
    initialize_media_read_count = indexed_state_count = restart_count = 0u;
    indexed_state_argument = published_drive_state = published_phase = 0u;
    published_c85 = 0u;
    memory_bytes[0xc85u] = 0x80u;
}

static void TestCommonInitializationAndWait(void)
{
    ResetFixture();
    memory_bytes[0x1304u] = 0xa0u;
    memory_bytes[0x1037u] = 0x55u;
    memory_bytes[0x1300u] = 0x66u;
    ProcessDriveStateMachine();

    assert(p3_write_count == 2u);
    assert(p3_history[0][0] == 0u && p3_history[0][1] == 0u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);
    assert(load_calibration_count == 1u && load_offsets_count == 1u);
    assert(completion_bit == 0u);
    assert(memory_bytes[0x1037u] == 0u);
    assert(memory_bytes[0x1304u] == 0xa1u);
    assert(memory_bytes[0x1300u] == 0u);
    assert(ReadWord(0x1004u) == 0x0078u);
    assert(yield_count == 1u && recognize_count == 1u);
    assert(indexed_transitions_count == 1u);
    assert(configure_media_read_count == 1u);
    assert(initialize_media_read_count == 1u);
    assert(indexed_state_count == 1u && indexed_state_argument == 0x0du);
    assert(restart_count == 1u && restart_entries[0] == 0x00f85d74ul);
    assert(memory_bytes[0xfaeu] == 0u);
}

static void TestRecognitionResultMappings(void)
{
    static const u8 results[] = { 2u, 0x11u, 0x14u, 0x15u, 0x16u, 0u, 0x7fu };
    static const u8 states[] = { 4u, 8u, 0x0cu, 0x0eu, 0x0fu, 0u, 5u };
    static const u8 phases[] = { 2u, 4u, 6u, 6u, 6u, 3u, 4u };
    unsigned index;

    for(index = 0u; index < sizeof(results); ++index)
    {
        ResetFixture();
        memory_bytes[0xfaebu] = 1u;
        recognize_result = results[index];
        ProcessDriveStateMachine();
        assert(published_drive_state == states[index]);
        assert(published_phase == phases[index]);
        assert(memory_bytes[0xfae7u] == states[index]);
        assert((published_c85 & 0x80u) == (results[index] == 2u ? 0u : 0x80u));
        if(states[index] == 0u)
        {
            assert(restart_count == 1u);
        }
        else
        {
            assert(restart_count == 2u);
            assert(restart_entries[0] == 0x00f853cful);
        }
        assert(restart_entries[restart_count - 1u] == 0x00f85d74ul);
    }
}

static void TestMediaTypeTwoIndexedRoute(void)
{
    ResetFixture();
    memory_bytes[0xfaebu] = 1u;
    memory_bytes[0x1008u] = 2u;
    indexed_state_result = 1u;
    ProcessDriveStateMachine();
    assert(build_interpolation_count == 1u);
    assert(restart_count == 1u && restart_entries[0] == 0x00fa6e33ul);

    ResetFixture();
    memory_bytes[0xfaebu] = 1u;
    memory_bytes[0x1008u] = 2u;
    indexed_state_result = 2u;
    ProcessDriveStateMachine();
    assert(build_interpolation_count == 1u);
    assert(restart_entries[0] == 0x00f85d74ul);
}

static void TestModeOneReadInitializationRoutes(void)
{
    ResetFixture();
    memory_bytes[0xfaebu] = 1u;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x130au] = 0x0eu;
    ProcessDriveStateMachine();
    assert(p3_history[0][0] == 0u && p3_history[0][1] == 1u);
    assert(initialize_media_read_count == 1u);
    assert((memory_bytes[0xc85u] & 2u) == 0u);

    ResetFixture();
    memory_bytes[0xfaebu] = 1u;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x130au] = 0x0fu;
    ProcessDriveStateMachine();
    assert(initialize_media_read_count == 1u);

    ResetFixture();
    memory_bytes[0xfaebu] = 1u;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x130au] = 0x0du;
    ProcessDriveStateMachine();
    assert(initialize_media_read_count == 0u);
    assert((memory_bytes[0xc85u] & 2u) != 0u);
}

int main(void)
{
    TestCommonInitializationAndWait();
    TestRecognitionResultMappings();
    TestMediaTypeTwoIndexedRoute();
    TestModeOneReadInitializationRoutes();
    puts("drive_state_machine_tests: ok");
    return 0;
}
