#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCurrentModeWriteBit41(u8 value);
extern void ObserveCurrentModeWriteBit42(u8 value);
extern void ObserveCurrentModeWriteBit30(u8 value);
extern void ObserveCurrentModeInitializeSectorControl(void);
extern void ObserveCurrentModeInitializeSubchannelIdentification(void);
extern void ObserveCurrentModeInitializeTrackSelection(void);
extern void ObserveCurrentModeClearPipelineStatus(void);
extern void ObserveCurrentModeInitializeRandomBytes(void);
extern void ObserveCurrentModeUpdateQTypeState(void);
extern u8 ObserveCurrentModeIsIsrcAvailable(void);
extern void ObserveCurrentModeLoadIsrc(void);
extern u8 ObserveCurrentModePreparePlan(u8 alternate);
extern u8 ObserveCurrentModePoll(u8 count);
extern void ObserveCurrentModeReportFailure(u8 code);
extern void ObserveCurrentModeReportQueueSense(void);
extern void ObserveCurrentModeConfigureTransferLayout(void);
extern u8 ObserveCurrentModeIsMediaCatalogAvailable(void);
extern void ObserveCurrentModeLoadMediaCatalog(void);
extern void ObserveCurrentModeInitializeTrackRecords(void);
extern u8 ObserveCurrentModeAcquireLeadIn(void);
extern void ObserveCurrentModeAdvanceTrackRecord(void);
extern void ObserveCurrentModeRefreshIsrc(void);
extern void ObserveCurrentModeCopy(fw_addr_t source, fw_addr_t destination, u16 count);
extern void ObserveCurrentModeDecrementMsf(u16 address);
extern fw_addr_t ObserveCurrentModeResumeRing(void);
extern u8 ObserveCurrentModeAcquireFrames(u32 count, u8 reverse);
extern u32 ObserveCurrentModeSelectBuffer(u32 length, u32 fallback);
extern void ObserveCurrentModeFill(fw_addr_t address, u32 count, u8 value);
#define CURRENT_MODE_WRITE_BIT41(v) ObserveCurrentModeWriteBit41(v)
#define CURRENT_MODE_WRITE_BIT42(v) ObserveCurrentModeWriteBit42(v)
#define CURRENT_MODE_WRITE_BIT30(v) ObserveCurrentModeWriteBit30(v)
#define CURRENT_MODE_INIT_CONTROL() ObserveCurrentModeInitializeSectorControl()
#define CURRENT_MODE_INIT_IDENTIFICATION() ObserveCurrentModeInitializeSubchannelIdentification()
#define CURRENT_MODE_INIT_TRACKS() ObserveCurrentModeInitializeTrackSelection()
#define CURRENT_MODE_CLEAR_STATUS() ObserveCurrentModeClearPipelineStatus()
#define CURRENT_MODE_INIT_RANDOM() ObserveCurrentModeInitializeRandomBytes()
#define CURRENT_MODE_UPDATE_Q_TYPE() ObserveCurrentModeUpdateQTypeState()
#define CURRENT_MODE_IS_ISRC_AVAILABLE() ObserveCurrentModeIsIsrcAvailable()
#define CURRENT_MODE_LOAD_ISRC() ObserveCurrentModeLoadIsrc()
#define CURRENT_MODE_PREPARE(a) ObserveCurrentModePreparePlan(a)
#define CURRENT_MODE_POLL(c) ObserveCurrentModePoll(c)
#define CURRENT_MODE_FAILURE(c) ObserveCurrentModeReportFailure(c)
#define CURRENT_MODE_QUEUE_SENSE() ObserveCurrentModeReportQueueSense()
#define CURRENT_MODE_CONFIGURE_LAYOUT() ObserveCurrentModeConfigureTransferLayout()
#define CURRENT_MODE_IS_MCN_AVAILABLE() ObserveCurrentModeIsMediaCatalogAvailable()
#define CURRENT_MODE_LOAD_MCN() ObserveCurrentModeLoadMediaCatalog()
#define CURRENT_MODE_INIT_TRACK_RECORDS() ObserveCurrentModeInitializeTrackRecords()
#define CURRENT_MODE_ACQUIRE_LEAD_IN() ObserveCurrentModeAcquireLeadIn()
#define CURRENT_MODE_ADVANCE_TRACK() ObserveCurrentModeAdvanceTrackRecord()
#define CURRENT_MODE_REFRESH_ISRC() ObserveCurrentModeRefreshIsrc()
#define CURRENT_MODE_COPY(s, d, c) ObserveCurrentModeCopy((s), (d), (c))
#define CURRENT_MODE_DECREMENT_MSF(a) ObserveCurrentModeDecrementMsf(a)
#define CURRENT_MODE_RESUME_RING() ObserveCurrentModeResumeRing()
#define CURRENT_MODE_ACQUIRE_FRAMES(c, r) ObserveCurrentModeAcquireFrames((c), (r))
#define CURRENT_MODE_SELECT_BUFFER(l, f) ObserveCurrentModeSelectBuffer((l), (f))
#define CURRENT_MODE_FILL(a, c, v) ObserveCurrentModeFill((a), (c), (v))
#else
u8 bdata CurrentModeRawReadBitByte26 _at_ 0x26;
u8 bdata CurrentModeRawReadBitByte28 _at_ 0x28;
sbit CurrentModeRawReadBit0030 = CurrentModeRawReadBitByte26 ^ 0;
sbit CurrentModeRawReadBit0041 = CurrentModeRawReadBitByte28 ^ 1;
sbit CurrentModeRawReadBit0042 = CurrentModeRawReadBitByte28 ^ 2;
extern void PLEXTOR_NEAR_FUNCTION LoadMediaCatalogNumberNearAdapter(void);
#define CURRENT_MODE_WRITE_BIT41(v) (CurrentModeRawReadBit0041 = (v))
#define CURRENT_MODE_WRITE_BIT42(v) (CurrentModeRawReadBit0042 = (v))
#define CURRENT_MODE_WRITE_BIT30(v) (CurrentModeRawReadBit0030 = (v))
#define CURRENT_MODE_INIT_CONTROL() InitializeRawReadSectorControlState()
#define CURRENT_MODE_INIT_IDENTIFICATION() InitializeSubchannelQIdentificationState()
#define CURRENT_MODE_INIT_TRACKS() InitializeRawReadTrackSelectionState()
#define CURRENT_MODE_CLEAR_STATUS() ClearRawReadPipelineStatusState()
#define CURRENT_MODE_INIT_RANDOM() InitializeRawReadRandomBytesFromDriveConfiguration()
#define CURRENT_MODE_UPDATE_Q_TYPE() UpdateRawReadQTypeStateFlag()
#define CURRENT_MODE_IS_ISRC_AVAILABLE() IsIsrcAvailableForCurrentTrack()
#define CURRENT_MODE_LOAD_ISRC() LoadIsrcCharactersForCurrentTrack()
#define CURRENT_MODE_PREPARE(a) PrepareRawReadAcquisitionPlan(a)
#define CURRENT_MODE_POLL(c) PollRawReadPipelineProgress(c)
#define CURRENT_MODE_FAILURE(c) ReportReadPipelineFailure(c)
#define CURRENT_MODE_QUEUE_SENSE() ReportSense030C09FromRawReadQueue()
#define CURRENT_MODE_CONFIGURE_LAYOUT() ConfigureRawReadTransferLayoutFromQTypeCode()
#define CURRENT_MODE_IS_MCN_AVAILABLE() IsMediaCatalogNumberAvailable()
#define CURRENT_MODE_LOAD_MCN() LoadMediaCatalogNumberNearAdapter()
#define CURRENT_MODE_INIT_TRACK_RECORDS() InitializeRawReadTrackRecordSelectionAndIdentificationState()
#define CURRENT_MODE_ACQUIRE_LEAD_IN() AcquireLeadInRawReadDescriptorsWithRetry()
#define CURRENT_MODE_ADVANCE_TRACK() AdvanceRawReadTrackRecordAndUpdateCurrentQPacket()
#define CURRENT_MODE_REFRESH_ISRC() RefreshIsrcForCurrentRawTrack()
#define CURRENT_MODE_COPY(s, d, c) CopyBytesWithinFarBanks((s), (d), (c))
#define CURRENT_MODE_DECREMENT_MSF(a) DecrementPackedBcdMsf(a)
#define CURRENT_MODE_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define CURRENT_MODE_ACQUIRE_FRAMES(c, r) AcquireRawReadFrameSequenceWithQUpdates((c), (r))
#define CURRENT_MODE_SELECT_BUFFER(l, f) SelectRawReadBufferAddressForLength((l), (f))
#define CURRENT_MODE_FILL(a, c, v) FillControllerBufferRange((a), (c), (v))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t tuple_address = FW_NEAR_UNKNOWN_7000;
#else
u8 tuple[4];
fw_addr_t tuple_address = (fw_addr_t)tuple;
#endif
#endif
