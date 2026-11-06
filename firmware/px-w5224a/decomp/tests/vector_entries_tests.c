#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "boot/vector_entries.h"

static u8 firmware_memory[0x100];
static unsigned events[16];
static unsigned event_count;

static void Record(unsigned event)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count++] = event;
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(firmware_memory));
    return firmware_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    firmware_memory[address] = value;
    Record(10u + value);
}

void FirmwareMainLoop(void)
{
    Record(1u);
}
void HandleControllerEventInterrupt(void)
{
    assert((VectorEntryPsw1 & 0x08u) != 0u);
    VectorEntryPsw1 = 0xffu;
    Record(2u);
}
void HandleHostInterfaceInterrupt(void)
{
    Record(3u);
}
void HandleControllerPacketInterrupt(void)
{
    assert((VectorEntryPsw1 & 0x10u) != 0u);
    VectorEntryPsw1 = 0xffu;
    Record(4u);
}
void Timer0TickHandler(void)
{
    Record(5u);
}
void HandleSfrE1Bit7Interrupt(void)
{
    VectorEntryPsw = 0xffu;
    VectorEntryPsw1 = 0xffu;
    Record(6u);
}
void RestartFirmware(void)
{
    Record(7u);
}

static void ResetState(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    VectorEntryPsw = 0u;
    VectorEntryPsw1 = 0u;
}

int main(void)
{
    ResetState();
    ResetVector();
    assert(event_count == 1u && events[0] == 1u);

    ResetState();
    VectorEntryPsw1 = 0xa5u;
    ControllerEventInterruptEntry();
    assert(event_count == 1u && events[0] == 2u);
    assert(VectorEntryPsw1 == 0xa5u);

    ResetState();
    HostInterfaceInterruptEntry();
    assert(event_count == 1u && events[0] == 3u);
    ResetState();
    firmware_memory[0xfau] = 1u;
    HostInterfaceInterruptEntry();
    assert(event_count == 3u && events[0] == 3u && events[1] == 10u && events[2] == 7u);
    assert(firmware_memory[0xfau] == 0u);

    ResetState();
    VectorEntryPsw1 = 0xa5u;
    ControllerPacketInterruptEntry();
    assert(event_count == 1u && events[0] == 4u);
    assert(VectorEntryPsw1 == 0xa5u);

    ResetState();
    Timer0InterruptEntry();
    assert(event_count == 1u && events[0] == 5u);

    ResetState();
    VectorEntryPsw = 0x96u;
    SfrE1Bit7InterruptEntry();
    assert(event_count == 1u && events[0] == 6u);
    assert(VectorEntryPsw1 == 0x96u);

    puts("vector entry tests passed");
    return 0;
}
