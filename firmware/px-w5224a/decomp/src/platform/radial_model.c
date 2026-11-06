#include "radial_model.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "controller_position_average.h"
#include "firmware_memory.h"
#include "frame_radial_float.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/radial_model_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/radial_model_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/radial_model_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/radial_model_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/radial_model_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/radial_model_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fce8b4 */
void RecalculateRadialOperatingThresholdState(void)
{
    s32 endpoint = 0x000514c8l;
    volatile float radial_value;
    volatile float scaled_value;
    volatile float numerator_source;
    volatile float numerator;
    volatile float inverse_value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1) != 0ul)
            endpoint = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1);
    }
    else if(FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION) != 0ul && (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1) >= 382500l && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 5u
            && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == 0xffu || (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 4u) == 0u))
    {
        endpoint = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1);
    }
    else
    {
        endpoint = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION);
    }

    radial_value = MAP_FRAME_POSITION_TO_RADIAL_FLOAT(endpoint);
    scaled_value = radial_value * 48.6f;
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_F9FC, (u16)(s32)scaled_value);

    numerator_source = 28224.0f;
    numerator = numerator_source * 500.0f;
    inverse_value = numerator / scaled_value;
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF1, (u16)(s32)inverse_value);
    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0CF3) <= 0x05beu)
    {
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF3, 0x05beu);
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_F9FC, 0x2580u);
    }

    scaled_value = radial_value * 53.0f;
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_F9FE, (u16)(s32)scaled_value);

    numerator_source = 28224.0f;
    numerator = numerator_source * 500.0f;
    inverse_value = numerator / scaled_value;
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF3, (u16)(s32)inverse_value);
    if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0CF3) <= 0x0540u)
    {
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF3, 0x0540u);
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_F9FE, 0x2904u);
    }

    SET_HIGH_OPERATING_CODE_THRESHOLDS(0x34u);
}

/* RAM:fceb1a */
u8 PLEXTOR_NEAR_FUNCTION CalibrateRadialModelSpanAtFrame90000(void)
{
    u16 measurement = 0u;
    u16 span;

    if((s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1) < 90000l && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 1500u);
        return 0u;
    }
    if((s32)FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION) < 90000l && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) != 1u)
    {
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 1500u);
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 2u);
        if(ACQUIRE_RADIAL_SPAN_TARGET(90000l) == 0u)
        {
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) < 0 && (s32)FirmwareReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION) > 360000l)
            {
                FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 1500u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB57, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB57) & 0x7fu));
                DECODE_RADIAL_SPAN_ATIP();
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 2u) != 0u)
                    FirmwareWriteBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION, FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_FB8C) + FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_FB90) - 6750ul);
                return 0u;
            }
            FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 1500u);
            return 0u;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 7u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, 90000ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 1u);
        if(ACQUIRE_RADIAL_SPAN_POSITION() == 1u)
        {
            if(TEST_RADIAL_SPAN_POSITION(90000l) == 1u)
            {
                measurement = MEASURE_RADIAL_SPAN_POSITION();
                break;
            }
        }
        else if(RESET_RADIAL_SPAN_CONTROLLER() == 0u)
        {
            return 0x11u;
        }
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
    {
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 1500u);
        return 0u;
    }

    span = CALCULATE_RADIAL_SPAN(measurement);
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, span);
    if(span >= 1800u)
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 1800u);
    else if(span <= 1200u)
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 1200u);
    return 0u;
}

/* RAM:fcecdc */
u16 MapSignedFrameSecondsToRadialModelValue(s32 signed_frame_seconds)
{
    u32 radial_model_scale = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5);
    u16 ratio = DivideU16QuotientNative(0x5618u, (u16)(radial_model_scale >> 8));
    u32 magnitude;
    u32 transformed;
    u16 root;
    u32 scaled_root;

    if(signed_frame_seconds > 0)
    {
        transformed = 0x04cb0000ul + MultiplyU32Low(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5), (u32)signed_frame_seconds);
    }
    else
    {
        magnitude = 0ul - (u32)signed_frame_seconds;
        transformed = 0x04cb0000ul - MultiplyU32Low(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5), magnitude);
    }
    root = (u16)IntegerSquareRootU32(transformed);
    scaled_root = (u32)root * (u32)ratio;
    return (u16)(scaled_root >> 8);
}

/* RAM:fc8a4c */
void PLEXTOR_NEAR_FUNCTION InitializeReadAcquisitionRadialState(void)
{
    s32 target;
    u16 radial_model_value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EF9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3C, 0u);

    RAMP_CONTROLLER_WORD_0255();
    target = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);
    radial_model_value = MAP_SIGNED_FRAME_SECONDS(DivideS32QuotientNative(target, 75l));
    FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0EE0, radial_model_value);
    if(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B) == 0x10000000ul)
        FirmwareWriteBigEndianU16Near(FW_NEAR_UNKNOWN_0EE0, MAP_SIGNED_FRAME_SECONDS(0l));

    radial_model_value = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_0EE0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE4, (u8)(radial_model_value >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3D, 3u);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3D, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1207, 0x50u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1207, 0x3cu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE2, 0u);
    CLEAR_PENDING_TARGET_NUDGE();
}
