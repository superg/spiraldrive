#include "controller_d8d9_window.h"
#include "arithmetic.h"
#include "firmware_memory.h"

extern void PLEXTOR_NEAR_FUNCTION SetBit0719Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0719Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit070FAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit070FAtomic(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_d8d9_window_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadD8D9WindowU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteD8D9WindowU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u16 SelectD8D9CaptureInitialDelay(u16 *sample_interval)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

    if(operating_code >= 8u)
    {
        *sample_interval = 2u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 1u) == 0u)
            return 80u;
        return 1u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 4u)
    {
        *sample_interval = 2u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 1u) == 0u)
            return 190u;
        return 1u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 2u)
    {
        *sample_interval = 15u;
        return 400u;
    }
    *sample_interval = 20u;
    return 1000u;
}

static void CaptureAndSortD8D9Window(u16 initial_delay, u16 sample_interval)
{
    u8 index;

    WaitTimer1ScaledDuration(initial_delay, 0u);
    FirmwareWriteDirectSfr8(E1, 0u);
    SetBit0719Atomic();
    SetBit070FAtomic();
    for(index = 0u; index < 10u; ++index)
    {
        do
        {
            while((FirmwareReadDirectSfr8(F1) & 1u) == 0u)
            {
            }
        } while((FirmwareReadDirectSfr8(F1) & 2u) == 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9B7 + index, FirmwareReadDirectSfr8(D8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9CB + index, FirmwareReadDirectSfr8(D9));
        if(index < 9u)
            WaitTimer1ScaledDuration(sample_interval, 0u);
    }
    ClearBit070FAtomic();
    ClearBit0719Atomic();
    SortByteArrayAscendingInPlace(FW_NEAR_UNKNOWN_F9B7, 10u);
    SortByteArrayAscendingInPlace(FW_NEAR_UNKNOWN_F9CB, 10u);
}

/* RAM:fb9838 */
s16 CaptureControllerD8D9WindowAndUpdateAdjustment(void)
{
    u16 initial_delay;
    u16 sample_interval;
    u16 d8_sum = 0u;
    u16 d9_sum = 0u;
    u16 d8_average;
    u16 d9_average;
    s16 d8_delta;
    s16 d9_gap;
    s16 numerator;
    s16 denominator;
    u8 metric;
    u8 index;
    s32 scaled_numerator;

    initial_delay = SelectD8D9CaptureInitialDelay(&sample_interval);
    CaptureAndSortD8D9Window(initial_delay, sample_interval);

    for(index = 3u; index < 7u; ++index)
    {
        d8_sum = (u16)(d8_sum + FirmwareReadNear8(FW_NEAR_UNKNOWN_F9B7 + index));
        d8_sum = (u16)(d8_sum + 0x78u);
        d9_sum = (u16)(d9_sum + 0x11u);
        d9_sum = (u16)(d9_sum + FirmwareReadNear8(FW_NEAR_UNKNOWN_F9CB + index));
    }
    d8_average = d8_sum >> 2;
    d9_average = d9_sum >> 2;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
    {
        u8 phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED);

        WriteD8D9WindowU16(FW_NEAR_UNKNOWN_EFC5 + (fw_addr_t)((u16)phase * 2u), d8_average);
        phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED);
        WriteD8D9WindowU16(FW_NEAR_UNKNOWN_EFE3 + (fw_addr_t)((u16)phase * 2u), d9_average);
    }

    d8_delta = (s16)(u16)(d8_average - ReadD8D9WindowU16(FW_NEAR_UNKNOWN_F9A0));
    d9_gap = (s16)(u16)(ReadD8D9WindowU16(FW_NEAR_UNKNOWN_F9A2) - d9_average);
    if(d8_delta <= 0 || d9_gap <= 0)
    {
        metric = 0u;
    }
    else
    {
        u16 combined = (u16)(d8_delta + d9_gap);

        metric = combined < 0x100u ? (u8)combined : 0xffu;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9A4, metric);

    if(metric <= 0x10u)
    {
        d8_delta = 1;
        d9_gap = 3;
    }
    numerator = (s16)(d8_delta - d9_gap);
    denominator = (s16)(d8_delta + d9_gap);
    scaled_numerator = (s32)MultiplyU32Low((u32)(s32)numerator, 1000ul);
    return (s16)DivideS32QuotientNative(scaled_numerator, (s32)denominator);
}
