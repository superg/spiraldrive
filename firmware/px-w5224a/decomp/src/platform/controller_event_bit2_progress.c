#include "controller_event_bit2_progress.h"
#include "controller_operating_interpolation.h"
#include "controller_operating_window.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_event_bit2_progress_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadEvent2U16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteEvent2U16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadEvent2U32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteEvent2U32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void PublishControllerModeByte(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 8u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
}

static void FinishControllerModeByte(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
}

static void ConfigureControllerMeasurementMode(u8 c0_value, u8 use_high_cf_profile, u8 c3_test_value, u8 c3_normal_value, u8 configure_c6)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C0, c0_value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u ? (u8)(use_high_cf_profile != 0u ? 0xc4u : 4u) : (u8)(use_high_cf_profile != 0u ? 0xc0u : 0u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, c3_test_value);
    }
    else
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, c3_normal_value);
        if(configure_c6 != 0u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C6, FirmwareReadNear8(FW_NEAR_UNKNOWN_1158) == 5u ? 0x20u : 0x23u);
    }
    FinishControllerModeByte();
}

static void DispatchMeasurementControllerSubstate(void)
{
    u8 substate = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD6);

    if(substate == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, EVENT2_READ_BIT33() != 0u ? 4u : 2u);
    }
    else if(substate == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 3u);
    }
    else if(substate == 3u)
    {
        EVENT2_SET_BIT28();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 0u);
        PublishControllerModeByte();
        ConfigureControllerMeasurementMode(0u, 0u, 0x60u, 0xe0u, 0u);
    }
    else if(substate == 4u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 5u);
    }
    else if(substate == 5u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 6u);
    }
    else if(substate == 6u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 7u);
    }
    else if(substate == 7u)
    {
        EVENT2_SET_BIT28();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 8u);
        PublishControllerModeByte();
        ConfigureControllerMeasurementMode(0x20u, 1u, 0x60u, 0xe0u, 1u);
    }
    else if(substate == 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 0u);
        PublishControllerModeByte();
        ConfigureControllerMeasurementMode(0u, 1u, 0x60u, 0xe0u, 1u);
    }
    else if(substate == 10u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 0u);
        PublishControllerModeByte();
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u ? 0xc4u : 0xc0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, 0x60u);
        FinishControllerModeByte();
    }
    else if(substate == 11u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 12u);
    }
    else if(substate == 12u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 13u);
        PublishControllerModeByte();
        ConfigureControllerMeasurementMode(0u, 0u, 0x20u, 0xa0u, 1u);
    }
    else if(substate == 13u)
    {
        u16 consumer = (u16)(ReadEvent2U16(FW_NEAR_UNKNOWN_1294) - 5u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 0u);
        EVENT2_SET_BIT33();
        WriteEvent2U16(FW_NEAR_UNKNOWN_1292, (u16)(ReadEvent2U16(FW_NEAR_UNKNOWN_1292) + 5u));
        consumer &= ReadEvent2U16(FW_NEAR_UNKNOWN_129A);
        WriteEvent2U16(FW_NEAR_UNKNOWN_1294, consumer);
        EVENT2_FINALIZE_UPDATE();
    }
}

/* RAM:fb621b */
void HandleControllerEventBit2ReadMeasurementProgress(void)
{
    u16 adjustment;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 8u)
    {
        EVENT2_ADVANCE_SETUP();
        return;
    }

    if(EVENT2_READ_BIT37() == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 0u)
        {
            EVENT2_INITIALIZE_PHASE();
            return;
        }
        if(EVENT2_READ_BIT33() == 0u)
        {
            u8 status = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018032);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_150A, status);
            if(((status & 0x20u) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_14C0) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_154D, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 15u);
            }
        }
    }

    EVENT2_SERVICE_SETUP();
    DispatchMeasurementControllerSubstate();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 1u && EVENT2_READ_BIT28() != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        adjustment = (u16)(EVENT2_LOOKUP_ADJUSTMENT() + ReadEvent2U16(FW_NEAR_UNKNOWN_EF81));
        if(adjustment > 0x7eu)
            adjustment = 0x7eu;
        WriteEvent2U16(FW_NEAR_UNKNOWN_FBC3, adjustment);
    }

    if(EVENT2_READ_BIT37() == 0u && EVENT2_READ_BIT33() != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && EVENT2_WITHIN_WINDOW(1u) == 0u)
    {
        EVENT2_CLEAR_BIT2B();
        EVENT2_CLEAR_BIT43();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
        WriteEvent2U32(FW_NEAR_UNKNOWN_14DE, ReadEvent2U32(FW_NEAR_UNKNOWN_1235) - 2ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xefu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) & 0x1fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9F, 2u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF));
        EVENT2_FINALIZE_UPDATE();
    }
}
