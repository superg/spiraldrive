#include "controller_position_average.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

extern void YieldCoroutine1(void);
extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);

static void WritePositionAverageU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u8 IsControllerVariationMeasurementEnabled(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC32) & 1u);
}

/* RAM:fce2e1 */
u16 PLEXTOR_NEAR_FUNCTION MeasureAverageControllerPositionWord(void)
{
    u32 sample_sum = 0ul;
    s32 variation_sum = 0l;
    s16 minimum = -1;
    s16 maximum = -1;
    u8 sample_index;

    for(sample_index = 0u; sample_index < 10u; ++sample_index)
    {
        u16 sample;

        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180CC, 0x12u);
        while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180CC) & 0x10u) != 0u)
        {
            if(IsControllerVariationMeasurementEnabled() != 0u && sample_index >= 5u)
            {
                s16 word = (s16)ReadControllerWord(0x0233u);

                if(minimum == -1 && maximum == -1)
                {
                    minimum = word;
                    maximum = word;
                }
                else
                {
                    if(maximum <= word)
                        maximum = word;
                    if(word <= minimum)
                        minimum = word;
                }
            }
        }

        sample = (u16)((u16)WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180CF) << 8);
        sample = (u16)(sample | WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180CE));
        sample_sum += sample;

        if(IsControllerVariationMeasurementEnabled() != 0u && sample_index >= 5u)
        {
            s16 variation = (s16)((u16)maximum - (u16)minimum);

            variation_sum += variation;
            minimum = -1;
            maximum = -1;
        }
        YieldCoroutine1();
    }

    if(IsControllerVariationMeasurementEnabled() != 0u)
    {
        u16 variation_metric = (u16)DivideS32Quotient(variation_sum, 5l);

        WritePositionAverageU16(FW_NEAR_UNKNOWN_FC30, variation_metric);
        if(variation_metric >= 0x48u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC32, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC32) | 2u));
    }

    return (u16)DivideU32Quotient(sample_sum, 10ul);
}
