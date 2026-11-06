#include "controller_profile_selector.h"
#include "arithmetic.h"
#include "controller_mapping.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_profile_selector_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define FIXED_PROFILE_BASE PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8363D)
#define FIXED_PROFILE_SIZE 24u
#define GENERATED_PROFILE_BASE FW_NEAR_UNKNOWN_E806
#define GENERATED_AUXILIARY_BASE FW_NEAR_UNKNOWN_E823
#define GENERATED_PROFILE_SIZE 34u

static const u16 kProfilePorts[FIXED_PROFILE_SIZE] = { 0x8082u, 0x8083u, 0x8094u, 0x8095u, 0x8084u, 0x8085u, 0x808eu, 0x8096u, 0x8097u, 0x808cu, 0x8086u, 0x8087u, 0x808fu, 0x8090u, 0x8098u, 0x8099u,
    0x808du, 0x8088u, 0x8089u, 0x8091u, 0x8092u, 0x809au, 0x809bu, 0x808bu };

static u8 IsMappedProfileByte(u8 index)
{
    return (u8)(index == 6u || index == 9u || index == 13u || index == 16u || index == 20u || index == 23u);
}

static fw_addr_t GeneratedRecordAddress(fw_addr_t base, u8 index)
{
    return (fw_addr_t)(u16)((u16)base + (u16)((u16)index * GENERATED_PROFILE_SIZE));
}

/* RAM:f9dec5 */
void PLEXTOR_NEAR_FUNCTION ProgramControllerRegisterProfileSelector(u8 profile_selector)
{
    u8 auxiliary_index = CAPTURE_AUXILIARY_INDEX();
    fw_addr_t source;
    fw_addr_t auxiliary_source;
    u8 shadow;
    u8 index;

    CLEAR_PROFILE_EA();
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);

    if(profile_selector < 0x14u)
    {
        source = (fw_addr_t)(FIXED_PROFILE_BASE + MultiplyU32Low((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F), (u32)FIXED_PROFILE_SIZE));
    }
    else
    {
        auxiliary_index = (u8)(profile_selector - 0x14u);
        source = GeneratedRecordAddress(GENERATED_PROFILE_BASE, auxiliary_index);
    }

    for(index = 0u; index != FIXED_PROFILE_SIZE; ++index)
    {
        u8 value = FirmwareRead8(FirmwareAddressIncrementLow16(source, index));

        if(IsMappedProfileByte(index) != 0u)
            value = MapControllerRegisterIndexSkippingGap(value);
        FirmwareWrite8((fw_addr_t)(FW_HUGE_UNKNOWN_010000 + kProfilePorts[index]), value);
    }

    auxiliary_source = GeneratedRecordAddress(GENERATED_AUXILIARY_BASE, auxiliary_index);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A9, FirmwareRead8(auxiliary_source));
    FirmwareWrite8(FW_HUGE_UNKNOWN_0180A6, FirmwareRead8(FirmwareAddressIncrementLow16(auxiliary_source, 1u)));
    FirmwareWrite8(FW_HUGE_UNKNOWN_0180A7, FirmwareRead8(FirmwareAddressIncrementLow16(auxiliary_source, 2u)));
    FirmwareWrite8(FW_HUGE_UNKNOWN_0180A8, FirmwareRead8(FirmwareAddressIncrementLow16(auxiliary_source, 3u)));

    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    SET_PROFILE_EA();
    SET_PROFILE_EC();
    SET_PROFILE_A9_BIT2();
}
