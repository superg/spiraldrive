#include "controller_threshold_probe.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "status_mapping.h"
#include "write_speed_reduction.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_threshold_probe_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadControllerThresholdWriteSpeedFlags(void)
{
    return (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_FAD7);
}

static u8 IsControllerThresholdWriteSpeedEligible(void)
{
    if((ReadControllerThresholdWriteSpeedFlags() & 0x0001u) != 0u && (ReadControllerThresholdWriteSpeedFlags() & 0x0002u) != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        return 1u;
    return (u8)((ReadControllerThresholdWriteSpeedFlags() & 0x1000u) != 0u);
}

static u8 ProbeControllerThreshold(u8 command, fw_addr_t result_address, u8 threshold)
{
    u8 result;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, command);
    WaitTimer1ScaledDuration(2000u, 1u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0x40u);
    result = WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180C7);
    FirmwareWrite8(result_address, result);
    return (u8)(result >= threshold);
}

/* RAM:f8fbe1 */
u8 ProbeControllerThresholdsAndMaybeReduceWriteSpeed(void)
{
    u8 threshold_exceeded = 0u;
    u8 second_threshold = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) == 0x37u ? 0x68u : 0x50u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u
        && READ_CONTROLLER_THRESHOLD_BIT31() == 0u)
    {
        threshold_exceeded = ProbeControllerThreshold(0x14u, 0x000f9a9ul, 0x50u);
        if(threshold_exceeded == 0u)
            threshold_exceeded = ProbeControllerThreshold(0x16u, 0x000f9aaul, second_threshold);

        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 0u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        if(IsIndexedDriveStateActive(4u) != 0u)
            threshold_exceeded = 0u;

        if(threshold_exceeded != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u && IsControllerThresholdWriteSpeedEligible() != 0u)
        {
            AccumulateWriteSpeedFlagsFromPositionAndOperatingCode();
            TryReduceWriteSpeedForCurrentMediaState(IsControllerThresholdWriteSpeedEligible());
            return 0u;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) | 0x10u));
    return 1u;
}
