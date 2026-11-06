#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "ata/host_command_dispatch.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_ATA,
    EVENT_SCSI,
    EVENT_RESTART,
    EVENT_CLEAR,
    EVENT_PREPARE,
    EVENT_PUBLISH,
    EVENT_SCAN,
    EVENT_ADMISSION,
    EVENT_SENSE,
    EVENT_TRAY
};

struct event_record
{
    enum event_kind kind;
    u8 argument;
};

static u8 memory[1u << 24];
static struct event_record events[16];
static size_t event_count;
static unsigned opcode_read_count;
static unsigned controller_snapshot_read_count;
static u8 ata_result;
static u16 scan_result;
static u8 scan_force;
static u32 scan_frame_count;
static s32 scan_start_frame;
static u8 mutate_controller_snapshot;
static u32 first_controller_snapshot;
static u32 second_controller_snapshot;
static u8 admission_result;
static u8 controller_command_bytes[12];
static unsigned controller_command_read_count;
static u8 observed_sense_key;
static u16 observed_sense_code;

u8 FirmwareRead8(fw_addr_t address)
{
    address &= 0xfffffful;
    if(address == 0x000b0aul)
        opcode_read_count++;
    if(address == 0x018000ul)
    {
        assert(controller_command_read_count < 12u);
        return controller_command_bytes[controller_command_read_count++];
    }
    if(address >= 0x2b9704ul && address <= 0x2b9707ul)
    {
        if(mutate_controller_snapshot != 0u)
        {
            unsigned offset = (unsigned)(address - 0x2b9704ul);
            u32 value = controller_snapshot_read_count < 4u ? first_controller_snapshot : second_controller_snapshot;
            controller_snapshot_read_count++;
            return (u8)(value >> ((3u - offset) * 8u));
        }
        controller_snapshot_read_count++;
    }
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, u8 argument)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].argument = argument;
    event_count++;
}

u8 ObserveHostCommandAtaDispatch(void)
{
    RecordEvent(EVENT_ATA, 0u);
    return ata_result;
}

void ObserveHostCommandScsiDispatch(u8 opcode)
{
    RecordEvent(EVENT_SCSI, opcode);
}

void ObserveHostCommandRestart(void)
{
    RecordEvent(EVENT_RESTART, 0u);
}

void ObserveHostCommandClearCompletion(void)
{
    RecordEvent(EVENT_CLEAR, 0u);
}

void ObserveHostCommandPrepareCompletion(void)
{
    RecordEvent(EVENT_PREPARE, 0u);
}

void ObserveHostCommandPublishCompletion(void)
{
    RecordEvent(EVENT_PUBLISH, 0u);
}

u16 ObserveActiveHostScanControllerBanks(u8 force_bank_scan, u32 frame_count, s32 start_frame_position)
{
    RecordEvent(EVENT_SCAN, force_bank_scan);
    scan_force = force_bank_scan;
    scan_frame_count = frame_count;
    scan_start_frame = start_frame_position;
    return scan_result;
}

u8 ObserveActiveHostValidateScsiAdmission(void)
{
    RecordEvent(EVENT_ADMISSION, 0u);
    return admission_result;
}

void ObservePendingHostSetSense(u8 sense_key, u16 sense_code)
{
    observed_sense_key = sense_key;
    observed_sense_code = sense_code;
    RecordEvent(EVENT_SENSE, sense_key);
}

