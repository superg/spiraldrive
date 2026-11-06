#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
static s32 DivideControllerRecordSlotPosition(s32 dividend, s32 divisor)
{
    return dividend / divisor;
}

extern u8 ObservePreparePrimaryType2ControllerRecordSlots(u8 acquisition_mode);
extern u8 ObservePrepareSecondaryType2ControllerRecordSlots(u8 acquisition_mode);
extern u8 ObserveAcquirePrimaryType2ControllerRecordSlot(u8 acquisition_mode, u8 slot_index, s32 start_frame_position);
extern u8 ObserveAcquireSecondaryType2ControllerRecordSlot(u8 acquisition_mode, u8 slot_index, s32 start_frame_position);

#define AcquirePrimaryType2ControllerRecordSlot ObserveAcquirePrimaryType2ControllerRecordSlot
#define AcquireSecondaryType2ControllerRecordSlot ObserveAcquireSecondaryType2ControllerRecordSlot
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
extern u8 AcquirePrimaryType2ControllerRecordSlot(u8 acquisition_mode, u8 slot_index, s32 start_frame_position);
extern u8 AcquireSecondaryType2ControllerRecordSlot(u8 acquisition_mode, u8 slot_index, s32 start_frame_position);
#define DivideControllerRecordSlotPosition(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u8 intercepted = ObservePreparePrimaryType2ControllerRecordSlots(acquisition_mode);
if(intercepted != 0xffu)
    return intercepted;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
u8 intercepted = ObservePrepareSecondaryType2ControllerRecordSlots(acquisition_mode);
if(intercepted != 0xffu)
    return intercepted;
#endif
#endif
