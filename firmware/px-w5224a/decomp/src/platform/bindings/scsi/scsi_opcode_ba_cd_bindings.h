#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateOpcodeBaCdCdb(fw_addr_t mask_address);
extern void ObserveRejectOpcodeBaCd(void);
extern u8 ObserveValidateOpcodeBaCdMsf(fw_addr_t tuple_address);
extern s32 ObserveConvertOpcodeBaCdMsf(fw_addr_t tuple_address);
extern u8 ObserveGetOpcodeBaCdLastTrack(void);
extern void ObserveLoadOpcodeBaCdTrack(u8 track_number);
extern s32 ObserveSelectOpcodeBaCdTrackBoundary(void);
extern void ObserveSetOpcodeBaCdSense(u8 sense_key, u16 asc_ascq);
extern void ObserveWriteOpcodeBaCdBit0018(u8 value);
extern void ObserveWriteOpcodeBaCdBit0019(u8 value);
extern void ObserveStartOpcodeBaCdRead(s32 frame_position);
#define VALIDATE_OPCODE_BA_CD_CDB(address) ObserveValidateOpcodeBaCdCdb(address)
#define REJECT_OPCODE_BA_CD() ObserveRejectOpcodeBaCd()
#define VALIDATE_OPCODE_BA_CD_MSF(address) ObserveValidateOpcodeBaCdMsf(address)
#define CONVERT_OPCODE_BA_CD_MSF(address) ObserveConvertOpcodeBaCdMsf(address)
#define GET_OPCODE_BA_CD_LAST_TRACK() ObserveGetOpcodeBaCdLastTrack()
#define LOAD_OPCODE_BA_CD_TRACK(track) ObserveLoadOpcodeBaCdTrack(track)
#define SELECT_OPCODE_BA_CD_TRACK_BOUNDARY() ObserveSelectOpcodeBaCdTrackBoundary()
#define SET_OPCODE_BA_CD_SENSE(key, code) ObserveSetOpcodeBaCdSense((key), (code))
#define WRITE_OPCODE_BA_CD_BIT0018(value) ObserveWriteOpcodeBaCdBit0018(value)
#define WRITE_OPCODE_BA_CD_BIT0019(value) ObserveWriteOpcodeBaCdBit0019(value)
#define START_OPCODE_BA_CD_READ(frame) ObserveStartOpcodeBaCdRead(frame)
#else
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_address);
extern void HandleUnsupportedScsiCommand(void);
extern u8 IsBinaryMsfTupleValid(fw_addr_t tuple_address);
extern s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address);
extern u8 GetEffectiveLastTrackNumber(void);
extern void LoadTrackInformationByNumber(u8 track_number);
extern s32 SelectWorkingTrackFramePosition(void);
extern void SetSenseData(u8 sense_key, u16 asc_ascq);
extern void SetBit0018Atomic(void);
extern void WriteBit0019FromAcc(u8 value);
extern void StartFormat2RawSectorReadAtFramePosition(s32 frame_position);
#define VALIDATE_OPCODE_BA_CD_CDB(address) ValidateScsiCdbFieldMasks(address)
#define REJECT_OPCODE_BA_CD() HandleUnsupportedScsiCommand()
#define VALIDATE_OPCODE_BA_CD_MSF(address) IsBinaryMsfTupleValid(address)
#define CONVERT_OPCODE_BA_CD_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define GET_OPCODE_BA_CD_LAST_TRACK() GetEffectiveLastTrackNumber()
#define LOAD_OPCODE_BA_CD_TRACK(track) LoadTrackInformationByNumber(track)
#define SELECT_OPCODE_BA_CD_TRACK_BOUNDARY() SelectWorkingTrackFramePosition()
#define SET_OPCODE_BA_CD_SENSE(key, code) SetSenseData((key), (code))
#define WRITE_OPCODE_BA_CD_BIT0018(value) SetBit0018Atomic()
#define WRITE_OPCODE_BA_CD_BIT0019(value) WriteBit0019FromAcc(value)
#define START_OPCODE_BA_CD_READ(frame) StartFormat2RawSectorReadAtFramePosition(frame)
#endif
#endif
