#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ata/task_file_command_dispatch.h"
#include "platform/mcs251.h"

static u8 memory[0x1000000ul];
static u8 sfr[0x100u];
static unsigned build_identify_count;
static unsigned data_in_count;
static fw_addr_t data_in_address;
static u32 data_in_length;
static u8 data_in_clears_identify_bit;
static unsigned set_features_count;
static u8 set_features_result;
static unsigned delay_count;
static unsigned yield_count;
static u8 yield_mode;
static unsigned reset_media_count;
static unsigned write_d6_count;
static unsigned power_down_count;
static unsigned restart_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    return sfr[address];
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    sfr[address] = value;
}

void BuildAtaIdentifyPacketDeviceData(void)
{
    build_identify_count++;
}

void ExecuteControllerDataInTransfer(fw_addr_t address, u32 length)
{
    data_in_count++;
    data_in_address = address;
    data_in_length = length;
    if(data_in_clears_identify_bit != 0u)
        memory[0x0c87u] &= 0xf7u;
}

u8 ApplyAtaSetFeaturesCommand(void)
{
    set_features_count++;
    return set_features_result;
}

void DelayTimerUnits59(void)
{
    delay_count++;
}

void YieldCoroutine0(void)
{
    yield_count++;
    if(yield_mode == 1u && yield_count == 2u)
        memory[0x18043u] &= 0xefu;
    else if(yield_mode == 2u)
        memory[0x120fu] = 0u;
    else if(yield_mode == 3u)
        memory[0x15c3u] = 1u;
}

void ResetMediaProcessingCoroutines(void)
{
    reset_media_count++;
}

void WriteControllerParameterD6Value7E(void)
{
    write_d6_count++;
}

void EnterPowerDownModeWithInterruptsEnabled(void)
{
    power_down_count++;
}

void RestartFirmware(void)
{
    restart_count++;
}

static void ResetCase(void)
{
    memset(memory, 0, 0x2000u);
    memset(&memory[0xe700u], 0, 0x100u);
    memset(&memory[0x18000u], 0, 0x100u);
    memset(sfr, 0, sizeof(sfr));
    build_identify_count = 0u;
    data_in_count = 0u;
    data_in_address = 0u;
    data_in_length = 0u;
    data_in_clears_identify_bit = 0u;
    set_features_count = 0u;
    set_features_result = 1u;
    delay_count = 0u;
    yield_count = 0u;
    yield_mode = 0u;
    reset_media_count = 0u;
    write_d6_count = 0u;
    power_down_count = 0u;
    restart_count = 0u;
}

static u8 IsRecognizedCommand(u8 command)
{
    switch(command)
    {
    case 0x20u:
    case 0x90u:
    case 0xa1u:
    case 0xdau:
    case 0xe0u:
    case 0xe1u:
    case 0xe6u:
    case 0xecu:
    case 0xefu:
    case 0xfdu:
    case 0xfeu:
    case 0xffu:
        return 1u;
    default:
        return 0u;
    }
}

static void TestCompleteDispatchMatrix(void)
{
    unsigned command;

    for(command = 0u; command < 256u; ++command)
    {
        ResetCase();
        memory[0x18057u] = (u8)command;
        memory[0x0faeu] = 1u;
        memory[0x0c79u] = 2u;
        data_in_clears_identify_bit = 1u;
        assert(HandleAtaTaskFileCommand() == (u8)(command == 0xffu ? 0x64u : 0u));
        if(IsRecognizedCommand((u8)command) == 0u)
        {
            assert(memory[0x18056u] == 0x41u);
            assert(memory[0x18050u] == 4u);
            assert(memory[0x18051u] == 3u);
            assert(memory[0x1804au] == 0xc0u);
            assert(memory[0x0c83u] == 2u);
        }
    }
}

static void TestCommonResetCommands(void)
{
    static const u8 commands[] = { 0x20u, 0xdau, 0xecu };
    unsigned index;

    for(index = 0u; index < sizeof(commands); ++index)
    {
        ResetCase();
        memory[0x18057u] = commands[index];
        memory[0x0fcau] = 0x31u;
        sfr[0xa8u] = 0x80u;
        assert(HandleAtaTaskFileCommand() == 0u);
        assert(memory[0x1800du] == 0xffu);
        assert(memory[0x1800eu] == 0xebu);
        assert(memory[0x18022u] == 0xe0u);
        assert(memory[0x0fcau] == 0x21u && memory[0x18002u] == 0x21u);
        assert(sfr[0xa8u] == 0xc0u && (sfr[0xa9u] & 4u) != 0u);
        assert(memory[0x0c78u] == 1u && memory[0x0c83u] == 1u);
        assert(memory[0x0c2fu] == 0x51u && memory[0x0c30u] == 4u);
    }
}

