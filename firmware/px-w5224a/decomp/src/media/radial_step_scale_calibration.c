#include "radial_step_scale_calibration.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/radial_step_scale_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRadialScaleU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadRadialScaleU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRadialScaleU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fc88fa */
void AccumulateRadialStepScaleCalibrationSample(void)
{
    u8 drive_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF);

    if((drive_flags & 0x80u) == 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) >> 3) & 1u) != 1u)
    {
        u16 reference_value = ReadRadialScaleU16(FW_NEAR_UNKNOWN_0ED3);

        if(reference_value != 0xffffu)
        {
            s32 frame_seconds = RADIAL_SCALE_DIVIDE_S32((s32)ReadRadialScaleU32(FW_NEAR_UNKNOWN_1235), 75l);
            u16 mapped_value = RADIAL_SCALE_MAP_SECONDS((u16)frame_seconds);
            u16 delta = mapped_value < reference_value ? (u16)(reference_value - mapped_value) : (u16)(mapped_value - reference_value);

            if(delta >= 5000u)
            {
                u32 ratio = RADIAL_SCALE_DIVIDE_U32(RADIAL_SCALE_MULTIPLY(1000ul, (u32)delta), (u32)ReadRadialScaleU16(FW_NEAR_UNKNOWN_112E));
                u32 candidate = RADIAL_SCALE_DIVIDE_U32(0x03e80000ul, ratio);
                u16 default_scale = ReadRadialScaleU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F848FE));
                u32 minimum = (u32)(u16)((default_scale >> 4) * 12u);
                u32 maximum = (u32)(u16)((default_scale >> 4) * 20u);

                if(candidate >= minimum && candidate <= maximum)
                {
                    u8 cursor = FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED5);
                    u8 index = (u8)(cursor & 0x3fu);

                    WriteRadialScaleU16(FW_NEAR_UNKNOWN_0E53 + (fw_addr_t)((u16)index * 2u), delta);
                    if(cursor >= 0x3fu)
                    {
                        u32 sum = 0ul;
                        u8 sample_index;

                        for(sample_index = 0u; sample_index < 0x40u; sample_index++)
                            sum += ReadRadialScaleU16(FW_NEAR_UNKNOWN_0E53 + (fw_addr_t)((u16)sample_index * 2u));
                        WriteRadialScaleU16(FW_NEAR_UNKNOWN_0FFA, (u16)(sum >> 6));
                        WriteRadialScaleU16(FW_NEAR_UNKNOWN_1000, (u16)(sum >> 6));
                    }
                    cursor = (u8)(cursor + 1u);
                    if(cursor == 0xf0u)
                        cursor = 0x40u;
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED5, cursor);
                }
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x7fu));
}
