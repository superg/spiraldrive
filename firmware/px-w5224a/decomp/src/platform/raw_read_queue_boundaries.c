#include "coroutine_initialization.h"
#include "firmware_code_addresses.h"
#include "mcs251.h"

u8 bdata RawReadQueueBitByte22 _at_ 0x22;
sbit RawReadQueueBit0011 = RawReadQueueBitByte22 ^ 1;

void PLEXTOR_NEAR_FUNCTION ClearBit0011Atomic(void)
{
    RawReadQueueBit0011 = 0;
}

void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF84A00ForC(void)
{
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
}

void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF90000ForC(void)
{
    InitializeCoroutine2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
}

void PLEXTOR_NEAR_FUNCTION InitializeCoroutine1AtF85D74ForC(void)
{
    InitializeCoroutine1(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1);
}

void PLEXTOR_NEAR_FUNCTION InitializeCoroutine1AtF84B0BForC(void)
{
    InitializeCoroutine1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
}

void PLEXTOR_NEAR_FUNCTION InitializeCoroutine3AtF84B00ForC(void)
{
    InitializeCoroutine3(FW_ENTRY_IDLE_COROUTINE3);
}

void PLEXTOR_NEAR_FUNCTION InitializeCoroutine1AtF852EBForC(void)
{
    InitializeCoroutine1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COMPLETION_COROUTINE1);
}
