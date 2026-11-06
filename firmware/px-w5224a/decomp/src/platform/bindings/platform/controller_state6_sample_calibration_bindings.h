#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveState6SamplesWriteD4E2(u8 code);
extern void ObserveState6SamplesYield(void);
extern void ObserveState6SamplesWriteFC(u8 value);
extern void ObserveState6SamplesWriteFB(u8 value);
extern void ObserveState6SamplesSubmitWord(u16 value, u16 selector);
extern void ObserveState6SamplesSelectFC(u8 operating_code);
extern void ObserveState6SamplesRestoreD412(void);
extern u8 ObserveState6SamplesIsIndexed(u8 state);
extern u8 ObserveState6SamplesSelectFB(void);
extern void ObserveState6SamplesSetChannel(u8 channel, u8 value);
extern void ObserveState6SamplesClearD1(void);
extern void ObserveState6SamplesWriteD2D3(void);
extern void ObserveState6SamplesWaitPortE4(void);
extern void ObserveState6SamplesApplyD1(void);
extern void ObserveState6SamplesWriteD1D2D3(void);
extern void ObserveState6SamplesConfigureRange(void);
extern void ObserveState6SamplesRefreshFB(void);
extern u8 ObserveState6SamplesTestPulse(u8 count);
#define S6_WRITE_D4_E2(code) ObserveState6SamplesWriteD4E2(code)
#define S6_YIELD() ObserveState6SamplesYield()
#define S6_WRITE_FC(value) ObserveState6SamplesWriteFC(value)
#define S6_WRITE_FB(value) ObserveState6SamplesWriteFB(value)
#define S6_SUBMIT_WORD(value, selector) ObserveState6SamplesSubmitWord((value), (selector))
#define S6_SELECT_FC(code) ObserveState6SamplesSelectFC(code)
#define S6_RESTORE_D412() ObserveState6SamplesRestoreD412()
#define S6_IS_INDEXED(state) ObserveState6SamplesIsIndexed(state)
#define S6_SELECT_FB() ObserveState6SamplesSelectFB()
#define S6_SET_CHANNEL(channel, value) ObserveState6SamplesSetChannel((channel), (value))
#define S6_CLEAR_D1() ObserveState6SamplesClearD1()
#define S6_WRITE_D2_D3() ObserveState6SamplesWriteD2D3()
#define S6_WAIT_PORT_E4() ObserveState6SamplesWaitPortE4()
#define S6_APPLY_D1() ObserveState6SamplesApplyD1()
#define S6_WRITE_D1_D2_D3() ObserveState6SamplesWriteD1D2D3()
#define S6_CONFIGURE_RANGE() ObserveState6SamplesConfigureRange()
#define S6_REFRESH_FB() ObserveState6SamplesRefreshFB()
#define S6_TEST_PULSE(count) ObserveState6SamplesTestPulse(count)
#else
#include "platform/controller_parameters.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine1(void);
#define S6_WRITE_D4_E2(code) WriteControllerParametersD4AndE2ForCode(code)
#define S6_YIELD() YieldCoroutine1()
#define S6_WRITE_FC(value) WriteControllerParameterFCValue(value)
#define S6_WRITE_FB(value) WriteControllerParameterFBValueWithBit0Set(value)
#define S6_SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define S6_SELECT_FC(code) SelectAndWriteControllerParameterFCForOperatingCode(code)
#define S6_RESTORE_D412() WriteControllerParametersD412AndE2Value58()
#define S6_IS_INDEXED(state) IsIndexedDriveStateActive(state)
#define S6_SELECT_FB() SelectAndWriteControllerParameterFBValue()
#define S6_SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#define S6_CLEAR_D1() ClearControllerParameterD1ShadowBit6()
#define S6_WRITE_D2_D3() WriteControllerParametersD2AndD3FromState()
#define S6_WAIT_PORT_E4() WriteControllerPort80E4Value03AndWait()
#define S6_APPLY_D1() ApplyControllerParameterD1ModeBits()
#define S6_WRITE_D1_D2_D3() WriteControllerParametersD1D2D3FromConfiguration()
#define S6_CONFIGURE_RANGE() ConfigureControllerPorts8496Through8499ForOperatingRange()
#define S6_REFRESH_FB() RefreshControllerParameterFBIfSfrD8RangeWide()
#define S6_TEST_PULSE(count) TestP1Cex0PulseSuccessThreshold(count)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
static void WriteState6SharedCountdown(s32 value)
{
    u32 bits = (u32)value;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FC, (u8)(bits >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FD, (u8)(bits >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FE, (u8)(bits >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FF, (u8)bits);
}

static s32 ReadState6SharedCountdown(void)
{
    u32 bits = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_11FC) << 24;
    bits |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_11FD) << 16;
    bits |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_11FE) << 8;
    bits |= FirmwareReadNear8(FW_NEAR_UNKNOWN_11FF);
    return (s32)bits;
}
#define S6_WRITE_SHARED_COUNTDOWN(value) WriteState6SharedCountdown(value)
#define S6_READ_SHARED_COUNTDOWN() ReadState6SharedCountdown()
#else
#define S6_WRITE_SHARED_COUNTDOWN(value) (*(volatile s32 huge *)FW_NEAR_UNKNOWN_11FC = (value))
#define S6_READ_SHARED_COUNTDOWN() (*(volatile s32 huge *)FW_NEAR_UNKNOWN_11FC)
#endif
#endif
