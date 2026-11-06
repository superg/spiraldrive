#ifndef PLEXTOR_COROUTINE_ENTRIES_H
#define PLEXTOR_COROUTINE_ENTRIES_H

#include "mcs251.h"

void InitializeThenIdleCoroutine2(void);
void YieldCoroutine0Forever(void);
void RunIndexedState04MediaReadCoroutine0(void);
void RunMediaReadControlCoroutine3(void);
void IdleCoroutine3(void);
void LoadControllerBufferSetupCoroutineTail(void);
void PLEXTOR_NEAR_FUNCTION FinalizeRawReadPipelineAndIdleCoroutine2(void);
void RestartCoroutine1AndYield(u32 entry_point);

#endif
