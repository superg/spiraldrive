#include "controller_measurement_window.h"
#include "arithmetic.h"
#include "controller_mapping.h"
#include "controller_operating_interpolation.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

extern s16 DivideS16Quotient(s16 dividend, s16 divisor);
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_window_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_measurement_window_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_measurement_window_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadMeasurementWindowWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadMeasurementWindowDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteMeasurementWindowDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void WriteMeasurementWindowWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:fa747a */
void BeginReadRangeControllerMeasurement(void)
{
    u8 profile_index;
    u16 interval;
    u16 reload_value;
    u16 scaled_value;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x11u);
    BEGIN_RANGE_CLEAR_GLOBAL();

    profile_index = MapOperatingCodeToProfileIndex(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    interval = ReadMeasurementWindowWord((fw_addr_t)(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9234) + (u16)profile_index * 2u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802E, 8u);
    profile_index = MapOperatingCodeToProfileIndex(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, (u8)(FirmwareRead8((fw_addr_t)(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF922C) + profile_index)) | 0x80u));
    BEGIN_RANGE_WRITE_SCALED_PROFILE(FirmwareRead8((fw_addr_t)(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF922C) + FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43))));

    reload_value = (u16)(0xffffu - interval);
    FirmwareWriteDirectSfr8(8D, (u8)(reload_value >> 8));
    FirmwareWriteDirectSfr8(8B, (u8)reload_value);
    BEGIN_RANGE_CLEAR_TIMER_OVERFLOW();
    BEGIN_RANGE_SET_TIMER_RUN();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802E, 8u);
    while(BEGIN_RANGE_TIMER_OVERFLOW() == 0u)
    {
    }
    BEGIN_RANGE_CLEAR_TIMER_RUN();
    BEGIN_RANGE_CLEAR_TIMER_OVERFLOW();

    scaled_value = ReadMeasurementWindowWord(FW_NEAR_UNKNOWN_F074);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
        scaled_value = (u16)(scaled_value + 0x0ce0u);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
        scaled_value = (u16)(scaled_value + 0x0c40u);
    else
        scaled_value = (u16)(scaled_value + 0x0b40u);
    BEGIN_RANGE_WRITE_EA_EB(scaled_value);

    BEGIN_RANGE_WRITE_PARAMETER((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0D)));
    BEGIN_RANGE_SET_GLOBAL();
    BEGIN_RANGE_SET_CONTROLLER();
    BEGIN_RANGE_SET_SFR_A9_BIT2();
    BEGIN_RANGE_WRITE_PARAMETER((u16)(0xe720u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15)));
}

/* RAM:f8e5f1 */
void RecomputeMediaState5ControllerThreshold(void)
{
    s8 measurement = (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCD);

    if(measurement > 114)
    {
        WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2, 0x2cecu);
    }
    else
    {
        measurement = (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCD);
        if(measurement < -74)
        {
            WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2, 0x251cu);
        }
        else
        {
            fw_addr_t record_address;
            float coefficient;
            float base_value;
            float adjusted_measurement;
            s32 calculated;
            u16 threshold;

            record_address = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8576) + MultiplyU32Low((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E), 3ul);
            coefficient = (float)(s8)FirmwareRead8(record_address);
            record_address = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8577) + MultiplyU32Low((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E), 3ul);
            base_value = (float)(s16)ReadMeasurementWindowWord(record_address);
            measurement = (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCD);
            adjusted_measurement = (float)(s8)((u8)measurement + 0x80u);
            calculated = (s32)(base_value - ((adjusted_measurement * 8.66f + 550.0f) * coefficient / 100.0f));
            WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2, (u16)calculated);

            threshold = ReadMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2);
            if((s16)threshold > (s16)0x2cecu)
                WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2, 0x2cecu);
            threshold = ReadMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2);
            if(threshold < 0x251cu)
                WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2, 0x251cu);
        }
    }

    /* Firmware 1.04 deliberately overrides every preceding result. */
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_F9E2, 0x2710u);
}

/* RAM:fd9e94 */
void ResetReadMeasurementState(void)
{
    u8 controller_state;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 0u);
    WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_FBA2, 0ul);
    WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_FBA6, 0ul);
    WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_FBAA, 0ul);
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_FBAE, 0u);
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_FBB4, 0u);
    WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_FBB0, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBB6, 0u);
    WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_FBBE, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBC2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBB7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBBA, 0u);
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_FBBB, 0u);
    CLEAR_READ_MEASUREMENT_BIT0028();

    if(ReadMeasurementWindowDword(FW_NEAR_UNKNOWN_14D6) == 0ul)
    {
        WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_1513, 0ul);
        WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_150F, 0ul);
        WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_150B, 0ul);
    }

    CLEAR_READ_MEASUREMENT_WINDOW_BITS();
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_FBC7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBC9, FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4));
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_FBCE, 0u);
    WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_FBCA, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD4, 0u);
    WriteMeasurementWindowDword(FW_NEAR_UNKNOWN_1548, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_154D, 0u);

    if(READ_READ_MEASUREMENT_BIT0032() == 0u)
    {
        CLEAR_READ_MEASUREMENT_GLOBAL_INTERRUPT();
        controller_state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_state);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_state);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0u);
        controller_state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_state);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_state);
        SET_READ_MEASUREMENT_GLOBAL_INTERRUPT();
        SET_READ_MEASUREMENT_CONTROLLER_INTERRUPT();
        SET_READ_MEASUREMENT_SFR_A9_BIT2();
    }

    CLEAR_READ_MEASUREMENT_BIT0032();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1555, 0u);
}

/* RAM:f8e560 */
void ComputeProcessedSfrDbWindowValue(void)
{
    u16 sum = 0u;
    u16 processed;
    u8 index;

    for(index = 0u; index < 16u; ++index)
        sum = (u16)(sum + ReadMeasurementWindowWord(FW_NEAR_UNKNOWN_100A + (u16)index * 2u));

    processed = (u16)DivideS16Quotient((s16)sum, 16);
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_102A, processed);

    processed = (u16)ScaleUnsignedWordBy83Point5ToU32(ReadMeasurementWindowWord(FW_NEAR_UNKNOWN_102A));
    WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_102A, processed);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 4u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        processed = ReadMeasurementWindowWord(FW_NEAR_UNKNOWN_102A);
        processed = (u16)DivideU32Quotient((u32)processed * 9ul, 10ul);
        WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_102A, processed);
    }

    if(READ_OFFSET_ENABLE_BIT() != 0u)
    {
        u8 offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_F970);

        processed = (u16)(offset + ReadMeasurementWindowWord(FW_NEAR_UNKNOWN_102A));
        WriteMeasurementWindowWord(FW_NEAR_UNKNOWN_102A, processed);
    }
}
