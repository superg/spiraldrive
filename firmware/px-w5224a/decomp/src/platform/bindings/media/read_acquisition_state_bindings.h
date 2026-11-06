#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveReadAcquisitionStateStep(u8 step);
extern u16 ObserveReadAcquisitionToleranceFrames(void);
extern void ObserveReadAcquisitionOperatingEnable(u8 enabled);
extern s8 ObserveReadAcquisitionDiscPosition(void);
extern u8 ObserveReadAcquisitionTolerance(u8 percent, u8 lower_only);
extern u8 ObserveReadAcquisitionQStatus(void);
extern u8 ObserveReadAcquisitionThreeByteStatus(void);
extern u8 ObserveReadAcquisitionPulse(void);
extern void ObserveReadAcquisitionSpeedReduction(u8 force_minimum);
extern u8 ObserveReadAcquisitionCalibration(void);
extern u8 ObserveReadAcquisitionBit(u8 bit_address);
#define STATE_STEP(step) ObserveReadAcquisitionStateStep(step)
#define UPDATE_MEASUREMENT() STATE_STEP(READ_ACQUISITION_UPDATE_MEASUREMENT)
#define RESET_EIGHT_CHANNEL() STATE_STEP(READ_ACQUISITION_RESET_EIGHT_CHANNEL)
#define ADJUST_TARGET() STATE_STEP(READ_ACQUISITION_ADJUST_TARGET)
#define COMPUTE_TOLERANCE() ObserveReadAcquisitionToleranceFrames()
#define SET_OPERATING_ENABLE(value) ObserveReadAcquisitionOperatingEnable(value)
#define SELECT_OPERATING_CODE() STATE_STEP(READ_ACQUISITION_SELECT_OPERATING_CODE)
#define ACQUIRE_POSITION() ObserveReadAcquisitionDiscPosition()
#define TEST_TOLERANCE(percent, lower) ObserveReadAcquisitionTolerance((percent), (lower))
#define YIELD_STATE() STATE_STEP(READ_ACQUISITION_YIELD)
#define CONFIGURE_ALTERNATE() STATE_STEP(READ_ACQUISITION_CONFIGURE_ALTERNATE)
#define CONFIGURE_NORMAL() STATE_STEP(READ_ACQUISITION_CONFIGURE_NORMAL)
#define APPLY_OPERATING_CODE() STATE_STEP(READ_ACQUISITION_APPLY_OPERATING_CODE)
#define RESET_PORTS() STATE_STEP(READ_ACQUISITION_RESET_PORTS)
#define APPLY_CONDITIONAL() STATE_STEP(READ_ACQUISITION_APPLY_CONDITIONAL)
#define PREPARE_COMMAND0E() STATE_STEP(READ_ACQUISITION_PREPARE_COMMAND0E)
#define CLEAR_RAW_PORTS() STATE_STEP(READ_ACQUISITION_CLEAR_RAW_PORTS)
#define REARM_Q() STATE_STEP(READ_ACQUISITION_REARM_Q)
#define PROCESS_Q() ObserveReadAcquisitionQStatus()
#define ARM_THREE_BYTE() STATE_STEP(READ_ACQUISITION_ARM_THREE_BYTE)
#define PARSE_THREE_BYTE() ObserveReadAcquisitionThreeByteStatus()
#define TEST_PULSE() ObserveReadAcquisitionPulse()
#define COUNT_MATCH() STATE_STEP(READ_ACQUISITION_COUNT_MATCH)
#define SERVICE_EIGHT_CHANNEL() STATE_STEP(READ_ACQUISITION_SERVICE_EIGHT_CHANNEL)
#define SERVICE_COMMAND0E() STATE_STEP(READ_ACQUISITION_SERVICE_COMMAND0E)
#define FINALIZE_EIGHT_CHANNEL() STATE_STEP(READ_ACQUISITION_FINALIZE_EIGHT_CHANNEL)
#define SET_TARGET_FROM_Q() STATE_STEP(READ_ACQUISITION_SET_TARGET)
#define REDUCE_SPEED(force) ObserveReadAcquisitionSpeedReduction(force)
#define RESET_DRIVE_CONTROL() STATE_STEP(READ_ACQUISITION_RESET_DRIVE_CONTROL)
#define INITIALIZE_CALIBRATION() ObserveReadAcquisitionCalibration()
#define READ_STATE_BIT(address) ObserveReadAcquisitionBit(address)
#else
extern void YieldCoroutine1(void);
u8 bdata ReadAcquisitionBitByte20 _at_ 0x20;
u8 bdata ReadAcquisitionBitByte21 _at_ 0x21;
u8 bdata ReadAcquisitionBitByte22 _at_ 0x22;
u8 bdata ReadAcquisitionBitByte23 _at_ 0x23;
sbit ReadAcquisitionBit0002 = ReadAcquisitionBitByte20 ^ 2;
sbit ReadAcquisitionBit000E = ReadAcquisitionBitByte21 ^ 6;
sbit ReadAcquisitionBit0013 = ReadAcquisitionBitByte22 ^ 3;
sbit ReadAcquisitionBit001C = ReadAcquisitionBitByte23 ^ 4;
sbit ReadAcquisitionBit001D = ReadAcquisitionBitByte23 ^ 5;
#define UPDATE_MEASUREMENT() UpdateReadAcquisitionMeasurementIfNeeded()
#define RESET_EIGHT_CHANNEL() ResetEightChannelMeasurementCaptureState()
#define ADJUST_TARGET() AdjustReadAcquisitionTargetForModes()
#define COMPUTE_TOLERANCE() ComputeReadAcquisitionDynamicToleranceFrames()
#define SET_OPERATING_ENABLE(value) SetControllerOperatingEnableState(value)
#define SELECT_OPERATING_CODE() SelectRequestedOperatingCodeForMediaState()
#define ACQUIRE_POSITION() ((s8)AcquireDiscReadPosition())
#define TEST_TOLERANCE(percent, lower) IsControllerPort80A8A9WithinScaledTolerance((percent), (lower))
#define YIELD_STATE() YieldCoroutine1()
#define CONFIGURE_ALTERNATE() ConfigureControllerPort8085Value80And8086Value00()
#define CONFIGURE_NORMAL() ConfigureControllerPortsForDisabledOperatingPath()
#define APPLY_OPERATING_CODE() ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define RESET_PORTS() ResetControllerPort8087And808A808BUnlessFlag0E()
#define APPLY_CONDITIONAL() ApplyConditionalReadPathPortConfiguration()
#define PREPARE_COMMAND0E() PrepareControllerCommand0EMeasurementCapture()
#define CLEAR_RAW_PORTS() ClearControllerPorts80F8And80FAIfRawReadEnabled()
#define REARM_Q() RearmControllerQPacketCaptureWithTimeout()
#define PROCESS_Q() ProcessCapturedControllerQPacket()
#define ARM_THREE_BYTE() ArmControllerThreeBytePacketCapture()
#define PARSE_THREE_BYTE() ParseControllerThreeBytePacket()
#define TEST_PULSE() CheckP1Cex0PulseWithinTimer1Window()
#define COUNT_MATCH() CountRawReadTargetPositionMatch()
#define SERVICE_EIGHT_CHANNEL() ServiceEightChannelMeasurementCapture()
#define SERVICE_COMMAND0E() ServiceControllerCommand0EMeasurementCapture()
#define FINALIZE_EIGHT_CHANNEL() FinalizeEightChannelMeasurementCapture()
#define SET_TARGET_FROM_Q() SetReadAcquisitionTargetFromCurrentQPosition()
#define REDUCE_SPEED(force) ReduceReadSpeedForAcquisitionRecovery(force)
#define RESET_DRIVE_CONTROL() ResetDriveControlStateToEight()
#define INITIALIZE_CALIBRATION() InitializeControllerOperatingStateAndRunCalibration()
#define READ_STATE_BIT(address)                                                                                                                                                                        \
    ((address) == 0x1cu      ? (u8)ReadAcquisitionBit001C                                                                                                                                              \
        : (address) == 0x1du ? (u8)ReadAcquisitionBit001D                                                                                                                                              \
        : (address) == 0x13u ? (u8)ReadAcquisitionBit0013                                                                                                                                              \
        : (address) == 0x0eu ? (u8)ReadAcquisitionBit000E                                                                                                                                              \
                             : (u8)ReadAcquisitionBit0002)
#endif
#endif
