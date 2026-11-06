#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/format_unit_code11_scan.h"

static u8 memory[0x1000000ul];
static u8 bit_state[0x100];
static u8 poll_results[16];
static u32 poll_total_overrides[16];
static u8 poll_has_total_override[16];
static s32 map_results[16];
static u32 reset_transfer_results[16];
static s32 reset_targets[16];
static u8 reset_modes[16];
static s32 updated_positions[64];
static u8 updated_counts[64];
static unsigned finalize_count;
static unsigned initialize_read_verify_count;
static unsigned probe_count;
static u8 probe_result;
static unsigned map_count;
static unsigned map_result_count;
static unsigned reset_transfer_count;
static unsigned reset_transfer_result_count;
static unsigned position_count;
static s32 last_position_target;
static unsigned reset_queue_count;
static unsigned initialize_coroutine_count;
static unsigned yield_count;
static unsigned receive_count;
static unsigned poll_count;
static unsigned poll_result_count;
static unsigned reset_target_count;
static unsigned update_count;
static unsigned append_count;
static unsigned service_host_count;
static unsigned record_scan_count;
static u16 record_scan_result;
static u8 bit000a;

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

void ObserveFormat11ScanFinalizeWrite(void)
{
    ++finalize_count;
}

void ObserveFormat11ScanInitializeReadVerify(void)
{
    ++initialize_read_verify_count;
}

u8 ObserveFormat11ScanProbe(void)
{
    ++probe_count;
    return probe_result;
}

s32 ObserveFormat11ScanMapLogical(s32 logical_position)
{
    (void)logical_position;
    assert(map_count < map_result_count);
    return map_results[map_count++];
}

u32 ObserveFormat11ScanResetTransfer(void)
{
    assert(reset_transfer_count < reset_transfer_result_count);
    return reset_transfer_results[reset_transfer_count++];
}

void ObserveFormat11ScanPositionTransfer(s32 frame_position)
{
    ++position_count;
    last_position_target = frame_position;
}

void ObserveFormat11ScanResetQueue(void)
{
    ++reset_queue_count;
}

void ObserveFormat11ScanInitializeCoroutine2(u32 entry_point)
{
    assert(entry_point == 0x00f90000ul);
    ++initialize_coroutine_count;
}

void ObserveFormat11ScanYield(void)
{
    ++yield_count;
    memory[0x000e40u] = 0u;
}

void ObserveFormat11ScanReceiveHostPacket(void)
{
    ++receive_count;
}

u8 ObserveFormat11ScanPoll(void)
{
    unsigned index = poll_count++;

    assert(index < poll_result_count);
    if(poll_has_total_override[index] != 0u)
        WriteU32(0x2bc6fbu, poll_total_overrides[index]);
    return poll_results[index];
}

void ObserveFormat11ScanResetForTarget(s32 target_position, u8 mode)
{
    assert(reset_target_count < 16u);
    reset_targets[reset_target_count] = target_position;
    reset_modes[reset_target_count] = mode;
    ++reset_target_count;
}

void ObserveFormat11ScanUpdateRecordPage(s32 frame_position, u8 count)
{
    assert(update_count < 64u);
    updated_positions[update_count] = frame_position;
    updated_counts[update_count] = count;
    ++update_count;
}

u8 ObserveFormat11ScanAppendTargets(u8 requested_count, u8 initial_output_index)
{
    u8 processed;
    u8 output_index = initial_output_index;

    ++append_count;
    for(processed = 0u; processed < requested_count; ++processed)
    {
        u32 position = ReadU32(0x0015dcu) + 1ul;

        WriteU32(0x0015dcu, position);
        WriteU32(0x00fa2bu + (u16)((u16)output_index * 4u), position);
        ++output_index;
        WriteU32(0x2bc6fbu, ReadU32(0x2bc6fbu) + 1ul);
    }
    WriteU32(0x000b06u, 1ul);
    memory[0x001037u] = 0u;
    return output_index;
}

u32 ObserveFormat11ScanMapFixedPacket(u32 logical_block_address)
{
    assert(logical_block_address != 0xfffffffful);
    return 10ul;
}

void ObserveFormat11ScanServiceHost(void)
{
    ++service_host_count;
}

u16 ObserveFormat11ScanRecordBanks(u8 force_bank_scan, s32 start_frame_position, u32 frame_count)
{
    assert(force_bank_scan == 1u);
    assert(start_frame_position == (s32)ReadU32(0x001153u));
    assert(frame_count == 0ul);
    ++record_scan_count;
    return record_scan_result;
}

u8 ObserveFormat11ScanReadBit000A(void)
{
    return bit000a;
}

void ObserveFormat11ScanWriteBit(u8 bit_address, u8 value)
{
    bit_state[bit_address] = value;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(bit_state, 0xff, sizeof(bit_state));
    memset(poll_results, 0, sizeof(poll_results));
    memset(poll_total_overrides, 0, sizeof(poll_total_overrides));
    memset(poll_has_total_override, 0, sizeof(poll_has_total_override));
    memset(map_results, 0, sizeof(map_results));
    memset(reset_transfer_results, 0, sizeof(reset_transfer_results));
    memset(reset_targets, 0, sizeof(reset_targets));
    memset(reset_modes, 0, sizeof(reset_modes));
    memset(updated_positions, 0, sizeof(updated_positions));
    memset(updated_counts, 0, sizeof(updated_counts));
    finalize_count = initialize_read_verify_count = probe_count = 0u;
    map_count = map_result_count = 0u;
    reset_transfer_count = reset_transfer_result_count = 0u;
    position_count = reset_queue_count = initialize_coroutine_count = 0u;
    yield_count = receive_count = poll_count = poll_result_count = 0u;
    reset_target_count = update_count = append_count = 0u;
    service_host_count = record_scan_count = 0u;
    probe_result = bit000a = 0u;
    record_scan_result = 0u;
    last_position_target = 0;
    WriteU32(0x2ba52eu, 0x101ul);
    WriteU32(0x000fb4u, 0x12345678ul);
    WriteU32(0x00faffu, 0x00020000ul);
    WriteU32(0x001153u, 0x00034567ul);
    WriteU32(0x2ba59fu, 0x00000123ul);
}

