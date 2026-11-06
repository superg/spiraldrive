#include "controller_measurement_capture.h"
#include "arithmetic.h"
#include "firmware_memory.h"

extern void PLEXTOR_NEAR_FUNCTION SetBit0719Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit070FAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit070FAtomic(void);
extern u16 PLEXTOR_NEAR_FUNCTION CaptureInheritedWR6ForMeasurementDuration(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_measurement_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadCaptureU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteCaptureU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fa78fd */
void AdvanceControllerD8D9MeasurementSeries(void)
{
    u8 sample_count = CAPTURE_D8D9_SERIES_INHERITED_COUNT();
    u8 phase;
    u8 index;
    u16 d8_total = 0u;
    u16 d9_total = 0u;
    u16 divisor;
    u16 d8_average;
    u16 d9_average;
    s16 d8_delta;
    s16 d9_gap;
    s16 numerator;
    s16 denominator;
    s32 quotient;

    OBSERVE_D8D9_SERIES_INVOCATION();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) >= 15u)
        return;
    phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75);

    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43))
    {
    case 4u:
        WAIT_D8D9_SERIES(120u, 0u);
        sample_count = 5u;
        break;
    case 8u:
        WAIT_D8D9_SERIES(50u, 0u);
        sample_count = 3u;
        break;
    case 2u:
        WAIT_D8D9_SERIES(300u, 0u);
        sample_count = 7u;
        break;
    default:
        break;
    }

    for(index = 0u; index < sample_count; ++index)
        CAPTURE_D8D9_SERIES(index);
    SORT_D8D9_SERIES(FW_NEAR_UNKNOWN_F1D4, sample_count);
    SORT_D8D9_SERIES(FW_NEAR_UNKNOWN_F1E2, sample_count);

    for(index = 1u; index < (u8)(sample_count - 1u); ++index)
    {
        d8_total = (u16)(d8_total + ReadCaptureU16(FW_NEAR_UNKNOWN_F1D4 + (u16)index * 2u));
        d9_total = (u16)(d9_total + ReadCaptureU16(FW_NEAR_UNKNOWN_F1E2 + (u16)index * 2u));
    }
    divisor = (u16)((u16)(sample_count - 2u) * 4u);
    d8_average = DivideU16QuotientNative(d8_total, divisor);
    d9_average = DivideU16QuotientNative(d9_total, divisor);
    WriteCaptureU16(FW_NEAR_UNKNOWN_EFC5 + (u16)phase * 2u, d8_average);
    WriteCaptureU16(FW_NEAR_UNKNOWN_EFE3 + (u16)phase * 2u, d9_average);

    d8_delta = (s16)(d8_average - ReadCaptureU16(FW_NEAR_UNKNOWN_F9A0));
    d9_gap = (s16)(ReadCaptureU16(FW_NEAR_UNKNOWN_F9A2) - d9_average);
    numerator = (s16)(d8_delta - d9_gap);
    denominator = (s16)(d8_delta + d9_gap);
    quotient = DivideS32QuotientNative((s32)MultiplyU32Low((u32)(s32)numerator, 1000ul), (s32)denominator);
    WriteCaptureU16(FW_NEAR_UNKNOWN_F03F + (u16)phase * 2u, (u16)quotient);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x40u) != 0u)
    {
        quotient = DivideS32QuotientNative((s32)MultiplyU32Low((u32)(s32)denominator, 1000ul), (s32)d8_average);
        if(quotient < 0l)
            quotient = 0l;
        WriteCaptureU16(FW_NEAR_UNKNOWN_F001 + (u16)phase * 2u, (u16)quotient);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) + 1u));
}

/* RAM:f97d5a */
u16 PLEXTOR_NEAR_FUNCTION SelectMeasurementDurationFromOperatingCode(void)
{
    u16 duration = CaptureInheritedWR6ForMeasurementDuration();

    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB82))
    {
    case 0u:
        return 500u;
    case 2u:
        return 600u;
    case 4u:
        return 750u;
    case 6u:
        return 900u;
    case 8u:
        return 1100u;
    case 10u:
        return 1350u;
    case 12u:
        return 1650u;
    case 14u:
        return 2000u;
    default:
        return duration;
    }
}

