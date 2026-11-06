#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/scaled_radial_pickup_move_sequence.h"

typedef struct
{
    fw_addr_t address;
    u8 value;
} PortWrite;

typedef struct
{
    u8 direction;
    u16 magnitude;
} MoveCall;

static u8 memory_image[0x20000];
static PortWrite port_writes[80];
static MoveCall move_calls[8];
static u16 waits[80];
static u16 port_write_count, move_call_count, wait_count;
static u16 configure_count, output_count, restore_count, divide_count;
static u8 configured_direction, output_direction, output_displacement;
static u8 output_be, output_bf;

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static void ResetHarness(void)
{
    u16 index;

    memset(memory_image, 0, sizeof(memory_image));
    memset(port_writes, 0, sizeof(port_writes));
    memset(move_calls, 0, sizeof(move_calls));
    memset(waits, 0, sizeof(waits));
    port_write_count = move_call_count = wait_count = 0u;
    configure_count = output_count = restore_count = divide_count = 0u;
    configured_direction = output_direction = 0u;
    output_displacement = 0u;
    output_be = output_bf = 0xffu;
    memory_image[0xe43u] = 9u;
    WriteWord(0x10c8u, 480u);
    WriteDword(0x10c4u, 15ul);
    for(index = 0u; index != 110u; ++index)
        memory_image[0x10cau + index] = (u8)(index + 1u);
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

s16 DivideS16Quotient(s16 dividend, s16 divisor)
{
    ++divide_count;
    assert(divisor != 0u);
    return (s16)(dividend / (s16)divisor);
}

void ObserveScaledRadialMoveConfigure(u8 inward_direction)
{
    ++configure_count;
    configured_direction = inward_direction;
}

u8 ObserveScaledRadialMoveConfigureOutputs(u8 inward_direction)
{
    ++output_count;
    output_direction = inward_direction;
    memory_image[0x10beu] = output_be;
    memory_image[0x10bfu] = output_bf;
    return output_displacement;
}

void ObserveScaledRadialMoveExecute(u8 inward_direction, u16 scaled_magnitude)
{
    assert(move_call_count < sizeof(move_calls) / sizeof(move_calls[0]));
    move_calls[move_call_count].direction = inward_direction;
    move_calls[move_call_count].magnitude = scaled_magnitude;
    ++move_call_count;
}

void ObserveScaledRadialMovePortWrite(fw_addr_t address, u8 value)
{
    assert(port_write_count < sizeof(port_writes) / sizeof(port_writes[0]));
    port_writes[port_write_count].address = address;
    port_writes[port_write_count].value = value;
    ++port_write_count;
}

void ObserveScaledRadialMoveWait(u16 duration, u8 yield_while_waiting)
{
    assert(wait_count < sizeof(waits) / sizeof(waits[0]));
    assert(yield_while_waiting == 0u);
    waits[wait_count++] = duration;
}

void ObserveScaledRadialMoveRestore(void)
{
    ++restore_count;
}

static void AssertPortWrite(u16 index, fw_addr_t address, u8 value)
{
    assert(index < port_write_count);
    assert(port_writes[index].address == address);
    assert(port_writes[index].value == value);
}

static void TestEveryShortPathGate(void)
{
    u8 gate;

    for(gate = 0u; gate != 4u; ++gate)
    {
        ResetHarness();
        if(gate == 1u)
            memory_image[0xfaeu] = 3u;
        else if(gate == 2u)
            memory_image[0xfafu] = 5u;
        else if(gate == 3u)
            memory_image[0xe43u] = 8u;

        ExecuteScaledRadialPickupMoveSequence(2u, gate == 0u ? 95u : 96u);
        assert(memory_image[0x1130u] == 0u);
        assert(memory_image[0x1131u] == 1u);
        assert(configure_count == 1u && configured_direction == 2u);
        assert(output_count == 0u && divide_count == 0u);
        assert(move_call_count == 1u);
        assert(move_calls[0].direction == 2u);
        assert(move_calls[0].magnitude == (gate == 0u ? 95u : 96u));
        assert(port_write_count == 1u);
        AssertPortWrite(0u, 0x1849dul, 0u);
        assert(restore_count == 1u);
    }
}

static void TestShortRangeMirroredProfile(void)
{
    static const u16 expected_waits[] = { 1u, 1u, 2u, 2u, 98u, 98u, 99u, 99u, 100u };
    u16 index;

    ResetHarness();
    ExecuteScaledRadialPickupMoveSequence(0u, 160u);

    assert(memory_image[0x1130u] == 1u);
    assert(memory_image[0x1131u] == 0u);
    assert(output_count == 1u && output_direction == 0u);
    assert(configure_count == 1u && divide_count == 8u);
    assert(move_call_count == 1u);
    assert(move_calls[0].direction == 0u && move_calls[0].magnitude == 16u);
    assert(wait_count == sizeof(expected_waits) / sizeof(expected_waits[0]));
    for(index = 0u; index != wait_count; ++index)
        assert(waits[index] == expected_waits[index]);

    assert(port_write_count == 16u);
    AssertPortWrite(0u, 0x1809aul, 0x80u);
    AssertPortWrite(1u, 0x18496ul, 0x80u);
    AssertPortWrite(2u, 0x18497ul, 0x80u);
    AssertPortWrite(3u, 0x18494ul, 0u);
    AssertPortWrite(4u, 0x18495ul, 0u);
    AssertPortWrite(5u, 0x18494ul, 0xffu);
    AssertPortWrite(6u, 0x18495ul, 0xffu);
    AssertPortWrite(7u, 0x18494ul, 0u);
    AssertPortWrite(8u, 0x18495ul, 0u);
    AssertPortWrite(9u, 0x18494ul, 0xffu);
    AssertPortWrite(10u, 0x18495ul, 0xffu);
    AssertPortWrite(11u, 0x18494ul, 0x80u);
    AssertPortWrite(12u, 0x1849dul, 0u);
    AssertPortWrite(13u, 0x1849ful, 0xc0u);
    AssertPortWrite(14u, 0x1849dul, 0x80u);
    AssertPortWrite(15u, 0x1849dul, 0u);
    assert(memory_image[0x10beu] == 0x80u);
    assert(memory_image[0x10bfu] == 0xffu);
    assert(restore_count == 1u);
}

static void TestLongRangeRampPlateauTailAndLeadingMove(void)
{
    static const u16 expected_waits[] = { 1u, 1u, 2u, 2u, 4u, 4u, 4u, 4u, 4u, 4u, 98u, 98u, 99u, 99u, 100u };
    u16 index;

    ResetHarness();
    WriteWord(0x10c8u, 10u);
    WriteDword(0x10c4u, 2ul);
    output_displacement = 8u;
    output_bf = 0u;

    ExecuteScaledRadialPickupMoveSequence(2u, 264u);

    assert(output_count == 1u && output_direction == 2u);
    assert(configure_count == 1u && configured_direction == 2u);
    assert(divide_count == 2u);
    assert(move_call_count == 2u);
    assert(move_calls[0].direction == 2u && move_calls[0].magnitude == 8u);
    assert(move_calls[1].direction == 2u && move_calls[1].magnitude == 16u);
    assert(wait_count == sizeof(expected_waits) / sizeof(expected_waits[0]));
    for(index = 0u; index != wait_count; ++index)
        assert(waits[index] == expected_waits[index]);

    assert(port_write_count == 22u);
    AssertPortWrite(17u, 0x18494ul, 0x80u);
    AssertPortWrite(18u, 0x1849dul, 0u);
    AssertPortWrite(19u, 0x1849ful, 0xc0u);
    AssertPortWrite(20u, 0x1849dul, 0x80u);
    AssertPortWrite(21u, 0x1849dul, 0u);
    assert(memory_image[0x10beu] == 0x80u);
    assert(memory_image[0x10bfu] == 0xffu);
    assert(memory_image[0x1131u] == 0u);
    assert(restore_count == 1u);
}

int main(void)
{
    TestEveryShortPathGate();
    TestShortRangeMirroredProfile();
    TestLongRangeRampPlateauTailAndLeadingMove();
    puts("scaled radial pickup move sequence tests passed");
    return 0;
}
