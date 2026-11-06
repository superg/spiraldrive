#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 FirmwareReadBit000E(void);
extern void SelectAndApplyOperatingCodeForMediaStateEnabled(void);
#else
extern u8 FirmwareReadBit000E(void);
extern void SelectAndApplyOperatingCodeForMediaStateEnabled(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePort8097CalibrationWrite(fw_addr_t address, u8 value);
extern void ObservePort8097CalibrationWait(u16 duration_units, u8 yield_while_waiting);
extern u8 ObservePort8097CalibrationRead(fw_addr_t address);
#define WRITE_PORT8097_CALIBRATION(address, value) ObservePort8097CalibrationWrite((address), (value))
#define WAIT_PORT8097_CALIBRATION(duration, yield) ObservePort8097CalibrationWait((duration), (yield))
#define READ_PORT8097_CALIBRATION(address) ObservePort8097CalibrationRead(address)
#define Port8097CalibrationFunction CalibrateControllerPort8097WithCommand08ForTest
#else
#define WRITE_PORT8097_CALIBRATION(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define WAIT_PORT8097_CALIBRATION(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define READ_PORT8097_CALIBRATION(address) WaitControllerPortReadyAndReadByte(address)
#define Port8097CalibrationFunction CalibrateControllerPort8097WithCommand08
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern u8 InterceptMeasureSfrD8RangeAtLeast16ForParameterFBRefresh(void);
extern void ObserveServiceControllerCommand0EMeasurementCapture(void);
extern u8 ObserveReadAcquisitionEightSampleReadiness(u8 sample_count);
extern void ObserveReadAcquisitionEightSampleControllerCode(u8 selector_code);
extern void ObserveReadAcquisitionEightSampleWait(u16 duration_units, u8 yield_while_waiting);
extern u8 ObserveReadAcquisitionEightSamplePulseWindow(void);
extern u8 ObserveReadAcquisitionEightSampleMeasurement(void);
extern void ObserveReadAcquisitionEightSampleRestore(void);
#define DispatchReadAcquisitionEightSampleReadiness(sample_count) ObserveReadAcquisitionEightSampleReadiness(sample_count)
#define DispatchReadAcquisitionEightSampleControllerCode(selector_code) ObserveReadAcquisitionEightSampleControllerCode(selector_code)
#define DispatchReadAcquisitionEightSampleWait(duration, yield) ObserveReadAcquisitionEightSampleWait((duration), (yield))
#define DispatchReadAcquisitionEightSamplePulseWindow() ObserveReadAcquisitionEightSamplePulseWindow()
#define DispatchReadAcquisitionEightSampleMeasurement() ObserveReadAcquisitionEightSampleMeasurement()
#define DispatchReadAcquisitionEightSampleRestore() ObserveReadAcquisitionEightSampleRestore()
#else
#define DispatchReadAcquisitionEightSampleReadiness(sample_count) TestP1Cex0PulseSuccessThreshold(sample_count)
#define DispatchReadAcquisitionEightSampleControllerCode(selector_code) WriteControllerParametersD4AndE2ForCode(selector_code)
#define DispatchReadAcquisitionEightSampleWait(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define DispatchReadAcquisitionEightSamplePulseWindow() CheckP1Cex0PulseWithinTimer1Window()
#define DispatchReadAcquisitionEightSampleMeasurement() MeasureAveragedSfrD8MinusD9WithOffsets()
#define DispatchReadAcquisitionEightSampleRestore() WriteControllerParametersD412AndE2Value58()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWaitControllerPortForDisabledOperatingPath(fw_addr_t port_address, u8 value);
extern void ObserveUpdateControllerWord035FForDisabledOperatingPath(void);
extern void ObserveStrobeControllerPortForDisabledOperatingPath(fw_addr_t port_address, u8 value);
extern void ObserveApplyControllerParameterD1ForDisabledOperatingPath(void);
extern void ObserveConfigureControllerD0D1D3ForDisabledOperatingPath(void);
#define DispatchWaitControllerPortForDisabledOperatingPath(address, value) ObserveWaitControllerPortForDisabledOperatingPath((address), (value))
#define DispatchUpdateControllerWord035FForDisabledOperatingPath() ObserveUpdateControllerWord035FForDisabledOperatingPath()
#define DispatchStrobeControllerPortForDisabledOperatingPath(address, value) ObserveStrobeControllerPortForDisabledOperatingPath((address), (value))
#define DispatchApplyControllerParameterD1ForDisabledOperatingPath() ObserveApplyControllerParameterD1ForDisabledOperatingPath()
#define DispatchConfigureControllerD0D1D3ForDisabledOperatingPath() ObserveConfigureControllerD0D1D3ForDisabledOperatingPath()
#else
extern void UpdateControllerWord035FFromProgressBucket(void);
#define DispatchWaitControllerPortForDisabledOperatingPath(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define DispatchUpdateControllerWord035FForDisabledOperatingPath() UpdateControllerWord035FFromProgressBucket()
#define DispatchStrobeControllerPortForDisabledOperatingPath(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define DispatchApplyControllerParameterD1ForDisabledOperatingPath() ApplyControllerParameterD1ModeBits()
#define DispatchConfigureControllerD0D1D3ForDisabledOperatingPath() ConfigureControllerPorts80D0D1D3ForCurrentState()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
/* Reconstruction helper; no direct Ghidra function address. */
static void SetP0Bit3Atomic(void)
{
    FirmwareWriteDirectSfr8(80, (u8)(FirmwareReadDirectSfr8(80) | 0x08u));
}
/* Reconstruction helper; no direct Ghidra function address. */
static void ClearP0Bit3Atomic(void)
{
    FirmwareWriteDirectSfr8(80, (u8)(FirmwareReadDirectSfr8(80) & 0xf7u));
}
/* Reconstruction helper; no direct Ghidra function address. */
static void ClearBit0547Atomic(void)
{
    FirmwareWriteBit0547(0u);
}
/* Reconstruction helper; no direct Ghidra function address. */
static void SetBit0547Atomic(void)
{
    FirmwareWriteBit0547(1u);
}
/* Reconstruction helper; no direct Ghidra function address. */
static void SetBit0546Atomic(void)
{
    FirmwareWriteBit0546(1u);
}
/* Reconstruction helper; no direct Ghidra function address. */
static void ClearBit0542Atomic(void)
{
    FirmwareWriteBit0542(0u);
}
/* Reconstruction helper; no direct Ghidra function address. */
static void SetP1Bit0Atomic(void)
{
    FirmwareWriteDirectSfr8(90, (u8)(FirmwareReadDirectSfr8(90) | 1u));
}
/* Reconstruction helper; no direct Ghidra function address. */
static void SetP1Bit1Atomic(void)
{
    FirmwareWriteDirectSfr8(90, (u8)(FirmwareReadDirectSfr8(90) | 2u));
}
#else
extern void SetP0Bit3Atomic(void);
extern void ClearP0Bit3Atomic(void);
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0542Atomic(void);
extern void SetP1Bit0Atomic(void);
extern void SetP1Bit1Atomic(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU16ControllerState(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}
#else
#define FirmwareWriteBigEndianU16ControllerState(address, value) (*((volatile u16 huge *)(fw_addr_t)(address)) = (u16)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU32ControllerState(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}
#else
#define FirmwareWriteBigEndianU32ControllerState(address, value) (*((volatile u32 huge *)(fw_addr_t)(address)) = (u32)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 8
#if defined(PLEXTOR_HOST_TEST)
static u16 FirmwareReadBigEndianU16Near(u16 address)
{
    return FirmwareReadNearU16(address);
}
#else
#define FirmwareReadBigEndianU16Near(address) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 9
#if defined(PLEXTOR_HOST_TEST)
ObserveServiceControllerCommand0EMeasurementCapture();
#else
ServiceControllerCommand0EMeasurementCapture();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 10
#if defined(PLEXTOR_HOST_TEST)
range_is_wide = InterceptMeasureSfrD8RangeAtLeast16ForParameterFBRefresh();
#else
range_is_wide = MeasureSfrD8RangeAtLeast16();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 11
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveCaptureInheritedDR0ForControllerStrobe(void);
#define CAPTURE_CONTROLLER_STROBE_ADDRESS() ObserveCaptureInheritedDR0ForControllerStrobe()
#else
extern fw_addr_t CaptureInheritedDR0ForControllerStrobe(void);
#define CAPTURE_CONTROLLER_STROBE_ADDRESS() CaptureInheritedDR0ForControllerStrobe()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 12
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveCaptureInheritedDR0ForControllerF8Wait(void);
#define CAPTURE_CONTROLLER_F8_TIMEOUT() ObserveCaptureInheritedDR0ForControllerF8Wait()
#else
extern s32 CaptureInheritedDR0ForControllerF8Wait(void);
#define CAPTURE_CONTROLLER_F8_TIMEOUT() CaptureInheritedDR0ForControllerF8Wait()
#endif
#endif
