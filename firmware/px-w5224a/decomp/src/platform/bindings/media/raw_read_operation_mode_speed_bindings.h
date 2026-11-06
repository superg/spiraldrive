#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawReadOperationIndexed(u8 state_code);
extern void ObserveRawReadOperationYield(void);
extern void ObserveRawReadOperationNoOp(void);
extern u8 ObserveRawReadOperationBit0406(void);
extern u8 ObserveRawReadOperationBit0402(void);
extern void ObserveRawReadOperationNormalize(void);
extern u8 ObserveRawReadOperationOpc(void);
extern void ObserveRawReadOperationBuildTdi(fw_addr_t destination);
extern u8 ObserveRawReadOperationPrepareSynthetic(void);
extern u8 ObserveRawReadOperationAcquireRepeated(u32 frame_spacing);
extern void ObserveRawReadOperationInitializeCoroutine1(u32 entry_point);
#define RAW_OPERATION_INDEXED(code) ObserveRawReadOperationIndexed(code)
#define RAW_OPERATION_YIELD() ObserveRawReadOperationYield()
#define RAW_OPERATION_NOOP() ObserveRawReadOperationNoOp()
#define RAW_OPERATION_BIT0406() ObserveRawReadOperationBit0406()
#define RAW_OPERATION_BIT0402() ObserveRawReadOperationBit0402()
#define RAW_OPERATION_NORMALIZE() ObserveRawReadOperationNormalize()
#define RAW_OPERATION_OPC() ObserveRawReadOperationOpc()
#define RAW_OPERATION_BUILD_TDI(destination) ObserveRawReadOperationBuildTdi(destination)
#define RAW_OPERATION_PREPARE() ObserveRawReadOperationPrepareSynthetic()
#define RAW_OPERATION_REPEAT(spacing) ObserveRawReadOperationAcquireRepeated(spacing)
#define RAW_OPERATION_INITIALIZE(entry) ObserveRawReadOperationInitializeCoroutine1(entry)
#else
extern void YieldCoroutine0(void);
extern u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0406(void);
extern u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0402(void);
#define RAW_OPERATION_INDEXED(code) IsIndexedDriveStateActive(code)
#define RAW_OPERATION_YIELD() YieldCoroutine0()
#define RAW_OPERATION_NOOP() ReturnFromRawReadOperation()
#define RAW_OPERATION_BIT0406() FirmwareReadBit0406()
#define RAW_OPERATION_BIT0402() FirmwareReadBit0402()
#define RAW_OPERATION_NORMALIZE() NormalizeWriteSpeedForCurrentMedia()
#define RAW_OPERATION_OPC() EnsureOpcCalibrationForRawReadOperation()
#define RAW_OPERATION_BUILD_TDI(destination) BuildRawSectorTdiPayloadPrefix(destination)
#define RAW_OPERATION_PREPARE() PrepareSyntheticRawReadOperationState()
#define RAW_OPERATION_REPEAT(spacing) AcquireRepeatedRawReadRangeWindows(spacing)
#define RAW_OPERATION_INITIALIZE(entry) InitializeCoroutine1(entry)
#endif
#endif
