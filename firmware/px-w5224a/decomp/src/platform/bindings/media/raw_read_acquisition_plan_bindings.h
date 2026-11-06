#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveRawReadAcquisitionPlanOperation(u8 operation, s32 argument0, s32 argument1, s32 argument2);
#else
#include "platform/encoding.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return (u8)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_INITIALIZE_RANGE, alternate_plan, 0l, 0l);
#else
return InitializeRawReadPreparationRangeAndPacket(alternate_plan);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
return (u8)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_TEST_ELIGIBILITY, alternate_state, 0l, 0l);
#else
return IsRawReadPreparationStateEligible(alternate_state);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
(void)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_SET_SEQUENCE_ERROR, 0l, 0l, 0l);
#else
SetCommandSequenceErrorSense();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
return (u8)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_MAP_STATUS, field_r7, field_r6, flags);
#else
return MapThreeFieldStatusCode(flags, field_r7, field_r6);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
return (u8)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_EXECUTE_PENDING_RANGE, state_eligible, 0l, 0l);
#else
return ExecutePendingRawReadPreparationRange(state_eligible);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
return (u8)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_SELECT_SECTOR_CLASS, 0l, state_eligible, 0l);
#else
return SelectRawReadSectorClass(0u, state_eligible);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 8
#if defined(PLEXTOR_HOST_TEST)
return (u8)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_ACQUIRE_SEGMENTS, state_eligible, alternate_plan, 0l);
#else
return AcquireConfiguredRawReadPreparationSegments(state_eligible, alternate_plan);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 9
#if defined(PLEXTOR_HOST_TEST)
(void)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_SET_MSF_TUPLES, relative_position, absolute_position, 0l);
#else
(void)SetRawReadQPacketMsfTuples(relative_position, absolute_position);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 10
#if defined(PLEXTOR_HOST_TEST)
(void)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_CONFIGURE_LAYOUT, 0l, 0l, 0l);
#else
ConfigureRawReadTransferLayoutFromQTypeCode();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 11
#if defined(PLEXTOR_HOST_TEST)
(void)ObserveRawReadAcquisitionPlanOperation(RAW_READ_PLAN_DECREMENT_MSF, address, 0l, 0l);
#else
DecrementPackedBcdMsf(address);
#endif
#endif
