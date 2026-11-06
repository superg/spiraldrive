#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveExecuteControllerDataInTransfer(u32 buffer_address, u32 transfer_length);
#define DispatchExecuteControllerDataInTransfer(address, length) ObserveExecuteControllerDataInTransfer((address), (length))
#else
extern void ExecuteControllerDataInTransfer(u32 buffer_address, u32 transfer_length);
#define DispatchExecuteControllerDataInTransfer(address, length) ExecuteControllerDataInTransfer((address), (length))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
extern u8 ReadBit000DForVendorD8(void);
extern void ClearVendorD8RawReadModeBitsAtomic(void);
extern void SetBit0046ForReadCommandAtomic(void);
extern void ClearBit0046ForReadCommandAtomic(void);
extern void SetBit0023Atomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION PollVendorDfSubchannelTransfer(void);
extern void YieldCoroutine0(void);

#define ValidateVendorDfCdb(mask_descriptor) ValidateScsiCdbFieldMasks(mask_descriptor)
#define ReadVendorDfAudioStatus(acknowledge_terminal_status) GetScsiAudioStatusCode(acknowledge_terminal_status)
#define PollVendorDfTransfer() PollVendorDfSubchannelTransfer()
#define YieldVendorDfCoroutine0() YieldCoroutine0()
#define ResetVendorDfRawReadState() ResetRawReadQueueAndTransferState()
#define InitializeVendorDfCoroutine2(entry_point) InitializeCoroutine2(entry_point)
#define SetVendorDfSubchannelTransferBit() SetBit0023Atomic()
#define SetVendorF6SenseFromStatus(status_code) SetSenseDataFromStatusCode(status_code)
#define SetVendorF6Sense(sense_key, asc_ascq) SetSenseData((sense_key), (asc_ascq))
#define SetVendorF6SequenceError() SetCommandSequenceErrorSense()
#define RejectVendorF6Command() HandleUnsupportedScsiCommand()
#define CompleteVendorF6Command() CompleteScsiCommandWithNoSense()
#define ConvertVendorF6Msf(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define ResetVendorF6RawReadState() ResetRawReadQueueAndTransferState()
#define InitializeVendorF6Coroutine2(entry_point) InitializeCoroutine2(entry_point)
#define YieldVendorF6Coroutine0() YieldCoroutine0()
#define ExecuteVendorF6Acquisition() ExecuteVendorF6ReadAcquisitionAndReportStatus()
#define ValidateVendorF2Media() ValidateRecordableMediaCommandEligibility()
#define ClearVendorF2MediaFlags() ClearMediaStateFlags7Through12()
#define NormalizeVendorF2WriteSpeed() NormalizeWriteSpeedForCurrentMedia()
#define ResetVendorF2RawReadState() ResetRawReadQueueAndTransferState()
#define InitializeVendorF2Coroutine2(entry_point) InitializeCoroutine2(entry_point)
#define YieldVendorF2Coroutine0() YieldCoroutine0()
#define RunVendorF2Opc() RunOpcForCurrentWriteSpeed()
#define BuildVendorF2FullResponse() BuildVendorF2ControllerMeasurementResponse()
#define BuildVendorF2ShortResponse() BuildVendorF2ShortControllerMeasurementResponse()
#define TransferVendorF2Direct(byte_count) ExecuteControllerDataInAndMapStatus(byte_count)
#define TransferVendorF2Buffer(source, byte_count) CopyResponseToControllerBufferAndExecuteDataIn((source), (byte_count))
#define SetVendorF2NoSense() SetNoSenseData()
#else
extern u8 FirmwareReadBit000D(void);
extern void FirmwareWriteBit000D(u8 value);
extern void FirmwareWriteBit000E(u8 value);
extern void FirmwareWriteBit0013ForVendorD8(u8 value);
extern void FirmwareWriteBit0018(u8 value);
extern void FirmwareWriteBit0046(u8 value);
extern u8 DispatchValidateVendorDfCdb(fw_addr_t mask_descriptor);
extern u8 DispatchReadVendorDfAudioStatus(u8 acknowledge_terminal_status);
extern u8 DispatchPollVendorDfSubchannelTransfer(void);
extern void DispatchYieldVendorDfCoroutine0(void);
extern void DispatchResetVendorDfRawReadState(void);
extern void DispatchInitializeVendorDfCoroutine2(u32 entry_point);
extern void DispatchSetVendorF6SenseFromStatus(u8 status_code);
extern void DispatchSetVendorF6Sense(u8 sense_key, u16 asc_ascq);
extern void DispatchSetVendorF6SequenceError(void);
extern void DispatchRejectVendorF6Command(void);
extern void DispatchCompleteVendorF6Command(void);
extern s32 DispatchConvertVendorF6Msf(fw_addr_t address);
extern void DispatchResetVendorF6RawReadState(void);
extern void DispatchInitializeVendorF6Coroutine2(u32 entry_point);
extern void DispatchYieldVendorF6Coroutine0(void);
extern void DispatchExecuteVendorF6Acquisition(void);
extern u8 DispatchValidateVendorF2Media(void);
extern void DispatchClearVendorF2MediaFlags(void);
extern void DispatchNormalizeVendorF2WriteSpeed(void);
extern void DispatchResetVendorF2RawReadState(void);
extern void DispatchInitializeVendorF2Coroutine2(u32 entry_point);
extern void DispatchYieldVendorF2Coroutine0(void);
extern u8 DispatchRunVendorF2Opc(void);
extern void DispatchBuildVendorF2FullResponse(void);
extern void DispatchBuildVendorF2ShortResponse(void);
extern u8 DispatchTransferVendorF2Direct(u32 byte_count);
extern u8 DispatchTransferVendorF2Buffer(fw_addr_t source_address, u32 byte_count);
extern void DispatchSetVendorF2NoSense(void);

