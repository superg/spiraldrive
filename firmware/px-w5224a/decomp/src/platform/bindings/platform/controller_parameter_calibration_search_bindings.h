#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerParameterCalibrationInterrupts(u8 enabled);
#define DISABLE_CALIBRATION_INTERRUPTS() ObserveControllerParameterCalibrationInterrupts(0u)
#define RESTORE_CALIBRATION_INTERRUPTS() ObserveControllerParameterCalibrationInterrupts(1u)
#else
sbit ControllerParameterCalibrationGlobalInterruptEnable = 0xaf;
sbit ControllerParameterCalibrationControllerInterruptEnable = 0xae;
extern void SetSfrA9Bit2Atomic(void);
#define DISABLE_CALIBRATION_INTERRUPTS() (ControllerParameterCalibrationGlobalInterruptEnable = 0)
#define RESTORE_CALIBRATION_INTERRUPTS()                                                                                                                                                               \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ControllerParameterCalibrationGlobalInterruptEnable = 1;                                                                                                                                       \
        ControllerParameterCalibrationControllerInterruptEnable = 1;                                                                                                                                   \
        SetSfrA9Bit2Atomic();                                                                                                                                                      \
    } while(0)
#endif
#endif
