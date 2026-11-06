#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveModePageInitializationCopy(fw_addr_t destination, fw_addr_t source, u8 byte_count);
extern void ObserveModePageInitializationApply(u8 page_code);
extern void ObserveModePage05WriteTestBit(u8 enabled);
extern void ObserveModePage05SnapshotTrackTail(void);
extern void ObserveModePage05Copy(fw_addr_t destination, fw_addr_t source, u8 byte_count);
#define INITIALIZE_MODE_PAGE_COPY(destination, source, byte_count) ObserveModePageInitializationCopy(destination, source, byte_count)
#define INITIALIZE_MODE_PAGE_APPLY(page_code, function_call) ObserveModePageInitializationApply(page_code)
#define MODE_PAGE_05_WRITE_TEST_BIT(enabled) ObserveModePage05WriteTestBit(enabled)
#define MODE_PAGE_05_SNAPSHOT_TRACK_TAIL() ObserveModePage05SnapshotTrackTail()
#define MODE_PAGE_05_COPY(destination, source, byte_count) ObserveModePage05Copy(destination, source, byte_count)
#else
extern void PLEXTOR_NEAR_FUNCTION SetBit002FForModePage05Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit002FForModePage05Atomic(void);
#define INITIALIZE_MODE_PAGE_COPY(destination, source, byte_count) CopyBytes8(destination, source, byte_count)
#define INITIALIZE_MODE_PAGE_APPLY(page_code, function_call) function_call
#define MODE_PAGE_05_WRITE_TEST_BIT(enabled)                                                                                                                                                           \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if((enabled) != 0u)                                                                                                                                                                            \
            SetBit002FForModePage05Atomic();                                                                                                                                                           \
        else                                                                                                                                                                                           \
            ClearBit002FForModePage05Atomic();                                                                                                                                                         \
    } while(0)
#define MODE_PAGE_05_SNAPSHOT_TRACK_TAIL() SnapshotTrackTableTailState()
#define MODE_PAGE_05_COPY(destination, source, byte_count) CopyBytesWith8BitCount(destination, source, byte_count)
#endif
#endif
