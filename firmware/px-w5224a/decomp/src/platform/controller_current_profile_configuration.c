#include "controller_current_profile_configuration.h"
#include "arithmetic.h"
#include "controller_mapping.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_current_profile_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void BeginProfileProgramming(void)
{
    u8 shadow;

    CLEAR_PROFILE_EA();
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
}

static void EndProfileProgramming(void)
{
    u8 shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    SET_PROFILE_EA();
    SET_PROFILE_EC();
    SET_PROFILE_A9_BIT2();
}

static void WriteProfileAuxiliary(fw_addr_t source, u8 write_port_aa)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A9, FirmwareRead8(source));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A6, FirmwareRead8(source + 1ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A7, FirmwareRead8(source + 2ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A8, FirmwareRead8(source + 3ul));
    if(write_port_aa != 0u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180AA, FirmwareRead8(source + 4ul));
}

static void CopyIndexedProfile(fw_addr_t source, u8 operating_code)
{
    u8 index;

    for(index = 0u; index < 28u; ++index)
    {
        u8 value = 0u;

        if(operating_code >= 9u || (((index < 12u) || (index > 17u)) && index < 20u))
            value = FirmwareRead8(source++);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018081 + index, value);
    }
}

static void ConfigureIndexedProfile(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
    fw_addr_t source = 0ul;
    fw_addr_t auxiliary = 0ul;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0x82u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0x82u);
    if(operating_code == 1u || operating_code == 2u || operating_code == 4u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B6, 0u);
        source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8395D) + (fw_addr_t)PROFILE_MULTIPLY(14ul, (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_F969));
        auxiliary = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E) == 1u ? PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83ECF) : PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EA2);
    }
    else if(operating_code == 6u || operating_code == 8u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B6, 0u);
        source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83A3D) + (fw_addr_t)PROFILE_MULTIPLY(14ul, (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_F96A));
        auxiliary = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E) == 1u ? PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83ED4) : PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EA7);
    }
    else if(operating_code == 10u || operating_code == 12u || operating_code == 16u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B6, 0u);
        source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83B1D) + (fw_addr_t)PROFILE_MULTIPLY(28ul, (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_F96B));
        auxiliary = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E) == 1u ? PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EDE) : PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EB1);
    }
    else if(operating_code == 0x14u || operating_code == 0x18u || operating_code == 0x20u || operating_code == 0x28u || operating_code == 0x34u || operating_code == 0xffu)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B6, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x20u ? 0u : 4u);
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B0, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B7, 0x80u);
    if(operating_code < 0x14u)
    {
        CopyIndexedProfile(source, operating_code);
        WriteProfileAuxiliary(auxiliary, 1u);
    }
    else
    {
        APPLY_FIXED_PROFILE(operating_code == 0xffu ? 0x34u : operating_code);
    }
}

static fw_addr_t ScaledRecordAddress(fw_addr_t base, u8 stride)
{
    return base + (fw_addr_t)PROFILE_MULTIPLY((u32)stride, (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F));
}

static void SelectOrdinaryProfileSources(fw_addr_t *primary_source, fw_addr_t *mapped_source, fw_addr_t *auxiliary_source)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 use_mapped = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 1u);

    *primary_source = 0ul;
    *mapped_source = 0ul;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
    {
        if(operating_code == 0x10u)
        {
            *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8345D);
            if(use_mapped == 0u)
                *primary_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8363D), 24u);
            else
                *mapped_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8363D), 24u);
        }
        else if(operating_code == 0x14u)
        {
            *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83461);
            *mapped_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8372D), 24u);
        }
        else if(operating_code == 0x18u)
        {
            *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83465);
            *mapped_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8381D), 24u);
        }
        else if(operating_code == 0x0bu)
        {
            *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83455);
            if(use_mapped == 0u)
                *primary_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8354D), 24u);
            else
                *mapped_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8354D), 24u);
        }
        else
        {
            *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83465);
            *mapped_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8381D), 24u);
        }
    }
    else if(operating_code == 2u || operating_code == 4u)
    {
        *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83451);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u)
        {
            *primary_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83469), 3u);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 8u) != 0u)
                *primary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83447);
        }
        else
        {
            *primary_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F834BA), 3u);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 8u) != 0u)
                *primary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8344A);
        }
    }
    else if(operating_code == 0x0bu)
    {
        *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83455);
        *primary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83505);
    }
    else
    {
        *auxiliary_source = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83459);
        *primary_source = ScaledRecordAddress(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83505), 3u);
    }

    /* Canonical reachable combinations initialize the source selected by
     * RAM:00103b. Mirror the initialized source for invalid combinations so
     * a corrupt state cannot turn stale native stack bytes into an address. */
    if(*primary_source == 0ul)
        *primary_source = *mapped_source;
    if(*mapped_source == 0ul)
        *mapped_source = *primary_source;
}

static void WriteMappedProfile(fw_addr_t source)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018082, FirmwareRead8(source));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018083, FirmwareRead8(source + 1ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018094, FirmwareRead8(source + 2ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018095, FirmwareRead8(source + 3ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018084, FirmwareRead8(source + 4ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018085, FirmwareRead8(source + 5ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01808E, MapControllerRegisterIndexSkippingGap(FirmwareRead8(source + 6ul)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018096, FirmwareRead8(source + 7ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018097, FirmwareRead8(source + 8ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01808C, MapControllerRegisterIndexSkippingGap(FirmwareRead8(source + 9ul)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018086, FirmwareRead8(source + 10ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018087, FirmwareRead8(source + 11ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01808F, FirmwareRead8(source + 12ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018090, MapControllerRegisterIndexSkippingGap(FirmwareRead8(source + 13ul)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018098, FirmwareRead8(source + 14ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018099, FirmwareRead8(source + 15ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01808D, MapControllerRegisterIndexSkippingGap(FirmwareRead8(source + 16ul)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018088, FirmwareRead8(source + 17ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018089, FirmwareRead8(source + 18ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018091, FirmwareRead8(source + 19ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018092, MapControllerRegisterIndexSkippingGap(FirmwareRead8(source + 20ul)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01809A, FirmwareRead8(source + 21ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01809B, FirmwareRead8(source + 22ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01808B, MapControllerRegisterIndexSkippingGap(FirmwareRead8(source + 23ul)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01808A, 8u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018093, 0u);
}

static void ConfigureOrdinaryProfile(void)
{
    fw_addr_t primary_source;
    fw_addr_t mapped_source;
    fw_addr_t auxiliary_source;
    u8 index;

    SelectOrdinaryProfileSources(&primary_source, &mapped_source, &auxiliary_source);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0xa8u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0xa8u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0x98u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0x98u);
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180AA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0x30u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0x10u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 0x85u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018081, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B0, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B6, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B7, 0u);
    for(index = 0u; index < 27u; ++index)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018082 + index, 0u);
    WriteProfileAuxiliary(auxiliary_source, 0u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 1u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B2, FirmwareRead8(primary_source));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B3, FirmwareRead8(primary_source + 1ul));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B4, FirmwareRead8(primary_source + 2ul));
    }
    else
    {
        WriteMappedProfile(mapped_source);
    }
}

/* RAM:f9c525 */
void ConfigureControllerRegisterProfileForCurrentMediaState(void)
{
    BeginProfileProgramming();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
        ConfigureIndexedProfile();
    else
        ConfigureOrdinaryProfile();
    EndProfileProgramming();
}