static void TestDiagnosticRoutes(void)
{
    ResetCase();
    memory[0x18057u] = 0x90u;
    memory[0x0c79u] = 0u;
    memory[0x18043u] = 0x10u;
    yield_mode = 1u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(delay_count == 1u && yield_count == 2u);
    assert(memory[0x18041u] == 0u && memory[0x0c30u] == 1u);
    assert(memory[0x0c78u] == 1u && memory[0x0c2fu] == 0u);

    ResetCase();
    memory[0x18057u] = 0x90u;
    memory[0x0c79u] = 1u;
    memory[0x18043u] = 0x10u;
    yield_mode = 2u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(yield_count == 1u && memory[0x0c30u] == 0x81u);

    ResetCase();
    memory[0x18057u] = 0x90u;
    memory[0x0c79u] = 0u;
    memory[0x00ffu] = 1u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(memory[0x18046u] == 2u && yield_count == 0u);

    ResetCase();
    memory[0x18057u] = 0x90u;
    memory[0x0c79u] = 2u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(delay_count == 1u && memory[0x18046u] == 0xd0u);
}

static void TestIdentifyRoutes(void)
{
    ResetCase();
    memory[0x18057u] = 0xa1u;
    memory[0x155au] = 0x00u;
    memory[0x155bu] = 0x12u;
    memory[0x155cu] = 0x34u;
    memory[0x155du] = 0x56u;
    data_in_clears_identify_bit = 1u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(build_identify_count == 1u && data_in_count == 1u);
    assert(data_in_address == 0x00123456ul && data_in_length == 0x200ul);
    assert(memory[0x0c83u] == 2u);

    ResetCase();
    memory[0x18057u] = 0xa1u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(memory[0x0c83u] == 1u && memory[0x0c2fu] == 0xd0u);
    assert(memory[0x0c30u] == 0u && (memory[0x0c87u] & 8u) == 0u);
}

static void TestPowerAndSleepRoutes(void)
{
    static const u8 power_commands[] = { 0xe0u, 0xe1u };
    unsigned index;

    for(index = 0u; index < sizeof(power_commands); ++index)
    {
        ResetCase();
        memory[0x18057u] = power_commands[index];
        memory[0x0faeu] = 4u;
        assert(HandleAtaTaskFileCommand() == 0u);
        assert(reset_media_count == 1u);
        assert(memory[0x18056u] == 0x50u && memory[0x18050u] == 0u);
        assert(memory[0x1804au] == 0xc0u && memory[0x0c83u] == 2u);
    }

    ResetCase();
    memory[0x18057u] = 0xe0u;
    memory[0x0faeu] = 3u;
    assert(HandleAtaTaskFileCommand() == 0u && reset_media_count == 0u);

    ResetCase();
    memory[0x18057u] = 0xe6u;
    memory[0x0faeu] = 0u;
    sfr[0x90u] = 3u;
    yield_mode = 3u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(memory[0xe751u] == 1u && reset_media_count == 1u);
    assert(yield_count == 1u && sfr[0xb0u] == 3u && sfr[0x90u] == 0u);
    assert(write_d6_count == 1u && power_down_count == 1u);
    assert(memory[0x18017u] == 0u && memory[0x1800cu] == 0x11u);
    assert(memory[0x1800du] == 0u && memory[0x18040u] == 0x10u);
    assert((sfr[0xa8u] & 2u) != 0u);
}

static void TestSetFeaturesRestartAndTerminalCommands(void)
{
    ResetCase();
    memory[0x18057u] = 0xefu;
    set_features_result = 1u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(set_features_count == 1u && memory[0x0c78u] == 1u);
    assert(memory[0x0c2fu] == 0x50u && memory[0x0c30u] == 0u);

    ResetCase();
    memory[0x18057u] = 0xefu;
    set_features_result = 0u;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(memory[0x0c2fu] == 0x41u && memory[0x0c30u] == 4u);

    ResetCase();
    memory[0x18057u] = 0xfdu;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(restart_count == 1u && memory[0x00fdu] == 0x98u);
    assert(memory[0x1800du] == 0xffu && memory[0x18022u] == 0xe0u);

    ResetCase();
    memory[0x18057u] = 0xfeu;
    assert(HandleAtaTaskFileCommand() == 0u);
    assert(memory[0x0c83u] == 1u && memory[0x0c2fu] == 0x40u);

    ResetCase();
    memory[0x18057u] = 0xffu;
    assert(HandleAtaTaskFileCommand() == 0x64u);
    assert(memory[0x0c83u] == 1u && memory[0x0c2fu] == 0x40u);
}

int main(void)
{
    TestCompleteDispatchMatrix();
    TestCommonResetCommands();
    TestDiagnosticRoutes();
    TestIdentifyRoutes();
    TestPowerAndSleepRoutes();
    TestSetFeaturesRestartAndTerminalCommands();
    puts("ATA task-file command dispatch tests passed");
    return 0;
}
