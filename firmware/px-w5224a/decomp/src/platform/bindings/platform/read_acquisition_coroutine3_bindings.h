#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveReadAcquisitionCoroutine3Dependency(u8 event, u16 argument);
extern u8 ObserveReadAcquisitionCoroutine3ReadBit(u16 bit_address);
extern void ObserveReadAcquisitionCoroutine3WriteBit(u16 bit_address, u8 value);
extern void ObserveReadAcquisitionCoroutine3Yield(void);

#define COROUTINE3_DEPENDENCY(event, argument) ObserveReadAcquisitionCoroutine3Dependency((event), (argument))
#define COROUTINE3_READ_BIT(address) ObserveReadAcquisitionCoroutine3ReadBit(address)
#define COROUTINE3_WRITE_BIT(address, value) ObserveReadAcquisitionCoroutine3WriteBit((address), (value))
#define COROUTINE3_YIELD() ObserveReadAcquisitionCoroutine3Yield()
#else
extern void ResetReadMeasurementState(void);
extern void LoadControllerBufferSetupCoroutineTail(void);
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode0(void);
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode1(void);
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode2(void);
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode3(void);
extern void WriteControllerParametersE8E9ForValue(u8 value);
extern u8 RestoreRawReadDescriptorsToQueueHead(u16 descriptor_count);
extern void InitializeReadAcquisitionControllerRegisters(void);
extern void MarkReadAcquisitionControllerRecordRange(void);
extern void PrepareReadMeasurementControllerRecordRangeAndQueue(void);
extern void YieldCoroutine3(void);
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0029ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0029ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION SetP3RxHighTxLowForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION SetP3RxLowTxHighForReadAcquisitionCoroutine3(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0030ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0030ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0031ForReadAcquisitionCoroutine3(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0032ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0032ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0033ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0035ForReadAcquisitionCoroutine3(void);

#define COROUTINE3_CONFIGURE_CONTROLLER() ConfigureReadAcquisitionControllerForCurrentState()
#define COROUTINE3_RESET_MEASUREMENT() ResetReadMeasurementState()
#define COROUTINE3_LOAD_BUFFER_SETUP() LoadControllerBufferSetupCoroutineTail()
#define COROUTINE3_PREPARE_MODE0() PrepareReadMeasurementForAcquisitionMode0()
#define COROUTINE3_PREPARE_MODE1() PrepareReadMeasurementForAcquisitionMode1()
#define COROUTINE3_PREPARE_MODE2() PrepareReadMeasurementForAcquisitionMode2()
#define COROUTINE3_PREPARE_MODE3() PrepareReadMeasurementForAcquisitionMode3()
#define COROUTINE3_WRITE_E8E9(value) WriteControllerParametersE8E9ForValue(value)
#define COROUTINE3_RESTORE_DESCRIPTORS(count) RestoreRawReadDescriptorsToQueueHead(count)
#define COROUTINE3_INITIALIZE_REGISTERS() InitializeReadAcquisitionControllerRegisters()
#define COROUTINE3_MARK_RECORD_RANGE() MarkReadAcquisitionControllerRecordRange()
#define COROUTINE3_PREPARE_RECORD_RANGE() PrepareReadMeasurementControllerRecordRangeAndQueue()
#define COROUTINE3_SET_BIT29() SetBit0029ForReadAcquisitionCoroutine3()
#define COROUTINE3_CLEAR_BIT29() ClearBit0029ForReadAcquisitionCoroutine3()
#define COROUTINE3_SET_P3_RX_HIGH_TX_LOW() SetP3RxHighTxLowForReadAcquisitionCoroutine3()
#define COROUTINE3_SET_P3_RX_LOW_TX_HIGH() SetP3RxLowTxHighForReadAcquisitionCoroutine3()
#define COROUTINE3_READ_BIT30() ReadBit0030ForReadAcquisitionCoroutine3()
#define COROUTINE3_CLEAR_BIT30() ClearBit0030ForReadAcquisitionCoroutine3()
#define COROUTINE3_CLEAR_BIT31() ClearBit0031ForReadAcquisitionCoroutine3()
#define COROUTINE3_READ_BIT32() ReadBit0032ForReadAcquisitionCoroutine3()
#define COROUTINE3_CLEAR_BIT32() ClearBit0032ForReadAcquisitionCoroutine3()
#define COROUTINE3_CLEAR_BIT33() ClearBit0033ForReadAcquisitionCoroutine3()
#define COROUTINE3_CLEAR_BIT35() ClearBit0035ForReadAcquisitionCoroutine3()
#define COROUTINE3_DISABLE_GLOBAL_INTERRUPT() ClearBit0547Atomic()
#define COROUTINE3_ENABLE_GLOBAL_INTERRUPT() SetBit0547Atomic()
#define COROUTINE3_ENABLE_CONTROLLER_INTERRUPT() SetBit0546Atomic()
#define COROUTINE3_SET_SFR_A9_BIT2() SetSfrA9Bit2Atomic()
#define COROUTINE3_YIELD() YieldCoroutine3()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define COROUTINE3_CONFIGURE_CONTROLLER() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_CONFIGURE_CONTROLLER, 0u)
#define COROUTINE3_RESET_MEASUREMENT() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_RESET_MEASUREMENT, 0u)
#define COROUTINE3_LOAD_BUFFER_SETUP() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_LOAD_BUFFER_SETUP, 0u)
#define COROUTINE3_PREPARE_MODE0() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_PREPARE_MODE0, 0u)
#define COROUTINE3_PREPARE_MODE1() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_PREPARE_MODE1, 0u)
#define COROUTINE3_PREPARE_MODE2() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_PREPARE_MODE2, 0u)
#define COROUTINE3_PREPARE_MODE3() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_PREPARE_MODE3, 0u)
#define COROUTINE3_WRITE_E8E9(value) COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_WRITE_E8E9, value)
#define COROUTINE3_RESTORE_DESCRIPTORS(count) COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_RESTORE_DESCRIPTORS, count)
#define COROUTINE3_INITIALIZE_REGISTERS() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_INITIALIZE_REGISTERS, 0u)
#define COROUTINE3_MARK_RECORD_RANGE() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_MARK_RECORD_RANGE, 0u)
#define COROUTINE3_PREPARE_RECORD_RANGE() COROUTINE3_DEPENDENCY(READ_ACQUISITION_COROUTINE3_PREPARE_RECORD_RANGE, 0u)
#define COROUTINE3_SET_BIT29() COROUTINE3_WRITE_BIT(0x0029u, 1u)
#define COROUTINE3_CLEAR_BIT29() COROUTINE3_WRITE_BIT(0x0029u, 0u)
#define COROUTINE3_SET_P3_RX_HIGH_TX_LOW()                                                                                                                                                             \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        COROUTINE3_WRITE_BIT(0x0580u, 1u);                                                                                                                                                             \
        COROUTINE3_WRITE_BIT(0x0581u, 0u);                                                                                                                                                             \
    } while(0)
#define COROUTINE3_SET_P3_RX_LOW_TX_HIGH()                                                                                                                                                             \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        COROUTINE3_WRITE_BIT(0x0580u, 0u);                                                                                                                                                             \
        COROUTINE3_WRITE_BIT(0x0581u, 1u);                                                                                                                                                             \
    } while(0)
