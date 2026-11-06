#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void FirmwareWriteBit000D(u8 value);
extern void FirmwareWriteBit0013(u8 value);
extern void FirmwareWriteBit0018(u8 value);
extern void FirmwareWriteBit001B(u8 value);
extern void FirmwareWriteBit001C(u8 value);
extern void FirmwareWriteBit001D(u8 value);
#define CLEAR_READ_MASTER_CUE_ENTRY_BITS()                                                                                                                                                             \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit000E(0u);                                                                                                                                                                      \
        FirmwareWriteBit0018(0u);                                                                                                                                                                      \
    } while(0)
#define CLEAR_READ_MASTER_CUE_MODE_BITS()                                                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit000C(0u);                                                                                                                                                                      \
        FirmwareWriteBit000D(0u);                                                                                                                                                                      \
        FirmwareWriteBit001D(0u);                                                                                                                                                                      \
        FirmwareWriteBit001C(0u);                                                                                                                                                                      \
        FirmwareWriteBit0013(0u);                                                                                                                                                                      \
        FirmwareWriteBit001B(0u);                                                                                                                                                                      \
    } while(0)
#define ENABLE_READ_MASTER_CUE_ACQUISITION() FirmwareWriteBit000F(1u)
#define SET_READ_MASTER_CUE_PINS(high)                                                                                                                                                                 \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit0580((high));                                                                                                                                                                  \
        FirmwareWriteBit0581((high));                                                                                                                                                                  \
    } while(0)
#else
extern void ClearReadMasterCueEntryBitsAtomic(void);
extern void ClearReadMasterCueModeBitsAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetExternalInterrupt0EnableAtomic(void);
extern void SetReadMasterCueHandshakePinsLowAtomic(void);
extern void SetReadMasterCueHandshakePinsHighAtomic(void);
#define CLEAR_READ_MASTER_CUE_ENTRY_BITS() ClearReadMasterCueEntryBitsAtomic()
#define CLEAR_READ_MASTER_CUE_MODE_BITS() ClearReadMasterCueModeBitsAtomic()
#define ENABLE_READ_MASTER_CUE_ACQUISITION() SetExternalInterrupt0EnableAtomic()
#define SET_READ_MASTER_CUE_PINS(high) ((high) != 0u ? SetReadMasterCueHandshakePinsHighAtomic() : SetReadMasterCueHandshakePinsLowAtomic())
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t response_prefix_address = FW_NEAR_UNKNOWN_7000;
#else
u8 response_prefix[4];
fw_addr_t response_prefix_address = (fw_addr_t)response_prefix;
#endif
#endif
