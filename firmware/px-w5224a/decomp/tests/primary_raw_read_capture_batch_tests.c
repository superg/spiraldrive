#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/primary_raw_read_capture_batch.h"
#include "platform/firmware_memory.h"

typedef struct SubmitRecord
{
    fw_addr_t address;
    u8 parameter;
} SubmitRecord;

static u8 memory[0x1000000ul];
static u8 busy_reads[8];
static unsigned busy_read_count;
static unsigned busy_read_index;
static unsigned yield_count;
static s32 conversion_frames[8];
static fw_addr_t conversion_destinations[8];
static unsigned conversion_count;
static u16 pack_addresses[8];
static unsigned pack_count;
static fw_addr_t copy_sources[8];
static fw_addr_t copy_destinations[8];
static u16 copy_counts[8];
static unsigned copy_count;
static fw_addr_t controller_blocks[8];
static unsigned controller_block_count;
static SubmitRecord submits[8];
static unsigned submit_count;
static u32 multiply_left[8];
static u32 multiply_right[8];
static unsigned multiply_count;
static u16 increment_addresses[24];
static unsigned increment_count;

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

u8 ObservePrimaryRawReadCaptureBusy(void)
{
    assert(busy_read_index < busy_read_count);
    return busy_reads[busy_read_index++];
}

void ObservePrimaryRawReadCaptureYield(void)
{
    ++yield_count;
}

void ObservePrimaryRawReadCaptureConvert(s32 frame_position, fw_addr_t destination)
{
    assert(conversion_count < 8u);
    conversion_frames[conversion_count] = frame_position;
    conversion_destinations[conversion_count] = destination;
    ++conversion_count;
    memory[destination] = (u8)((u32)frame_position >> 16);
    memory[destination + 1u] = (u8)((u32)frame_position >> 8);
    memory[destination + 2u] = (u8)frame_position;
}

void ObservePrimaryRawReadCapturePackBcd(u16 tuple_address)
{
    u8 index;

    assert(pack_count < 8u);
    pack_addresses[pack_count++] = tuple_address;
    for(index = 0u; index < 3u; ++index)
        memory[(fw_addr_t)tuple_address + index] ^= 0x80u;
}

void ObservePrimaryRawReadCaptureCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    u16 index;

    assert(copy_count < 8u);
    copy_sources[copy_count] = source;
    copy_destinations[copy_count] = destination;
    copy_counts[copy_count] = byte_count;
    ++copy_count;
    for(index = 0u; index < byte_count; ++index)
        memory[destination + index] = memory[source + index];
}

void ObservePrimaryRawReadCaptureWriteControllerBlock(fw_addr_t source_address)
{
    assert(controller_block_count < 8u);
    controller_blocks[controller_block_count++] = source_address;
}

void ObservePrimaryRawReadCaptureSubmit(fw_addr_t controller_buffer_address, u8 command_parameter)
{
    assert(submit_count < 8u);
    submits[submit_count].address = controller_buffer_address;
    submits[submit_count].parameter = command_parameter;
    ++submit_count;
}

u32 ObservePrimaryRawReadCaptureMultiply(u32 factor_a, u32 factor_b)
{
    assert(multiply_count < 8u);
    multiply_left[multiply_count] = factor_a;
    multiply_right[multiply_count] = factor_b;
    ++multiply_count;
    return factor_a * factor_b;
}

