#include <assert.h>
#include <string.h>
#include "platform/controller_event_dispatch.h"
#include "platform/firmware_memory.h"

enum
{
    EVENT_COMPLETE = 1,
    EVENT_40,
    EVENT_3,
    EVENT_READ,
    EVENT_PREPARE_4,
    EVENT_FINALIZE_4,
    EVENT_PUBLISH,
    EVENT_PROGRESS_2,
    EVENT_APPLY_BLOCK,
    EVENT_TARGET,
    EVENT_CAPTURE,
    EVENT_SERVICE_2,
    EVENT_7
};

static u8 memory[1u << 24];
static u8 events[32];
static unsigned event_count;
static u8 event_mask_input, source_status_input, bit43;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x018003ul)
        return event_mask_input;
    if(address == 0x01807ful)
        return source_status_input;
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveReadControllerEventBit43(void)
{
    return bit43;
}
void ObserveClearControllerEventBit43(void)
{
    bit43 = 0u;
}

#define EVENT_STUB(name, event)                                                                                                                                                                        \
    void name(void)                                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        events[event_count++] = event;                                                                                                                                                                 \
    }
EVENT_STUB(CompleteControllerOperatingResponse, EVENT_COMPLETE)
EVENT_STUB(HandleControllerEvent40Completion, EVENT_40)
EVENT_STUB(HandleControllerEventBit3Completion, EVENT_3)
EVENT_STUB(ProcessControllerReadEvent, EVENT_READ)
EVENT_STUB(PrepareControllerEventBit4OperatingUpdate, EVENT_PREPARE_4)
EVENT_STUB(FinalizeControllerEventBit4OperatingUpdate, EVENT_FINALIZE_4)
EVENT_STUB(PublishControllerOperatingResponse, EVENT_PUBLISH)
EVENT_STUB(HandleControllerEventBit2ReadMeasurementProgress, EVENT_PROGRESS_2)
EVENT_STUB(ApplyControllerRegisterBlock34And36Through3F, EVENT_APPLY_BLOCK)
EVENT_STUB(HandleReadMeasurementTargetPacketMatch, EVENT_TARGET)
EVENT_STUB(CaptureControllerThreeBytePacket, EVENT_CAPTURE)
EVENT_STUB(HandleControllerEventBit2MeasurementService, EVENT_SERVICE_2)
EVENT_STUB(HandleControllerEventBit7TransferProgress, EVENT_7)

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    event_mask_input = 0xfeu;
    source_status_input = 0xffu;
    bit43 = 0u;
}

static void TestAllActiveLowEventsAndFlagUpdates(void)
{
    static const u8 expected[] = { EVENT_COMPLETE, EVENT_40, EVENT_3, EVENT_READ, EVENT_PREPARE_4, EVENT_FINALIZE_4, EVENT_PUBLISH, EVENT_APPLY_BLOCK, EVENT_TARGET, EVENT_CAPTURE, EVENT_SERVICE_2,
        EVENT_7 };

    ResetState();
    event_mask_input = 0u;
    source_status_input = 0u;
    bit43 = 1u;
    memory[0x001038ul] = 0x0eu;
    memory[0x000fadul] = 0x36u;
    memory[0x001039ul] = 0x81u;
    memory[0x001301ul] = 1u;

    HandleControllerEventInterrupt();
    assert(event_count == sizeof(expected));
    assert(memcmp(events, expected, sizeof(expected)) == 0);
    assert(memory[0x00122aul] == 0u && memory[0x001097ul] == 0u);
    assert(memory[0x001038ul] == 0x08u);
    assert(memory[0x2ba5c0ul] == 1u && memory[0x2ba5c1ul] == 1u);
    assert(memory[0x000f78ul] == 0x50u);
    assert(memory[0x000fadul] == 0x24u);
    assert(memory[0x001302ul] == 0x20u);
    assert(bit43 == 0u);
    assert(memory[0x018003ul] == 0x24u);
    assert(memory[0x01807eul] == 0x08u);
}

static void TestBit2ProgressWithoutBit4Clear(void)
{
    static const u8 expected[] = { EVENT_PROGRESS_2, EVENT_APPLY_BLOCK, EVENT_TARGET, EVENT_CAPTURE };

    ResetState();
    event_mask_input = 0xfau;
    bit43 = 1u;
    memory[0x000fadul] = 4u;

    HandleControllerEventInterrupt();
    assert(event_count == sizeof(expected));
    assert(memcmp(events, expected, sizeof(expected)) == 0);
    assert(bit43 == 1u && memory[0x000fadul] == 0u);
    assert(memory[0x01802dul] == 4u);
}

static void TestSourceStatusCompletionGates(void)
{
    ResetState();
    source_status_input = 0xf1u;
    memory[0x001038ul] = 0x0eu;

    HandleControllerEventInterrupt();
    assert(event_count == 1u && events[0] == EVENT_COMPLETE);
    assert(memory[0x01802eul] == 8u);
    assert(memory[0x01806ful] == 0x10u);
    assert(memory[0x001038ul] == 8u);
    assert(memory[0x2ba5c0ul] == 1u && memory[0x2ba5c1ul] == 1u);
    assert(memory[0x01807eul] == 8u);
}

static void TestNoActiveEventsRepublishesShadows(void)
{
    ResetState();
    memory[0x000fadul] = 0xa5u;
    memory[0x001038ul] = 0x5au;

    HandleControllerEventInterrupt();
    assert(event_count == 0u);
    assert(memory[0x00122aul] == 0xfeu);
    assert(memory[0x001097ul] == 0xffu);
    assert(memory[0x018003ul] == 0xa5u);
    assert(memory[0x01807eul] == 0x5au);
}

int main(void)
{
    TestAllActiveLowEventsAndFlagUpdates();
    TestBit2ProgressWithoutBit4Clear();
    TestSourceStatusCompletionGates();
    TestNoActiveEventsRepublishesShadows();
    return 0;
}
