#include "controller_words0204_0319_calibration.h"
#include "arithmetic.h"
#include "controller_calibration_outcome.h"
#include "controller_parameters.h"
#include "controller_word0319_offset_calibration.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_words0204_0319_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define FEEDBACK_COUNT 40u
#define FEEDBACK_CLEAR_COUNT 46u

static u16 ReadCalibrationWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteCalibrationWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WaitForSignedPositiveCountdown(fw_addr_t address, u8 count)
{
    FirmwareWrite8(address, count);
    while((s8)FirmwareRead8(address) > 0)
        CAL_YIELD();
}

static void ClearPublishedFeedbackTable(void)
{
    u8 index;

    for(index = 0u; index < FEEDBACK_CLEAR_COUNT; ++index)
        WriteCalibrationWord((fw_addr_t)(FW_NEAR_UNKNOWN_116E + (u16)index * 2u), 0u);
}

static u8 CaptureControllerWord0204Feedback(u16 *samples)
{
    u8 index;
    u16 current_word;

    current_word = ReadCalibrationWord(FW_NEAR_UNKNOWN_F990);
    while(current_word != 0xd000u)
    {
        current_word = (u16)(current_word - 0x0200u);
        WriteCalibrationWord(FW_NEAR_UNKNOWN_F990, current_word);
        SUBMIT_WORD(current_word, 0x0204u);
        WaitForSignedPositiveCountdown(FW_NEAR_UNKNOWN_1220, 10u);
    }
    WaitForSignedPositiveCountdown(FW_NEAR_UNKNOWN_1220, 100u);
    current_word = (u16)(current_word - 0x0200u);
    WriteCalibrationWord(FW_NEAR_UNKNOWN_F990, current_word);

    for(index = 0u; index < FEEDBACK_COUNT; ++index)
    {
        current_word = (u16)(ReadCalibrationWord(FW_NEAR_UNKNOWN_F990) + 0x0200u);
        WriteCalibrationWord(FW_NEAR_UNKNOWN_F990, current_word);
        SUBMIT_WORD(current_word, 0x0204u);
        WaitForSignedPositiveCountdown(FW_NEAR_UNKNOWN_1220, 20u);
        if(TEST_PULSE(10u) == 0u && IS_INDEXED_STATE(5u) == 0u)
        {
            LOWER_PARAMETER_F4();
            WriteCalibrationWord(FW_NEAR_UNKNOWN_11BE, 2u);
            return 0u;
        }
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 1) & 1u) != 1u)
        {
            LOWER_PARAMETER_F4();
            WriteCalibrationWord(FW_NEAR_UNKNOWN_11BE, 3u);
            return 0u;
        }
        samples[index] = RUN_COMMAND_0E();
        if(samples[index] == 0u)
        {
            LOWER_PARAMETER_F4();
            WriteCalibrationWord(FW_NEAR_UNKNOWN_11BE, 4u);
            return 0u;
        }
    }
    LOWER_PARAMETER_F4();
    return 1u;
}

static void SmoothAndPublishFeedback(u16 *samples)
{
    u8 index;
    u16 sum;

    for(index = 0u; index < FEEDBACK_COUNT; ++index)
    {
        if(index == 0u || index == FEEDBACK_COUNT - 1u)
        {
            samples[index + FEEDBACK_COUNT] = samples[index];
        }
        else
        {
            sum = (u16)(samples[index - 1u] + samples[index]);
            sum = (u16)(sum + samples[index + 1u]);
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_words0204_0319_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        }
    }

    for(index = 0u; index < FEEDBACK_COUNT; ++index)
    {
        samples[index] = samples[index + FEEDBACK_COUNT];
        WriteCalibrationWord((fw_addr_t)(FW_NEAR_UNKNOWN_116E + (u16)index * 2u), samples[index]);
    }
}

static u8 SelectControllerWord0204Candidate(const u16 *samples, u16 *candidate)
{
    u8 index;
    u8 maximum_index = 0u;
    u8 first_index = 0xffu;
    u8 last_index = 0xffu;
    u16 maximum = 0u;
    u16 left_minimum = 0xffffu;
    u16 right_minimum = 0xffffu;
    u16 baseline;
    u16 scaled_prominence;
    u16 prominence_threshold;

    for(index = 0u; index < FEEDBACK_COUNT; ++index)
    {
        if(samples[index] >= maximum)
        {
            maximum = samples[index];
            maximum_index = index;
        }
    }
    for(index = 0u; index < FEEDBACK_COUNT; ++index)
    {
        if(index <= maximum_index)
        {
            if(samples[index] <= left_minimum)
                left_minimum = samples[index];
        }
        else if(samples[index] <= right_minimum)
        {
            right_minimum = samples[index];
        }
    }

    baseline = right_minimum;
    if(baseline <= left_minimum)
        baseline = left_minimum;
    scaled_prominence = (u16)((u16)(maximum - baseline) * 4u);
    prominence_threshold = (u16)(scaled_prominence / 5u);

    for(index = 0u; index < FEEDBACK_COUNT; ++index)
    {
        if((u16)(maximum - prominence_threshold) < samples[index])
        {
            first_index = index;
            break;
        }
    }
    for(index = FEEDBACK_COUNT - 1u; (s8)index >= 0; --index)
    {
        if((u16)(maximum - prominence_threshold) < samples[index] && first_index != 0xffu)
        {
            last_index = index;
            break;
        }
    }
    if(first_index == 0xffu || last_index == 0xffu)
        return 0u;

    *candidate = (u16)(0xd000u + (u16)((u8)(first_index + last_index) >> 1) * 0x0200u);
    return 1u;
}

