#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFormat11FinalizeActiveWrite(void);
extern void ObserveFormat11NormalizeWriteSpeed(void);
extern void ObserveFormat11ResetRawRead(void);
extern void ObserveFormat11InitializeCoroutine2(u32 entry_point);
extern void ObserveFormat11YieldCoroutine0(void);
extern u8 ObserveFormat11EnsureOpc(void);
extern u8 ObserveFormat11InitializeType2Pipeline(u8 mode);
extern void ObserveFormat11DispatchCompletion(void);
#define FORMAT11_FINALIZE_ACTIVE_WRITE() ObserveFormat11FinalizeActiveWrite()
#define FORMAT11_NORMALIZE_WRITE_SPEED() ObserveFormat11NormalizeWriteSpeed()
#define FORMAT11_RESET_RAW_READ() ObserveFormat11ResetRawRead()
#define FORMAT11_INITIALIZE_COROUTINE2(entry) ObserveFormat11InitializeCoroutine2(entry)
#define FORMAT11_YIELD_COROUTINE0() ObserveFormat11YieldCoroutine0()
#define FORMAT11_ENSURE_OPC() ObserveFormat11EnsureOpc()
#define FORMAT11_INITIALIZE_TYPE2_PIPELINE(mode) ObserveFormat11InitializeType2Pipeline(mode)
#define FORMAT11_DISPATCH_COMPLETION() ObserveFormat11DispatchCompletion()
#else
#include "platform/format_unit_type2_pipeline.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine0(void);
#define FORMAT11_FINALIZE_ACTIVE_WRITE() FinalizeActiveWritePipelineAndTrackState()
#define FORMAT11_NORMALIZE_WRITE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define FORMAT11_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define FORMAT11_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define FORMAT11_YIELD_COROUTINE0() YieldCoroutine0()
#define FORMAT11_ENSURE_OPC() EnsureOpcForSelectedWriteSpeed()
#define FORMAT11_INITIALIZE_TYPE2_PIPELINE(mode) InitializeFormatUnitCode11Type2ControllerRecordPipelineState(mode)
#define FORMAT11_DISPATCH_COMPLETION() DispatchCurrentHostCommandAndPublishCompletion()
#endif
#endif
