#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRecordPageHeaderCompact(u8 include_type_zero_and_one);
extern void ObserveRecordPageHeaderCopyFrom(fw_addr_t source_base, fw_addr_t destination, u8 byte_count);
extern void ObserveRecordPageHeaderCopyTo(fw_addr_t destination_base, fw_addr_t source, u8 byte_count);
extern u32 ObserveRecordPageHeaderFind(u32 address_key, u8 search_mode);
extern void ObserveRecordPageHeaderClearPresence(u16 group_index, u16 page_index);
#define COMPACT_RECORD_PAGE(value) ObserveRecordPageHeaderCompact(value)
#define COPY_RECORD_PAGE_FROM(source, destination, count) ObserveRecordPageHeaderCopyFrom((source), (destination), (count))
#define COPY_RECORD_PAGE_TO(destination, source, count) ObserveRecordPageHeaderCopyTo((destination), (source), (count))
#define FIND_RECORD_PAGE_ENTRY(key, mode) ObserveRecordPageHeaderFind((key), (mode))
#define CLEAR_RECORD_PAGE_PRESENCE(group, page) ObserveRecordPageHeaderClearPresence((group), (page))
#else
extern void CompactAndWriteControllerBufferRecordPage(u8 include_type_zero_and_one);
extern u32 FindControllerBufferSixByteRecord(u32 address_key, u8 search_mode);
#define COMPACT_RECORD_PAGE(value) CompactAndWriteControllerBufferRecordPage(value)
#define COPY_RECORD_PAGE_FROM(source, destination, count) CopyBytesFromAddressPlus1MiB((source), (destination), (count))
#define COPY_RECORD_PAGE_TO(destination, source, count) CopyBytesToAddressPlus1MiB((destination), (source), (count))
#define FIND_RECORD_PAGE_ENTRY(key, mode) FindControllerBufferSixByteRecord((key), (mode))
#define CLEAR_RECORD_PAGE_PRESENCE(group, page) ClearControllerBufferRecordZonePresenceBit((group), (page))
#endif
#endif
