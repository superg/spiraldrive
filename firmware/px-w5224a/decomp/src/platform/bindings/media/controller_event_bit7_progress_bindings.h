#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u16 ObserveGetRawReadConsumerDescriptorForEventBit7(void);
extern void ObserveConsumeRawReadDescriptorsForEventBit7(u16 count);
extern void ObserveResetControllerEventBit7TransferState(void);
extern void ObserveResetControllerPortsForEventBit7(void);
extern void ObserveClearControllerPortsForEventBit7(void);

#define GetRawReadConsumerDescriptorAddressOrZero ObserveGetRawReadConsumerDescriptorForEventBit7
#define ConsumeRawReadDescriptors ObserveConsumeRawReadDescriptorsForEventBit7
#define ResetControllerEventBit7TransferState ObserveResetControllerEventBit7TransferState
#define ResetControllerPort8087And808A808BUnlessFlag0E ObserveResetControllerPortsForEventBit7
#define ClearControllerPorts8088And8089UnlessFlag0E ObserveClearControllerPortsForEventBit7
#else
extern u16 GetRawReadConsumerDescriptorAddressOrZero(void);
extern void ConsumeRawReadDescriptors(u16 count);
extern void PLEXTOR_NEAR_FUNCTION ResetControllerEventBit7TransferState(void);
extern void ResetControllerPort8087And808A808BUnlessFlag0E(void);
extern void ClearControllerPorts8088And8089UnlessFlag0E(void);
#endif
#endif