static s16 AverageCalibrationCandidates(u16 first, u16 second)
{
    s16 sum = (s16)(u16)(first + second);

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_words0204_0319_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u16 AbsoluteCandidateDifference(u16 first, u16 second)
{
    return (u16)AbsoluteValueI16((s16)(u16)(first - second));
}

static void MoveControllerWord0204DownTo(u16 selected_word)
{
    u16 current_word = ReadCalibrationWord(FW_NEAR_UNKNOWN_F990);

    if(selected_word == current_word)
        return;
    while(selected_word != current_word)
    {
        current_word = (u16)(current_word - 0x0100u);
        WriteCalibrationWord(FW_NEAR_UNKNOWN_F990, current_word);
        SUBMIT_WORD(current_word, 0x0204u);
        WaitForSignedPositiveCountdown(FW_NEAR_UNKNOWN_1220, 3u);
    }
    WaitForSignedPositiveCountdown(FW_NEAR_UNKNOWN_1220, 100u);
}

/* RAM:f8da66 */
u8 CalibrateControllerWords0204And0319Offset(void)
{
    u16 samples[FEEDBACK_COUNT * 2u];
    u16 candidates[3] = { 0u, 0u, 0u };
    u16 selected_word = 0u;
    u16 difference_01 = 0u;
    u16 difference_12;
    u16 difference_02;
    u16 parameter_word;
    u8 pass = 0u;
    u8 index;
    u8 calibration_result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 40u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) >= 0)
        CAL_YIELD();

    WriteCalibrationWord(FW_NEAR_UNKNOWN_F990, 0u);
    CONFIGURE_CONTROLLER();

    while(pass < 3u)
    {
        parameter_word = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) - 0x0024u);
        WRITE_PARAMETER((u16)(parameter_word | 0xf400u));
        ClearPublishedFeedbackTable();
        for(index = 0u; index < FEEDBACK_COUNT * 2u; ++index)
            samples[index] = 0u;

        if(CaptureControllerWord0204Feedback(samples) == 0u)
            return 0u;
        SmoothAndPublishFeedback(samples);
        if(SelectControllerWord0204Candidate(samples, &candidates[pass]) == 0u)
        {
            WriteCalibrationWord(FW_NEAR_UNKNOWN_11BE, 5u);
            return 0u;
        }

        if(pass == 1u)
        {
            difference_01 = AbsoluteCandidateDifference(candidates[0], candidates[1]);
            selected_word = (u16)AverageCalibrationCandidates(candidates[0], candidates[1]);
            if(difference_01 <= 0x0400u)
                break;
        }
        else if(pass == 2u)
        {
            difference_12 = AbsoluteCandidateDifference(candidates[1], candidates[2]);
            difference_02 = AbsoluteCandidateDifference(candidates[0], candidates[2]);
            if(difference_12 <= difference_02)
            {
                if(difference_12 <= difference_01)
                    selected_word = (u16)AverageCalibrationCandidates(candidates[1], candidates[2]);
            }
            else if(difference_02 <= difference_01)
            {
                selected_word = (u16)AverageCalibrationCandidates(candidates[0], candidates[2]);
            }
            break;
        }
        ++pass;
    }

    MoveControllerWord0204DownTo(selected_word);
    WriteCalibrationWord(FW_NEAR_UNKNOWN_11C4, candidates[0]);
    WriteCalibrationWord(FW_NEAR_UNKNOWN_11C6, candidates[1]);
    WriteCalibrationWord(FW_NEAR_UNKNOWN_11C8, candidates[2]);
    WriteCalibrationWord(FW_NEAR_UNKNOWN_11C0, selected_word);
    WriteCalibrationWord(FW_NEAR_UNKNOWN_F990, selected_word);
    APPLY_WORD_0204_AND_SETTLE();
    SUBMIT_WORD(ReadCalibrationWord(FW_NEAR_UNKNOWN_F990), 0x0204u);

    if(IS_INDEXED_STATE(5u) == 1u)
    {
        WriteCalibrationWord(FW_NEAR_UNKNOWN_FAB8, ReadCalibrationWord(FW_NEAR_UNKNOWN_F990));
        UPDATE_OUTCOME(3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xdfu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xfbu));
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) >> 6) & 1u) != 0u)
                return 1u;
        }
    }

    SUBMIT_WORD(ReadCalibrationWord(FW_NEAR_UNKNOWN_F99A), 0x0319u);
    calibration_result = CALIBRATE_WORD_0319();
    if(calibration_result == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 4u));
        return 0u;
    }
    return 1u;
}
