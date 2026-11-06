#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define CLEAR_CONTROLLER_PACKET_P1_BIT1() FirmwareWriteDirectSfr8(91, (u8)(FirmwareReadDirectSfr8(91) & 0xfdu))
#define CLEAR_CONTROLLER_PACKET_GLOBAL_INTERRUPT() FirmwareWriteBit0547(0u)
#define CLEAR_CONTROLLER_PACKET_EX1() FirmwareWriteBit0542(0u)
#else
sbit ControllerPacketP1Bit1 = 0x91;
sbit ControllerPacketGlobalInterruptEnable = 0xaf;
sbit ControllerPacketExternalInterrupt1Enable = 0xaa;
#define CLEAR_CONTROLLER_PACKET_P1_BIT1() (ControllerPacketP1Bit1 = 0)
#define CLEAR_CONTROLLER_PACKET_GLOBAL_INTERRUPT() (ControllerPacketGlobalInterruptEnable = 0)
#define CLEAR_CONTROLLER_PACKET_EX1() (ControllerPacketExternalInterrupt1Enable = 0)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadControllerPortForEightChannelCapture(fw_addr_t port_address);
#define READ_EIGHT_CHANNEL_PORT_8083() ObserveReadControllerPortForEightChannelCapture(FW_CONTROLLER_UNKNOWN_018083)
#define READ_EIGHT_CHANNEL_PORT_8084() ObserveReadControllerPortForEightChannelCapture(FW_CONTROLLER_UNKNOWN_018084)
#else
extern u8 ReadControllerPort8083ForEightChannelCapture(void);
extern u8 ReadControllerPort8084ForEightChannelCapture(void);
#define READ_EIGHT_CHANNEL_PORT_8083() ReadControllerPort8083ForEightChannelCapture()
#define READ_EIGHT_CHANNEL_PORT_8084() ReadControllerPort8084ForEightChannelCapture()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveConvertFullTocMsfForA2Matcher(fw_addr_t tuple_address);
#define CONVERT_FULL_TOC_MSF_FOR_A2_MATCHER(address) ObserveConvertFullTocMsfForA2Matcher(address)
#else
extern s32 ConvertFullTocMsfForA2Matcher(fw_addr_t tuple_address);
#define CONVERT_FULL_TOC_MSF_FOR_A2_MATCHER(address) ConvertFullTocMsfForA2Matcher(address)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRestoreControllerParametersForCommand0EFinalizer(void);
extern void ObserveWriteControllerParameterWordForCommand0EFinalizer(u16 parameter_and_value);
extern s32 ObserveDivideS32ForCommand0EFinalizer(s32 dividend, s32 divisor);
extern u32 ObserveDivideU32ForCommand0EFinalizer(u32 dividend, u32 divisor);
#define RESTORE_COMMAND0E_CONTROLLER_PARAMETERS() ObserveRestoreControllerParametersForCommand0EFinalizer()
#define WRITE_COMMAND0E_CONTROLLER_PARAMETER(value) ObserveWriteControllerParameterWordForCommand0EFinalizer(value)
#define DIVIDE_COMMAND0E_SIGNED(dividend, divisor) ObserveDivideS32ForCommand0EFinalizer((dividend), (divisor))
#define DIVIDE_COMMAND0E_UNSIGNED(dividend, divisor) ObserveDivideU32ForCommand0EFinalizer((dividend), (divisor))
#else
extern void WriteControllerParametersD412AndE2Value58(void);
extern void WriteControllerParameterWord(u16 parameter_and_value);
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
#define RESTORE_COMMAND0E_CONTROLLER_PARAMETERS() WriteControllerParametersD412AndE2Value58()
#define WRITE_COMMAND0E_CONTROLLER_PARAMETER(value) WriteControllerParameterWord(value)
#define DIVIDE_COMMAND0E_SIGNED(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#define DIVIDE_COMMAND0E_UNSIGNED(dividend, divisor) DivideU32QuotientNative((dividend), (divisor))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveClearTimer1OverflowForCommand0EWordRead(void);
extern void ObserveSetTimer1RunForCommand0EWordRead(void);
extern void ObserveClearTimer1RunForCommand0EWordRead(void);
extern u8 ObserveTimer1OverflowForCommand0EWordRead(void);
extern u8 ObserveControllerStatusForCommand0EWordRead(void);
extern void ObserveControllerPortWriteForCommand0EWordRead(fw_addr_t port_address, u8 value);
extern u8 ObserveControllerPortReadForCommand0EWordRead(fw_addr_t port_address);
extern void ObserveControllerTimeoutWaitForCommand0EWordRead(void);
#define CLEAR_COMMAND0E_TIMER_OVERFLOW() ObserveClearTimer1OverflowForCommand0EWordRead()
#define SET_COMMAND0E_TIMER_RUN() ObserveSetTimer1RunForCommand0EWordRead()
#define CLEAR_COMMAND0E_TIMER_RUN() ObserveClearTimer1RunForCommand0EWordRead()
#define READ_COMMAND0E_TIMER_OVERFLOW() ObserveTimer1OverflowForCommand0EWordRead()
#define READ_COMMAND0E_CONTROLLER_STATUS() ObserveControllerStatusForCommand0EWordRead()
#define WRITE_COMMAND0E_CONTROLLER_PORT(address, value) ObserveControllerPortWriteForCommand0EWordRead((address), (value))
#define READ_COMMAND0E_CONTROLLER_PORT(address) ObserveControllerPortReadForCommand0EWordRead(address)
#define WAIT_COMMAND0E_CONTROLLER_TIMEOUT() ObserveControllerTimeoutWaitForCommand0EWordRead()
#else
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1OverflowAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetTimer1RunAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1RunAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadTimer1OverflowAtomic(void);
extern void WaitControllerPortReadyAndWriteByte(fw_addr_t port_address, u8 value);
extern u8 WaitControllerPortReadyAndReadByte(fw_addr_t port_address);
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearWithoutTimeout(void);
#define CLEAR_COMMAND0E_TIMER_OVERFLOW() ClearTimer1OverflowAtomic()
#define SET_COMMAND0E_TIMER_RUN() SetTimer1RunAtomic()
#define CLEAR_COMMAND0E_TIMER_RUN() ClearTimer1RunAtomic()
#define READ_COMMAND0E_TIMER_OVERFLOW() ReadTimer1OverflowAtomic()
#define READ_COMMAND0E_CONTROLLER_STATUS() FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8)
#define WRITE_COMMAND0E_CONTROLLER_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define READ_COMMAND0E_CONTROLLER_PORT(address) WaitControllerPortReadyAndReadByte(address)
#define WAIT_COMMAND0E_CONTROLLER_TIMEOUT() WaitControllerPort80F8Bit7ClearWithoutTimeout()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveIssueControllerCommand0EForAccumulator(void);
extern s16 ObserveCaptureControllerD8D9ForCommand0EAccumulator(void);
extern u16 ObserveReadControllerWordForCommand0EAccumulator(void);
#define ISSUE_COMMAND0E_FOR_ACCUMULATOR() ObserveIssueControllerCommand0EForAccumulator()
#define CAPTURE_COMMAND0E_PRIMARY_SAMPLE() ObserveCaptureControllerD8D9ForCommand0EAccumulator()
#define READ_COMMAND0E_COMPANION_WORD() ObserveReadControllerWordForCommand0EAccumulator()
#else
extern void IssueControllerCommand0E(void);
extern s16 CaptureControllerD8D9WindowAndUpdateAdjustment(void);
#define ISSUE_COMMAND0E_FOR_ACCUMULATOR() IssueControllerCommand0E()
#define CAPTURE_COMMAND0E_PRIMARY_SAMPLE() CaptureControllerD8D9WindowAndUpdateAdjustment()
#define READ_COMMAND0E_COMPANION_WORD() ReadControllerCommand0EMeasurementWordWithTimeout()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0547((u8)(saved_ie >> 7));
#endif
#endif
