#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
/* Reconstruction helper; no direct Ghidra function address. */
static void SetP3Bit0Atomic(void)
{
    FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) | 0x01u));
}

/* Reconstruction helper; no direct Ghidra function address. */
static void ClearP3Bit0Atomic(void)
{
    FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) & 0xfeu));
}

/* Reconstruction helper; no direct Ghidra function address. */
static void SetP3Bit1Atomic(void)
{
    FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) | 0x02u));
}

/* Reconstruction helper; no direct Ghidra function address. */
static void ClearP3Bit1Atomic(void)
{
    FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) & 0xfdu));
}
#else
extern void SetP3Bit0Atomic(void);
extern void ClearP3Bit0Atomic(void);
extern void SetP3Bit1Atomic(void);
extern void ClearP3Bit1Atomic(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
/* Reconstruction helper; no direct Ghidra function address. */
static void SetDriveTransitionOutputLinesHigh(void)
{
    FirmwareWriteBit0580(1u);
    FirmwareWriteBit0581(1u);
}
#else
extern void SetP3Bits0And1HighForTimedOutputSequence(void);
#define SetDriveTransitionOutputLinesHigh() SetP3Bits0And1HighForTimedOutputSequence()
#endif
#endif
