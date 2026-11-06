#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
static u32 FirmwareReadBigEndianU32Near(u16 address)
{
    return FirmwareReadNearU32(address);
}
#else
#define FirmwareReadBigEndianU32Near(address) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
static u16 FirmwareReadBigEndianU16Near(u16 address)
{
    return FirmwareReadNearU16(address);
}
#else
#define FirmwareReadBigEndianU16Near(address) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU32Near(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}
#else
#define FirmwareWriteBigEndianU32Near(address, value) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)) = (u32)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU16Near(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}
#else
#define FirmwareWriteBigEndianU16Near(address, value) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)) = (u16)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if !defined(PLEXTOR_HOST_TEST)
extern u16 DivideU16QuotientNative(u16 dividend, u16 divisor);
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#define MAP_SIGNED_FRAME_SECONDS MapSignedFrameSecondsToRadialModelValue
#define RAMP_CONTROLLER_WORD_0255 RampControllerWord0255TowardFeedbackValue
#define CLEAR_PENDING_TARGET_NUDGE ClearPendingReadAcquisitionTargetNudge
#define MAP_FRAME_POSITION_TO_RADIAL_FLOAT MapFramePositionToRadialModelFloat
#define SET_HIGH_OPERATING_CODE_THRESHOLDS SetHighOperatingCodeThresholds
#define ACQUIRE_RADIAL_SPAN_TARGET AcquireDiscPositionAtTargetWithRetryBudget
#define ACQUIRE_RADIAL_SPAN_POSITION AcquireDiscReadPosition
#define RESET_RADIAL_SPAN_CONTROLLER ResetDriveControlAndInitializeControllerCalibration
#define TEST_RADIAL_SPAN_POSITION IsDecodedFramePositionWithinTwoFramesOfTarget
#define MEASURE_RADIAL_SPAN_POSITION MeasureAverageControllerPositionWord
#define CALCULATE_RADIAL_SPAN CalculateRadialModelSpanFromMeasurement
#define DECODE_RADIAL_SPAN_ATIP DecodeAtipTimeFieldsAndFramePositions
#else
extern u16 InterceptMapSignedFrameSecondsForRadialInitialization(s32 signed_frame_seconds);
extern void InterceptRampControllerWord0255ForRadialInitialization(void);
extern void InterceptClearPendingTargetNudgeForRadialInitialization(void);
extern float ObserveMapFramePositionForRadialOperatingThresholds(s32 frame_position);
extern void ObserveSetHighOperatingCodeThresholdsForRadialModel(u8 operating_code);
extern u8 ObserveAcquireRadialSpanTarget(s32 target_frame_position);
extern u8 ObserveAcquireRadialSpanPosition(void);
extern u8 ObserveResetRadialSpanController(void);
extern u8 ObserveTestRadialSpanPosition(s32 target_frame_position);
extern u16 ObserveMeasureRadialSpanPosition(void);
extern u16 ObserveCalculateRadialSpan(u16 measurement);
extern void ObserveDecodeRadialSpanAtip(void);
#define MAP_SIGNED_FRAME_SECONDS InterceptMapSignedFrameSecondsForRadialInitialization
#define RAMP_CONTROLLER_WORD_0255 InterceptRampControllerWord0255ForRadialInitialization
#define CLEAR_PENDING_TARGET_NUDGE InterceptClearPendingTargetNudgeForRadialInitialization
#define MAP_FRAME_POSITION_TO_RADIAL_FLOAT ObserveMapFramePositionForRadialOperatingThresholds
#define SET_HIGH_OPERATING_CODE_THRESHOLDS ObserveSetHighOperatingCodeThresholdsForRadialModel
#define ACQUIRE_RADIAL_SPAN_TARGET ObserveAcquireRadialSpanTarget
#define ACQUIRE_RADIAL_SPAN_POSITION ObserveAcquireRadialSpanPosition
#define RESET_RADIAL_SPAN_CONTROLLER ObserveResetRadialSpanController
#define TEST_RADIAL_SPAN_POSITION ObserveTestRadialSpanPosition
#define MEASURE_RADIAL_SPAN_POSITION ObserveMeasureRadialSpanPosition
#define CALCULATE_RADIAL_SPAN ObserveCalculateRadialSpan
#define DECODE_RADIAL_SPAN_ATIP ObserveDecodeRadialSpanAtip

static u16 DivideU16QuotientNative(u16 dividend, u16 divisor)
{
    /* Focused host vectors cover the defined nonzero-divisor domain. */
    return divisor == 0u ? 0u : (u16)(dividend / divisor);
}

static s32 DivideS32QuotientNative(s32 dividend, s32 divisor)
{
    return dividend / divisor;
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if !defined(PLEXTOR_HOST_TEST)
extern u8 AcquireDiscReadPosition(void);
#endif
#endif
