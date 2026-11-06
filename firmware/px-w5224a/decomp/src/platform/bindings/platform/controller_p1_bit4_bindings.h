#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
void ClearP1Bit4(void)
{
    FirmwareWriteDirectSfr8(90, (u8)(FirmwareReadDirectSfr8(90) & 0xefu));
}

void SetP1Bit4(void)
{
    FirmwareWriteDirectSfr8(90, (u8)(FirmwareReadDirectSfr8(90) | 0x10u));
}
#else
sbit P1Bit4Latch = 0x94;

/* RAM:f8e72e */
void ClearP1Bit4(void)
{
    P1Bit4Latch = 0;
}

/* RAM:f8e731 */
void SetP1Bit4(void)
{
    P1Bit4Latch = 1;
}
#endif
#endif