void ObservePendingHostTrayStatus(void)
{
    RecordEvent(EVENT_TRAY, 0u);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    opcode_read_count = 0u;
    controller_snapshot_read_count = 0u;
    ata_result = 0u;
    scan_result = 0u;
    scan_force = 0u;
    scan_frame_count = 0ul;
    scan_start_frame = 0l;
    mutate_controller_snapshot = 0u;
    first_controller_snapshot = 0ul;
    second_controller_snapshot = 0ul;
    admission_result = 0u;
    memset(controller_command_bytes, 0, sizeof(controller_command_bytes));
    controller_command_read_count = 0u;
    observed_sense_key = 0u;
    observed_sense_code = 0u;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void AssertEvent(size_t index, enum event_kind kind, u8 argument)
{
    assert(index < event_count);
    assert(events[index].kind == kind);
    assert(events[index].argument == argument);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void TestFailureLatchPriorityAndSignedBoundary(void)
{
    ResetState();
    WriteU32(0x2ba532ul, 0xfffffffful);
    WriteU32(0x2ba52eul, 0ul);
    memory[0x2ba5b1u] = 1u;
    memory[0x2ba5afu] = 1u;

    DispatchCurrentHostCommandAndPublishCompletion();

    assert(memory[0x2ba5b1u] == 0u);
    assert(memory[0x2ba5afu] == 1u);
    assert(memory[0x000c89u] == 0x88u);
    assert(memory[0x00122bu] == 20u);
    assert(event_count == 0u);

    ResetState();
    memory[0x000c89u] = 0x80u;
    memory[0x2ba5b1u] = 1u;
    memory[0x00e750u] = 1u;
    DispatchCurrentHostCommandAndPublishCompletion();
    assert(memory[0x00122bu] == 60u);
}

static void TestActiveRecordAcquisitionRoute(void)
{
    ResetState();
    memory[0x2ba5afu] = 1u;
    memory[0x000c8au] = 0x80u;
    memory[0x000c89u] = 0x10u;

    DispatchCurrentHostCommandAndPublishCompletion();

    assert(memory[0x000c8au] == 0x81u);
    assert(memory[0x000c89u] == 0x90u);
    assert(memory[0x00122cu] == 10u);
    assert(event_count == 0u);
}

static void TestCompletionCleanupGate(void)
{
    ResetState();
    memory[0x000c83u] = 0x10u;
    DispatchCurrentHostCommandAndPublishCompletion();
    assert(event_count == 1u);
    AssertEvent(0u, EVENT_CLEAR, 0u);
}

static void TestAtaDispatchAndPublication(void)
{
    ResetState();
    DispatchCurrentHostCommandAndPublishCompletion();
    assert(event_count == 3u);
    AssertEvent(0u, EVENT_ATA, 0u);
    AssertEvent(1u, EVENT_PREPARE, 0u);
    AssertEvent(2u, EVENT_PUBLISH, 0u);

    ResetState();
    ata_result = 1u;
    DispatchCurrentHostCommandAndPublishCompletion();
    assert(event_count == 4u);
    AssertEvent(0u, EVENT_ATA, 0u);
    AssertEvent(1u, EVENT_RESTART, 0u);
    AssertEvent(2u, EVENT_PREPARE, 0u);
    AssertEvent(3u, EVENT_PUBLISH, 0u);

    ResetState();
    memory[0x000c83u] = 1u;
    DispatchCurrentHostCommandAndPublishCompletion();
    assert(memory[0x000c83u] == 0u);
    assert(event_count == 2u);
    AssertEvent(0u, EVENT_ATA, 0u);
    AssertEvent(1u, EVENT_PUBLISH, 0u);
}

static void TestScsiTableDispatch(void)
{
    ResetState();
    memory[0x000c78u] = 1u;
    memory[0x000b0au] = 0xd8u;
    DispatchCurrentHostCommandAndPublishCompletion();
    assert(event_count == 3u);
    AssertEvent(0u, EVENT_SCSI, 0xd8u);
    AssertEvent(1u, EVENT_PREPARE, 0u);
    AssertEvent(2u, EVENT_PUBLISH, 0u);
    assert(opcode_read_count == 1u);
}

static void TestDeferredReadCompletion(void)
{
    static const u8 opcodes[] = { 0x08u, 0x28u, 0xa8u, 0xbeu };
    static const unsigned reads[] = { 2u, 3u, 4u, 5u };
    size_t index;

    for(index = 0u; index < sizeof(opcodes); index++)
    {
        ResetState();
        memory[0x000c78u] = 1u;
        memory[0x000b0au] = opcodes[index];
        memory[0x000c83u] = 2u;
        memory[0x000c89u] = 0x10u;
        DispatchCurrentHostCommandAndPublishCompletion();
        assert(memory[0x000c83u] == 2u);
        assert(memory[0x000c89u] == 0u);
        assert(event_count == 1u);
        AssertEvent(0u, EVENT_SCSI, opcodes[index]);
        assert(opcode_read_count == reads[index]);
    }

    ResetState();
    memory[0x000c78u] = 1u;
    memory[0x000b0au] = 0xd8u;
    memory[0x000c83u] = 2u;
    memory[0x000c89u] = 0x10u;
    DispatchCurrentHostCommandAndPublishCompletion();
    assert(memory[0x000c83u] == 0u);
    assert(memory[0x000c89u] == 0x10u);
    assert(event_count == 1u);
    AssertEvent(0u, EVENT_SCSI, 0xd8u);
    assert(opcode_read_count == 5u);
}

static void TestActiveOperationCompletionLatch(void)
{
    ResetState();
    memory[0x2ba5b0u] = 1u;
    memory[0x001588u] = 0x80u;
    memory[0x00fc48u] = 1u;
    memory[0x000c9eu] = 2u;
    memory[0x000c8au] = 1u;
    memory[0x000c89u] = 0xffu;
    ServiceCurrentHostCommandDuringActiveOperation();
    assert(memory[0x2ba5b0u] == 0u);
    assert(memory[0x001588u] == 0x84u);
    assert(memory[0x00fc49u] == 1u);
    assert(memory[0x000c8au] == 0x81u);
    assert(memory[0x000c89u] == 0xfeu);
    assert(event_count == 0u);

    ResetState();
    memory[0x2ba5b0u] = 1u;
    memory[0x00fc48u] = 1u;
    memory[0x000c9eu] = 3u;
    ServiceCurrentHostCommandDuringActiveOperation();
    assert(memory[0x00fc49u] == 3u);
    assert(memory[0x000c8bu] == 1u);

    ResetState();
    memory[0x2ba5b0u] = 1u;
    memory[0x000c9eu] = 1u;
    ServiceCurrentHostCommandDuringActiveOperation();
    assert(memory[0x000c8bu] == 1u);
}

static void TestActiveOperationScanSnapshots(void)
{
    ResetState();
    WriteU32(0x001153ul, 0x12345678ul);
    memory[0x000c9eu] = 3u;
    memory[0x000c89u] = 1u;
    memory[0x000c83u] = 0x10u;
    ServiceCurrentHostCommandDuringActiveOperation();
    assert(scan_force == 1u);
    assert(scan_frame_count == 0x12345678ul);
    assert(scan_start_frame == 0l);
    assert(ReadU32(0x000ce5ul) == 0x12345678ul);
    assert(ReadU32(0x000ce1ul) == 0x12345678ul);
    assert(memory[0x000c8bu] == 1u);
    assert(memory[0x000c89u] == 0u);
    assert(event_count == 2u);
    AssertEvent(0u, EVENT_SCAN, 1u);
    AssertEvent(1u, EVENT_CLEAR, 0u);

    ResetState();
    WriteU32(0x001153ul, 0x01020304ul);
    scan_result = 1u;
    mutate_controller_snapshot = 1u;
    first_controller_snapshot = 0x11223344ul;
    second_controller_snapshot = 0x55667788ul;
    memory[0x000c78u] = 1u;
    memory[0x000b0au] = 0xd8u;
    ServiceCurrentHostCommandDuringActiveOperation();
    assert(ReadU32(0x000ce1ul) == first_controller_snapshot);
    assert(ReadU32(0x000ce5ul) == second_controller_snapshot);
    assert(controller_snapshot_read_count == 8u);
    assert(event_count == 4u);
    AssertEvent(0u, EVENT_SCAN, 1u);
    AssertEvent(1u, EVENT_SCSI, 0xd8u);
    AssertEvent(2u, EVENT_PREPARE, 0u);
    AssertEvent(3u, EVENT_PUBLISH, 0u);
}

static void TestActiveOperationAsyncCompletion(void)
{
    ResetState();
    memory[0x2ba5b0u] = 2u;
    memory[0x000c83u] = 2u;
    ServiceCurrentHostCommandDuringActiveOperation();
    assert(memory[0x2ba5b0u] == 2u);
    assert(memory[0x000c83u] == 0u);
    assert(event_count == 2u);
    AssertEvent(0u, EVENT_SCAN, 1u);
    AssertEvent(1u, EVENT_ATA, 0u);
}

static void SetPacket(u8 opcode, u8 byte4)
{
    unsigned index;

    for(index = 0u; index < 12u; ++index)
        controller_command_bytes[index] = (u8)(0x80u + index);
    controller_command_bytes[0] = opcode;
    controller_command_bytes[4] = byte4;
    memory[0x000c32u] = 0x11u;
}

static void TestActivePacketPendingStates(void)
{
    ResetState();
    memory[0x000c32u] = 1u;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(memory[0x000c32u] == 1u && event_count == 0u);

    ResetState();
    memory[0x000c32u] = 0x10u;
    memory[0x000c8au] = 0x80u;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(memory[0x000c32u] == 1u);
    assert(memory[0x000c8au] == 0x82u);
    assert(event_count == 0u);

    ResetState();
    memory[0x000c32u] = 4u;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(memory[0x000c32u] == 1u && event_count == 0u);
}

static void TestActivePacketReceiveAndAdmissionFailure(void)
{
    unsigned index;

    ResetState();
    SetPacket(0x12u, 0x34u);
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(memory[0x000c32u] == 1u);
    assert(controller_command_read_count == 12u);
    for(index = 0u; index < 12u; ++index)
        assert(memory[0x000b0au + index] == controller_command_bytes[index]);
    assert(event_count == 3u);
    AssertEvent(0u, EVENT_ADMISSION, 0u);
    AssertEvent(1u, EVENT_PREPARE, 0u);
    AssertEvent(2u, EVENT_PUBLISH, 0u);
    assert(opcode_read_count == 0u);
}

static void TestActivePacketDeferredOpcodes(void)
{
    static const u8 opcodes[] = { 0x08u, 0x0au, 0x28u, 0x2au, 0x2eu, 0x35u, 0xa1u, 0xa8u, 0xaau, 0xb9u, 0xbbu, 0xbeu, 0x04u };
    size_t index;

    for(index = 0u; index < sizeof(opcodes); ++index)
    {
        ResetState();
        SetPacket(opcodes[index], 0u);
        admission_result = 1u;
        ReceiveAndDispatchHostPacketDuringActiveOperation();
        assert(memory[0x000c8au] == 2u);
        assert(event_count == 1u);
        AssertEvent(0u, EVENT_ADMISSION, 0u);
        assert(opcode_read_count == 1u);
    }
}

static void TestActivePacketStartStopRoutes(void)
{
    ResetState();
    SetPacket(0x1bu, 0u);
    admission_result = 1u;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(memory[0x000c8au] == 2u && memory[0x000c8bu] == 0u);
    assert(event_count == 1u);

    ResetState();
    SetPacket(0x1bu, 2u);
    admission_result = 1u;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(memory[0x000c8au] == 2u && memory[0x000c8bu] == 1u);
    assert(event_count == 1u);

    ResetState();
    SetPacket(0x1bu, 3u);
    admission_result = 1u;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(event_count == 4u);
    AssertEvent(0u, EVENT_ADMISSION, 0u);
    AssertEvent(1u, EVENT_SCSI, 0x1bu);
    AssertEvent(2u, EVENT_PREPARE, 0u);
    AssertEvent(3u, EVENT_PUBLISH, 0u);
}

static void TestActivePacketGetEventAndOrdinaryDispatch(void)
{
    ResetState();
    SetPacket(0x4au, 0u);
    admission_result = 1u;
    memory[0x2ba5b0u] = 2u;
    memory[0x001588u] = 0xffu;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(memory[0x001588u] == 0xfbu);
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_SCSI, 0x4au);

    ResetState();
    SetPacket(0x12u, 0u);
    admission_result = 1u;
    ReceiveAndDispatchHostPacketDuringActiveOperation();
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_SCSI, 0x12u);
    AssertEvent(2u, EVENT_PREPARE, 0u);
    AssertEvent(3u, EVENT_PUBLISH, 0u);
}

