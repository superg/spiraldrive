#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawSectorDescriptorSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveRawSectorDescriptorSetInformation(u32 information);
#define SET_DESCRIPTOR_SENSE(key, code) ObserveRawSectorDescriptorSetSense((key), (code))
#define SET_DESCRIPTOR_INFORMATION(value) ObserveRawSectorDescriptorSetInformation(value)
#else
extern void SetSenseData(u8 sense_key, u16 asc_ascq);
extern void SetSenseInformation(u32 information);
#define SET_DESCRIPTOR_SENSE(key, code) SetSenseData((key), (code))
#define SET_DESCRIPTOR_INFORMATION(value) SetSenseInformation(value)
#endif
#endif
