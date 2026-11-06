#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveBlankTrackClearWorking(void);
extern void ObserveBlankTrackResetTable(void);
extern void ObserveBlankTrackLoad(u8 track);
extern s32 ObserveBlankTrackConvertMsf(fw_addr_t address);
extern u8 ObserveBlankTrackLastTrack(void);
extern void ObserveBlankTrackClearRecord(u8 index);
extern u8 ObserveBlankTrackFindEntry(u8 track_number);
extern u8 ObserveBlankTrackFindLastForSession(u8 session);
extern u8 ObserveBlankTrackTailUninitialized(void);
extern void ObserveBlankTrackResetRecords(u8 first_index);
extern void ObserveBlankTrackCommit(void);
#define BLANK_TRACK_CLEAR_WORKING() ObserveBlankTrackClearWorking()
#define BLANK_TRACK_RESET_TABLE() ObserveBlankTrackResetTable()
#define BLANK_TRACK_LOAD(track) ObserveBlankTrackLoad(track)
#define BLANK_TRACK_CONVERT(address) ObserveBlankTrackConvertMsf(address)
#define BLANK_TRACK_LAST_TRACK() ObserveBlankTrackLastTrack()
#define BLANK_TRACK_CLEAR_RECORD(index) ObserveBlankTrackClearRecord(index)
#define BLANK_TRACK_FIND_ENTRY(track) ObserveBlankTrackFindEntry(track)
#define BLANK_TRACK_FIND_LAST(session) ObserveBlankTrackFindLastForSession(session)
#define BLANK_TRACK_TAIL_UNINITIALIZED() ObserveBlankTrackTailUninitialized()
#define BLANK_TRACK_RESET_RECORDS(index) ObserveBlankTrackResetRecords(index)
#define BLANK_TRACK_COMMIT() ObserveBlankTrackCommit()
#else
#include "platform/encoding.h"
#include "platform/status_mapping.h"
#include "platform/track_state.h"
extern void LoadTrackInformationByNumber(u8 track_number);
#define BLANK_TRACK_CLEAR_WORKING() ClearWorkingTrackInformation()
#define BLANK_TRACK_RESET_TABLE() ResetTrackInformationTableState()
#define BLANK_TRACK_LOAD(track) LoadTrackInformationByNumber(track)
#define BLANK_TRACK_CONVERT(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define BLANK_TRACK_LAST_TRACK() GetEffectiveLastTrackNumber()
#define BLANK_TRACK_CLEAR_RECORD(index) ClearTrackTableRecordAtIndex(index)
#define BLANK_TRACK_FIND_ENTRY(track) FindTrackTableEntryByNumber(track)
#define BLANK_TRACK_FIND_LAST(session) FindLastTrackTableIndexForSessionByte(session)
#define BLANK_TRACK_TAIL_UNINITIALIZED() IsTrackTableTailEntryUninitialized()
#define BLANK_TRACK_RESET_RECORDS(index) ResetTrackInformationRecordsFromIndex(index)
#define BLANK_TRACK_COMMIT() UpdateTrackTableFromWorkingTrack()
#endif
#endif
