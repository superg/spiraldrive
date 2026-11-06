#include "timer_phase0.h"
#include "firmware_memory.h"
#include "timer_debounce.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/timer_phase0_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void ServiceDebouncedP1Bit6Low(u8 debounced_inputs)
{
    u8 old_faed = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED);
    u8 old_faef = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF);
    u8 old_debounced = FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED);
    u8 call_transition = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(old_faed | 0x20u));
    if((old_faed & 2u) == 0u || (old_faef & 0x10u) != 0u)
    {
        if((old_faed & 2u) != 0u && (old_faef & 0x10u) != 0u && ((old_faef & 1u) == 0u || (old_faef & 0x40u) != 0u))
            call_transition = 1u;
    }
    else if((old_faed & 4u) == 0u || (old_faed & 0x10u) != 0u)
    {
        call_transition = 1u;
    }

    if(call_transition != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11ED, debounced_inputs);
        ServiceP1Bit6DriveStateTransition();
        debounced_inputs = FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11ED, debounced_inputs);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xbfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120C, FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 12u ? 7u : 20u);
    if((old_debounced & 0x40u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1309, 0u);
}

static void ServiceDebouncedP1Bit6High(u8 debounced_inputs)
{
    u8 old_debounced_bit6 = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 0x40u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11ED, debounced_inputs);
    if(old_debounced_bit6 == 0u)
    {
        u8 old_faed = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED);
        u8 new_faed = (u8)(old_faed & 0xdfu);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, new_faed);
        if((old_faed & 1u) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 10u)
        {
            ServiceP1Bit6DriveStateTransition();
        }
        else if((new_faed & 2u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1210, 30u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(new_faed | 2u));
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 2u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x10u) == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1210) <= 0
        && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 4u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) | 0x0cu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C43, 0x34u);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x10u) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1211) <= 0 && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 1u) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) | 4u));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) | 0x21u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0xf3u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C43, 0x28u);
    }
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120C) <= 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 0x40u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 12u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x40u));
    }
}

/* RAM:fd1b3d */
void TimerDispatchPhase0(void)
{
    u8 active_low_inputs = (u8)((READ_P1() & 0xe4u) ^ 0xffu);
    u8 previous_inputs = FirmwareReadNear8(FW_NEAR_UNKNOWN_11EE);
    u8 debounced_inputs = FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED);

    debounced_inputs = (u8)(((active_low_inputs | previous_inputs) & debounced_inputs) | (previous_inputs & active_low_inputs));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EE, active_low_inputs);
    if((debounced_inputs & 0x40u) == 0u)
        ServiceDebouncedP1Bit6Low(debounced_inputs);
    else
        ServiceDebouncedP1Bit6High(debounced_inputs);
}
