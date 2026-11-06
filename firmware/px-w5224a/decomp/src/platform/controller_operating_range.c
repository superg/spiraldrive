#include "controller_measurement_profile_initialization.h"
#include "controller_operating_code_interpolation_tables.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f9e2ca */
void PLEXTOR_NEAR_FUNCTION BuildControllerOperatingCodeRange14To18(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1058, 0x14u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1059, 0x18u);
    BuildControllerOperatingCodeInterpolationTables();
}

/* RAM:f9d45c */
void PLEXTOR_NEAR_FUNCTION RouteControllerOperatingCodeRange(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) == 0x0au)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1058, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1059, 0x0au);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 1u) != 0u)
            BuildControllerOperatingCodeInterpolationTables();
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 0x18u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 1u) != 0u)
            BuildControllerOperatingCodeRange14To18();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1058, 0x14u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1059, 0x18u);
    }
}

/* RAM:f9536c */
void PLEXTOR_NEAR_FUNCTION SelectControllerMeasurementPassByEf72Bit5(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x20u) == 0u)
        RUN_CONTROLLER_MEASUREMENT_PASS0();
    else
        RUN_CONTROLLER_MEASUREMENT_PASS1();
}