#define COROUTINE3_READ_BIT30() COROUTINE3_READ_BIT(0x0030u)
#define COROUTINE3_CLEAR_BIT30() COROUTINE3_WRITE_BIT(0x0030u, 0u)
#define COROUTINE3_CLEAR_BIT31() COROUTINE3_WRITE_BIT(0x0031u, 0u)
#define COROUTINE3_READ_BIT32() COROUTINE3_READ_BIT(0x0032u)
#define COROUTINE3_CLEAR_BIT32() COROUTINE3_WRITE_BIT(0x0032u, 0u)
#define COROUTINE3_CLEAR_BIT33() COROUTINE3_WRITE_BIT(0x0033u, 0u)
#define COROUTINE3_CLEAR_BIT35() COROUTINE3_WRITE_BIT(0x0035u, 0u)
#define COROUTINE3_DISABLE_GLOBAL_INTERRUPT() COROUTINE3_WRITE_BIT(0x0547u, 0u)
#define COROUTINE3_ENABLE_GLOBAL_INTERRUPT() COROUTINE3_WRITE_BIT(0x0547u, 1u)
#define COROUTINE3_ENABLE_CONTROLLER_INTERRUPT() COROUTINE3_WRITE_BIT(0x0546u, 1u)
#define COROUTINE3_SET_SFR_A9_BIT2() COROUTINE3_WRITE_BIT(0x054au, 1u)
#endif
#endif
