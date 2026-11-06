#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCalibrationPathWriteE8E9(u8 value);
extern u8 ObserveCalibrationPathRunCalibration(void);
extern void ObserveCalibrationPathCaptureSamples(void);
extern void ObserveCalibrationPathStrobe(fw_addr_t address, u8 value);
extern void ObserveCalibrationPathSetChannel(u8 channel, u8 value);
extern void ObserveCalibrationPathWritePort(fw_addr_t address, u8 value);
extern void ObserveCalibrationPathWriteF6(u8 value);
extern void ObserveCalibrationPathWaitF8(void);
extern void ObserveCalibrationPathWaitTimer(u16 duration, u8 yield_mode);
extern void ObserveCalibrationPathSetControllerFlag(u8 enabled);
extern void ObserveCalibrationPathResetChannel(u8 channel);
extern u8 ObserveCalibrationPathReadPort(fw_addr_t address);
extern void ObserveCalibrationPathConfigureState2(void);
#define PATH_WRITE_E8E9(value) ObserveCalibrationPathWriteE8E9(value)
#define PATH_RUN_CALIBRATION() ObserveCalibrationPathRunCalibration()
#define PATH_CAPTURE_SAMPLES() ObserveCalibrationPathCaptureSamples()
#define PATH_STROBE(address, value) ObserveCalibrationPathStrobe((address), (value))
#define PATH_SET_CHANNEL(channel, value) ObserveCalibrationPathSetChannel((channel), (value))
#define PATH_WRITE_PORT(address, value) ObserveCalibrationPathWritePort((address), (value))
#define PATH_WRITE_F6(value) ObserveCalibrationPathWriteF6(value)
#define PATH_WAIT_F8() ObserveCalibrationPathWaitF8()
#define PATH_WAIT_TIMER(duration, yield_mode) ObserveCalibrationPathWaitTimer((duration), (yield_mode))
#define PATH_SET_FLAG(enabled) ObserveCalibrationPathSetControllerFlag(enabled)
#define PATH_RESET_CHANNEL(channel) ObserveCalibrationPathResetChannel(channel)
#define PATH_READ_PORT(address) ObserveCalibrationPathReadPort(address)
#define PATH_CONFIGURE_STATE2() ObserveCalibrationPathConfigureState2()
#else
#include "platform/arithmetic.h"
#include "platform/controller_calibration.h"
#include "platform/controller_parameters.h"
#include "platform/controller_sample_series.h"
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearWithoutTimeout(void);
#define PATH_WRITE_E8E9(value) WriteControllerParametersE8E9ForValue(value)
#define PATH_RUN_CALIBRATION() RunControllerParameterCalibrationAndReferenceCapture()
#define PATH_CAPTURE_SAMPLES() CaptureControllerSampleSeries()
#define PATH_STROBE(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define PATH_SET_CHANNEL(channel, value) SetIndexedControllerChannelValue((channel), (value))
#define PATH_WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define PATH_WRITE_F6(value) WriteControllerParameterF6ValueWithBits0And1Set(value)
#define PATH_WAIT_F8() WaitControllerPort80F8Bit7ClearWithoutTimeout()
#define PATH_WAIT_TIMER(duration, yield_mode) WaitTimer1ScaledDuration((duration), (yield_mode))
#define PATH_SET_FLAG(enabled) SetControllerFlagFC07Bit2AndPort8098Bit6(enabled)
#define PATH_RESET_CHANNEL(channel) ResetIndexedControllerChannel(channel)
#define PATH_READ_PORT(address) WaitControllerPortReadyAndReadByte(address)
#define PATH_CONFIGURE_STATE2() ConfigureControllerForParameterState2()
#endif
#endif
