#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRadialPickupParameterPortWrite(fw_addr_t address, u8 value);
extern void ObserveRadialPickupParameterWordSubmit(u16 value, u16 selector);
extern u8 ObserveRadialPickupParameterPortRead(fw_addr_t address);
extern void ObserveRadialPickupParameterYield(void);
extern void ObserveRadialPickupParameterProgramSequence(u16 scaled_magnitude);
#define WRITE_CONTROLLER_PORT(address, value) ObserveRadialPickupParameterPortWrite((address), (value))
#define SUBMIT_CONTROLLER_WORD(value, selector) ObserveRadialPickupParameterWordSubmit((value), (selector))
#define READ_CONTROLLER_PORT(address) ObserveRadialPickupParameterPortRead((address))
#define YIELD_COROUTINE1() ObserveRadialPickupParameterYield()
#define PROGRAM_PARAMETER_SEQUENCE(value) ObserveRadialPickupParameterProgramSequence((value))
#else
#define WRITE_CONTROLLER_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define SUBMIT_CONTROLLER_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define READ_CONTROLLER_PORT(address) WaitControllerPortReadyAndReadByte((address))
#define YIELD_COROUTINE1() YieldCoroutine1()
#define PROGRAM_PARAMETER_SEQUENCE(value) ProgramRadialPickupMoveParameterSequenceForMagnitude((value))
extern void YieldCoroutine1(void);
#endif
#endif
