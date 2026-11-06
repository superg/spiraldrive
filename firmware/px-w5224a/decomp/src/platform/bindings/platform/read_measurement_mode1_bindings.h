#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveReadMeasurementMode1Event(u8 event);
extern u8 ObserveReadMeasurementMode1Bit0032(void);
#define Mode1PrepareDescriptor() ObserveReadMeasurementMode1Event(1u)
#define Mode1ResetToc() ObserveReadMeasurementMode1Event(2u)
#define Mode1RemoveSession() ObserveReadMeasurementMode1Event(3u)
#define Mode1AppendSession() ObserveReadMeasurementMode1Event(4u)
#define Mode1FinalizeC3() ObserveReadMeasurementMode1Event(5u)
#define Mode1FinalizeC4() ObserveReadMeasurementMode1Event(6u)
#define Mode1PrepareLowDescriptor() ObserveReadMeasurementMode1Event(7u)
#define Mode1ReadBit0032() ObserveReadMeasurementMode1Bit0032()
#else
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForDescriptorMode(void);
extern void ResetFullTocDescriptorWorkspace(void);
extern void FUN_RAM_fbb2c3(void);
extern void FUN_RAM_fbb2c4(void);
extern void PrepareReadMeasurementForLowDescriptorMode(void);
extern u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0032(void);
#define Mode1PrepareDescriptor() PrepareReadMeasurementForDescriptorMode()
#define Mode1ResetToc() ResetFullTocDescriptorWorkspace()
#define Mode1RemoveSession() RemoveLatestFullTocSessionAndRebuildState()
#define Mode1AppendSession() AppendFullTocSessionFromRawReadQRecordTable()
#define Mode1FinalizeC3() FUN_RAM_fbb2c3()
#define Mode1FinalizeC4() FUN_RAM_fbb2c4()
#define Mode1PrepareLowDescriptor() PrepareReadMeasurementForLowDescriptorMode()
#define Mode1ReadBit0032() FirmwareReadBit0032()
#endif
#endif
