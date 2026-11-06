#include "timing.h"
#include "../boot/startup_progress_diagnostics.h"
#include "firmware_memory.h"
#include "timer_phase0.h"
#include "timer_phase2.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/timing_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void DispatchTimer0Phase(u8 phase)
{
    switch(phase)
    {
    case 0u:
        TIMER0_PHASE0();
        break;
    case 1u:
        TIMER0_ODD_PHASE();
        break;
    case 2u:
        TIMER0_PHASE2();
        break;
    case 3u:
        TIMER0_ODD_PHASE();
        break;
    case 4u:
        TIMER0_PHASE4();
        break;
    case 5u:
        TIMER0_ODD_PHASE();
        break;
    case 6u:
        TIMER0_NO_OP();
        break;
    case 7u:
        TIMER0_ODD_PHASE();
        break;
    case 8u:
        TIMER0_NO_OP();
        break;
    case 9u:
        TIMER0_ODD_PHASE();
        break;
    default:
        TIMER0_NO_OP();
        break;
    }
}

/* RAM:fd12af */
void Timer0TickHandler(void)
{
    u8 phase;

    WRITE_TIMER0_LOW(0xe6u);
    WRITE_TIMER0_HIGH(0xf8u);
    STARTUP_DIAGNOSTIC_TICK();
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F76) > 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F76, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F76) - 1u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) & 0xfdu));
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F77) > 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F77, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F77) - 1u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) & 0xfeu));
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F78) > 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F78, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F78) - 1u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) & 0xfbu));

    phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_1231);
    DispatchTimer0Phase(phase);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1231, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1231) + 1u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1231) >= 10u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1231, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1232, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1232) + 1u));
    if(TIMER0_INDEXED_STATE(5u) == 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) >> 2) & 1u) != 0u)
        TIMER0_SWEEP();
}

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/timing_bindings.h"
#undef PLEXTOR_BINDING_SECTION
