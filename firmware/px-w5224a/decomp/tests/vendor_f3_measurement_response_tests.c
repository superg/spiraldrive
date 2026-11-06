#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_f3_measurement_response.h"

static u8 memory[1u << 24];
static u8 inherited_r15;
static u8 transfer_result;
static fw_addr_t transfer_source;
static u32 transfer_count;
static unsigned transfer_calls;
static unsigned unsupported_calls;
static unsigned no_sense_calls;
static unsigned status_1080_reads;
static unsigned status_10a2_reads;

u8 FirmwareRead8(fw_addr_t address)
{
    address &= 0xfffffful;
    if(address == 0x001080ul)
        ++status_1080_reads;
    if(address == 0x0010a2ul)
        ++status_10a2_reads;
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveCaptureInheritedR15ForVendorF3MeasurementResponse(void)
{
    return inherited_r15;
}

void ObserveUnsupportedVendorF3MeasurementResponse(void)
{
    ++unsupported_calls;
}

u8 ObserveTransferVendorF3MeasurementResponse(fw_addr_t source, u32 byte_count)
{
    ++transfer_calls;
    transfer_source = source;
    transfer_count = byte_count;
    return transfer_result;
}

void ObserveVendorF3MeasurementResponseNoSense(void)
{
    ++no_sense_calls;
}

static void WriteBe16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteBe32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(&memory[0x000b2au], 0xcc, 0x20u);
    inherited_r15 = 9u;
    transfer_result = 0u;
    transfer_source = 0u;
    transfer_count = 0u;
    transfer_calls = 0u;
    unsupported_calls = 0u;
    no_sense_calls = 0u;
    status_1080_reads = 0u;
    status_10a2_reads = 0u;
    memory[0x000b14u] = 0xffu;
    memory[0x001098u] = 0xffu;
}

static void AssertTransfer(u32 count)
{
    assert(transfer_calls == 1u);
    assert(transfer_source == 0x000b2aul);
    assert(transfer_count == count);
}

static void AssertBytes(fw_addr_t address, const u8 *expected, unsigned count)
{
    unsigned index;

    for(index = 0u; index < count; ++index)
        assert(memory[address + index] == expected[index]);
}

static void AssertConsumed(void)
{
    assert(memory[0x001098u] == 0xdfu);
    assert(memory[0x001082u] == 0u);
    assert(no_sense_calls == 1u);
}

static void TestCompactEightChannelResult(void)
{
    static const u8 expected[16] = { 0x00u, 0x0eu, 0x01u, 0x05u, 0x00u, 0x07u, 0x00u, 0x00u, 0x0fu, 0x6au, 0x11u, 0x22u, 0xaau, 0xbbu, 0x00u, 0x00u };

    ResetState();
    memory[0x001080u] = 1u;
    memory[0x001082u] = 5u;
    memory[0x001081u] = 7u;
    WriteBe32(0x001083u, 0x00001000ul);
    WriteBe16(0x001087u, 0x1122u);
    WriteBe16(0x001091u, 0xaabbu);
    BuildVendorF3MeasurementResultResponse();

    AssertBytes(0x000b2au, expected, sizeof(expected));
    AssertTransfer(16u);
    AssertConsumed();
    assert(status_1080_reads == 2u);
}

static void TestCompactUnavailableResult(void)
{
    unsigned index;

    ResetState();
    memory[0x001080u] = 0u;
    BuildVendorF3MeasurementResultResponse();
    assert(memory[0x000b2au] == 0u);
    assert(memory[0x000b2bu] == 0x0eu);
    for(index = 2u; index < 16u; ++index)
        assert(memory[0x000b2au + index] == 0u);
    AssertTransfer(16u);
    assert(status_1080_reads == 2u);
}

