#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveMediaState5ReadAcquisitionOperation(u8 operation, s32 argument);
#define STATE5_ACQUISITION_CALL(operation, argument) ObserveMediaState5ReadAcquisitionOperation((operation), (argument))
#else
#include "media/decoder.h"
#include "media/disc_read_position.h"
#include "platform/arithmetic.h"
#include "platform/controller_current_profile_configuration.h"
#include "platform/controller_operating_state_initialization.h"
#include "platform/controller_parameters.h"
#include "platform/controller_register_profile_generation.h"
#include "platform/controller_threshold_probe.h"
#include "platform/drive_control_reset.h"
#include "platform/leaf_workers.h"
#include "platform/radial_model_profile_coordinate.h"
#include "platform/read_acquisition_controller_config.h"
#include "platform/status_mapping.h"
#include "platform/write_speed_reduction.h"
extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
sbit MediaState5AcquisitionP0Bit7 = 0x87;
#define STATE5_ACQUISITION_NATIVE_CALL_I(operation, argument) STATE5_ACQUISITION_NATIVE_CALL_##operation(argument)
#define STATE5_ACQUISITION_NATIVE_CALL(operation, argument) STATE5_ACQUISITION_NATIVE_CALL_I(operation, argument)
#define STATE5_ACQUISITION_CALL(operation, argument) STATE5_ACQUISITION_NATIVE_CALL(operation, argument)
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_ACQUIRE_POSITION(argument) ((s32)AcquireDiscReadPosition())
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_RESET_DRIVE_CONTROL(argument) ResetDriveControlStateToEight()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_ACQ_INITIALIZE_CONTROLLER(argument) ((s32)InitializeControllerOperatingStateAndRunCalibration())
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_SET_OPERATING_ENABLE(argument) SetControllerOperatingEnableState((u8)(argument))
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_ACQ_UPDATE_SECONDARY_CODE(argument) UpdateSecondaryOperatingCodeForSupportedCurrentCode()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_CHECK_TOLERANCE(argument) ((s32)CheckControllerMeasurementToleranceOrTimeout((u8)(argument)))
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_STROBE_PORT_8080(argument) StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018080, (u8)(argument))
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_WRITE_PARAMETERS_FC_FD_F6_F7(argument) WriteControllerParametersFCFDF6F7FromCurrentState()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_GENERATE_PROFILES(argument) GenerateControllerRegisterProfiles((u16)(argument))
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_APPLY_OPERATING_CHANNELS(argument) ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_WAIT_FOR_POSITION(argument) ((s32)WaitForDecodedFramePositionPastTarget((s32)(argument)))
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_CONFIGURE_PORTS(argument) ConfigureControllerPortsAndParametersForCurrentOperatingCode((u8)(argument))
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_CONFIGURE_ACQUISITION_CODE(argument) ConfigureReadAcquisitionControllerForOperatingCode()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_READ_P0_BIT7(argument) ((s32)(u8)MediaState5AcquisitionP0Bit7)
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_REFRESH_PARAMETERS_FC_F6(argument) RefreshControllerParametersFCAndF6ForCurrentOperatingCode()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_CONFIGURE_ACQUISITION_STATE(argument) ConfigureReadAcquisitionControllerForCurrentState()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_CONFIGURE_REGISTER_PROFILE(argument) ConfigureControllerRegisterProfileForCurrentMediaState()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_PROBE_READINESS(argument) ((s32)RunControllerReadPathReadinessProbe())
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_PROBE_THRESHOLDS(argument) ((s32)ProbeControllerThresholdsAndMaybeReduceWriteSpeed())
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_ACQ_RESTORE_READ_PATH(argument) RestoreControllerReadPathConfiguration((u8)(argument))
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_ACCUMULATE_SPEED_FLAGS(argument) AccumulateWriteSpeedFlagsFromPositionAndOperatingCode()
#define STATE5_ACQUISITION_NATIVE_CALL_MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED(argument) ((s32)TryReduceWriteSpeedForCurrentMediaState((u8)(argument)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return (s32)(((u32)operating_code * (u32)multiplier) / FirmwareReadNear8(FW_NEAR_UNKNOWN_FAFC));
#else
return DivideS32Quotient((s32)MultiplyU32Low((u32)operating_code, (u32)multiplier), (s32)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAFC));
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
(void)discarded_endpoint;
return STATE5_ACQUISITION_CALL(MEDIA_STATE5_COMPUTE_RADIAL_COORDINATE, (s32)(((u32)profile_index << 16) | endpoint));
#else
if(discarded_endpoint == endpoint)
    return (s32)ComputeRadialModelProfileCoordinate(profile_index, endpoint);
return (s32)ComputeRadialModelProfileCoordinate(profile_index, endpoint);
#endif
#endif
