#include "controller_command_c0_calibration.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_command_c0_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCommandC0BigEndianWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* RAM:f8d615 */
u8 RunControllerCommandC0AndAdjustWord030A(void)
{
    u8 initial_pulse_result;

    CommandC0SetTimer(50l);
    while(CommandC0ReadTimer() >= 0l)
        CommandC0Yield();

    do
    {
        u8 repeated_pulse_result;

        initial_pulse_result = CommandC0PulseTest(10u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B7, 0x3au);
        else
            CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B7, 0x3au);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B5, 0x20u);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B2, 0x58u);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B3, 0x7eu);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B4, 0x0au);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180C5, CommandC0IndexedState(5u) != 0u ? 7u : 5u);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B8, 0xe3u);
        CommandC0SubmitWord(CommandC0IndexedState(8u) == 1u ? ReadCommandC0BigEndianWord(FW_NEAR_UNKNOWN_FBF2) : 0x0400u, 0x032au);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B0, 0xc0u);
        if(CommandC0Wait2000() == 0u)
        {
            CommandC0Abort();
            return 0u;
        }

        CommandC0SubmitWord(0u, 0x032au);
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 1) & 1u) != 1u)
            return 0u;
        repeated_pulse_result = CommandC0PulseTest(10u);
        if(initial_pulse_result == repeated_pulse_result)
            break;
    } while(1);

    if(initial_pulse_result == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        u16 word_030a = CommandC0ReadWord(0x030au);

        if(word_030a < 0x5000u)
            word_030a = (u16)(word_030a + 0x3000u);
        else
            word_030a = 0x7fffu;
        CommandC0SubmitWord(word_030a, 0x030au);
    }
    return 1u;
}

/* RAM:f8d774 */
u8 RunControllerCommandD0AndAdjustWord031A(void)
{
    u8 initial_pulse_result;

    CommandC0SetTimer(50l);
    while(CommandC0ReadTimer() >= 0l)
        CommandC0Yield();

    do
    {
        u8 repeated_pulse_result;

        initial_pulse_result = CommandC0PulseTest(10u);
        if((ReadCommandC0BigEndianWord(FW_NEAR_UNKNOWN_FAAC) & 0x0080u) != 0u)
            CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B7, 0x3au);
        else
            CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B7, 0x3au);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B5, 0x20u);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B2, 0x58u);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B3, 0x7eu);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B4, 0x10u);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180C5, CommandC0IndexedState(5u) != 0u ? 7u : 5u);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B8, 0xd1u);
        CommandC0SubmitWord(CommandC0IndexedState(8u) == 1u ? ReadCommandC0BigEndianWord(FW_NEAR_UNKNOWN_FBF4) : 0x0300u, 0x033au);
        CommandC0WritePort(FW_CONTROLLER_UNKNOWN_0180B0, 0xd0u);
        if(CommandC0Wait2000() == 0u)
        {
            CommandC0Abort();
            return 0u;
        }

        CommandC0SubmitWord(0u, 0x033au);
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 1) & 1u) != 1u)
            return 0u;
        repeated_pulse_result = CommandC0PulseTest(10u);
        if(initial_pulse_result == repeated_pulse_result)
            break;
    } while(1);

    if(initial_pulse_result == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        u16 word_031a = CommandC0ReadWord(0x031au);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
            if(word_031a < 0x7000u)
                word_031a = (u16)(word_031a + 0x1000u);
            else
                word_031a = 0x7fffu;
        else if(word_031a < 0x6600u)
            word_031a = (u16)(word_031a + 0x1a00u);
        else
            word_031a = 0x7fffu;
        CommandC0SubmitWord(word_031a, 0x031au);
    }
    return 1u;
}

static void SortThreeCalibrationBytes(u8 values[3])
{
    u8 temporary;

    if(values[0] > values[1])
    {
        temporary = values[0];
        values[0] = values[1];
        values[1] = temporary;
    }
    if(values[1] > values[2])
    {
        temporary = values[1];
        values[1] = values[2];
        values[2] = temporary;
    }
    if(values[0] > values[1])
    {
        temporary = values[0];
        values[0] = values[1];
        values[1] = temporary;
    }
}

static void WriteCalibrationBigEndianWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:f8d4a0 */
u8 CalibrateControllerWords030A031AAndCaptureReferences(void)
{
    u8 samples_030a[3] = { 0u, 0u, 0u };
    u8 samples_031a[3] = { 0u, 0u, 0u };
    u8 result;
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        WordCalibrationWritePort(FW_CONTROLLER_UNKNOWN_0180A6, 0xaau);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);
    WordCalibrationWritePort(FW_CONTROLLER_UNKNOWN_0180A4, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));

    if(WordCalibrationIndexedState(5u) != 0u)
    {
        result = 0u;
        for(index = 0u; index < 3u; ++index)
        {
            result = WordCalibrationCommandC0();
            samples_030a[index] = (u8)(WordCalibrationReadWord(0x030au) >> 8);
        }
        SortThreeCalibrationBytes(samples_030a);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FACC, samples_030a[1]);
    }
    else
    {
        result = WordCalibrationCommandC0();
    }

    if(result != 0u)
    {
        if(WordCalibrationIndexedState(5u) != 0u)
        {
            result = 0u;
            for(index = 0u; index < 3u; ++index)
            {
                result = WordCalibrationCommandD0();
                samples_031a[index] = (u8)(WordCalibrationReadWord(0x031au) >> 8);
            }
            SortThreeCalibrationBytes(samples_031a);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FACD, samples_031a[1]);
        }
        else
        {
            result = WordCalibrationCommandD0();
        }
    }
    else
    {
        result = 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));
    if(result != 1u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        WordCalibrationWritePort(FW_CONTROLLER_UNKNOWN_0180A6, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 5u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) > 0)
    {
        u16 word_030a;
        u16 word_031a;

        if(WordCalibrationP0Bit7() == 0u)
            continue;
        word_030a = WordCalibrationReadWord(0x030au);
        word_031a = WordCalibrationReadWord(0x031au);
        WriteCalibrationBigEndianWord(FW_NEAR_UNKNOWN_FBDE, word_030a);
        WriteCalibrationBigEndianWord(FW_NEAR_UNKNOWN_FBE0, word_031a);
        WriteCalibrationBigEndianWord(FW_NEAR_UNKNOWN_F984, (u16)WordCalibrationTransform(word_030a));
        WriteCalibrationBigEndianWord(FW_NEAR_UNKNOWN_F988, (u16)WordCalibrationTransform(word_031a));
        return 1u;
    }
    return 0u;
}
