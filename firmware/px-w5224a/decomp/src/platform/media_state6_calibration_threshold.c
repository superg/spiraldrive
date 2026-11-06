#include "media_state6_calibration_threshold.h"
#include "encoding.h"
#include "firmware_memory.h"

static u16 ReadMediaState6ThresholdWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static s8 DecodeAndPublishMediaState6Offset(fw_addr_t encoded_address)
{
    s8 decoded = DecodeControllerEncodedSignedOffset(FirmwareRead8(encoded_address));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FADB, (u8)decoded);
    return decoded;
}

/* RAM:f89ae5 */
void PLEXTOR_NEAR_FUNCTION DeriveMediaState6CalibrationThresholdState(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 base_value = 0u;
    s8 decoded_offset = 0;
    s8 threshold_adjustment = 0;
    u8 threshold;
    u8 candidate;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF3, 0u);

    if(operating_code == 1u)
    {
        base_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F963);
        decoded_offset = DecodeAndPublishMediaState6Offset(FW_NEAR_UNKNOWN_FADC);
    }
    else if(operating_code == 4u)
    {
        base_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F964);
        decoded_offset = DecodeAndPublishMediaState6Offset(FW_NEAR_UNKNOWN_FADD);
    }
    else if(operating_code == 8u)
    {
        base_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F965);
        decoded_offset = DecodeAndPublishMediaState6Offset(FW_NEAR_UNKNOWN_FADE);
    }
    else if(operating_code == 16u || operating_code == 20u || operating_code == 24u || operating_code == 32u || operating_code == 40u)
    {
        base_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F966);
        if(operating_code == 20u || operating_code == 24u)
            base_value = (u8)(base_value + 4u);
        else if(operating_code == 32u || operating_code == 40u)
            base_value = (u8)(base_value + 10u);

        decoded_offset = DecodeAndPublishMediaState6Offset(FW_NEAR_UNKNOWN_FADF);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FABF) != 0xffu)
            threshold_adjustment = DecodeControllerEncodedSignedOffset(FirmwareReadNear8(FW_NEAR_UNKNOWN_FABF));
        threshold_adjustment = (s8)(u8)((u8)threshold_adjustment + 2u);
    }

    threshold = (u8)(base_value + (u8)decoded_offset);
    if((s8)threshold <= 0)
        threshold = 0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F962, threshold);

    if((ReadMediaState6ThresholdWord(FW_NEAR_UNKNOWN_FAAC) & 0x0002u) != 0u)
        threshold_adjustment = 0;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 12u)
    {
        candidate = 0x21u;
    }
    else
    {
        candidate = (u8)((u8)threshold_adjustment + 0x1eu);
        if((s8)candidate <= 0)
            candidate = 0u;
    }

    if(candidate <= threshold)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF3, 3u);
        threshold = 0x21u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 12u)
            threshold = candidate;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F962, threshold);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F962) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF3, 2u);
}
