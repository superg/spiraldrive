#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveType3RecordBlockUpdate(s16 position_block_index);
extern u16 ObserveType3RecordBankScan(u8 force_bank_scan, s32 start_frame_position, u32 frame_count);
#define TYPE3_UPDATE_BLOCK(index) ObserveType3RecordBlockUpdate(index)
#define TYPE3_SCAN_BANKS(force, start, count) ObserveType3RecordBankScan((force), (start), (count))
#else
extern void UpdateThirtyTwoControllerBufferRecordPositionsIfBlockChanged(s16 position_block_index);
extern u16 ScanControllerRecordBanksForFrameRange(u8 force_bank_scan, s32 start_frame_position, u32 frame_count);
#define TYPE3_UPDATE_BLOCK(index) UpdateThirtyTwoControllerBufferRecordPositionsIfBlockChanged(index)
#define TYPE3_SCAN_BANKS(force, start, count) ScanControllerRecordBanksForFrameRange((force), (start), (count))
#endif
#endif
