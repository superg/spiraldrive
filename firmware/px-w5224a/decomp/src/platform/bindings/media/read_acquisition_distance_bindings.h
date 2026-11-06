#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveConvertReadAcquisitionMsfTuple(fw_addr_t tuple_address);
extern void ObserveUpdateReadAcquisitionFrameDistanceFromMsfTuple(void);
extern u16 ObserveCalculateRadialModelDistanceToReadTarget(void);
extern u16 ObserveCalculateFineReadAcquisitionMoveMagnitude(void);
extern void ObserveMovePickupByScaledRadialDelta(u8 direction, u16 magnitude);
extern void ObserveMovePickupByDistanceInChunks(u8 direction, u16 distance);
extern void ObserveMovePickupByByteDistance(u8 distance, u8 direction);
extern void ObserveReadPositionRearmQCapture(void);
extern void ObserveReadPositionApplyChannels0Through3(void);
extern void ObserveReadPositionApplyChannels1Through3(void);
extern u8 ObserveReadPositionProcessQPacket(void);
extern void ObserveReadPositionYield(void);
extern u8 ObserveReadPositionPacketAccepted(void);
extern u8 ObserveReadPositionBit001c(void);
extern u8 ObserveReadPositionBit001d(void);
extern u8 ObserveReadPositionTimingHandshake(void);
extern void ObserveReadPositionSetControllerOperating(u8 enabled);
extern void ObserveReadPositionSelectOperatingCode(u32 threshold);
extern void ObserveReadPositionArmThreeByteCapture(void);
extern u8 ObserveReadPositionParseThreeBytePacket(void);
extern void ObservePositionPickupAdjustTarget(void);
extern void ObservePositionPickupUpdateDistance(void);
extern u8 ObservePositionPickupIssueMove(void);
extern u8 ObservePositionPickupSpecialStep(void);
extern u32 ObservePositionPickupAbsoluteDistance(void);
extern u8 ObservePositionPickupDistanceAccepted(void);
extern u8 ObservePositionPickupNearTarget(void);
extern u16 ObservePositionPickupRadialDistance(void);
extern void ObservePositionPickupYield(void);
extern void ObservePositionPickupScaledMove(u8 direction, u16 magnitude);

#define DispatchConvertReadAcquisitionMsfTuple(address) ObserveConvertReadAcquisitionMsfTuple(address)
#define DispatchUpdateReadAcquisitionFrameDistanceFromMsfTuple() ObserveUpdateReadAcquisitionFrameDistanceFromMsfTuple()
#define DispatchCalculateRadialModelDistanceToReadTarget() ObserveCalculateRadialModelDistanceToReadTarget()
#define DispatchCalculateFineReadAcquisitionMoveMagnitude() ObserveCalculateFineReadAcquisitionMoveMagnitude()
#define DispatchMovePickupByScaledRadialDelta(direction, magnitude) ObserveMovePickupByScaledRadialDelta((direction), (magnitude))
#define DispatchMovePickupByDistanceInChunks(direction, distance) ObserveMovePickupByDistanceInChunks((direction), (distance))
#define DispatchMovePickupByByteDistance(distance, direction) ObserveMovePickupByByteDistance((distance), (direction))
#define DispatchReadPositionRearmQCapture() ObserveReadPositionRearmQCapture()
#define DispatchReadPositionApplyChannels0Through3() ObserveReadPositionApplyChannels0Through3()
#define DispatchReadPositionApplyChannels1Through3() ObserveReadPositionApplyChannels1Through3()
#define DispatchReadPositionProcessQPacket() ObserveReadPositionProcessQPacket()
#define DispatchReadPositionYield() ObserveReadPositionYield()
#define DispatchReadPositionPacketAccepted() ObserveReadPositionPacketAccepted()
#define DispatchReadPositionBit001c() ObserveReadPositionBit001c()
#define DispatchReadPositionBit001d() ObserveReadPositionBit001d()
#define DispatchReadPositionTimingHandshake() ObserveReadPositionTimingHandshake()
#define DispatchReadPositionSetControllerOperating(enabled) ObserveReadPositionSetControllerOperating(enabled)
#define DispatchReadPositionSelectOperatingCode(threshold) ObserveReadPositionSelectOperatingCode(threshold)
#define DispatchReadPositionArmThreeByteCapture() ObserveReadPositionArmThreeByteCapture()
#define DispatchReadPositionParseThreeBytePacket() ObserveReadPositionParseThreeBytePacket()
#define DispatchPositionPickupAdjustTarget() ObservePositionPickupAdjustTarget()
#define DispatchPositionPickupUpdateDistance() ObservePositionPickupUpdateDistance()
#define DispatchPositionPickupIssueMove() ObservePositionPickupIssueMove()
#define DispatchPositionPickupSpecialStep() ObservePositionPickupSpecialStep()
#define DispatchPositionPickupAbsoluteDistance() ObservePositionPickupAbsoluteDistance()
#define DispatchPositionPickupDistanceAccepted() ObservePositionPickupDistanceAccepted()
#define DispatchPositionPickupNearTarget() ObservePositionPickupNearTarget()
#define DispatchPositionPickupRadialDistance() ObservePositionPickupRadialDistance()
#define DispatchPositionPickupYield() ObservePositionPickupYield()
#define DispatchPositionPickupScaledMove(direction, magnitude) ObservePositionPickupScaledMove((direction), (magnitude))
#else
extern s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address);
extern void PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionFrameDistanceFromMsfTuple(void);
extern u16 PLEXTOR_NEAR_FUNCTION CalculateRadialModelDistanceToReadTarget(void);
extern u16 PLEXTOR_NEAR_FUNCTION CalculateFineReadAcquisitionMoveMagnitude(void);
extern void MovePickupByScaledRadialDelta(u8 direction, u16 magnitude);
extern void MovePickupByDistanceInChunks(u8 direction, u16 distance);
extern void MovePickupByByteDistance(u8 distance, u8 direction);
extern void RearmControllerQPacketCaptureWithTimeout(void);
extern void ApplyCurrentOperatingCodeToControllerChannels0Through3(void);
extern void ApplyCurrentOperatingCodeToControllerChannels1Through3(void);
extern void YieldCoroutine1(void);
extern u8 IsReadAcquisitionPacketAcceptedForCurrentTocState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001cForReadTargetAdjustment(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001dForReadTargetAdjustment(void);
extern u8 ExecuteReadPositionTimingHandshake(void);
extern void SetControllerOperatingEnableState(u8 enabled);
extern void SelectAndConfigureOperatingCodeForThreshold(u32 threshold);
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);

