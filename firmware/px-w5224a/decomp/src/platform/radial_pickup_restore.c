#include "radial_pickup_restore.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "controller_transition_wait.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/radial_pickup_restore_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRadialPickupRestoreWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteRadialPickupRestoreWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:fd8846 */
void PLEXTOR_NEAR_FUNCTION RestoreControllerStateAfterScaledRadialPickupMove(void)
{
    u8 run_initial_tolerance_wait = 1u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
    {
        if(ReadRadialPickupRestoreWord(FW_NEAR_UNKNOWN_112E) >= 62u)
        {
            u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

            if(operating_code >= 24u || (operating_code > 8u && ReadRadialPickupRestoreWord(FW_NEAR_UNKNOWN_112E) >= 124u))
                run_initial_tolerance_wait = 0u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 8u && ReadRadialPickupRestoreWord(FW_NEAR_UNKNOWN_112E) >= 124u)
    {
        run_initial_tolerance_wait = 0u;
    }

    if(run_initial_tolerance_wait != 0u)
        WAIT_TOLERANCE(0u, 0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 7u);
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180AC, 6u);
    while((READ_PORT(FW_CONTROLLER_UNKNOWN_0180AC) & 0x7fu) != 0u)
    {
    }

    WAIT_DURATION(FirmwareReadNear8(FW_NEAR_UNKNOWN_1130) == 1u ? 600u : 800u, 1u);
    SET_CHANNEL(1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98D));
    CONFIGURE_RANGE();
    WRITE_PORT_80E4();
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180E1, 0xb3u);
    SET_CHANNEL(2u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        APPLY_D1();
    WRITE_D1_D2_D3();

    if(run_initial_tolerance_wait == 0u)
    {
        u16 countdown;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
            countdown = ReadRadialPickupRestoreWord(FW_NEAR_UNKNOWN_10C0) >= 620u ? 100u : 60u;
        else
            countdown = 30u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 4u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0xfbu));
            countdown = 30u;
        }
        WriteRadialPickupRestoreWord(FW_NEAR_UNKNOWN_1200, countdown);
        WAIT_TOLERANCE(1u, 1u);
    }

    WRITE_PARAMETER((u16)(0xe700u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 0x80u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0xfeu));
    WriteRadialPickupRestoreWord(FW_NEAR_UNKNOWN_FC2B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2D, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2E, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2F, 3u);
}
