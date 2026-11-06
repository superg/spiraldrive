#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void FirmwareWriteBit001E(u8 value);
#define CLEAR_CD_TEXT_PARTIAL_PACK_BIT() FirmwareWriteBit001E(0u)
#else
u8 bdata CdTextPackValidationBitByte23 _at_ 0x23;
sbit CdTextPackValidationBit001E = CdTextPackValidationBitByte23 ^ 6;
#define CLEAR_CD_TEXT_PARTIAL_PACK_BIT() (CdTextPackValidationBit001E = 0)
#endif
#endif