static void SetPendingPacket(u8 opcode, u8 byte4)
{
    unsigned index;

    memory[0x000c32u] = 0x11u;
    for(index = 0u; index < 12u; index++)
        controller_command_bytes[index] = (u8)(0x80u + index);
    controller_command_bytes[0] = opcode;
    controller_command_bytes[4] = byte4;
}

static void TestPendingHostStateConsumption(void)
{
    unsigned index;

    ResetState();
    memory[0x000c32u] = 4u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(memory[0x000c32u] == 1u && event_count == 0u);

    ResetState();
    memory[0x000c32u] = 0x55u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(memory[0x000c32u] == 1u && event_count == 0u);

    ResetState();
    memory[0x000c32u] = 0x10u;
    memory[0x000c89u] = 0x80u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(memory[0x000c32u] == 1u && memory[0x000c89u] == 0x84u);
    assert(memory[0x01804au] == 0u && event_count == 0u);

    ResetState();
    memory[0x000c32u] = 0x10u;
    memory[0x00faefu] = 1u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(memory[0x018056u] == 0x41u);
    assert(memory[0x018050u] == 4u && memory[0x018051u] == 3u);
    assert(memory[0x01804au] == 0xc0u);

    ResetState();
    SetPendingPacket(0x12u, 0u);
    admission_result = 0u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(controller_command_read_count == 12u);
    for(index = 0u; index < 12u; index++)
        assert(memory[0x000b0au + index] == controller_command_bytes[index]);
    assert(event_count == 3u);
    AssertEvent(0u, EVENT_ADMISSION, 0u);
    AssertEvent(1u, EVENT_PREPARE, 0u);
    AssertEvent(2u, EVENT_PUBLISH, 0u);
}

