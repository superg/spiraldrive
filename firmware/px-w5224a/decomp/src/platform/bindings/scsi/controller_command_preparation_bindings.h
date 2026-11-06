#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSetControllerFlagForScsiPreparation(u8 enabled);
extern u8 ObserveReadPort8058ForScsiPreparation(void);
extern void ObserveConfigureControllerForScsiPreparation(u8 enabled);
extern void ObserveSetP1TimerPinsForScsiPreparation(void);
extern void ObserveEnableExternalInterrupt0ForScsiPreparation(void);
extern void ObserveWriteParameterD6ForScsiPreparation(void);
extern void ObserveSetP3HandshakeForScsiPreparation(void);
extern void ObserveDelayForScsiPreparation(void);
extern u8 ObserveReadPort8043ForScsiPreparation(void);
extern void ObserveYieldCoroutine0ForScsiPreparation(void);
#define SET_CONTROLLER_FLAG(enabled) ObserveSetControllerFlagForScsiPreparation(enabled)
#define READ_PORT_8058() ObserveReadPort8058ForScsiPreparation()
#define CONFIGURE_CONTROLLER(enabled) ObserveConfigureControllerForScsiPreparation(enabled)
#define SET_P1_TIMER_PINS() ObserveSetP1TimerPinsForScsiPreparation()
#define ENABLE_EXTERNAL_INTERRUPT0() ObserveEnableExternalInterrupt0ForScsiPreparation()
#define WRITE_PARAMETER_D6() ObserveWriteParameterD6ForScsiPreparation()
#define SET_P3_HANDSHAKE() ObserveSetP3HandshakeForScsiPreparation()
#define DELAY_TIMER_UNITS_59() ObserveDelayForScsiPreparation()
#define READ_PORT_8043() ObserveReadPort8043ForScsiPreparation()
#define YIELD_COROUTINE0() ObserveYieldCoroutine0ForScsiPreparation()
#else
extern void ConfigureControllerStartupModeRegisters(u8 enabled);
extern void PLEXTOR_NEAR_FUNCTION SetExternalInterrupt0EnableAtomic(void);
extern void YieldCoroutine0(void);
extern void FirmwareWriteBit0580(u8 value);
extern void FirmwareWriteBit0581(u8 value);
#define SET_CONTROLLER_FLAG(enabled) SetControllerFlagFC07Bit2AndPort8098Bit6(enabled)
#define READ_PORT_8058() FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018058)
#define CONFIGURE_CONTROLLER(enabled) ConfigureControllerStartupModeRegisters(enabled)
#define SET_P1_TIMER_PINS() SetP1Bits0And1()
#define ENABLE_EXTERNAL_INTERRUPT0() SetExternalInterrupt0EnableAtomic()
#define WRITE_PARAMETER_D6() WriteControllerParameterD6Value00()
#define SET_P3_HANDSHAKE()                                                                                                                                                                             \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit0580(1u);                                                                                                                                                                      \
        FirmwareWriteBit0581(0u);                                                                                                                                                                      \
    } while(0)
#define DELAY_TIMER_UNITS_59() DelayTimerUnits59()
#define READ_PORT_8043() FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043)
#define YIELD_COROUTINE0() YieldCoroutine0()
#endif
#endif
