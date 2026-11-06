#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCommandProcessorCall(u8 operation, u32 argument);
extern u8 ObserveCommandProcessorResult(u8 operation);
#define PROCESSOR_CALL(operation, argument) ObserveCommandProcessorCall((operation), (argument))
#define PROCESSOR_RESULT(operation) ObserveCommandProcessorResult(operation)
#define PROCESSOR_STARTUP_MODE() PROCESSOR_CALL(1u, 1ul)
#define PROCESSOR_REFRESH_INQUIRY() PROCESSOR_CALL(2u, 0ul)
#define PROCESSOR_BUILD_CRC() PROCESSOR_CALL(3u, 0ul)
#define PROCESSOR_ENABLE_TIMER0() PROCESSOR_CALL(4u, 0ul)
#define PROCESSOR_PREPARE_CONTROLLER() PROCESSOR_CALL(5u, 0ul)
#define PROCESSOR_INITIALIZE_COROUTINE1(entry) PROCESSOR_CALL(6u, entry)
#define PROCESSOR_SET_P3() PROCESSOR_CALL(7u, 0ul)
#define PROCESSOR_YIELD() PROCESSOR_CALL(8u, 0ul)
#define PROCESSOR_PREMIUM_UPDATE() PROCESSOR_CALL(9u, 0ul)
#define PROCESSOR_PRIMARY_EXPIRED() PROCESSOR_CALL(10u, 0ul)
#define PROCESSOR_RESET_WRITE_PIPELINE() PROCESSOR_CALL(11u, 0ul)
#define PROCESSOR_PREPARE_WRITE_PIPELINE() PROCESSOR_CALL(12u, 0ul)
#define PROCESSOR_MEDIA_SCAN() PROCESSOR_CALL(13u, 0ul)
#define PROCESSOR_FINALIZE_WRITE() PROCESSOR_CALL(14u, 0ul)
#define PROCESSOR_CLEAR_COMPLETION() PROCESSOR_CALL(15u, 0ul)
#define PROCESSOR_ATA_DISPATCH() PROCESSOR_RESULT(16u)
#define PROCESSOR_RESTART() PROCESSOR_CALL(17u, 0ul)
#define PROCESSOR_VALIDATE_SCSI() PROCESSOR_RESULT(18u)
#define PROCESSOR_DISPATCH_SCSI() PROCESSOR_CALL(19u, FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE))
#define PROCESSOR_FINALIZE_SCSI_WRITE() PROCESSOR_CALL(20u, 0ul)
#define PROCESSOR_PREPARE_COMPLETION() PROCESSOR_CALL(21u, 0ul)
#define PROCESSOR_PUBLISH_COMPLETION() PROCESSOR_CALL(22u, 0ul)
#else
extern void YieldCoroutine0(void);
extern void RunPremiumDiscFirmwareUpdateCoroutineStep(void);
extern void SetTimer0InterruptEnableAtomic(void);
extern void SetP3Bit0Atomic(void);
extern void ClearP3Bit1Atomic(void);
#define PROCESSOR_STARTUP_MODE() ConfigureControllerStartupModeRegisters(1u)
#define PROCESSOR_REFRESH_INQUIRY() RefreshInquiryResponseFromDriveConfiguration()
#define PROCESSOR_BUILD_CRC() BuildCrc16TablePolynomial1021()
#define PROCESSOR_ENABLE_TIMER0() SetTimer0InterruptEnableAtomic()
#define PROCESSOR_PREPARE_CONTROLLER() PrepareControllerForScsiCommandProcessing()
#define PROCESSOR_INITIALIZE_COROUTINE1(entry) InitializeCoroutine1(entry)
#define PROCESSOR_SET_P3()                                                                                                                                                                             \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        SetP3Bit0Atomic();                                                                                                                                                                             \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define PROCESSOR_YIELD() YieldCoroutine0()
#define PROCESSOR_PREMIUM_UPDATE() RunPremiumDiscFirmwareUpdateCoroutineStep()
#define PROCESSOR_PRIMARY_EXPIRED() HandleExpiredFormatUnitCode11PrimaryCountdown()
#define PROCESSOR_RESET_WRITE_PIPELINE() ResetRawReadQueueAndPrepareWritePipelineState()
#define PROCESSOR_PREPARE_WRITE_PIPELINE() PrepareMediaWritePipelineState()
#define PROCESSOR_MEDIA_SCAN() RunFormatUnitCode11MediaScan()
#define PROCESSOR_FINALIZE_WRITE() ((void)FinalizeActiveWritePipelineAndTrackState())
#define PROCESSOR_CLEAR_COMPLETION() ClearScsiCompletionBits0And1IfBit4Set()
#define PROCESSOR_ATA_DISPATCH() HandleAtaTaskFileCommand()
#define PROCESSOR_RESTART() RestartFirmware()
#define PROCESSOR_VALIDATE_SCSI() ValidateScsiCommandAdmission()
#define PROCESSOR_DISPATCH_SCSI() DispatchCurrentScsiOpcode()
#define PROCESSOR_FINALIZE_SCSI_WRITE() FinalizeScsiWriteCommandDispatchState()
#define PROCESSOR_PREPARE_COMPLETION() PrepareAtaCompletionStatusFromSense()
#define PROCESSOR_PUBLISH_COMPLETION() PublishAtaCommandCompletion()
#endif
#endif