static void TestPendingHostActiveOperationPolicy(void)
{
    ResetState();
    SetPendingPacket(0x03u, 0u);
    admission_result = 1u;
    memory[0x000c84u] = 8u;
    memory[0x000c88u] = 0x40u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_SCSI, 0x03u);
    AssertEvent(2u, EVENT_PREPARE, 0u);
    AssertEvent(3u, EVENT_PUBLISH, 0u);

    ResetState();
    SetPendingPacket(0x52u, 0u);
    admission_result = 1u;
    memory[0x000c84u] = 8u;
    memory[0x000c88u] = 0x40u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_SENSE, 2u);
    assert(observed_sense_key == 2u && observed_sense_code == 0x0408u);

    ResetState();
    SetPendingPacket(0x12u, 0u);
    admission_result = 1u;
    memory[0x000c84u] = 8u;
    memory[0x000c88u] = 0x40u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_SENSE, 2u);
    assert(observed_sense_code == 0x0404u);
}

static void TestPendingHostDeferredPolicy(void)
{
    static const u8 deferred_opcodes[] = { 0x08u, 0x0au, 0x28u, 0x2au, 0x2eu, 0x35u, 0x5bu, 0xa1u, 0xa8u, 0xaau, 0xb9u, 0xbeu, 0x04u };
    size_t index;

    for(index = 0u; index < sizeof(deferred_opcodes); index++)
    {
        ResetState();
        SetPendingPacket(deferred_opcodes[index], 0u);
        admission_result = 1u;
        memory[0x000c88u] = 0x40u;
        ServicePendingHostCommandDuringMediaOperation();
        assert(memory[0x000c89u] == 4u);
        assert(event_count == 1u);
        AssertEvent(0u, EVENT_ADMISSION, 0u);
    }

    ResetState();
    SetPendingPacket(0x1bu, 1u);
    admission_result = 1u;
    memory[0x000c88u] = 0x40u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(memory[0x000c89u] == 4u && event_count == 1u);

    ResetState();
    SetPendingPacket(0x1bu, 0u);
    admission_result = 1u;
    memory[0x000c88u] = 0x40u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_SCSI, 0x1bu);
}

