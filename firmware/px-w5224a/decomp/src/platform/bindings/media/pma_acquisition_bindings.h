#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePmaResetTables(void);
extern u8 ObservePmaOperationInTocSet(void);
extern u8 ObservePmaAcquirePosition(void);
extern u8 ObservePmaResetAcquisition(u8 reinitialize_controller);
extern void ObservePmaApplyOperatingCode(void);
extern void ObservePmaArmPacket(void);
extern u8 ObservePmaParsePacket(void);
extern void ObservePmaRearmPacket(void);
extern u8 ObservePmaProcessPacket(void);
extern u8 ObservePmaAccumulatePacket(void);
extern u16 ObservePmaDivideTimer(u16 dividend, u8 divisor);
extern void ObservePmaWaitTimer(u16 quotient);
extern u8 ObservePmaPulseThreshold(u8 limit);
extern u8 ObservePmaStateComplete(void);
extern void ObservePmaFinalizeStart(void);
extern void ObservePmaYield(void);
extern void ObservePmaResetDrive(void);
extern s32 ObservePmaConvertMsf(fw_addr_t tuple_address);
extern void ObservePmaCopyBytes(fw_addr_t source, fw_addr_t destination, u16 byte_count);
#define PMA_RESET_TABLES() ObservePmaResetTables()
#define PMA_OPERATION_IN_TOC_SET() ObservePmaOperationInTocSet()
#define PMA_ACQUIRE_POSITION() ObservePmaAcquirePosition()
#define PMA_RESET_ACQUISITION(value) ObservePmaResetAcquisition(value)
#define PMA_APPLY_OPERATING() ObservePmaApplyOperatingCode()
#define PMA_ARM_PACKET() ObservePmaArmPacket()
#define PMA_PARSE_PACKET() ObservePmaParsePacket()
#define PMA_REARM_PACKET() ObservePmaRearmPacket()
#define PMA_PROCESS_PACKET() ObservePmaProcessPacket()
#define PMA_ACCUMULATE_PACKET() ObservePmaAccumulatePacket()
#define PMA_DIVIDE_TIMER() ObservePmaDivideTimer(0x1733u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44))
#define PMA_WAIT_TIMER(value) ObservePmaWaitTimer(value)
#define PMA_PULSE_THRESHOLD(value) ObservePmaPulseThreshold(value)
#define PMA_STATE_COMPLETE() ObservePmaStateComplete()
#define PMA_FINALIZE_START() ObservePmaFinalizeStart()
#define PMA_YIELD() ObservePmaYield()
#define PMA_RESET_DRIVE() ObservePmaResetDrive()
#define PMA_CONVERT_MSF(address) ObservePmaConvertMsf(address)
#define PMA_COPY_BYTES(source, destination, count) ObservePmaCopyBytes((source), (destination), (count))
#else
extern void ResetAllPmaQRecordTables(void);
extern u8 IsCurrentDriveOperationCodeInTocAcquisitionSet(void);
extern u8 AcquireDiscReadPosition(void);
extern u8 ResetPmaAcquisitionAndOptionallyReinitializeController(u8 reinitialize_controller);
extern void ApplyCurrentOperatingCodeToControllerChannels0Through3(void);
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);
extern void RearmControllerQPacketCaptureWithTimeout(void);
extern u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoPmaState(void);
extern s16 DivideS16Quotient(s16 dividend, s16 divisor);
extern u8 TestP1Cex0PulseSuccessThreshold(u8 limit);
extern u8 PLEXTOR_NEAR_FUNCTION IsPmaQRecordStateComplete(void);
extern void PLEXTOR_NEAR_FUNCTION FinalizePmaStartFramePosition(void);
extern void YieldCoroutine1(void);
extern void ResetDriveControlStateToEight(void);
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1OverflowAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetTimer1RunAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1RunAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadTimer1OverflowAtomic(void);
#define PMA_RESET_TABLES() ResetAllPmaQRecordTables()
#define PMA_OPERATION_IN_TOC_SET() IsCurrentDriveOperationCodeInTocAcquisitionSet()
#define PMA_ACQUIRE_POSITION() AcquireDiscReadPosition()
#define PMA_RESET_ACQUISITION(value) ResetPmaAcquisitionAndOptionallyReinitializeController(value)
#define PMA_APPLY_OPERATING() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define PMA_ARM_PACKET() ArmControllerThreeBytePacketCapture()
#define PMA_PARSE_PACKET() ParseControllerThreeBytePacket()
#define PMA_REARM_PACKET() RearmControllerQPacketCaptureWithTimeout()
#define PMA_PROCESS_PACKET() ProcessCapturedControllerQPacket()
#define PMA_ACCUMULATE_PACKET() AccumulateCurrentQPacketIntoPmaState()
#define PMA_DIVIDE_TIMER() ((u16)DivideS16Quotient(0x1733, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44)))
#define PMA_PULSE_THRESHOLD(value) TestP1Cex0PulseSuccessThreshold(value)
#define PMA_STATE_COMPLETE() IsPmaQRecordStateComplete()
#define PMA_FINALIZE_START() FinalizePmaStartFramePosition()
#define PMA_YIELD() YieldCoroutine1()
#define PMA_RESET_DRIVE() ResetDriveControlStateToEight()
#define PMA_CONVERT_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define PMA_COPY_BYTES(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))

static void WaitPmaTimer(u16 quotient)
{
    u16 reload = (u16)(0xffffu - quotient);

    FirmwareWriteDirectSfr8(8D, (u8)(reload >> 8));
    FirmwareWriteDirectSfr8(8B, (u8)reload);
    ClearTimer1OverflowAtomic();
    SetTimer1RunAtomic();
    while(ReadTimer1OverflowAtomic() == 0u)
        YieldCoroutine1();
    ClearTimer1RunAtomic();
    ClearTimer1OverflowAtomic();
}
#define PMA_WAIT_TIMER(value) WaitPmaTimer(value)
#endif
#endif
