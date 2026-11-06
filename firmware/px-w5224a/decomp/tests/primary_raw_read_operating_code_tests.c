#include <assert.h>
#include <string.h>
#include "media/primary_raw_read_operating_code.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned long long operating_bits;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObservePrimaryRawReadOperatingCodeBit(u8 address)
{
    return (u8)((operating_bits >> address) & 1ull);
}

static void SetBit(u8 address)
{
    operating_bits |= 1ull << address;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    operating_bits = 0ull;
    memory[0x000e46ul] = 0x28u;
    memory[0x000f45ul] = 0x40u;
    memory[0x000f1cul] = 0xaau;
    memory[0x000f1dul] = 0xbbu;
    memory[0x000f3bul] = 0xccu;
    memory[0x000f3cul] = 0xddu;
}

static void CheckEntryGatesAndWordClears(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    memory[0x000c89ul] = 1u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000f3bul] == 0xccu && memory[0x000f3cul] == 0xddu && memory[0x000f1cul] == 0xaau && memory[0x000f1dul] == 0xbbu);

    Reset();
    memory[0x0015d7ul] = 1u;
    SetBit(0x1cu);
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000f3bul] == 0u && memory[0x000f3cul] == 0u && memory[0x000f1cul] == 0xaau && memory[0x000f1dul] == 0xbbu);

    Reset();
    SetBit(0x1du);
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000f1cul] == 0xaau && memory[0x000f1dul] == 0xbbu);

    Reset();
    memory[0x000f05ul] = 1u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000f1cul] == 0u && memory[0x000f1dul] == 0u);
}

static void CheckStateOneSelection(void)
{
    Reset();
    memory[0x000f05ul] = 1u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e44ul] == 4u);

    Reset();
    memory[0x000f05ul] = 1u;
    memory[0x000c86ul] = 0x40u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e44ul] == 0u);

    Reset();
    memory[0x000f05ul] = 1u;
    memory[0x000e49ul] = 0x30u;
    memory[0x000e46ul] = 8u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e49ul] == 8u);
}

static void CheckStateThreeCountdownAndBackoff(void)
{
    Reset();
    memory[0x000f05ul] = 3u;
    memory[0x000e4aul] = 1u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e44ul] == 5u && memory[0x000e4aul] == 0u && memory[0x000f3bul] == 0xccu);

    Reset();
    memory[0x000f05ul] = 3u;
    memory[0x000e43ul] = 8u;
    memory[0x000e4aul] = 0u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e4aul] == 5u && memory[0x000e49ul] == 4u && memory[0x000f45ul] == 4u && memory[0x000f3bul] == 0u && memory[0x000f3cul] == 0u);

    Reset();
    memory[0x000f05ul] = 3u;
    memory[0x000e43ul] = 2u;
    memory[0x000c86ul] = 0x40u;
    memory[0x000e4aul] = 0u;
    memory[0x001008ul] = 2u;
    SetBit(0x0bu);
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e49ul] == 1u && memory[0x000e44ul] == 0u && memory[0x000f45ul] == 1u);
}

static void CheckSynchronizedAndAdvancePaths(void)
{
    static const u8 operating_codes[] = { 2u, 4u, 8u, 0x18u, 0x20u };
    static const u8 expected_codes[] = { 4u, 8u, 0x18u, 0x20u, 0x28u };
    unsigned index;

    Reset();
    memory[0x000f05ul] = 2u;
    memory[0x000e48ul] = 1u;
    memory[0x000e4aul] = 9u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e44ul] == 1u && memory[0x000e49ul] == 0x28u && memory[0x000e4aul] == 0u);

    Reset();
    memory[0x000f05ul] = 2u;
    memory[0x000e49ul] = 0x28u;
    memory[0x000e4aul] = 9u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e44ul] == 1u && memory[0x000e4aul] == 0u);

    for(index = 0u; index < sizeof(operating_codes); ++index)
    {
        Reset();
        memory[0x000f05ul] = 2u;
        memory[0x000e43ul] = operating_codes[index];
        memory[0x000e49ul] = 0u;
        memory[0x001037ul] = 4u;
        UpdatePrimaryRawReadOperatingCodeSelector();
        assert(memory[0x000e49ul] == expected_codes[index] && memory[0x000e44ul] == 5u && memory[0x000f05ul] == 3u && memory[0x001037ul] == 5u);
    }

    Reset();
    memory[0x000f05ul] = 2u;
    memory[0x000e43ul] = 0x34u;
    memory[0x000e49ul] = 0u;
    UpdatePrimaryRawReadOperatingCodeSelector();
    assert(memory[0x000e49ul] == 0x28u && memory[0x000e44ul] == 1u);
}

int main(void)
{
    CheckEntryGatesAndWordClears();
    CheckStateOneSelection();
    CheckStateThreeCountdownAndBackoff();
    CheckSynchronizedAndAdvancePaths();
    return 0;
}
