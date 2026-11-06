#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/read_acquisition_controller_config.h"

typedef struct ControllerWrite
{
    fw_addr_t address;
    u8 value;
} ControllerWrite;

static u8 memory[0x1000000ul];
static ControllerWrite controller_writes[32];
static unsigned controller_write_count;
static u8 bit0033;
static u8 bit0042;
static unsigned bit0043_write_count;
static u8 bit0043_value;
static u8 interrupt_events[4];
static unsigned interrupt_event_count;
static unsigned register01_write_count;
static u8 register01_value;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
    if(address >= 0x018000ul && address <= 0x0180fful)
    {
        assert(controller_write_count < sizeof(controller_writes) / sizeof(controller_writes[0]));
        controller_writes[controller_write_count].address = address;
        controller_writes[controller_write_count].value = value;
        ++controller_write_count;
    }
}

u8 ObserveReadAcquisitionConfigBit0033(void)
{
    return bit0033;
}

u8 ObserveReadAcquisitionConfigBit0042(void)
{
    return bit0042;
}

void ObserveReadAcquisitionConfigBit0043(u8 value)
{
    ++bit0043_write_count;
    bit0043_value = value;
}

void ObserveReadAcquisitionConfigInterrupt(u8 event)
{
    assert(interrupt_event_count < sizeof(interrupt_events));
    interrupt_events[interrupt_event_count++] = event;
}

void ObserveReadAcquisitionConfigRegister01(u8 value)
{
    ++register01_write_count;
    register01_value = value;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(controller_writes, 0, sizeof(controller_writes));
    memset(interrupt_events, 0, sizeof(interrupt_events));
    controller_write_count = 0u;
    bit0033 = 0u;
    bit0042 = 0u;
    bit0043_write_count = 0u;
    bit0043_value = 0xffu;
    interrupt_event_count = 0u;
    register01_write_count = 0u;
    register01_value = 0u;
    memory[0x0e4fu] = 25u;
    memory[0x0e47u] = 11u;
    memory[0x1158u] = 4u;
    memory[0x0fcau] = 0xffu;
    memory[0x0fabu] = 0xffu;
    memory[0x18040u] = 0xabu;
}

static void AssertCommonTail(void)
{
    static const u8 expected_interrupts[4] = { 0u, 1u, 2u, 3u };

    assert(memory[0x180c1u] == 0x14u);
    assert(memory[0x180c0u] == 0u);
    assert(memory[0x0fcau] == 0xe7u);
    assert(memory[0x18002u] == 0xe7u);
    assert(memory[0x1806eu] == 0u);
    assert(memory[0x0fabu] == 0xcfu);
    assert(memory[0x18003u] == 0xcfu);
    assert(memory[0x1802du] == 0x30u);
    assert(memory[0x0efbu] == 2u);
    assert(memory[0x18004u] == 2u);
    assert(memory[0x1157u] == 0x0bu);
    assert(memory[0x18033u] == 0x1eu);
    assert(memory[0x18034u] == 0u);
    assert(memory[0x18035u] == 0u);
    assert(bit0043_write_count == 1u && bit0043_value == 0u);
    assert(register01_write_count == 1u && register01_value == 25u);
    assert(interrupt_event_count == 4u);
    assert(memcmp(interrupt_events, expected_interrupts, sizeof(expected_interrupts)) == 0);
}

static void TestExactControllerWriteTrace(void)
{
    static const ControllerWrite expected[] = {
        { 0x018002ul, 0xffu },
        { 0x0180c1ul, 0x14u },
        { 0x0180c2ul, 0xe5u },
        { 0x0180c0ul, 0x00u },
        { 0x0180cful, 0x00u },
        { 0x0180c3ul, 0x60u },
        { 0x018002ul, 0xe7u },
        { 0x018002ul, 0xefu },
        { 0x018031ul, 0x80u },
        { 0x018008ul, 0x41u },
        { 0x018009ul, 0x00u },
        { 0x01800aul, 0xf0u },
        { 0x01803aul, 0x80u },
        { 0x01806eul, 0x00u },
        { 0x018002ul, 0xe7u },
        { 0x018032ul, 0x16u },
        { 0x018033ul, 0x1eu },
        { 0x018034ul, 0x00u },
        { 0x018035ul, 0x00u },
        { 0x018003ul, 0xdfu },
        { 0x018003ul, 0xcfu },
        { 0x01802dul, 0x30u },
        { 0x018004ul, 0x02u }
    };
    unsigned index;

    ResetFixture();
    ConfigureReadAcquisitionControllerForCurrentState();

    assert(controller_write_count == sizeof(expected) / sizeof(expected[0]));
    for(index = 0u; index < controller_write_count; ++index)
    {
        assert(controller_writes[index].address == expected[index].address);
        assert(controller_writes[index].value == expected[index].value);
    }
    AssertCommonTail();
}

