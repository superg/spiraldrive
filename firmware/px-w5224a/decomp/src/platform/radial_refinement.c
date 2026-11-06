#include "radial_refinement.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "controller_position_average.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "radial_position_table.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/radial_refinement_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRefinementU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRefinementU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fcf1d4 */
void PLEXTOR_NEAR_FUNCTION RefineRadialCalibrationAtFrame150(void)
{
    u8 delay_before_measurement = 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 5u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        WriteRefinementU32(FW_NEAR_UNKNOWN_0E3B, 150ul);
        if(RefinementAcquire() == 1u)
            break;
        if(RefinementReset() == 0u)
            return;
    }

    WriteRefinementU32(FW_NEAR_UNKNOWN_FC33, ReadRefinementU32(FW_NEAR_UNKNOWN_0CF5));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 7u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        WriteRefinementU32(FW_NEAR_UNKNOWN_0E3B, 150ul);
        RefinementOperatingEnable(0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 1u);
        if(RefinementAcquire() != 1u)
        {
            if(RefinementReset() == 0u)
                break;
            continue;
        }
        if(delay_before_measurement != 0u)
        {
            delay_before_measurement = 0u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 10u);
            while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) >= 0)
                RefinementYield();
            continue;
        }
        if(RefinementTarget(150l) == 1u)
        {
            u32 old_scale;
            u32 new_scale;
            u32 ratio;
            u8 classification;
            volatile double adjusted_scale;

            RefinementSetMeasurement(RefinementMeasure());
            old_scale = ReadRefinementU32(FW_NEAR_UNKNOWN_FC33);
            new_scale = ReadRefinementU32(FW_NEAR_UNKNOWN_0CF5);
            ratio = DivideU32QuotientNative(MultiplyU32Low(100ul, old_scale), new_scale);
            if(ratio < 64ul)
                classification = 0x3cu;
            else if(ratio < 74ul)
                classification = 0x46u;
            else if(ratio < 84ul)
                classification = 0x50u;
            else if(ratio < 114ul)
                classification = 0x64u;
            else if(ratio < 124ul)
                classification = 0x78u;
            else if(ratio < 134ul)
                classification = 0x82u;
            else
                classification = 0x8cu;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E8, classification);
            if(classification != 0x64u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) | 0x04u));
            adjusted_scale = (double)new_scale * 0.98;
            new_scale = (u32)adjusted_scale;
            WriteRefinementU32(FW_NEAR_UNKNOWN_0CF5, new_scale);
            WriteRefinementU32(FW_NEAR_UNKNOWN_FC37, new_scale);
            RefinementRebuild();
            break;
        }
    }
    RefinementOperatingEnable(1u);
}