#define ValidateVendorDfCdb(mask_descriptor) DispatchValidateVendorDfCdb(mask_descriptor)
#define ReadVendorDfAudioStatus(acknowledge_terminal_status) DispatchReadVendorDfAudioStatus(acknowledge_terminal_status)
#define PollVendorDfTransfer() DispatchPollVendorDfSubchannelTransfer()
#define YieldVendorDfCoroutine0() DispatchYieldVendorDfCoroutine0()
#define ResetVendorDfRawReadState() DispatchResetVendorDfRawReadState()
#define InitializeVendorDfCoroutine2(entry_point) DispatchInitializeVendorDfCoroutine2(entry_point)
#define SetVendorDfSubchannelTransferBit() FirmwareWriteBit0023(1u)
#define SetVendorF6SenseFromStatus(status_code) DispatchSetVendorF6SenseFromStatus(status_code)
#define SetVendorF6Sense(sense_key, asc_ascq) DispatchSetVendorF6Sense((sense_key), (asc_ascq))
#define SetVendorF6SequenceError() DispatchSetVendorF6SequenceError()
#define RejectVendorF6Command() DispatchRejectVendorF6Command()
#define CompleteVendorF6Command() DispatchCompleteVendorF6Command()
#define ConvertVendorF6Msf(address) DispatchConvertVendorF6Msf(address)
#define ResetVendorF6RawReadState() DispatchResetVendorF6RawReadState()
#define InitializeVendorF6Coroutine2(entry_point) DispatchInitializeVendorF6Coroutine2(entry_point)
#define YieldVendorF6Coroutine0() DispatchYieldVendorF6Coroutine0()
#define ExecuteVendorF6Acquisition() DispatchExecuteVendorF6Acquisition()
#define ValidateVendorF2Media() DispatchValidateVendorF2Media()
#define ClearVendorF2MediaFlags() DispatchClearVendorF2MediaFlags()
#define NormalizeVendorF2WriteSpeed() DispatchNormalizeVendorF2WriteSpeed()
#define ResetVendorF2RawReadState() DispatchResetVendorF2RawReadState()
#define InitializeVendorF2Coroutine2(entry_point) DispatchInitializeVendorF2Coroutine2(entry_point)
#define YieldVendorF2Coroutine0() DispatchYieldVendorF2Coroutine0()
#define RunVendorF2Opc() DispatchRunVendorF2Opc()
#define BuildVendorF2FullResponse() DispatchBuildVendorF2FullResponse()
#define BuildVendorF2ShortResponse() DispatchBuildVendorF2ShortResponse()
#define TransferVendorF2Direct(byte_count) DispatchTransferVendorF2Direct(byte_count)
#define TransferVendorF2Buffer(source, byte_count) DispatchTransferVendorF2Buffer((source), (byte_count))
#define SetVendorF2NoSense() DispatchSetVendorF2NoSense()

static u8 ReadBit000DForVendorD8(void)
{
    return FirmwareReadBit000D();
}

static void ClearVendorD8RawReadModeBitsAtomic(void)
{
    FirmwareWriteBit000D(0u);
    FirmwareWriteBit0013ForVendorD8(0u);
    FirmwareWriteBit000E(0u);
    FirmwareWriteBit0018(0u);
}

static void SetBit0046ForReadCommandAtomic(void)
{
    FirmwareWriteBit0046(1u);
}

static void ClearBit0046ForReadCommandAtomic(void)
{
    FirmwareWriteBit0046(0u);
}
#endif
#endif
