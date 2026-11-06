#include "controller_profile_sample_interpolation.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_profile_sample_interpolation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PROFILE_RECORD_SIZE 28u
#define PROFILE_LOW_TABLE PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83B1D)
#define PROFILE_HIGH_TABLE PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83CDD)

static s8 DecodeSignedMagnitude(u8 value, u8 sign_mask)
{
    if((value & sign_mask) != 0u)
        return (s8)(u8)(0u - (u8)(value & (u8)~sign_mask));
    return (s8)value;
}

static s8 ReadSignedProfileByte(fw_addr_t record, u8 offset)
{
    return (s8)FirmwareRead8(record + offset);
}

static s8 InterpolateProfileSample(s16 start, s16 end, float profile_value, u8 encode_20)
{
    float coefficient = (float)(s16)(end - start) / 2048.0f;
    float offset = (float)start - coefficient * 256.0f;
    float result = coefficient * profile_value;
    u8 byte_result;

    result = result * profile_value;
    result = result + offset;
    byte_result = (u8)(s32)FLOOR_PROFILE_VALUE(result);
    if((encode_20 != 0u) && ((s8)byte_result < 0))
        byte_result = (u8)((u8)(0u - byte_result) | 0x20u);
    return (s8)byte_result;
}

/* RAM:f9e627 */
/* DR4=profile_value, ACC=sample_index, ACC=result. */
s8 PLEXTOR_NEAR_FUNCTION InterpolateControllerProfileSampleByIndex(float profile_value, u8 sample_index)
{
    fw_addr_t row_offset;
    fw_addr_t low_record;
    fw_addr_t high_record;
    s16 low_value;
    s16 high_value;
    u8 offset;

    if(sample_index > 35u)
        return 0;

    row_offset = (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F96B) * PROFILE_RECORD_SIZE);
    low_record = PROFILE_LOW_TABLE + row_offset;
    high_record = PROFILE_HIGH_TABLE + row_offset;

    if(sample_index <= 8u)
    {
        offset = (u8)(sample_index + 1u);
        low_value = (s16)DecodeSignedMagnitude(FirmwareRead8(low_record), 0x40u) + (s16)ReadSignedProfileByte(low_record, offset);
        high_value = (s16)DecodeSignedMagnitude(FirmwareRead8(high_record), 0x40u) + (s16)ReadSignedProfileByte(high_record, offset);
        return InterpolateProfileSample(low_value, high_value, profile_value, 0u);
    }

    if(sample_index <= 17u)
    {
        offset = (u8)(sample_index + 1u);
        low_value = DecodeSignedMagnitude(FirmwareRead8(low_record + offset), 0x20u);
        high_value = DecodeSignedMagnitude(FirmwareRead8(high_record + offset), 0x20u);
        return InterpolateProfileSample(low_value, high_value, profile_value, 1u);
    }

    if(sample_index <= 26u)
    {
        offset = (u8)(sample_index + 1u);
        low_value = (s16)(FirmwareRead8(low_record + offset) >> 4);
        high_value = (s16)(FirmwareRead8(high_record + offset) >> 4);
        return InterpolateProfileSample(low_value, high_value, profile_value, 0u);
    }

    offset = (u8)(sample_index - 8u);
    low_value = (s16)DecodeSignedMagnitude(FirmwareRead8(low_record), 0x40u) - (s16)(FirmwareRead8(low_record + offset) & 0x0fu);
    high_value = (s16)DecodeSignedMagnitude(FirmwareRead8(high_record), 0x40u) - (s16)(FirmwareRead8(high_record + offset) & 0x0fu);
    return InterpolateProfileSample(low_value, high_value, profile_value, 0u);
}
