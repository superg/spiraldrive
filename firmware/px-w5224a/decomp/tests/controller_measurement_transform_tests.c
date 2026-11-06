#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_transform.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 transform_calls;
static u8 transform_mode6;

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

void TransformControllerMeasurementSeries04To05AndDerive(u8 mode6)
{
    ++transform_calls;
    transform_mode6 = mode6;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    transform_calls = 0u;
    transform_mode6 = 0xffu;
    WriteWord(0x00f003ul, 151u);
    WriteWord(0x00f009ul, 200u);
    WriteWord(0x00f01bul, 200u);
    memory[0x00ef77ul] = 0u;
    memory[0x00ef78ul] = 50u;
}

static void CheckSuccessfulTransform(void)
{
    Reset();
    TransformControllerMeasurementSeries04To05();
    assert(transform_calls == 1u && transform_mode6 == 0u);
    assert(memory[0x00ef80ul] == 0u);
}

static void CheckHighRetry(void)
{
    Reset();
    WriteWord(0x00f009ul, 201u);
    TransformControllerMeasurementSeries04To05();
    assert(memory[0x00ef80ul] == 4u);
    assert(memory[0x00ef77ul] == 0xfcu);
    assert(memory[0x00ef78ul] == 150u);
    assert(transform_calls == 0u);

    Reset();
    WriteWord(0x00f009ul, 201u);
    memory[0x00ef77ul] = 0xffu;
    TransformControllerMeasurementSeries04To05();
    assert(memory[0x00ef80ul] == 4u);
    assert(memory[0x00ef77ul] == 0xffu);
    assert(memory[0x00ef78ul] == 50u);
}

static void CheckLowRetry(void)
{
    Reset();
    WriteWord(0x00f003ul, 150u);
    WriteWord(0x00f01bul, 199u);
    TransformControllerMeasurementSeries04To05();
    assert(memory[0x00ef80ul] == 3u);
    assert(memory[0x00ef77ul] == 4u);
    assert(memory[0x00ef78ul] == (u8)-50);
    assert(transform_calls == 0u);

    Reset();
    WriteWord(0x00f003ul, 150u);
    WriteWord(0x00f01bul, 199u);
    memory[0x00ef77ul] = 1u;
    TransformControllerMeasurementSeries04To05();
    assert(memory[0x00ef80ul] == 3u);
    assert(memory[0x00ef77ul] == 1u);
    assert(memory[0x00ef78ul] == 50u);
}

int main(void)
{
    CheckSuccessfulTransform();
    CheckHighRetry();
    CheckLowRetry();
    return 0;
}
