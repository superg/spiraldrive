#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 FirmwareReadBit000D(void);
extern void FirmwareWriteBit000D(u8 value);
extern void FirmwareWriteBit000E(u8 value);
extern void FirmwareWriteBit0018(u8 value);
extern void FirmwareWriteBit0046(u8 value);
#define READ_PENDING_RAW_SECTOR_COMMAND() FirmwareReadBit000D()
#define CLEAR_PENDING_RAW_SECTOR_COMMAND() FirmwareWriteBit000D(0u)
#define CLEAR_RAW_SECTOR_BIT000E() FirmwareWriteBit000E(0u)
#define CLEAR_RAW_SECTOR_BIT0018() FirmwareWriteBit0018(0u)
#define SET_RAW_SECTOR_SCOPE() FirmwareWriteBit0046(1u)
#define CLEAR_RAW_SECTOR_SCOPE() FirmwareWriteBit0046(0u)
#else
extern u8 ReadBit000DForVendorD8(void);
extern void ClearBit000DForReadHeaderAtomic(void);
extern void ClearBit000EForRawSectorTransferSetupAtomic(void);
extern void ClearBit0018Atomic(void);
extern void SetBit0046ForReadCommandAtomic(void);
extern void ClearBit0046ForReadCommandAtomic(void);
#define READ_PENDING_RAW_SECTOR_COMMAND() ReadBit000DForVendorD8()
#define CLEAR_PENDING_RAW_SECTOR_COMMAND() ClearBit000DForReadHeaderAtomic()
#define CLEAR_RAW_SECTOR_BIT000E() ClearBit000EForRawSectorTransferSetupAtomic()
#define CLEAR_RAW_SECTOR_BIT0018() ClearBit0018Atomic()
#define SET_RAW_SECTOR_SCOPE() SetBit0046ForReadCommandAtomic()
#define CLEAR_RAW_SECTOR_SCOPE() ClearBit0046ForReadCommandAtomic()
#endif
#endif
