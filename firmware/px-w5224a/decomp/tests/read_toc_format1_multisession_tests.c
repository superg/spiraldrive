#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_toc_format1_multisession.h"

static u8 memory[0x1000000ul];
static u8 unsupported;
static u8 no_sense;
static u8 transfer_result;
static fw_addr_t transfer_source;
static u32 transfer_count;
static fw_addr_t converted_source;
static s32 converted_value;

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

void HandleUnsupportedScsiCommand(void)
{
    unsupported = 1u;
}
void SetNoSenseData(void)
{
    no_sense = 1u;
}

u16 MinU16(u16 value, u16 limit)
{
    return value < limit ? value : limit;
}

void WriteBigEndianU16(fw_addr_t address, u16 value)
{
    FirmwareWrite8(address, (u8)(value >> 8));
    FirmwareWrite8(address + 1u, (u8)value);
}

u16 ReadBigEndianU16(fw_addr_t address)
{
    return (u16)(((u16)FirmwareRead8(address) << 8) | FirmwareRead8(address + 1u));
}

u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source, u32 count)
{
    transfer_source = source;
    transfer_count = count;
    return transfer_result;
}

s32 ObserveReadTocFormat1ConvertMsf(fw_addr_t source_address)
{
    converted_source = source_address;
    return converted_value;
}

static fw_addr_t Descriptor(u8 index)
{
    return 0x2b8000ul + (fw_addr_t)((u16)index * 11u);
}

static u32 ReadResponseDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    unsupported = 0u;
    no_sense = 0u;
    transfer_result = 4u;
    transfer_source = 0u;
    transfer_count = 0u;
    converted_source = 0u;
    converted_value = 0x1000l;
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 0xffu;
}

static void CheckActiveResponse(void)
{
    u8 high_flag;
    u8 msf;

    for(high_flag = 0u; high_flag <= 1u; ++high_flag)
    {
        for(msf = 0u; msf <= 1u; ++msf)
        {
            Reset();
            memory[0x000c9eul] = 1u;
            memory[0x000c88ul] = high_flag != 0u ? 0x80u : 0u;
            memory[0x000b0bul] = msf != 0u ? 2u : 0u;
            ExecuteReadTocFormat1MultiSessionResponse();
            assert(memory[0x000b2aul] == 0u && memory[0x000b2bul] == 10u);
            assert(memory[0x000b2cul] == 1u && memory[0x000b2dul] == 1u);
            assert(ReadResponseDword(0x000b2eul) == 0x00170100ul);
            assert(ReadResponseDword(0x000b32ul) == (msf == 0u ? 0ul : (high_flag != 0u ? 0x00000200ul : 0x0000163cul)));
            assert(transfer_source == 0x000b2aul && transfer_count == 12u);
            assert(unsupported == 0u && no_sense == 0u);
        }
    }
}

static void CheckCachedUnavailable(void)
{
    Reset();
    ExecuteReadTocFormat1MultiSessionResponse();
    assert(unsupported == 1u && transfer_count == 0u);
}

static void FillOrdinaryCachedResponse(void)
{
    fw_addr_t descriptor;

    memory[0x0012c9ul] = 2u;
    memory[Descriptor(1u)] = 1u;
    memory[Descriptor(2u)] = 1u;
    memory[Descriptor(3u)] = 1u;
    memory[Descriptor(4u)] = 2u;
    descriptor = Descriptor(7u);
    memory[descriptor + 1u] = 0x41u;
    memory[descriptor + 3u] = 7u;
    memory[descriptor + 8u] = 8u;
    memory[descriptor + 9u] = 9u;
    memory[descriptor + 10u] = 10u;
}

static void CheckCachedMsfAndLba(void)
{
    fw_addr_t descriptor;

    Reset();
    FillOrdinaryCachedResponse();
    memory[0x000b0bul] = 2u;
    ExecuteReadTocFormat1MultiSessionResponse();
    assert(memory[0x000b2cul] == 1u && memory[0x000b2dul] == 2u);
    assert(memory[0x000b2eul] == 0u && memory[0x000b2ful] == 0x14u);
    assert(memory[0x000b30ul] == 7u && memory[0x000b31ul] == 0u);
    assert(ReadResponseDword(0x000b32ul) == 0x0008090aul);
    assert(converted_source == 0u && transfer_count == 12u);

    Reset();
    FillOrdinaryCachedResponse();
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 8u;
    transfer_result = 3u;
    ExecuteReadTocFormat1MultiSessionResponse();
    descriptor = Descriptor(7u);
    assert(converted_source == descriptor + 8u);
    assert(ReadResponseDword(0x000b32ul) == 0x00000f6aul);
    assert(transfer_count == 8u && no_sense == 1u);
}

static void CheckFixedAndDiscontinuousSessions(void)
{
    Reset();
    memory[0x0012c9ul] = 1u;
    memory[0x0012beul] = 8u;
    memory[Descriptor(1u)] = 1u;
    memory[0x000b0bul] = 2u;
    ExecuteReadTocFormat1MultiSessionResponse();
    assert(ReadResponseDword(0x000b2eul) == 0x00140100ul);
    assert(ReadResponseDword(0x000b32ul) == 0x00000200ul);

    Reset();
    memory[0x0012c9ul] = 3u;
    memory[Descriptor(1u)] = 1u;
    memory[Descriptor(2u)] = 4u;
    memory[Descriptor(4u) + 1u] = 0x12u;
    memory[Descriptor(4u) + 3u] = 0x44u;
    ExecuteReadTocFormat1MultiSessionResponse();
    assert(memory[0x000b2dul] == 1u);
    assert(memory[0x000b2ful] == 0x21u && memory[0x000b30ul] == 0x44u);
}

int main(void)
{
    CheckActiveResponse();
    CheckCachedUnavailable();
    CheckCachedMsfAndLba();
    CheckFixedAndDiscontinuousSessions();
    puts("read_toc_format1_multisession_tests: ok");
    return 0;
}
