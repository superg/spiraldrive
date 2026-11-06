#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_eb.h"

static u8 memory[0x1000000ul];
static unsigned unsupported_calls;
static unsigned lowest_speed_calls;
static u8 lowest_speed;
static unsigned speed_write_calls;
static u8 speed_write_values[3];
static fw_addr_t speed_write_addresses[3];
static unsigned copy_calls;
static fw_addr_t copy_source;
static u32 copy_offset;
static u32 copy_count;
static unsigned data_in_calls;
static u32 data_in_length;
static u8 data_in_result;
static unsigned no_sense_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void HandleUnsupportedScsiCommand(void)
{
    ++unsupported_calls;
}

u8 SelectStateEligibleLowestWriteSpeed(void)
{
    ++lowest_speed_calls;
    return lowest_speed;
}

void WriteCdSpeedKilobytesPerSecond(u8 speed_x, fw_addr_t output_address)
{
    speed_write_values[speed_write_calls] = speed_x;
    speed_write_addresses[speed_write_calls++] = output_address;
    FirmwareWrite8(output_address, 0xa5u);
    FirmwareWrite8(output_address + 1u, speed_x);
}

u32 CopyToControllerBufferWindow(fw_addr_t source, u32 controller_offset, u32 byte_count)
{
    ++copy_calls;
    copy_source = source;
    copy_offset = controller_offset;
    copy_count = byte_count;
    return controller_offset + byte_count;
}

u8 ExecuteControllerDataInAndMapStatus(u32 transfer_length)
{
    ++data_in_calls;
    data_in_length = transfer_length;
    return data_in_result;
}

void SetNoSenseData(void)
{
    ++no_sense_calls;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(speed_write_values, 0, sizeof(speed_write_values));
    memset(speed_write_addresses, 0, sizeof(speed_write_addresses));
    unsupported_calls = lowest_speed_calls = speed_write_calls = 0u;
    copy_calls = data_in_calls = no_sense_calls = 0u;
    copy_source = copy_offset = copy_count = data_in_length = 0ul;
    lowest_speed = 6u;
    data_in_result = 4u;
    memory[0x000b13ul] = 10u;
    memory[0x00155aul] = 0x01u;
    memory[0x00155bul] = 0x23u;
    memory[0x00155cul] = 0x45u;
    memory[0x00155dul] = 0x67u;
}

static u16 ReadResponseWord(unsigned offset)
{
    return (u16)(((u16)memory[0x000b2aul + offset] << 8) | memory[0x000b2aul + offset + 1u]);
}

static void AssertTransfer(u32 expected_length)
{
    assert(copy_calls == 1u && copy_source == 0x000b2aul);
    assert(copy_offset == 0x01234567ul && copy_count == 10ul);
    assert(data_in_calls == 1u && data_in_length == expected_length);
}

static void TestRejectsReservedCdbBitsAfterAllocationRead(void)
{
    Reset();
    memory[0x000b0bul] = 1u;
    memory[0x000b2aul] = 0x5au;
    HandleScsiOpcodeEB();
    assert(unsupported_calls == 1u);
    assert(copy_calls == 0u && data_in_calls == 0u);
    assert(memory[0x000b2aul] == 0x5au);
}

static void TestRawCurrentSpeedResponse(void)
{
    Reset();
    memory[0x000b0cul] = 1u;
    memory[0x000c7aul] = 8u;
    memory[0x000c7bul] = 12u;
    memory[0x000c7cul] = 16u;
    HandleScsiOpcodeEB();
    assert(ReadResponseWord(0u) == 8u);
    assert(memory[0x000b2cul] == 1u && memory[0x000b2dul] == 0u);
    assert(ReadResponseWord(4u) == 12u);
    assert(ReadResponseWord(6u) == 8u);
    assert(ReadResponseWord(8u) == 16u);
    assert(speed_write_calls == 0u && lowest_speed_calls == 0u);
    AssertTransfer(10ul);
}

static void TestRawRequestedAndEligibleSpeedResponse(void)
{
    Reset();
    memory[0x000b0cul] = 0xf1u;
    memory[0x000b0dul] = 1u;
    memory[0x000c7cul] = 10u;
    memory[0x000c7dul] = 24u;
    lowest_speed = 4u;
    HandleScsiOpcodeEB();
    assert(memory[0x000b2cul] == 1u && memory[0x000b2dul] == 1u);
    assert(ReadResponseWord(4u) == 24u);
    assert(ReadResponseWord(6u) == 4u);
    assert(ReadResponseWord(8u) == 10u);
    assert(lowest_speed_calls == 1u);
}

static void TestFallbackSpeedSelection(void)
{
    Reset();
    memory[0x000b0cul] = 1u;
    memory[0x000b0dul] = 3u;
    memory[0x001008ul] = 1u;
    HandleScsiOpcodeEB();
    assert(ReadResponseWord(6u) == 4u);

    Reset();
    memory[0x000b0cul] = 1u;
    memory[0x000b0dul] = 3u;
    memory[0x001008ul] = 2u;
    memory[0x00fb88ul] = 3u;
    HandleScsiOpcodeEB();
    assert(ReadResponseWord(6u) == 0x18u);

    Reset();
    memory[0x000b0cul] = 1u;
    memory[0x000b0dul] = 3u;
    memory[0x001008ul] = 2u;
    memory[0x00fb88ul] = 1u;
    memory[0x00fb80ul] = 1u;
    HandleScsiOpcodeEB();
    assert(ReadResponseWord(6u) == 0u);

    Reset();
    memory[0x000b0cul] = 1u;
    memory[0x000b0dul] = 3u;
    memory[0x001008ul] = 2u;
    memory[0x00fb88ul] = 1u;
    memory[0x00fb80ul] = 2u;
    HandleScsiOpcodeEB();
    assert(ReadResponseWord(6u) == 4u);
}

static void TestKilobyteFormatAndShortTransfer(void)
{
    Reset();
    memory[0x000b0cul] = 2u;
    memory[0x000b0dul] = 1u;
    memory[0x000b13ul] = 6u;
    memory[0x000c7cul] = 16u;
    memory[0x000c7dul] = 24u;
    lowest_speed = 8u;
    HandleScsiOpcodeEB();
    assert(speed_write_calls == 3u);
    assert(speed_write_values[0] == 24u && speed_write_values[1] == 8u && speed_write_values[2] == 16u);
    assert(speed_write_addresses[0] == 0x000b2eul && speed_write_addresses[1] == 0x000b30ul && speed_write_addresses[2] == 0x000b32ul);
    assert(memory[0x000b2eul] == 0xa5u && memory[0x000b2ful] == 24u);
    AssertTransfer(6ul);
}

static void TestNonGoodTransferClearsSense(void)
{
    Reset();
    memory[0x000b0cul] = 1u;
    data_in_result = 5u;
    HandleScsiOpcodeEB();
    assert(no_sense_calls == 1u);
}

int main(void)
{
    TestRejectsReservedCdbBitsAfterAllocationRead();
    TestRawCurrentSpeedResponse();
    TestRawRequestedAndEligibleSpeedResponse();
    TestFallbackSpeedSelection();
    TestKilobyteFormatAndShortTransfer();
    TestNonGoodTransferClearsSense();
    return 0;
}
