#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetChannelForControllerCommand03(u8 channel_index);
extern u16 ObserveReadWordForControllerCommand03(u16 selector);
extern void ObserveSubmitWordForControllerCommand03(u16 value, u16 selector);
extern void ObserveWritePortForControllerCommand03(fw_addr_t port_address, u8 value);
extern void ObserveWaitTimerForControllerCommand03(u16 duration_units, u8 yield_while_waiting);
extern u8 ObserveWait500ForControllerCommand03(void);
extern void ObserveWaitWithoutTimeoutForControllerCommand03(void);
extern u8 ObserveReadPortForControllerCommand03(fw_addr_t port_address);
extern void ObserveSetChannelForControllerCommand03(u8 channel_index, u8 value);
#define RESET_CHANNEL(index) ObserveResetChannelForControllerCommand03(index)
#define READ_WORD(selector) ObserveReadWordForControllerCommand03(selector)
#define SUBMIT_WORD(value, selector) ObserveSubmitWordForControllerCommand03((value), (selector))
#define WRITE_PORT(address, value) ObserveWritePortForControllerCommand03((address), (value))
#define WAIT_TIMER(duration, yield) ObserveWaitTimerForControllerCommand03((duration), (yield))
#define WAIT_500_AND_YIELD() ObserveWait500ForControllerCommand03()
#define WAIT_WITHOUT_TIMEOUT() ObserveWaitWithoutTimeoutForControllerCommand03()
#define READ_PORT(address) ObserveReadPortForControllerCommand03(address)
#define SET_CHANNEL(index, value) ObserveSetChannelForControllerCommand03((index), (value))
#else
extern u8 PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7Clear500AndYield(void);
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearWithoutTimeout(void);
#define RESET_CHANNEL(index) ResetIndexedControllerChannel(index)
#define READ_WORD(selector) ReadControllerWord(selector)
#define SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define WAIT_TIMER(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define WAIT_500_AND_YIELD() WaitControllerPort80F8Bit7Clear500AndYield()
#define WAIT_WITHOUT_TIMEOUT() WaitControllerPort80F8Bit7ClearWithoutTimeout()
#define READ_PORT(address) WaitControllerPortReadyAndReadByte(address)
#define SET_CHANNEL(index, value) SetIndexedControllerChannelValue((index), (value))
#endif
#endif
