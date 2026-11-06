#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRadialPickupRestoreToleranceWait(u8 first, u8 second);
extern void ObserveRadialPickupRestorePortWrite(fw_addr_t address, u8 value);
extern u8 ObserveRadialPickupRestorePortRead(fw_addr_t address);
extern void ObserveRadialPickupRestoreDelay(u16 duration, u8 yield);
extern void ObserveRadialPickupRestoreChannel(u8 channel, u8 value);
extern void ObserveRadialPickupRestoreStep(u8 step);
extern void ObserveRadialPickupRestoreWord(u16 value);
#define WAIT_TOLERANCE(first, second) ObserveRadialPickupRestoreToleranceWait((first), (second))
#define WRITE_PORT(address, value) ObserveRadialPickupRestorePortWrite((address), (value))
#define READ_PORT(address) ObserveRadialPickupRestorePortRead((address))
#define WAIT_DURATION(duration, yield) ObserveRadialPickupRestoreDelay((duration), (yield))
#define SET_CHANNEL(channel, value) ObserveRadialPickupRestoreChannel((channel), (value))
#define CONFIGURE_RANGE() ObserveRadialPickupRestoreStep(1u)
#define WRITE_PORT_80E4() ObserveRadialPickupRestoreStep(2u)
#define APPLY_D1() ObserveRadialPickupRestoreStep(3u)
#define WRITE_D1_D2_D3() ObserveRadialPickupRestoreStep(4u)
#define WRITE_PARAMETER(value) ObserveRadialPickupRestoreWord((value))
#else
#define WAIT_TOLERANCE(first, second) WaitForControllerMeasurementWithinToleranceOrExpiry((first), (second))
#define WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define READ_PORT(address) WaitControllerPortReadyAndReadByte((address))
#define WAIT_DURATION(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#define CONFIGURE_RANGE() ConfigureControllerPorts8496Through8499ForOperatingRange()
#define WRITE_PORT_80E4() WriteControllerPort80E4Value03AndWait()
#define APPLY_D1() ApplyControllerParameterD1ModeBits()
#define WRITE_D1_D2_D3() WriteControllerParametersD1D2D3FromConfiguration()
#define WRITE_PARAMETER(value) WriteControllerParameterWord((value))
#endif
#endif
