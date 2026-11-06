#include "read_acquisition_state.h"
#include "decoder.h"
#include "disc_read_position.h"
#include "platform/controller_mapping.h"
#include "platform/controller_operating_state_initialization.h"
#include "platform/controller_parameters.h"
#include "platform/controller_q_packet_process.h"
#include "platform/drive_control_reset.h"
#include "platform/eight_channel_capture.h"
#include "platform/firmware_memory.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#include "read_acquisition_distance.h"

enum
{
    READ_ACQUISITION_UPDATE_MEASUREMENT = 1,
    READ_ACQUISITION_RESET_EIGHT_CHANNEL = 2,
    READ_ACQUISITION_ADJUST_TARGET = 3,
    READ_ACQUISITION_SELECT_OPERATING_CODE = 4,
    READ_ACQUISITION_YIELD = 5,
    READ_ACQUISITION_CONFIGURE_ALTERNATE = 6,
    READ_ACQUISITION_CONFIGURE_NORMAL = 7,
    READ_ACQUISITION_APPLY_OPERATING_CODE = 8,
    READ_ACQUISITION_RESET_PORTS = 9,
    READ_ACQUISITION_APPLY_CONDITIONAL = 10,
    READ_ACQUISITION_PREPARE_COMMAND0E = 11,
    READ_ACQUISITION_CLEAR_RAW_PORTS = 12,
    READ_ACQUISITION_REARM_Q = 13,
    READ_ACQUISITION_ARM_THREE_BYTE = 14,
    READ_ACQUISITION_COUNT_MATCH = 15,
    READ_ACQUISITION_SERVICE_EIGHT_CHANNEL = 16,
    READ_ACQUISITION_SERVICE_COMMAND0E = 17,
    READ_ACQUISITION_FINALIZE_EIGHT_CHANNEL = 18,
    READ_ACQUISITION_SET_TARGET = 19,
    READ_ACQUISITION_RESET_DRIVE_CONTROL = 20
};

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/read_acquisition_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadAcquisitionWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static s32 ReadAcquisitionDword(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static void ConfigureReadAcquisitionControllerPath(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 1u)
        CONFIGURE_ALTERNATE();
    else
        CONFIGURE_NORMAL();
    APPLY_OPERATING_CODE();
    RESET_PORTS();
}

