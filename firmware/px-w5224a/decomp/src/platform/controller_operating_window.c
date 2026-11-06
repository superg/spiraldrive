#include "controller_operating_window.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_window_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fb9624 */
u8 IsControllerReadPathWithinOperatingWindow(u8 use_wide_window)
{
    float base_threshold;
    s16 threshold;
    s16 sample;

    if(FirmwareReadBit0031() == 0u)
        return (u8)((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018032) & 0x20u) == 0u);

    sample = CAPTURE_OPERATING_SAMPLE();
    base_threshold = 14794.92F / (float)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51);
    if(use_wide_window == 1u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51) <= 0x10u)
            threshold = 0x06e4;
        else
            threshold = (s16)(base_threshold * 3.0F);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51) <= 0x10u)
        threshold = 0x003a;
    else
        threshold = (s16)(base_threshold * 0.1F);

    return (u8)(sample < threshold && sample > (s16)-threshold);
}
