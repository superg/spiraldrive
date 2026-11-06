#include "post_toc_controller_state.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_parameters.h"
#include "../platform/drive_control_reset.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/media_interrupt_leaves.h"
#include "../platform/status_mapping.h"
#include "../scsi/read_buffer_snapshot_restore.h"
#include "../startup/controller_calibration_configuration.h"

extern void ResetDriveControlStateToEight(void);
extern u8 InitializeControllerOperatingStateAndRunCalibration(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/post_toc_controller_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16At(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadBigEndianU32At(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU16At(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteBigEndianU32At(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fbb2c5 */
u8 PLEXTOR_NEAR_FUNCTION InitializePostTocControllerState(void)
{
    u8 saved_parameter_fc;
    u8 flags;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);

    RESET_ALL_TOC_STATE();
    RESET_ALL_PMA_STATE();
    RESET_ATIP_STATE();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x01u));
    LOAD_CALIBRATION_STATE();
    RESET_CONTROLLER_STATE();
    RESET_CONTROLLER_HARDWARE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xfeu));
    RESTORE_READ_BUFFER_SNAPSHOT();

    WriteBigEndianU32At(FW_NEAR_UNKNOWN_FBE6, ReadBigEndianU32At(FW_NEAR_UNKNOWN_0CF5));
    WriteBigEndianU16At(FW_NEAR_UNKNOWN_FBEA, ReadBigEndianU16At(FW_NEAR_UNKNOWN_0CF9));
    RESET_DRIVE_CONTROL();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        saved_parameter_fc = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F);
        CONFIGURE_PARAMETER_STATE2();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, saved_parameter_fc);
    }

    flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_1305);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)((flags & 0xe7u) | 0x08u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
    {
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_1305);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(flags & 0xe7u));
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
        DECODE_ATIP_FIELDS();

    APPLY_POST_TOC_PROFILE();
    RECALCULATE_RADIAL_STATE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 0u);
    WriteBigEndianU16At(FW_NEAR_UNKNOWN_F990, ReadBigEndianU16At(FW_NEAR_UNKNOWN_FAB8));
    WriteBigEndianU16At(FW_NEAR_UNKNOWN_F984, (u16)COMPUTE_LOGARITHMIC_SCALE(ReadBigEndianU16At(FW_NEAR_UNKNOWN_FBDE)));
    WriteBigEndianU16At(FW_NEAR_UNKNOWN_F988, (u16)COMPUTE_LOGARITHMIC_SCALE(ReadBigEndianU16At(FW_NEAR_UNKNOWN_FBE0)));

    return INITIALIZE_CONTROLLER() != 0u ? 0u : 2u;
}
