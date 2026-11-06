#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/host_interface_interrupt.h"

static u8 memory[1u << 20];
static u8 bit0008;
static u8 bit0023;
static u8 bit0480;
static u8 bit0481;
static u8 bit0580;
static u8 bit0581;
static unsigned action_counts[8];

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

u8 FirmwareReadBit0008(void)
{
    return bit0008;
}
u8 FirmwareReadBit0023(void)
{
    return bit0023;
}
void FirmwareWriteBit0480(u8 value)
{
    bit0480 = value;
}
void FirmwareWriteBit0481(u8 value)
{
    bit0481 = value;
}
void FirmwareWriteBit0580(u8 value)
{
    bit0580 = value;
}
void FirmwareWriteBit0581(u8 value)
{
    bit0581 = value;
}
void ObserveSetExternalInterrupt0Enable(void)
{
    ++action_counts[0];
}

void SetControllerFlagFC07Bit2AndPort8098Bit6(u8 enabled)
{
    assert(enabled == 0u);
    ++action_counts[1];
}

void WriteControllerParameterD6Value00(void)
{
    ++action_counts[2];
}
void HandleRawReadHostTransferCompletion(void)
{
    ++action_counts[3];
}
void HandleRawReadHostTransferDescriptorCompletion(void)
{
    ++action_counts[4];
}
void CompleteBatchedHostTransferBuffers(void)
{
    ++action_counts[5];
}
void CompleteSingleHostTransferBuffer(void)
{
    ++action_counts[6];
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(action_counts, 0, sizeof(action_counts));
    bit0008 = bit0023 = bit0480 = bit0481 = bit0580 = bit0581 = 0u;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void TestTransferCountSnapshot(void)
{
    ResetFixture();
    memory[0x01805au] = 2u;
    memory[0x0c83u] = 1u;
    WriteU32(0x1455u, 0x12345678ul);

    HandleHostInterfaceInterrupt();

    assert(memory[0x1596u] == 1u);
    assert(memory[0x1459u] == 0x12u && memory[0x145cu] == 0x78u);
    assert(memory[0x1462u] == 0u);
    assert(memory[0x0c83u] == 0x11u);
    assert(memory[0x018059u] == 0x97u);
}

static void TestDescriptorCompletion(void)
{
    ResetFixture();
    memory[0x01805au] = 4u;
    memory[0x1462u] = 1u;
    memory[0x127eu] = 1u;
    memory[0x0c2bu] = 7u;
    memory[0x018049u] = 0x55u;
    WriteU32(0x1455u, 0x01020304ul);
    WriteU32(0x1459u, 0x01020304ul);

    HandleHostInterfaceInterrupt();

    assert(memory[0x018049u] == 0u);
    assert(action_counts[4] == 1u);
    assert(action_counts[3] == 0u && action_counts[5] == 0u);
}

static void TestCommandRecovery(void)
{
    ResetFixture();
    memory[0x01805au] = 1u;
    memory[0x0c79u] = 0u;
    memory[0x018057u] = 0xfdu;
    memory[0x0fcau] = 0xffu;

    HandleHostInterfaceInterrupt();

    assert(action_counts[1] == 1u);
    assert(memory[0x01800eu] == 0xebu);
    assert(memory[0x01800du] == 0xffu);
    assert(memory[0x018022u] == 0xe0u);
    assert(memory[0x0fcau] == 0xe7u && memory[0x018002u] == 0xe7u);
    assert(memory[0x00fdu] == 0x98u && memory[0x00fau] == 1u);
    assert(memory[0x018059u] == 0x17u);
}

static void TestCommandAdmission(void)
{
    ResetFixture();
    memory[0x01805au] = 0x41u;
    memory[0x0c79u] = 1u;
    memory[0x0c32u] = 1u;

    HandleHostInterfaceInterrupt();

    assert(memory[0x0c32u] == 0x11u);
    assert(memory[0x0c78u] == 1u);
}

static void TestStatusCompletionAndPinRecovery(void)
{
    ResetFixture();
    memory[0x01805au] = 0x30u;
    memory[0x0c79u] = 0u;
    memory[0x158eu] = 3u;
    memory[0x158cu] = 24u;
    memory[0x0c37u] = 12u;
    memory[0xe751u] = 1u;
    memory[0x0faeu] = 0u;
    memory[0x0fcau] = 0xffu;

    HandleHostInterfaceInterrupt();

    assert(memory[0x0c38u] == 24u && memory[0x0e47u] == 24u);
    assert(memory[0x158eu] == 0u && memory[0x158bu] == 0u);
    assert(memory[0x1588u] == 0x21u);
    assert(memory[0x0e46u] == 12u && memory[0xfc20u] == 12u);
    assert(memory[0x0c83u] == 0x18u);
    assert(action_counts[0] == 1u && action_counts[2] == 1u);
    assert(bit0480 == 1u && bit0481 == 1u);
    assert(bit0580 == 1u && bit0581 == 0u);
    assert(memory[0x018050u] == 1u && memory[0x018051u] == 1u);
    assert(memory[0x01804au] == 0x80u);
}

static void TestStatusResetHandoff(void)
{
    ResetFixture();
    memory[0x01805au] = 0x10u;
    memory[0x0c86u] = 0u;

    HandleHostInterfaceInterrupt();

    assert(memory[0x00fdu] == 0x98u);
    assert(memory[0x00feu] == 0xffu);
    assert(memory[0x00fau] == 1u);
    assert(action_counts[1] == 1u);
}

int main(void)
{
    TestTransferCountSnapshot();
    TestDescriptorCompletion();
    TestCommandRecovery();
    TestCommandAdmission();
    TestStatusCompletionAndPinRecovery();
    TestStatusResetHandoff();
    puts("host interface interrupt tests passed");
    return 0;
}
