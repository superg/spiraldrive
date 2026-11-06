#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_subchannel_command.h"

static u8 memory[0x1000000ul], valid, unsupported, no_sense, status_result;
static u8 acquire_track, load_track, current_calls, isrc_format, mcn_calls;
static fw_addr_t isrc_destination;
static u32 transfer_count;
u8 FirmwareRead8(fw_addr_t a)
{
    assert(a < sizeof(memory));
    return memory[a];
}
void FirmwareWrite8(fw_addr_t a, u8 v)
{
    assert(a < sizeof(memory));
    memory[a] = v;
}
u8 ValidateScsiCdbFieldMasks(fw_addr_t m)
{
    assert(m == 0xff8dc8ul);
    return valid;
}
void HandleUnsupportedScsiCommand(void)
{
    unsupported = 1u;
}
void SetNoSenseData(void)
{
    no_sense = 1u;
}
u8 GetScsiAudioStatusCode(u8 a)
{
    assert(a == 1u);
    return 0x13u;
}
u16 ReadBigEndianU16(fw_addr_t a)
{
    return (u16)(((u16)memory[a] << 8) | memory[a + 1u]);
}
u16 MinU16(u16 a, u16 b)
{
    return a < b ? a : b;
}
u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t s, u32 n)
{
    assert(s == 0xb2aul);
    transfer_count = n;
    return status_result;
}
u8 GetEffectiveFirstTrackNumber(void)
{
    return 2u;
}
u8 GetEffectiveLastTrackNumber(void)
{
    return 5u;
}
void LoadTrackInformationByNumber(u8 t)
{
    load_track = t;
    memory[0xdd34ul] = 1u;
    memory[0xdd35ul] = 0x42u;
}
void AcquireReadSubChannelIdentificationForTrack(u8 t)
{
    acquire_track = t;
}
void BuildReadSubChannelMediaCatalogNumberField(fw_addr_t d)
{
    mcn_calls++;
    memory[d] = 0x80u;
}
void BuildReadSubChannelCurrentPositionPayloadC(fw_addr_t d)
{
    current_calls++;
    memory[d] = 1u;
}
void BuildReadSubChannelIsrcOrF1PayloadC(u8 f, fw_addr_t d)
{
    isrc_format = f;
    isrc_destination = d;
    memory[d] = 0x80u;
}

static void W(u16 v)
{
    memory[0xb11ul] = (u8)(v >> 8);
    memory[0xb12ul] = (u8)v;
}
static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    valid = 1u;
    unsupported = no_sense = 0u;
    status_result = 4u;
    acquire_track = load_track = current_calls = isrc_format = mcn_calls = 0u;
    isrc_destination = 0u;
    transfer_count = 0u;
    W(0xffffu);
}
static void Format(u8 f, u8 track, u8 subq)
{
    memory[0xb0dul] = f;
    memory[0xb10ul] = track;
    memory[0xb0cul] = subq ? 0x40u : 0u;
}

static void CheckRejectAndHeader(void)
{
    Reset();
    valid = 0u;
    HandleReadSubChannelCommand();
    assert(unsupported && transfer_count == 0u);
    Reset();
    Format(4u, 0u, 0u);
    HandleReadSubChannelCommand();
    assert(unsupported);
    Reset();
    Format(1u, 0u, 0u);
    W(3u);
    HandleReadSubChannelCommand();
    assert(transfer_count == 3u && memory[0xb2bul] == 0x13u);
}
static void CheckFormats(void)
{
    Reset();
    Format(1u, 0u, 1u);
    HandleReadSubChannelCommand();
    assert(current_calls == 1u && transfer_count == 16u);
    Reset();
    Format(2u, 0u, 1u);
    memory[0x1259ul] = 0u;
    memory[0x12bful] = 1u;
    HandleReadSubChannelCommand();
    assert(acquire_track == 1u && mcn_calls == 1u && transfer_count == 24u);
    Reset();
    Format(0xf0u, 0u, 1u);
    memory[0xfb7cul] = 5u;
    HandleReadSubChannelCommand();
    assert(memory[0xb31ul] == 0x3fu && transfer_count == 12u);
    Reset();
    Format(3u, 2u, 1u);
    memory[0x12c0ul] = 3u;
    memory[0xf2c3ul] = 0x41u;
    HandleReadSubChannelCommand();
    assert(acquire_track == 2u && isrc_format == 3u && isrc_destination == 0xb32ul && memory[0xb2ful] == 0x14u && transfer_count == 24u);
}
static void CheckWorkingAndFailure(void)
{
    Reset();
    Format(0xf1u, 4u, 1u);
    memory[0x1008ul] = 1u;
    status_result = 3u;
    HandleReadSubChannelCommand();
    assert(load_track == 4u && acquire_track == 4u && isrc_format == 0xf1u && memory[0xb2ful] == 0x24u && no_sense == 1u);
    Reset();
    Format(3u, 6u, 1u);
    memory[0x1008ul] = 1u;
    HandleReadSubChannelCommand();
    assert(unsupported);
}
int main(void)
{
    CheckRejectAndHeader();
    CheckFormats();
    CheckWorkingAndFailure();
    puts("read_subchannel_command_tests: ok");
    return 0;
}
