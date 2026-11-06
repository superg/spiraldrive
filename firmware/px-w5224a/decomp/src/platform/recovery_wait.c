#include "recovery_wait.h"
#include "firmware_memory.h"
#include "../boot/startup_progress_diagnostics.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/recovery_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd6bab */
void PLEXTOR_NEAR_FUNCTION RunRecoveryWaitAndRestartIfNeeded(void)
{
    s16 wait_index;

    INITIALIZE_RECOVERY_COROUTINE1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
    SET_RECOVERY_HANDSHAKE_HIGH();
    for(wait_index = 0; wait_index < 300; ++wait_index)
        DELAY_RECOVERY_TIMER_UNITS_59();

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C4) == 0u)
    {
        SET_RECOVERY_BIT0547();
        SET_RECOVERY_BIT0546();
        SET_RECOVERY_SFR_A9_BIT2();
        SERVICE_RECOVERY_HOST_COMMAND();
        YIELD_RECOVERY_COROUTINE0();
    }

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) >> 2) & 1u) == 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0x98u);
    RESTART_RECOVERY_FIRMWARE();
}

/* RAM:fda757 */
void PLEXTOR_NEAR_FUNCTION RunControllerStartupStatusHandshake(void)
{
    u8 completed = 0u;
    u16 state = 0u;
    u8 status;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 4u);
    while(completed == 0u)
    {
#if defined(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)
        /* High thirteen bits count pending passes in this coroutine's saved
         * state word; low three bits retain the original handshake phase.
         * Phase changes reset the budget. No diagnostic RAM allocation. */
        switch(state & 7u)
#else
        switch(state)
#endif
        {
        case 0u:
            state = 1u;
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0u);
            break;
        case 1u:
#if defined(PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS)
            StartupHandshakeDelay59();
#else
            DELAY_CONTROLLER_STARTUP();
#endif
            STARTUP_HANDSHAKE_DELAY_DONE();
            state = 2u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121E, 45u);
            break;
        case 2u:
            status = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043);
            if((status & 0x01u) == 0u)
            {
#if defined(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)
                StartupHandshakePhaseHalt(1u);
#endif
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FE, 0xffu);
                RESTART_CONTROLLER_STARTUP();
            }
            status = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043);
            if((status & 0x20u) == 0u)
            {
                state = 3u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfdu));
            }
            else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121E) <= 0)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 0x02u));
                completed = 1u;
            }
            break;
        case 3u:
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1204, 6u);
            state = 4u;
            break;
        case 4u:
            status = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043);
            if((status & 0x01u) == 0u)
            {
#if defined(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)
                StartupHandshakePhaseHalt(2u);
#endif
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FE, 0xffu);
                RESTART_CONTROLLER_STARTUP();
            }
            status = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043);
            completed = 1u;
            if((status & 0x10u) == 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 0x04u));
            else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1204) <= 0)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfbu));
            else
                completed = 0u;
            break;
        default:
#if defined(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)
            StartupHandshakePhaseHalt(8u);
#endif
            completed = 1u;
            break;
        }

        if(completed == 0u)
        {
#if defined(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)
            if((state & 7u) == 2u || (state & 7u) == 4u)
            {
                state = (u16)(state + 8u);
                if((state & 0xfff8u) == 0xfff8u)
                    StartupHandshakePhaseHalt((state & 7u) == 2u ? 6u : 7u);
            }
#endif
            YIELD_CONTROLLER_STARTUP();
        }
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
#if defined(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)
    if((state & 7u) == 2u)
        StartupHandshakePhaseHalt(3u);
    else if((state & 7u) == 4u)
        StartupHandshakePhaseHalt((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 4u) != 0u ? 4u : 5u);
    else
        StartupHandshakePhaseHalt(8u);
#endif
}
