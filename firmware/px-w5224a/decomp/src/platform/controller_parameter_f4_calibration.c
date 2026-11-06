#include "controller_parameter_f4_calibration.h"
#include "firmware_memory.h"
#include "mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_parameter_f4_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fca1cc */
void CalibrateControllerParameterF4(void)
{
    u16 sample_sum;
    u8 sample_index;
    u8 average;
    u8 candidate;

    CONFIGURE_CALIBRATION(0x74u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC14, 0xd8u);
    APPLY_INITIAL_F4();
    WAIT_CALIBRATION(7000u, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 10u);

    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) >= 0)
    {
        candidate = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14);
        WRITE_PARAMETER((u16)(0xf400u | candidate));
        if(candidate == 0xfcu || candidate == 0u)
            break;

        WAIT_CALIBRATION(400u, 1u);
        sample_sum = 0u;
        for(sample_index = 0u; sample_index != 4u; ++sample_index)
        {
            sample_sum = (u16)(sample_sum + MEASURE_CALIBRATION());
            WAIT_CALIBRATION(100u, 1u);
        }
        average = (u8)(sample_sum >> 2);
        if(average >= 91u && average <= 107u)
            break;
        if(average > 107u)
            candidate = (u8)(candidate - 0x24u);
        else
            candidate = (u8)(candidate + 0x24u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC14, candidate);
    }

    RESTORE_PARAMETERS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FACA, 0xd8u);
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) >= 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FACA, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14));
}
