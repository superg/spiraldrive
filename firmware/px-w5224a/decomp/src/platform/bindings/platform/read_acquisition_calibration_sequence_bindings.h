#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveReadAcquisitionCalibration(u8 event, u32 argument0, u32 argument1);

#define CALIBRATION_EVENT(event, argument0, argument1) ObserveReadAcquisitionCalibration((event), (argument0), (argument1))
#define CALIBRATION_SET_ENABLE(value) ((void)CALIBRATION_EVENT(READ_CALIBRATION_SET_OPERATING_ENABLE, (value), 0ul))
#define CALIBRATION_ACQUIRE_POSITION() ((u8)CALIBRATION_EVENT(READ_CALIBRATION_ACQUIRE_POSITION, 0ul, 0ul))
#define CALIBRATION_RESET_DRIVE_CONTROL() ((void)CALIBRATION_EVENT(READ_CALIBRATION_RESET_DRIVE_CONTROL, 0ul, 0ul))
#define CALIBRATION_INITIALIZE_CONTROLLER() ((u8)CALIBRATION_EVENT(READ_CALIBRATION_INITIALIZE_CONTROLLER, 0ul, 0ul))
#define CALIBRATION_CHECK_TOLERANCE(value) ((u8)CALIBRATION_EVENT(READ_CALIBRATION_CHECK_TOLERANCE, (value), 0ul))
#define CALIBRATION_APPLY_OPERATING_CODE() ((void)CALIBRATION_EVENT(READ_CALIBRATION_APPLY_OPERATING_CODE, 0ul, 0ul))
#define CALIBRATION_WAIT_FOR_TARGET(value) ((u8)CALIBRATION_EVENT(READ_CALIBRATION_WAIT_FOR_TARGET, (u32)(value), 0ul))
#define CALIBRATION_CONFIGURE_PORTS(value) ((void)CALIBRATION_EVENT(READ_CALIBRATION_CONFIGURE_PORTS, (value), 0ul))
#define CALIBRATION_CONFIGURE_MODE1() ((void)CALIBRATION_EVENT(READ_CALIBRATION_CONFIGURE_MODE1, 0ul, 0ul))
#define CALIBRATION_RESET_DATA_PATH() ((void)CALIBRATION_EVENT(READ_CALIBRATION_RESET_DATA_PATH, 0ul, 0ul))
#define CALIBRATION_CONFIGURE_PROFILE() ((void)CALIBRATION_EVENT(READ_CALIBRATION_CONFIGURE_PROFILE, 0ul, 0ul))
#define CALIBRATION_STROBE(address, value) ((void)CALIBRATION_EVENT(READ_CALIBRATION_STROBE_PORT, (address), (value)))
#define CALIBRATION_PROBE_READINESS() ((u8)CALIBRATION_EVENT(READ_CALIBRATION_PROBE_READINESS, 0ul, 0ul))
#define CALIBRATION_WAIT_RANGE_START() ((u8)CALIBRATION_EVENT(READ_CALIBRATION_WAIT_RANGE_START, 0ul, 0ul))
#define CALIBRATION_ACQUIRE_RANGE() ((u8)CALIBRATION_EVENT(READ_CALIBRATION_ACQUIRE_RANGE, 0ul, 0ul))
#define CALIBRATION_RESTORE(value) ((void)CALIBRATION_EVENT(READ_CALIBRATION_RESTORE_CONFIGURATION, (value), 0ul))
#define CALIBRATION_MULTIPLY(a, b) ((u32)(a) * (u32)(b))

static void SetCalibrationP3Bit(u8 mask, u8 value)
{
    u8 port_value = FirmwareReadDirectSfr8(B0);

    if(value != 0u)
        port_value = (u8)(port_value | mask);
    else
        port_value = (u8)(port_value & (u8)~mask);
    FirmwareWriteDirectSfr8(B0, port_value);
}

#define CALIBRATION_SET_P3_RX(value) SetCalibrationP3Bit(0x01u, (value))
#define CALIBRATION_SET_P3_TX(value) SetCalibrationP3Bit(0x02u, (value))
#else
extern u8 AcquireDiscReadPosition(void);
extern u8 InitializeControllerOperatingStateAndRunCalibration(void);
extern void ConfigureControllerRegisterProfileForCurrentMediaState(void);

sbit ReadCalibrationP3Rx = 0xb0;
sbit ReadCalibrationP3Tx = 0xb1;

#define CALIBRATION_SET_ENABLE(value) SetControllerOperatingEnableState(value)
#define CALIBRATION_ACQUIRE_POSITION() AcquireDiscReadPosition()
#define CALIBRATION_RESET_DRIVE_CONTROL() ResetDriveControlStateToEight()
#define CALIBRATION_INITIALIZE_CONTROLLER() InitializeControllerOperatingStateAndRunCalibration()
#define CALIBRATION_CHECK_TOLERANCE(value) CheckControllerMeasurementToleranceOrTimeout(value)
#define CALIBRATION_APPLY_OPERATING_CODE() ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define CALIBRATION_WAIT_FOR_TARGET(value) WaitForDecodedFramePositionPastTarget(value)
#define CALIBRATION_CONFIGURE_PORTS(value) ConfigureControllerPortsAndParametersForCurrentOperatingCode(value)
#define CALIBRATION_CONFIGURE_MODE1() ConfigureReadAcquisitionControllerForOperatingCode()
#define CALIBRATION_RESET_DATA_PATH() ResetControllerDataPathRegisters()
#define CALIBRATION_CONFIGURE_PROFILE() ConfigureControllerRegisterProfileForCurrentMediaState()
#define CALIBRATION_STROBE(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define CALIBRATION_PROBE_READINESS() RunControllerReadPathReadinessProbe()
#define CALIBRATION_WAIT_RANGE_START() WaitForReadMeasurementRangeStart()
#define CALIBRATION_ACQUIRE_RANGE() AcquireConfiguredReadMeasurementRange()
#define CALIBRATION_RESTORE(value) RestoreControllerReadPathConfiguration(value)
#define CALIBRATION_MULTIPLY(a, b) MultiplyU32Low((a), (b))
#define CALIBRATION_SET_P3_RX(value) (ReadCalibrationP3Rx = (value))
#define CALIBRATION_SET_P3_TX(value) (ReadCalibrationP3Tx = (value))
#endif
#endif
