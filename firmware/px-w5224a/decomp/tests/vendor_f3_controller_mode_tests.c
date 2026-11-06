#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_f3_controller_mode.h"

typedef struct
{
    u8 event;
    u32 argument0;
    u32 argument1;
} Event;

typedef struct
{
    fw_addr_t address;
    u8 value;
} Write;

static u8 memory[1u << 24];
static Event events[32];
static Write writes[64];
static unsigned event_count;
static unsigned write_count;
static unsigned retained_word_high_reads;
static unsigned retained_word_low_reads;

u8 FirmwareRead8(fw_addr_t address)
{
    address &= 0xfffffful;
    if(address == 0x00f087ul)
        ++retained_word_high_reads;
    if(address == 0x00f088ul)
        ++retained_word_low_reads;
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    address &= 0xfffffful;
    assert(write_count < sizeof(writes) / sizeof(writes[0]));
    writes[write_count].address = address;
    writes[write_count].value = value;
    ++write_count;
    memory[address] = value;
}

void ObserveVendorF3ControllerMode(u8 event, u32 argument0, u32 argument1)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].event = event;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    ++event_count;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(writes, 0, sizeof(writes));
    event_count = 0u;
    write_count = 0u;
    retained_word_high_reads = 0u;
    retained_word_low_reads = 0u;
    memory[0x000fcau] = 0xb0u;
    memory[0x001302u] = 0xa5u;
    memory[0x00103bu] = 0x03u;
}

static void AssertEvent(unsigned index, u8 event, u32 argument0, u32 argument1)
{
    assert(index < event_count);
    assert(events[index].event == event);
    assert(events[index].argument0 == argument0);
    assert(events[index].argument1 == argument1);
}

static unsigned FindWrite(fw_addr_t address, u8 value, unsigned start)
{
    unsigned index;

    for(index = start; index < write_count; ++index)
        if(writes[index].address == address && writes[index].value == value)
            return index;
    assert(0);
    return 0u;
}

static void AssertCommonState(u8 mode)
{
    unsigned first_port8002;
    unsigned second_port8002;

    assert(memory[0x00f1bau] == mode);
    assert(memory[0x000e47u] == 2u);
    assert(memory[0x000e43u] == 2u);
    assert(memory[0x000e4fu] == 2u);
    assert(memory[0x00f05fu] == 0u);
    assert(memory[0x000efcu] == 0x40u);
    assert(memory[0x018005u] == 0x40u);
    assert(memory[0x000fcau] == 0xa0u);
    assert(memory[0x018002u] == 0xa0u);
    assert(memory[0x018032u] == 0u);
    assert(memory[0x018033u] == 0u);
    assert(memory[0x018034u] == 0u);
    assert(memory[0x018035u] == 0x13u);
    assert((memory[0x00103bu] & 0x30u) == 0x10u);

    first_port8002 = FindWrite(0x018002ul, 0xb8u, 0u);
    second_port8002 = FindWrite(0x018002ul, 0xa0u, first_port8002 + 1u);
    assert(first_port8002 < second_port8002);
}

static void AssertCommonHelperPrefix(u8 expects_state2)
{
    unsigned index = 0u;

    AssertEvent(index++, VENDOR_F3_MODE_WRITE_D6, 0u, 0u);
    if(expects_state2 != 0u)
        AssertEvent(index++, VENDOR_F3_MODE_CONFIGURE_STATE2, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_RESET_DATA_PATH, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_SET_CONTROLLER_FLAG, 1u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_WAIT_TIMER, 0x14u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_WRITE_E8E9, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_WRITE_REGISTER01, 2u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_INITIALIZE_STRATEGY, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_CONFIGURE_PROFILE, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_WRITE_EF00, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_WRITE_EF00, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_SET_GLOBAL_INTERRUPT, 0u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_SET_GLOBAL_INTERRUPT, 1u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_SET_CONTROLLER_INTERRUPT, 1u, 0u);
    AssertEvent(index++, VENDOR_F3_MODE_SET_A9_BIT2, 1u, 0u);
}

static void TestAdmissionFailures(void)
{
    ResetState();
    memory[0x00fab4u] = 0x80u;
    ExecuteVendorF3Selector0DControllerMode(1u);
    assert(event_count == 1u);
    AssertEvent(0u, VENDOR_F3_MODE_SEQUENCE_ERROR, 0u, 0u);
    assert(write_count == 0u);

    ResetState();
    memory[0x00103bu] = 0x20u;
    ExecuteVendorF3Selector0DControllerMode(1u);
    assert(event_count == 1u);
    AssertEvent(0u, VENDOR_F3_MODE_SEQUENCE_ERROR, 0u, 0u);
    assert(write_count == 0u);
}

