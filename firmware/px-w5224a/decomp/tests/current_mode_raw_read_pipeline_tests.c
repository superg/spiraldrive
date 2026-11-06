#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/current_mode_raw_read_pipeline.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x20000];
static u8 bank_one_image[0x20000];
static u8 bit41, bit42, bit30;
static u8 init_control_calls, init_identification_calls, init_tracks_calls;
static u8 clear_status_calls, init_random_calls, update_q_calls;
static u8 isrc_result, isrc_calls, load_isrc_calls;
static u8 mcn_result, mcn_calls, load_mcn_calls;
static u8 plan_results[2], plan_calls, poll_result, poll_calls;
static u8 failure_calls, failure_code, sense_calls, layout_calls;
static u8 init_track_records_calls, lead_result, lead_calls;
static u8 advance_calls, refresh_calls, copy_calls, decrement_calls;
static u8 frame_result, frame_calls, select_calls, fill_calls;
static u32 resume_address, select_result, select_length, select_fallback;
static fw_addr_t fill_address;
static u32 fill_count;
static u8 fill_value;

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

static void ResetHarness(u8 mode)
{
    memset(memory_image, 0xa5, sizeof(memory_image));
    memset(bank_one_image, 0xa5, sizeof(bank_one_image));
    bit41 = bit42 = bit30 = 1u;
    init_control_calls = init_identification_calls = init_tracks_calls = 0u;
    clear_status_calls = init_random_calls = update_q_calls = 0u;
    isrc_result = mcn_result = 0u;
    isrc_calls = load_isrc_calls = mcn_calls = load_mcn_calls = 0u;
    plan_results[0] = plan_results[1] = 1u;
    plan_calls = poll_calls = failure_calls = sense_calls = layout_calls = 0u;
    poll_result = lead_result = frame_result = 1u;
    failure_code = 0xffu;
    init_track_records_calls = lead_calls = advance_calls = refresh_calls = 0u;
    copy_calls = decrement_calls = frame_calls = select_calls = fill_calls = 0u;
    resume_address = 0x00005500ul;
    select_result = 0x00006000ul;
    select_length = select_fallback = 0u;
    fill_address = fill_count = 0u;
    fill_value = 0xffu;
    memory_image[0x0c44u] = mode;
    memory_image[0x114eu] = 1u;
    memory_image[0x0c71u] = 0x11u;
    memory_image[0x0c72u] = 0x22u;
    memory_image[0x0c73u] = 0x33u;
    memory_image[0x0c74u] = 0x44u;
    memory_image[0x18050u] = 0u;
    memory_image[0x1428u] = 0x12u;
    memory_image[0x1429u] = 0x34u;
    memory_image[0x142au] = 0x56u;
    memory_image[0x142bu] = 0x78u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    if(address >= 0x100000ul && address < 0x120000ul)
        return bank_one_image[address - 0x100000ul];
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    if(address >= 0x100000ul && address < 0x120000ul)
    {
        bank_one_image[address - 0x100000ul] = value;
        return;
    }
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

void ObserveCurrentModeWriteBit41(u8 value)
{
    bit41 = value;
}
void ObserveCurrentModeWriteBit42(u8 value)
{
    bit42 = value;
}
void ObserveCurrentModeWriteBit30(u8 value)
{
    bit30 = value;
}
void ObserveCurrentModeInitializeSectorControl(void)
{
    ++init_control_calls;
}
void ObserveCurrentModeInitializeSubchannelIdentification(void)
{
    ++init_identification_calls;
}
void ObserveCurrentModeInitializeTrackSelection(void)
{
    ++init_tracks_calls;
}
void ObserveCurrentModeClearPipelineStatus(void)
{
    ++clear_status_calls;
}
void ObserveCurrentModeInitializeRandomBytes(void)
{
    ++init_random_calls;
}
void ObserveCurrentModeUpdateQTypeState(void)
{
    ++update_q_calls;
}
u8 ObserveCurrentModeIsIsrcAvailable(void)
{
    ++isrc_calls;
    return isrc_result;
}
void ObserveCurrentModeLoadIsrc(void)
{
    ++load_isrc_calls;
}
u8 ObserveCurrentModePreparePlan(u8 alternate)
{
    u8 index = plan_calls++;
    assert(alternate == 0u && index < 2u);
    return plan_results[index];
}
u8 ObserveCurrentModePoll(u8 count)
{
    ++poll_calls;
    assert(count == 1u);
    return poll_result;
}
void ObserveCurrentModeReportFailure(u8 code)
{
    ++failure_calls;
    failure_code = code;
}
void ObserveCurrentModeReportQueueSense(void)
{
    ++sense_calls;
}
void ObserveCurrentModeConfigureTransferLayout(void)
{
    ++layout_calls;
}
u8 ObserveCurrentModeIsMediaCatalogAvailable(void)
{
    ++mcn_calls;
    return mcn_result;
}
void ObserveCurrentModeLoadMediaCatalog(void)
{
    ++load_mcn_calls;
}
void ObserveCurrentModeInitializeTrackRecords(void)
{
    ++init_track_records_calls;
}
u8 ObserveCurrentModeAcquireLeadIn(void)
{
    ++lead_calls;
    return lead_result;
}
void ObserveCurrentModeAdvanceTrackRecord(void)
{
    ++advance_calls;
}
void ObserveCurrentModeRefreshIsrc(void)
{
    ++refresh_calls;
}
void ObserveCurrentModeCopy(fw_addr_t source, fw_addr_t destination, u16 count)
{
    u16 index;
    ++copy_calls;
    assert(source == 0x1428ul && destination == 0x7000ul && count == 4u);
    for(index = 0u; index < count; ++index)
        FirmwareWrite8(destination + index, FirmwareRead8(source + index));
}
void ObserveCurrentModeDecrementMsf(u16 address)
{
    ++decrement_calls;
    assert(address == 0x7000u);
    assert(memory_image[0x7000u] == 0x12u && memory_image[0x7001u] == 0x34u && memory_image[0x7002u] == 0x56u && memory_image[0x7003u] == 0x78u);
}
fw_addr_t ObserveCurrentModeResumeRing(void)
{
    return resume_address;
}
u8 ObserveCurrentModeAcquireFrames(u32 count, u8 reverse)
{
    ++frame_calls;
    assert(count == 1ul && reverse == 0u);
    return frame_result;
}
u32 ObserveCurrentModeSelectBuffer(u32 length, u32 fallback)
{
    ++select_calls;
    select_length = length;
    select_fallback = fallback;
    return select_result;
}
void ObserveCurrentModeFill(fw_addr_t address, u32 count, u8 value)
{
    ++fill_calls;
    fill_address = address;
    fill_count = count;
    fill_value = value;
}

static void CheckCommonState(u8 mode)
{
    assert(bit41 == 0u && bit42 == 1u);
    assert(memory_image[0x1284u] == 0u && memory_image[0x88b4u] == 0u);
    assert(memory_image[0x127du] == 0u && memory_image[0x127eu] == 0u);
    assert(memory_image[0x128cu] == 0u && memory_image[0x128du] == 0u);
    assert(memory_image[0x1283u] == mode && memory_image[0x1286u] == 0u);
    assert(ReadU16(0x800eu) == 0u && memory_image[0x1490u] == 0u);
    assert(ReadU32(0x1488u) == 0u && ReadU32(0x148cu) == 0u);
    assert(init_control_calls == 1u && init_identification_calls == 1u && init_tracks_calls == 1u && clear_status_calls == 1u);
    assert(memory_image[0x18021u] == 0u);
    assert(memory_image[0x18025u] == 0x11u && memory_image[0x18026u] == 0x22u && memory_image[0x18027u] == 0x33u && memory_image[0x18028u] == 0x44u);
    assert(memory_image[0x1590u] == 1u);
}

static void TestCommonAndUnknownMode(void)
{
    ResetHarness(4u);
    memory_image[0x114eu] = 0u;
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    CheckCommonState(4u);
    assert(init_random_calls == 1u && update_q_calls == 0u);

    ResetHarness(4u);
    memory_image[0x18050u] = 1u;
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(bit42 == 0u && init_random_calls == 0u);
}

static void TestModeZeroOrdinaryAndInitialFailure(void)
{
    ResetHarness(0u);
    memory_image[0xdd36u] = 0x20u;
    isrc_result = 1u;
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    CheckCommonState(0u);
    assert(update_q_calls == 1u && isrc_calls == 1u && load_isrc_calls == 1u);
    assert(plan_calls == 1u && layout_calls == 1u);
    assert(memory_image[0x1288u] == 1u && memory_image[0x1281u] == 2u);
    assert(memory_image[0x1289u] == 0u && memory_image[0x127fu] == 0u);

    ResetHarness(0u);
    plan_results[0] = 0u;
    assert(InitializeRawReadPipelineForCurrentMode() == 0u);
    assert(plan_calls == 1u && layout_calls == 0u);
}

static void PrepareModeZeroContinuation(void)
{
    memory_image[0x14b4u] = 1u;
    memory_image[0xdd34u] = 0u;
    memory_image[0x0c9eu] = 0u;
}

static void TestModeZeroContinuationRoutes(void)
{
    ResetHarness(0u);
    PrepareModeZeroContinuation();
    poll_result = 0u;
    memory_image[0x0c84u] = 0x40u;
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(bit30 == 0u && poll_calls == 1u && failure_calls == 1u && failure_code == 8u && plan_calls == 2u);
    assert(memory_image[0x14b4u] == 0u && memory_image[0x14b6u] == 0u && memory_image[0x14b7u] == 0u);

    ResetHarness(0u);
    PrepareModeZeroContinuation();
    poll_result = 0u;
    WriteU16(0x1292u, 0u);
    memory_image[0xfba0u] = 0u;
    assert(InitializeRawReadPipelineForCurrentMode() == 0u);
    assert(sense_calls == 1u && failure_calls == 0u && plan_calls == 1u);

    ResetHarness(0u);
    PrepareModeZeroContinuation();
    poll_result = 0u;
    WriteU16(0x1292u, 1u);
    assert(InitializeRawReadPipelineForCurrentMode() == 0u);
    assert(failure_calls == 1u && failure_code == 0u && sense_calls == 0u);

    ResetHarness(0u);
    PrepareModeZeroContinuation();
    plan_results[1] = 0u;
    assert(InitializeRawReadPipelineForCurrentMode() == 0u);
    assert(plan_calls == 2u && memory_image[0x14b6u] == 1u && memory_image[0x14b7u] == 1u && memory_image[0x14b4u] == 1u);

    ResetHarness(0u);
    memory_image[0x14b4u] = 1u;
    memory_image[0xdd34u] = 0u;
    memory_image[0x0c9eu] = 1u;
    memory_image[0x0c46u] = 2u;
    WriteU32(0x14b8u, 0x12345678ul);
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(poll_calls == 0u && plan_calls == 1u);
    assert(memory_image[0x14b2u] == 0u && ReadU32(0x14b8u) == 0u);
}

static void TestModeOne(void)
{
    ResetHarness(1u);
    mcn_result = isrc_result = 1u;
    memory_image[0xdd36u] = 0x20u;
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(update_q_calls == 1u && mcn_calls == 1u && load_mcn_calls == 1u);
    assert(isrc_calls == 1u && load_isrc_calls == 1u);
    assert(memory_image[0x146fu] == 0u && memory_image[0x1288u] == 1u);
    assert(plan_calls == 1u && layout_calls == 1u);
}

static void TestModeTwo(void)
{
    ResetHarness(2u);
    memory_image[0xb4bdu] = 0x20u;
    memory_image[0x1404u] = 0u;
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(init_track_records_calls == 1u && lead_calls == 1u);
    assert(memory_image[0x1404u] == 1u && memory_image[0x1405u] == 0u && memory_image[0x1403u] == 0u);
    assert(advance_calls == 1u && refresh_calls == 1u);
    assert(memory_image[0x1288u] == 1u && memory_image[0x1281u] == 1u);

    ResetHarness(2u);
    lead_result = 0u;
    memory_image[0x1404u] = 0u;
    assert(InitializeRawReadPipelineForCurrentMode() == 0u);
    assert(lead_calls == 1u && advance_calls == 0u && refresh_calls == 0u);

    ResetHarness(2u);
    memory_image[0x1404u] = 1u;
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(lead_calls == 0u && advance_calls == 1u && refresh_calls == 1u);
}

static void PrepareModeThree(u8 q_type)
{
    memory_image[0x0c48u] = q_type;
    WriteU32(0x1598u, 0u);
    memory_image[0x127fu] = 0x55u;
}

static void TestModeThreeLayouts(void)
{
    u8 index;

    ResetHarness(3u);
    PrepareModeThree(1u);
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(copy_calls == 1u && decrement_calls == 1u && frame_calls == 1u);
    assert(ReadU32(0x1598u) == 0xfffffffful);
    assert(select_calls == 1u && select_length == 0x10ul && select_fallback == resume_address);
    for(index = 0u; index < 16u; ++index)
        assert(bank_one_image[0x6000u + index] == 0u);
    assert(memory_image[0x127fu] == 1u && layout_calls == 1u);

    ResetHarness(3u);
    PrepareModeThree(2u);
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(select_calls == 1u && select_length == 0x60ul);
    assert(fill_calls == 1u && fill_address == select_result && fill_count == 0x60ul && fill_value == 0u);
    assert(memory_image[0x127fu] == 2u);

    ResetHarness(3u);
    PrepareModeThree(0u);
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(select_calls == 0u && fill_calls == 0u && memory_image[0x127fu] == 0u);

    ResetHarness(3u);
    PrepareModeThree(4u);
    assert(InitializeRawReadPipelineForCurrentMode() == 1u);
    assert(memory_image[0x127fu] == 0x55u && layout_calls == 1u);

    ResetHarness(3u);
    PrepareModeThree(3u);
    frame_result = 0u;
    assert(InitializeRawReadPipelineForCurrentMode() == 0u);
    assert(ReadU32(0x1598u) == 0xfffffffful);
    assert(select_calls == 0u && fill_calls == 0u && layout_calls == 0u);
}

int main(void)
{
    TestCommonAndUnknownMode();
    TestModeZeroOrdinaryAndInitialFailure();
    TestModeZeroContinuationRoutes();
    TestModeOne();
    TestModeTwo();
    TestModeThreeLayouts();
    puts("current-mode raw-read pipeline tests passed");
    return 0;
}