static void TestPortC2SelectionMatrix(void)
{
    static const struct
    {
        u8 media_code;
        u8 write_speed;
        u8 operating_code;
        u8 expected;
    } cases[] = {
        { 5u, 10u, 25u, 0xe0u },
        { 5u, 11u, 16u, 0xe0u },
        { 5u, 11u, 17u, 0xe1u },
        { 5u, 11u, 25u, 0xe2u },
        { 4u, 10u, 25u, 0xe0u },
        { 4u, 11u, 16u, 0xe1u },
        { 4u, 11u, 17u, 0xe3u },
        { 4u, 11u, 25u, 0xe5u }
    };
    unsigned index;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetFixture();
        memory[0x1158u] = cases[index].media_code;
        memory[0x0e47u] = cases[index].write_speed;
        memory[0x0e4fu] = cases[index].operating_code;

        ConfigureReadAcquisitionControllerForCurrentState();

        assert(memory[0x0e52u] == cases[index].operating_code);
        assert(memory[0x180c2u] == cases[index].expected);
    }
}

static void TestBit33AndSpecialOptionRoutes(void)
{
    ResetFixture();
    bit0033 = 1u;
    memory[0x1157u] = 2u;
    memory[0x1008u] = 2u;
    memory[0x0c46u] = 1u;

    ConfigureReadAcquisitionControllerForCurrentState();

    assert(memory[0x180cfu] == 0xc4u);
    assert(memory[0x180c3u] == 0x20u);
    assert(memory[0x1503u] == 0x80u);
    assert(memory[0x18032u] == 0x80u);
    AssertCommonTail();

    ResetFixture();
    memory[0x1157u] = 3u;

    ConfigureReadAcquisitionControllerForCurrentState();

    assert(memory[0x180cfu] == 0u);
    assert(memory[0x180c3u] == 0x60u);
    assert(memory[0x1503u] == 0x16u);
    assert(memory[0x18032u] == 0x16u);
}

static void TestReadModeSelection(void)
{
    ResetFixture();
    memory[0x0e4fu] = 19u;
    ConfigureReadAcquisitionControllerForCurrentState();
    assert(memory[0x1800au] == 0u && memory[0x1803au] == 0x80u);

    ResetFixture();
    bit0042 = 0u;
    memory[0x1285u] = 3u;
    ConfigureReadAcquisitionControllerForCurrentState();
    assert(memory[0x1800au] == 0xf0u && memory[0x1803au] == 0x80u);

    ResetFixture();
    bit0042 = 0u;
    memory[0x1285u] = 5u;
    ConfigureReadAcquisitionControllerForCurrentState();
    assert(memory[0x1800au] == 0xf1u && memory[0x1803au] == 0x42u);

    ResetFixture();
    bit0042 = 1u;
    memory[0x1285u] = 3u;
    ConfigureReadAcquisitionControllerForCurrentState();
    assert(memory[0x1800au] == 0xb0u && memory[0x1803au] == 0x80u);

    ResetFixture();
    bit0042 = 1u;
    memory[0xe750u] = 1u;
    memory[0x1285u] = 9u;
    ConfigureReadAcquisitionControllerForCurrentState();
    assert(memory[0x1800au] == 0xf0u && memory[0x1803au] == 0x80u);

    ResetFixture();
    bit0042 = 1u;
    memory[0xe750u] = 1u;
    memory[0x1285u] = 5u;
    ConfigureReadAcquisitionControllerForCurrentState();
    assert(memory[0x1800au] == 0xf1u && memory[0x1803au] == 0x42u);
}

static void TestRawQAndFinalPublication(void)
{
    ResetFixture();
    memory[0x1158u] = 5u;
    memory[0x1285u] = 9u;

    ConfigureReadAcquisitionControllerForCurrentState();

    assert(memory[0x18031u] == 0x40u);
    assert(memory[0x18008u] == 0x41u);
    assert(memory[0x18009u] == 0u);
    AssertCommonTail();

    ResetFixture();
    memory[0x1158u] = 5u;
    memory[0x1285u] = 8u;
    ConfigureReadAcquisitionControllerForCurrentState();
    assert(memory[0x18031u] == 0x80u);
}

int main(void)
{
    TestExactControllerWriteTrace();
    TestPortC2SelectionMatrix();
    TestBit33AndSpecialOptionRoutes();
    TestReadModeSelection();
    TestRawQAndFinalPublication();
    puts("read acquisition controller configuration tests passed");
    return 0;
}
