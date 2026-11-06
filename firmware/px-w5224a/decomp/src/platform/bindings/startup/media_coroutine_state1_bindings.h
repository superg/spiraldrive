#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveMediaState1Operation(u8 operation, s32 argument);
extern void ObserveMediaState1WriteP3(u8 bit0, u8 bit1);
#define MEDIA_STATE1_CALL(operation, argument) ObserveMediaState1Operation((operation), (argument))
#define MEDIA_STATE1_WRITE_P3(bit0, bit1) ObserveMediaState1WriteP3((bit0), (bit1))
#else
#include "platform/controller_parameter_f4_calibration.h"
#include "platform/controller_parameters.h"
#include "platform/controller_q_packet_process.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern u8 AcquireDiscReadPosition(void);
extern void RestartCoroutine1AndYield(u32 entry_point);
extern void YieldCoroutine1(void);
sbit MediaState1P3Bit0 = 0xb0 ^ 0;
sbit MediaState1P3Bit1 = 0xb0 ^ 1;
#define MEDIA_STATE1_NATIVE_CALL_I(operation, argument) MEDIA_STATE1_NATIVE_CALL_##operation(argument)
#define MEDIA_STATE1_NATIVE_CALL(operation, argument) MEDIA_STATE1_NATIVE_CALL_I(operation, argument)
#define MEDIA_STATE1_CALL(operation, argument) MEDIA_STATE1_NATIVE_CALL(operation, argument)
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_UPDATE_MEASUREMENT(argument) UpdateReadAcquisitionMeasurementIfNeeded()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_SET_OPERATING_ENABLE(argument) SetControllerOperatingEnableState((u8)(argument))
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_RESET_PORTS(argument) ResetControllerPorts8087Through808B()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_SELECT_OPERATING_CODE(argument) SelectRequestedOperatingCodeForMediaState()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_ACQUIRE_POSITION(argument) ((s32)AcquireDiscReadPosition())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_HANDLE_ACQUISITION_STATUS(argument) ((s32)HandleDiscReadPositionAcquisitionStatus((s8)(argument)))
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_CONFIGURE_PORTS_8085_8086(argument) ConfigureControllerPorts8085And8086Value80()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_APPLY_OPERATING_CHANNELS(argument) ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_BUILD_PORT_SHADOWS(argument) BuildControllerPort8087And808BShadowsFromConfiguration()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_APPLY_PORT_SHADOWS(argument) ApplyControllerPortShadowsForModeFlag0E()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_SET_P1_BIT4(argument) SetP1Bit4()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_REARM_Q_CAPTURE(argument) RearmControllerQPacketCaptureWithTimeout()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_PROCESS_Q_PACKET(argument) ((s32)ProcessCapturedControllerQPacket())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_YIELD(argument) YieldCoroutine1()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_SERVICE_CALIBRATION(argument) ((s32)ServiceMediaState1ControllerCalibrationAttempt())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_RESTART_COROUTINE(argument) RestartCoroutine1AndYield((u32)(argument))
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_PROCESS_DRIVE_FLAGS(argument) ((s32)ProcessDriveStateFlags())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_TEST_INDEXED_STATE(argument) ((s32)IsIndexedDriveStateActive((u8)(argument)))
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_CONFIGURE_PORTS_8079_809D_809F(argument) ConfigureControllerPorts8079And809DAnd809F()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_SELECT_NARROW_TARGET(argument) ((s32)SelectReadAcquisitionTargetForDirectionBitClear(0u))
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_SET_TARGET_FROM_Q(argument) SetReadAcquisitionTargetFromCurrentQPosition()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_DISPATCH_INDEXED_STATE(argument) ((s32)DispatchIndexedState())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_CALIBRATE_PORT_8097(argument) ((s32)CalibrateControllerPort8097ForState1AfterFrame45750())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_CALIBRATE_PARAMETER_F4(argument) CalibrateControllerParameterF4()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_INITIALIZE_CALIBRATION(argument) ((s32)InitializeControllerCalibrationForState1AfterFrame3928())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_APPLY_PARSED_Q_PORT_STATE(argument) ApplyControllerPortStateForParsedQStatus()
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_EXECUTE_DIRECTION_MOVE(argument) ((s32)ExecuteReadAcquisitionDirectionSelectedMove())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_SERVICE_TARGET_WINDOW(argument) ((s32)ServiceMediaState1ReadTargetWindow())
#define MEDIA_STATE1_NATIVE_CALL_MEDIA_STATE1_PROCESS_INDEXED_COMPLETION(argument) ((s32)ProcessIndexedStateCompletion())
#define MEDIA_STATE1_WRITE_P3(bit0, bit1)                                                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        MediaState1P3Bit0 = (bit0);                                                                                                                                                                    \
        MediaState1P3Bit1 = (bit1);                                                                                                                                                                    \
    } while(0)
#endif
#endif
