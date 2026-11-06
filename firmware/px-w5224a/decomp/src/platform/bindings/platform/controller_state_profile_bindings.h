#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteControllerStateProfilePort(fw_addr_t port_address, u8 value);
extern u8 ObserveReadControllerStateProfilePort(fw_addr_t port_address);
extern void ObserveSubmitControllerStateProfileWord(u16 value, u16 selector);
extern void ObserveConfigureControllerStateProfileOperatingRange(void);
#define WRITE_CONTROLLER_PORT(address, value) ObserveWriteControllerStateProfilePort((address), (value))
#define READ_CONTROLLER_PORT(address) ObserveReadControllerStateProfilePort((address))
#define SUBMIT_CONTROLLER_WORD(value, selector) ObserveSubmitControllerStateProfileWord((value), (selector))
#define CONFIGURE_OPERATING_RANGE() ObserveConfigureControllerStateProfileOperatingRange()
#else
#define WRITE_CONTROLLER_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define READ_CONTROLLER_PORT(address) WaitControllerPortReadyAndReadByte((address))
#define SUBMIT_CONTROLLER_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define CONFIGURE_OPERATING_RANGE() ConfigureControllerPorts8496Through8499ForOperatingRange()
#endif
#endif
