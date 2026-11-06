#include <assert.h>
#include <string.h>
#include "media/active_write_pipeline.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u8 bit35;
static u8 bit32;
static u8 acquire_tail_result;
static u8 acquire_padding_result;
static u8 prepare_prefix_result;
static u8 execute_range_result;
static u8 run_opc_result;
static u8 raw_q_results[4];
static u8 raw_q_result_count;
static u8 raw_q_result_index;
static u8 clear_bit35_count;
static u8 clear_bit30_count;
static u8 yield_count;
static u8 restart_count;
static u8 acquire_tail_count;
static s32 acquire_tail_argument;
static u8 failure_count;
static u8 padding_count;
static u8 prefix_count;
static u8 prefix_omit;
static u8 prefix_wait;
static u8 execute_count;
static u32 execute_argument;
static u8 readback_count;
static u8 clear_controller_count;
static u8 build_adr1_count;
static u8 raw_q_count;
static u8 opc_count;
static u8 sense_count;
static u8 sense_key;
static u16 sense_code;
static u8 advance_packet_count;
static u8 finalize_type1_count;
static u8 finalize_type2_count;
static u8 initialize_coroutine_count;
static u32 coroutine_entry;
static u8 finalize_opcode_count;
static u8 finalized_opcode;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveActiveWriteReadBit35(void)
{
    return bit35;
}

void ObserveActiveWriteClearBit35(void)
{
    clear_bit35_count++;
    bit35 = 0u;
}

void ObserveActiveWriteClearBit30(void)
{
    clear_bit30_count++;
}

u8 ObserveActiveWriteReadBit32(void)
{
    return bit32;
}

void ObserveActiveWriteYield(void)
{
    yield_count++;
    memory[0x00fb9au]--;
}

void ObserveActiveWriteRestartProducer(void)
{
    restart_count++;
}

u8 ObserveActiveWriteAcquireTail(s32 frame_count)
{
    acquire_tail_count++;
    acquire_tail_argument = frame_count;
    return acquire_tail_result;
}

void ObserveActiveWriteFailure(void)
{
    failure_count++;
}

u8 ObserveActiveWriteAcquirePadding(void)
{
    padding_count++;
    return acquire_padding_result;
}

u8 ObserveActiveWritePreparePrefix(u8 omit_final_frame, u8 wait_for_completion)
{
    prefix_count++;
    prefix_omit = omit_final_frame;
    prefix_wait = wait_for_completion;
    return prepare_prefix_result;
}

u8 ObserveActiveWriteExecuteRange(u32 block_count)
{
    execute_count++;
    execute_argument = block_count;
    return execute_range_result;
}

void ObserveActiveWriteVerifyReadback(void)
{
    readback_count++;
}

void ObserveActiveWriteClearControllerWords(void)
{
    clear_controller_count++;
}

void ObserveActiveWriteBuildAdr1(void)
{
    build_adr1_count++;
}

u8 ObserveActiveWriteBuildRawQ(void)
{
    raw_q_count++;
    assert(raw_q_result_index < raw_q_result_count);
    return raw_q_results[raw_q_result_index++];
}

u8 ObserveActiveWriteRunOpc(void)
{
    opc_count++;
    return run_opc_result;
}

void ObserveActiveWriteSetSense(u8 key, u16 code)
{
    sense_count++;
    sense_key = key;
    sense_code = code;
}

void ObserveActiveWriteAdvancePacket(void)
{
    advance_packet_count++;
}

void ObserveActiveWriteFinalizeType1(void)
{
    finalize_type1_count++;
}

void ObserveActiveWriteFinalizeType2(void)
{
    finalize_type2_count++;
}

void ObserveActiveWriteInitializeCoroutine1(u32 entry_point)
{
    initialize_coroutine_count++;
    coroutine_entry = entry_point;
}

