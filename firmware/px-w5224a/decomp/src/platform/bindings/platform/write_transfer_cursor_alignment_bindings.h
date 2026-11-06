#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveAlignWriteTransferMapFixedPacket(u32 logical_address);
extern s32 ObserveAlignWriteTransferExpandRecordOverhead(s32 logical_address);
extern u32 ObserveAlignWriteTransferInitializeHostRange(void);
#define MAP_FIXED_PACKET(address) ObserveAlignWriteTransferMapFixedPacket(address)
#define EXPAND_RECORD_OVERHEAD(address) ObserveAlignWriteTransferExpandRecordOverhead(address)
#define INITIALIZE_HOST_RANGE() ObserveAlignWriteTransferInitializeHostRange()
#else
#define MAP_FIXED_PACKET(address) MapFixedPacketLogicalToPhysicalAddress(address)
#define EXPAND_RECORD_OVERHEAD(address) ExpandWriteLogicalAddressForRecordOverhead(address)
#define INITIALIZE_HOST_RANGE() InitializeHostTransferRange()
#endif
#endif
