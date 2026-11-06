#include "media_state6_lead_in_reference.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/media_state6_lead_in_reference_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadLeadInReferenceU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteLeadInReferenceU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void PublishLeadInReferenceFailure(u8 result, u8 clear_busy)
{
    if(clear_busy != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, result);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, result);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
}

/* RAM:f94dc0 */
void PLEXTOR_NEAR_FUNCTION AcquireMediaState6LeadInMeasurementReference(void)
{
    u8 restart_timeout = 1u;
    u8 attempt_count = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 1u);
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 0u);

    for(;;)
    {
        u8 acquisition_result;
        u8 capture_result;
        u32 target = ReadLeadInReferenceU32(FW_NEAR_UNKNOWN_114F) - 0x047eul;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
        WriteLeadInReferenceU32(FW_NEAR_UNKNOWN_0E3B, target);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        acquisition_result = AcquireDiscReadPosition();

        if(acquisition_result == 0u)
        {
            if(attempt_count >= 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 4u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
                return;
            }
            ResetDriveControlStateToEight();
            if(InitializeControllerOperatingStateAndRunCalibration() == 0u)
            {
                PublishLeadInReferenceFailure(7u, 1u);
                return;
            }
            SetControllerOperatingEnableState(1u);
            ++attempt_count;
            continue;
        }

        if(acquisition_result == 1u)
        {
            if(CheckControllerMeasurementToleranceOrTimeout(restart_timeout) == 0u)
            {
                restart_timeout = 0u;
                continue;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        }

        ConfigureControllerPortsAndParametersForCurrentOperatingCode(0u);
        ApplyOperatingCodeAndFixedControllerChannels0Through2();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
        capture_result = CaptureMediaState6LeadInMeasurementOffset();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));

        if(capture_result == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 1u);
            return;
        }
        if(capture_result != 0u)
            continue;
        if(attempt_count >= 3u)
        {
            PublishLeadInReferenceFailure(5u, 0u);
            return;
        }
        ++attempt_count;
    }
}