void ObserveActiveWriteFinalizeOpcode(u8 opcode)
{
    finalize_opcode_count++;
    finalized_opcode = opcode;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void SetRawQResults(u8 first, u8 second, u8 count)
{
    raw_q_results[0] = first;
    raw_q_results[1] = second;
    raw_q_result_count = count;
    raw_q_result_index = 0u;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    bit35 = 0u;
    bit32 = 0u;
    acquire_tail_result = 1u;
    acquire_padding_result = 1u;
    prepare_prefix_result = 1u;
    execute_range_result = 1u;
    run_opc_result = 1u;
    memset(raw_q_results, 0, sizeof(raw_q_results));
    raw_q_result_count = 0u;
    raw_q_result_index = 0u;
    clear_bit35_count = 0u;
    clear_bit30_count = 0u;
    yield_count = 0u;
    restart_count = 0u;
    acquire_tail_count = 0u;
    acquire_tail_argument = 0;
    failure_count = 0u;
    padding_count = 0u;
    prefix_count = 0u;
    prefix_omit = 0xffu;
    prefix_wait = 0xffu;
    execute_count = 0u;
    execute_argument = 0ul;
    readback_count = 0u;
    clear_controller_count = 0u;
    build_adr1_count = 0u;
    raw_q_count = 0u;
    opc_count = 0u;
    sense_count = 0u;
    sense_key = 0u;
    sense_code = 0u;
    advance_packet_count = 0u;
    finalize_type1_count = 0u;
    finalize_type2_count = 0u;
    initialize_coroutine_count = 0u;
    coroutine_entry = 0ul;
    finalize_opcode_count = 0u;
    finalized_opcode = 0u;
    memory[0x000c44u] = 0xffu;
    memory[0x000c46u] = 1u;
    memory[0x000c9eu] = 1u;
    WriteU32(0x000b06u, 0x12345678ul);
    WriteU32(0x001455u, 0xfffffffful);
}

static void TestDrainAndMissingTailFailure(void)
{
    ResetState();
    bit35 = 1u;
    bit32 = 1u;
    memory[0x00fb9au] = 2u;
    memory[0x000c9eu] = 0u;
    memory[0x000c44u] = 1u;
    memory[0x00dd61u] = 2u;
    WriteU32(0x00dd4du, 100ul);
    WriteU32(0x00dd41u, 105ul);
    acquire_tail_result = 0u;

    assert(FinalizeActiveWritePipelineAndTrackState() == 0u);
    assert(ReadU32(0x001455u) == 0ul);
    assert(clear_bit35_count == 1u && clear_bit30_count == 1u);
    assert(yield_count == 2u && restart_count == 1u);
    assert(acquire_tail_count == 1u && acquire_tail_argument == 3l);
    assert(failure_count == 1u);
}

static void TestPacketPrefixAndPaddingFailures(void)
{
    ResetState();
    memory[0x000c9eu] = 0u;
    memory[0x000c44u] = 0u;
    memory[0x000c46u] = 2u;
    prepare_prefix_result = 0u;

    assert(FinalizeActiveWritePipelineAndTrackState() == 0u);
    assert(memory[0x001554u] == 0u && memory[0x0014b5u] == 1u);
    assert(prefix_count == 1u && prefix_omit == 0u && prefix_wait == 1u);
    assert(failure_count == 1u);

    ResetState();
    memory[0x000c9eu] = 0u;
    memory[0x000c44u] = 0u;
    memory[0x000c46u] = 2u;
    WriteU32(0x0014b8u, 1ul);
    acquire_padding_result = 0u;

    assert(FinalizeActiveWritePipelineAndTrackState() == 0u);
    assert(padding_count == 1u && prefix_count == 0u);
    assert(failure_count == 0u);
}

static void TestExecuteRangeAndReadback(void)
{
    ResetState();
    memory[0x000c44u] = 3u;
    memory[0x000c8au] = 0xa0u;

    assert(FinalizeActiveWritePipelineAndTrackState() == 1u);
    assert(execute_count == 1u && execute_argument == 0x12345678ul);
    assert(readback_count == 1u && memory[0x000c8au] == 0xc0u);
    assert(initialize_coroutine_count == 0u);

    ResetState();
    execute_range_result = 0u;
    assert(FinalizeActiveWritePipelineAndTrackState() == 0u);
    assert(readback_count == 0u);
}

static void TestType1LowSpeedRawQAndFinalizer(void)
{
    ResetState();
    memory[0x000c44u] = 1u;
    memory[0x00dd61u] = 0u;
    memory[0x00e74cu] = 1u;
    memory[0x000e47u] = 9u;
    WriteU32(0x00dd4du, 100ul);
    SetRawQResults(1u, 0u, 1u);

    assert(FinalizeActiveWritePipelineAndTrackState() == 1u);
    assert(ReadU32(0x00dd41u) == 102ul);
    assert(clear_controller_count == 2u && build_adr1_count == 1u);
    assert(raw_q_count == 1u && opc_count == 0u);
    assert(finalize_type1_count == 1u);
}

static void TestHighSpeedOpcFallback(void)
{
    ResetState();
    memory[0x000c44u] = 2u;
    memory[0x000c46u] = 0u;
    memory[0x000e47u] = 10u;
    memory[0x000c38u] = 24u;
    SetRawQResults(0u, 0u, 1u);
    run_opc_result = 0u;

    assert(FinalizeActiveWritePipelineAndTrackState() == 0u);
    assert(raw_q_count == 1u && opc_count == 1u);
    assert(sense_count == 1u && sense_key == 3u && sense_code == 0x7304u);
    assert(memory[0x000e47u] == 24u && finalize_type2_count == 0u);

    ResetState();
    memory[0x000c44u] = 2u;
    memory[0x000c46u] = 1u;
    memory[0x000e47u] = 10u;
    memory[0x000c38u] = 24u;
    SetRawQResults(0u, 1u, 2u);

    assert(FinalizeActiveWritePipelineAndTrackState() == 1u);
    assert(raw_q_count == 2u && opc_count == 0u);
    assert(memory[0x000e47u] == 24u && finalize_type2_count == 1u);
}

static void TestPacketProgressAndOpcodeFinalization(void)
{
    ResetState();
    memory[0x000c9eu] = 0u;
    memory[0x000c44u] = 0u;
    WriteU32(0x00dd4du, 100ul);
    WriteU32(0x00dd5bu, 9ul);

    assert(FinalizeActiveWritePipelineAndTrackState() == 1u);
    assert(ReadU32(0x00dd4du) == 102ul && ReadU32(0x00dd45u) == 102ul);
    assert(ReadU32(0x00dd5bu) == 0ul);
    assert(finalize_opcode_count == 1u && finalized_opcode == 0x35u);

    ResetState();
    memory[0x000c44u] = 0u;
    memory[0x00dd56u] = 1u;
    WriteU32(0x00dd5bu, 7ul);

    assert(FinalizeActiveWritePipelineAndTrackState() == 1u);
    assert(advance_packet_count == 1u && ReadU32(0x00dd5bu) == 0ul);
    assert(finalize_opcode_count == 0u);
}

static void TestType3CoroutineRestart(void)
{
    ResetState();
    memory[0x000c44u] = 3u;
    memory[0x000c46u] = 0u;

    assert(FinalizeActiveWritePipelineAndTrackState() == 1u);
    assert(initialize_coroutine_count == 1u);
    assert(coroutine_entry == 0x00f84c30ul);
}

int main(void)
{
    TestDrainAndMissingTailFailure();
    TestPacketPrefixAndPaddingFailures();
    TestExecuteRangeAndReadback();
    TestType1LowSpeedRawQAndFinalizer();
    TestHighSpeedOpcFallback();
    TestPacketProgressAndOpcodeFinalization();
    TestType3CoroutineRestart();
    return 0;
}