static void TestModeZero(void)
{
    ResetState();
    ExecuteVendorF3Selector0DControllerMode(0u);
    AssertCommonHelperPrefix(0u);
    AssertCommonState(0u);
    assert(memory[0x001008u] == 1u);
    assert(memory[0x00fb5du] == 4u);
    assert(memory[0x00113bu] == 0x4au);
    assert(memory[0x001153u] == 0u && memory[0x001154u] == 5u && memory[0x001155u] == 0x14u && memory[0x001156u] == 0xc8u);
    assert(memory[0x0180b9u] == 0x40u);
    assert(memory[0x0180bau] == 0x50u);
    assert(memory[0x0180bdu] == 0u);
    assert(memory[0x001302u] == 0xb5u);
    AssertEvent(event_count - 5u, VENDOR_F3_MODE_SET_P3_RX, 1u, 0u);
    AssertEvent(event_count - 4u, VENDOR_F3_MODE_SET_P3_TX, 0u, 0u);
    AssertEvent(event_count - 3u, VENDOR_F3_MODE_SET_P3_RX, 0u, 0u);
    AssertEvent(event_count - 2u, VENDOR_F3_MODE_SET_P3_TX, 1u, 0u);
    AssertEvent(event_count - 1u, VENDOR_F3_MODE_NO_SENSE, 0u, 0u);
}

static void TestModeOneRetainedWordAndDirectP3(void)
{
    ResetState();
    memory[0x00f087u] = 0x12u;
    memory[0x00f088u] = 0x34u;
    memory[0x000c8au] = 8u;
    memory[0xf7c01fu] = 0u;
    ExecuteVendorF3Selector0DControllerMode(1u);
    AssertCommonHelperPrefix(1u);
    AssertCommonState(1u);
    assert(memory[0x0180b9u] == 0x40u);
    assert(memory[0x0180bau] == 0x50u);
    assert(memory[0x0180bdu] == 1u);
    assert(memory[0x001302u] == 0xa5u);
    assert(retained_word_high_reads == 2u);
    assert(retained_word_low_reads == 2u);
    AssertEvent(event_count - 6u, VENDOR_F3_MODE_APPLY_RETAINED_WORD, 0x1234u, 0u);
    AssertEvent(event_count - 5u, VENDOR_F3_MODE_SET_P3_RX, 1u, 0u);
    AssertEvent(event_count - 4u, VENDOR_F3_MODE_SET_P3_TX, 0u, 0u);
    AssertEvent(event_count - 3u, VENDOR_F3_MODE_SET_P3_RX, 1u, 0u);
    AssertEvent(event_count - 2u, VENDOR_F3_MODE_SET_P3_TX, 0u, 0u);
    AssertEvent(event_count - 1u, VENDOR_F3_MODE_NO_SENSE, 0u, 0u);
}

static void TestModeOneSentinelSkipsRetainedWord(void)
{
    ResetState();
    memory[0x00f087u] = 0xffu;
    memory[0x00f088u] = 0xffu;
    ExecuteVendorF3Selector0DControllerMode(1u);
    assert(retained_word_high_reads == 1u);
    assert(retained_word_low_reads == 1u);
    assert(events[event_count - 1u].event == VENDOR_F3_MODE_NO_SENSE);
}

static void TestModeTwoPreservesMediaGeometry(void)
{
    ResetState();
    memory[0x001008u] = 0x77u;
    memory[0x00fb5du] = 0x88u;
    memory[0x00113bu] = 0x99u;
    memory[0x001153u] = 1u;
    memory[0x001154u] = 2u;
    memory[0x001155u] = 3u;
    memory[0x001156u] = 4u;
    ExecuteVendorF3Selector0DControllerMode(2u);
    AssertCommonHelperPrefix(1u);
    AssertCommonState(2u);
    assert(memory[0x001008u] == 0x77u);
    assert(memory[0x00fb5du] == 0x88u);
    assert(memory[0x00113bu] == 0x99u);
    assert(memory[0x001153u] == 1u && memory[0x001154u] == 2u && memory[0x001155u] == 3u && memory[0x001156u] == 4u);
    assert(memory[0x0180b9u] == 0x30u);
    assert(memory[0x0180bau] == 0x10u);
    assert(memory[0x0180bdu] == 0u);
}

static void TestModesThreeAndUnsupported(void)
{
    ResetState();
    ExecuteVendorF3Selector0DControllerMode(3u);
    assert(memory[0x0180b9u] == 0x40u);
    assert(memory[0x0180bau] == 0x50u);
    assert(memory[0x0180bdu] == 1u);
    assert(memory[0x001008u] == 1u);

    ResetState();
    memory[0x0180b9u] = 0x21u;
    memory[0x0180bau] = 0x43u;
    memory[0x0180bdu] = 0x65u;
    ExecuteVendorF3Selector0DControllerMode(4u);
    assert(memory[0x0180b9u] == 0x21u);
    assert(memory[0x0180bau] == 0x43u);
    assert(memory[0x0180bdu] == 0x65u);
    assert(memory[0x001008u] == 1u);
    assert(events[event_count - 1u].event == VENDOR_F3_MODE_NO_SENSE);
}

int main(void)
{
    TestAdmissionFailures();
    TestModeZero();
    TestModeOneRetainedWordAndDirectP3();
    TestModeOneSentinelSkipsRetainedWord();
    TestModeTwoPreservesMediaGeometry();
    TestModesThreeAndUnsupported();
    return 0;
}
