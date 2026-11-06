#include "led_boot_diagnostics.h"
#include "../platform/timing.h"
#include "../platform/firmware_memory.h"

/* The generic 80251 names for these port bits are RXD/TXD, but the stock
 * firmware uses them as a two-line status output. Their exact board wiring is
 * still unconfirmed. The reporter uses the same 01, 10, and 11 states seen in
 * stock code so it remains visible with either a common-anode or a two-lead
 * anti-parallel two-color LED. */
#if defined(PLEXTOR_HOST_TEST)
static u8 LedDiagnosticP3Bit0;
static u8 LedDiagnosticP3Bit1;
u8 LedDiagnosticGlobalInterruptEnable;
extern void ObserveLedDiagnosticFrame(u8 count);
#else
sbit LedDiagnosticP3Bit0 = 0xb0;
sbit LedDiagnosticP3Bit1 = 0xb1;
sbit LedDiagnosticGlobalInterruptEnable = 0xaf;
#endif
#if !defined(PLEXTOR_LED_STARTUP_DIAGNOSTICS)
volatile u8 LedBootDiagnosticInquiryStage;
#endif

#define LED_DIAGNOSTIC_STATE(bit0, bit1)                                                                                                                                                               \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        LedDiagnosticP3Bit0 = (bit0);                                                                                                                                                                  \
        LedDiagnosticP3Bit1 = (bit1);                                                                                                                                                                  \
    } while(0)

static void LedDiagnosticShortDelay(void)
{
    BusyWaitAccumulatorTimes2300(128u);
    BusyWaitAccumulatorTimes2300(128u);
    BusyWaitAccumulatorTimes2300(128u);
    BusyWaitAccumulatorTimes2300(128u);
}

/* This reporter deliberately uses a CPU busy wait rather than a peripheral
 * timer. The long pulse uses the first LED as the frame marker; count only the
 * following short pulses on the second LED. The routine leaves both lines
 * high, matching the stock idle state. */
void LedBootDiagnosticCheckpoint(u8 checkpoint)
{
    u8 pulse;

#if defined(PLEXTOR_HOST_TEST)
    ObserveLedDiagnosticFrame(checkpoint);
#endif

    LED_DIAGNOSTIC_STATE(0, 1);
    LedDiagnosticShortDelay();
    LedDiagnosticShortDelay();
    LedDiagnosticShortDelay();
    LED_DIAGNOSTIC_STATE(1, 1);
    LedDiagnosticShortDelay();
    LedDiagnosticShortDelay();

    for(pulse = 0u; pulse < checkpoint; ++pulse)
    {
        LED_DIAGNOSTIC_STATE(1, 0);
        LedDiagnosticShortDelay();
        LED_DIAGNOSTIC_STATE(1, 1);
        LedDiagnosticShortDelay();
    }

    LedDiagnosticShortDelay();
    LedDiagnosticShortDelay();
    LedDiagnosticShortDelay();
    LedDiagnosticShortDelay();
}

#if !defined(PLEXTOR_LED_STARTUP_DIAGNOSTICS)
void LedBootDiagnosticArm(void)
{
    LedBootDiagnosticInquiryStage = 0u;
}

void LedBootDiagnosticBeginCommand(void)
{
    LedBootDiagnosticInquiryStage = 0u;
    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PROTOCOL) == 1u &&
       FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x12u)
    {
        /* Diag13: report immediately, before admission/dispatch. Use a
         * literal count so later code or a mutable stage cannot reinterpret
         * this receipt marker. No transfer or completion is implied. */
        LedDiagnosticGlobalInterruptEnable = 0;
        LedBootDiagnosticInquiryStage = 1u;
        for(;;)
            LedBootDiagnosticCheckpoint(1u);
    }
}

/* Keep progress monotonic across foreground/ISR races. Mask only for the
 * byte update and restore the previous EA state; no delay runs here. */
void LedBootDiagnosticMark(u8 stage)
{
    u8 interrupts_enabled = LedDiagnosticGlobalInterruptEnable;
    LedDiagnosticGlobalInterruptEnable = 0;
    if(LedBootDiagnosticInquiryStage != 0u && LedBootDiagnosticInquiryStage < stage &&
       (stage != 4u || LedBootDiagnosticInquiryStage >= 2u))
        LedBootDiagnosticInquiryStage = stage;
    LedDiagnosticGlobalInterruptEnable = interrupts_enabled;
}

void LedBootDiagnosticReport(void)
{
    if(LedBootDiagnosticInquiryStage == 0u)
        return;
    LedDiagnosticGlobalInterruptEnable = 0;
    for(;;)
        LedBootDiagnosticCheckpoint(LedBootDiagnosticInquiryStage);
}

void LedBootDiagnosticDataInExit(void)
{
    if(LedBootDiagnosticInquiryStage == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 13u)
        LedBootDiagnosticInquiryStage = 5u;
    else
    {
        LedBootDiagnosticInquiryStage = 7u;
        LedBootDiagnosticReport();
    }
}

void LedBootDiagnosticCommandExit(void)
{
    if(LedBootDiagnosticInquiryStage == 5u)
        LedBootDiagnosticInquiryStage = 6u;
    LedBootDiagnosticReport();
}
#endif
