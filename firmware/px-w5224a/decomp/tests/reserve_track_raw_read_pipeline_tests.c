#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/reserve_track_raw_read_pipeline.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x10000];
static u8 staged_tuple[3];
static u8 queue_results[2], queue_calls, opc_result, opc_calls;
static u8 plan_result, plan_calls, poll_result, poll_calls;
static u8 reset_calls, fill_calls, bcd_calls, msf_calls;
static u8 sense_calls, sense_key, sequence_error_calls;
static u8 queue_sense_calls, failure_calls, failure_code;
static u16 sense_code;
static s32 msf_result;
static u8 mutate_write_type_on_msf;
static u8 mutate_speed_on_first_queue;

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void ResetHarness(void)
{
    memset(memory_image, 0xa5, sizeof(memory_image));
    staged_tuple[0] = 0x12u;
    staged_tuple[1] = 0x34u;
    staged_tuple[2] = 0x56u;
    queue_results[0] = queue_results[1] = 1u;
    queue_calls = opc_calls = 0u;
    opc_result = plan_result = poll_result = 1u;
    plan_calls = poll_calls = reset_calls = fill_calls = 0u;
    bcd_calls = msf_calls = sense_calls = sequence_error_calls = 0u;
    queue_sense_calls = failure_calls = failure_code = 0u;
    sense_key = 0u;
    sense_code = 0u;
    msf_result = 0x00123456l;
    mutate_write_type_on_msf = 0xffu;
    mutate_speed_on_first_queue = 0xffu;
    WriteU32(0x156au, 16ul);
    WriteU32(0x1562u, 0x000fedcbul);
    memory_image[0x0e47u] = 8u;
    memory_image[0x0c38u] = 24u;
    memory_image[0x0c44u] = 2u;
    memory_image[0x0c46u] = 0u;
    memory_image[0x0c48u] = 0x41u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    if(address >= 0x2bc122ul && address <= 0x2bc124ul)
        return staged_tuple[address - 0x2bc122ul];
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

void ObserveReserveTrackRawReset(void)
{
    ++reset_calls;
}
void ObserveReserveTrackRawFill(fw_addr_t address, u32 count, u8 value)
{
    ++fill_calls;
    assert(address == 0ul && count == 0x1b0000ul && value == 0u);
}
u8 ObserveReserveTrackRawInitializeQueue(void)
{
    u8 index = queue_calls++;
    assert(index < 2u);
    if(index == 0u && mutate_speed_on_first_queue != 0xffu)
        memory_image[0x0e47u] = mutate_speed_on_first_queue;
    return queue_results[index];
}
u8 ObserveReserveTrackRawRunOpc(void)
{
    ++opc_calls;
    return opc_result;
}
void ObserveReserveTrackRawSetSense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}
void ObserveReserveTrackRawConvertBcd(u16 address)
{
    ++bcd_calls;
    assert(address == 0x7000u);
    memory_image[address] = 12u;
    memory_image[address + 1u] = 34u;
    memory_image[address + 2u] = 56u;
}
s32 ObserveReserveTrackRawConvertMsf(fw_addr_t address)
{
    ++msf_calls;
    assert(address == 0x7000ul);
    assert(memory_image[address] == 12u && memory_image[address + 1u] == 34u && memory_image[address + 2u] == 56u);
    if(mutate_write_type_on_msf != 0xffu)
        memory_image[0x0c44u] = mutate_write_type_on_msf;
    return msf_result;
}
void ObserveReserveTrackRawSequenceError(void)
{
    ++sequence_error_calls;
}
u8 ObserveReserveTrackRawPreparePlan(u8 alternate)
{
    ++plan_calls;
    assert(alternate == 0u);
    return plan_result;
}
u8 ObserveReserveTrackRawPoll(u8 count)
{
    ++poll_calls;
    assert(count == 1u);
    return poll_result;
}
void ObserveReserveTrackRawQueueSense(void)
{
    ++queue_sense_calls;
}
void ObserveReserveTrackRawFailure(u8 code)
{
    ++failure_calls;
    failure_code = code;
}

static void CheckInitialReset(void)
{
    assert(reset_calls == 1u && fill_calls == 1u);
    assert(memory_image[0x14b5u] == 0u && memory_image[0x1490u] == 0u);
    assert(memory_image[0x0c2au] == 6u);
    assert(memory_image[0x127du] == 0u && memory_image[0x128cu] == 0u && memory_image[0x128du] == 0u && memory_image[0x1286u] == 0u);
    assert(memory_image[0x14b4u] == 1u);
    assert(ReadU16(0x1298u) == 0u && ReadU16(0x1292u) == 0u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x1262u) == 0u && ReadU32(0x1266u) == 0u);
    assert(ReadU32(0x126au) == 0x000fedcbul && ReadU32(0x126eu) == 0u);
}