#define DispatchConvertReadAcquisitionMsfTuple(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define DispatchUpdateReadAcquisitionFrameDistanceFromMsfTuple() UpdateReadAcquisitionFrameDistanceFromMsfTuple()
#define DispatchCalculateRadialModelDistanceToReadTarget() CalculateRadialModelDistanceToReadTarget()
#define DispatchCalculateFineReadAcquisitionMoveMagnitude() CalculateFineReadAcquisitionMoveMagnitude()
#define DispatchMovePickupByScaledRadialDelta(direction, magnitude) MovePickupByScaledRadialDelta((direction), (magnitude))
#define DispatchMovePickupByDistanceInChunks(direction, distance) MovePickupByDistanceInChunks((direction), (distance))
#define DispatchMovePickupByByteDistance(distance, direction) MovePickupByByteDistance((distance), (direction))
#define DispatchReadPositionRearmQCapture() RearmControllerQPacketCaptureWithTimeout()
#define DispatchReadPositionApplyChannels0Through3() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define DispatchReadPositionApplyChannels1Through3() ApplyCurrentOperatingCodeToControllerChannels1Through3()
#define DispatchReadPositionProcessQPacket() ProcessCapturedControllerQPacket()
#define DispatchReadPositionYield() YieldCoroutine1()
#define DispatchReadPositionPacketAccepted() IsReadAcquisitionPacketAcceptedForCurrentTocState()
#define DispatchReadPositionBit001c() ReadBit001cForReadTargetAdjustment()
#define DispatchReadPositionBit001d() ReadBit001dForReadTargetAdjustment()
#define DispatchReadPositionTimingHandshake() ExecuteReadPositionTimingHandshake()
#define DispatchReadPositionSetControllerOperating(enabled) SetControllerOperatingEnableState(enabled)
#define DispatchReadPositionSelectOperatingCode(threshold) SelectAndConfigureOperatingCodeForThreshold(threshold)
#define DispatchReadPositionArmThreeByteCapture() ArmControllerThreeBytePacketCapture()
#define DispatchReadPositionParseThreeBytePacket() ParseControllerThreeBytePacket()
#define DispatchPositionPickupAdjustTarget() AdjustReadAcquisitionTargetNearFullTocPointB0()
#define DispatchPositionPickupUpdateDistance() UpdateReadAcquisitionMoveDistance()
#define DispatchPositionPickupIssueMove() IssueReadAcquisitionPickupMove()
#define DispatchPositionPickupSpecialStep() ProcessSpecialReadAcquisitionPositioningStep()
#define DispatchPositionPickupAbsoluteDistance() CalculateReadAcquisitionAbsoluteFrameDistance()
#define DispatchPositionPickupDistanceAccepted() IsReadAcquisitionFrameDistanceAccepted()
#define DispatchPositionPickupNearTarget() UpdateReadAcquisitionNearTargetFromDynamicTolerance()
#define DispatchPositionPickupRadialDistance() CalculatePositiveRadialModelDistanceToReadTarget()
#define DispatchPositionPickupYield() YieldCoroutine1()
#define DispatchPositionPickupScaledMove(direction, magnitude) MovePickupByScaledRadialDelta((direction), (magnitude))
#endif
#endif
