#include "controller_threshold_sampling.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_threshold_sampling_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadThresholdSamplingFlags(void)
{
    return (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_FAD7);
}

static s32 ReadThresholdSamplingPosition(void)
{
    u32 value = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1235) << 24;

    value |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1236) << 16;
    value |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1237) << 8;
    value |= FirmwareReadNear8(FW_NEAR_UNKNOWN_1238);
    return (s32)value;
}

static u8 IsThresholdSamplingWriteSpeedEligible(void)
{
    if((ReadThresholdSamplingFlags() & 1u) != 0u && (ReadThresholdSamplingFlags() & 2u) != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        return 1u;
    return (u8)((ReadThresholdSamplingFlags() & 0x1000u) != 0u);
}

static u8 ReadThresholdSamplingControllerResult(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0x40u);
    return WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180C7);
}

static void ClearThresholdSamplingControllerCommand(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
}

static u8 SelectPhase1Threshold(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 0x28u)
        return ReadThresholdSamplingPosition() < 270000l ? 0x28u : 0x26u;
    return ReadThresholdSamplingPosition() < 270000l ? 0x30u : 0x2eu;
}

static u8 SelectPhase2Threshold(void)
{
    u8 threshold;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 0x28u)
        return ReadThresholdSamplingPosition() < 270000l ? 0x50u : 0x4cu;
    threshold = ReadThresholdSamplingPosition() < 270000l ? 0x58u : 0x54u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) == 0x37u)
        threshold = (u8)(threshold - 4u);
    return threshold;
}

static void UpdateThresholdSamplingMaximum(fw_addr_t address, u8 sample)
{
    u8 maximum = FirmwareRead8(address);

    if(maximum == 0u || sample >= maximum)
        FirmwareWrite8(address, sample);
}

/* RAM:f8f8e1 */
void ServiceControllerOperatingThresholdSamplingCycle(void)
{
    u8 threshold_latched = 0u;

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121C) <= 0 && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u)
    {
        u8 state;

        THRESHOLD_SAMPLING_INTERRUPT(0u);
        state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0);
        if((state & 1u) != 0u)
        {
            u8 phase_bits = (u8)((state + 2u) & 6u);
            u8 phase = (u8)(phase_bits >> 1);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)((state & 0xf9u) | phase_bits));
            if(phase == 0u)
            {
                WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
                WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0x14u);
            }
            else if(phase == 1u)
            {
                u8 sample = ReadThresholdSamplingControllerResult();
                u8 count;

                ClearThresholdSamplingControllerCommand();
                if(sample >= SelectPhase1Threshold())
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCE) + 1u));
                count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCE);
                if(count >= 3u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD6) | 4u));
                    threshold_latched = 1u;
                }
                else
                {
                    UpdateThresholdSamplingMaximum(FW_NEAR_UNKNOWN_0FCB, sample);
                    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
                    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0x16u);
                }
            }
            else if(phase == 2u)
            {
                u8 sample = ReadThresholdSamplingControllerResult();
                u8 count;

                ClearThresholdSamplingControllerCommand();
                if(sample >= SelectPhase2Threshold())
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCF) + 1u));
                count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCF);
                if(count >= 3u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD6) | 2u));
                    threshold_latched = 1u;
                }
                else
                {
                    UpdateThresholdSamplingMaximum(FW_NEAR_UNKNOWN_0FCC, sample);
                    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
                    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 6u);
                }
            }
            else
            {
                WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCD, WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180C7));
                ClearThresholdSamplingControllerCommand();
            }

            if(IsIndexedDriveStateActive(4u) != 0u && ReadThresholdSamplingPosition() <= 315000l)
                threshold_latched = 0u;
            if(threshold_latched != 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCF, 0u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCE, 0u);
                }
                else if(IsThresholdSamplingWriteSpeedEligible() != 0u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCF, 0u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCE, 0u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) | 8u));
                    AccumulateWriteSpeedFlagsFromPositionAndOperatingCode();
                }
            }
        }
        THRESHOLD_SAMPLING_INTERRUPT(1u);
        THRESHOLD_SAMPLING_INTERRUPT(2u);
        THRESHOLD_SAMPLING_INTERRUPT(3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121C, 10u);
    }
}