static void TestPendingHostReadinessPolicy(void)
{
    ResetState();
    SetPendingPacket(0x4au, 0u);
    admission_result = 1u;
    memory[0x000c9eu] = 2u;
    memory[0x2ba5b0u] = 1u;
    memory[0x001588u] = 0xffu;
    ServicePendingHostCommandDuringMediaOperation();
    assert(memory[0x001588u] == 0xfbu);
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_SCSI, 0x4au);

    ResetState();
    SetPendingPacket(0x52u, 0u);
    admission_result = 1u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(event_count == 4u && observed_sense_code == 0x0408u);

    ResetState();
    SetPendingPacket(0x55u, 0u);
    admission_result = 1u;
    memory[0x000c8bu] = 0x20u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(event_count == 4u);
    AssertEvent(1u, EVENT_TRAY, 0u);

    ResetState();
    SetPendingPacket(0x55u, 0u);
    admission_result = 1u;
    memory[0x000c87u] = 0x20u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(observed_sense_code == 0x0404u);

    ResetState();
    SetPendingPacket(0x55u, 0u);
    admission_result = 1u;
    ServicePendingHostCommandDuringMediaOperation();
    assert(observed_sense_code == 0x0408u);
}

int main(void)
{
    TestFailureLatchPriorityAndSignedBoundary();
    TestActiveRecordAcquisitionRoute();
    TestCompletionCleanupGate();
    TestAtaDispatchAndPublication();
    TestScsiTableDispatch();
    TestDeferredReadCompletion();
    TestActiveOperationCompletionLatch();
    TestActiveOperationScanSnapshots();
    TestActiveOperationAsyncCompletion();
    TestActivePacketPendingStates();
    TestActivePacketReceiveAndAdmissionFailure();
    TestActivePacketDeferredOpcodes();
    TestActivePacketStartStopRoutes();
    TestActivePacketGetEventAndOrdinaryDispatch();
    TestPendingHostStateConsumption();
    TestPendingHostActiveOperationPolicy();
    TestPendingHostDeferredPolicy();
    TestPendingHostReadinessPolicy();
    return 0;
}
