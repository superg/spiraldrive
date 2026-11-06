#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_master_cue_response.h"

static u8 memory[1u << 24];
static u8 bits[0x600];
static u8 poll_results[8];
static unsigned poll_count;
static unsigned poll_index;
static u16 descriptors[8];
static unsigned descriptor_count;
static unsigned descriptor_index;
static unsigned reset_queue_count;
static unsigned reset_transfer_count;
static u32 reset_transfer_result;
static unsigned initialize_count;
static u32 initialized_entries[4];
static unsigned yield_count;
static unsigned dispatch_count;
static unsigned sense_count;
static u8 sense_key;
static u16 sense_code;
static unsigned information_count;
static u32 sense_information;
static unsigned no_sense_count;
static fw_addr_t atomic_addresses[32];
static unsigned atomic_count;
static unsigned prefix_copy_count;
static fw_addr_t prefix_copy_source;
static u32 prefix_copy_length;
static fw_addr_t transfer_addresses[8];
static u32 transfer_lengths[8];
static unsigned transfer_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

#define DEFINE_BIT_WRITER(name, index)                                                                                                                                                                 \
    void name(u8 value)                                                                                                                                                                                \
    {                                                                                                                                                                                                  \
        bits[(index)] = (u8)(value != 0u);                                                                                                                                                             \
    }
DEFINE_BIT_WRITER(FirmwareWriteBit000C, 0x000c)
DEFINE_BIT_WRITER(FirmwareWriteBit000D, 0x000d)
DEFINE_BIT_WRITER(FirmwareWriteBit000E, 0x000e)
DEFINE_BIT_WRITER(FirmwareWriteBit000F, 0x000f)
DEFINE_BIT_WRITER(FirmwareWriteBit0013, 0x0013)
DEFINE_BIT_WRITER(FirmwareWriteBit0018, 0x0018)
DEFINE_BIT_WRITER(FirmwareWriteBit001B, 0x001b)
DEFINE_BIT_WRITER(FirmwareWriteBit001C, 0x001c)
DEFINE_BIT_WRITER(FirmwareWriteBit001D, 0x001d)
DEFINE_BIT_WRITER(FirmwareWriteBit0580, 0x0580)
DEFINE_BIT_WRITER(FirmwareWriteBit0581, 0x0581)

void SetSenseData(u8 key, u16 code)
{
    ++sense_count;
    sense_key = key;
    sense_code = code;
}

void SetSenseInformation(u32 information)
{
    ++information_count;
    sense_information = information;
}

void SetNoSenseData(void)
{
    ++no_sense_count;
}
void ResetRawReadQueueAndTransferState(void)
{
    ++reset_queue_count;
}
u32 ResetRawReadTransferState(void)
{
    ++reset_transfer_count;
    return reset_transfer_result;
}

void InitializeCoroutine2(u32 entry_point)
{
    initialized_entries[initialize_count++] = entry_point;
}

void YieldCoroutine0(void)
{
    ++yield_count;
}
void DispatchReadAcquisitionRequest5ForPipelineTarget(void)
{
    ++dispatch_count;
}

u8 PollReadMasterCueRawReadStatus(void)
{
    assert(poll_index < poll_count);
    return poll_results[poll_index++];
}

u16 GetRawReadSecondaryCursorDescriptorAddress(void)
{
    assert(descriptor_index < descriptor_count);
    return descriptors[descriptor_index++];
}

u8 ReadControllerBufferByteAtomic(fw_addr_t buffer_offset)
{
    atomic_addresses[atomic_count++] = buffer_offset;
    return FirmwareRead8(buffer_offset + 0x10u);
}

u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source, u32 byte_count)
{
    u32 index;

    ++prefix_copy_count;
    prefix_copy_source = source;
    prefix_copy_length = byte_count;
    for(index = 0u; index < byte_count; ++index)
        assert(FirmwareRead8(source + index) == 0u);
    return 4u;
}

void ExecuteControllerDataInTransfer(fw_addr_t source, u32 byte_count)
{
    transfer_addresses[transfer_count] = source;
    transfer_lengths[transfer_count++] = byte_count;
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

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(bits, 1, sizeof(bits));
    memset(poll_results, 0, sizeof(poll_results));
    memset(descriptors, 0, sizeof(descriptors));
    memset(initialized_entries, 0, sizeof(initialized_entries));
    memset(atomic_addresses, 0, sizeof(atomic_addresses));
    memset(transfer_addresses, 0, sizeof(transfer_addresses));
    memset(transfer_lengths, 0, sizeof(transfer_lengths));
    poll_count = poll_index = 0u;
    descriptor_count = descriptor_index = 0u;
    reset_queue_count = reset_transfer_count = 0u;
    reset_transfer_result = 0u;
    initialize_count = yield_count = dispatch_count = 0u;
    sense_count = information_count = no_sense_count = 0u;
    sense_key = 0u;
    sense_code = 0u;
    sense_information = 0u;
    atomic_count = prefix_copy_count = transfer_count = 0u;
    prefix_copy_source = 0u;
    prefix_copy_length = 0u;
}

static void SetDescriptor(u16 descriptor, u32 controller_address, u8 byte0, u8 byte1, u8 byte2, u8 byte3)
{
    WriteU32((fw_addr_t)descriptor + 4u, controller_address);
    memory[controller_address + 0x1cu] = byte0;
    memory[controller_address + 0x1du] = byte1;
    memory[controller_address + 0x1eu] = byte2;
    memory[controller_address + 0x1fu] = byte3;
}

