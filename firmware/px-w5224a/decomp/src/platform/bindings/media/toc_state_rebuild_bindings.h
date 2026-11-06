#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveParseRawReadQRecordTableIntoTocState(void);
#define REBUILD_TOC_PARSE_RAW_Q() ObserveParseRawReadQRecordTableIntoTocState()
#else
#include "media/toc_state_parse.h"
#define REBUILD_TOC_PARSE_RAW_Q() ParseRawReadQRecordTableIntoTocState()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return (u16)(((u16)FirmwareRead8(address) << 8) | FirmwareReadNear8((u16)(address + 1u)));
#else
return *((volatile u16 near *)address);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
return ((u32)FirmwareRead8(address) << 24) | ((u32)FirmwareReadNear8((u16)(address + 1u)) << 16) | ((u32)FirmwareReadNear8((u16)(address + 2u)) << 8) | FirmwareReadNear8((u16)(address + 3u));
#else
return *((volatile u32 near *)address);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
FirmwareWrite8(address, (u8)(value >> 24));
FirmwareWriteNear8((u16)(address + 1u), (u8)(value >> 16));
FirmwareWriteNear8((u16)(address + 2u), (u8)(value >> 8));
FirmwareWriteNear8((u16)(address + 3u), (u8)value);
#else
*((volatile u32 near *)address) = value;
#endif
#endif