static void TestExtendedEightChannelResult(void)
{
    unsigned index;

    ResetState();
    memory[0x000b0cu] = 1u;
    memory[0x000b14u] = 20u;
    memory[0x001080u] = 2u;
    memory[0x001082u] = 3u;
    memory[0x001081u] = 4u;
    WriteBe32(0x001083u, 0x00000100ul);
    for(index = 0u; index < 8u; ++index)
        WriteBe16(0x001087u + index * 2u, (u16)(0x1000u + index));
    BuildVendorF3MeasurementResultResponse();

    assert(memory[0x000b2au] == 0u && memory[0x000b2bu] == 0x18u);
    assert(memory[0x000b2cu] == 2u && memory[0x000b2du] == 3u);
    assert(memory[0x000b2eu] == 0u && memory[0x000b2fu] == 4u);
    assert(memory[0x000b30u] == 0u && memory[0x000b31u] == 0u && memory[0x000b32u] == 0u && memory[0x000b33u] == 0x6au);
    for(index = 0u; index < 8u; ++index)
    {
        assert(memory[0x000b34u + index * 2u] == 0x10u);
        assert(memory[0x000b35u + index * 2u] == index);
    }
    AssertTransfer(20u);
    AssertConsumed();
}

static void TestExtendedUnavailableResult(void)
{
    unsigned index;

    ResetState();
    memory[0x000b0cu] = 2u;
    BuildVendorF3MeasurementResultResponse();
    assert(memory[0x000b2au] == 0u && memory[0x000b2bu] == 0x18u);
    for(index = 2u; index < 26u; ++index)
        assert(memory[0x000b2au + index] == 0u);
    AssertTransfer(26u);
}

static void TestControllerCommand0EResult(void)
{
    static const u8 expected[16] = { 0x00u, 0x0eu, 0x03u, 0xa1u, 0x00u, 0x06u, 0x00u, 0x00u, 0x01u, 0x6au, 0x12u, 0x34u, 0x56u, 0x78u, 0x01u, 0x00u };

    ResetState();
    memory[0x000b0cu] = 0xa0u;
    memory[0xf7c01fu] = 1u;
    memory[0x0010a2u] = 3u;
    memory[0x0010a3u] = 6u;
    WriteBe32(0x0010a4u, 0x00000200ul);
    WriteBe16(0x0010acu, 0x1234u);
    WriteBe16(0x0010b2u, 0x5678u);
    memory[0x001098u] = 0xe0u;
    BuildVendorF3MeasurementResultResponse();

    AssertBytes(0x000b2au, expected, sizeof(expected));
    AssertTransfer(16u);
    assert(status_10a2_reads == 2u);
    assert(memory[0x001098u] == 0xc0u);
    assert(no_sense_calls == 1u);
}

static void TestControllerCommand0EUnavailableResult(void)
{
    unsigned index;

    ResetState();
    memory[0x000b0cu] = 0x50u;
    memory[0xf7c01fu] = 1u;
    memory[0x0010a2u] = 0u;
    memory[0x001098u] = 0x40u;
    BuildVendorF3MeasurementResultResponse();

    assert(memory[0x000b2au] == 0u && memory[0x000b2bu] == 0x0eu);
    assert(memory[0x000b2cu] == 0u && memory[0x000b2du] == 0x50u);
    for(index = 4u; index < 14u; ++index)
        assert(memory[0x000b2au + index] == 0u);
    assert(memory[0x000b38u] == 1u && memory[0x000b39u] == 0u);
    AssertTransfer(16u);
}

static void TestUnsupportedUsesInheritedTransferLimit(void)
{
    unsigned index;

    ResetState();
    memory[0x000b0cu] = 0x10u;
    memory[0xf7c01fu] = 0u;
    inherited_r15 = 9u;
    BuildVendorF3MeasurementResultResponse();

    assert(unsupported_calls == 1u);
    AssertTransfer(9u);
    for(index = 0u; index < 0x20u; ++index)
        assert(memory[0x000b2au + index] == 0xccu);
    AssertConsumed();
}

static void TestTransferStatusFourPreservesReadyState(void)
{
    ResetState();
    memory[0x001080u] = 1u;
    memory[0x001082u] = 5u;
    transfer_result = 4u;
    BuildVendorF3MeasurementResultResponse();

    assert(memory[0x001098u] == 0xffu);
    assert(memory[0x001082u] == 5u);
    assert(no_sense_calls == 0u);
}

int main(void)
{
    TestCompactEightChannelResult();
    TestCompactUnavailableResult();
    TestExtendedEightChannelResult();
    TestExtendedUnavailableResult();
    TestControllerCommand0EResult();
    TestControllerCommand0EUnavailableResult();
    TestUnsupportedUsesInheritedTransferLimit();
    TestTransferStatusFourPreservesReadyState();
    return 0;
}
