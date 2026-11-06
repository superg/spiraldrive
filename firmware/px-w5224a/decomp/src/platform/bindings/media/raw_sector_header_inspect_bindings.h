#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u16 ObserveGetLatestRawSectorHeaderDescriptor(void);
extern u16 ObserveGetSecondaryRawSectorHeaderDescriptor(void);
extern u8 ObserveReadRawSectorHeaderRingByte(void);
#define GET_LATEST_HEADER_DESCRIPTOR() ObserveGetLatestRawSectorHeaderDescriptor()
#define GET_SECONDARY_HEADER_DESCRIPTOR() ObserveGetSecondaryRawSectorHeaderDescriptor()
#define READ_HEADER_RING_BYTE() ObserveReadRawSectorHeaderRingByte()
#else
#define GET_LATEST_HEADER_DESCRIPTOR() GetLatestQueuedRawSectorDescriptorAddress()
#define GET_SECONDARY_HEADER_DESCRIPTOR() GetRawReadSecondaryCursorDescriptorAddress()
#define READ_HEADER_RING_BYTE() ReadMainDataRingByteAtCursorPlus16()
#endif
#endif
