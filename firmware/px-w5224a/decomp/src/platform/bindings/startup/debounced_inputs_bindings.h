#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveStartupDebouncedP1Bit7Low(void);
extern u8 ObserveStartupDebouncedP1Bit5Low(void);
extern u8 ObserveStartupDebouncedP1Bit6Low(void);
extern void ObserveStartupYieldCoroutine1(void);
extern void ObserveStartupClearP1AndControllerWord0252(void);
extern void ObserveStartupUpdateP3Pattern(u8 restart_pattern);
extern void ObserveStartupApplyConfigurationFlag(void);
extern u8 ObserveStartupTakeStateFlag11EFBit5(void);
#define READ_P1_BIT7_LOW() ObserveStartupDebouncedP1Bit7Low()
#define READ_P1_BIT5_LOW() ObserveStartupDebouncedP1Bit5Low()
#define READ_P1_BIT6_LOW() ObserveStartupDebouncedP1Bit6Low()
#define STARTUP_YIELD() ObserveStartupYieldCoroutine1()
#define CLEAR_P1_AND_CONTROLLER_WORD() ObserveStartupClearP1AndControllerWord0252()
#define UPDATE_P3_PATTERN(restart) ObserveStartupUpdateP3Pattern(restart)
#define APPLY_CONFIGURATION_FLAG() ObserveStartupApplyConfigurationFlag()
#define TAKE_STATE_FLAG() ObserveStartupTakeStateFlag11EFBit5()
#else
extern u8 IsDebouncedP1Bit7Low(void);
extern u8 IsDebouncedP1Bit5Low(void);
extern u8 IsDebouncedP1Bit6Low(void);
extern void YieldCoroutine1(void);
extern void PLEXTOR_NEAR_FUNCTION ClearP1Bits0And1AndSubmitControllerWord0252(void);
extern void UpdateP3Bits0And1PatternState(u8 restart_pattern);
extern void ApplyConfigurationFlagE750Bit0ToState1303Bit0(void);
extern u8 TakeStateFlag11EFBit5(void);
#define READ_P1_BIT7_LOW() IsDebouncedP1Bit7Low()
#define READ_P1_BIT5_LOW() IsDebouncedP1Bit5Low()
#define READ_P1_BIT6_LOW() IsDebouncedP1Bit6Low()
#define STARTUP_YIELD() YieldCoroutine1()
#define CLEAR_P1_AND_CONTROLLER_WORD() ClearP1Bits0And1AndSubmitControllerWord0252()
#define UPDATE_P3_PATTERN(restart) UpdateP3Bits0And1PatternState(restart)
#define APPLY_CONFIGURATION_FLAG() ApplyConfigurationFlagE750Bit0ToState1303Bit0()
#define TAKE_STATE_FLAG() TakeStateFlag11EFBit5()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerProbeSetP1Bits0And1(void);
extern void ObserveControllerProbeStrobeWrite(fw_addr_t port_address, u8 value);
extern void ObserveControllerProbeWrite(fw_addr_t port_address, u8 value);
extern u8 ObserveControllerProbeRead(fw_addr_t port_address);
extern void ObserveControllerProbeWait(u16 duration_units, u8 yield_while_waiting);
extern void ObserveControllerProbeResolveDriveControl(void);
#define PROBE_SET_P1_BITS() ObserveControllerProbeSetP1Bits0And1()
#define PROBE_STROBE_WRITE(address, value) ObserveControllerProbeStrobeWrite((address), (value))
#define PROBE_WRITE(address, value) ObserveControllerProbeWrite((address), (value))
#define PROBE_READ(address) ObserveControllerProbeRead(address)
#define PROBE_WAIT(duration, yield) ObserveControllerProbeWait((duration), (yield))
#define PROBE_RESOLVE_DRIVE_CONTROL() ObserveControllerProbeResolveDriveControl()
#else
#define PROBE_SET_P1_BITS() SetP1Bits0And1()
#define PROBE_STROBE_WRITE(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define PROBE_WRITE(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define PROBE_READ(address) WaitControllerPortReadyAndReadByte(address)
#define PROBE_WAIT(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define PROBE_RESOLVE_DRIVE_CONTROL() ResolveDriveControlStateToZero()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDriveState9WritePhaseBit(u8 value);
extern void ObserveDriveState9SubmitControllerWord(u8 select_value_006f);
extern u8 ObserveDriveState9DebouncedP1Bit7Low(void);
extern void ObserveDriveState9YieldCoroutine1(void);
extern u8 ObserveDriveState9TakeStateFlag11EFBit5(void);
extern void ObserveDriveState9Wait(u16 duration_units, u8 yield_while_waiting);
extern void ObserveDriveState9ClearControllerWord0252(void);
#define DRIVE_STATE9_WRITE_PHASE_BIT(value) ObserveDriveState9WritePhaseBit(value)
#define DRIVE_STATE9_SUBMIT_WORD(value) ObserveDriveState9SubmitControllerWord(value)
#define DRIVE_STATE9_READ_P1_BIT7_LOW() ObserveDriveState9DebouncedP1Bit7Low()
#define DRIVE_STATE9_YIELD() ObserveDriveState9YieldCoroutine1()
#define DRIVE_STATE9_TAKE_STATE_FLAG() ObserveDriveState9TakeStateFlag11EFBit5()
#define DRIVE_STATE9_WAIT(duration, yield) ObserveDriveState9Wait((duration), (yield))
#define DRIVE_STATE9_CLEAR_WORD() ObserveDriveState9ClearControllerWord0252()
#else
extern void SetBit003BAtomic(void);
extern void ClearBit003BAtomic(void);
extern void YieldCoroutine1(void);
#define DRIVE_STATE9_WRITE_PHASE_BIT(value) ((value) != 0u ? SetBit003BAtomic() : ClearBit003BAtomic())
#define DRIVE_STATE9_SUBMIT_WORD(value) SubmitSelectedControllerWord0252WithP1Bit0Set(value)
#define DRIVE_STATE9_READ_P1_BIT7_LOW() IsDebouncedP1Bit7Low()
#define DRIVE_STATE9_YIELD() YieldCoroutine1()
#define DRIVE_STATE9_TAKE_STATE_FLAG() TakeStateFlag11EFBit5()
#define DRIVE_STATE9_WAIT(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define DRIVE_STATE9_CLEAR_WORD() ClearControllerWord0252AndResetChannel2()
#endif
#endif
