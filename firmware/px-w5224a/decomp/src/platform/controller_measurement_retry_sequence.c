#include "controller_measurement_retry_sequence.h"
#include "controller_mapping.h"
#include "controller_measurement_pass.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

extern void YieldCoroutine1(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_retry_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteMeasurementRetryTarget(u32 target)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3B, (u8)(target >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3C, (u8)(target >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3D, (u8)(target >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3E, (u8)target);
}

static void SetMeasurementRetryIdlePins(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
    SetMeasurementRetryP3RxHighTxLow();
}

static void RecalibrateModeFourIfNeeded(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 4u)
    {
        (void)ExecuteReadAcquisitionCalibrationForMode2();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, 1u);
    }
}

/* RAM:f949ba */
void RunMediaState6ControllerMeasurementRetrySequence(void)
{
    u8 attempt = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x11u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15B2, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF78, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF77, 0u);
    (void)ClearControllerMeasurementSeries();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F076, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08B, 0u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 8u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF73, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 0xf8u) | 4u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 4u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF73, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 0xf8u) | 2u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF73, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 0xf8u) | 6u));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 6u)
            return;
    }

    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 8u) != 0u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
        SetMeasurementRetryP3RxHighTxLow();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x10u));
        SetMeasurementRetryP3RxLowTxHigh();
    }

    for(;;)
    {
        u8 result = ExecuteControllerMeasurementPassAndClassifyResult();

        if(result == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            if(AcquireDiscReadPositionWithControllerReinitializationRetry() == 0u)
            {
                SetMeasurementRetryIdlePins();
                return;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x12u);
            YieldCoroutine1();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
                SetMeasurementRetryIdlePins();
                return;
            }
            WriteMeasurementRetryTarget(0x96ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            SetMeasurementRetryIdlePins();
            return;
        }

        if(result != 3u)
        {
            RecalibrateModeFourIfNeeded();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7E));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            SetMeasurementRetryIdlePins();
            YieldCoroutine1();
            return;
        }

        ++attempt;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F076, attempt);
        if(attempt < 5u)
            continue;

        RecalibrateModeFourIfNeeded();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x1eu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        WriteMeasurementRetryTarget(0x96ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        SetMeasurementRetryIdlePins();
        YieldCoroutine1();
        return;
    }
}
