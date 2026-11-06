#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateReadCapacity10Cdb(fw_addr_t mask_descriptor);
extern void ObserveUnsupportedReadCapacity10Command(void);
extern u32 ObserveComputeReadCapacity10Geometry(void);
extern u8 ObserveTransferReadCapacity10Response(fw_addr_t source_address, u32 byte_count);
extern void ObserveSetNoSenseForReadCapacity10(void);
#define VALIDATE_READ_CAPACITY_10(mask) ObserveValidateReadCapacity10Cdb(mask)
#define HANDLE_UNSUPPORTED_READ_CAPACITY_10() ObserveUnsupportedReadCapacity10Command()
#define COMPUTE_READ_CAPACITY_10_GEOMETRY() ObserveComputeReadCapacity10Geometry()
#define TRANSFER_READ_CAPACITY_10_RESPONSE(source, count) ObserveTransferReadCapacity10Response((source), (count))
#define SET_NO_SENSE_FOR_READ_CAPACITY_10() ObserveSetNoSenseForReadCapacity10()
#else
#define VALIDATE_READ_CAPACITY_10(mask) ValidateScsiCdbFieldMasks(mask)
#define HANDLE_UNSUPPORTED_READ_CAPACITY_10() HandleUnsupportedScsiCommand()
#define COMPUTE_READ_CAPACITY_10_GEOMETRY() ComputeAndPublishMediaCapacityGeometry()
#define TRANSFER_READ_CAPACITY_10_RESPONSE(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define SET_NO_SENSE_FOR_READ_CAPACITY_10() SetNoSenseData()
#endif
#endif
