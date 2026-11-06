#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveOperatingResetConfigurePorts(void);
extern void ObserveOperatingResetStrobePort(fw_addr_t address, u8 value);
extern void ObserveOperatingResetConfigureParameters(u8 set_p0_bit3);
extern void ObserveOperatingResetApplyParameterD1(void);
extern void ObserveOperatingResetInitializeRegisterBlock(void);
extern void ObserveOperatingResetResetParameterState(void);
extern void ObserveOperatingResetWritePort(fw_addr_t address, u8 value);
extern void ObserveOperatingResetSetControllerFlag(u8 enabled);
extern void ObserveOperatingResetResetChannels(void);
extern u8 ObserveOperatingResetReadPort(fw_addr_t address);
extern void ObserveOperatingResetLoadProfile(u8 profile_index);
extern void ObserveOperatingResetInitializeD4D5(void);
extern u8 ObserveOperatingResetReadP1Bit(u8 bit_index);
extern void ObserveOperatingResetWriteP1Bit(u8 bit_index, u8 value);
extern void ObserveOperatingResetTail(void);
extern void ObserveOperatingResetResetPorts(void);
extern void ObserveOperatingResetInitializeC2D0D3(void);
extern void ObserveOperatingResetInitializeD8D9(void);
extern void ObserveOperatingResetClearDCDD(void);
extern void ObserveOperatingResetSubmitByteWord(u8 value, u16 selector);
extern void ObserveOperatingResetSubmitWordPair(u16 value, u16 selector);
#define OPERATING_RESET_CONFIGURE_PORTS() ObserveOperatingResetConfigurePorts()
#define OPERATING_RESET_STROBE(address, value) ObserveOperatingResetStrobePort((address), (value))
#define OPERATING_RESET_CONFIGURE_PARAMETERS(value) ObserveOperatingResetConfigureParameters(value)
#define OPERATING_RESET_APPLY_D1() ObserveOperatingResetApplyParameterD1()
#define OPERATING_RESET_INITIALIZE_BLOCK() ObserveOperatingResetInitializeRegisterBlock()
#define OPERATING_RESET_RESET_PARAMETERS() ObserveOperatingResetResetParameterState()
#define OPERATING_RESET_WRITE_PORT(address, value) ObserveOperatingResetWritePort((address), (value))
#define OPERATING_RESET_SET_FLAG(value) ObserveOperatingResetSetControllerFlag(value)
#define OPERATING_RESET_RESET_CHANNELS() ObserveOperatingResetResetChannels()
#define OPERATING_RESET_READ_PORT(address) ObserveOperatingResetReadPort(address)
#define OPERATING_RESET_LOAD_PROFILE(index) ObserveOperatingResetLoadProfile(index)
#define OPERATING_RESET_INITIALIZE_D4_D5() ObserveOperatingResetInitializeD4D5()
#define OPERATING_RESET_READ_P1_BIT0() ObserveOperatingResetReadP1Bit(0u)
#define OPERATING_RESET_READ_P1_BIT1() ObserveOperatingResetReadP1Bit(1u)
#define OPERATING_RESET_WRITE_P1_BIT0(value) ObserveOperatingResetWriteP1Bit(0u, (value))
#define OPERATING_RESET_WRITE_P1_BIT1(value) ObserveOperatingResetWriteP1Bit(1u, (value))
#define OPERATING_RESET_TAIL() ObserveOperatingResetTail()
#define OPERATING_RESET_RESET_PORTS() ObserveOperatingResetResetPorts()
#define OPERATING_RESET_INITIALIZE_C2_D0_D3() ObserveOperatingResetInitializeC2D0D3()
#define OPERATING_RESET_INITIALIZE_D8_D9() ObserveOperatingResetInitializeD8D9()
#define OPERATING_RESET_CLEAR_DC_DD() ObserveOperatingResetClearDCDD()
#define OPERATING_RESET_SUBMIT_BYTE_WORD(value, selector) ObserveOperatingResetSubmitByteWord((value), (selector))
#define OPERATING_RESET_SUBMIT_WORD(value, selector) ObserveOperatingResetSubmitWordPair((value), (selector))
#else
#include "platform/controller_parameters.h"
#include "platform/status_mapping.h"
sbit OperatingResetP1Bit0 = 0x90;
sbit OperatingResetP1Bit1 = 0x91;
#define OPERATING_RESET_CONFIGURE_PORTS() ConfigureControllerPorts8086And809DThrough80AE()
#define OPERATING_RESET_STROBE(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define OPERATING_RESET_CONFIGURE_PARAMETERS(value) ConfigureP0Bit3AndControllerParametersE7E5(value)
#define OPERATING_RESET_APPLY_D1() ApplyControllerParameterD1ModeBits()
#define OPERATING_RESET_INITIALIZE_BLOCK() InitializeControllerRegisterBlock81ThroughBD()
#define OPERATING_RESET_RESET_PARAMETERS() ResetControllerParameterState()
#define OPERATING_RESET_WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define OPERATING_RESET_SET_FLAG(value) SetControllerFlagFC07Bit2AndPort8098Bit6(value)
#define OPERATING_RESET_RESET_CHANNELS() ResetFourIndexedControllerChannels()
#define OPERATING_RESET_READ_PORT(address) WaitControllerPortReadyAndReadByte(address)
#define OPERATING_RESET_LOAD_PROFILE(index) LoadControllerOperatingProfile(index)
#define OPERATING_RESET_INITIALIZE_D4_D5() InitializeControllerRegistersD4D5AndWord23F()
#define OPERATING_RESET_READ_P1_BIT0() ((u8)OperatingResetP1Bit0)
#define OPERATING_RESET_READ_P1_BIT1() ((u8)OperatingResetP1Bit1)
#define OPERATING_RESET_WRITE_P1_BIT0(value) (OperatingResetP1Bit0 = ((value) != 0u))
#define OPERATING_RESET_WRITE_P1_BIT1(value) (OperatingResetP1Bit1 = ((value) != 0u))
#define OPERATING_RESET_TAIL() ((void)ResetControllerOperatingStateTail())
#define OPERATING_RESET_RESET_PORTS() ResetControllerPorts8087Through808B()
#define OPERATING_RESET_INITIALIZE_C2_D0_D3() InitializeControllerRegistersC2D0ThroughD3()
#define OPERATING_RESET_INITIALIZE_D8_D9() InitializeControllerRegistersD8D9()
#define OPERATING_RESET_CLEAR_DC_DD() ClearControllerRegistersDCDDAndWord23E()
#define OPERATING_RESET_SUBMIT_BYTE_WORD(value, selector) SubmitControllerByteAndWord((value), (selector))
#define OPERATING_RESET_SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#endif
#endif
