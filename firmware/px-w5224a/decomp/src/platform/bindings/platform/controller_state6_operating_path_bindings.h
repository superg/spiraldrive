#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveState6ProbeController(void);
extern u8 ObserveState6ValidateCalibrationPath(void);
extern u8 ObserveState6EnterControllerState5(void);
extern void ObserveState6HomePickup(void);
extern void ObserveState6ConfigureProfile(u8 enabled, u16 profile_value);
extern void ObserveState6WriteControllerPort(fw_addr_t address, u8 value);
extern u8 ObserveState6RetryPortCalibration(u8 calibration_mode);
extern void ObserveState6SetP1TimerPins(void);
extern void ObserveState6PulseControllerWord(u16 duration_units);
extern void ObserveState6SetIndexedChannel(u8 channel, u8 value);
extern void ObserveState6ResetIndexedChannel(u8 channel);
extern u8 ObserveState6IsIndexedState(u8 state_code);
extern void ObserveState6ClearP3Pins(void);
extern u8 ObserveState6CalibrateParameterFb(void);
extern void ObserveState6SetControllerFlag(u8 enabled);
extern u8 ObserveState6FinalizeCalibration(void);
extern void ObserveState6LoadOperatingProfile(u8 profile_index);
extern void ObserveState6ConfigurePort8006(u8 enabled);
extern void ObserveState6ApplyOperatingChannels(void);
extern void ObserveState6ConfigureOperatingRange(void);
extern u8 ObserveState6ReadP0Bit7(void);
#define STATE6_PROBE_CONTROLLER() ObserveState6ProbeController()
#define STATE6_VALIDATE_PATH() ObserveState6ValidateCalibrationPath()
#define STATE6_ENTER_STATE5() ObserveState6EnterControllerState5()
#define STATE6_HOME_PICKUP() ObserveState6HomePickup()
#define STATE6_CONFIGURE_PROFILE(enabled, value) ObserveState6ConfigureProfile((enabled), (value))
#define STATE6_WRITE_PORT(address, value) ObserveState6WriteControllerPort((address), (value))
#define STATE6_RETRY_CALIBRATION(mode) ObserveState6RetryPortCalibration(mode)
#define STATE6_SET_P1_PINS() ObserveState6SetP1TimerPins()
#define STATE6_PULSE_WORD(duration) ObserveState6PulseControllerWord(duration)
#define STATE6_SET_CHANNEL(channel, value) ObserveState6SetIndexedChannel((channel), (value))
#define STATE6_RESET_CHANNEL(channel) ObserveState6ResetIndexedChannel(channel)
#define STATE6_IS_INDEXED_STATE(state) ObserveState6IsIndexedState(state)
#define STATE6_CLEAR_P3_PINS() ObserveState6ClearP3Pins()
#define STATE6_CALIBRATE_PARAMETER_FB() ObserveState6CalibrateParameterFb()
#define STATE6_SET_CONTROLLER_FLAG(enabled) ObserveState6SetControllerFlag(enabled)
#define STATE6_FINALIZE_CALIBRATION() ObserveState6FinalizeCalibration()
#define STATE6_LOAD_PROFILE(index) ObserveState6LoadOperatingProfile(index)
#define STATE6_CONFIGURE_PORT8006(enabled) ObserveState6ConfigurePort8006(enabled)
#define STATE6_APPLY_CHANNELS() ObserveState6ApplyOperatingChannels()
#define STATE6_CONFIGURE_RANGE() ObserveState6ConfigureOperatingRange()
#define STATE6_READ_P0_BIT7() ObserveState6ReadP0Bit7()
#else
#include "platform/controller_parameters.h"
#include "platform/controller_state_profile.h"
#include "platform/leaf_workers.h"
#include "platform/pickup_move_outputs.h"
#include "platform/status_mapping.h"
#include "startup/debounced_inputs.h"
extern u8 ConfigureControllerCalibrationPathAndValidatePort80C7(void);
extern u8 CalibrateControllerParameterFBFromSampleSeriesForState6(void);
extern void ClearP3Bit0Atomic(void);
extern void ClearP3Bit1Atomic(void);
#define STATE6_PROBE_CONTROLLER() ProbeControllerSignalAndInitializeDriveControl()
#define STATE6_VALIDATE_PATH() ConfigureControllerCalibrationPathAndValidatePort80C7()
#define STATE6_ENTER_STATE5() EnterControllerState5AndMeasurePort80A9Minimum()
#define STATE6_HOME_PICKUP() HomePickupAgainstP1Bit2AndResetEstimate()
#define STATE6_CONFIGURE_PROFILE(enabled, value) ConfigureControllerStateProfileFromFirmwareDefaults((enabled), (value))
#define STATE6_WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define STATE6_RETRY_CALIBRATION(mode) RetryControllerPort80F8Calibration(mode)
#define STATE6_SET_P1_PINS() SetP1Bits0And1()
#define STATE6_PULSE_WORD(duration) PulseControllerWord0253Value7F00ForDuration(duration)
#define STATE6_SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#define STATE6_RESET_CHANNEL(channel) ResetIndexedControllerChannel(channel)
#define STATE6_IS_INDEXED_STATE(state) IsIndexedDriveStateActive(state)
#define STATE6_CLEAR_P3_PINS()                                                                                                                                                                         \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ClearP3Bit0Atomic();                                                                                                                                                                           \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define STATE6_CALIBRATE_PARAMETER_FB() CalibrateControllerParameterFBFromSampleSeriesForState6()
#define STATE6_SET_CONTROLLER_FLAG(enabled) SetControllerFlagFC07Bit2AndPort8098Bit6(enabled)
#define STATE6_FINALIZE_CALIBRATION() FinalizeControllerState6OperatingCalibration()
#define STATE6_LOAD_PROFILE(index) LoadControllerOperatingProfile(index)
#define STATE6_CONFIGURE_PORT8006(enabled) ConfigureControllerPort8006AndChannel3(enabled)
#define STATE6_APPLY_CHANNELS() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define STATE6_CONFIGURE_RANGE() ConfigureControllerPorts8496Through8499ForOperatingRange()
#define STATE6_READ_P0_BIT7() FirmwareReadBit0407()
#endif
#endif
