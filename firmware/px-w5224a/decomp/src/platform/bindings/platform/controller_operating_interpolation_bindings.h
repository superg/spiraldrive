#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define DIVIDE_OPERATING_PROFILE(value, divisor) ((value) / (divisor))
#else
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
#define DIVIDE_OPERATING_PROFILE(value, divisor) DivideU32Quotient((value), (divisor))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u16 ObserveLookupControllerOperatingAdjustment(void);
extern void ObserveWriteControllerOperatingInterpolationEaEb(u16 value);
extern void ObserveWriteControllerOperatingInterpolationEf(u8 value);
extern u16 ObserveCaptureControllerProfileSelectorInputs(u8 radial_coordinate);
extern u8 ObserveComputeControllerProfileByteFromCoordinate(u8 radial_coordinate, u16 radial_endpoint);
extern u8 ObserveCaptureInheritedR15ForSecondaryProfileUpdate(void);
extern u8 ObserveSelectControllerProfileForSecondaryUpdate(u8 radial_coordinate, u8 inherited_profile);
extern void ObserveApplySecondaryControllerRegisterProfile(u8 profile);
extern u8 ObserveReadBit0038ForSecondaryProfileUpdate(void);
extern void ObserveInterpolateSecondaryControllerProfile(u8 profile);
extern void ObserveWriteSecondaryControllerParameter(u16 value);
extern u8 ObserveCaptureInheritedR15ForMediaState5ProfileUpdate(void);
extern u8 ObserveDivideMediaState5PositionBy4500(s32 position);
extern u8 ObserveSelectControllerProfileForMediaState5Update(u8 radial_coordinate, u8 inherited_profile);
extern void ObserveApplyMediaState5ControllerRegisterProfile(u8 profile);
extern void ObserveComputeOperatingCode14ControllerMeasurementBounds(void);
extern void RunControllerOperatingCodeSelection(void);
#define LOOKUP_OPERATING_ADJUSTMENT() ObserveLookupControllerOperatingAdjustment()
#define WRITE_EA_EB(value) ObserveWriteControllerOperatingInterpolationEaEb(value)
#define WRITE_EF(value) ObserveWriteControllerOperatingInterpolationEf(value)
#define CAPTURE_CONTROLLER_PROFILE_SELECTOR_INPUTS(coordinate) ObserveCaptureControllerProfileSelectorInputs(coordinate)
#define COMPUTE_CONTROLLER_PROFILE_BYTE(coordinate, endpoint) ObserveComputeControllerProfileByteFromCoordinate((coordinate), (endpoint))
#define CAPTURE_SECONDARY_PROFILE_INHERITED_R15() ObserveCaptureInheritedR15ForSecondaryProfileUpdate()
#define SELECT_SECONDARY_PROFILE(coordinate, inherited_profile) ObserveSelectControllerProfileForSecondaryUpdate((coordinate), (inherited_profile))
#define APPLY_SECONDARY_PROFILE(profile) ObserveApplySecondaryControllerRegisterProfile(profile)
#define READ_SECONDARY_PROFILE_INTERPOLATION_BIT() ObserveReadBit0038ForSecondaryProfileUpdate()
#define INTERPOLATE_SECONDARY_PROFILE(profile) ObserveInterpolateSecondaryControllerProfile(profile)
#define WRITE_SECONDARY_CONTROLLER_PARAMETER(value) ObserveWriteSecondaryControllerParameter(value)
#define CAPTURE_MEDIA_STATE5_PROFILE_INHERITED_R15() ObserveCaptureInheritedR15ForMediaState5ProfileUpdate()
#define DIVIDE_MEDIA_STATE5_POSITION_BY_4500(position) ObserveDivideMediaState5PositionBy4500(position)
#define SELECT_MEDIA_STATE5_PROFILE(coordinate, inherited_profile) ObserveSelectControllerProfileForMediaState5Update((coordinate), (inherited_profile))
#define APPLY_MEDIA_STATE5_PROFILE(profile) ObserveApplyMediaState5ControllerRegisterProfile(profile)
#define COMPUTE_OPERATING_CODE14_BOUNDS() ObserveComputeOperatingCode14ControllerMeasurementBounds()
#define ROUTE_CONTROLLER_OPERATING_CODE_RANGE() RunControllerOperatingCodeSelection()
#else
extern u16 LookupControllerOperatingAdjustmentForCurrentPosition(void);
extern u16 CaptureControllerProfileSelectorInputs(void);
extern u8 ComputeControllerProfileByteFromCoordinateForC(u8 radial_coordinate, u16 radial_endpoint);
extern u8 CaptureInheritedR15ForSecondaryControllerProfileUpdate(void);
extern u8 SelectControllerProfileForOperatingCodeFromC(u8 radial_coordinate, u8 inherited_profile);
extern u8 ReadBit0038ForSecondaryControllerProfileUpdate(void);
extern u8 CaptureInheritedR15ForMediaState5ProfileUpdate(void);
extern u8 DivideMediaState5PositionBy4500ForC(s32 position);
extern void PLEXTOR_NEAR_FUNCTION RouteControllerOperatingCodeRange(void);
#define LOOKUP_OPERATING_ADJUSTMENT() LookupControllerOperatingAdjustmentForCurrentPosition()
#define WRITE_EA_EB(value) WriteControllerParametersEAEBFromScaledValue(value)
#define WRITE_EF(value) WriteControllerParameterEF00AndConditionalD0Value(value)
#define CAPTURE_CONTROLLER_PROFILE_SELECTOR_INPUTS(coordinate) CaptureControllerProfileSelectorInputs()
#define COMPUTE_CONTROLLER_PROFILE_BYTE(coordinate, endpoint) ComputeControllerProfileByteFromCoordinateForC((coordinate), (endpoint))
#define CAPTURE_SECONDARY_PROFILE_INHERITED_R15() CaptureInheritedR15ForSecondaryControllerProfileUpdate()
#define SELECT_SECONDARY_PROFILE(coordinate, inherited_profile) SelectControllerProfileForOperatingCodeFromC((coordinate), (inherited_profile))
#define APPLY_SECONDARY_PROFILE(profile) ApplyControllerRegisterProfileSelector(profile)
#define READ_SECONDARY_PROFILE_INTERPOLATION_BIT() ReadBit0038ForSecondaryControllerProfileUpdate()
#define INTERPOLATE_SECONDARY_PROFILE(profile) ApplyControllerOperatingInterpolationAtIndex(profile)
#define WRITE_SECONDARY_CONTROLLER_PARAMETER(value) WriteControllerParameterWord(value)
#define CAPTURE_MEDIA_STATE5_PROFILE_INHERITED_R15() CaptureInheritedR15ForMediaState5ProfileUpdate()
#define DIVIDE_MEDIA_STATE5_POSITION_BY_4500(position) DivideMediaState5PositionBy4500ForC(position)
#define SELECT_MEDIA_STATE5_PROFILE(coordinate, inherited_profile) SelectControllerProfileForOperatingCodeFromC((coordinate), (inherited_profile))
#define APPLY_MEDIA_STATE5_PROFILE(profile) ApplyControllerRegisterProfileSelector(profile)
#define COMPUTE_OPERATING_CODE14_BOUNDS() ComputeOperatingCode14ControllerMeasurementBounds()
#define ROUTE_CONTROLLER_OPERATING_CODE_RANGE() RouteControllerOperatingCodeRange()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveClearTimer1OverflowForOperatingProfileScale(void);
extern void ObserveSetTimer1RunForOperatingProfileScale(void);
extern void ObserveClearTimer1RunForOperatingProfileScale(void);
extern u8 ObserveReadTimer1OverflowForOperatingProfileScale(void);
#define CLEAR_PROFILE_SCALE_TIMER_OVERFLOW() ObserveClearTimer1OverflowForOperatingProfileScale()
#define SET_PROFILE_SCALE_TIMER_RUN() ObserveSetTimer1RunForOperatingProfileScale()
#define CLEAR_PROFILE_SCALE_TIMER_RUN() ObserveClearTimer1RunForOperatingProfileScale()
#define READ_PROFILE_SCALE_TIMER_OVERFLOW() ObserveReadTimer1OverflowForOperatingProfileScale()
#else
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1OverflowAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetTimer1RunAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1RunAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadTimer1OverflowAtomic(void);
#define CLEAR_PROFILE_SCALE_TIMER_OVERFLOW() ClearTimer1OverflowAtomic()
#define SET_PROFILE_SCALE_TIMER_RUN() SetTimer1RunAtomic()
#define CLEAR_PROFILE_SCALE_TIMER_RUN() ClearTimer1RunAtomic()
#define READ_PROFILE_SCALE_TIMER_OVERFLOW() ReadTimer1OverflowAtomic()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteScaledOperatingProfileAfterResponse(u8 scale);
extern void ObserveClearTimer1OverflowAfterOperatingResponse(void);
extern void ObserveSetTimer1RunAfterOperatingResponse(void);
extern void ObserveClearTimer1RunAfterOperatingResponse(void);
extern u8 ObserveReadTimer1OverflowAfterOperatingResponse(void);
extern void ObserveWriteControllerParameterAfterOperatingResponse(u16 value);
extern u8 ObserveReadBit0044AfterOperatingResponse(void);
extern void ObserveWriteD4E2AfterOperatingResponse(u8 code);
extern void ObserveSetBit0038AfterOperatingResponse(void);
extern void ObserveClearBit003AAfterOperatingResponse(void);
#define RESPONSE_WRITE_SCALED_PROFILE(scale) ObserveWriteScaledOperatingProfileAfterResponse(scale)
#define RESPONSE_CLEAR_TIMER_OVERFLOW() ObserveClearTimer1OverflowAfterOperatingResponse()
#define RESPONSE_SET_TIMER_RUN() ObserveSetTimer1RunAfterOperatingResponse()
#define RESPONSE_CLEAR_TIMER_RUN() ObserveClearTimer1RunAfterOperatingResponse()
#define RESPONSE_READ_TIMER_OVERFLOW() ObserveReadTimer1OverflowAfterOperatingResponse()
#define RESPONSE_WRITE_PARAMETER(value) ObserveWriteControllerParameterAfterOperatingResponse(value)
#define RESPONSE_READ_BIT0044() ObserveReadBit0044AfterOperatingResponse()
#define RESPONSE_WRITE_D4_E2(code) ObserveWriteD4E2AfterOperatingResponse(code)
#define RESPONSE_SET_BIT0038() ObserveSetBit0038AfterOperatingResponse()
#define RESPONSE_CLEAR_BIT003A() ObserveClearBit003AAfterOperatingResponse()
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0044ForReadAcquisitionTarget(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0038ForOperatingProfileAfterResponse(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit003AForOperatingProfileAfterResponse(void);
extern void WriteControllerParametersD4AndE2ForCode(u8 selector_code);
#define RESPONSE_WRITE_SCALED_PROFILE(scale) WriteScaledOperatingProfileWordViaSfr8B8D(scale)
#define RESPONSE_CLEAR_TIMER_OVERFLOW() ClearTimer1OverflowAtomic()
#define RESPONSE_SET_TIMER_RUN() SetTimer1RunAtomic()
#define RESPONSE_CLEAR_TIMER_RUN() ClearTimer1RunAtomic()
#define RESPONSE_READ_TIMER_OVERFLOW() ReadTimer1OverflowAtomic()
#define RESPONSE_WRITE_PARAMETER(value) WriteControllerParameterWord(value)
#define RESPONSE_READ_BIT0044() ReadBit0044ForReadAcquisitionTarget()
#define RESPONSE_WRITE_D4_E2(code) WriteControllerParametersD4AndE2ForCode(code)
#define RESPONSE_SET_BIT0038() SetBit0038ForOperatingProfileAfterResponse()
#define RESPONSE_CLEAR_BIT003A() ClearBit003AForOperatingProfileAfterResponse()
#endif
#endif
