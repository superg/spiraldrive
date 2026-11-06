#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePort80F8AttemptSetControllerFlag(u8 enabled);
extern void ObservePort80F8AttemptWriteParameters(u8 value);
extern void ObservePort80F8AttemptWritePort(fw_addr_t address, u8 value);
extern u8 ObservePort80F8AttemptReadPort(fw_addr_t address);
extern void ObservePort80F8AttemptWaitForClear(void);
extern void ObservePort80F8AttemptWaitTimer(u16 duration, u8 yield_mode);
extern void ObservePort80F8AttemptSetChannel(u8 channel, u8 value);
extern void ObservePort80F8AttemptResetChannel(u8 channel);
extern void ObservePort80F8AttemptYield(void);
#define PORT80F8_SET_FLAG(value) ObservePort80F8AttemptSetControllerFlag(value)
#define PORT80F8_WRITE_PARAMETERS(value) ObservePort80F8AttemptWriteParameters(value)
#define PORT80F8_WRITE_PORT(address, value) ObservePort80F8AttemptWritePort((address), (value))
#define PORT80F8_READ_PORT(address) ObservePort80F8AttemptReadPort(address)
#define PORT80F8_WAIT_FOR_CLEAR() ObservePort80F8AttemptWaitForClear()
#define PORT80F8_WAIT_TIMER(duration, yield_mode) ObservePort80F8AttemptWaitTimer((duration), (yield_mode))
#define PORT80F8_SET_CHANNEL(channel, value) ObservePort80F8AttemptSetChannel((channel), (value))
#define PORT80F8_RESET_CHANNEL(channel) ObservePort80F8AttemptResetChannel(channel)
#define PORT80F8_YIELD() ObservePort80F8AttemptYield()
#else
#include "platform/arithmetic.h"
#include "platform/controller_parameters.h"
extern void YieldCoroutine1(void);
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearZeroFalseFalse(void);
#define PORT80F8_SET_FLAG(value) SetControllerFlagFC07Bit2AndPort8098Bit6(value)
#define PORT80F8_WRITE_PARAMETERS(value) WriteControllerParametersE8E9ForValue(value)
#define PORT80F8_WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define PORT80F8_READ_PORT(address) WaitControllerPortReadyAndReadByte(address)
#define PORT80F8_WAIT_FOR_CLEAR() WaitControllerPort80F8Bit7ClearZeroFalseFalse()
#define PORT80F8_WAIT_TIMER(duration, yield_mode) WaitTimer1ScaledDuration((duration), (yield_mode))
#define PORT80F8_SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#define PORT80F8_RESET_CHANNEL(channel) ResetIndexedControllerChannel(channel)
#define PORT80F8_YIELD() YieldCoroutine1()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
static void WritePort80F8SharedCountdown(s32 value)
{
    u32 bits = (u32)value;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FC, (u8)(bits >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FD, (u8)(bits >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FE, (u8)(bits >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11FF, (u8)bits);
}

static s32 ReadPort80F8SharedCountdown(void)
{
    u32 bits = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_11FC) << 24;
    bits |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_11FD) << 16;
    bits |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_11FE) << 8;
    bits |= FirmwareReadNear8(FW_NEAR_UNKNOWN_11FF);
    return (s32)bits;
}
#define PORT80F8_WRITE_SHARED_COUNTDOWN(value) WritePort80F8SharedCountdown(value)
#define PORT80F8_READ_SHARED_COUNTDOWN() ReadPort80F8SharedCountdown()
#else
#define PORT80F8_WRITE_SHARED_COUNTDOWN(value) (*(volatile s32 huge *)FW_NEAR_UNKNOWN_11FC = (value))
#define PORT80F8_READ_SHARED_COUNTDOWN() (*(volatile s32 huge *)FW_NEAR_UNKNOWN_11FC)
#endif
#endif
