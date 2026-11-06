#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveRecordPageUpdateFind(s32 key, u8 mode);
extern void ObserveRecordPageUpdateCopyFrom(u32 source, fw_addr_t destination, u8 count);
extern void ObserveRecordPageUpdateCopyTo(u32 destination, fw_addr_t source, u8 count);
extern void ObserveRecordPageUpdateCopySix(u32 destination, u32 source);
extern void ObserveRecordPageUpdateCopyDwords(u32 destination, fw_addr_t source, u16 count);
extern void ObserveRecordPageUpdateCompact(u8 include_type_zero_and_one);
extern void ObserveRecordPageUpdateClear(u16 group, u16 page);
extern void ObserveRecordPageUpdateRebuild(u8 group, u8 page);
extern void ObserveRecordPageUpdateReconcileBackward(u8 group, u8 page);
extern void ObserveRecordPageUpdateReconcileForward(u8 group, u8 page);
#define FIND_RECORD(key, mode) ObserveRecordPageUpdateFind((key), (mode))
#define COPY_FROM_PAGE(source, destination, count) ObserveRecordPageUpdateCopyFrom((source), (destination), (count))
#define COPY_TO_PAGE(destination, source, count) ObserveRecordPageUpdateCopyTo((destination), (source), (count))
#define COPY_SIX(destination, source) ObserveRecordPageUpdateCopySix((destination), (source))
#define COPY_DWORDS(destination, source, count) ObserveRecordPageUpdateCopyDwords((destination), (source), (count))
#define COMPACT_PAGE(include_types) ObserveRecordPageUpdateCompact(include_types)
#define CLEAR_PRESENCE(group, page) ObserveRecordPageUpdateClear((group), (page))
#define REBUILD_PAGE(group, page) ObserveRecordPageUpdateRebuild((group), (page))
#define RECONCILE_BACKWARD(group, page) ObserveRecordPageUpdateReconcileBackward((group), (page))
#define RECONCILE_FORWARD(group, page) ObserveRecordPageUpdateReconcileForward((group), (page))
#else
extern u32 FindControllerBufferSixByteRecord(u32 address_key, u8 search_mode);
extern void CompactAndWriteControllerBufferRecordPage(u8 include_type_zero_and_one);
#define FIND_RECORD(key, mode) FindControllerBufferSixByteRecord((u32)(key), (mode))
#define COPY_FROM_PAGE(source, destination, count) CopyBytesFromAddressPlus1MiB((source), (destination), (count))
#define COPY_TO_PAGE(destination, source, count) CopyBytesToAddressPlus1MiB((destination), (source), (count))
#define COPY_SIX(destination, source) CopySixBytesFar((destination), (source))
#define COPY_DWORDS(destination, source, count) CopyDwordsToAddressPlus1MiB((destination), (source), (count))
#define COMPACT_PAGE(include_types) CompactAndWriteControllerBufferRecordPage(include_types)
#define CLEAR_PRESENCE(group, page) ClearControllerBufferRecordZonePresenceBit((group), (page))
#define REBUILD_PAGE(group, page) RebuildControllerBufferRecordPageHeader((group), (page))
#define RECONCILE_BACKWARD(group, page) ReconcileControllerBufferRecordsBackward((group), (page))
#define RECONCILE_FORWARD(group, page) ReconcileControllerBufferRecordsForward((group), (page))
#endif
#endif
