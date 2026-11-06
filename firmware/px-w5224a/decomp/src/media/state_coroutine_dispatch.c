#include "state_coroutine_dispatch.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "../startup/media_coroutine_state1.h"
#include "coroutine_state5.h"
#include "read_acquisition_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/state_coroutine_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/state_coroutine_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f85d74 */
void RunMediaStateCoroutine1(void)
{
    u8 previous_state = 0xffu;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFD, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x08u));

    for(;;)
    {
        u8 state;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F971, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 0xfeu));
        if(previous_state != 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE8, 0u);
        previous_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF);
        state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF);

        switch(state)
        {
        case 0u:
            DispatchMediaState0();
            break;
        case 1u:
            DispatchMediaState1();
            break;
        case 2u:
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) | 0x20u));
            DispatchReadAcquisitionState();
            DispatchFinalizeMediaState2();
            break;
        case 3u:
            DispatchMediaState3();
            break;
        case 5u:
            DispatchMediaState5();
            DispatchFinalizeMediaState5();
            break;
        case 6u:
            DispatchMediaState6();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x08u));
            break;
        default:
            EnterMediaStateTerminalTrap();
            break;
        }
    }
}

/* RAM:f85eac */
void PLEXTOR_NEAR_FUNCTION ProcessMediaCoroutineState0(void)
{
    u8 state;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        MediaState0SetIndexedP3Pattern();
    MediaState0ResetDriveControl();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 1u);
    MediaState0WriteD6Value7E();
    MediaState0SetP1TimerPinsLow();
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u)
    {
        MediaState0Yield();
        MediaState0ServiceIndexedWait();
        MediaState0TryAdvanceState();
    }
    MediaState0SetP1TimerPinsHigh();
    MediaState0WriteD6Value00();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);

    state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF);
    if(state == 2u || state == 1u || state == 3u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u
            || (FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) != 0xffu && (((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) >> 6) & 1u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x04u) != 0u)))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xefu));
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x10u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xefu));
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x01u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xefu));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) != 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 8u);
    if(MediaState0InitializeControllerCalibration() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        MediaState0Yield();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xefu));
}
