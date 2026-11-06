#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/pickup_move_controller_sequence.h"

typedef struct
{
    fw_addr_t address;
    u8 value;
} PortWrite;

static u8 memory[1u << 24];
static u8 status_samples[16];
static unsigned status_count, status_index;
static PortWrite ports[32];
static unsigned port_count, yield_count, clear_d1_count, write_d2d3_count;
static unsigned write_d1d2d3_count, submit_count, configure_count;
static unsigned classify_count, fixed_one_count, fixed_both_count;
static unsigned profile_count, clear_flag_count, finalize_count;
static unsigned channel_count, apply_d1_count;
static u16 submit_values[4], submit_selectors[4];
static u16 configured_distance, profile_distance;
static u8 configured_direction, profile_direction;
static u8 classification_result, finalize_result, force_negative_timeout;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x0180f8ul && status_index < status_count)
        return status_samples[status_index++];
    if(address == 0x001213ul && force_negative_timeout != 0u)
        return 0x80u;
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void YieldCoroutine1(void)
{
    ++yield_count;
}
void ClearControllerParameterD1ShadowBit6(void)
{
    ++clear_d1_count;
}
void WriteControllerParametersD2AndD3FromState(void)
{
    ++write_d2d3_count;
}
void WriteControllerParametersD1D2D3FromConfiguration(void)
{
    ++write_d1d2d3_count;
}
void SubmitControllerWordPair(u16 value, u16 selector)
{
    submit_values[submit_count] = value;
    submit_selectors[submit_count++] = selector;
}
void WaitControllerPortReadyAndWriteByte(fw_addr_t address, u8 value)
{
    ports[port_count].address = address;
    ports[port_count++].value = value;
}
void ConfigurePickupMoveControllerForDistance(u16 distance, u8 direction)
{
    ++configure_count;
    configured_distance = distance;
    configured_direction = direction;
}
u8 DoesControllerModeOneClassificationDifferFromCache(u16 mode_word)
{
    ++classify_count;
    (void)mode_word;
    return classification_result;
}
void ApplyFixedControllerParameterTableForOperatingCodeClass(void)
{
    ++fixed_one_count;
}
void ApplyFixedControllerParameterTables1And2(void)
{
    ++fixed_both_count;
}
void ProgramPickupMoveProfileForDistance(u8 direction, u16 distance)
{
    ++profile_count;
    profile_direction = direction;
    profile_distance = distance;
}
void SetControllerFlagFC07Bit2AndPort8098Bit6(u8 enabled)
{
    assert(enabled == 0u);
    ++clear_flag_count;
}
u8 FinalizePickupMoveControllerSequenceAndClassifyStatus(void)
{
    ++finalize_count;
    return finalize_result;
}
void SetIndexedControllerChannelValue(u8 channel, u8 value)
{
    assert(channel == 2u);
    assert(value == memory[0x00f98eul]);
    ++channel_count;
}
void ApplyControllerParameterD1ModeBits(void)
{
    ++apply_d1_count;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(status_samples, 0, sizeof(status_samples));
    memset(ports, 0, sizeof(ports));
    memset(submit_values, 0, sizeof(submit_values));
    memset(submit_selectors, 0, sizeof(submit_selectors));
    status_count = status_index = port_count = yield_count = 0u;
    clear_d1_count = write_d2d3_count = write_d1d2d3_count = 0u;
    submit_count = configure_count = classify_count = 0u;
    fixed_one_count = fixed_both_count = profile_count = 0u;
    clear_flag_count = finalize_count = channel_count = apply_d1_count = 0u;
    configured_distance = profile_distance = 0u;
    configured_direction = profile_direction = 0u;
    classification_result = 0u;
    finalize_result = 1u;
    force_negative_timeout = 0u;
    memory[0x00f98eul] = 0x45u;
    memory[0x00f98ful] = 0x67u;
    WriteU16(0x000cf9ul, 24000u);
    WriteU16(0x001031ul, 100u);
}

