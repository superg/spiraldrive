#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveOperatingStateResetState(void);
extern void ObserveOperatingStateResetHardware(void);
extern void ObserveOperatingStateConfigureParameterState2(void);
extern u8 ObserveOperatingStateIsIndexedState(u8 state_code);
extern void ObserveOperatingStateUpdateOutcome(u8 stage);
extern void ObserveOperatingStateSetP3LowHigh(void);
extern void ObserveOperatingStateSelectParameterFb(void);
extern void ObserveOperatingStateWaitConfigurationBit7(void);
extern u8 ObserveOperatingStateEstablishState6(void);
extern void ObserveOperatingStateApplyChannels(void);
extern u8 ObserveOperatingStateValidateState5(void);
extern u8 ObserveOperatingStateRunCurrentCalibration(void);
extern void ObserveOperatingStateRunCommand03(void);
extern void ObserveOperatingStateLoadProfile(u8 profile_index);
extern void ObserveOperatingStateWriteRegister01(u8 value);
extern void ObserveOperatingStateSetP1Pins(void);
extern void ObserveOperatingStatePulseWord(u16 duration);
extern u16 ObserveOperatingStateReadWord(u16 selector);
extern void ObserveOperatingStateSubmitWord(u16 value, u16 selector);
extern u8 ObserveOperatingStateRetryPortCalibration(u8 mode);
extern void ObserveOperatingStateSetChannel(u8 channel, u8 value);
extern void ObserveOperatingStateConfigureRange(void);
extern void ObserveOperatingStateConfigureCode4(void);
#define STATE_INIT_RESET_STATE() ObserveOperatingStateResetState()
#define STATE_INIT_RESET_HARDWARE() ObserveOperatingStateResetHardware()
#define STATE_INIT_CONFIGURE_PARAMETER_STATE2() ObserveOperatingStateConfigureParameterState2()
#define STATE_INIT_IS_INDEXED(state) ObserveOperatingStateIsIndexedState(state)
#define STATE_INIT_UPDATE_OUTCOME(stage) ObserveOperatingStateUpdateOutcome(stage)
#define STATE_INIT_SET_P3_LOW_HIGH() ObserveOperatingStateSetP3LowHigh()
#define STATE_INIT_SELECT_FB() ObserveOperatingStateSelectParameterFb()
#define STATE_INIT_WAIT_CONFIGURATION_BIT7() ObserveOperatingStateWaitConfigurationBit7()
#define STATE_INIT_ESTABLISH_STATE6() ObserveOperatingStateEstablishState6()
#define STATE_INIT_APPLY_CHANNELS() ObserveOperatingStateApplyChannels()
#define STATE_INIT_VALIDATE_STATE5() ObserveOperatingStateValidateState5()
#define STATE_INIT_RUN_CURRENT_CALIBRATION() ObserveOperatingStateRunCurrentCalibration()
#define STATE_INIT_RUN_COMMAND03() ObserveOperatingStateRunCommand03()
#define STATE_INIT_LOAD_PROFILE(index) ObserveOperatingStateLoadProfile(index)
#define STATE_INIT_WRITE_REGISTER01(value) ObserveOperatingStateWriteRegister01(value)
#define STATE_INIT_SET_P1_PINS() ObserveOperatingStateSetP1Pins()
#define STATE_INIT_PULSE_WORD(duration) ObserveOperatingStatePulseWord(duration)
#define STATE_INIT_READ_WORD(selector) ObserveOperatingStateReadWord(selector)
#define STATE_INIT_SUBMIT_WORD(value, selector) ObserveOperatingStateSubmitWord((value), (selector))
#define STATE_INIT_RETRY_PORT(mode) ObserveOperatingStateRetryPortCalibration(mode)
#define STATE_INIT_SET_CHANNEL(channel, value) ObserveOperatingStateSetChannel((channel), (value))
#define STATE_INIT_CONFIGURE_RANGE() ObserveOperatingStateConfigureRange()
#define STATE_INIT_CONFIGURE_CODE4() ObserveOperatingStateConfigureCode4()
#else
#include "platform/controller_calibration_outcome.h"
#include "platform/controller_command03.h"
#include "platform/controller_current_state_calibration.h"
#include "platform/controller_parameters.h"
#include "platform/controller_state6_operating_path.h"
#include "platform/drive_control_reset.h"
#include "platform/leaf_workers.h"
#include "platform/media_interrupt_leaves.h"
#include "platform/status_mapping.h"
extern void SetP3Bit1Atomic(void);
extern void ClearP3Bit0Atomic(void);
#define STATE_INIT_RESET_STATE() ResetControllerOperatingState()
#define STATE_INIT_RESET_HARDWARE() ResetControllerHardwareRegisters()
#define STATE_INIT_CONFIGURE_PARAMETER_STATE2() ConfigureControllerForParameterState2()
#define STATE_INIT_IS_INDEXED(state) IsIndexedDriveStateActive(state)
#define STATE_INIT_UPDATE_OUTCOME(stage) UpdateCalibrationOutcomeStateForStage(stage)
#define STATE_INIT_SET_P3_LOW_HIGH()                                                                                                                                                                   \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ClearP3Bit0Atomic();                                                                                                                                                                           \
        SetP3Bit1Atomic();                                                                                                                                                                             \
    } while(0)
#define STATE_INIT_SELECT_FB() ((void)SelectAndWriteControllerParameterFBValue())
#define STATE_INIT_WAIT_CONFIGURATION_BIT7() WaitForConfigurationBit7ClearAndRestartControllerState()
#define STATE_INIT_ESTABLISH_STATE6() EstablishControllerDriveState6OperatingPath()
#define STATE_INIT_APPLY_CHANNELS() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define STATE_INIT_VALIDATE_STATE5() ValidateControllerState5AndCalibrateWord0319()
#define STATE_INIT_RUN_CURRENT_CALIBRATION() RunControllerCalibrationForCurrentDriveState()
#define STATE_INIT_RUN_COMMAND03() RunControllerCommand03AndUpdateWord03AF()
#define STATE_INIT_LOAD_PROFILE(index) LoadControllerOperatingProfile(index)
#define STATE_INIT_WRITE_REGISTER01(value) WriteControllerRegister01Clamped(value)
#define STATE_INIT_SET_P1_PINS() SetP1Bits0And1()
#define STATE_INIT_PULSE_WORD(duration) PulseControllerWord0253Value7F00ForDuration(duration)
#define STATE_INIT_READ_WORD(selector) ReadControllerWord(selector)
#define STATE_INIT_SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define STATE_INIT_RETRY_PORT(mode) RetryControllerPort80F8Calibration(mode)
#define STATE_INIT_SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#define STATE_INIT_CONFIGURE_RANGE() ConfigureControllerPorts8496Through8499ForOperatingRange()
#define STATE_INIT_CONFIGURE_CODE4() ConfigureControllerOperatingCode4AndChannel3()
#endif
#endif
