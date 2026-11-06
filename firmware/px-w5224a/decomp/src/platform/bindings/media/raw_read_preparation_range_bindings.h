#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveInitializeRawReadPreparationSegments(u8 read_plan_code);
extern s32 ObserveSelectRawReadPreparationRangeEnd(u8 alternate_range);
extern u8 ObserveSelectRawReadSectorClassForPreparation(u8 prefer_working_track_class, u8 fallback_class);
extern void ObserveBuildRawReadQPacketForPreparation(s32 frame_position, fw_addr_t destination);
extern void ObserveBuildRawSectorTdiPrefixForPreparation(fw_addr_t destination);
#define INITIALIZE_PREPARATION_SEGMENTS(code) ObserveInitializeRawReadPreparationSegments(code)
#define SELECT_PREPARATION_RANGE_END(alternate) ObserveSelectRawReadPreparationRangeEnd(alternate)
#define SELECT_PREPARATION_SECTOR_CLASS(prefer, fallback) ObserveSelectRawReadSectorClassForPreparation((prefer), (fallback))
#define BUILD_PREPARATION_Q_PACKET(position, destination) ObserveBuildRawReadQPacketForPreparation((position), (destination))
#define BUILD_PREPARATION_TDI_PREFIX(destination) ObserveBuildRawSectorTdiPrefixForPreparation(destination)
#else
#define INITIALIZE_PREPARATION_SEGMENTS(code) InitializeRawReadPreparationSegmentsFromPlanCode(code)
#define SELECT_PREPARATION_RANGE_END(alternate) SelectRawReadPreparationRangeEnd(alternate)
#define SELECT_PREPARATION_SECTOR_CLASS(prefer, fallback) SelectRawReadSectorClass((prefer), (fallback))
#define BUILD_PREPARATION_Q_PACKET(position, destination) BuildRawReadQPacketForFramePosition((position), (destination))
#define BUILD_PREPARATION_TDI_PREFIX(destination) BuildRawSectorTdiPayloadPrefix(destination)
#endif
#endif
