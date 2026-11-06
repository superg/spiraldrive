#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFormat11StartResetRawRead(void);
extern void ObserveFormat11StartInitializeCoroutine2(u32 entry_point);
extern void ObserveFormat11StartYield(void);
extern u8 ObserveFormat11StartEnsureOpc(void);
extern void ObserveFormat11StartConfigureRing(u8 geometry_mode);
extern u8 ObserveFormat11StartPrepareType2(void);
extern void ObserveFormat11StartSetSense(u8 sense_key, u16 sense_code);
extern void ObserveFormat11StartCompleteNoSense(void);
extern u8 ObserveFormat11StartInitializeType2(u8 mode);
extern void ObserveFormat11StartDispatchCompletion(void);
#define START11_RESET_RAW_READ() ObserveFormat11StartResetRawRead()
#define START11_INITIALIZE_COROUTINE2(entry) ObserveFormat11StartInitializeCoroutine2(entry)
#define START11_YIELD() ObserveFormat11StartYield()
#define START11_ENSURE_OPC() ObserveFormat11StartEnsureOpc()
#define START11_CONFIGURE_RING(mode) ObserveFormat11StartConfigureRing(mode)
#define START11_PREPARE_TYPE2() ObserveFormat11StartPrepareType2()
#define START11_SET_SENSE(key, code) ObserveFormat11StartSetSense((key), (code))
#define START11_COMPLETE_NO_SENSE() ObserveFormat11StartCompleteNoSense()
#define START11_INITIALIZE_TYPE2(mode) ObserveFormat11StartInitializeType2(mode)
#define START11_DISPATCH_COMPLETION() ObserveFormat11StartDispatchCompletion()
#else
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(u32 entry_point);
extern void YieldCoroutine0(void);
extern u8 EnsureOpcForSelectedWriteSpeed(void);
extern void ConfigureMainDataRingGeometry(u8 geometry_mode);
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void CompleteScsiCommandWithNoSense(void);
extern u8 InitializeFormatUnitCode11Type2ControllerRecordPipelineState(u8 mode);
extern void DispatchCurrentHostCommandAndPublishCompletion(void);
#define START11_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define START11_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define START11_YIELD() YieldCoroutine0()
#define START11_ENSURE_OPC() EnsureOpcForSelectedWriteSpeed()
#define START11_CONFIGURE_RING(mode) ConfigureMainDataRingGeometry(mode)
#define START11_PREPARE_TYPE2() PrepareFormatUnitCode11Type2ControllerRecordPipeline()
#define START11_SET_SENSE(key, code) SetSenseData((key), (code))
#define START11_COMPLETE_NO_SENSE() CompleteScsiCommandWithNoSense()
#define START11_INITIALIZE_TYPE2(mode) InitializeFormatUnitCode11Type2ControllerRecordPipelineState(mode)
#define START11_DISPATCH_COMPLETION() DispatchCurrentHostCommandAndPublishCompletion()
#endif
#endif
