#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveBackwardRecordPageRebuild(u8 group_index, u8 page_index);
extern void ObserveBackwardRecordPageTransfer(u32 first_controller_address, u32 second_controller_address, u32 controller_word_count);
extern void ObserveBackwardRecordPageYield(void);
#define REBUILD_RECORD_PAGE(group, page) ObserveBackwardRecordPageRebuild((group), (page))
#define TRANSFER_RECORD_PAGE_WORDS(first, second, count) ObserveBackwardRecordPageTransfer((first), (second), (count))
#define YIELD_BACKWARD_RECONCILER() ObserveBackwardRecordPageYield()
#else
extern void RebuildControllerBufferRecordPageHeader(u8 group_index, u8 page_index);
extern void YieldCoroutine0(void);
#define REBUILD_RECORD_PAGE(group, page) RebuildControllerBufferRecordPageHeader((group), (page))
#define TRANSFER_RECORD_PAGE_WORDS(first, second, count) TransferControllerBufferWordsAndWait((first), (second), (count))
#define YIELD_BACKWARD_RECONCILER() YieldCoroutine0()
#endif
#endif