static void TestProbeFailureRestoresTransferState(void)
{
    unsigned index;

    ResetFixture();
    memory[0x000c84u] = 0xabu;
    memory[0x000c85u] = 0xffu;
    memory[0x000c88u] = 0x80u;
    memory[0x000c8au] = 0xffu;
    memory[0x000c8bu] = 0x08u;
    memory[0x000c9eu] = 3u;
    RunFormatUnitCode11MediaScan();
    assert(finalize_count == 1u && initialize_read_verify_count == 1u);
    assert(probe_count == 1u && service_host_count == 1u);
    assert(poll_count == 0u && record_scan_count == 0u);
    assert(memory[0x000c84u] == 0xa3u && memory[0x000c85u] == 0xfeu);
    assert(memory[0x000c88u] == 0x80u);
    assert((memory[0x000c8bu] & 0x48u) == 0x40u);
    assert(memory[0x000c8au] == 0xfcu && memory[0x2ba52du] == 0xd0u);
    for(index = 0u; index < 0x20u; ++index)
        assert(ReadU32(0x00fa2bu + index * 4u) == 0ul);
    assert(bit_state[0x1bu] == 0u && bit_state[0x1du] == 0u && bit_state[0x1cu] == 0u && bit_state[0x13u] == 0u && bit_state[0x0du] == 0u && bit_state[0x0eu] == 0u && bit_state[0x18u] == 0u);
}

static void TestStatusOneNormalCompletion(void)
{
    ResetFixture();
    memory[0x000c8bu] = 4u;
    map_results[map_result_count++] = 0x91;
    reset_transfer_results[reset_transfer_result_count++] = 0x11223344ul;
    poll_results[poll_result_count++] = 1u;
    RunFormatUnitCode11MediaScan();
    assert(probe_count == 0u && poll_count == 1u);
    assert(reset_queue_count == 1u && initialize_coroutine_count == 1u);
    assert(ReadU32(0x001598u) == 0x11223344ul);
    assert(reset_target_count == 1u && reset_targets[0] == 0 && reset_modes[0] == 1u);
    assert(ReadU32(0x2bc6fbu) == 1ul);
    assert(ReadU32(0x2ba5a3u) == 0x123ul);
    assert((memory[0x000c8bu] & 5u) == 5u && (memory[0x000c89u] & 1u) == 0u);
    assert(record_scan_count == 1u && service_host_count == 0u);
    assert(ReadU32(0x000ce1u) == 0x34567ul && ReadU32(0x000ce5u) == 0x34567ul);
}

static void TestExceptionalStatusUpdatesResumeBoundary(void)
{
    ResetFixture();
    memory[0x000c2au] = 2u;
    memory[0x000c8bu] = 4u;
    WriteU32(0x0015dcu, 5ul);
    WriteU32(0x2ba5a3u, 100ul);
    reset_transfer_results[reset_transfer_result_count++] = 77ul;
    poll_results[poll_result_count++] = 0xffu;
    RunFormatUnitCode11MediaScan();
    assert(position_count == 1u && last_position_target == 77);
    assert(reset_queue_count == 0u && reset_target_count == 1u);
    assert(ReadU32(0x2ba5a3u) == 255ul);
    assert(ReadU32(0x2ba532u) == 346ul);
    assert(service_host_count == 1u && record_scan_count == 1u);
}

static void TestContiguousTargetsRefreshAtThirtyTwoBlocks(void)
{
    ResetFixture();
    memory[0x000c2au] = 2u;
    memory[0x000c8bu] = 4u;
    memory[0x000e40u] = 1u;
    WriteU32(0x2ba52eu, 0x120ul);
    map_results[map_result_count++] = 0x91 + 0x27 * 2;
    memory[0x00020002u] = 1u;
    reset_transfer_results[reset_transfer_result_count++] = 10ul;
    reset_transfer_results[reset_transfer_result_count++] = 11ul;
    poll_results[poll_result_count++] = 0u;
    poll_results[poll_result_count++] = 1u;
    poll_has_total_override[1] = 1u;
    poll_total_overrides[1] = 31ul;
    record_scan_result = 2u;
    WriteU32(0x2b9704u, 0x11111111ul);
    WriteU32(0x2b970cu, 0x22222222ul);
    RunFormatUnitCode11MediaScan();
    assert(poll_count == 2u && append_count == 1u);
    assert(position_count == 2u);
    assert(update_count == 3u);
    assert(updated_positions[0] == -1 && updated_counts[0] == 0u);
    assert(updated_positions[1] == 0 && updated_positions[2] == 1);
    assert(ReadU32(0x2bc6f7u) == 3ul);
    assert(ReadU32(0x00fa2bu) == 0ul && ReadU32(0x00fa2fu) == 0ul);
    assert(ReadU32(0x000ce1u) == 0x11111111ul && ReadU32(0x000ce5u) == 0x22222222ul);
}

int main(void)
{
    TestProbeFailureRestoresTransferState();
    TestStatusOneNormalCompletion();
    TestExceptionalStatusUpdatesResumeBoundary();
    TestContiguousTargetsRefreshAtThirtyTwoBlocks();
    puts("format unit code-11 scan tests passed");
    return 0;
}
