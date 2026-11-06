#include "indexed_drive_state_transitions.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/indexed_drive_state_transitions_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadIndexedDriveStateU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteIndexedDriveStateU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void ClearConfigurationWordBit1(void)
{
    WriteIndexedDriveStateU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadIndexedDriveStateU16(FW_NEAR_UNKNOWN_FAAC) & 0xfffdu));
}

static void PublishFatalIndexedState(u8 reason_code)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0x0du);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
    INDEXED_RESET_DRIVE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, reason_code);
    INDEXED_SIGNAL_PATTERN();
}

/* RAM:f850ca */
void PLEXTOR_NEAR_FUNCTION ServiceActiveIndexedDriveStateTransitions(void)
{
    u8 reject_indexed_state;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) != 1u))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0x0du);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x39u);
        ClearConfigurationWordBit1();
    }

    if(INDEXED_OPERATION_IN_TOC_SET() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 2u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) != 0x3eu
            && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) != 0x2cu && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) != 0x4eu)
        {
            PublishFatalIndexedState(0x3bu);
            return;
        }

        if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 4u) != 0u)
        {
            if((ReadIndexedDriveStateU16(FW_NEAR_UNKNOWN_FAAC) & 0x2000u) != 0u)
            {
                PublishFatalIndexedState(0x3du);
                return;
            }
            if((ReadIndexedDriveStateU16(FW_NEAR_UNKNOWN_FAAC) & 0x0800u) == 0u)
            {
                PublishFatalIndexedState(0x3au);
                return;
            }
            INDEXED_REPLACE_WRITE_SPEED(0xffu);
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x20u) != 0u && (ReadIndexedDriveStateU16(FW_NEAR_UNKNOWN_FAAC) & 0x0600u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0x0du);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
            INDEXED_RESET_DRIVE();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x39u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
            {
                INDEXED_SIGNAL_PATTERN();
                return;
            }
        }
        ClearConfigurationWordBit1();
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
    {
        reject_indexed_state = 0u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
        {
            PublishFatalIndexedState(0x39u);
            return;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) != 0x91u)
            reject_indexed_state = 1u;
        ClearConfigurationWordBit1();
        if(reject_indexed_state != 0u)
        {
            INDEXED_RESET_DRIVE();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x27u);
            INDEXED_SIGNAL_PATTERN();
            return;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 5u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 8u) == 0u)
               || FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB2) != 0u)
            && FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 0u)
        {
            INDEXED_PERSIST_CALIBRATION();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
            INDEXED_UPDATE_CALIBRATION(5u);
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x12u)
        INDEXED_RESTART(FW_ENTRY_CONFIGURE_MEDIA_READ_STATE_FOR_INDEXED_DRIVE_STATE);
}
