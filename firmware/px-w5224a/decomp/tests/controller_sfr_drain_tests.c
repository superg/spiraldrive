#include <assert.h>
#include <string.h>
#include "platform/controller_parameters.h"
#include "platform/firmware_memory.h"

static u8 sfr[256];
static u8 d8_samples[4], d9_samples[4];
static unsigned d8_index, d9_index, ready_reads;

u8 FirmwareReadSfr8(u8 address)
{
    if(address == 0xf1u)
    {
        ++ready_reads;
        return (ready_reads & 1u) != 0u ? 0u : 3u;
    }
    if(address == 0xd8u)
        return d8_samples[d8_index++];
    if(address == 0xd9u)
        return d9_samples[d9_index++];
    return sfr[address];
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    sfr[address] = value;
}

static void ResetState(void)
{
    memset(sfr, 0, sizeof(sfr));
    d8_index = d9_index = ready_reads = 0u;
    sfr[0xd1u] = 0xa5u;
    sfr[0xe1u] = 0x35u;
}

int main(void)
{
    ResetState();
    d8_samples[0] = 1u;
    d8_samples[1] = 2u;
    d8_samples[2] = 3u;
    d8_samples[3] = 4u;
    assert(DrainFourSfrD8OrD9Bytes(0u) == 490u);
    assert(d8_index == 4u && d9_index == 0u && ready_reads == 8u);
    assert(sfr[0xd1u] == 0xa5u && sfr[0xe1u] == 0u);

    ResetState();
    d9_samples[0] = 0x10u;
    d9_samples[1] = 0x20u;
    d9_samples[2] = 0x30u;
    d9_samples[3] = 0x40u;
    assert(DrainFourSfrD8OrD9Bytes(0xfeu) == 0xa0u);
    assert(d8_index == 0u && d9_index == 4u && ready_reads == 8u);
    assert(sfr[0xd1u] == 0xa5u && sfr[0xe1u] == 0u);
    return 0;
}
