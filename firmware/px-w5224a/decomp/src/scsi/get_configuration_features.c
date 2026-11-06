#include "get_configuration_features.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "spc.h"

static void GetFeatureTemplate(u8 index, u16 *number, fw_addr_t *source, u8 *length)
{
    switch(index)
    {
    case 0u:
        *number = 0x0000u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9102);
        *length = 16u;
        break;
    case 1u:
        *number = 0x0001u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9112);
        *length = 8u;
        break;
    case 2u:
        *number = 0x0002u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF911A);
        *length = 8u;
        break;
    case 3u:
        *number = 0x0003u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9122);
        *length = 8u;
        break;
    case 4u:
        *number = 0x0010u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF912A);
        *length = 12u;
        break;
    case 5u:
        *number = 0x001du;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9136);
        *length = 4u;
        break;
    case 6u:
        *number = 0x001eu;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF913A);
        *length = 8u;
        break;
    case 7u:
        *number = 0x0020u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9142);
        *length = 16u;
        break;
    case 8u:
        *number = 0x0021u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9152);
        *length = 12u;
        break;
    case 9u:
        *number = 0x0023u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF915E);
        *length = 4u;
        break;
    case 10u:
        *number = 0x0024u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9162);
        *length = 8u;
        break;
    case 11u:
        *number = 0x0026u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF916A);
        *length = 4u;
        break;
    case 12u:
        *number = 0x0028u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF916E);
        *length = 8u;
        break;
    case 13u:
        *number = 0x002du;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9176);
        *length = 8u;
        break;
    case 14u:
        *number = 0x002eu;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF917E);
        *length = 8u;
        break;
    case 15u:
        *number = 0x0100u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9186);
        *length = 4u;
        break;
    case 16u:
        *number = 0x0101u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF918A);
        *length = 8u;
        break;
    case 17u:
        *number = 0x0103u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9192);
        *length = 8u;
        break;
    case 18u:
        *number = 0x0104u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF919A);
        *length = 4u;
        break;
    case 19u:
        *number = 0x0105u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF919E);
        *length = 4u;
        break;
    case 20u:
        *number = 0x0107u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91A2);
        *length = 8u;
        break;
    default:
        *number = 0x0108u;
        *source = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF90FE);
        *length = 4u;
        break;
    }
}

static u16 ReadFeatureWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void CopyFeatureTemplate(fw_addr_t destination, fw_addr_t source, u8 length)
{
    u8 index;

    for(index = 0u; index < length; ++index)
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination, index), FirmwareRead8(source + index));
}

static u8 GetCurrentFeatureFlag(u8 current_profile)
{
    u8 current = 1u;

    if(current_profile == 10u
        && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) != 3u) || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) > 3u
            || (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB89) & 3u) != 0u))
        current = 0u;
    return current;
}

static u8 GetProfileCurrentFlag(u8 current_profile)
{
    return (u8)(current_profile != 0u && (FirmwareRead8(FW_HUGE_UNKNOWN_2BB045) & 0x50u) == 0x50u);
}

static u8 HasDataTrack(void)
{
    u8 descriptor_count;
    u8 index;

    descriptor_count = (u8)(ReadFeatureWord(FW_NEAR_UNKNOWN_12CA) / 11u);
    for(index = 1u; index <= descriptor_count; ++index)
        if((FirmwareRead8(FW_HUGE_UNKNOWN_2B8001 + (fw_addr_t)(index * 11u)) & 0x40u) == 0u)
            return 1u;

    for(index = 1u; index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_1149); ++index)
        if((FirmwareRead8(FW_HUGE_UNKNOWN_2BB03B + (fw_addr_t)(index * 10u)) & 0x40u) == 0u)
            return 1u;
    return 0u;
}

