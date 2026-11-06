#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetControllerPortsForDriveControlReset(void);
extern void ObserveWriteControllerPortForDriveControlReset(fw_addr_t port_address, u8 value);
extern void ObserveResolveDriveControlStateToZero(void);
extern void ObserveHomePickupForDriveControlReset(void);
extern void ObserveResetControllerHardwareForDriveControlReset(void);
extern void ObserveResetControllerOperatingStateForDriveControlReset(void);
extern void ObserveWriteControllerRegister01ForHardwareReset(u8 value);
#define RESET_CONTROLLER_PORTS() ObserveResetControllerPortsForDriveControlReset()
#define WRITE_CONTROLLER_PORT(address, value) ObserveWriteControllerPortForDriveControlReset((address), (value))
#define RESOLVE_DRIVE_CONTROL_STATE() ObserveResolveDriveControlStateToZero()
#define HOME_PICKUP() ObserveHomePickupForDriveControlReset()
#define RESET_CONTROLLER_HARDWARE() ObserveResetControllerHardwareForDriveControlReset()
#define RESET_CONTROLLER_OPERATING_STATE() ObserveResetControllerOperatingStateForDriveControlReset()
#define WRITE_CONTROLLER_REGISTER_01(value) ObserveWriteControllerRegister01ForHardwareReset(value)
#else
extern void ResolveDriveControlStateToZero(void);
extern void PLEXTOR_NEAR_FUNCTION HomePickupAgainstP1Bit2AndResetEstimate(void);
extern void ResetControllerHardwareRegisters(void);
extern void ResetControllerOperatingState(void);
#define RESET_CONTROLLER_PORTS() ResetControllerPorts8087Through808B()
#define WRITE_CONTROLLER_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define RESOLVE_DRIVE_CONTROL_STATE() ResolveDriveControlStateToZero()
#define HOME_PICKUP() HomePickupAgainstP1Bit2AndResetEstimate()
#define RESET_CONTROLLER_HARDWARE() ResetControllerHardwareRegisters()
#define RESET_CONTROLLER_OPERATING_STATE() ResetControllerOperatingState()
#define WRITE_CONTROLLER_REGISTER_01(value) WriteControllerRegister01Clamped(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR15ForResolveDriveControlState(void);
extern void ObserveResetControllerPortsForDriveControlResolution(void);
extern void ObserveRampControllerWord0255ForDriveControlResolution(void);
extern void ObserveResetControllerHardwareForDriveControlResolution(void);
extern void ObserveResetControllerOperatingStateForDriveControlResolution(void);
extern void ObserveFinalizeOperatingCodeForDriveControlResolution(s32 value);
extern void ObserveSetControllerFlagForDriveControlResolution(u8 enabled);
extern void ObserveWriteControllerParametersForDriveControlResolution(u8 value);
extern void ObserveClearExternalInterrupt1ForDriveControlResolution(void);
extern void ObserveResetFourChannelsForDriveControlResolution(void);
extern void ObserveSetP1BitsForDriveControlResolution(void);
extern void ObserveConfigureControllerPortsForDriveControlResolution(u8 enabled);
extern void ObserveLoadChannel3ProfileForDriveControlResolution(u8 code);
extern void ObserveWriteControllerPortForDriveControlResolution(fw_addr_t address, u8 value);
extern void ObservePulseControllerWord0253ForDriveControlResolution(u16 duration);
#define RESOLUTION_CAPTURE_R15() ObserveCaptureInheritedR15ForResolveDriveControlState()
#define RESOLUTION_RESET_PORTS() ObserveResetControllerPortsForDriveControlResolution()
#define RESOLUTION_RAMP_WORD_0255() ObserveRampControllerWord0255ForDriveControlResolution()
#define RESOLUTION_RESET_HARDWARE() ObserveResetControllerHardwareForDriveControlResolution()
#define RESOLUTION_RESET_OPERATING_STATE() ObserveResetControllerOperatingStateForDriveControlResolution()
#define RESOLUTION_FINALIZE_CODE(value) ObserveFinalizeOperatingCodeForDriveControlResolution(value)
#define RESOLUTION_SET_CONTROLLER_FLAG(enabled) ObserveSetControllerFlagForDriveControlResolution(enabled)
#define RESOLUTION_WRITE_PARAMETERS(value) ObserveWriteControllerParametersForDriveControlResolution(value)
#define RESOLUTION_CLEAR_EXTERNAL_INTERRUPT1() ObserveClearExternalInterrupt1ForDriveControlResolution()
#define RESOLUTION_RESET_CHANNELS() ObserveResetFourChannelsForDriveControlResolution()
#define RESOLUTION_SET_P1_BITS() ObserveSetP1BitsForDriveControlResolution()
#define RESOLUTION_CONFIGURE_PORTS(enabled) ObserveConfigureControllerPortsForDriveControlResolution(enabled)
#define RESOLUTION_LOAD_CHANNEL3(code) ObserveLoadChannel3ProfileForDriveControlResolution(code)
#define RESOLUTION_WRITE_PORT(address, value) ObserveWriteControllerPortForDriveControlResolution((address), (value))
#define RESOLUTION_PULSE_0253(duration) ObservePulseControllerWord0253ForDriveControlResolution(duration)
#else
extern u8 CaptureInheritedR15ForResolveDriveControlState(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0542Atomic(void);
#define RESOLUTION_CAPTURE_R15() CaptureInheritedR15ForResolveDriveControlState()
#define RESOLUTION_RESET_PORTS() ResetControllerPorts8087Through808B()
#define RESOLUTION_RAMP_WORD_0255() RampControllerWord0255TowardFeedbackValue()
#define RESOLUTION_RESET_HARDWARE() ResetControllerHardwareRegisters()
#define RESOLUTION_RESET_OPERATING_STATE() ResetControllerOperatingState()
#define RESOLUTION_FINALIZE_CODE(value) FinalizeControllerOperatingCodeTransition(value)
#define RESOLUTION_SET_CONTROLLER_FLAG(enabled) SetControllerFlagFC07Bit2AndPort8098Bit6(enabled)
#define RESOLUTION_WRITE_PARAMETERS(value) WriteControllerParametersE8E9ForValue(value)
#define RESOLUTION_CLEAR_EXTERNAL_INTERRUPT1() ClearBit0542Atomic()
#define RESOLUTION_RESET_CHANNELS() ResetFourIndexedControllerChannels()
#define RESOLUTION_SET_P1_BITS() SetP1Bits0And1()
#define RESOLUTION_CONFIGURE_PORTS(enabled) ConfigureControllerPorts8046And804EAnd80C9Through80CB(enabled)
#define RESOLUTION_LOAD_CHANNEL3(code) LoadControllerChannel3OperatingProfile(code)
#define RESOLUTION_WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define RESOLUTION_PULSE_0253(duration) PulseControllerWord0253Value8000ForDuration(duration)
#endif
#endif
