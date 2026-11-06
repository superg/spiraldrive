#include "operating_code14_measurement_bounds.h"
#include "../media/controller_measurement_dispatch.h"
#include "firmware_memory.h"
#include "operating_code_measurement_bounds.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/operating_code14_measurement_bounds_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadOperatingCode14Word(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteOperatingCode14Word(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fa6560 */
void ComputeOperatingCode14ControllerMeasurementBounds(void)
{
    u8 selector;
    s16 position_delta;
    s16 scale;
    s16 reference_offset;
    double initial_value;
    double denominator;
    double transformed_value;
    float scaled_value;
    u16 first_bound;

    position_delta = (s16)(ReadOperatingCode14Word(FW_NEAR_UNKNOWN_EF87) - ReadOperatingCode14Word(FW_NEAR_UNKNOWN_F074));
    initial_value = (double)position_delta * 0.0043;
    initial_value = initial_value - 4.6592;
    scaled_value = (float)initial_value;
    selector = FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F);
    scale = (s16)ReadOperatingCode14Word(FirmwareAddressIncrement24(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8390F), (u16)selector * 4u));
    scaled_value = scaled_value * (float)scale;
    scaled_value = scaled_value / 100.0f;
    first_bound = UPDATE_MEASUREMENT_BOUND(scaled_value);
    WriteOperatingCode14Word(FW_NEAR_UNKNOWN_EF83, first_bound);

    position_delta = (s16)(ReadOperatingCode14Word(FW_NEAR_UNKNOWN_EF87) - ReadOperatingCode14Word(FW_NEAR_UNKNOWN_F074));
    denominator = (double)position_delta * 0.0089;
    denominator = denominator + 14.3;
    denominator = denominator * 0.01;
    reference_offset = (s16)(ReadOperatingCode14Word(FW_NEAR_UNKNOWN_EF85) - (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A));
    transformed_value = denominator * (double)reference_offset;
    position_delta = (s16)(ReadOperatingCode14Word(FW_NEAR_UNKNOWN_EF87) - ReadOperatingCode14Word(FW_NEAR_UNKNOWN_F074));
    transformed_value = transformed_value + (double)position_delta * 0.001;
    transformed_value = transformed_value - 12.0;
    scaled_value = (float)transformed_value;
    selector = FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F);
    scale = (s16)ReadOperatingCode14Word(FirmwareAddressIncrement24(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8390D), (u16)selector * 4u));
    scaled_value = scaled_value * (float)scale;
    scaled_value = scaled_value / 100.0f;
    WriteOperatingCode14Word(FW_NEAR_UNKNOWN_EF81, COMPUTE_BOUNDED_ADJUSTMENT(scaled_value, ReadOperatingCode14Word(FW_NEAR_UNKNOWN_EF83)));
}
