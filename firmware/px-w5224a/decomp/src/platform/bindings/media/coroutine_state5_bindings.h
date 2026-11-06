#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveMediaCoroutineState5Operation(u8 operation, s32 argument);
#define MEDIA_STATE5_CALL(operation, argument) ObserveMediaCoroutineState5Operation((operation), (argument))
#else
#include "media/decoder.h"
#include "media/state5_position_window.h"
#include "platform/controller_measurement_window.h"
#include "platform/controller_operating_interpolation.h"
#include "platform/controller_parameters.h"
#include "platform/leaf_workers.h"
#include "platform/media_state_updates.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine1(void);
u8 bdata MediaCoroutineState5BitByte26 _at_ 0x26;
sbit MediaCoroutineState5Bit0033 = MediaCoroutineState5BitByte26 ^ 3;
#define MEDIA_STATE5_NATIVE_CALL_I(operation, argument) MEDIA_STATE5_NATIVE_CALL_##operation(argument)
#define MEDIA_STATE5_NATIVE_CALL(operation, argument) MEDIA_STATE5_NATIVE_CALL_I(operation, argument)
#define MEDIA_STATE5_CALL(operation, argument) MEDIA_STATE5_NATIVE_CALL(operation, argument)
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_INITIALIZE_CONTROL(argument) InitializeMediaState5ControlState()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_INITIALIZE_CONTROLLER(argument) InitializeMediaState5ControllerPath()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_TEST_INDEXED_STATE(argument) ((s32)IsIndexedDriveStateActive((u8)(argument)))
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_COMPUTE_POSITION_WINDOW(argument) ComputeMediaState5PositionWindowFrames()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_RESET_PORTS_8087_808B(argument) ResetControllerPorts8087Through808B()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_RUN_ACQUISITION(argument) ((s32)RunMediaState5ReadAcquisitionSequence((s32)(argument)))
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_COMPUTE_SFR_DB_WINDOW(argument) ComputeProcessedSfrDbWindowValue()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_UPDATE_SECONDARY_CODE(argument) UpdateSecondaryOperatingCodeForSupportedCurrentCode()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_CONFIGURE_TARGET(argument) ConfigureControllerForReadAcquisitionTarget((s32)(argument))
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_BUILD_ADJACENT_MSF(argument) BuildMediaState5AdjacentPackedBcdMsfTuples()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_SET_INDEXED_FLAGS(argument) SetDriveStateFlagsForIndexedMediaState()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_UPDATE_CONTROLLER_PROFILE(argument) UpdateMediaState5ControllerProfileForCurrentPosition()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_YIELD(argument) YieldCoroutine1()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_APPLY_STATE_F6_PARAMETERS(argument) ApplyStateF6ZeroScaledControllerParameters()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_ARM_PACKET_CAPTURE(argument) ArmControllerThreeBytePacketCapture()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_PARSE_PACKET(argument) ((s32)ParseControllerThreeBytePacket())
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_UPDATE_POSITION_TRACKING(argument) UpdateMediaState5PositionTracking((s32)(argument))
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_SERVICE_POST_TARGET(argument) ServiceMediaState5PostTargetThreshold((s32)(argument))
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_READ_BIT0033(argument) ((s32)(u8)MediaCoroutineState5Bit0033)
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_RESTORE_READ_PATH(argument) RestoreControllerReadPathConfiguration((u8)(argument))
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_RESET_PORTS_80C4_80C5(argument) ResetControllerPorts80C4And80C5State()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_COPY_CURRENT_POSITION(argument) CopyCurrentDecodedFramePositionToReadAcquisitionTarget()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_SERVICE_SECONDARY_CODE(argument) ServiceSecondaryOperatingCodeUpdateTrigger()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_UPDATE_LEAD_OUT_STAGE(argument) UpdateMediaState5LeadOutProgressStage()
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_DISPATCH_INDEXED_STATE(argument) ((s32)DispatchIndexedState())
#define MEDIA_STATE5_NATIVE_CALL_MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES(argument) ServiceMediaState5ControllerAndOperatingUpdates()
#endif
#endif
