#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveScaledRadialMoveConfigure(u8 inward_direction);
extern u8 ObserveScaledRadialMoveConfigureOutputs(u8 inward_direction);
extern void ObserveScaledRadialMoveExecute(u8 inward_direction, u16 scaled_magnitude);
extern void ObserveScaledRadialMovePortWrite(fw_addr_t address, u8 value);
extern void ObserveScaledRadialMoveWait(u16 duration, u8 yield_while_waiting);
extern void ObserveScaledRadialMoveRestore(void);
#define CONFIGURE_MOVE(direction) ObserveScaledRadialMoveConfigure(direction)
#define CONFIGURE_OUTPUTS(direction) ObserveScaledRadialMoveConfigureOutputs(direction)
#define EXECUTE_MOVE(direction, magnitude) ObserveScaledRadialMoveExecute((direction), (magnitude))
#define WRITE_MOVE_PORT(address, value) ObserveScaledRadialMovePortWrite((address), (value))
#define WAIT_MOVE_DURATION(duration) ObserveScaledRadialMoveWait((duration), 0u)
#define RESTORE_MOVE_STATE() ObserveScaledRadialMoveRestore()
#else
#define CONFIGURE_MOVE(direction) ConfigureControllerStateForScaledRadialPickupMove(direction)
#define CONFIGURE_OUTPUTS(direction) ConfigureScaledRadialMovePulseOutputs(direction)
#define EXECUTE_MOVE(direction, magnitude) ProgramAndExecuteRadialPickupMove((direction), (magnitude))
#define WRITE_MOVE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define WAIT_MOVE_DURATION(duration) WaitTimer1ScaledDuration((duration), 0u)
#define RESTORE_MOVE_STATE() RestoreControllerStateAfterScaledRadialPickupMove()
#endif
#endif
