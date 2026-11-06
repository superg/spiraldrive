#include "controller_state6_sample_calibration.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_state6_sample_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_state6_sample_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void SortState6Samples(u8 *values)
{
    u8 index;

    for(index = 1u; index < 20u; ++index)
    {
        u8 value = values[index];
        u8 position = index;
        while(position != 0u && values[position - 1u] > value)
        {
            values[position] = values[position - 1u];
            position--;
        }
        values[position] = value;
    }
}

static u8 RejectState6SampleCalibration(u8 *override_failure)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0x40u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0x15u);
        return 0u;
    }
    *override_failure = 1u;
    return 1u;
}

/* RAM:fb1600 */
u8 CalibrateControllerParameterFBFromSampleSeriesForState6(void)
{
    u8 scratch[20];
    u8 pass = 0u;
    u8 override_failure = 0u;
    /* Native leaves this frame slot stale on the indexed diagnostic-override
     * route. Initialize it so the reconstruction has defined C behavior. */
    u16 accumulated_value = 0u;

    S6_WRITE_D4_E2(0x34u);
    S6_WRITE_SHARED_COUNTDOWN(50l);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9A8, 0u);
    while(S6_READ_SHARED_COUNTDOWN() > 0l)
        S6_YIELD();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        S6_WRITE_FC(0x68u);
    else
        S6_WRITE_FC(0x80u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, 0xc0u);
    S6_WRITE_FB(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));

    {
        u8 index;
        for(index = 0u; index < 20u; ++index)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9B7 + index, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9CB + index, 0u);
        }
        for(index = 0u; index < 16u; ++index)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_100A + (u16)index * 2u, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_100B + (u16)index * 2u, 0u);
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15CA, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 60u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) > 0)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 4u) == 0u)
            break;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 2u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 4u));
            break;
        }
        S6_YIELD();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfbu));
    S6_SUBMIT_WORD(0u, 0x0251u);
    S6_SELECT_FC(1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) != 0u)
        S6_WRITE_FC((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) + 0x20u));
    S6_RESTORE_D412();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 4u) != 0u || (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) <= 0)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0x40u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, 0x80u);
            S6_WRITE_FB(0x80u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0x15u);
            return 0u;
        }
        override_failure = 1u;
    }

    while(pass < 2u)
    {
        u8 index;
        u8 run_length;
        u8 match_count = 1u;
        u8 sorted_index = 0u;
        u8 source_index = 0u;
        u8 first_source_index = 0u;
        u8 lower_index = 0u;
        u8 upper_index = 0u;
        u16 candidate_value = 0u;
        fw_addr_t source_base = pass == 0u ? FW_NEAR_UNKNOWN_F9B7 : FW_NEAR_UNKNOWN_F9CB;

        for(index = 0u; index < 20u; ++index)
            scratch[index] = 0u;
        scratch[0] = FirmwareRead8(source_base);
        for(index = 1u; index < 14u; ++index)
        {
            u16 sum = FirmwareRead8(source_base + index - 1u);
            sum = (u16)(sum + FirmwareRead8(source_base + index));
            sum = (u16)(sum + FirmwareRead8(source_base + index + 1u));
            scratch[index] = (u8)(sum / 3u);
        }
        scratch[14] = FirmwareRead8(source_base + 14u);
        for(index = 0u; index < 15u; ++index)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9B7 + index, scratch[index]);
        for(index = 15u; index < 20u; ++index)
            scratch[index] = 0xffu;
        SortState6Samples(scratch);

        run_length = 1u;
        for(index = 1u; index < 15u && scratch[index] == scratch[0]; ++index)
            run_length++;

        while(source_index < 15u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9B7 + source_index) == scratch[sorted_index])
            {
                if(run_length == 1u)
                {
                    if(sorted_index == 0u)
                    {
                        candidate_value = (u16)(0x00c0u - (u16)source_index * 8u);
                        first_source_index = source_index;
                        upper_index = source_index < 13u ? (u8)(source_index + 2u) : 15u;
                        lower_index = source_index < 3u ? 0u : (u8)(source_index - 2u);
                        source_index = 0u;
                        sorted_index++;
                        continue;
                    }
                    if(source_index > upper_index || source_index < lower_index)
                    {
                        if(RejectState6SampleCalibration(&override_failure) == 0u)
                            return 0u;
                        break;
                    }
                    sorted_index++;
                    source_index = 0u;
                    if(sorted_index == 3u)
                        break;
                    continue;
                }

                if(match_count == 1u)
                {
                    candidate_value = (u16)(0x00c0u - (u16)source_index * 8u);
                    first_source_index = source_index;
                }
                else
                {
                    candidate_value = (u16)(candidate_value + 0x00c0u - (u16)source_index * 8u);
                }
                if(source_index > (u8)(first_source_index + run_length - 1u))
                {
                    if(RejectState6SampleCalibration(&override_failure) == 0u)
                        return 0u;
                    break;
                }
                if(match_count == run_length)
                {
                    candidate_value = (u16)(candidate_value / run_length);
                    break;
                }
                match_count++;
            }
            source_index++;
        }

        if(override_failure != 0u)
            break;
        if(pass == 0u)
            accumulated_value = candidate_value;
        else
            accumulated_value = (u16)(accumulated_value + candidate_value);
        pass++;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, (u8)((accumulated_value >> 1) & 0xf8u));
    if(S6_IS_INDEXED(5u) == 1u)
        S6_WRITE_FB(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));
    else if(override_failure == 0u)
        (void)S6_SELECT_FB();
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, S6_SELECT_FB());

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 10u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) > 0)
    {
    }
    S6_SET_CHANNEL(1u, 0x94u);
    S6_CLEAR_D1();
    S6_WRITE_D2_D3();
    S6_SUBMIT_WORD(0x0420u, 0x0133u);
    S6_SUBMIT_WORD(0x0420u, 0x0134u);
    S6_WAIT_PORT_E4();
    S6_SUBMIT_WORD(0x0160u, 0x0133u);
    S6_SUBMIT_WORD(0x0160u, 0x0134u);
    S6_APPLY_D1();
    S6_WRITE_D1_D2_D3();
    if(S6_IS_INDEXED(5u) != 1u)
    {
        S6_SET_CHANNEL(2u, 0xc1u);
        S6_CONFIGURE_RANGE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 5u);
        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) > 0)
        {
        }
        S6_REFRESH_FB();
    }

    {
        u8 pulse_result = S6_TEST_PULSE(10u);
        u8 flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xf7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(flags | ((pulse_result & 1u) << 3)));
    }
    return 1u;
}
