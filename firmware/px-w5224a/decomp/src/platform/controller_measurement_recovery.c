#include "controller_measurement_recovery.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_recovery_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadMeasurementRecoveryU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteMeasurementRecoveryU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:f9599d */
void PLEXTOR_NEAR_FUNCTION RetryControllerMeasurementEvaluationWithReadPositionRecovery(void)
{
    u8 restart_timeout = 1u;
    u8 retry_count = 0u;

    for(;;)
    {
        u8 acquisition_result;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 8u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 4u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
            WriteMeasurementRecoveryU32(FW_NEAR_UNKNOWN_0E3B, ReadMeasurementRecoveryU32(FW_NEAR_UNKNOWN_103D) - 20ul);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 8u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 8u);
            WriteMeasurementRecoveryU32(FW_NEAR_UNKNOWN_0E3B, ReadMeasurementRecoveryU32(FW_NEAR_UNKNOWN_103D) - 40ul);
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        acquisition_result = RECOVERY_ACQUIRE_POSITION();

        if(acquisition_result == 0u)
        {
            if(retry_count >= 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 4u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
                return;
            }

            RECOVERY_RESET_DRIVE_CONTROL();
            if(RECOVERY_INITIALIZE_CONTROLLER() == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, 7u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 4u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
                return;
            }
            RECOVERY_SET_OPERATING_ENABLE(1u);
            retry_count++;
            continue;
        }

        if(acquisition_result == 1u)
        {
            if(RECOVERY_CHECK_TOLERANCE(restart_timeout) != 1u)
            {
                restart_timeout = 0u;
                continue;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        }

        RECOVERY_CONFIGURE_PORTS(0u);
        RECOVERY_APPLY_CHANNELS();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 0x01u));
        RECOVERY_WRITE_D4_E2(0x74u);
        RECOVERY_RAISE_F4();
        RECOVERY_POLL_PACKET();
        RECOVERY_RESTORE_D4_E2();
        RECOVERY_LOWER_F4();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xf7u));
        RECOVERY_EVALUATE_TAIL();

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 4u)
        {
            if(retry_count >= 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, 5u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
                return;
            }
            retry_count++;
            continue;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 3u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, 0x1eu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            return;
        }

        if(RECOVERY_EVALUATE_SERIES(restart_timeout) != 0u)
            return;
        retry_count++;
        if(retry_count >= 3u)
            return;
    }
}