static u8 IncludeFeature0024(u8 current_profile)
{
    if(current_profile == 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
        return 1u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
        return 0u;
    return (u8)(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u);
}

static void SetDescriptorCurrent(fw_addr_t descriptor, u8 value)
{
    FirmwareWrite8(descriptor + 2u, (u8)(FirmwareRead8(descriptor + 2u) | value));
}

static void PatchFeatureDescriptor(u16 feature, fw_addr_t descriptor, u8 current_profile, u8 current_flag, u8 profile_current, u8 has_data_track)
{
    switch(feature)
    {
    case 0x0000u:
        if(current_profile == 9u)
            FirmwareWrite8(descriptor + 10u, 1u);
        else if(current_profile == 10u)
            FirmwareWrite8(descriptor + 14u, 1u);
        else if(current_profile != 0u)
            FirmwareWrite8(descriptor + 6u, 1u);
        break;
    case 0x0010u:
        if(current_profile != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x10u) == 0u)
        {
            SetDescriptorCurrent(descriptor, 1u);
            FirmwareWrite8(descriptor + 6u, 8u);
            FirmwareWrite8(descriptor + 9u, 1u);
        }
        break;
    case 0x001eu:
        if(current_profile != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x10u) == 0u)
            SetDescriptorCurrent(descriptor, 1u);
        break;
    case 0x0020u:
        if(current_profile == 10u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        {
            SetDescriptorCurrent(descriptor, current_flag);
            (void)ComputeAndPublishMediaCapacityGeometry();
            FirmwareWrite8(descriptor + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C9A));
            FirmwareWrite8(descriptor + 5u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C9B));
            FirmwareWrite8(descriptor + 6u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C9C));
            FirmwareWrite8(descriptor + 7u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C9D));
            FirmwareWrite8(descriptor + 10u, 8u);
            FirmwareWrite8(descriptor + 13u, 1u);
        }
        break;
    case 0x0021u:
        if(current_profile != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u && profile_current == 0u && current_profile != 8u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x0023u:
        if(current_profile == 10u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x0024u:
        if(current_profile == 10u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x0026u:
        if(current_profile == 10u && profile_current != 0u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x0028u:
        if(current_profile == 10u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x002du:
        if((current_profile == 9u || current_profile == 10u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u && profile_current == 0u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x002eu:
        if((current_profile == 9u || current_profile == 10u) && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x10u) != 0u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x0101u:
        if(current_profile == 10u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    case 0x0103u:
        if(current_profile != 0u && has_data_track != 0u)
            SetDescriptorCurrent(descriptor, 1u);
        break;
    case 0x0107u:
        if(current_profile != 0u)
            SetDescriptorCurrent(descriptor, current_flag);
        break;
    default:
        break;
    }
}

/* RAM:f9ba19 */
u16 PLEXTOR_NEAR_FUNCTION BuildGetConfigurationFeatureResponse(u8 request_type, u16 starting_feature_number, u16 initial_response_length)
{
    u16 response_length = initial_response_length;
    u8 current_profile = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B31);
    u8 current_flag = GetCurrentFeatureFlag(current_profile);
    u8 profile_current = GetProfileCurrentFlag(current_profile);
    u8 has_data_track = current_profile != 0u ? HasDataTrack() : 0u;
    u8 index;

    for(index = 0u; index < 22u; ++index)
    {
        u16 feature_number;
        fw_addr_t feature_source;
        u8 feature_length;
        fw_addr_t descriptor;
        u8 payload_length;

        GetFeatureTemplate(index, &feature_number, &feature_source, &feature_length);
        if(request_type == 2u)
        {
            if(feature_number != starting_feature_number)
                continue;
        }
        else if(feature_number < starting_feature_number)
        {
            continue;
        }

        if(feature_number == 0x0024u && IncludeFeature0024(current_profile) == 0u)
        {
            if(request_type == 2u)
                return response_length;
            continue;
        }

        descriptor = (fw_addr_t)(u16)(0x0b2au + response_length);
        CopyFeatureTemplate(descriptor, feature_source, feature_length);

        if(feature_number == 0x0108u)
        {
            payload_length = AppendDriveSerialNumberFeaturePayload(FirmwareAddressIncrementLow16(descriptor, 4u));
            if(payload_length != 0u)
                response_length = (u16)(response_length + 4u + payload_length);
        }
        else
        {
            PatchFeatureDescriptor(feature_number, descriptor, current_profile, current_flag, profile_current, has_data_track);
            if(request_type != 1u || (FirmwareRead8(descriptor + 2u) & 1u) != 0u)
                response_length = (u16)(response_length + feature_length);
        }

        if(request_type == 2u)
            return response_length;
    }

    return response_length;
}
