#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/state5_read_acquisition_sequence.h"
#include "platform/firmware_memory.h"

#define RESULT_CAPACITY 8u
#define EVENT_CAPACITY 128u

struct OperationEvent
{
    u8 operation;
    s32 argument;
};

static u8 memory_bytes[0x20000ul];
static s32 results[MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED + 1u][RESULT_CAPACITY];
static s32 defaults[MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED + 1u];
static unsigned result_lengths[MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED + 1u];
static unsigned operation_counts[MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED + 1u];
static struct OperationEvent events[EVENT_CAPACITY];
static unsigned event_count;
static u8 mutate_target_on_acquire;
static u8 clear_eligibility_on_accumulate;

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

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

s32 ObserveMediaState5ReadAcquisitionOperation(u8 operation, s32 argument)
{
    unsigned index;

    assert(operation <= MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED);
    assert(event_count < EVENT_CAPACITY);
    events[event_count].operation = operation;
    events[event_count].argument = argument;
    ++event_count;
    index = operation_counts[operation]++;
    if(operation == MEDIA_STATE5_ACQUIRE_POSITION && mutate_target_on_acquire != 0u)
        WriteDword(0x0e3bu, 1234ul);
    if(operation == MEDIA_STATE5_ACCUMULATE_SPEED_FLAGS && clear_eligibility_on_accumulate != 0u)
        WriteWord(0xfad7u, 0u);
    if(index < result_lengths[operation])
        return results[operation][index];
    return defaults[operation];
}

static void SetResult(u8 operation, unsigned index, s32 result)
{
    assert(index < RESULT_CAPACITY);
    results[operation][index] = result;
    if(result_lengths[operation] <= index)
        result_lengths[operation] = index + 1u;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(results, 0, sizeof(results));
    memset(defaults, 0, sizeof(defaults));
    memset(result_lengths, 0, sizeof(result_lengths));
    memset(operation_counts, 0, sizeof(operation_counts));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    mutate_target_on_acquire = 0u;
    clear_eligibility_on_accumulate = 0u;
    defaults[MEDIA_STATE5_ACQUIRE_POSITION] = 1l;
    defaults[MEDIA_STATE5_ACQ_INITIALIZE_CONTROLLER] = 1l;
    defaults[MEDIA_STATE5_CHECK_TOLERANCE] = 1l;
    defaults[MEDIA_STATE5_WAIT_FOR_POSITION] = 1l;
    defaults[MEDIA_STATE5_READ_P0_BIT7] = 1l;
    defaults[MEDIA_STATE5_PROBE_READINESS] = 1l;
    defaults[MEDIA_STATE5_PROBE_THRESHOLDS] = 1l;
    defaults[MEDIA_STATE5_COMPUTE_RADIAL_COORDINATE] = 30l;
    WriteDword(0x0e3bu, 10000ul);
    memory_bytes[0x0e4bu] = 1u;
    memory_bytes[0x0e4fu] = 0xffu;
    memory_bytes[0xfafcu] = 5u;
    memory_bytes[0x0fd0u] = 0x10u;
}

static void AssertEventArgument(u8 operation, unsigned occurrence, s32 argument)
{
    unsigned index;
    unsigned seen = 0u;

    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].operation == operation)
        {
            if(seen == occurrence)
            {
                assert(events[index].argument == argument);
                return;
            }
            ++seen;
        }
    }
    assert(0);
}

