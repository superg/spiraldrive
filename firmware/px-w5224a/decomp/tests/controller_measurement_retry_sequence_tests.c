#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_retry_sequence.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 pass_results[6];
static u8 pass_count;
static u8 acquisition_result;
static u8 acquisition_count;
static u8 calibration_count;
static u8 clear_count;
static u8 yield_count;
static u8 set_request_during_yield;
static u8 pin_rx;
static u8 pin_tx;
static u8 pin_write_count;

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

u8 ClearControllerMeasurementSeries(void)
{
    ++clear_count;
    return 2u;
}

u8 ExecuteControllerMeasurementPassAndClassifyResult(void)
{
    return pass_results[pass_count++];
}

u8 AcquireDiscReadPositionWithControllerReinitializationRetry(void)
{
    ++acquisition_count;
    return acquisition_result;
}

u8 ExecuteReadAcquisitionCalibrationForMode2(void)
{
    ++calibration_count;
    return 1u;
}

void YieldCoroutine1(void)
{
    ++yield_count;
    if(set_request_during_yield != 0u)
        memory[0x000e3ful] = 1u;
}

void SetMeasurementRetryP3RxHighTxLow(void)
{
    pin_rx = 1u;
    pin_tx = 0u;
    ++pin_write_count;
}

void SetMeasurementRetryP3RxLowTxHigh(void)
{
    pin_rx = 0u;
    pin_tx = 1u;
    ++pin_write_count;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(pass_results, 2, sizeof(pass_results));
    pass_count = acquisition_count = calibration_count = clear_count = 0u;
    yield_count = set_request_during_yield = pin_write_count = 0u;
    acquisition_result = 1u;
    pin_rx = pin_tx = 0xffu;
    memory[0x001302ul] = 0xa5u;
}

static void CheckInitializationAndEntryGate(void)
{
    Reset();
    memory[0x00ef72ul] = 0x38u;
    memory[0x00ef73ul] = 0xa3u;
    memory[0x000e3ful] = 1u;
    memory[0x000faful] = 5u;
    RunMediaState6ControllerMeasurementRetrySequence();
    assert(clear_count == 1u && pass_count == 0u && pin_write_count == 0u);
    assert(memory[0x00faabul] == 0x11u && memory[0x0015b2ul] == 2u);
    assert(memory[0x000e4bul] == 1u && memory[0x00ef78ul] == 0u && memory[0x00ef77ul] == 0u);
    assert(memory[0x00ef72ul] == 8u && (memory[0x00ef73ul] & 7u) == 4u);
    assert(memory[0x000e47ul] == 4u && memory[0x000c38ul] == 4u);
    assert(memory[0x000e3ful] == 0u);
}

static void CheckModeSixAndInitialPins(void)
{
    Reset();
    memory[0x00ef73ul] = 0xa1u;
    memory[0x00fb88ul] = 3u;
    memory[0x000c8aul] = 8u;
    memory[0xf7c01ful] = 0u;
    RunMediaState6ControllerMeasurementRetrySequence();
    assert((memory[0x00ef73ul] & 7u) == 6u);
    assert(pin_write_count == 2u && pin_rx == 1u && pin_tx == 0u);
    assert((memory[0x001302ul] & 0x10u) == 0u);
}

static void CheckSuccessRoutes(void)
{
    Reset();
    pass_results[0] = 1u;
    acquisition_result = 0u;
    RunMediaState6ControllerMeasurementRetrySequence();
    assert(pass_count == 1u && acquisition_count == 1u && yield_count == 0u);
    assert(pin_rx == 1u && pin_tx == 0u && (memory[0x001302ul] & 0x10u) == 0u);

    Reset();
    pass_results[0] = 1u;
    RunMediaState6ControllerMeasurementRetrySequence();
    assert(yield_count == 1u && memory[0x00faabul] == 0x12u);
    assert(ReadDword(0x000e3bul) == 0x96ul && memory[0x000faful] == 3u);

    Reset();
    pass_results[0] = 1u;
    set_request_during_yield = 1u;
    RunMediaState6ControllerMeasurementRetrySequence();
    assert(memory[0x000e3ful] == 0u && memory[0x000faful] == 0u);
    assert(ReadDword(0x000e3bul) == 0u);
}

static void CheckFailureAndExhaustion(void)
{
    Reset();
    memory[0x00ef72ul] = 8u;
    memory[0x00ef7eul] = 7u;
    pass_results[0] = 2u;
    RunMediaState6ControllerMeasurementRetrySequence();
    assert(calibration_count == 1u && memory[0x00ef74ul] == 1u);
    assert(memory[0x00faabul] == 7u && memory[0x000e44ul] == 1u);
    assert(yield_count == 1u);

    Reset();
    memory[0x00ef72ul] = 8u;
    memset(pass_results, 3, sizeof(pass_results));
    RunMediaState6ControllerMeasurementRetrySequence();
    assert(pass_count == 5u && memory[0x00f076ul] == 5u);
    assert(calibration_count == 1u && memory[0x00ef74ul] == 1u);
    assert(memory[0x00faabul] == 0x1eu && memory[0x000e44ul] == 1u);
    assert(ReadDword(0x000e3bul) == 0x96ul && memory[0x000faful] == 3u);
    assert(yield_count == 1u && pin_rx == 1u && pin_tx == 0u);
}

int main(void)
{
    CheckInitializationAndEntryGate();
    CheckModeSixAndInitialPins();
    CheckSuccessRoutes();
    CheckFailureAndExhaustion();
    return 0;
}
