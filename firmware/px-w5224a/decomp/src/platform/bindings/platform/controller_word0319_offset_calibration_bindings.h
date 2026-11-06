#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveWord0319PulseTest(u8 sample_count);
extern void ObserveWord0319SetChannel(u8 channel, u8 value);
extern void ObserveWord0319WritePort(fw_addr_t address, u8 value);
extern u16 ObserveWord0319ReadControllerWord(u16 selector);
extern void ObserveWord0319SubmitWord(u16 value, u16 selector);
extern void ObserveWord0319UpdateConfiguration(void);
extern u8 ObserveWord0319LoadParameterFC(void);
extern void ObserveWord0319ResetChannel(u8 channel);
extern void ObserveWord0319EnterMeasurement(void);
extern u8 ObserveWord0319RunCommand17(void);
extern s16 ObserveWord0319Divide(s16 dividend, u16 divisor);
extern u8 ObserveWord0319IsIndexedState(u8 state);
extern void ObserveWord0319UpdateOutcome(u8 stage);
extern void ObserveWord0319LeaveMeasurement(void);
extern void ObserveWord0319Wait(u16 duration, u8 yield_while_waiting);
extern void ObserveWord0319ClearD1Bit6(void);
extern void ObserveWord0319WriteD2D3(void);
extern void ObserveWord0319WritePort80E4(void);
extern void ObserveWord0319ApplyD1(void);
extern void ObserveWord0319WriteD1D2D3(void);
extern u8 ObserveWord0319MapChannel3(u8 operating_code);
extern void ObserveWord0319SelectParameterFC(u8 operating_code);
#define PULSE_TEST(count) ObserveWord0319PulseTest(count)
#define SET_CHANNEL(channel, value) ObserveWord0319SetChannel((channel), (value))
#define WRITE_PORT(address, value) ObserveWord0319WritePort((address), (value))
#define READ_CONTROLLER_WORD(selector) ObserveWord0319ReadControllerWord(selector)
#define SUBMIT_WORD(value, selector) ObserveWord0319SubmitWord((value), (selector))
#define UPDATE_CONFIGURATION() ObserveWord0319UpdateConfiguration()
#define LOAD_PARAMETER_FC() ObserveWord0319LoadParameterFC()
#define RESET_CHANNEL(channel) ObserveWord0319ResetChannel(channel)
#define ENTER_MEASUREMENT() ObserveWord0319EnterMeasurement()
#define RUN_COMMAND_17() ObserveWord0319RunCommand17()
#define DIVIDE_SIGNED(value, divisor) ObserveWord0319Divide((value), (divisor))
#define IS_INDEXED_STATE(state) ObserveWord0319IsIndexedState(state)
#define UPDATE_OUTCOME(stage) ObserveWord0319UpdateOutcome(stage)
#define LEAVE_MEASUREMENT() ObserveWord0319LeaveMeasurement()
#define WAIT_DURATION(duration, yield_mode) ObserveWord0319Wait((duration), (yield_mode))
#define CLEAR_D1_BIT6() ObserveWord0319ClearD1Bit6()
#define WRITE_D2_D3() ObserveWord0319WriteD2D3()
#define WRITE_PORT_80E4() ObserveWord0319WritePort80E4()
#define APPLY_D1() ObserveWord0319ApplyD1()
#define WRITE_D1_D2_D3() ObserveWord0319WriteD1D2D3()
#define MAP_CHANNEL3(code) ObserveWord0319MapChannel3(code)
#define SELECT_PARAMETER_FC(code) ObserveWord0319SelectParameterFC(code)
#else
extern s16 DivideS16Quotient(s16 dividend, s16 divisor);
#define PULSE_TEST(count) TestP1Cex0PulseSuccessThreshold(count)
#define SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#define WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define READ_CONTROLLER_WORD(selector) ReadControllerWord(selector)
#define SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define UPDATE_CONFIGURATION() UpdateConfigurationWordFAACBit7FromMeasurement()
#define LOAD_PARAMETER_FC() LoadOrCalibrateControllerParameterFC()
#define RESET_CHANNEL(channel) ResetIndexedControllerChannel(channel)
#define ENTER_MEASUREMENT() EnterControllerMeasurementConfiguration()
#define RUN_COMMAND_17() RunControllerCommand17AndReadPort80C7()
#define DIVIDE_SIGNED(value, divisor) DivideS16Quotient((value), (s16)(divisor))
#define IS_INDEXED_STATE(state) IsIndexedDriveStateActive(state)
#define UPDATE_OUTCOME(stage) UpdateCalibrationOutcomeStateForStage(stage)
#define LEAVE_MEASUREMENT() LeaveControllerMeasurementConfiguration()
#define WAIT_DURATION(duration, yield_mode) WaitTimer1ScaledDuration((duration), (yield_mode))
#define CLEAR_D1_BIT6() ClearControllerParameterD1ShadowBit6()
#define WRITE_D2_D3() WriteControllerParametersD2AndD3FromState()
#define WRITE_PORT_80E4() WriteControllerPort80E4Value03AndWait()
#define APPLY_D1() ApplyControllerParameterD1ModeBits()
#define WRITE_D1_D2_D3() WriteControllerParametersD1D2D3FromConfiguration()
#define MAP_CHANNEL3(code) MapOperatingCodeToControllerChannel3Value(code)
#define SELECT_PARAMETER_FC(code) SelectAndWriteControllerParameterFCForOperatingCode(code)
#endif
#endif
