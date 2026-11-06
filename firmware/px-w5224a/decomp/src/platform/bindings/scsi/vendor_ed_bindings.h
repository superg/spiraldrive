#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveVendorEdUnsupportedCommand(void);
extern u8 ObserveVendorEdLowestWriteSpeed(void);
extern void ObserveVendorEdWriteCdSpeed(u8 speed_x, fw_addr_t output_address);
extern u8 ObserveVendorEdDataIn(fw_addr_t source_address, u32 byte_count);
extern void ObserveVendorEdNoSense(void);
extern void ObserveVendorEeNoSense(void);
extern void ObserveVendorEeRestartFirmware(void);
#define RejectVendorEdCommand() ObserveVendorEdUnsupportedCommand()
#define SelectVendorEdWriteSpeed() ObserveVendorEdLowestWriteSpeed()
#define WriteVendorEdCdSpeed(speed, address) ObserveVendorEdWriteCdSpeed((speed), (address))
#define ExecuteVendorEdDataIn(source, count) ObserveVendorEdDataIn((source), (count))
#define SetVendorEdNoSense() ObserveVendorEdNoSense()
#define SetVendorEeNoSense() ObserveVendorEeNoSense()
#define RestartVendorEeFirmware() ObserveVendorEeRestartFirmware()
#else
#define RejectVendorEdCommand() HandleUnsupportedScsiCommand()
#define SelectVendorEdWriteSpeed() SelectStateEligibleLowestWriteSpeed()
#define WriteVendorEdCdSpeed(speed, address) WriteCdSpeedKilobytesPerSecond((speed), (address))
#define ExecuteVendorEdDataIn(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define SetVendorEdNoSense() SetNoSenseData()
extern void RestartFirmware(void);
#define SetVendorEeNoSense() SetNoSenseData()
#define RestartVendorEeFirmware() RestartFirmware()
#endif
#endif
