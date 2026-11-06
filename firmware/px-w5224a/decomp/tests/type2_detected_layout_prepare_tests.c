#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/type2_detected_layout_prepare.h"

enum Event
{
    EVENT_CONVERT = 1,
    EVENT_MULTIPLY,
    EVENT_PROBE_RECORD,
    EVENT_RESET,
    EVENT_CONFIGURE_RING,
    EVENT_INITIALIZE_PRIMARY,
    EVENT_INITIALIZE_TABLES,
    EVENT_INITIALIZE_MIP,
    EVENT_INITIALIZE_RUNTIME,
    EVENT_PROBE_TDI,
    EVENT_LOCATE,
    EVENT_SCAN
};

static u8 memory[0x20000ul];
static enum Event events[64];
static unsigned event_count;
static u8 probe_results[8];
static unsigned probe_result_count;
static unsigned probe_count;
static s32 probe_positions[8];
static u8 probe_tdi_result;
static u8 locate_result;
static u8 scan_result;
static u8 probe_tdi_argument;
static u8 scan_argument;

static void Record(enum Event event)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count++] = event;
}

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

s32 ObserveType2DetectedLayoutConvertMsf(fw_addr_t address)
{
    Record(EVENT_CONVERT);
    assert(address == 0xdd94ul);
    assert(memory[0x102fu] == 1u);
    return 12345l;
}

u32 ObserveType2DetectedLayoutMultiply(u32 left, u32 right)
{
    Record(EVENT_MULTIPLY);
    assert(left == 5ul);
    return left * right;
}

u8 ObserveType2DetectedLayoutProbeRecord(s32 frame_position)
{
    Record(EVENT_PROBE_RECORD);
    assert(probe_count < probe_result_count);
    probe_positions[probe_count] = frame_position;
    return probe_results[probe_count++];
}

void ObserveType2DetectedLayoutReset(void)
{
    Record(EVENT_RESET);
    memory[0x0c9eu] = 0u;
}

void ObserveType2DetectedLayoutConfigureRing(u8 geometry_mode)
{
    Record(EVENT_CONFIGURE_RING);
    assert(geometry_mode == 1u);
}

void ObserveType2DetectedLayoutInitializePrimary(void)
{
    Record(EVENT_INITIALIZE_PRIMARY);
}

void ObserveType2DetectedLayoutInitializeTables(void)
{
    Record(EVENT_INITIALIZE_TABLES);
}

void ObserveType2DetectedLayoutInitializeMip(void)
{
    Record(EVENT_INITIALIZE_MIP);
}

void ObserveType2DetectedLayoutInitializeRuntime(void)
{
    Record(EVENT_INITIALIZE_RUNTIME);
}

u8 ObserveType2DetectedLayoutProbeTdi(u8 fixed_offset)
{
    Record(EVENT_PROBE_TDI);
    probe_tdi_argument = fixed_offset;
    return probe_tdi_result;
}

u8 ObserveType2DetectedLayoutLocate(void)
{
    Record(EVENT_LOCATE);
    return locate_result;
}

u8 ObserveType2DetectedLayoutScan(u8 fallback_scan)
{
    Record(EVENT_SCAN);
    scan_argument = fallback_scan;
    return scan_result;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(probe_results, 0, sizeof(probe_results));
    memset(probe_positions, 0, sizeof(probe_positions));
    memory[0xdd93u] = 0x50u;
    memory[0xdd9cu] = 1u;
    event_count = 0u;
    probe_result_count = 0u;
    probe_count = 0u;
    probe_tdi_result = 1u;
    locate_result = 1u;
    scan_result = 1u;
    probe_tdi_argument = 0xffu;
    scan_argument = 0xffu;
}

static void AssertEvents(const enum Event *expected, unsigned count)
{
    assert(event_count == count);
    assert(memcmp(events, expected, count * sizeof(*expected)) == 0);
}

static void TestIneligibleTrackIsAlreadySuccessful(void)
{
    ResetFixture();
    memory[0xdd93u] = 0x40u;

    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 1u);
    assert(event_count == 0u);

    ResetFixture();
    memory[0xdd9cu] = 0u;
    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 1u);
    assert(event_count == 0u);
}

