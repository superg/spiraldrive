#include "get_configuration.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"
#include "get_configuration_features.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/get_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadGetConfigurationWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadGetConfigurationDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteGetConfigurationDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:f9c3d0 */
void HandleGetConfigurationCommand(void)
{
    u16 allocation_length;
    u16 response_length;
    u16 transfer_length;
    u8 media_state;
    u8 request_type;
    u16 starting_feature_number;

    if(VALIDATE_GET_CONFIGURATION_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8EA2)) == 0u)
    {
        REJECT_GET_CONFIGURATION();
        return;
    }

    allocation_length = ReadGetConfigurationWord(FW_NEAR_UNKNOWN_0B11);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
    }
    else
    {
        media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
        if(media_state == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 9u);
        else if(media_state == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 10u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 8u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 2u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 8u);
    }

    request_type = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 3u);
    starting_feature_number = ReadGetConfigurationWord(FW_NEAR_UNKNOWN_0B0C);
    response_length = BUILD_GET_CONFIGURATION_FEATURES(request_type, starting_feature_number, 8u);
    WriteGetConfigurationDword(FW_NEAR_UNKNOWN_0B2A, (u32)(u16)(response_length - 4u));
    STAGE_GET_CONFIGURATION_RESPONSE(FW_NEAR_UNKNOWN_0B2A, ReadGetConfigurationDword(FW_NEAR_UNKNOWN_155A), (u32)response_length);

    transfer_length = response_length < allocation_length ? response_length : allocation_length;
    if(EXECUTE_GET_CONFIGURATION_DATA_IN((u32)transfer_length) != 4u)
        SET_GET_CONFIGURATION_NO_SENSE();
}