/* Reconstruction helper; no direct Ghidra function address. */
static void CaptureAndSortD8D9Samples(void)
{
    u8 index;

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
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1C0 + index, FirmwareReadDirectSfr8(D8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1CA + index, FirmwareReadDirectSfr8(D9));
    }
    ClearBit070FAtomic();
    SortByteArrayAscendingInPlace(FW_NEAR_UNKNOWN_F1C0, 10u);
    SortByteArrayAscendingInPlace(FW_NEAR_UNKNOWN_F1CA, 10u);
}

/* RAM:fa7ae1 */
void PLEXTOR_NEAR_FUNCTION CaptureControllerD8D9MiddleFourSumsForIndex(u8 series_index)
{
    fw_addr_t d8_sum_address = FW_NEAR_UNKNOWN_F1D4 + (fw_addr_t)((u16)series_index * 2u);
    fw_addr_t d9_sum_address = FW_NEAR_UNKNOWN_F1E2 + (fw_addr_t)((u16)series_index * 2u);
    u16 sum;
    u8 index;
    u8 sample;

    CaptureAndSortD8D9Samples();
    WriteCaptureU16(d8_sum_address, 0u);
    WriteCaptureU16(d9_sum_address, 0u);
    for(index = 3u; index < 7u; ++index)
    {
        sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_F1C0 + index);
        sum = ReadCaptureU16(d8_sum_address);
        WriteCaptureU16(d8_sum_address, (u16)(sum + sample));
        sum = ReadCaptureU16(d8_sum_address);
        WriteCaptureU16(d8_sum_address, (u16)(sum + 0x78u));

        sum = ReadCaptureU16(d9_sum_address);
        WriteCaptureU16(d9_sum_address, (u16)(sum + 0x11u));
        sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_F1CA + index);
        sum = ReadCaptureU16(d9_sum_address);
        WriteCaptureU16(d9_sum_address, (u16)(sum + sample));
    }
}

/* RAM:fa7bd7 */
void PLEXTOR_NEAR_FUNCTION CaptureControllerD8D9NormalizedDifferenceSample(void)
{
    u16 d8_sum = 0u;
    u16 d9_sum = 0u;
    s16 d8_delta;
    s16 d9_gap;
    s16 numerator;
    s16 divisor;
    s32 scaled_numerator;
    s32 quotient;
    u16 sample_index;
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 4u)
        WaitTimer1ScaledDuration(250u, 0u);
    else
        WaitTimer1ScaledDuration(DivideU16QuotientNative(1000u, (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43)), 0u);

    CaptureAndSortD8D9Samples();
    for(index = 3u; index < 7u; ++index)
    {
        d8_sum = (u16)(d8_sum + FirmwareReadNear8(FW_NEAR_UNKNOWN_F1C0 + index) + 0x78u);
        d9_sum = (u16)(d9_sum + 0x11u + FirmwareReadNear8(FW_NEAR_UNKNOWN_F1CA + index));
    }

    d8_delta = (s16)((u16)(d8_sum >> 2) - ReadCaptureU16(FW_NEAR_UNKNOWN_F9A0));
    d9_gap = (s16)(ReadCaptureU16(FW_NEAR_UNKNOWN_F9A2) - (u16)(d9_sum >> 2));
    numerator = (s16)((u16)d8_delta - (u16)d9_gap);
    divisor = (s16)((u16)d8_delta + (u16)d9_gap);
    scaled_numerator = (s32)MultiplyU32Low((u32)(s32)numerator, 1000ul);
    quotient = DivideS32QuotientNative(scaled_numerator, (s32)divisor);

    sample_index = ReadCaptureU16(FW_NEAR_UNKNOWN_F03D);
    WriteCaptureU16(FW_NEAR_UNKNOWN_F03F + (fw_addr_t)(u16)(sample_index << 1), (u16)quotient);
}
