#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveDriveState10PhaseBit(void);
extern void ObserveDriveState10ClearPhaseBit(void);
extern void ObserveDriveState10SubmitControllerWord(u16 controller_value);
extern u8 ObserveDriveState10DebouncedP1Bit7Low(void);
extern u8 ObserveDriveState10DebouncedP1Bit5Low(void);
extern void ObserveDriveState10YieldCoroutine1(void);
extern u8 ObserveDriveState10TakeStateFlag11EFBit5(void);
extern void ObserveDriveState10ClearControllerWord0252(void);
#define DriveState10PhaseBitSet() ObserveDriveState10PhaseBit()
#define DriveState10ClearPhaseBit() ObserveDriveState10ClearPhaseBit()
#define DriveState10SubmitWord(value) ObserveDriveState10SubmitControllerWord(value)
#define DriveState10P1Bit7Low() ObserveDriveState10DebouncedP1Bit7Low()
#define DriveState10P1Bit5Low() ObserveDriveState10DebouncedP1Bit5Low()
#define DriveState10Yield() ObserveDriveState10YieldCoroutine1()
#define DriveState10TakeStateFlag() ObserveDriveState10TakeStateFlag11EFBit5()
#define DriveState10ClearWord() ObserveDriveState10ClearControllerWord0252()
#else
extern void YieldCoroutine1(void);
u8 bdata DriveState10PhaseBitByte _at_ 0x27;
sbit DriveState10PhaseBit = DriveState10PhaseBitByte ^ 3;
#define DriveState10PhaseBitSet() ((u8)DriveState10PhaseBit)
#define DriveState10ClearPhaseBit() (DriveState10PhaseBit = 0)
#define DriveState10SubmitWord(value) SubmitControllerWord0252WithP1Bit0Set(value)
#define DriveState10P1Bit7Low() IsDebouncedP1Bit7Low()
#define DriveState10P1Bit5Low() IsDebouncedP1Bit5Low()
#define DriveState10Yield() YieldCoroutine1()
#define DriveState10TakeStateFlag() TakeStateFlag11EFBit5()
#define DriveState10ClearWord() ClearControllerWord0252AndResetChannel2()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_FAAC);
#else
return *((volatile u16 near *)0xfaac);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAC, (u8)(value >> 8));
FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAD, (u8)value);
#else
*((volatile u16 near *)0xfaac) = value;
#endif
#endif
