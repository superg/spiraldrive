#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSyntheticTrackResetTable(void);
extern void ObserveSyntheticTrackClearWorking(void);
extern void ObserveSyntheticTrackUpdateTail(void);
extern void ObserveSyntheticTrackClearControllerWords(void);
extern void ObserveSyntheticTrackUpdateControl(void);
extern void ObserveSyntheticTrackBuildPrimary(void);
extern void ObserveSyntheticTrackBuildSecondary(void);
extern u8 ObserveSyntheticTrackAcquireDescriptors(void);
#define RESET_SYNTHETIC_TRACK_TABLE() ObserveSyntheticTrackResetTable()
#define CLEAR_SYNTHETIC_WORKING_TRACK() ObserveSyntheticTrackClearWorking()
#define UPDATE_SYNTHETIC_TRACK_TAIL() ObserveSyntheticTrackUpdateTail()
#define CLEAR_SYNTHETIC_CONTROLLER_WORDS() ObserveSyntheticTrackClearControllerWords()
#define UPDATE_SYNTHETIC_TRACK_CONTROL() ObserveSyntheticTrackUpdateControl()
#define BUILD_SYNTHETIC_PRIMARY() ObserveSyntheticTrackBuildPrimary()
#define BUILD_SYNTHETIC_SECONDARY() ObserveSyntheticTrackBuildSecondary()
#define ACQUIRE_SYNTHETIC_DESCRIPTORS() ObserveSyntheticTrackAcquireDescriptors()
#else
extern u8 AcquireLeadInAndLeadOutRawSectorDescriptorSequences(void);
#define RESET_SYNTHETIC_TRACK_TABLE() ResetTrackInformationTableState()
#define CLEAR_SYNTHETIC_WORKING_TRACK() ClearWorkingTrackInformation()
#define UPDATE_SYNTHETIC_TRACK_TAIL() UpdateSelectedTrackTableTailFrameBounds()
#define CLEAR_SYNTHETIC_CONTROLLER_WORDS() ClearControllerBufferWordsC119AndC517()
#define UPDATE_SYNTHETIC_TRACK_CONTROL() UpdateTrackTableControlBit6Intersection()
#define BUILD_SYNTHETIC_PRIMARY() BuildTrackTablePrimaryStagedRawQRecordSequence()
#define BUILD_SYNTHETIC_SECONDARY() BuildTrackTableSecondaryStagedRawQRecordSequence()
#define ACQUIRE_SYNTHETIC_DESCRIPTORS() AcquireLeadInAndLeadOutRawSectorDescriptorSequences()
#endif
#endif
