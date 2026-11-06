#include "sfr_da_db_measurement.h"
#include "controller_mapping.h"
#include "firmware_memory.h"

extern u32 DivideU32Quotient(u32 dividend, u32 divisor);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/sfr_da_db_measurement_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadMeasurementU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteMeasurementU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadMeasurementU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteMeasurementU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 FramePositionWindowCode(void)
{
    u32 frame_position = ReadMeasurementU32(FW_NEAR_UNKNOWN_1235);
    u16 middle_bytes = (u16)(frame_position >> 8);
    u16 scaled_position = (u16)(((u8)frame_position / 75u) + (u16)((middle_bytes / 75u) * 256u));

    return (u8)(scaled_position / 360u + 11u);
}

static void ClearSfrDaWindowAccumulators(void)
{
    WriteMeasurementU16(FW_NEAR_UNKNOWN_FBC7, 0u);
    WriteMeasurementU16(FW_NEAR_UNKNOWN_FBCE, 0u);
    WriteMeasurementU32(FW_NEAR_UNKNOWN_FBCA, 0ul);
}

static void AccumulateLatestSfrDaSample(void)
{
    WriteMeasurementU16(FW_NEAR_UNKNOWN_FBCE, (u16)(ReadMeasurementU16(FW_NEAR_UNKNOWN_FBCE) + 1u));
    WriteMeasurementU32(FW_NEAR_UNKNOWN_FBCA, ReadMeasurementU32(FW_NEAR_UNKNOWN_FBCA) + FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD2));
}

static u8 CompleteSfrDaAverage(void)
{
    return (u8)DivideU32Quotient(ReadMeasurementU32(FW_NEAR_UNKNOWN_FBCA), ReadMeasurementU16(FW_NEAR_UNKNOWN_FBCE));
}

static void RefreshMeasurementWindowTarget(void)
{
    if((s32)ReadMeasurementU32(FW_NEAR_UNKNOWN_1235) < 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBC9, FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBC9, FramePositionWindowCode());
}

static void ServiceSfrDaWindows(void)
{
    u16 progress;
    u16 target;
    u8 average;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD2, READ_SFR_DA());
    if(READ_BIT0039() != 0u && READ_BIT0036() == 0u)
    {
        progress = ReadMeasurementU16(FW_NEAR_UNKNOWN_FBC7);
        target = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4) * 16u);
        if(target <= progress)
        {
            WRITE_BIT0039(0u);
            WRITE_BIT002D(1u);
            average = CompleteSfrDaAverage();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD0, average);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD1, average);
            ClearSfrDaWindowAccumulators();
            RefreshMeasurementWindowTarget();
        }
        else
        {
            WRITE_BIT002D(0u);
            AccumulateLatestSfrDaSample();
        }
    }

    if(READ_BIT002D() != 0u)
    {
        progress = ReadMeasurementU16(FW_NEAR_UNKNOWN_FBC7);
        target = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FBC9) * 16u);
        if(target <= progress)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD3, CompleteSfrDaAverage());
            ClearSfrDaWindowAccumulators();
            RefreshMeasurementWindowTarget();
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && READ_BIT0028() != 0u)
                ApplySfrDaStrategyAdjustment();
        }
        else
        {
            AccumulateLatestSfrDaSample();
        }
    }
}

/* RAM:fd1ed8 */
void PLEXTOR_NEAR_FUNCTION ProcessSfrDaDbMeasurementSample(void)
{
    u8 index = FirmwareReadNear8(FW_NEAR_UNKNOWN_102C);
    u8 next_index = (u8)(index + 1u);
    fw_addr_t sample_address = FW_NEAR_UNKNOWN_100A + (u16)(index * 2u);
    u16 adjusted_sample;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102C, next_index);
    WriteMeasurementU16(sample_address, READ_SFR_DB());
    adjusted_sample = (u16)(ReadMeasurementU16(sample_address) + ((FirmwareReadNear8(FW_NEAR_UNKNOWN_102D) & 1u) == 1u ? 0x78u : 0x11u));
    WriteMeasurementU16(sample_address, adjusted_sample);
    if(adjusted_sample >= 0xbfu && (FirmwareReadNear8(FW_NEAR_UNKNOWN_102D) & 1u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_102D) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_102C) & 0x0fu));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) >= 0x30u)
        return;
    if(READ_BIT002C() == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E52) == 0xffu)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE4, FramePositionWindowCode());
        if((u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4) * 48u) < ReadMeasurementU16(FW_NEAR_UNKNOWN_FBC7))
        {
            ClearSfrDaWindowAccumulators();
            WRITE_BIT002C(1u);
        }
    }
    else
    {
        ServiceSfrDaWindows();
    }
}
