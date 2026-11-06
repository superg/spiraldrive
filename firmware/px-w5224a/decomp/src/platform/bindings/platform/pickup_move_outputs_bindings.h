#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadScaledRadialMovePort(fw_addr_t port_address);
extern void ObserveWriteScaledRadialMovePort(fw_addr_t port_address, u8 value);
extern void ObserveClearScaledRadialMoveExternalInterrupt1(void);
extern void ObserveClearScaledRadialMoveControllerChannel86(void);
extern void ObserveSubmitScaledRadialMoveControllerWord(u16 value, u16 selector);
extern void ObserveClearScaledRadialMoveParameterD1Bit6(void);
extern void ObserveWriteScaledRadialMoveParametersD2D3(void);
extern void ObserveWriteScaledRadialMoveParameterWord(u16 value);
extern void ObserveApplyScaledRadialMoveTransitionProfile(u8 direction);
extern void ObserveHomePickupWritePort(fw_addr_t port_address, u8 value);
extern void ObserveHomePickupConfigureDrive(u8 drive_code, u8 enabled);
extern void ObserveHomePickupYield(void);
extern void ObserveHomePickupResetChannel2(void);
#define READ_MOVE_PORT(address) ObserveReadScaledRadialMovePort(address)
#define WRITE_MOVE_PORT(address, value) ObserveWriteScaledRadialMovePort((address), (value))
#define CLEAR_MOVE_EXTERNAL_INTERRUPT1() ObserveClearScaledRadialMoveExternalInterrupt1()
#define CLEAR_MOVE_CONTROLLER_CHANNEL86() ObserveClearScaledRadialMoveControllerChannel86()
#define SUBMIT_MOVE_CONTROLLER_WORD(value, selector) ObserveSubmitScaledRadialMoveControllerWord((value), (selector))
#define CLEAR_MOVE_PARAMETER_D1_BIT6() ObserveClearScaledRadialMoveParameterD1Bit6()
#define WRITE_MOVE_PARAMETERS_D2_D3() ObserveWriteScaledRadialMoveParametersD2D3()
#define WRITE_MOVE_PARAMETER_WORD(value) ObserveWriteScaledRadialMoveParameterWord(value)
#define APPLY_MOVE_TRANSITION_PROFILE(direction) ObserveApplyScaledRadialMoveTransitionProfile(direction)
#define HOME_PICKUP_WRITE_PORT(address, value) ObserveHomePickupWritePort((address), (value))
#define HOME_PICKUP_CONFIGURE_DRIVE(code, enabled) ObserveHomePickupConfigureDrive((code), (enabled))
#define HOME_PICKUP_YIELD() ObserveHomePickupYield()
#define HOME_PICKUP_RESET_CHANNEL2() ObserveHomePickupResetChannel2()
#define READ_HOME_PICKUP_P1_BIT1() ((u8)((FirmwareReadDirectSfr8(90) >> 1) & 1u))
#define WRITE_HOME_PICKUP_P1_BIT1(value)                                                                                                                                                               \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        u8 home_pickup_p1 = FirmwareReadDirectSfr8(90);                                                                                                                                                \
        if((value) != 0u)                                                                                                                                                                              \
            home_pickup_p1 |= 2u;                                                                                                                                                                      \
        else                                                                                                                                                                                           \
            home_pickup_p1 &= 0xfdu;                                                                                                                                                                   \
        FirmwareWriteDirectSfr8(90, home_pickup_p1);                                                                                                                                                   \
    } while(0)
#else
extern void PLEXTOR_NEAR_FUNCTION ClearBit0542Atomic(void);
extern void YieldCoroutine1(void);
sbit HomePickupP1Bit1 = 0x91;
/* The native caller forwards ACC, but RAM:fd016f does not consume it. */
static void ApplyMoveTransitionProfile(u8 direction)
{
    if(direction == 0u)
        ApplyControllerMeasurementReferenceAndTransitionProfile();
    else
        ApplyControllerMeasurementReferenceAndTransitionProfile();
}
#define READ_MOVE_PORT(address) WaitControllerPortReadyAndReadByte(address)
#define WRITE_MOVE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define CLEAR_MOVE_EXTERNAL_INTERRUPT1() ClearBit0542Atomic()
#define CLEAR_MOVE_CONTROLLER_CHANNEL86() ClearControllerChannel86ForOperatingThresholds()
#define SUBMIT_MOVE_CONTROLLER_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define CLEAR_MOVE_PARAMETER_D1_BIT6() ClearControllerParameterD1ShadowBit6()
#define WRITE_MOVE_PARAMETERS_D2_D3() WriteControllerParametersD2AndD3FromState()
#define WRITE_MOVE_PARAMETER_WORD(value) WriteControllerParameterWord(value)
#define APPLY_MOVE_TRANSITION_PROFILE(direction) ApplyMoveTransitionProfile(direction)
#define HOME_PICKUP_WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define HOME_PICKUP_CONFIGURE_DRIVE(code, enabled) ConfigureControllerDrivePorts8496Through849D((code), (enabled))
#define HOME_PICKUP_YIELD() YieldCoroutine1()
#define HOME_PICKUP_RESET_CHANNEL2() ResetControllerChannel2()
#define READ_HOME_PICKUP_P1_BIT1() ((u8)HomePickupP1Bit1)
#define WRITE_HOME_PICKUP_P1_BIT1(value) (HomePickupP1Bit1 = ((value) != 0u))
#endif
#endif