static void TestRejectsMissingMedia(void)
{
    ResetFixture();
    WriteU32(0x0b02u, 0x12345678ul);
    ExecuteReadMasterCueResponse();
    assert(bits[0x000e] == 0u && bits[0x0018] == 0u);
    assert(sense_count == 1u && sense_key == 5u && sense_code == 0x6400u);
    assert(information_count == 1u && sense_information == 0x12345678ul);
    assert(reset_queue_count == 0u && prefix_copy_count == 0u);
}

static void TestRestartsAfterNonAcceptedPollStatus(void)
{
    ResetFixture();
    memory[0x1008u] = 1u;
    memory[0x0c2au] = 2u;
    reset_transfer_result = 0x00123456ul;
    poll_results[0] = 0xfeu;
    poll_results[1] = 0u;
    poll_count = 2u;
    ExecuteReadMasterCueResponse();
    assert(reset_queue_count == 2u && reset_transfer_count == 1u);
    assert(initialize_count == 2u && initialized_entries[0] == 0x00f90000ul && initialized_entries[1] == 0x00f84a00ul);
    assert(yield_count == 3u && dispatch_count == 1u);
    assert(ReadU32(0x1598u) == 0x00123456ul);
    assert(memory[0x1226u] == 4u && bits[0x0580] == 1u && bits[0x0581] == 1u && bits[0x000f] == 1u);
    assert(memory[0x0f08u] == 0x32u && ReadU32(0x0f0cu) == 0u);
    assert(bits[0x000c] == 0u && bits[0x000d] == 0u && bits[0x0013] == 0u && bits[0x001b] == 0u && bits[0x001c] == 0u && bits[0x001d] == 0u);
    assert(prefix_copy_count == 0u && no_sense_count == 0u);
}

static void TestReportsMissingSelectorAndRestoresCursor(void)
{
    ResetFixture();
    memory[0x1008u] = 1u;
    memory[0x0c2au] = 12u;
    memory[0x0b0eu] = 0x44u;
    WriteU16(0x1292u, 2u);
    WriteU16(0x1298u, 5u);
    WriteU16(0x129au, 7u);
    descriptors[0] = 0x0200u;
    descriptors[1] = 0x0220u;
    descriptor_count = 2u;
    SetDescriptor(0x0200u, 0x020000ul, 0xa9u, 0x44u, 0u, 1u);
    SetDescriptor(0x0220u, 0x021000ul, 0xaau, 0x45u, 0u, 1u);
    ExecuteReadMasterCueResponse();
    assert(descriptor_index == 2u && atomic_count == 8u);
    assert(ReadU16(0x1298u) == 5u);
    assert(sense_count == 1u && sense_key == 5u && sense_code == 0x6400u);
    assert(prefix_copy_count == 0u && no_sense_count == 0u);
}

static void TestTransfersMatchingChunksAndRemainder(void)
{
    ResetFixture();
    memory[0x1008u] = 1u;
    memory[0x0c2au] = 12u;
    memory[0x0b0eu] = 0x44u;
    WriteU32(0x0b06u, 0x1004ul);
    WriteU16(0x1292u, 2u);
    WriteU16(0x1298u, 5u);
    WriteU16(0x129au, 7u);
    descriptors[0] = 0x0200u;
    descriptors[1] = 0x0220u;
    descriptor_count = 2u;
    SetDescriptor(0x0200u, 0x020000ul, 0xa9u, 0x44u, 0u, 1u);
    SetDescriptor(0x0220u, 0x030000ul, 0xabu, 0x44u, 0x5au, 1u);
    memory[0x030010ul + 0x930ul + 0x0dul] = 0x44u;
    ExecuteReadMasterCueResponse();
    assert(descriptor_index == 2u);
    assert(prefix_copy_count == 1u && prefix_copy_source == 0x007000ul && prefix_copy_length == 4u);
    assert(transfer_count == 2u);
    assert(transfer_addresses[0] == 0x030010ul && transfer_lengths[0] == 0x800ul);
    assert(transfer_addresses[1] == 0x030940ul && transfer_lengths[1] == 0x800ul);
    assert(ReadU32(0x0b06u) == 0x800ul);
    assert(ReadU16(0x1298u) == 5u && memory[0x0c2au] == 12u);
    assert(no_sense_count == 1u && sense_count == 0u);
}

static void TestStopsAtMismatchingNextChunk(void)
{
    ResetFixture();
    memory[0x1008u] = 1u;
    memory[0x0c2au] = 12u;
    memory[0x0b0eu] = 0x44u;
    WriteU32(0x0b06u, 0x1005ul);
    WriteU16(0x1292u, 1u);
    WriteU16(0x1298u, 3u);
    WriteU16(0x129au, 7u);
    descriptors[0] = 0x0200u;
    descriptor_count = 1u;
    SetDescriptor(0x0200u, 0x040000ul, 0x20u, 0x44u, 0u, 0u);
    memory[0x040010ul + 0x930ul + 0x0dul] = 0x45u;
    ExecuteReadMasterCueResponse();
    assert(transfer_count == 1u && transfer_addresses[0] == 0x040010ul && transfer_lengths[0] == 0x800ul);
    assert(ReadU32(0x0b06u) == 0x801ul);
    assert(ReadU16(0x1298u) == 3u && no_sense_count == 1u);
}

int main(void)
{
    TestRejectsMissingMedia();
    TestRestartsAfterNonAcceptedPollStatus();
    TestReportsMissingSelectorAndRestoresCursor();
    TestTransfersMatchingChunksAndRemainder();
    TestStopsAtMismatchingNextChunk();
    return 0;
}
