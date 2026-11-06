#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRepeatedRangeYield(void);
extern void ObserveRepeatedRangeInitializeCoroutine1(u32 entry_point);
extern s32 ObserveRepeatedRangeConvert(fw_addr_t tuple_address);
extern u32 ObserveRepeatedRangeMultiply(u32 left, u32 right);
extern u8 ObserveRepeatedRangePrepare(u8 use_plan, s32 start, s32 end);
extern u8 ObserveRepeatedRangePrefix(u8 omit_final, u8 wait);
extern u16 ObserveRepeatedRangeRemainder20(u16 value);
extern u8 ObserveRepeatedRangeOpc(void);
extern u8 ObserveRepeatedRangeBuildQ(void);
#define REPEATED_YIELD() ObserveRepeatedRangeYield()
#define REPEATED_INITIALIZE(entry) ObserveRepeatedRangeInitializeCoroutine1(entry)
#define REPEATED_CONVERT(address) ObserveRepeatedRangeConvert(address)
#define REPEATED_MULTIPLY(left, right) ObserveRepeatedRangeMultiply((left), (right))
#define REPEATED_PREPARE(use_plan, start, end) ObserveRepeatedRangePrepare((use_plan), (start), (end))
#define REPEATED_PREFIX(omit, wait) ObserveRepeatedRangePrefix((omit), (wait))
#define REPEATED_REMAINDER(value) ObserveRepeatedRangeRemainder20(value)
#define REPEATED_OPC() ObserveRepeatedRangeOpc()
#define REPEATED_BUILD_Q() ObserveRepeatedRangeBuildQ()
#else
extern void YieldCoroutine0(void);
extern u16 PLEXTOR_NEAR_FUNCTION RepeatedRawReadWindowIndexRemainder20(u16 value);
#define REPEATED_YIELD() YieldCoroutine0()
#define REPEATED_INITIALIZE(entry) InitializeCoroutine1(entry)
#define REPEATED_CONVERT(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define REPEATED_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define REPEATED_PREPARE(use_plan, start, end) PrepareSyntheticRawReadPatternRange((use_plan), (start), (end))
#define REPEATED_PREFIX(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define REPEATED_REMAINDER(value) RepeatedRawReadWindowIndexRemainder20(value)
#define REPEATED_OPC() EnsureOpcCalibrationForRawReadOperation()
#define REPEATED_BUILD_Q() InitializeAndBuildRawReadQRecordQueue()
#endif
#endif