/* RAM:f86475 */
void PLEXTOR_NEAR_FUNCTION ProcessReadAcquisitionState(void)
{
    u16 dynamic_tolerance = 0u;
    u8 bypass_operating_gate = 0u;
    u8 pulse_successes = 0u;
    s8 retry_count = 3;
    u8 match_limit = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1309, 0u);
    UPDATE_MEASUREMENT();
    RESET_EIGHT_CHANNEL();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfbu));
    ADJUST_TARGET();

    {
        u8 flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 0xfeu);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u && ReadAcquisitionDword(FW_NEAR_UNKNOWN_0E3B) >= 0
            && (u32)ReadAcquisitionDword(FW_NEAR_UNKNOWN_0E3B) != 0x10000000ul)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1036, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 0xfcu) | 1u));
            dynamic_tolerance = COMPUTE_TOLERANCE();
            flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_1036);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1036, flags);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) == 1u)
        bypass_operating_gate = 1u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 4u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 4u) != 0u && READ_STATE_BIT(FW_BIT_UNKNOWN_001C) == 0u
        && READ_STATE_BIT(FW_BIT_UNKNOWN_001D) == 0u)
        bypass_operating_gate = 1u;

    if((bypass_operating_gate != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x80u) != 0u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        SET_OPERATING_ENABLE(0u);

acquire_again:
    for(;;)
    {
        s8 acquisition_status;

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 0x18u
            && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) > 0x18u))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0x18u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 3u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 4u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 1u ? 1u : 0u);
        if(READ_STATE_BIT(FW_BIT_UNKNOWN_001C) == 1u || READ_STATE_BIT(FW_BIT_UNKNOWN_001D) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 1u);

        SELECT_OPERATING_CODE();
        acquisition_status = ACQUIRE_POSITION();
        if(acquisition_status == 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1036, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 0xfeu));
            return;
        }
        if(acquisition_status == -1)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1036, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 0xfeu));
            return;
        }
        if(acquisition_status != 1)
        {
            for(;;)
            {
            }
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            return;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 1u) == 0u)
            break;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 2u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1034, (u8)(dynamic_tolerance >> 8));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1035, (u8)dynamic_tolerance);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1036, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) | 2u));
        }
        if((s16)ReadAcquisitionWord(FW_NEAR_UNKNOWN_1034) < 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1036, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 0xfeu));
            break;
        }
        if(TEST_TOLERANCE(3u, 0u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1036, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1036) & 0xfeu));
            break;
        }
        SET_OPERATING_ENABLE(0u);
    }

    if(READ_STATE_BIT(FW_BIT_UNKNOWN_0013) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1034, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1035, 10u);
        for(;;)
            if((s16)ReadAcquisitionWord(FW_NEAR_UNKNOWN_1034) < 0)
                break;
    }

    if(bypass_operating_gate != 0u || READ_STATE_BIT(FW_BIT_UNKNOWN_000E) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 4u) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        YIELD_STATE();
        ConfigureReadAcquisitionControllerPath();
        APPLY_CONDITIONAL();
        UPDATE_MEASUREMENT();
        PREPARE_COMMAND0E();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) | 8u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) | 0x10u));
    }
    else
    {
        SET_OPERATING_ENABLE(0u);
        goto acquire_again;
    }

capture_again:
    for(;;)
    {
        u8 status;

        CLEAR_RAW_PORTS();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 2u) != 0u)
        {
            REARM_Q();
            while((status = PROCESS_Q()) == 0xfeu)
            {
                YIELD_STATE();
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
                    FINALIZE_EIGHT_CHANNEL();
                    return;
                }
                COUNT_MATCH();
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB94, 0u);
        }
        else
        {
            ARM_THREE_BYTE();
            while((status = PARSE_THREE_BYTE()) == 0xfeu)
            {
                YIELD_STATE();
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
                    return;
                }
                COUNT_MATCH();
            }

            if(TEST_PULSE() != 0u)
                ++pulse_successes;
            else
                pulse_successes = 0u;

            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == -1 || (((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u) && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 4u) == 0u)))
                match_limit = 0x19u;
            else
                match_limit = 0x46u;

            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 2u) != 0u && pulse_successes > 4u)
            {
                REARM_Q();
                while(PROCESS_Q() == 0xfeu)
                    YIELD_STATE();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfdu));
            }

            if(pulse_successes >= match_limit && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE8) < 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) != 1u && READ_STATE_BIT(FW_BIT_UNKNOWN_001C) != 1u
                && READ_STATE_BIT(FW_BIT_UNKNOWN_001D) != 1u && READ_STATE_BIT(FW_BIT_UNKNOWN_0002) != 1u)
            {
                SET_OPERATING_ENABLE(0u);
                APPLY_CONDITIONAL();
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79A, 0u);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
            break;

        SERVICE_EIGHT_CHANNEL();
        SERVICE_COMMAND0E();
        retry_count = 3;
        match_limit = 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            FINALIZE_EIGHT_CHANNEL();
            return;
        }
        SET_TARGET_FROM_Q();
    }

    FINALIZE_EIGHT_CHANNEL();
    --retry_count;
    if(retry_count < 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        REDUCE_SPEED(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) == 7u ? 1u : 0u);
    RESET_DRIVE_CONTROL();
    if(INITIALIZE_CALIBRATION() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 2u) != 0u)
    {
        YIELD_STATE();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            return;
        }
    }
    ConfigureReadAcquisitionControllerPath();
    goto capture_again;
}
