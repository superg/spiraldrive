#include <assert.h>
#include <string.h>
#include "platform/arithmetic.h"
#include "platform/firmware_memory.h"
#include "scsi/vendor_f5_media_strategy_response.h"

static u8 memory[1u << 24];
static u32 inherited_dr12;
static u8 transfer_result;
static fw_addr_t transfer_source;
static u32 transfer_count;
static unsigned transfer_calls;
static unsigned no_sense_calls;
static unsigned sense_calls;
static u8 sense_key;
static u16 sense_code;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u16 MinU16(u16 value, u16 limit)
{
    return value < limit ? value : limit;
}

u32 ObserveCaptureInheritedDR12ForVendorF5MediaStrategyResponse(void)
{
    return inherited_dr12;
}

void ObserveVendorF5MediaStrategySense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}

u8 ObserveTransferVendorF5MediaStrategyResponse(fw_addr_t source, u32 byte_count)
{
    ++transfer_calls;
    transfer_source = source;
    transfer_count = byte_count;
    return transfer_result;
}

void ObserveVendorF5MediaStrategyNoSense(void)
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

static u16 ReadBe16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadBe32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(&memory[0x000b2au], 0xee, 0x52u);
    inherited_dr12 = 0x00f84000ul;
    transfer_result = 0u;
    transfer_source = 0u;
    transfer_count = 0u;
    transfer_calls = 0u;
    no_sense_calls = 0u;
    sense_calls = 0u;
    sense_key = 0u;
    sense_code = 0u;
    memory[0x000c85u] = 2u;
    memory[0x00fb53u] = 0x11u;
    memory[0x00fb54u] = 0x22u;
    memory[0x00fb55u] = 0x33u;
    memory[0x00fb5du] = 4u;
    WriteBe16(0x000b12u, 0x0060u);
}

static void AssertCommonHeader(void)
{
    assert(memory[0x000b2au] == 0u);
    assert(memory[0x000b2bu] == 0x50u);
    assert(memory[0x000b2cu] == 0u && memory[0x000b2du] == 0u && memory[0x000b2eu] == 0u && memory[0x000b2fu] == 0u);
    assert(memory[0x000b30u] == 0x11u);
    assert(memory[0x000b31u] == 0x22u);
    assert(memory[0x000b32u] == 0x33u);
    assert(memory[0x000b33u] == 4u);
}

static void AssertTransfer(u32 count)
{
    assert(transfer_calls == 1u);
    assert(transfer_source == 0x000b2aul);
    assert(transfer_count == count);
}

static void TestMediaNotReady(void)
{
    ResetState();
    memory[0x000c85u] = 0u;
    BuildVendorF5MediaStrategyResponse();
    assert(sense_calls == 1u && sense_key == 2u && sense_code == 0x0401u);
    assert(transfer_calls == 0u && no_sense_calls == 0u);
}

static void TestFixedMediaResponse(void)
{
    unsigned index;

    ResetState();
    memory[0x001008u] = 1u;
    memory[0x00fb5eu] = 0xaau;
    WriteBe16(0x00fb63u, 0x1234u);
    WriteBe16(0x00fb65u, 0x5678u);
    WriteBe16(0x00fb67u, 0x9abcu);
    memory[0x00f964u] = 0x21u;
    memory[0x00f965u] = 0x22u;
    memory[0x00f966u] = 0x23u;
    WriteBe16(0x00e752u, 0x3101u);
    WriteBe16(0x00e754u, 0x3202u);
    WriteBe16(0x00e756u, 0x3303u);
    WriteBe16(0x00e758u, 0x3404u);
    WriteBe16(0x00f9fau, 200u);
    WriteBe16(0x000ffeu, 50u);
    memory[0x00f968u] = 0x41u;
    memory[0x00f969u] = 0x42u;
    memory[0x00f96au] = 0x43u;
    memory[0x00f96bu] = 0x44u;
    WriteBe16(0x00fad7u, 1u);
    memory[0x00fc21u] = 4u;
    memory[0x00fb57u] = 0x51u;
    memory[0x00fb58u] = 0x52u;
    memory[0x00fb59u] = 0x53u;
    WriteBe32(0x00f9eeu, 0x11223344ul);
    WriteBe32(0x00f9f2u, 0x55667788ul);
    WriteBe32(0x00f9f6u, 0x99aabbccul);
    memory[0x00fad4u] = 0x5au;
    BuildVendorF5MediaStrategyResponse();

    AssertCommonHeader();
    assert(memory[0x000b34u] == 0xaau && memory[0x000b35u] == 0u);
    assert(ReadBe16(0x000b36u) == 0x1234u);
    assert(ReadBe16(0x000b38u) == 0x5678u);
    assert(ReadBe16(0x000b3au) == 0x9abcu);
    assert(memory[0x000b3cu] == 0x21u && memory[0x000b3du] == 0x22u && memory[0x000b3eu] == 0x23u && memory[0x000b3fu] == 0u);
    assert(ReadBe16(0x000b40u) == 0x3101u);
    assert(ReadBe16(0x000b42u) == 0x3202u);
    assert(ReadBe16(0x000b44u) == 0x3303u);
    assert(ReadBe16(0x000b46u) == 0x3404u);
    assert(ReadBe16(0x000b48u) == 100u);
    assert(memory[0x000b4au] == 0x41u && memory[0x000b4du] == 0x44u);
    assert(ReadBe16(0x000b56u) == 0x8001u);
    assert(memory[0x000b58u] == 0x51u && memory[0x000b5au] == 0x53u);
    assert(ReadBe32(0x000b5cu) == 0x11223344ul);
    assert(ReadBe32(0x000b60u) == 0x55667788ul);
    assert(ReadBe32(0x000b64u) == 0x99aabbccul);
    assert(memory[0x000b68u] == 0x5au);
    for(index = 0x3fu; index < 0x52u; ++index)
        assert(memory[0x000b2au + index] == 0u);
    AssertTransfer(0x52u);
    assert(no_sense_calls == 1u);
}

