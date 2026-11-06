#ifndef PLEXTOR_COROUTINE_CONTEXT_SWITCH_H
#define PLEXTOR_COROUTINE_CONTEXT_SWITCH_H

#include "mcs251.h"

void ResumeCoroutine0(void);
void ResumeCoroutine1(void);
void ResumeCoroutine2(void);
void ResumeCoroutine3(void);
void YieldCoroutine0(void);
void YieldCoroutine1(void);
void YieldCoroutine2(void);
void YieldCoroutine3(void);
void RestartCoroutine1AndYieldImpl(u32 entry_point);

#if defined(PLEXTOR_HOST_TEST)
typedef struct
{
    u16 current_stack_pointer;
    u16 scheduler_stack_pointer;
    u16 coroutine_stack_pointer[4];
    u32 current_dr12;
    u32 coroutine_dr12[4];
    u32 coroutine1_entry_point;
} CoroutineContextSwitchState;

extern CoroutineContextSwitchState CoroutineContextState;
#endif

#endif
