#include "arithmetic.h"
#include "controller_mapping.h"
#include "controller_measurement_transition.h"
#include "firmware_memory.h"

extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);

static u16 ReadControllerMeasurementReferenceU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteControllerMeasurementReferenceU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u8 UseFixedControllerMeasurementReference(u8 operating_code)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u && operating_code >= 0x18u)
        return 1u;
    if(operating_code == 0xffu)
        return 1u;
    if(operating_code == 0x0au && (FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x40u) != 0u)
        return 1u;
    return 0u;
}

static u32 SelectFixedControllerMeasurementReference(u8 operating_code)
{
    if(operating_code == 0x0au)
        return ReadControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_0CE9);
    if(operating_code == 0x18u)
        return ReadControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_0CEB);
    if(operating_code == 0x20u)
        return ReadControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_0CED);
    if(operating_code == 0x28u)
        return ReadControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_0CEF);
    if(operating_code == 0x34u || operating_code == 0xffu)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x34u)
            return ReadControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_0CF1);
        return ReadControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_0CF3);
    }

    /* The native fixed branch is reached only for the canonical code set. */
    return 0ul;
}

static u16 ComputeControllerMeasurementAuxiliaryWord(u16 root, u8 operating_code)
{
    if(root < 250u)
        return 0u;
    if(operating_code == 1u)
        return (u16)((root - 250u) / 67u);
    return (u16)((root - 250u) / 83u);
}

/* RAM:fd0380 */
u32 PLEXTOR_NEAR_FUNCTION ComputeControllerMeasurementReferenceForOperatingCode(u32 reference_value, u16 radial_scale_quotient, u8 operating_code)
{
    u32 value;
    u32 numerator;
    u32 denominator;
    u16 root;
    u8 profile_index;
    s8 profile_offset;
    fw_addr_t profile_word_address;
    u16 profile_word;

    if(UseFixedControllerMeasurementReference(operating_code) != 0u)
    {
        WriteControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_FC25, 0u);
        return SelectFixedControllerMeasurementReference(operating_code);
    }

    value = (u32)DivideS32Quotient((s32)reference_value, 750l);
    value = MultiplyU32Low(value, (u32)radial_scale_quotient);
    value = MultiplyU32Low(value, ReadControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_0CF9));
    value = (u32)DivideS32Quotient((s32)value, 1000l);
    value = MultiplyU32Low(value, 113ul);
    value = (u32)DivideS32Quotient((s32)value, 355l);
    root = (u16)IntegerSquareRootU32(value + 62500ul);
    WriteControllerMeasurementReferenceU16(FW_NEAR_UNKNOWN_FC25, ComputeControllerMeasurementAuxiliaryWord(root, operating_code));

    numerator = MultiplyU32Low((u32)radial_scale_quotient, 6780ul);
    denominator = MultiplyU32Low((u32)(root / 10u), 710ul);
    value = DivideU32Quotient(numerator, denominator);
    value = MultiplyU32Low(value, (u32)operating_code);

    profile_index = MapOperatingCodeToProfileIndex(operating_code);
    profile_offset = (s8)(u8)(profile_index - 1u);
    profile_word_address = (fw_addr_t)(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF89E6) + (s32)profile_offset * 2l);
    profile_word = ReadControllerMeasurementReferenceU16(profile_word_address);
    numerator = MultiplyU32Low((u32)profile_word, 500ul);
    return DivideU32Quotient(numerator, value);
}
