#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)

CoroutineContextSwitchState CoroutineContextState;

static void ResumeCoroutine(u8 index)
{
    CoroutineContextState.scheduler_stack_pointer = CoroutineContextState.current_stack_pointer;
    CoroutineContextState.current_dr12 = CoroutineContextState.coroutine_dr12[index];
    CoroutineContextState.current_stack_pointer = (u16)(CoroutineContextState.coroutine_stack_pointer[index] - 4u);
}

static void YieldCoroutine(u8 index)
{
    CoroutineContextState.coroutine_dr12[index] = CoroutineContextState.current_dr12;
    CoroutineContextState.coroutine_stack_pointer[index] = (u16)(CoroutineContextState.current_stack_pointer + 4u);
    CoroutineContextState.current_stack_pointer = CoroutineContextState.scheduler_stack_pointer;
}

void ResumeCoroutine0(void)
{
    ResumeCoroutine(0u);
}
void ResumeCoroutine1(void)
{
    ResumeCoroutine(1u);
}
void ResumeCoroutine2(void)
{
    ResumeCoroutine(2u);
}
void ResumeCoroutine3(void)
{
    ResumeCoroutine(3u);
}
void YieldCoroutine0(void)
{
    YieldCoroutine(0u);
}
void YieldCoroutine1(void)
{
    YieldCoroutine(1u);
}
void YieldCoroutine2(void)
{
    YieldCoroutine(2u);
}
void YieldCoroutine3(void)
{
    YieldCoroutine(3u);
}

void RestartCoroutine1AndYieldImpl(u32 entry_point)
{
    CoroutineContextState.coroutine1_entry_point = entry_point;
    CoroutineContextState.coroutine_dr12[1] = 0u;
    CoroutineContextState.coroutine_stack_pointer[1] = 0x0507u;
    CoroutineContextState.current_stack_pointer = CoroutineContextState.scheduler_stack_pointer;
}

#else

sfr CoroutineStackPointerLow = 0x81;
sfr CoroutineStackPointerHigh = 0xbe;
sbit CoroutineGlobalInterruptEnable = 0xaf;

#define COROUTINE_SCHEDULER_SP_ADDRESS 0x115au
#define COROUTINE0_SP_ADDRESS 0x115eu
#define COROUTINE1_SP_ADDRESS 0x1162u
#define COROUTINE2_SP_ADDRESS 0x1166u
#define COROUTINE3_SP_ADDRESS 0x116au

#define COROUTINE_EDATA_U16(address) (*(volatile u16 edata *)(address))
#define COROUTINE_EDATA_U32(address) (*(volatile u32 edata *)(address))
#define COROUTINE_DR12 (*(volatile u32 data *)0x000cu)
#define READ_COROUTINE_SP() ((u16)(((u16)CoroutineStackPointerHigh << 8) | CoroutineStackPointerLow))
#define WRITE_COROUTINE_SP(value, saved_interrupt_enable)                                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        (saved_interrupt_enable) = CoroutineGlobalInterruptEnable;                                                                                                                                     \
        CoroutineGlobalInterruptEnable = 0;                                                                                                                                                            \
        CoroutineStackPointerHigh = (u8)((value) >> 8);                                                                                                                                                \
        CoroutineStackPointerLow = (u8)(value);                                                                                                                                                        \
        CoroutineGlobalInterruptEnable = (saved_interrupt_enable);                                                                                                                                     \
    } while(0)

#define DEFINE_RESUME_COROUTINE(name, saved_sp_address)                                                                                                                                                \
    void name(void)                                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        u16 saved_sp = COROUTINE_EDATA_U16(saved_sp_address);                                                                                                                                          \
        u16 resumed_sp = (u16)(saved_sp - 4u);                                                                                                                                                         \
        u32 saved_dr12 = COROUTINE_EDATA_U32((u16)(saved_sp - 3u));                                                                                                                                    \
        u8 interrupt_enabled;                                                                                                                                                                          \
        COROUTINE_EDATA_U16(COROUTINE_SCHEDULER_SP_ADDRESS) = READ_COROUTINE_SP();                                                                                                                     \
        COROUTINE_DR12 = saved_dr12;                                                                                                                                                                   \
        WRITE_COROUTINE_SP(resumed_sp, interrupt_enabled);                                                                                                                                             \
    }

#define DEFINE_YIELD_COROUTINE(name, saved_sp_address)                                                                                                                                                 \
    void name(void)                                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        u16 current_sp = READ_COROUTINE_SP();                                                                                                                                                          \
        u16 scheduler_sp;                                                                                                                                                                              \
        u8 interrupt_enabled;                                                                                                                                                                          \
        COROUTINE_EDATA_U32((u16)(current_sp + 1u)) = COROUTINE_DR12;                                                                                                                                  \
        COROUTINE_EDATA_U16(saved_sp_address) = (u16)(current_sp + 4u);                                                                                                                                \
        scheduler_sp = COROUTINE_EDATA_U16(COROUTINE_SCHEDULER_SP_ADDRESS);                                                                                                                            \
        WRITE_COROUTINE_SP(scheduler_sp, interrupt_enabled);                                                                                                                                           \
    }

/* RAM:fee4fd */
DEFINE_RESUME_COROUTINE(ResumeCoroutine0, COROUTINE0_SP_ADDRESS)
/* RAM:fee508 */
DEFINE_RESUME_COROUTINE(ResumeCoroutine1, COROUTINE1_SP_ADDRESS)
/* RAM:fee513 */
DEFINE_RESUME_COROUTINE(ResumeCoroutine2, COROUTINE2_SP_ADDRESS)
/* RAM:fee51e */
DEFINE_RESUME_COROUTINE(ResumeCoroutine3, COROUTINE3_SP_ADDRESS)

/* RAM:fee529 */
DEFINE_YIELD_COROUTINE(YieldCoroutine0, COROUTINE0_SP_ADDRESS)
/* RAM:fee534 */
DEFINE_YIELD_COROUTINE(YieldCoroutine1, COROUTINE1_SP_ADDRESS)
/* RAM:fee53f */
DEFINE_YIELD_COROUTINE(YieldCoroutine2, COROUTINE2_SP_ADDRESS)
/* RAM:fee54a */
DEFINE_YIELD_COROUTINE(YieldCoroutine3, COROUTINE3_SP_ADDRESS)

/* RAM:fee559 */
void RestartCoroutine1AndYieldImpl(u32 entry_point)
{
    u16 scheduler_sp = COROUTINE_EDATA_U16(COROUTINE_SCHEDULER_SP_ADDRESS);
    u8 interrupt_enabled;

    COROUTINE_EDATA_U32(0x0500u) = entry_point;
    COROUTINE_EDATA_U16(COROUTINE1_SP_ADDRESS) = 0x0507u;
    WRITE_COROUTINE_SP(scheduler_sp, interrupt_enabled);
}

#endif
#endif
