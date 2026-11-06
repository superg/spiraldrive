#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/coroutine_context_switch.h"

static void ResetState(void)
{
    memset(&CoroutineContextState, 0, sizeof(CoroutineContextState));
    CoroutineContextState.scheduler_stack_pointer = 0x0200u;
}

static void TestResumeAndYieldPair(u8 index, void (*resume)(void), void (*yield)(void))
{
    u16 scheduler_call_sp = (u16)(0x0210u + index);
    u16 coroutine_call_sp = (u16)(0x0400u + (u16)index * 0x0100u);
    u32 saved_dr12 = 0x12345670ul + index;

    ResetState();
    CoroutineContextState.current_stack_pointer = scheduler_call_sp;
    CoroutineContextState.coroutine_stack_pointer[index] = (u16)(coroutine_call_sp + 4u);
    CoroutineContextState.coroutine_dr12[index] = saved_dr12;
    CoroutineContextState.current_dr12 = 0xfffffffful;

    resume();
    assert(CoroutineContextState.scheduler_stack_pointer == scheduler_call_sp);
    assert(CoroutineContextState.current_stack_pointer == coroutine_call_sp);
    assert(CoroutineContextState.current_dr12 == saved_dr12);

    CoroutineContextState.current_dr12 ^= 0x00ff00fful;
    yield();
    assert(CoroutineContextState.coroutine_stack_pointer[index] == (u16)(coroutine_call_sp + 4u));
    assert(CoroutineContextState.coroutine_dr12[index] == (saved_dr12 ^ 0x00ff00fful));
    assert(CoroutineContextState.current_stack_pointer == scheduler_call_sp);
}

int main(void)
{
    TestResumeAndYieldPair(0u, ResumeCoroutine0, YieldCoroutine0);
    TestResumeAndYieldPair(1u, ResumeCoroutine1, YieldCoroutine1);
    TestResumeAndYieldPair(2u, ResumeCoroutine2, YieldCoroutine2);
    TestResumeAndYieldPair(3u, ResumeCoroutine3, YieldCoroutine3);

    ResetState();
    CoroutineContextState.current_stack_pointer = 0x0777u;
    CoroutineContextState.coroutine_dr12[1] = 0xfffffffful;
    RestartCoroutine1AndYieldImpl(0xf852ebul);
    assert(CoroutineContextState.coroutine1_entry_point == 0xf852ebul);
    assert(CoroutineContextState.coroutine_dr12[1] == 0u);
    assert(CoroutineContextState.coroutine_stack_pointer[1] == 0x0507u);
    assert(CoroutineContextState.current_stack_pointer == 0x0200u);

    puts("coroutine context switch tests passed");
    return 0;
}
