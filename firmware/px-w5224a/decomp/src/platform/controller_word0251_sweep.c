#include "controller_word0251_sweep.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

static u16 ReadSweepU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteSweepU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void CaptureSweepWindow(fw_addr_t d8_sum_address, fw_addr_t d9_sum_address, fw_addr_t average_address, u8 final_phase)
{
    u16 d8_sum = (u16)(ReadSweepU16(d8_sum_address) + DrainFourSfrD8OrD9Bytes(0u));
    u16 d9_sum = (u16)(ReadSweepU16(d9_sum_address) + DrainFourSfrD8OrD9Bytes(1u));

    WriteSweepU16(d8_sum_address, d8_sum);
    WriteSweepU16(d9_sum_address, d9_sum);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C9) == final_phase)
    {
        d8_sum >>= 4;
        d9_sum >>= 4;
        WriteSweepU16(d8_sum_address, d8_sum);
        WriteSweepU16(d9_sum_address, d9_sum);
        WriteSweepU16(average_address, (u16)((d8_sum + d9_sum) >> 1));
    }
}

static u16 AbsoluteSweepDifference(u16 left, u16 right)
{
    return left > right ? (u16)(left - right) : (u16)(right - left);
}

static void PublishCompletedSweepPass(void)
{
    u16 d8_difference = AbsoluteSweepDifference(ReadSweepU16(FW_NEAR_UNKNOWN_15CB), ReadSweepU16(FW_NEAR_UNKNOWN_15CD));
    u16 d9_difference = AbsoluteSweepDifference(ReadSweepU16(FW_NEAR_UNKNOWN_15CF), ReadSweepU16(FW_NEAR_UNKNOWN_15D1));
    u16 combined_average = (u16)((ReadSweepU16(FW_NEAR_UNKNOWN_15D3) + ReadSweepU16(FW_NEAR_UNKNOWN_15D5)) >> 1);
    u8 pass_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_15CA);

    WriteSweepU16(FW_NEAR_UNKNOWN_15D3, combined_average);
    if(pass_index == 0u)
        return;
    if(pass_index == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9B7, (u8)d8_difference);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9CB, (u8)d9_difference);
        WriteSweepU16(FW_NEAR_UNKNOWN_100A, combined_average);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, 0xb8u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9B6 + pass_index, (u8)d8_difference);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9CA + pass_index, (u8)d9_difference);
        WriteSweepU16(FW_NEAR_UNKNOWN_1008 + (u16)(pass_index * 2u), combined_average);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16) == 0x50u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfbu));
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16) > 0x50u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16) - 8u));
    }
    WriteControllerParameterFBValueWithBit0Set(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));
}

/* RAM:fd13dd */
void PLEXTOR_NEAR_FUNCTION ServiceControllerWord0251SweepSampleTick(void)
{
    u8 phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_15C9);
    u16 step_word = (u16)((u16)FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F848C7) + phase) * 0x60u);

    SubmitControllerWordPair(step_word, 0x0251u);
    if(phase >= 11u && phase <= 14u)
        CaptureSweepWindow(FW_NEAR_UNKNOWN_15CB, FW_NEAR_UNKNOWN_15CF, FW_NEAR_UNKNOWN_15D3, 14u);
    if(phase >= 35u && phase <= 38u)
        CaptureSweepWindow(FW_NEAR_UNKNOWN_15CD, FW_NEAR_UNKNOWN_15D1, FW_NEAR_UNKNOWN_15D5, 38u);
    if(phase == 39u)
        PublishCompletedSweepPass();

    phase = (u8)(phase + 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C9, phase);
    if(phase == 48u)
    {
        WriteSweepU16(FW_NEAR_UNKNOWN_15D1, 0u);
        WriteSweepU16(FW_NEAR_UNKNOWN_15CF, 0u);
        WriteSweepU16(FW_NEAR_UNKNOWN_15CD, 0u);
        WriteSweepU16(FW_NEAR_UNKNOWN_15CB, 0u);
        WriteSweepU16(FW_NEAR_UNKNOWN_15D5, 0u);
        WriteSweepU16(FW_NEAR_UNKNOWN_15D3, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15CA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_15CA) + 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C9, 0u);
    }
}
