#include "controller_progress.h"
#include "controller_progress_transition.h"
#include "firmware_memory.h"

extern void StartControllerMeasurementEvaluation(void);
extern u8 PollControllerMeasurementEvaluation(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_progress_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fb8de8 */
u8 NormalizeControllerMeasurementPollStatus(void)
{
    u8 status = PollControllerMeasurementEvaluation();

    if(status == 0xfeu)
        return 1u;
    if(status == 1u)
    {
        StartControllerMeasurementEvaluation();
        return 1u;
    }
    return status;
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ReadBigEndianS32(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static u16 ReadControllerProgressWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* RAM:fcee18 */
void UpdateControllerWord035FFromProgressBucket(void)
{
    u32 inherited_dr12 = CAPTURE_PROGRESS_DR12();
    u16 controller_word = (u16)(inherited_dr12 >> 16);
    u8 profile_index = (u8)inherited_dr12;
    u8 progress_bucket;
    fw_addr_t profile_table = 0u;
    fw_addr_t word_address = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x18u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_125C) == FirmwareReadNear8(FW_NEAR_UNKNOWN_125D) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D))
        return;

    progress_bucket = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125C) / 5u);
    if(progress_bucket >= 15u)
        progress_bucket = 15u;

    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43))
    {
    case 0x18u:
        word_address = FW_NEAR_UNKNOWN_0CEB;
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF904E);
        break;
    case 0x20u:
        word_address = FW_NEAR_UNKNOWN_0CED;
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF905E);
        break;
    case 0x28u:
        word_address = FW_NEAR_UNKNOWN_0CEF;
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF906E);
        break;
    case 0x34u:
        word_address = FW_NEAR_UNKNOWN_0CF3;
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF907E);
        break;
    default:
        break;
    }

    if(profile_table != 0u)
    {
        controller_word = ReadControllerProgressWord(word_address);
        profile_index = FirmwareRead8(profile_table + progress_bucket);
    }

    SUBMIT_PROGRESS_WORD(controller_word, 0x035fu);
    ApplyControllerProgressProfileTransition(profile_index, progress_bucket, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125D, FirmwareReadNear8(FW_NEAR_UNKNOWN_125C));
}

/* RAM:fceedd */
void UpdateControllerProfileFromReadAcquisitionTarget(void)
{
    s32 target = ReadBigEndianS32(FW_NEAR_UNKNOWN_0E3B);
    u8 progress = (u8)(target / 4500l);
    fw_addr_t profile_table;
    u8 profile_index;

    if(ReadBigEndianS32(FW_NEAR_UNKNOWN_0E3B) < 0l)
        progress = 0u;
    if(progress == FirmwareReadNear8(FW_NEAR_UNKNOWN_125C))
        return;

    progress = (u8)(progress / 5u);
    if(progress >= 15u)
        progress = 15u;

    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43))
    {
    case 0x18u:
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF904E);
        break;
    case 0x20u:
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF905E);
        break;
    case 0x28u:
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF906E);
        break;
    case 0x34u:
        profile_table = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF907E);
        break;
    default:
        for(;;)
        {
        }
    }

    profile_index = FirmwareRead8(profile_table + progress);
    ApplyControllerProgressProfileTransition(profile_index, progress, 0u);
}
