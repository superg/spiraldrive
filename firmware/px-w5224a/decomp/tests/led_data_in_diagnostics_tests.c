#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include "boot/led_boot_diagnostics.h"
#include "platform/firmware_memory.h"

#if defined(PLEXTOR_LED_BOOT_DIAGNOSTICS)
static jmp_buf stop_report;
static unsigned frame_count;
static u8 frames[2];
static u8 protocol;
static u8 opcode;
static u8 transfer_status;
extern u8 LedDiagnosticGlobalInterruptEnable;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == FW_NEAR_HOST_COMMAND_PROTOCOL) return protocol;
    if(address == FW_NEAR_HOST_COMMAND_OPCODE) return opcode;
    assert(address == FW_NEAR_UNKNOWN_0C33);
    return transfer_status;
}
void BusyWaitAccumulatorTimes2300(u8 count) { assert(count == 128u); }
void ObserveLedDiagnosticFrame(u8 count)
{
    if(frame_count == 2u) longjmp(stop_report, 1);
    frames[frame_count++] = count;
}
static void ExpectReport(void (*action)(void), u8 expected)
{
    frame_count = 0u;
    if(setjmp(stop_report) == 0)
    {
        action();
        assert(0); /* Armed outcome must stop, not return. */
    }
    assert(frame_count == 2u && frames[0] == expected && frames[1] == expected);
}
static void StartInquiry(void)
{
    /* Exercise retained outcome helpers without entering diag13's terminal
     * receipt stop. The real begin-command gate is checked separately. */
    protocol = 1u; opcode = 0x12u;
    LedBootDiagnosticInquiryStage = 1u;
    assert(LedBootDiagnosticInquiryStage == 1u);
}
#endif

int main(void)
{
#if defined(PLEXTOR_LED_BOOT_DIAGNOSTICS)
    unsigned value;
    LedBootDiagnosticArm();
    for(value = 0u; value < 256u; ++value)
    {
        protocol = 0u; opcode = (u8)value; /* Includes ATA IDENTIFY 0xa1. */
        LED_COMMAND_BEGIN();
        LED_DATA_IN_BEGIN();
        LED_INQUIRY_MARK(3u);
        LED_INQUIRY_MARK(4u);
        LED_DATA_IN_EXIT();
        LED_COMMAND_EXIT();
        LED_INQUIRY_RESET();
        assert(LedBootDiagnosticInquiryStage == 0u && frame_count == 0u);
        protocol = 1u;
        if(value == 0x12u)
        {
            LedDiagnosticGlobalInterruptEnable = 1u;
            ExpectReport(LedBootDiagnosticBeginCommand, 1u);
            assert(LedDiagnosticGlobalInterruptEnable == 0u);
            frame_count = 0u;
        }
        else
        {
            LED_COMMAND_BEGIN();
            assert(LedBootDiagnosticInquiryStage == 0u);
        }
        LedBootDiagnosticArm();
    }
    StartInquiry();
    LedDiagnosticGlobalInterruptEnable = 0u;
    LED_DATA_IN_BEGIN();
    assert(LedDiagnosticGlobalInterruptEnable == 0u);
    LedDiagnosticGlobalInterruptEnable = 1u;
    LED_INQUIRY_MARK(3u);
    assert(LedDiagnosticGlobalInterruptEnable == 1u);
    StartInquiry();
    LED_INQUIRY_MARK(4u); /* Ignore unrelated IRQ before DATA-IN begins. */
    assert(LedBootDiagnosticInquiryStage == 1u);
    ExpectReport(LedBootDiagnosticCommandExit, 1u);
    for(value = 2u; value <= 4u; ++value)
    {
        StartInquiry();
        LED_DATA_IN_BEGIN();
        LED_INQUIRY_MARK((u8)value);
        ExpectReport(LedBootDiagnosticReport, (u8)value);
    }
    StartInquiry();
    LED_DATA_IN_BEGIN();
    LED_INQUIRY_MARK(4u);
    LED_INQUIRY_MARK(3u); /* Programming milestone must not erase IRQ progress. */
    assert(LedBootDiagnosticInquiryStage == 4u);
    transfer_status = 13u;
    LED_DATA_IN_EXIT();
    assert(LedBootDiagnosticInquiryStage == 5u);
    LED_INQUIRY_MARK(4u);
    ExpectReport(LedBootDiagnosticReport, 5u);
    ExpectReport(LedBootDiagnosticCommandExit, 6u);
    for(value = 0u; value < 256u; ++value)
    {
        if(value == 13u) continue;
        StartInquiry();
        LED_DATA_IN_BEGIN();
        transfer_status = (u8)value;
        ExpectReport(LedBootDiagnosticDataInExit, 7u);
    }
    LedBootDiagnosticArm();
    assert(LedBootDiagnosticInquiryStage == 0u);
#else
    u8 side_effect = 0u;
    LED_COMMAND_BEGIN();
    LED_DATA_IN_BEGIN();
    LED_INQUIRY_MARK(++side_effect);
    LED_DATA_IN_EXIT();
    LED_COMMAND_EXIT();
    LED_INQUIRY_RESET();
    assert(side_effect == 0u);
#endif
    puts("LED INQUIRY receipt and progress tests passed");
    return 0;
}
