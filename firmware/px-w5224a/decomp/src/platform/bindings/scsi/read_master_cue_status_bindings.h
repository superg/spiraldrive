#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSetReadMasterCueSense(u8 sense_key, u16 asc_ascq);
extern u16 ObserveGetLatestReadMasterCueDescriptor(void);
extern u16 ObserveGetReadMasterCueSecondaryDescriptor(void);
extern u8 ObserveValidateReadMasterCueDescriptor(u16 descriptor_address);
extern u8 ObserveReadMasterCueQueueHighWater(void);
#define SET_READ_MASTER_CUE_SENSE(key, code) ObserveSetReadMasterCueSense((key), (code))
#define GET_LATEST_READ_MASTER_CUE_DESCRIPTOR() ObserveGetLatestReadMasterCueDescriptor()
#define GET_READ_MASTER_CUE_SECONDARY_DESCRIPTOR() ObserveGetReadMasterCueSecondaryDescriptor()
#define VALIDATE_READ_MASTER_CUE_DESCRIPTOR(address) ObserveValidateReadMasterCueDescriptor(address)
#define READ_MASTER_CUE_QUEUE_HIGH_WATER() ObserveReadMasterCueQueueHighWater()
#else
extern u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address);
#define SET_READ_MASTER_CUE_SENSE(key, code) SetSenseData((key), (code))
#define GET_LATEST_READ_MASTER_CUE_DESCRIPTOR() GetLatestQueuedRawSectorDescriptorAddress()
#define GET_READ_MASTER_CUE_SECONDARY_DESCRIPTOR() GetRawReadSecondaryCursorDescriptorAddress()
#define VALIDATE_READ_MASTER_CUE_DESCRIPTOR(address) ValidateRawSectorDescriptorStatus(address)
#define READ_MASTER_CUE_QUEUE_HIGH_WATER() IsRawReadQueueAtHighWater()
#endif
#endif
