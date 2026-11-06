#include "controller_measurement_transition.h"
#include "controller_parameters.h"
#include "controller_progress.h"
#include "firmware_memory.h"

extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_transition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadControllerTransitionU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadControllerTransitionU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteControllerTransitionU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 SelectControllerTransitionReferencePosition(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 4u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED7) == 1u && ReadControllerTransitionU32(FW_NEAR_UNKNOWN_0E3B) != 0x10000000ul)
            return 150ul;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 0u)
            return ReadControllerTransitionU32(FW_NEAR_UNKNOWN_0EE9);
        return ReadControllerTransitionU32(FW_NEAR_UNKNOWN_0E3B);
    }
    return ReadControllerTransitionU32(FW_NEAR_UNKNOWN_125E);
}

static void InitializeControllerTransitionP1State(void)
{
    u8 drive_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1300);
    u8 current_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

    SET_CONTROLLER_TRANSITION_P1_BITS();
    if(drive_state == 4u && (current_code < 0x18u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) != 0xffu)))
        SET_CONTROLLER_TRANSITION_P1_BITS();
}

static void ApplyControllerTransitionOperatingCodeP1State(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D) > 8u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D))
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 4u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x18u)
            {
                SET_CONTROLLER_TRANSITION_P1_BITS();
                SET_CONTROLLER_TRANSITION_P1_BITS();
            }
        }
        else
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x10u) == 0u)
                SET_CONTROLLER_TRANSITION_P1_BITS();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0xefu));
            SET_CONTROLLER_TRANSITION_P1_BITS();
        }
    }
}

static u16 SelectControllerTransitionWord(u8 transition_override, u8 saved_secondary_code)
{
    u16 divisor = ReadControllerTransitionU16(FW_NEAR_UNKNOWN_FC25);
    u16 value = (u16)(0x7fffu / divisor);

    if(transition_override == 1u)
        value = 0x7fffu;
    if(saved_secondary_code >= 0x18u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        value = 0x7fffu;
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0xffu && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        value = 0x7fffu;
    return value;
}

/* RAM:fd016f */
void ApplyControllerMeasurementReferenceAndTransitionProfile(void)
{
    u8 transition_override = 0u;
    u8 secondary_code;
    u16 radial_scale_quotient;
    u16 reference;
    u16 transition_word;
    u32 reference_position;

    InitializeControllerTransitionP1State();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 1u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A8, 0u);
        transition_override = 1u;
        SET_CONTROLLER_TRANSITION_P1_BITS();
        SET_CONTROLLER_TRANSITION_P1_BITS();
    }
    ApplyControllerTransitionOperatingCodeP1State();

    reference_position = SelectControllerTransitionReferencePosition();
    secondary_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
    radial_scale_quotient = (u16)DivideU32QuotientNative(ReadControllerTransitionU32(FW_NEAR_UNKNOWN_0CF5), 65ul);
    reference = (u16)ComputeControllerMeasurementReferenceForOperatingCode(reference_position, radial_scale_quotient, secondary_code);
    WriteControllerTransitionU16(FW_NEAR_UNKNOWN_FC27, reference);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 4u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x10u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) != 0xffu)
    {
        volatile float scaled_reference = (float)reference;

        scaled_reference = scaled_reference * 1.1f;
        reference = (u16)(s32)scaled_reference;
        WriteControllerTransitionU16(FW_NEAR_UNKNOWN_FC27, reference);
    }
    SubmitControllerWordPair(reference, 0x035fu);

    if((s16)ReadControllerTransitionU16(FW_NEAR_UNKNOWN_FC25) <= 1)
        WriteControllerTransitionU16(FW_NEAR_UNKNOWN_FC25, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u)
        WriteControllerTransitionU16(FW_NEAR_UNKNOWN_FC25, 1u);
    transition_word = SelectControllerTransitionWord(transition_override, secondary_code);
    SubmitControllerWordPair(transition_word, 0x0369u);

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A6, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F98F) & 0xf8u) | 0xa2u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x18u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        UpdateControllerProfileFromReadAcquisitionTarget();
}