static void TestAcquisitionZeroRoutes(void)
{
    ResetFixture();
    defaults[MEDIA_STATE5_ACQUIRE_POSITION] = 0l;
    memory_bytes[0xfc3cu] = 1u;
    memory_bytes[0x1305u] = 0xffu;
    memory_bytes[0x0e40u] = 1u;
    memory_bytes[0x0fafu] = 9u;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 1u);
    assert(memory_bytes[0x1305u] == 0xddu);
    assert(memory_bytes[0x0e40u] == 0u);
    assert(memory_bytes[0x0fafu] == 0u);
    assert(operation_counts[MEDIA_STATE5_RESET_DRIVE_CONTROL] == 0u);

    ResetFixture();
    defaults[MEDIA_STATE5_ACQUIRE_POSITION] = 0l;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 2u);
    assert(operation_counts[MEDIA_STATE5_RESET_DRIVE_CONTROL] == 1u);
    assert(operation_counts[MEDIA_STATE5_ACQ_INITIALIZE_CONTROLLER] == 1u);
    AssertEventArgument(MEDIA_STATE5_SET_OPERATING_ENABLE, 0u, 1l);

    ResetFixture();
    defaults[MEDIA_STATE5_ACQUIRE_POSITION] = 0l;
    defaults[MEDIA_STATE5_ACQ_INITIALIZE_CONTROLLER] = 0l;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 1u);
    assert(memory_bytes[0x0e41u] == 7u);
    assert(memory_bytes[0xfaabu] == 7u);
    assert(memory_bytes[0x0fafu] == 0u);
}

static void TestMinusOneRestoresOnlyUnchangedTarget(void)
{
    ResetFixture();
    defaults[MEDIA_STATE5_ACQUIRE_POSITION] = 0xffl;
    memory_bytes[0x0e3fu] = 1u;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 1u);
    assert(ReadDword(0x0e3bu) == 10000ul);
    assert(memory_bytes[0x0e3fu] == 0u);

    ResetFixture();
    defaults[MEDIA_STATE5_ACQUIRE_POSITION] = 0xffl;
    mutate_target_on_acquire = 1u;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 1u);
    assert(ReadDword(0x0e3bu) == 1234ul);
}

static void TestToleranceRetryPublication(void)
{
    ResetFixture();
    defaults[MEDIA_STATE5_CHECK_TOLERANCE] = 0l;
    memory_bytes[0x0e43u] = 0x10u;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 2u);
    assert((memory_bytes[0x1305u] & 0x22u) == 0x22u);
    assert(memory_bytes[0x131cu] == 0x80u);
    AssertEventArgument(MEDIA_STATE5_CHECK_TOLERANCE, 0u, 1l);
    AssertEventArgument(MEDIA_STATE5_STROBE_PORT_8080, 0u, 0x80l);
}

static void TestNonRequestedOperatingCodeRecovery(void)
{
    ResetFixture();
    memory_bytes[0x0e4bu] = 2u;
    memory_bytes[0xfad5u] = 1u;
    WriteWord(0xfad7u, 3u);
    clear_eligibility_on_accumulate = 1u;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 2u);
    assert(operation_counts[MEDIA_STATE5_ACCUMULATE_SPEED_FLAGS] == 1u);
    AssertEventArgument(MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED, 0u, 0l);
    assert(memory_bytes[0xfad5u] == 0u);
    assert(memory_bytes[0x0e41u] == 7u);
    assert(memory_bytes[0xfaabu] == 7u);
}

static void TestPositionWaitFailureRoutes(void)
{
    ResetFixture();
    defaults[MEDIA_STATE5_WAIT_FOR_POSITION] = 0l;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 1u);
    assert(ReadDword(0x0e3bu) == 10000ul);
    assert(memory_bytes[0x0e40u] == 0u);

    ResetFixture();
    defaults[MEDIA_STATE5_WAIT_FOR_POSITION] = 0l;
    memory_bytes[0x0fafu] = 3u;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 2u);
    assert(ReadDword(0x0e3bu) == 9900ul);
}

