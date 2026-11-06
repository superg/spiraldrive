#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/lead_in_raw_sector_descriptor_sequence.h"
#include "platform/firmware_memory.h"

#define MEMORY_SIZE 0x2c2000ul

static u8 memory_image[MEMORY_SIZE];
static u8 bit0028;
static u8 bit0041;
static u8 adjacent_class;
static int capture_with_q_failure_call;
static int capture_failure_call;
static int wait_failure_call;
static u16 initialize_calls;
static u16 synthetic_calls;
static u16 merge_calls;
static u16 fixed_prefix_calls;
static u16 q_prefix_calls;
static u16 capture_with_q_calls;
static u16 capture_calls;
static u16 wait_calls;
static u16 populate_calls;
static u16 update_calls;
static u16 enqueue_calls;
static u16 increment_calls;
static u16 advance_calls;
static u16 yield_calls;
static u16 latch_calls;
static u16 sense_calls;
static u16 information_calls;
static u8 sense_key;
static u16 sense_code;
static u32 sense_information;
static fw_addr_t capture_with_q_cursors[32];
static fw_addr_t captured_cursors[16];
static u8 enqueued_q[32][10];

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

static fw_addr_t TrackRecord(u16 index)
{
    return 0xb4bdul + (fw_addr_t)index * 8u;
}

static void SeedQTable(void)
{
    u8 index;

    WriteU16(0x8008u, 1u);
    WriteU16(0x800au, 1u);
    for(index = 0u; index != 10u; ++index)
        memory_image[0x8012u + index] = (u8)(0x10u + index);
    memory_image[0x8012u] = 0x11u;
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    bit0028 = 0u;
    bit0041 = 1u;
    adjacent_class = 0u;
    capture_with_q_failure_call = -1;
    capture_failure_call = -1;
    wait_failure_call = -1;
    initialize_calls = 0u;
    synthetic_calls = 0u;
    merge_calls = 0u;
    fixed_prefix_calls = 0u;
    q_prefix_calls = 0u;
    capture_with_q_calls = 0u;
    capture_calls = 0u;
    wait_calls = 0u;
    populate_calls = 0u;
    update_calls = 0u;
    enqueue_calls = 0u;
    increment_calls = 0u;
    advance_calls = 0u;
    yield_calls = 0u;
    latch_calls = 0u;
    sense_calls = 0u;
    information_calls = 0u;
    sense_key = 0u;
    sense_code = 0u;
    sense_information = 0u;
    memset(capture_with_q_cursors, 0, sizeof(capture_with_q_cursors));
    memset(captured_cursors, 0, sizeof(captured_cursors));
    memset(enqueued_q, 0, sizeof(enqueued_q));
    WriteU32(0x156au, 8ul);
    WriteU32(0x1562u, 0x126100ul);
    WriteU32(0x114fu, 0xfffffffbul);
    memory_image[0xc44u] = 0u;
    memory_image[0x2bc139u] = 0u;
    memory_image[0x2bb5deu] = 1u;
    memory_image[0x2bb5dfu] = 2u;
    memory_image[0x2bb5e0u] = 3u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < MEMORY_SIZE);
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory_image[address] = value;
}

u8 ObserveLeadInSequenceReadBit0028(void)
{
    return bit0028;
}
void ObserveLeadInSequenceClearBit0028(void)
{
    bit0028 = 0u;
}
u8 ObserveLeadInSequenceReadBit0041(void)
{
    return bit0041;
}
void ObserveLeadInSequenceWriteBit0041(u8 value)
{
    bit0041 = value;
}

void ObserveLeadInSequenceInitializeToc(void)
{
    ++initialize_calls;
}

void ObserveLeadInSequenceBuildSyntheticQ(void)
{
    ++synthetic_calls;
    SeedQTable();
}

void ObserveLeadInSequenceMergeStagedQ(void)
{
    ++merge_calls;
    SeedQTable();
}

u16 ObserveLeadInSequenceFindTrack(u8 track_number)
{
    assert(track_number == memory_image[0x14a5u]);
    return 1u;
}

u8 ObserveLeadInSequenceDecodeType(u16 record_index)
{
    assert(record_index == 1u);
    return 9u;
}

u8 ObserveLeadInSequenceFindAdjacent(u16 record_index, u8 forward)
{
    assert(forward == 1u && record_index == 1u);
    return adjacent_class;
}

void ObserveLeadInSequenceConvert(s32 frame_position, fw_addr_t output_tuple)
{
    memory_image[output_tuple] = (u8)((u32)frame_position >> 16);
    memory_image[output_tuple + 1u] = (u8)((u32)frame_position >> 8);
    memory_image[output_tuple + 2u] = (u8)frame_position;
}