void ObservePrimaryRawReadCaptureIncrement(u16 tuple_address)
{
    assert(increment_count < 24u);
    increment_addresses[increment_count++] = tuple_address;
    ++memory[(fw_addr_t)tuple_address + 2u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void ResetFixture(void)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(busy_reads, 0, sizeof(busy_reads));
    memset(conversion_frames, 0, sizeof(conversion_frames));
    memset(conversion_destinations, 0, sizeof(conversion_destinations));
    memset(pack_addresses, 0, sizeof(pack_addresses));
    memset(copy_sources, 0, sizeof(copy_sources));
    memset(copy_destinations, 0, sizeof(copy_destinations));
    memset(copy_counts, 0, sizeof(copy_counts));
    memset(controller_blocks, 0, sizeof(controller_blocks));
    memset(submits, 0, sizeof(submits));
    memset(multiply_left, 0, sizeof(multiply_left));
    memset(multiply_right, 0, sizeof(multiply_right));
    memset(increment_addresses, 0, sizeof(increment_addresses));
    busy_read_count = 1u;
    busy_read_index = 0u;
    yield_count = 0u;
    conversion_count = 0u;
    pack_count = 0u;
    copy_count = 0u;
    controller_block_count = 0u;
    submit_count = 0u;
    multiply_count = 0u;
    increment_count = 0u;
    WriteU32(0x1598u, 1000ul);
    WriteU32(0x155eu, 0x1260ul);
    WriteU32(0x1562u, 0x126100ul);
    WriteU32(0x156au, 2ul);
    WriteU32(0x156eu, 0x156f00ul);
}

static void TestWaitAndZeroBatchInitialization(void)
{
    unsigned index;

    ResetFixture();
    busy_reads[0] = 1u;
    busy_reads[1] = 1u;
    busy_reads[2] = 0u;
    busy_read_count = 3u;

    PrepareAndSubmitPrimaryRawReadCaptureBatch(0u);

    assert(yield_count == 2u && busy_read_index == 3u);
    assert(conversion_count == 3u && pack_count == 3u && copy_count == 3u);
    assert(conversion_frames[0] == 1000l && conversion_frames[1] == 850l && conversion_frames[2] == 1000l);
    for(index = 0u; index < 3u; ++index)
    {
        assert(conversion_destinations[index] == 0x7000ul);
        assert(pack_addresses[index] == 0x7000u);
        assert(copy_sources[index] == 0x7000ul && copy_counts[index] == 3u);
    }
    assert(copy_destinations[0] == 0x1428ul);
    assert(copy_destinations[1] == 0x1275ul);
    assert(copy_destinations[2] == 0x1279ul);
    assert(memory[0x1272u] == 0x71u && memory[0x1273u] == 1u && memory[0x1274u] == 1u && memory[0x1278u] == 0u && memory[0x142bu] == 2u);
    assert(submit_count == 0u && increment_count == 0u);
}

static void TestTwoFrameBatchWrapsAllLocalCursors(void)
{
    fw_addr_t metadata_first = 0x256f10ul;
    fw_addr_t metadata_second = 0x256f00ul;
    fw_addr_t auxiliary_first = 0x226226ul;
    fw_addr_t auxiliary_second = 0x226100ul;
    u8 first_q[10];
    u8 second_q[10];
    unsigned index;

    ResetFixture();
    WriteU32(0x126eu, 0x930ul);
    WriteU32(0x1262u, 1ul);
    WriteU32(0x126au, 0x126226ul);
    memory[0x0f06u] = 1u;
    memory[0x0f0bu] = 2u;

    PrepareAndSubmitPrimaryRawReadCaptureBatch(2u);

    assert(submit_count == 2u && controller_block_count == 2u);
    assert(submits[0].address == 0x930ul && submits[1].address == 0ul);
    assert(submits[0].parameter == 0xaau && submits[1].parameter == 0xaau);
    assert(controller_blocks[0] == 0x1428ul && controller_blocks[1] == 0x1428ul);

    for(index = 0u; index < 10u; ++index)
        first_q[index] = memory[0x1272u + index];
    memcpy(second_q, first_q, sizeof(second_q));
    first_q[5] = (u8)(first_q[5] - 2u);
    first_q[9] = (u8)(first_q[9] - 2u);
    --second_q[5];
    --second_q[9];
    for(index = 0u; index < 10u; ++index)
    {
        assert(memory[metadata_first + index] == first_q[index]);
        assert(memory[metadata_second + index] == second_q[index]);
    }
    assert(memory[metadata_first + 10u] == 0u && memory[metadata_first + 11u] == 0u && memory[metadata_second + 10u] == 0u && memory[metadata_second + 11u] == 0u);

    for(index = 0u; index < 0x126u; ++index)
    {
        assert(memory[auxiliary_first + index] == 0u);
        assert(memory[auxiliary_second + index] == 0u);
    }
    assert(memory[auxiliary_second - 1u] == 0xa5u && memory[auxiliary_first + 0x126u] == 0xa5u);
    assert(multiply_count == 3u);
    for(index = 0u; index < multiply_count; ++index)
        assert(multiply_left[index] == 0x126ul && multiply_right[index] == 2ul);
    assert(increment_count == 6u);
    for(index = 0u; index < increment_count; index += 3u)
    {
        assert(increment_addresses[index] == 0x1428u);
        assert(increment_addresses[index + 1u] == 0x1279u);
        assert(increment_addresses[index + 2u] == 0x1275u);
    }
    assert(memory[0x142au] == (u8)(memory[0x127bu] + 0u));
    assert(memory[0x1262u] == 0u && memory[0x1265u] == 1u);
    assert(memory[0x126du] == 0x26u && memory[0x1271u] == 0x30u);
}

static void TestSignedFormatAndDisabledAuxiliarySkipOptionalWrites(void)
{
    ResetFixture();
    memory[0x0f06u] = 0xffu;
    memory[0x0f0bu] = 0u;

    PrepareAndSubmitPrimaryRawReadCaptureBatch(1u);

    assert(submit_count == 1u && multiply_count == 0u);
    assert(memory[0x256f00u] == 0xa5u);
    assert(memory[0x226100u] == 0xa5u);
    assert(increment_count == 3u);
}

int main(void)
{
    TestWaitAndZeroBatchInitialization();
    TestTwoFrameBatchWrapsAllLocalCursors();
    TestSignedFormatAndDisabledAuxiliarySkipOptionalWrites();
    puts("primary raw-read capture batch tests passed");
    return 0;
}
