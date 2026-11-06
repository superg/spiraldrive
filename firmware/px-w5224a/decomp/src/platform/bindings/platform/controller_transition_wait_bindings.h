#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSetP1TimerInputsForOperatingCodeSettling(void);
extern u8 ObserveReadP0Bit7ForOperatingCodeSettling(void);
extern u8 ObserveReadP1Cex0ForOperatingCodeSettling(void);
extern void ObserveYieldCoroutine1ForOperatingCodeSettling(void);
extern void ObserveFinalizeP1TimerInputsForOperatingCodeSettling(void);
extern u32 ObserveCaptureInheritedDR12ForOperatingCodeSettling(void);
extern void ObserveRestoreInheritedDR12ForOperatingCodeSettling(u32 value);
extern void ObserveYieldCoroutine1ForControllerMeasurementWait(void);
extern u8 ObserveControllerMeasurementWithinTolerance(u8 tolerance_percent, u8 lower_bound_only);
extern u16 ObserveReadControllerWordForMeasurementWait(u16 selector);
extern void ObserveSetP1BitsForControllerMeasurementWait(void);
extern void ObserveSubmitControllerWordForMeasurementWait(u16 value, u16 selector);
extern void ObserveStrobeControllerPortForMeasurementWait(fw_addr_t address, u8 value);
extern void ObserveSetControllerChannelForMeasurementWait(u8 channel, u8 value);
#define SET_P1_TIMER_INPUTS() ObserveSetP1TimerInputsForOperatingCodeSettling()
#define READ_P0_BIT7() ObserveReadP0Bit7ForOperatingCodeSettling()
#define READ_P1_CEX0() ObserveReadP1Cex0ForOperatingCodeSettling()
#define YIELD_COROUTINE1() ObserveYieldCoroutine1ForOperatingCodeSettling()
#define FINALIZE_P1_TIMER_INPUTS() ObserveFinalizeP1TimerInputsForOperatingCodeSettling()
#define CAPTURE_INHERITED_DR12() ObserveCaptureInheritedDR12ForOperatingCodeSettling()
#define RESTORE_INHERITED_DR12(value) ObserveRestoreInheritedDR12ForOperatingCodeSettling(value)
#define MEASUREMENT_WAIT_YIELD() ObserveYieldCoroutine1ForControllerMeasurementWait()
#define MEASUREMENT_WITHIN_TOLERANCE(percent, lower_only) ObserveControllerMeasurementWithinTolerance((percent), (lower_only))
#define MEASUREMENT_READ_WORD(selector) ObserveReadControllerWordForMeasurementWait(selector)
#define MEASUREMENT_SET_P1_BITS() ObserveSetP1BitsForControllerMeasurementWait()
#define MEASUREMENT_SUBMIT_WORD(value, selector) ObserveSubmitControllerWordForMeasurementWait((value), (selector))
#define MEASUREMENT_STROBE_PORT(address, value) ObserveStrobeControllerPortForMeasurementWait((address), (value))
#define MEASUREMENT_SET_CHANNEL(channel, value) ObserveSetControllerChannelForMeasurementWait((channel), (value))
#else
extern void SetP1Bits0And1(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadP1Bit3Atomic(void);
extern void YieldCoroutine1(void);
extern u32 PLEXTOR_NEAR_FUNCTION CaptureInheritedDR12ForEventBit7Transfer(void);
extern void PLEXTOR_NEAR_FUNCTION RestoreInheritedDR12ForEventBit7Transfer(u32 value);
#define SET_P1_TIMER_INPUTS() SetP1Bits0And1()
#define READ_P0_BIT7() FirmwareReadBit0407()
#define READ_P1_CEX0() ReadP1Bit3Atomic()
#define YIELD_COROUTINE1() YieldCoroutine1()
#define FINALIZE_P1_TIMER_INPUTS() SetP1Bits0And1()
#define CAPTURE_INHERITED_DR12() CaptureInheritedDR12ForEventBit7Transfer()
#define RESTORE_INHERITED_DR12(value) RestoreInheritedDR12ForEventBit7Transfer(value)
#define MEASUREMENT_WAIT_YIELD() YieldCoroutine1()
#define MEASUREMENT_WITHIN_TOLERANCE(percent, lower_only) IsControllerPort80A8A9WithinScaledTolerance((percent), (lower_only))
#define MEASUREMENT_READ_WORD(selector) ReadControllerWord(selector)
#define MEASUREMENT_SET_P1_BITS() SetP1Bits0And1()
#define MEASUREMENT_SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define MEASUREMENT_STROBE_PORT(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define MEASUREMENT_SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#endif
#endif