void ObserveLeadInSequencePack(u16 tuple_address)
{
    memory_image[tuple_address] ^= 0x10u;
    memory_image[tuple_address + 1u] ^= 0x10u;
    memory_image[tuple_address + 2u] ^= 0x10u;
}

void ObserveLeadInSequenceCopyTuple(u16 destination, u16 source)
{
    memory_image[destination] = memory_image[source];
    memory_image[destination + 1u] = memory_image[source + 1u];
    memory_image[destination + 2u] = memory_image[source + 2u];
}

u8 ObserveLeadInSequenceBinaryToBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

u8 ObserveLeadInSequenceFixedPrefix(u8 mode, u8 source_kind)
{
    ++fixed_prefix_calls;
    assert(mode == 2u && source_kind == 7u);
    WriteU32(0x1598u, ReadU32(0x1598u) + 1ul);
    return 1u;
}

u8 ObserveLeadInSequenceQPrefix(s32 frame_count, u8 reverse)
{
    ++q_prefix_calls;
    assert(frame_count == 1l && reverse == 0u);
    WriteU32(0x1598u, ReadU32(0x1598u) + 1ul);
    return 1u;
}

fw_addr_t ObserveLeadInSequenceResumeRing(void)
{
    return 0x930ul;
}

u8 ObserveLeadInSequenceCaptureWithQ(fw_addr_t tuple, fw_addr_t cursor, u8 source_kind)
{
    int call = (int)capture_with_q_calls;

    assert(tuple == 0x7000ul && source_kind == 7u);
    assert(capture_with_q_calls < 32u);
    capture_with_q_cursors[capture_with_q_calls++] = cursor;
    return (u8)(call != capture_with_q_failure_call);
}

u8 ObserveLeadInSequenceWaitCapacity(void)
{
    int call = (int)wait_calls++;
    return (u8)(call != wait_failure_call);
}

void ObserveLeadInSequenceLatchFailure(void)
{
    ++latch_calls;
}

void ObserveLeadInSequencePopulateSubheader(fw_addr_t cursor, u8 source_kind)
{
    ++populate_calls;
    assert(source_kind == 7u && cursor != 0ul);
}

void ObserveLeadInSequenceUpdateQ(void)
{
    ++update_calls;
}

void ObserveLeadInSequenceEnqueue(void)
{
    u8 index;

    assert(enqueue_calls < 32u);
    for(index = 0u; index != 10u; ++index)
        enqueued_q[enqueue_calls][index] = memory_image[0x1272u + index];
    ++enqueue_calls;
}

void ObserveLeadInSequenceIncrement(u16 tuple_address)
{
    ++increment_calls;
    ++memory_image[tuple_address + 2u];
}

fw_addr_t ObserveLeadInSequenceAdvanceRing(fw_addr_t cursor)
{
    ++advance_calls;
    return cursor + 0x930ul;
}

void ObserveLeadInSequenceYield(void)
{
    ++yield_calls;
}

void ObserveLeadInSequenceWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
}

u8 ObserveLeadInSequenceCapture(fw_addr_t cursor, u8 source_kind)
{
    int call = (int)capture_calls;

    assert(source_kind == 10u && capture_calls < 16u);
    captured_cursors[capture_calls++] = cursor;
    return (u8)(call != capture_failure_call);
}

void ObserveLeadInSequenceBuildRid01(fw_addr_t cursor)
{
    assert((cursor & 0xfu) == 0u);
}

u32 ObserveLeadInSequenceMultiply(u32 left, u32 right)
{
    return left * right;
}

void ObserveLeadInSequenceSetSense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}

void ObserveLeadInSequenceSetInformation(u32 information)
{
    ++information_calls;
    sense_information = information;
}

static void TestOrdinaryStagedSequence(void)
{
    u8 index;

    ResetHarness();
    bit0028 = 1u;
    memory_image[0xfba0u] = 7u;
    memset(&memory_image[0x8012u], 0xa5, 2200u);

    assert(AcquireLeadInRawSectorDescriptorSequence(1u, 0u) == 1u);
    assert(initialize_calls == 1u && merge_calls == 1u);
    assert(bit0028 == 0u && memory_image[0xfba0u] == 0u);
    assert(ReadU16(0x129au) == 7u && ReadU16(0x129cu) == 2u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x126au) == 0x126100ul);
    assert(q_prefix_calls == 1u && fixed_prefix_calls == 0u);
    assert(capture_with_q_calls == 5u && wait_calls == 0u);
    assert(update_calls == 5u && enqueue_calls == 5u);
    assert(ReadU32(0x1598u) == 1ul);
    assert(memory_image[0x1289u] == 0u);
    for(index = 0u; index != 5u; ++index)
        assert(capture_with_q_cursors[index] == (fw_addr_t)(0x930ul * (index + 1u)));
    assert(enqueued_q[0][0] == 0x11u && enqueued_q[0][2] == 0x12u);
    assert(memory_image[0x100930u] == 0u && memory_image[0x10093fu] == 0u);
}

