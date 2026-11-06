#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveOperatingCodePortsProgressUpdate(void);
extern void ObserveOperatingCodePortsStrobe(fw_addr_t address, u8 value);
extern void ObserveOperatingCodePortsWrite(fw_addr_t address, u8 value);
#define UPDATE_PROGRESS_WORD() ObserveOperatingCodePortsProgressUpdate()
#define STROBE_PORT(address, value) ObserveOperatingCodePortsStrobe((address), (value))
#define WRITE_PORT(address, value) ObserveOperatingCodePortsWrite((address), (value))
#else
extern void UpdateControllerWord035FFromProgressBucket(void);
#define UPDATE_PROGRESS_WORD() UpdateControllerWord035FFromProgressBucket()
#define STROBE_PORT(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#endif
#endif
