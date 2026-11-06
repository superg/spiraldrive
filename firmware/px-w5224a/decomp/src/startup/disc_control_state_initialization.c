#include "disc_control_state_initialization.h"
#include "../platform/controller_parameters.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/disc_control_state_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteBigEndianU16Near(u16 address, u16 value)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/startup/disc_control_state_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:fa619b */
void InitializeDiscControlStateFromConfiguration(void)
{
    u8 configuration[6];
    u8 index;
    u8 control_value;
    s8 doubled_control;
    s8 adjusted_control;
    u16 timing_value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1082, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1080, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xdfu));

    for(index = 0u; index < 0x63u; ++index)
        WriteBigEndianU16Near((u16)(0xf1f0u + (u16)index * 2u), 0u);

    RESET_MEASUREMENT_CAPTURE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xbfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0xcfu));

    READ_CONFIGURATION_TO_MEMORY(0x002eul, FW_NEAR_UNKNOWN_1041, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1041) == 0xffu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xfdu));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) | 0x02u));

    READ_CONFIGURATION_TO_LOCAL(0x0030ul, configuration, 6u);
    if(configuration[0] == 2u)
    {
        control_value = configuration[1];
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1042, control_value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC5, control_value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, control_value);
        doubled_control = (s8)((u8)control_value + (u8)control_value);
        if(doubled_control < 0)
            adjusted_control = (s8)((u8)doubled_control - 1u);
        else
            adjusted_control = (s8)((u8)doubled_control + 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1043, (u8)DIVIDE_ADJUSTED_BYTE(adjusted_control));

        if(TEST_INDEXED_STATE(0x1fu) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xfdu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1043, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1042, 0u);
        }
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1043, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1042, 0u);
    }

    if(TEST_INDEXED_STATE(0x1fu) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfbu));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F06A, (u8)DECODE_SIGNED_OFFSET(configuration[3]));
    timing_value = (u16)(((u16)configuration[4] << 8) | configuration[5]);
    if(timing_value == 0xffffu || timing_value == 0u)
        timing_value = 0x0b80u;
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F072, timing_value);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F074, (u16)(timing_value - 0x0b80u));
}