static void TestLowSpeedQueueFailure(void)
{
    ResetHarness();
    queue_results[0] = 0u;
    assert(PrepareReserveTrackRawReadPipeline() == 0u);
    CheckInitialReset();
    assert(queue_calls == 1u && opc_calls == 0u);
    assert(memory_image[0x128bu] == 0u);
}

static void TestHighSpeedFirstSuccessLeavesPreparationActive(void)
{
    ResetHarness();
    memory_image[0x0e47u] = 12u;
    assert(PrepareReserveTrackRawReadPipeline() == 1u);
    CheckInitialReset();
    assert(queue_calls == 1u && opc_calls == 0u);
    assert(memory_image[0x0e47u] == 12u);
    assert(memory_image[0x128bu] == 1u);
    assert(plan_calls == 0u && poll_calls == 0u);
}

static void TestHighSpeedOpcAndRetryFailures(void)
{
    ResetHarness();
    memory_image[0x0e47u] = 16u;
    queue_results[0] = 0u;
    opc_result = 0u;
    assert(PrepareReserveTrackRawReadPipeline() == 0u);
    assert(queue_calls == 1u && opc_calls == 1u);
    assert(memory_image[0x0e47u] == 4u);
    assert(sense_calls == 1u && sense_key == 3u && sense_code == 0x7304u);

    ResetHarness();
    memory_image[0x0e47u] = 16u;
    queue_results[0] = queue_results[1] = 0u;
    assert(PrepareReserveTrackRawReadPipeline() == 0u);
    assert(queue_calls == 2u && opc_calls == 1u && sense_calls == 0u);
    assert(memory_image[0x0e47u] == 4u);
}

static void TestHighSpeedOptionBypassRestoresSelectedSpeed(void)
{
    ResetHarness();
    memory_image[0x0e47u] = 16u;
    memory_image[0x0c46u] = 1u;
    queue_results[0] = 0u;
    assert(PrepareReserveTrackRawReadPipeline() == 1u);
    assert(queue_calls == 2u && opc_calls == 0u);
    assert(memory_image[0x0e47u] == 24u);
    assert(memory_image[0x128bu] == 1u);

    ResetHarness();
    memory_image[0x0e47u] = 16u;
    memory_image[0x0c46u] = 1u;
    queue_results[0] = 0u;
    mutate_speed_on_first_queue = 8u;
    assert(PrepareReserveTrackRawReadPipeline() == 1u);
    assert(queue_calls == 2u && memory_image[0x0e47u] == 24u);
}

static void TestWriteTypeZeroSuccess(void)
{
    ResetHarness();
    memory_image[0x0c44u] = 0u;
    msf_result = -150l;
    assert(PrepareReserveTrackRawReadPipeline() == 1u);
    assert(bcd_calls == 1u && msf_calls == 1u);
    assert(ReadU32(0x1598u) == (u32)-150l);
    assert(memory_image[0x1281u] == 2u && memory_image[0x1282u] == 0u);
    assert(memory_image[0x1283u] == 0u && memory_image[0x1285u] == 0x41u);
    assert(memory_image[0x127cu] == 1u);
    assert(plan_calls == 1u && poll_calls == 1u);
    assert(memory_image[0x128bu] == 0u);
    assert(fill_calls == 1u);
}

static void TestFreshWriteTypeAndPlanFailures(void)
{
    ResetHarness();
    memory_image[0x0c44u] = 0u;
    mutate_write_type_on_msf = 2u;
    assert(PrepareReserveTrackRawReadPipeline() == 0u);
    assert(sequence_error_calls == 1u && plan_calls == 0u);

    ResetHarness();
    memory_image[0x0c44u] = 0u;
    plan_result = 0u;
    assert(PrepareReserveTrackRawReadPipeline() == 0u);
    assert(plan_calls == 1u && poll_calls == 0u);
    assert(memory_image[0x128bu] == 0u);
}

static void TestPollFailureClassification(void)
{
    ResetHarness();
    memory_image[0x0c44u] = 0u;
    poll_result = 0u;
    WriteU16(0x1292u, 0u);
    memory_image[0xfba0u] = 0u;
    assert(PrepareReserveTrackRawReadPipeline() == 0u);
    assert(queue_sense_calls == 1u && failure_calls == 0u);

    ResetHarness();
    memory_image[0x0c44u] = 0u;
    poll_result = 0u;
    memory_image[0xfba0u] = 1u;
    assert(PrepareReserveTrackRawReadPipeline() == 0u);
    assert(queue_sense_calls == 0u && failure_calls == 1u && failure_code == 6u);
}

int main(void)
{
    TestLowSpeedQueueFailure();
    TestHighSpeedFirstSuccessLeavesPreparationActive();
    TestHighSpeedOpcAndRetryFailures();
    TestHighSpeedOptionBypassRestoresSelectedSpeed();
    TestWriteTypeZeroSuccess();
    TestFreshWriteTypeAndPlanFailures();
    TestPollFailureClassification();
    puts("reserve-track raw-read pipeline tests passed");
    return 0;
}
