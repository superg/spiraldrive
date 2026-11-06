#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSelectRawSectorCaptureModeAndLength(u8 *capture_mode, u16 *transfer_length, u8 raw_sector_source_kind);
extern u8 ObserveRawSectorCaptureCompletion(void);
#define SELECT_CAPTURE_MODE_AND_LENGTH(mode, length, source_kind) ObserveSelectRawSectorCaptureModeAndLength((mode), (length), (source_kind))
#define RAW_SECTOR_CAPTURE_COMPLETE() ObserveRawSectorCaptureCompletion()
#else
#define SELECT_CAPTURE_MODE_AND_LENGTH(mode, length, source_kind) SelectRawSectorCaptureModeAndLength((length), (mode), (source_kind))
#define RAW_SECTOR_CAPTURE_COMPLETE() FirmwareReadNear8(FW_NEAR_UNKNOWN_14DA)
#endif
#endif