static void AssertFixedPortSequence(void)
{
    static const fw_addr_t addresses[] = { 0x0180d8ul, 0x0180a4ul, 0x0180a6ul, 0x018498ul, 0x018499ul, 0x01809aul, 0x0184a2ul, 0x018496ul, 0x018497ul, 0x0180e4ul, 0x0180a5ul };
    static const u8 values[] = { 0x18u, 0x75u, 0x00u, 0x10u, 0xf0u, 0x80u, 0x80u, 0xc5u, 0x80u, 0x01u, 0x04u };
    unsigned index;

    for(index = 0u; index < sizeof(addresses) / sizeof(addresses[0]); ++index)
    {
        assert(ports[index].address == addresses[index]);
        assert(ports[index].value == values[index]);
    }
}

static void TestSuccessfulLongOutwardMove(void)
{
    ResetState();
    memory[0x001006ul] = 1u;
    memory[0x000e43ul] = 4u;
    memory[0x00fc2eul] = 1u;
    memory[0x00fc2ful] = 1u;
    status_samples[0] = 0u;
    status_samples[1] = 0u;
    status_count = 2u;

    assert(ExecutePickupMoveControllerSequence(12u, 0u) == 1u);
    assert(clear_d1_count == 1u && write_d2d3_count == 1u);
    assert(ReadU16(0x001031ul) == 124u);
    assert(configure_count == 1u && configured_distance == 12u);
    assert(configured_direction == 0u && classify_count == 1u);
    assert(fixed_one_count == 0u && fixed_both_count == 0u);
    assert(profile_count == 1u && profile_distance == 12u);
    assert(profile_direction == 0u);
    assert(port_count == 14u);
    assert(ports[2].address == 0x0180a5ul && ports[2].value == 3u);
    memmove(&ports[2], &ports[3], 11u * sizeof(ports[0]));
    AssertFixedPortSequence();
    assert(ports[11].address == 0x0180a6ul && ports[11].value == 0x67u);
    assert(ports[12].address == 0x0180d8ul && ports[12].value == 0u);
    assert(submit_count == 2u);
    assert(submit_values[0] == 0x3fffu && submit_selectors[0] == 0x033au);
    assert(submit_values[1] == 0u && submit_selectors[1] == 0x033au);
    assert(write_d1d2d3_count == 1u && finalize_count == 1u);
    assert(channel_count == 1u && apply_d1_count == 1u);
    assert(ReadU16(0x00fc2bul) == 12u);
    assert(memory[0x00fc2dul] == 0u && memory[0x00fc2eul] == 1u);
    assert(memory[0x00fc2ful] == 1u);
}

static void TestFailedUnitInwardMoveUsesSingleTable(void)
{
    ResetState();
    memory[0x000c9eul] = 2u;
    classification_result = 1u;
    finalize_result = 2u;
    status_count = 2u;

    assert(ExecutePickupMoveControllerSequence(1u, 1u) == 0u);
    assert(clear_d1_count == 1u);
    assert(ReadU16(0x001031ul) == 98u);
    assert(configured_distance == 1u && configured_direction == 1u);
    assert(fixed_one_count == 1u && fixed_both_count == 0u);
    assert(profile_count == 0u && channel_count == 0u);
    assert(apply_d1_count == 0u);
    assert(ports[port_count - 1u].address == 0x01849dul);
    assert(ports[port_count - 1u].value == 0u);
}

static void TestWaitsAndOrdinaryTableRefresh(void)
{
    ResetState();
    memory[0x001302ul] = 0u;
    memory[0x000e43ul] = 3u;
    memory[0x00fc2eul] = 1u;
    force_negative_timeout = 1u;
    status_samples[0] = 0x80u;
    status_samples[1] = 0u;
    status_samples[2] = 0x80u;
    status_samples[3] = 0u;
    status_count = 4u;

    assert(ExecutePickupMoveControllerSequence(5u, 0u) == 1u);
    assert(yield_count == 2u);
    assert(clear_flag_count == 1u);
    assert(fixed_one_count == 0u && fixed_both_count == 1u);
    assert(profile_count == 0u);
    assert(ReadU16(0x001031ul) == 110u);
    assert(memory[0x00fc2eul] == 0u);
}

int main(void)
{
    TestSuccessfulLongOutwardMove();
    TestFailedUnitInwardMoveUsesSingleTable();
    TestWaitsAndOrdinaryTableRefresh();
    return 0;
}
