#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCommandC0Yield(void);
extern u8 ObserveCommandC0PulseTest(u8 sample_count);
extern void ObserveCommandC0PortWrite(fw_addr_t address, u8 value);
extern u8 ObserveCommandC0IndexedState(u8 state_code);
extern void ObserveCommandC0SubmitWord(u16 value, u16 selector);
extern u8 ObserveCommandC0Wait2000(void);
extern void ObserveCommandC0Abort(void);
extern u16 ObserveCommandC0ReadWord(u16 selector);
extern void ObserveCommandC0TimerSet(s32 value);
extern s32 ObserveCommandC0TimerRead(void);
#define CommandC0SetTimer(value) ObserveCommandC0TimerSet(value)
#define CommandC0ReadTimer() ObserveCommandC0TimerRead()
#define CommandC0Yield() ObserveCommandC0Yield()
#define CommandC0PulseTest(count) ObserveCommandC0PulseTest(count)
#define CommandC0WritePort(address, value) ObserveCommandC0PortWrite((address), (value))
#define CommandC0IndexedState(state) ObserveCommandC0IndexedState(state)
#define CommandC0SubmitWord(value, selector) ObserveCommandC0SubmitWord((value), (selector))
#define CommandC0Wait2000() ObserveCommandC0Wait2000()
#define CommandC0Abort() ObserveCommandC0Abort()
#define CommandC0ReadWord(selector) ObserveCommandC0ReadWord(selector)
extern u8 ObserveWordCalibrationIndexedState(u8 state_code);
extern u8 ObserveWordCalibrationCommandC0(void);
extern u8 ObserveWordCalibrationCommandD0(void);
extern u16 ObserveWordCalibrationReadWord(u16 selector);
extern void ObserveWordCalibrationPortWrite(fw_addr_t address, u8 value);
extern s32 ObserveWordCalibrationTransform(u16 value);
extern u8 ObserveWordCalibrationP0Bit7(void);
#define WordCalibrationIndexedState(state) ObserveWordCalibrationIndexedState(state)
#define WordCalibrationCommandC0() ObserveWordCalibrationCommandC0()
#define WordCalibrationCommandD0() ObserveWordCalibrationCommandD0()
#define WordCalibrationReadWord(selector) ObserveWordCalibrationReadWord(selector)
#define WordCalibrationWritePort(address, value) ObserveWordCalibrationPortWrite((address), (value))
#define WordCalibrationTransform(value) ObserveWordCalibrationTransform(value)
#define WordCalibrationP0Bit7() ObserveWordCalibrationP0Bit7()
#else
extern void YieldCoroutine1(void);
extern u8 PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7Clear2000AndYield(void);
#define CommandC0SetTimer(value) (*(volatile s32 huge *)FW_NEAR_UNKNOWN_11FC = (value))
#define CommandC0ReadTimer() (*(volatile s32 huge *)FW_NEAR_UNKNOWN_11FC)
#define CommandC0Yield() YieldCoroutine1()
#define CommandC0PulseTest(count) TestP1Cex0PulseSuccessThreshold(count)
#define CommandC0WritePort(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define CommandC0IndexedState(state) IsIndexedDriveStateActive(state)
#define CommandC0SubmitWord(value, selector) SubmitControllerWordPair((value), (selector))
#define CommandC0Wait2000() WaitControllerPort80F8Bit7Clear2000AndYield()
#define CommandC0Abort() AbortControllerCommandAndClearWords032A033A()
#define CommandC0ReadWord(selector) ReadControllerWord(selector)
#define WordCalibrationIndexedState(state) IsIndexedDriveStateActive(state)
#define WordCalibrationCommandC0() RunControllerCommandC0AndAdjustWord030A()
#define WordCalibrationCommandD0() RunControllerCommandD0AndAdjustWord031A()
#define WordCalibrationReadWord(selector) ReadControllerWord(selector)
#define WordCalibrationWritePort(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define WordCalibrationTransform(value) Compute2000TimesLog10OfWordOver32767(value)
#define WordCalibrationP0Bit7() FirmwareReadBit0407()
#endif
#endif
