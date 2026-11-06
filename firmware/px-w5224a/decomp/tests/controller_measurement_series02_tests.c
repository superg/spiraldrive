#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_measurement_series02.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul], validate_result, validate_index;
u8 FirmwareRead8(fw_addr_t a)
{
    assert(a < sizeof(memory));
    return memory[a];
}
void FirmwareWrite8(fw_addr_t a, u8 v)
{
    assert(a < sizeof(memory));
    memory[a] = v;
}
s32 DivideS32Quotient(s32 a, s32 b)
{
    assert(b == 3l);
    return a / b;
}
u8 ValidateControllerMeasurementSeries02SelectionIndex(u8 i)
{
    validate_index = i;
    return validate_result;
}
static void W(fw_addr_t a, u16 v)
{
    memory[a] = (u8)(v >> 8);
    memory[a + 1u] = (u8)v;
}
static u16 R(fw_addr_t a)
{
    return (u16)(((u16)memory[a] << 8) | memory[a + 1u]);
}
static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    validate_result = 1u;
    validate_index = 0xffu;
    memory[0xfb80ul] = 2u;
    memory[0xf83438ul] = 10u;
}
static void Linear(s16 start, s16 step)
{
    u8 i;
    for(i = 0u; i < 15u; ++i)
        W(0xefc5ul + (u16)i * 2u, (u16)(start + (s16)i * step));
}

static void CheckSmoothingAndSuccess(void)
{
    Reset();
    Linear(0, 100);
    W(0xf9a0ul, 150u);
    W(0xef89ul + 2u, 0x1234u);
    W(0xf83436ul, 0x1200u);
    memory[0xef77ul] = 9u;
    memory[0xef78ul] = 8u;
    SmoothAndEvaluateControllerMeasurementSeries02();
    assert(R(0xefc5ul + 2u) == 100u && R(0xefc5ul + 26u) == 1300u);
    assert(validate_index == 1u && memory[0xef80ul] == 1u);
    assert(R(0xf06bul) == 0x1234u && memory[0xf06dul] == 0x34u && memory[0xf06aul] == 0x34u);
    assert(memory[0xef77ul] == 0u && memory[0xef78ul] == 0u);
}
static void CheckLowRetry(void)
{
    Reset();
    Linear(500, 0);
    W(0xf9a0ul, 0u);
    memory[0xef77ul] = 20u;
    SmoothAndEvaluateControllerMeasurementSeries02();
    assert(memory[0xef80ul] == 4u && memory[0xef77ul] == 12u && validate_index == 0xffu);
}
static void CheckHighRetry(void)
{
    Reset();
    Linear(5, 0);
    W(0xf9a0ul, 10u);
    memory[0xf83438ul] = 20u;
    memory[0xef77ul] = 1u;
    SmoothAndEvaluateControllerMeasurementSeries02();
    assert(memory[0xef80ul] == 3u && memory[0xef77ul] == 9u);
}
static void CheckInvalidSelection(void)
{
    Reset();
    Linear(0, 100);
    W(0xf9a0ul, 150u);
    validate_result = 0u;
    SmoothAndEvaluateControllerMeasurementSeries02();
    assert(validate_index == 1u && memory[0xef80ul] == 2u);
}
int main(void)
{
    CheckSmoothingAndSuccess();
    CheckLowRetry();
    CheckHighRetry();
    CheckInvalidSelection();
    puts("controller_measurement_series02_tests: ok");
    return 0;
}
