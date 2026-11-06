#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRecordableDiscTailUninitialized(void);
extern u8 ObserveRecordableDiscFirstTrackForLastSession(void);
extern u8 ObserveRecordableDiscType(void);
extern u32 ObserveRecordableDiscMultiply(u32 factor_a, u32 factor_b);
extern void ObserveRecordableDiscConvertMsf(s32 frame_position, fw_addr_t destination);
extern void ObserveRecordableDiscCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count);
#define RECORDABLE_TAIL_UNINITIALIZED() ObserveRecordableDiscTailUninitialized()
#define RECORDABLE_FIRST_TRACK_LAST_SESSION() ObserveRecordableDiscFirstTrackForLastSession()
#define RECORDABLE_DISC_TYPE() ObserveRecordableDiscType()
#define RECORDABLE_MULTIPLY(a, b) ObserveRecordableDiscMultiply((a), (b))
#define RECORDABLE_CONVERT_MSF(position, destination) ObserveRecordableDiscConvertMsf((position), (destination))
#define RECORDABLE_COPY(source, destination, count) ObserveRecordableDiscCopy((source), (destination), (count))
#else
#include "platform/arithmetic.h"
#include "platform/encoding.h"
#include "platform/leaf_workers.h"
#include "platform/memory.h"
#include "platform/status_mapping.h"
#include "scsi/spc.h"
#define RECORDABLE_TAIL_UNINITIALIZED() IsTrackTableTailEntryUninitialized()
#define RECORDABLE_FIRST_TRACK_LAST_SESSION() FindFirstTrackNumberForLastFullTocSession()
#define RECORDABLE_DISC_TYPE() SelectDiscInformationDiscTypeFromFullToc()
#define RECORDABLE_MULTIPLY(a, b) MultiplyU32Low((a), (b))
#define RECORDABLE_CONVERT_MSF(position, destination) ConvertFramePositionToBinaryMsfTuple((position), (destination))
#define RECORDABLE_COPY(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u32 value = (u32)FirmwareReadNear8(address) << 24;
value |= (u32)FirmwareReadNear8((u16)(address + 1u)) << 16;
value |= (u32)FirmwareReadNear8((u16)(address + 2u)) << 8;
value |= FirmwareReadNear8((u16)(address + 3u));
return value;
#else
return *((volatile u32 near *)address);
#endif
#endif
