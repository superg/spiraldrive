#include "runtime_drive_state.h"
#include "controller_asic_initialization.h"
#include "controller_startup_mode.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/runtime_drive_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra function addresses. */
static u16 ReadBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteBigEndianU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fda144 */
void InitializeRuntimeDriveState(void)
{
    u8 transfer_mode;
    u8 index;
    u8 speed;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfbu));
    READ_CONFIGURATION(FW_NEAR_UNKNOWN_004C, FW_NEAR_UNKNOWN_0B2A, 3u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x08u));
    if(READ_P0_BIT(0u) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xf7u));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x10u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A) & 0x80u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xefu));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E750, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 0xfdu));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A) & 0x40u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E750, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) | 0x02u));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) | 0x01u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C) & 0x01u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) | 0x02u));
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) & 0x01u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C) & 0x02u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) & 0xfdu));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2E, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2D, 2u);
    transfer_mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FE) & 0xf8u);
    switch(transfer_mode)
    {
    case 0x00u:
    case 0x08u:
    case 0x10u:
    case 0x20u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 3u);
        break;
    case 0x40u:
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x08u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 7u);
            transfer_mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FE) & 0x07u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2C, transfer_mode > 2u ? 2u : transfer_mode);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 3u);
        }
        break;
    default:
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x18u) == 0x18u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2C, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FE, 0x42u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 3u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FE, 0x22u);
        }
        break;
    }

    if(READ_P0_BIT(1u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C79, 1u);
    else if(READ_P0_BIT(2u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C79, 2u);
    else if(READ_P0_BIT(6u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C79, 1u);
    else if(READ_P0_BIT(5u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C79, 2u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C79, 1u);

    READ_CONFIGURATION(FW_NEAR_UNKNOWN_0001, FW_NEAR_UNKNOWN_0040, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E750, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) | 0x01u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E750, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 0xfeu));

    CLEAR_BIT_0547();
    INITIALIZE_CONTROLLER_ASIC();
    SET_BIT_0547();
    SET_BIT_0546();
    SET_SFR_A9_BIT2();
    CONFIGURE_STARTUP_MODE(0u);

    WriteBigEndianU16(FW_NEAR_UNKNOWN_0B00, 0x0800u);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_12F8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x40u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1584, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1584) & 0xfeu));

    for(index = 0u; index < 18u; ++index)
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_0B18 + index), 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B18, 0x70u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1F, 0x0au);

    INITIALIZE_INQUIRY();
    INITIALIZE_MODE_PAGES();
    REFRESH_CONFIGURATION_FLAG();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C43, 0x28u);
    speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C43);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F3A, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC20, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7B, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7A, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7B, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7A, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7E, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7D, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 0x34u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x34u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xefu));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_FAD7) | 1u));
    SET_BIT_000B();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C36, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C35, 0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x08u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158C, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xbfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xbfu));

    INITIALIZE_READ_RUNTIME();
    CLEAR_WRITE_BUFFER_SNAPSHOT();
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1004, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1585, 5u);
}
