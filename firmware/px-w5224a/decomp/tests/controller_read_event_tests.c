#include <assert.h>
#include <string.h>
#include "media/controller_read_event.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned long long event_bits;
static unsigned parsed_count;
static unsigned primary_count;
static unsigned completion_count;
static unsigned alignment_count;
static unsigned initialize_count;
static unsigned tracker_count;
static unsigned packed_count;
static unsigned parsed_advance_count;
static unsigned buffer_event_count;
static unsigned validation_count;
static unsigned countdown_count;
static u8 last_countdown_kind;
static u16 last_countdown_status;
static u32 last_alignment_cursor;
static u16 latest_descriptor;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObserveControllerReadEventReadBit(u8 address)
{
    return (u8)((event_bits >> address) & 1ull);
}
void ObserveControllerReadEventWriteBit(u8 address, u8 value)
{
    unsigned long long mask = 1ull << address;

    if(value != 0u)
        event_bits |= mask;
    else
        event_bits &= ~mask;
}
void ProcessParsedQRawReadProducerPosition(void)
{
    ++parsed_count;
}
void ProcessPrimaryRawReadProducerPosition(void)
{
    ++primary_count;
}
void ProcessControllerReadSectorCompletion(void)
{
    ++completion_count;
}
void AdvanceMainDataRingAndCheckControllerBufferAlignment(u32 cursor)
{
    ++alignment_count;
    last_alignment_cursor = cursor;
}
void InitializeControllerBufferCursorRegistersIfNeeded(void)
{
    ++initialize_count;
}
void TrackRawReadPositionAndArmTargetCapture(void)
{
    ++tracker_count;
}
void ProcessPackedSubchannelRecordBatch(void)
{
    ++packed_count;
}
void ServiceParsedQDescriptorCountdownAndPositionAdvance(void)
{
    ++parsed_advance_count;
}
void ProcessRawSectorBufferEvent(void)
{
    ++buffer_event_count;
}
void ServiceRawReadPositionValidationAndDescriptorEnqueue(void)
{
    ++validation_count;
}
void ServiceRawReadCountdownAndEnqueueDescriptor(u8 kind, u16 status)
{
    ++countdown_count;
    last_countdown_kind = kind;
    last_countdown_status = status;
}
u32 MultiplyU32Low(u32 left, u32 right)
{
    return left * right;
}
u16 GetLatestQueuedRawSectorDescriptorAddress(void)
{
    return latest_descriptor;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void WritePortCursor(fw_addr_t address, u32 value)
{
    memory[address] = (u8)value;
    memory[address + 1u] = (u8)(value >> 8);
    memory[address + 2u] = (u8)(value >> 16);
}

static void SetBit(u8 address)
{
    event_bits |= 1ull << address;
}

static u8 GetBit(u8 address)
{
    return (u8)((event_bits >> address) & 1ull);
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    event_bits = 0ull;
    parsed_count = primary_count = completion_count = 0u;
    alignment_count = initialize_count = tracker_count = packed_count = 0u;
    parsed_advance_count = buffer_event_count = validation_count = 0u;
    countdown_count = 0u;
    last_countdown_kind = 0xffu;
    last_countdown_status = 0xffffu;
    last_alignment_cursor = 0ul;
    latest_descriptor = 0x9000u;
    WriteDword(0x00156aul, 16ul);
    WriteDword(0x001562ul, 0x1000ul);
    WriteDword(0xff958dul + 4u, 4ul);
    WriteDword(0xff958dul + 8u, 1ul);
    WriteDword(0xff958dul + 16u, 1ul);
    WriteDword(0xff95a1ul + 4u, 0x498ul);
    WriteDword(0xff95a1ul + 8u, 0x126ul);
    WriteDword(0xff95a1ul + 16u, 0x372ul);
}

static void CheckStopAndSnapshotPaths(void)
{
    Reset();
    SetBit(0x15u);
    memory[0x000efcul] = 0xccu;
    memory[0x018005ul] = 0xccu;
    ProcessControllerReadEvent();
    assert(memory[0x000f76ul] == 0x7du && memory[0x000efcul] == 0u && memory[0x018005ul] == 0u && GetBit(0x15u) == 0u);

    Reset();
    memory[0x01800eul] = 0x82u;
    memory[0x018011ul] = 0x33u;
    memory[0x000f26ul] = 0x12u;
    memory[0x000f27ul] = 0x34u;
    memory[0x018010ul] = 0x56u;
    ProcessControllerReadEvent();
    assert(memory[0x000f2ful] == 0x82u && memory[0x000f31ul] == 0x33u && memory[0x000f30ul] == 0x12u && memory[0x000f26ul] == 0x34u && memory[0x000f27ul] == 0x56u);
}

static void CheckProducerDispatchAndMetadataSetup(void)
{
    Reset();
    memory[0x000f04ul] = 1u;
    ProcessControllerReadEvent();
    assert(parsed_count == 1u && primary_count == 0u);

    Reset();
    memory[0x000f04ul] = 1u;
    SetBit(0x1bu);
    ProcessControllerReadEvent();
    assert(parsed_count == 0u && primary_count == 1u);

    Reset();
    memory[0x000f04ul] = 2u;
    memory[0x000f20ul] = 1u;
    ProcessControllerReadEvent();
    assert(memory[0x000f20ul] == 0u && memory[0x000f04ul] == 2u);

    Reset();
    memory[0x000f04ul] = 2u;
    memory[0x018010ul] = 2u;
    memory[0x000e43ul] = 7u;
    WriteWord(0x000b00ul, 0x0a56u);
    ProcessControllerReadEvent();
    assert(GetBit(0x06u) == 1u && memory[0x000efbul] == 0x1cu && memory[0x018004ul] == 0x1cu && memory[0x000f04ul] == 3u);
}

static void CheckTransitionStates(void)
{
    Reset();
    memory[0x000f04ul] = 3u;
    memory[0x001302ul] = 0x80u;
    ProcessControllerReadEvent();
    assert(memory[0x000f21ul] == 1u && memory[0x000f04ul] == 4u && memory[0x000fcaul] == 4u && memory[0x018002ul] == 4u && memory[0x000f75ul] == 1u);

    Reset();
    memory[0x000f04ul] = 3u;
    memory[0x000f07ul] = 1u;
    ProcessControllerReadEvent();
    assert(memory[0x000f04ul] == 9u && memory[0x000f21ul] == 0u);

    Reset();
    memory[0x000f04ul] = 5u;
    ProcessControllerReadEvent();
    assert(tracker_count == 1u);

    Reset();
    memory[0x000f04ul] = 6u;
    ProcessControllerReadEvent();
    assert(memory[0x000efcul] == 0x87u && memory[0x018005ul] == 0x87u && memory[0x000f04ul] == 7u);

    Reset();
    memory[0x000f04ul] = 6u;
    memory[0x000efcul] = 0x55u;
    SetBit(0x13u);
    ProcessControllerReadEvent();
    assert(memory[0x000efcul] == 0x55u && memory[0x000f04ul] == 7u);
}

static void CheckState4Routes(void)
{
    Reset();
    memory[0x000f04ul] = 4u;
    memory[0x000ca1ul] = 1u;
    WriteWord(0x001296ul, 1u);
    ProcessControllerReadEvent();
    assert(initialize_count == 1u && ReadDword(0x000f7ful) == 0ul);

    Reset();
    memory[0x000f04ul] = 4u;
    memory[0x000f1ful] = 1u;
    SetBit(0x0cu);
    WritePortCursor(0x018008ul, 0x123456ul);
    ProcessControllerReadEvent();
    assert(alignment_count == 1u && last_alignment_cursor == 0x123456ul);

    Reset();
    memory[0x000f04ul] = 4u;
    memory[0x000f26ul] = 5u;
    memory[0x000f25ul] = 5u;
    WritePortCursor(0x018008ul, 0x654321ul);
    ProcessControllerReadEvent();
    assert(completion_count == 1u && ReadDword(0x00126eul) == 0x654321ul);

    Reset();
    memory[0x000f04ul] = 4u;
    memory[0x000f26ul] = 6u;
    memory[0x000f25ul] = 5u;
    SetBit(0x04u);
    ProcessControllerReadEvent();
    assert(countdown_count == 1u && last_countdown_kind == 3u && last_countdown_status == 0x0200u && GetBit(0x04u) == 0u && completion_count == 0u);

    Reset();
    memory[0x000f04ul] = 4u;
    memory[0x000f26ul] = 5u;
    memory[0x000f25ul] = 5u;
    memory[0x000f06ul] = 2u;
    memory[0x018020ul] = 10u;
    memory[0x018022ul] = 1u;
    memory[0x018023ul] = 0x40u;
    WriteWord(0x000b00ul, 0x0a56u);
    WritePortCursor(0x018019ul, 0x1200ul);
    ProcessControllerReadEvent();
    assert(completion_count == 1u && ReadDword(0x001262ul) == 9ul && ReadDword(0x00126aul) == 0x10daul && memory[0x000f74ul] == 1u);
}

static void CheckState7Routes(void)
{
    Reset();
    memory[0x000f04ul] = 7u;
    memory[0x000e46ul] = 0x20u;
    WritePortCursor(0x01800bul, 1ul);
    ProcessControllerReadEvent();
    assert(parsed_advance_count == 1u && buffer_event_count == 0u);

    Reset();
    memory[0x000f04ul] = 7u;
    SetBit(0x13u);
    WriteDword(0x001598ul, 0ul);
    ProcessControllerReadEvent();
    assert(packed_count == 1u && buffer_event_count == 0u);

    Reset();
    memory[0x000f04ul] = 7u;
    memory[0x000f06ul] = 4u;
    memory[0x000f0bul] = 2u;
    SetBit(0x13u);
    WriteDword(0x001598ul, (u32)-1000l);
    memory[0x01801dul] = 1u;
    WritePortCursor(0x018019ul, 0x123456ul);
    ProcessControllerReadEvent();
    assert(buffer_event_count == 1u);
    assert(ReadDword(0x00126aul) == 0x123456ul);
    assert(ReadDword(0x00126eul) == 0x123456ul);

    Reset();
    memory[0x000f04ul] = 7u;
    memory[0x000f06ul] = 1u;
    memory[0x018020ul] = 0xffu;
    memory[0x01801ful] = 0xffu;
    ProcessControllerReadEvent();
    assert(parsed_advance_count == 1u && memory[0x000f22ul] == 1u);

    Reset();
    memory[0x000f04ul] = 7u;
    memory[0x000f06ul] = 1u;
    memory[0x000f22ul] = 6u;
    memory[0x000e46ul] = 0x34u;
    memory[0x018020ul] = 0xffu;
    memory[0x01801ful] = 0xffu;
    WriteWord(0x9008ul, 14u);
    WritePortCursor(0x018008ul, 0x777ul);
    ProcessControllerReadEvent();
    assert(buffer_event_count == 1u && ReadDword(0x001262ul) == 15ul && ReadDword(0x00126eul) == 0x777ul);
}

static void CheckValidationState(void)
{
    Reset();
    memory[0x000f04ul] = 9u;
    memory[0x000f26ul] = 9u;
    WritePortCursor(0x018008ul, 0xabcdeful);
    ProcessControllerReadEvent();
    assert(ReadDword(0x00126eul) == 0xabcdeful && memory[0x000f1ful] == 7u && validation_count == 1u);
}

int main(void)
{
    CheckStopAndSnapshotPaths();
    CheckProducerDispatchAndMetadataSetup();
    CheckTransitionStates();
    CheckState4Routes();
    CheckState7Routes();
    CheckValidationState();
    return 0;
}