static void TestModeTwoCapacityPath(void)
{
    ResetHarness();
    memory_image[0x2bc139u] = 0x40u;
    WriteU32(0x156au, 1ul);

    assert(AcquireLeadInRawSectorDescriptorSequence(1u, 0u) == 1u);
    assert(fixed_prefix_calls == 1u && q_prefix_calls == 0u);
    assert(capture_with_q_calls == 3u);
    assert(wait_calls == 2u && populate_calls == 2u);
    assert(memory_image[0x10126fu] == 1u);
    assert(memory_image[0x101b9fu] == 1u);
    assert(enqueue_calls == 5u && memory_image[0x1289u] == 0u);
}

static void TestAdr6Injection(void)
{
    ResetHarness();
    bit0041 = 0u;

    assert(AcquireLeadInRawSectorDescriptorSequence(1u, 0u) == 1u);
    assert(enqueued_q[0][0] == 0x16u);
    assert(enqueued_q[0][1] == 0u && enqueued_q[0][2] == 0u);
    assert(enqueued_q[0][7] == 0x01u && enqueued_q[0][8] == 0x02u && enqueued_q[0][9] == 0x03u);
    assert(enqueued_q[1][0] == 0x11u);
}

static void TestProducerFailureLatches(void)
{
    ResetHarness();
    WriteU32(0x156au, 0ul);
    WriteU32(0x114fu, 0xfffffffdul);
    wait_failure_call = 0;

    assert(AcquireLeadInRawSectorDescriptorSequence(1u, 0u) == 0u);
    assert(wait_calls == 1u && latch_calls == 1u);
    assert(enqueue_calls == 0u);
}

static void TestSpecialTypeTwoSequence(void)
{
    ResetHarness();
    memory_image[0xc44u] = 2u;
    memory_image[0xc45u] = 0u;
    bit0041 = 0u;
    WriteU32(0x114fu, 0xfffffffeul);
    memory_image[TrackRecord(0u) + 3u] = 0u;
    memory_image[TrackRecord(1u)] = 1u;
    memory_image[TrackRecord(1u) + 1u] = 1u;
    memory_image[TrackRecord(3u)] = 1u;
    memory_image[TrackRecord(3u) + 1u] = 1u;
    memory_image[0x14a5u] = 1u;

    assert(AcquireLeadInRawSectorDescriptorSequence(1u, 0u) == 1u);
    assert(capture_calls == 10u && bit0041 == 1u);
    assert(synthetic_calls == 1u && capture_with_q_calls == 1u);
    assert(ReadU32(0x1598u) == 0ul);
    assert(ReadU16(0x149du) == 3u && ReadU16(0x149bu) == 3u);
}

static void TestSpecialCaptureFailure(void)
{
    ResetHarness();
    memory_image[0xc44u] = 2u;
    memory_image[0xc45u] = 0u;
    bit0041 = 0u;
    capture_failure_call = 0;

    assert(AcquireLeadInRawSectorDescriptorSequence(1u, 0u) == 0u);
    assert(capture_calls == 1u && sense_calls == 1u);
    assert(sense_key == 4u && sense_code == 0x4400u);
    assert(information_calls == 1u);
    assert(sense_information == ReadU32(0x114fu) - 10ul);
}

static void TestTypeTwoSpecialMarkerBypass(void)
{
    ResetHarness();
    memory_image[0xc44u] = 2u;
    memory_image[TrackRecord(0u) + 3u] = 0x41u;
    memory_image[TrackRecord(1u)] = 1u;
    memory_image[TrackRecord(1u) + 1u] = 1u;
    memory_image[TrackRecord(3u)] = 1u;
    memory_image[TrackRecord(3u) + 1u] = 1u;
    memory_image[0x14a5u] = 1u;

    assert(AcquireLeadInRawSectorDescriptorSequence(0u, 0u) == 1u);
    assert(memory_image[0x88b5u] == 1u && memory_image[0x1285u] == 9u);
    assert(capture_calls == 0u && capture_with_q_calls == 0u);
    assert(memory_image[0x1289u] == 1u);
}

int main(void)
{
    TestOrdinaryStagedSequence();
    TestModeTwoCapacityPath();
    TestAdr6Injection();
    TestProducerFailureLatches();
    TestSpecialTypeTwoSequence();
    TestSpecialCaptureFailure();
    TestTypeTwoSpecialMarkerBypass();
    puts("lead_in_raw_sector_descriptor_sequence_tests: ok");
    return 0;
}