static void TestP0RecoveryRoutes(void)
{
    ResetFixture();
    defaults[MEDIA_STATE5_READ_P0_BIT7] = 0l;
    memory_bytes[0x1301u] = 0xffu;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 2u);
    assert(memory_bytes[0x0e41u] == 5u);
    assert((memory_bytes[0x1301u] & 1u) == 0u);
    assert(operation_counts[MEDIA_STATE5_REFRESH_PARAMETERS_FC_F6] == 1u);

    ResetFixture();
    defaults[MEDIA_STATE5_READ_P0_BIT7] = 0l;
    defaults[MEDIA_STATE5_ACQ_INITIALIZE_CONTROLLER] = 0l;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 1u);
    assert(memory_bytes[0x0e41u] == 7u);
    assert(memory_bytes[0xfaabu] == 7u);
}

static void TestCompleteReadinessSequence(void)
{
    ResetFixture();
    memory_bytes[0x1008u] = 2u;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0xfb9bu] = 1u;
    memory_bytes[0xf977u] = 2u;
    memory_bytes[0x0e43u] = 0x28u;
    memory_bytes[0x1137u] = 7u;
    WriteWord(0xf9feu, 0x1111u);
    WriteWord(0xf9fcu, 0x2222u);
    WriteDword(0xfba2u, 0x12345678ul);
    WriteDword(0x1235u, 5000ul);
    defaults[MEDIA_STATE5_COMPUTE_RADIAL_COORDINATE] = 50l;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 0u);

    AssertEventArgument(MEDIA_STATE5_WAIT_FOR_POSITION, 0u, 9480l);
    AssertEventArgument(MEDIA_STATE5_WAIT_FOR_POSITION, 1u, 9600l);
    AssertEventArgument(MEDIA_STATE5_WAIT_FOR_POSITION, 2u, 9970l);
    AssertEventArgument(MEDIA_STATE5_COMPUTE_RADIAL_COORDINATE, 0u, (s32)0x00072222ul);
    AssertEventArgument(MEDIA_STATE5_ACQ_RESTORE_READ_PATH, 0u, 0l);
    assert(operation_counts[MEDIA_STATE5_PROBE_READINESS] == 2u);
    assert(operation_counts[MEDIA_STATE5_CONFIGURE_ACQUISITION_CODE] == 1u);
    assert(ReadDword(0xf97cu) == 0x12345678ul);
    assert(memory_bytes[0xf979u] == 0u);
    assert((memory_bytes[0xf978u] & 4u) != 0u);
    assert(memory_bytes[0x0e51u] == 0x28u);
}

static void TestReadinessAndThresholdFailures(void)
{
    ResetFixture();
    defaults[MEDIA_STATE5_PROBE_READINESS] = 0l;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 2u);
    assert(memory_bytes[0x0e41u] == 0x13u);

    ResetFixture();
    memory_bytes[0x0fd0u] = 0u;
    defaults[MEDIA_STATE5_PROBE_THRESHOLDS] = 0l;
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 2u);
    assert(memory_bytes[0x131cu] == 0u);
    assert(memory_bytes[0x0e41u] == 0x13u);
    AssertEventArgument(MEDIA_STATE5_STROBE_PORT_8080, 1u, 0l);
}

static void TestProfileGenerationTransitions(void)
{
    ResetFixture();
    memory_bytes[0x1008u] = 1u;
    memory_bytes[0x0e43u] = 0x34u;
    memory_bytes[0xfa05u] = 0x20u;
    WriteWord(0xf9feu, 0x3456u);
    assert(RunMediaState5ReadAcquisitionSequence(100l) == 0u);
    AssertEventArgument(MEDIA_STATE5_GENERATE_PROFILES, 0u, 0x3456l);
    assert(memory_bytes[0xfa05u] == 0x34u);
}

int main(void)
{
    TestAcquisitionZeroRoutes();
    TestMinusOneRestoresOnlyUnchangedTarget();
    TestToleranceRetryPublication();
    TestNonRequestedOperatingCodeRecovery();
    TestPositionWaitFailureRoutes();
    TestP0RecoveryRoutes();
    TestCompleteReadinessSequence();
    TestReadinessAndThresholdFailures();
    TestProfileGenerationTransitions();
    puts("media state-5 read-acquisition sequence tests passed");
    return 0;
}