static void FillRecordProfile(fw_addr_t profile, fw_addr_t auxiliary, fw_addr_t fixed, u8 seed)
{
    memory[profile] = seed;
    memory[profile + 1u] = (u8)(seed + 1u);
    memory[profile + 3u] = 0xfeu;
    WriteBe16(profile + 4u, (u16)(0x6000u | seed));
    memory[profile + 6u] = 0xfdu;
    memory[profile + 7u] = 4u;
    memory[profile + 8u] = 0xfbu;
    memory[profile + 9u] = (u8)(seed + 9u);
    memory[fixed] = (u8)(seed + 10u);
    memory[fixed + 1u] = (u8)(seed + 11u);
    memory[fixed + 2u] = (u8)(seed + 12u);
    memory[fixed + 3u] = (u8)(seed + 13u);
    memory[auxiliary] = (u8)(seed + 14u);
    memory[auxiliary + 1u] = (u8)(seed + 15u);
    memory[auxiliary + 2u] = (u8)(seed + 16u);
}

static void AssertRecordProfile(fw_addr_t destination, u8 seed)
{
    assert(ReadBe16(destination) == seed);
    assert(ReadBe16(destination + 2u) == 0xfffeu);
    assert(ReadBe16(destination + 4u) == (u16)(0x6000u | seed));
    assert(ReadBe16(destination + 6u) == 0xfffdu);
    assert(ReadBe16(destination + 8u) == 4u);
    assert(ReadBe16(destination + 10u) == 0xfffbu);
    assert(memory[destination + 12u] == (u8)(seed + 1u));
    assert(memory[destination + 13u] == (u8)(seed + 9u));
    assert(memory[destination + 14u] == (u8)(seed + 10u));
    assert(memory[destination + 17u] == (u8)(seed + 13u));
    assert(memory[destination + 18u] == (u8)(seed + 14u));
    assert(memory[destination + 20u] == (u8)(seed + 16u));
}

static void TestRecordableFamilies(void)
{
    ResetState();
    memory[0x001008u] = 2u;
    memory[0x00fb80u] = 2u;
    memory[0x00f05fu] = 1u;
    FillRecordProfile(0x00f8308ful, 0x00f8346cul, 0x00f83451ul, 0x20u);
    WriteBe16(0x00fad7u, 0x0002u);
    memory[0x00fb57u] = 0x71u;
    memory[0x00fb58u] = 0x72u;
    memory[0x00fb59u] = 0x73u;
    BuildVendorF5MediaStrategyResponse();
    AssertRecordProfile(0x000b34ul, 0x20u);
    assert(ReadBe16(0x000b76u) == 0x4000u);
    assert(memory[0x000b78u] == 0x71u && memory[0x000b7au] == 0x73u);

    ResetState();
    memory[0x001008u] = 2u;
    memory[0x00fb80u] = 4u;
    memory[0x00f05fu] = 2u;
    FillRecordProfile(0x00f831c3ul, 0x00f834c0ul, 0x00f83451ul, 0x30u);
    FillRecordProfile(0x00f832cbul, 0x00f8350bul, 0x00f83459ul, 0x40u);
    BuildVendorF5MediaStrategyResponse();
    AssertRecordProfile(0x000b34ul, 0x30u);
    AssertRecordProfile(0x000b4aul, 0x40u);
}

static void TestAllocationAndTransferStatus(void)
{
    ResetState();
    WriteBe16(0x000b12u, 7u);
    transfer_result = 4u;
    BuildVendorF5MediaStrategyResponse();
    AssertTransfer(7u);
    assert(no_sense_calls == 0u);
}

int main(void)
{
    TestMediaNotReady();
    TestFixedMediaResponse();
    TestRecordableFamilies();
    TestAllocationAndTransferStatus();
    return 0;
}
