#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_e9.h"

static u8 memory[0x1000000ul];
static unsigned unsupported_calls;
static unsigned transfer_calls;
static fw_addr_t transfer_source;
static u32 transfer_length;
static u8 transfer_result;
static unsigned no_sense_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u16 ReadBigEndianU16(fw_addr_t source_address)
{
    return (u16)(((u16)FirmwareRead8(source_address) << 8) | FirmwareRead8(source_address + 1u));
}

void HandleUnsupportedScsiCommand(void)
{
    ++unsupported_calls;
}

u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source_address, u32 byte_count)
{
    ++transfer_calls;
    transfer_source = source_address;
    transfer_length = byte_count;
    return transfer_result;
}

void SetNoSenseData(void)
{
    ++no_sense_calls;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    unsupported_calls = transfer_calls = no_sense_calls = 0u;
    transfer_source = transfer_length = 0u;
    transfer_result = 4u;
    memory[0x000b14ul] = 8u;
}

static void AssertCommonResponse(u8 selector, u8 flags)
{
    unsigned offset;

    assert(memory[0x000b2aul] == selector);
    assert(memory[0x000b2bul] == 6u);
    assert(memory[0x000b2cul] == flags);
    for(offset = 5u; offset < 8u; ++offset)
        assert(memory[0x000b2aul + offset] == 0u);
    assert(transfer_calls == 1u);
    assert(transfer_source == 0x000b2aul);
}

static void TestSelector1ReportsWithoutUpdating(void)
{
    Reset();
    memory[0x000b0cul] = 1u;
    memory[0x000b0dul] = 0u;
    memory[0x000c88ul] = 3u;
    HandleScsiOpcodeE9();
    AssertCommonResponse(1u, 3u);
    assert(memory[0x000c88ul] == 3u);
    assert(transfer_length == 8u && no_sense_calls == 0u);
}

static void TestSelector1UpdatesAndHonorsAllocation(void)
{
    Reset();
    memory[0x000b0bul] = 0x10u;
    memory[0x000b0cul] = 1u;
    memory[0x000b0dul] = 2u;
    memory[0x000b14ul] = 3u;
    memory[0x000c88ul] = 0xffu;
    HandleScsiOpcodeE9();
    AssertCommonResponse(1u, 2u);
    assert(memory[0x000c88ul] == 0xfeu);
    assert(transfer_length == 3u);
}

static void TestSelector2ReportsExistingSignedState(void)
{
    Reset();
    memory[0x000b0cul] = 2u;
    memory[0x00158eul] = 0x11u;
    memory[0x00158bul] = 0xfeu;
    memory[0x00158dul] = 7u;
    HandleScsiOpcodeE9();
    AssertCommonResponse(2u, 3u);
    assert(memory[0x000b2dul] == 0x82u);
    assert(memory[0x000b2eul] == 7u);
    assert(memory[0x00158bul] == 0xfeu);
}

static void TestSelector2UpdatesAndCapsSignedAdjustment(void)
{
    Reset();
    memory[0x000b0bul] = 0x10u;
    memory[0x000b0cul] = 2u;
    memory[0x000b0dul] = 1u;
    memory[0x000b0eul] = 0xffu;
    memory[0x000faeul] = 1u;
    memory[0x00158eul] = 0x13u;
    HandleScsiOpcodeE9();
    AssertCommonResponse(2u, 3u);
    assert(memory[0x00158eul] == 0x11u);
    assert(memory[0x00158bul] == 0xfeu);
    assert(memory[0x00158dul] == 1u);
    assert(memory[0x000b2dul] == 0x82u);
}

static void TestSelector2GuardPreservesState(void)
{
    Reset();
    memory[0x000b0bul] = 0x10u;
    memory[0x000b0cul] = 2u;
    memory[0x000b0dul] = 1u;
    memory[0x000b0eul] = 2u;
    memory[0x001008ul] = 2u;
    memory[0x00158eul] = 0x10u;
    memory[0x00158bul] = 5u;
    memory[0x00158dul] = 9u;
    HandleScsiOpcodeE9();
    AssertCommonResponse(2u, 2u);
    assert(memory[0x00158bul] == 5u && memory[0x00158dul] == 9u);
    assert(memory[0x000b2dul] == 5u);
}

static void TestSelectorBbRejectsReservedBytes(void)
{
    Reset();
    memory[0x000b0cul] = 0xbbu;
    memory[0x000b10ul] = 1u;
    memory[0x000b2bul] = 0xa5u;
    HandleScsiOpcodeE9();
    assert(unsupported_calls == 1u && transfer_calls == 0u);
    assert(memory[0x000b2aul] == 0xbbu);
    assert(memory[0x000b2bul] == 0xa5u);
}

static void TestSelectorBbReportsExistingState(void)
{
    Reset();
    memory[0x000b0cul] = 0xbbu;
    memory[0x000c88ul] = 8u;
    memory[0x00faedul] = 4u;
    HandleScsiOpcodeE9();
    AssertCommonResponse(0xbbu, 3u);
    assert(memory[0x000c88ul] == 8u && memory[0x00faedul] == 4u);
}

static void TestSelectorBbClearsSpeedOptionAndCapsSpeed(void)
{
    Reset();
    memory[0x000b0bul] = 0x10u;
    memory[0x000b0cul] = 0xbbu;
    memory[0x000b0dul] = 2u;
    memory[0x000c88ul] = 0x0cu;
    memory[0x00faedul] = 4u;
    memory[0x000c37ul] = 52u;
    HandleScsiOpcodeE9();
    AssertCommonResponse(0xbbu, 2u);
    assert(memory[0x000c88ul] == 8u);
    assert(memory[0x00faedul] == 0u);
    assert(memory[0x000c43ul] == 40u && memory[0x000c37ul] == 40u);
}

static void TestSelectorBbSetsSpeedOption(void)
{
    Reset();
    memory[0x000b0bul] = 0x10u;
    memory[0x000b0cul] = 0xbbu;
    memory[0x000b0dul] = 3u;
    HandleScsiOpcodeE9();
    AssertCommonResponse(0xbbu, 3u);
    assert(memory[0x000c88ul] == 0x0cu);
    assert(memory[0x000c43ul] == 52u);
}

static void TestUnsupportedSelectorAndTransferSense(void)
{
    Reset();
    memory[0x000b0cul] = 3u;
    HandleScsiOpcodeE9();
    assert(unsupported_calls == 1u && transfer_calls == 0u);

    Reset();
    memory[0x000b0cul] = 1u;
    transfer_result = 3u;
    HandleScsiOpcodeE9();
    assert(no_sense_calls == 1u);
}

int main(void)
{
    TestSelector1ReportsWithoutUpdating();
    TestSelector1UpdatesAndHonorsAllocation();
    TestSelector2ReportsExistingSignedState();
    TestSelector2UpdatesAndCapsSignedAdjustment();
    TestSelector2GuardPreservesState();
    TestSelectorBbRejectsReservedBytes();
    TestSelectorBbReportsExistingState();
    TestSelectorBbClearsSpeedOptionAndCapsSpeed();
    TestSelectorBbSetsSpeedOption();
    TestUnsupportedSelectorAndTransferSense();
    return 0;
}
