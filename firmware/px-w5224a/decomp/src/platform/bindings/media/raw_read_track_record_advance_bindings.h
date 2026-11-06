#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveTrackAdvanceConfigureLayout(void);
extern s32 ObserveTrackAdvanceDistance(u16 record_index);
extern u8 ObserveTrackAdvanceBcd(u8 value);
extern void ObserveTrackAdvanceRestoreIsrc(u8 decrement_relative_msf);
extern void ObserveTrackAdvanceRestoreMcn(u8 decrement_relative_msf);
extern void ObserveTrackAdvanceSetMsf(s32 relative, s32 absolute);
extern u8 ObserveTrackAdvanceDecode(u16 record_index);
extern void ObserveTrackAdvanceSelectIsrc(void);
extern s32 ObserveTrackAdvanceBoundary(u16 record_index);
#define TRACK_ADVANCE_CONFIGURE() ObserveTrackAdvanceConfigureLayout()
#define TRACK_ADVANCE_DISTANCE(i) ObserveTrackAdvanceDistance(i)
#define TRACK_ADVANCE_BCD(v) ObserveTrackAdvanceBcd(v)
#define TRACK_ADVANCE_RESTORE_ISRC(d) ObserveTrackAdvanceRestoreIsrc(d)
#define TRACK_ADVANCE_RESTORE_MCN(d) ObserveTrackAdvanceRestoreMcn(d)
#define TRACK_ADVANCE_SET_MSF(r, a) ObserveTrackAdvanceSetMsf((r), (a))
#define TRACK_ADVANCE_DECODE(i) ObserveTrackAdvanceDecode(i)
#define TRACK_ADVANCE_SELECT_ISRC() ObserveTrackAdvanceSelectIsrc()
#define TRACK_ADVANCE_BOUNDARY(i) ObserveTrackAdvanceBoundary(i)
#else
#define TRACK_ADVANCE_CONFIGURE() ConfigureRawReadTransferLayoutFromQTypeCode()
#define TRACK_ADVANCE_DISTANCE(i) ComputeFrameDistanceToNextType1TrackRecordBoundary(i)
#define TRACK_ADVANCE_BCD(v) BinaryByteToPackedBcd(v)
#define TRACK_ADVANCE_RESTORE_ISRC(d) RestoreCurrentQPacketAfterIsrc(d)
#define TRACK_ADVANCE_RESTORE_MCN(d) RestoreCurrentQPacketAfterMediaCatalog(d)
#define TRACK_ADVANCE_SET_MSF(r, a) SetRawReadQPacketMsfTuples((r), (a))
#define TRACK_ADVANCE_DECODE(i) DecodeRawReadQRecordTypeCode(i)
#define TRACK_ADVANCE_SELECT_ISRC() SelectIsrcRawTrackRecordForCurrentTrack()
#define TRACK_ADVANCE_BOUNDARY(i) GetType1TrackRecordFramePositionAtOrAfterIndex(i)
#endif
#endif
