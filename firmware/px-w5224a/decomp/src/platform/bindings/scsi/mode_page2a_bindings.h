#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveModePage2ASpeedWrite(u8 speed_x, fw_addr_t output_address);
extern void ObserveModePage2ANormalizeWriteSpeed(void);
extern void ObserveModePage2APerformanceDescriptors(u8 maximum_speed_x, u8 count_offset);
extern u8 ObserveModePage2ALowestWriteSpeed(void);
#define WRITE_MODE_PAGE_2A_SPEED(speed, address) ObserveModePage2ASpeedWrite((speed), (address))
#define NORMALIZE_MODE_PAGE_2A_WRITE_SPEED() ObserveModePage2ANormalizeWriteSpeed()
#define POPULATE_MODE_PAGE_2A_DESCRIPTORS(speed, offset) ObserveModePage2APerformanceDescriptors((speed), (offset))
#define SELECT_MODE_PAGE_2A_LOWEST_SPEED() ObserveModePage2ALowestWriteSpeed()
#else
extern void PLEXTOR_NEAR_FUNCTION PopulateModePage2AWriteSpeedPerformanceDescriptors(u8 maximum_speed_x, u8 count_offset);
#define WRITE_MODE_PAGE_2A_SPEED(speed, address) WriteCdSpeedKilobytesPerSecond((speed), (address))
#define NORMALIZE_MODE_PAGE_2A_WRITE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define POPULATE_MODE_PAGE_2A_DESCRIPTORS(speed, offset) PopulateModePage2AWriteSpeedPerformanceDescriptors((speed), (offset))
#define SELECT_MODE_PAGE_2A_LOWEST_SPEED() SelectStateEligibleLowestWriteSpeed()
#endif
#endif