static void TestStatus0fInitializesAllDetectedLayoutState(void)
{
    static const enum Event expected[] = { EVENT_CONVERT, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_CONFIGURE_RING, EVENT_INITIALIZE_PRIMARY, EVENT_INITIALIZE_TABLES, EVENT_INITIALIZE_MIP,
        EVENT_INITIALIZE_RUNTIME };

    ResetFixture();
    probe_results[0] = 0x0fu;
    probe_result_count = 1u;

    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 1u);
    assert(memory[0x102fu] == 0u);
    assert(probe_positions[0] == -30l);
    AssertEvents(expected, sizeof(expected) / sizeof(expected[0]));
}

static void TestStatusFeResetsThenAcceptsZeroMediaState(void)
{
    static const enum Event expected[] = { EVENT_CONVERT, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_RESET };

    ResetFixture();
    probe_results[0] = 0xfeu;
    probe_result_count = 1u;

    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 1u);
    AssertEvents(expected, sizeof(expected) / sizeof(expected[0]));
}

static void TestFourStatusFfCandidatesUseFallbackScanAndReset(void)
{
    static const enum Event expected[] = { EVENT_CONVERT, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_MULTIPLY,
        EVENT_PROBE_RECORD, EVENT_PROBE_TDI, EVENT_SCAN, EVENT_RESET };

    ResetFixture();
    probe_results[0] = 0xffu;
    probe_results[1] = 0xffu;
    probe_results[2] = 0xffu;
    probe_results[3] = 0xffu;
    probe_result_count = 4u;
    memory[0x0c9fu] = 1u;

    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 0u);
    assert(probe_positions[0] == -30l && probe_positions[1] == -25l && probe_positions[2] == -20l && probe_positions[3] == -15l);
    assert(probe_tdi_argument == 1u && scan_argument == 1u);
    AssertEvents(expected, sizeof(expected) / sizeof(expected[0]));
}

static void TestStatusFiveFailsAndResetsImmediately(void)
{
    static const enum Event expected[] = { EVENT_CONVERT, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_RESET };

    ResetFixture();
    probe_results[0] = 5u;
    probe_result_count = 1u;

    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 0u);
    AssertEvents(expected, sizeof(expected) / sizeof(expected[0]));
}

static void TestAcceptedFollowupOrderTracksDetectionFlag(void)
{
    static const enum Event zero_flag[] = { EVENT_CONVERT, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_CONFIGURE_RING, EVENT_INITIALIZE_PRIMARY, EVENT_INITIALIZE_TABLES, EVENT_INITIALIZE_MIP,
        EVENT_INITIALIZE_RUNTIME, EVENT_PROBE_TDI, EVENT_LOCATE, EVENT_SCAN };
    static const enum Event set_flag[] = { EVENT_CONVERT, EVENT_MULTIPLY, EVENT_PROBE_RECORD, EVENT_CONFIGURE_RING, EVENT_INITIALIZE_PRIMARY, EVENT_INITIALIZE_TABLES, EVENT_INITIALIZE_MIP,
        EVENT_INITIALIZE_RUNTIME, EVENT_PROBE_TDI, EVENT_SCAN, EVENT_LOCATE };

    ResetFixture();
    probe_results[0] = 0x0fu;
    probe_result_count = 1u;
    memory[0x0c9eu] = 1u;
    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 1u);
    assert(probe_tdi_argument == 0u && scan_argument == 0u);
    AssertEvents(zero_flag, sizeof(zero_flag) / sizeof(zero_flag[0]));

    ResetFixture();
    probe_results[0] = 0x0fu;
    probe_result_count = 1u;
    memory[0x0c9eu] = 3u;
    memory[0x0c9fu] = 1u;
    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 1u);
    AssertEvents(set_flag, sizeof(set_flag) / sizeof(set_flag[0]));
}

static void TestFollowupFailureRestoresDetectionState(void)
{
    ResetFixture();
    probe_results[0] = 0x0fu;
    probe_result_count = 1u;
    memory[0x0c9eu] = 1u;
    locate_result = 0u;

    assert(PrepareType2ControllerRecordPipelineFromDetectedLayout() == 0u);
    assert(events[event_count - 2u] == EVENT_LOCATE);
    assert(events[event_count - 1u] == EVENT_RESET);
}

int main(void)
{
    TestIneligibleTrackIsAlreadySuccessful();
    TestStatus0fInitializesAllDetectedLayoutState();
    TestStatusFeResetsThenAcceptsZeroMediaState();
    TestFourStatusFfCandidatesUseFallbackScanAndReset();
    TestStatusFiveFailsAndResetsImmediately();
    TestAcceptedFollowupOrderTracksDetectionFlag();
    TestFollowupFailureRestoresDetectionState();
    puts("type2 detected-layout preparation tests passed");
    return 0;
}
