#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveType2MipStateDeriveGeometry(void);
extern s32 ObserveType2MipStateDivideS32(s32 dividend, s32 divisor);
extern u8 ObserveType2MipStateFinalizeComparison(u8 matched_record_count, u8 unmatched_residual_count);
extern void ObserveType2MipStateBuildBoundaries(u8 inherited_segment_count);
#define MIP_STATE_DERIVE_GEOMETRY() ObserveType2MipStateDeriveGeometry()
#define MIP_STATE_DIVIDE(dividend, divisor) ObserveType2MipStateDivideS32((dividend), (divisor))
#define MIP_STATE_FINALIZE_COMPARISON(matched, residual) ObserveType2MipStateFinalizeComparison((matched), (residual))
#define MIP_STATE_BUILD_BOUNDARIES(count) ObserveType2MipStateBuildBoundaries(count)
#else
extern void PLEXTOR_NEAR_FUNCTION DeriveMipControllerRecordGeometryFromPayload(void);
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#define MIP_STATE_DERIVE_GEOMETRY() DeriveMipControllerRecordGeometryFromPayload()
#define MIP_STATE_DIVIDE(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#define MIP_STATE_FINALIZE_COMPARISON(matched, residual) FinalizeType2MipRecordComparisonStateForBoundaryBuild((matched), (residual))
#define MIP_STATE_BUILD_BOUNDARIES(count) BuildMipType2ControllerRecordBoundaryTableWithInheritedCount(count)
#endif
#endif
