#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFormatTrackClear(void);
extern void ObserveFormatTrackResetTable(void);
extern void ObserveFormatTrackUpdateDataBlock(u8 old_class, u8 old_mask);
extern u8 ObserveFormatTrackTailUninitialized(void);
extern void ObserveFormatTrackLoad(u8 track_number);
extern void ObserveFormatTrackCommit(void);
extern void ObserveFormatTrackSetBit000A(void);
#define FORMAT_TRACK_CLEAR() ObserveFormatTrackClear()
#define FORMAT_TRACK_RESET_TABLE() ObserveFormatTrackResetTable()
#define FORMAT_TRACK_UPDATE_DATA_BLOCK(old_class, old_mask) ObserveFormatTrackUpdateDataBlock((old_class), (old_mask))
#define FORMAT_TRACK_TAIL_UNINITIALIZED() ObserveFormatTrackTailUninitialized()
#define FORMAT_TRACK_LOAD(number) ObserveFormatTrackLoad(number)
#define FORMAT_TRACK_COMMIT() ObserveFormatTrackCommit()
#define FORMAT_TRACK_SET_ACTIVE_BIT() ObserveFormatTrackSetBit000A()
#else
#include "platform/status_mapping.h"
#include "platform/track_state.h"
extern void LoadTrackInformationByNumber(u8 track_number);
#define FORMAT_TRACK_CLEAR() ClearWorkingTrackInformation()
#define FORMAT_TRACK_RESET_TABLE() ResetTrackInformationTableState()
#define FORMAT_TRACK_UPDATE_DATA_BLOCK(old_class, old_mask) UpdateWorkingTrackDataBlockTypeFields((old_class), (old_mask))
#define FORMAT_TRACK_TAIL_UNINITIALIZED() IsTrackTableTailEntryUninitialized()
#define FORMAT_TRACK_LOAD(number) LoadTrackInformationByNumber(number)
#define FORMAT_TRACK_COMMIT() UpdateTrackTableFromWorkingTrack()
#define FORMAT_TRACK_SET_ACTIVE_BIT() FirmwareWriteBit000A(1u)
#endif
#endif
