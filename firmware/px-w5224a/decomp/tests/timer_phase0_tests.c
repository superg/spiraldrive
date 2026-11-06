#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/timer_phase0.h"

static u8 memory[0x200000];
static u8 p1;
static u8 transition_replacement;
static unsigned transition_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    assert(address == 0x90u);
    return p1;
}

void ServiceP1Bit6DriveStateTransition(void)
{
    ++transition_calls;
    if(transition_replacement != 0u)
        memory[0x11ed] = transition_replacement;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    p1 = 0u;
    transition_replacement = 0u;
    transition_calls = 0u;
}

static void SetStableBit6(u8 high)
{
    if(high != 0u)
    {
        p1 = 0u;
        memory[0x11ee] = 0x40u;
        memory[0x11ed] = 0x40u;
    }
    else
    {
        p1 = 0x40u;
        memory[0x11ee] = 0u;
        memory[0x11ed] = 0u;
    }
}

static void TestLowStateMaintenance(void)
{
    ResetState();
    p1 = 0x40u;
    memory[0x11ee] = 0u;
    memory[0x11ed] = 0x40u;
    memory[0x11ef] = 0xffu;
    memory[0x130a] = 12u;
    memory[0x1309] = 0x55u;

    TimerDispatchPhase0();

    assert(memory[0x11ee] == 0xbfu);
    assert(memory[0x11ed] == 0u);
    assert(memory[0xfaed] == 0x20u);
    assert(memory[0x11ef] == 0x9fu);
    assert(memory[0x120c] == 7u && memory[0x1309] == 0u);
    assert(transition_calls == 0u);
}

static void TestLowStateTransitionRoutes(void)
{
    ResetState();
    SetStableBit6(0u);
    memory[0xfaed] = 2u;
    memory[0xfaef] = 0x10u;
    transition_replacement = 0x55u;
    TimerDispatchPhase0();
    assert(transition_calls == 1u && memory[0xfaed] == 0x22u);
    assert(memory[0x11ed] == 0x55u);

    ResetState();
    SetStableBit6(0u);
    memory[0xfaed] = 2u;
    TimerDispatchPhase0();
    assert(transition_calls == 1u && memory[0xfaed] == 0x22u);

    ResetState();
    SetStableBit6(0u);
    memory[0xfaed] = 6u;
    TimerDispatchPhase0();
    assert(transition_calls == 0u && memory[0xfaed] == 0x26u);
}

static void TestHighRisingEdgeRoutes(void)
{
    ResetState();
    p1 = 0u;
    memory[0x11ee] = 0x40u;
    memory[0x11ed] = 0u;
    memory[0xfaed] = 0x20u;
    TimerDispatchPhase0();
    assert(memory[0x11ed] == 0x40u);
    assert(memory[0xfaed] == 0u && transition_calls == 1u);

    ResetState();
    p1 = 0u;
    memory[0x11ee] = 0x40u;
    memory[0x11ed] = 0u;
    memory[0xfaed] = 0x21u;
    memory[0x0c43] = 9u;
    TimerDispatchPhase0();
    assert(transition_calls == 0u);
    assert(memory[0xfaed] == 3u && memory[0x1210] == 30u);
}

static void TestHighRisingEdgeUsesDriveState(void)
{
    unsigned drive_state;
    unsigned request_code;

    /* Stock RAM:fd1b85 reads RAM:001300, independent of RAM:000c43.
     * Distinct seeds prevent a zero/zero smoke test from hiding this error. */
    ResetState();
    for(drive_state = 0; drive_state < 256u; ++drive_state)
    {
        for(request_code = 0; request_code < 256u; ++request_code)
        {
            p1 = 0u;
            memory[0x11ed] = 0u;
            memory[0x11ee] = 0x40u;
            memory[0xfaed] = 0x21u;
            memory[0xfaef] = 0u;
            memory[0x1210] = 1u;
            memory[0x120c] = 1u;
            memory[0x1300] = (u8)drive_state;
            memory[0x0c43] = (u8)request_code;
            transition_calls = 0u;
            TimerDispatchPhase0();
            assert(transition_calls == (drive_state == 10u ? 1u : 0u));
            assert(memory[0x1210] == (drive_state == 10u ? 1u : 30u));
            assert(memory[0x1300] == (u8)drive_state);
            assert(memory[0x0c43] == (u8)request_code);
        }
    }
}

static void TestHighHandshakeTimeouts(void)
{
    ResetState();
    SetStableBit6(1u);
    memory[0xfaed] = 2u;
    memory[0x1210] = 0u;
    TimerDispatchPhase0();
    assert(memory[0xfaed] == 0x0eu && memory[0x0c43] == 0x34u);

    ResetState();
    SetStableBit6(1u);
    memory[0xfaed] = 0xffu;
    memory[0xfaef] = 0x10u;
    memory[0x1211] = 0u;
    memory[0x1308] = 1u;
    TimerDispatchPhase0();
    assert(memory[0x1308] == 0u && memory[0xfaef] == 0x35u);
    assert(memory[0xfaed] == 0xf3u && memory[0x0c43] == 0x28u);
}

static void TestHighDriveCountdownExpiry(void)
{
    ResetState();
    SetStableBit6(1u);
    memory[0x120c] = 0xffu;
    memory[0x130a] = 12u;
    memory[0x1302] = 1u;
    TimerDispatchPhase0();
    assert(memory[0x11ef] == 0x40u && memory[0x1302] == 0x41u);
}

int main(void)
{
#if defined(_MSC_VER)
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    TestLowStateMaintenance();
    TestLowStateTransitionRoutes();
    TestHighRisingEdgeRoutes();
    TestHighRisingEdgeUsesDriveState();
    TestHighHandshakeTimeouts();
    TestHighDriveCountdownExpiry();
    puts("timer phase-zero tests passed");
    return 0;
}
