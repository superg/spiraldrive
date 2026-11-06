#include "controller_sample_series.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_sample_series_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteBigEndianU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:f8e2ef */
void CaptureControllerSampleSeries(void)
{
    u8 sample_index;
    u16 d8_sum = 0u;
    u16 d9_sum = 0u;
    u16 db_sum = 0u;

    WRITE_D4_E2(0x74u);
    RAISE_F4();
    WAIT_TIMER(3000u, 0u);

    WRITE_SFR_E1(0u);
    WRITE_SFR_E3((u8)(READ_SFR_E3() | 2u));
    WRITE_SFR_E1((u8)(READ_SFR_E1() | 0x80u));
    for(sample_index = 0u; sample_index < 20u; ++sample_index)
    {
        while((READ_SFR_F1() & 1u) == 0u)
        {
        }
        while((READ_SFR_F1() & 2u) == 0u)
        {
        }
        FirmwareWrite8(FW_HUGE_UNKNOWN_00F9B7 + sample_index, READ_SFR_D8());
        FirmwareWrite8(FW_HUGE_UNKNOWN_00F9CB + sample_index, READ_SFR_D9());
    }
    WRITE_SFR_E1((u8)(READ_SFR_E1() & 0x7fu));
    WRITE_SFR_E3((u8)(READ_SFR_E3() & 0xfdu));

    RESTORE_D4_E2();
    LOWER_F4();
    SortByteArrayAscendingInPlace(FW_NEAR_UNKNOWN_F9B7, 20u);
    SortByteArrayAscendingInPlace(FW_NEAR_UNKNOWN_F9CB, 20u);

    for(sample_index = 6u; sample_index < 14u; ++sample_index)
    {
        d8_sum = (u16)(d8_sum + FirmwareRead8(FW_HUGE_UNKNOWN_00F9B7 + sample_index) + 0x78u);
        d9_sum = (u16)(d9_sum + 0x11u + FirmwareRead8(FW_HUGE_UNKNOWN_00F9CB + sample_index));
    }
    WriteBigEndianU16(FW_NEAR_UNKNOWN_F9A0, (u16)(d8_sum >> 3));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_F9A2, (u16)(d9_sum >> 3));

    WRITE_SFR_E1(1u);
    WRITE_SFR_E1((u8)(READ_SFR_E1() | 0x80u));
    for(sample_index = 0u; sample_index < 16u; ++sample_index)
    {
        while((READ_SFR_F1() & 8u) == 0u)
        {
        }
        db_sum = (u16)(db_sum + READ_SFR_DB());
    }
    WRITE_SFR_E1((u8)(READ_SFR_E1() & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F99F, (u8)(db_sum >> 4));
}
