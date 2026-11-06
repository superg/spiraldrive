#include "debounced_inputs.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/debounced_inputs_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern void ResolveDriveControlStateToZero(void);

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/startup/debounced_inputs_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/startup/debounced_inputs_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fb0000 */
void RunStartupDebouncedInputSequence(void)
{
    u8 mode;

    if(READ_P1_BIT7_LOW() == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 0x20u));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1214, 10u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1214) > 0)
        STARTUP_YIELD();
    if(READ_P1_BIT7_LOW() == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 0x20u));
        return;
    }

    CLEAR_P1_AND_CONTROLLER_WORD();
    UPDATE_P3_PATTERN(1u);
    APPLY_CONFIGURATION_FLAG();

    while(READ_P1_BIT5_LOW() == 1u)
    {
        UPDATE_P3_PATTERN(0u);
        STARTUP_YIELD();
        if(TAKE_STATE_FLAG() == 1u)
            break;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        {
            mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_130A);
            if((mode == 0x0eu || mode == 0x0bu) && ReadBigEndianU32(FW_NEAR_UNKNOWN_FAE2) <= 0x00030d40ul)
                break;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 1u)
        return;
    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_130A);
    if(mode != 0x0eu && mode != 0x0bu)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 5u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x0eu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 30u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) > 0)
        STARTUP_YIELD();
    while(READ_P1_BIT6_LOW() == 1u)
        STARTUP_YIELD();
    WriteBigEndianU32(FW_NEAR_UNKNOWN_FAE2, ReadBigEndianU32(FW_NEAR_UNKNOWN_FAE2) + 1ul);
}

/* RAM:fb00ee */
void PLEXTOR_NEAR_FUNCTION ProbeControllerSignalAndInitializeDriveControl(void)
{
    u8 rejected = 0u;
    u16 sample = 0u;

    PROBE_SET_P1_BITS();
    PROBE_STROBE_WRITE(FW_CONTROLLER_UNKNOWN_018006, 0x80u);
    PROBE_WRITE(FW_CONTROLLER_UNKNOWN_0180A8, 0u);
    PROBE_WRITE(FW_CONTROLLER_UNKNOWN_0180CC, 0x12u);
    PROBE_WAIT(8000u, 0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1221, 50u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1221) > 0)
    {
        u8 high_byte;

        PROBE_WRITE(FW_CONTROLLER_UNKNOWN_0180AA, 2u);
        high_byte = PROBE_READ(FW_CONTROLLER_UNKNOWN_0180A9);
        sample = (u16)(((u16)high_byte << 8) | PROBE_READ(FW_CONTROLLER_UNKNOWN_0180A8));
        if(sample >= 0x5000u)
        {
            rejected = 1u;
            break;
        }
    }

    if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180CC) & 0x10u) != 0u && sample <= 0x1000u)
        rejected = 1u;
    if(rejected != 0u)
        return;

    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CF5, 0x00014cccul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CF9, 0x06u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CFA, 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);
    PROBE_SET_P1_BITS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBEC, 1u);
    PROBE_RESOLVE_DRIVE_CONTROL();
}

/* RAM:fb0226 */
u8 PLEXTOR_NEAR_FUNCTION RunDriveControlState9Sequence(void)
{
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfeu));
    DRIVE_STATE9_WRITE_PHASE_BIT(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 9u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1208, 50u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0x20u) == 0u)
    {
        DRIVE_STATE9_SUBMIT_WORD(1u);
        for(;;)
        {
            u8 countdown;

            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) <= 0)
                break;
            if(DRIVE_STATE9_READ_P1_BIT7_LOW() != 0u)
                break;

            DRIVE_STATE9_YIELD();
            countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_1208);
            if(countdown <= 10u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 2u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xbfu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 2u));
                DRIVE_STATE9_CLEAR_WORD();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
                WriteBigEndianU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_FAAC) | 0x0100u));
                DRIVE_STATE9_WRITE_PHASE_BIT(0u);
                return 2u;
            }

            countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_1208);
            if(countdown <= 32u)
            {
                DRIVE_STATE9_SUBMIT_WORD(1u);
            }
            else
            {
                countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_1208);
                if(countdown <= 44u)
                    DRIVE_STATE9_SUBMIT_WORD(0u);
            }

            if(DRIVE_STATE9_TAKE_STATE_FLAG() == 1u)
            {
                DRIVE_STATE9_WAIT(1200u, 0u);
                DRIVE_STATE9_CLEAR_WORD();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
                WriteBigEndianU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_FAAC) | 0x0100u));
                DRIVE_STATE9_WRITE_PHASE_BIT(0u);
                return 2u;
            }
        }
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) <= 0)
    {
        result = 0u;
        DRIVE_STATE9_WRITE_PHASE_BIT(0u);
    }
    else
    {
        result = 1u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfdu));
    }
    DRIVE_STATE9_CLEAR_WORD();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 0x20u));
    return result;
}
