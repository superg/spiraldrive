#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveVendorF3ControllerMode(u8 event, u32 argument0, u32 argument1);
#define VENDOR_F3_EVENT(event, argument0, argument1) ObserveVendorF3ControllerMode((event), (argument0), (argument1))
#define VENDOR_F3_SEQUENCE_ERROR() VENDOR_F3_EVENT(VENDOR_F3_MODE_SEQUENCE_ERROR, 0ul, 0ul)
#define VENDOR_F3_WRITE_D6() VENDOR_F3_EVENT(VENDOR_F3_MODE_WRITE_D6, 0ul, 0ul)
#define VENDOR_F3_CONFIGURE_STATE2() VENDOR_F3_EVENT(VENDOR_F3_MODE_CONFIGURE_STATE2, 0ul, 0ul)
#define VENDOR_F3_RESET_DATA_PATH() VENDOR_F3_EVENT(VENDOR_F3_MODE_RESET_DATA_PATH, 0ul, 0ul)
#define VENDOR_F3_SET_CONTROLLER_FLAG(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_SET_CONTROLLER_FLAG, (value), 0ul)
#define VENDOR_F3_WAIT_TIMER(duration, yield) VENDOR_F3_EVENT(VENDOR_F3_MODE_WAIT_TIMER, (duration), (yield))
#define VENDOR_F3_WRITE_E8E9(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_WRITE_E8E9, (value), 0ul)
#define VENDOR_F3_WRITE_REGISTER01(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_WRITE_REGISTER01, (value), 0ul)
#define VENDOR_F3_INITIALIZE_STRATEGY() VENDOR_F3_EVENT(VENDOR_F3_MODE_INITIALIZE_STRATEGY, 0ul, 0ul)
#define VENDOR_F3_CONFIGURE_PROFILE() VENDOR_F3_EVENT(VENDOR_F3_MODE_CONFIGURE_PROFILE, 0ul, 0ul)
#define VENDOR_F3_WRITE_EF00(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_WRITE_EF00, (value), 0ul)
#define VENDOR_F3_SET_GLOBAL_INTERRUPT(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_SET_GLOBAL_INTERRUPT, (value), 0ul)
#define VENDOR_F3_SET_CONTROLLER_INTERRUPT(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_SET_CONTROLLER_INTERRUPT, (value), 0ul)
#define VENDOR_F3_SET_A9_BIT2(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_SET_A9_BIT2, (value), 0ul)
#define VENDOR_F3_APPLY_RETAINED_WORD(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_APPLY_RETAINED_WORD, (value), 0ul)
#define VENDOR_F3_SET_P3_RX(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_SET_P3_RX, (value), 0ul)
#define VENDOR_F3_SET_P3_TX(value) VENDOR_F3_EVENT(VENDOR_F3_MODE_SET_P3_TX, (value), 0ul)
#define VENDOR_F3_NO_SENSE() VENDOR_F3_EVENT(VENDOR_F3_MODE_NO_SENSE, 0ul, 0ul)
#else
extern void ConfigureControllerRegisterProfileForCurrentMediaState(void);

sbit VendorF3GlobalInterruptEnable = 0xaf;
sbit VendorF3ControllerInterruptEnable = 0xae;
extern void SetSfrA9Bit2Atomic(void);
extern void ClearSfrA9Bit2Extended(void);
sbit VendorF3P3Rx = 0xb0;
sbit VendorF3P3Tx = 0xb1;

#define VENDOR_F3_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define VENDOR_F3_WRITE_D6() WriteControllerParameterD6Value00()
#define VENDOR_F3_CONFIGURE_STATE2() ConfigureControllerForParameterState2()
#define VENDOR_F3_RESET_DATA_PATH() ResetControllerDataPathRegisters()
#define VENDOR_F3_SET_CONTROLLER_FLAG(value) SetControllerFlagFC07Bit2AndPort8098Bit6(value)
#define VENDOR_F3_WAIT_TIMER(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define VENDOR_F3_WRITE_E8E9(value) WriteControllerParametersE8E9ForValue(value)
#define VENDOR_F3_WRITE_REGISTER01(value) WriteControllerRegister01Clamped(value)
#define VENDOR_F3_INITIALIZE_STRATEGY() InitializeMediaStrategyProfileAndWriteSpeedSelection()
#define VENDOR_F3_CONFIGURE_PROFILE() ConfigureControllerRegisterProfileForCurrentMediaState()
#define VENDOR_F3_WRITE_EF00(value) WriteControllerParameterEF00AndConditionalD0Value(value)
#define VENDOR_F3_SET_GLOBAL_INTERRUPT(value) (VendorF3GlobalInterruptEnable = (value))
#define VENDOR_F3_SET_CONTROLLER_INTERRUPT(value) (VendorF3ControllerInterruptEnable = (value))
#define VENDOR_F3_SET_A9_BIT2(value) ((value) ? SetSfrA9Bit2Atomic() : ClearSfrA9Bit2Extended())
#define VENDOR_F3_APPLY_RETAINED_WORD(value) ApplyVendorF3ControllerWordInRange(value)
#define VENDOR_F3_SET_P3_RX(value) (VendorF3P3Rx = (value))
#define VENDOR_F3_SET_P3_TX(value) (VendorF3P3Tx = (value))
#define VENDOR_F3_NO_SENSE() SetNoSenseData()
#endif
#endif
