#include "controller_measurement_transform.h"
#include "controller_measurement_mode6_transform.h"
#include "firmware_memory.h"

static u16 ReadMeasurementTransformWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* RAM:f9678f */
void PLEXTOR_NEAR_FUNCTION TransformControllerMeasurementSeries04To05(void)
{
    if((s16)ReadMeasurementTransformWord(FW_NEAR_UNKNOWN_F003) > 150 && (s16)ReadMeasurementTransformWord(FW_NEAR_UNKNOWN_F009) > 200)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 4u);
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77) >= 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF77, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77) - 4u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF78, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF78) + 100u));
        }
        return;
    }

    if((s16)ReadMeasurementTransformWord(FW_NEAR_UNKNOWN_F01B) < 200)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 3u);
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77) < 1)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF77, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77) + 4u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF78, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF78) - 100u));
        }
        return;
    }

    TransformControllerMeasurementSeries04To05AndDerive(0u);
}
