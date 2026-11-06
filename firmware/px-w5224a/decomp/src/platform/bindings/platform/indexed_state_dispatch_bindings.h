#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR15ForIndexedStateGroup(void);
extern void ObserveClearBit0018ForIndexedStateGroup(void);
extern void ObserveInitializeCoroutine0ForIndexedStateGroup(u32 entry_point);
#define CAPTURE_INDEXED_STATE_GROUP_R15() ObserveCaptureInheritedR15ForIndexedStateGroup()
#define CLEAR_INDEXED_STATE_GROUP_BIT0018() ObserveClearBit0018ForIndexedStateGroup()
#define INITIALIZE_INDEXED_STATE_GROUP_COROUTINE0(entry) ObserveInitializeCoroutine0ForIndexedStateGroup(entry)
#else
extern u8 PLEXTOR_NEAR_FUNCTION CaptureInheritedR15ForIndexedStateGroup(void);
extern void ClearBit0018Atomic(void);
#define CAPTURE_INDEXED_STATE_GROUP_R15() CaptureInheritedR15ForIndexedStateGroup()
#define CLEAR_INDEXED_STATE_GROUP_BIT0018() ClearBit0018Atomic()
#define INITIALIZE_INDEXED_STATE_GROUP_COROUTINE0(entry) InitializeCoroutine0(entry)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveIndexedStateDispatchActivity(u8 state_code);
extern u8 ObserveIndexedStateHandlerCall(u8 state_index);
extern u16 ObserveIndexedState04Random(void);
extern u8 ObserveIndexedState04Track(s32 frame_position);
extern void ObserveIndexedState04Restart(u32 entry_point);
extern u8 ObserveIndexedStateTimingWindow(u8 enabled);
static u8 TestIndexedStateDispatchActivity(u8 state_code)
{
    return ObserveIndexedStateDispatchActivity(state_code);
}
static u8 CallIndexedStateHandler(u8 state_index)
{
    return ObserveIndexedStateHandlerCall(state_index);
}
#define State04Random() ObserveIndexedState04Random()
#define State04Track(position) ObserveIndexedState04Track(position)
#define State04Restart(entry) ObserveIndexedState04Restart(entry)
#define IndexedStateTimingWindow(enabled) ObserveIndexedStateTimingWindow(enabled)
#else
extern void RestartCoroutine1AndYield(u32 entry_point);
extern u8 IsControllerReadPathWithinOperatingWindow(u8 enabled);
static u8 TestIndexedStateDispatchActivity(u8 state_code)
{
    return IsIndexedDriveStateActive(state_code);
}
static u8 CallIndexedStateHandler(u8 state_index)
{
    switch(state_index)
    {
    case 0x00u:
        return HandleIndexedState00();
    case 0x01u:
        return HandleIndexedState01();
    case 0x02u:
        return HandleIndexedState02();
    case 0x03u:
        return HandleIndexedState03();
    case 0x04u:
        return HandleIndexedState04();
    case 0x05u:
        return HandleIndexedState05();
    case 0x06u:
        return HandleIndexedState06();
    case 0x07u:
        return HandleIndexedState07();
    case 0x08u:
        return HandleIndexedState08();
    case 0x09u:
        return HandleIndexedState09();
    case 0x0au:
        return HandleIndexedState0A();
    case 0x0bu:
        return HandleIndexedState0B();
    case 0x0cu:
        return HandleIndexedState0C();
    case 0x0du:
        return HandleIndexedState0D();
    case 0x0eu:
        return HandleIndexedState0E();
    case 0x0fu:
        return HandleIndexedState0F();
    case 0x10u:
        return HandleIndexedState10();
    case 0x11u:
        return HandleIndexedState11();
    case 0x12u:
        return HandleIndexedState12();
    case 0x13u:
        return HandleIndexedState13();
    case 0x14u:
        return HandleIndexedState14();
    case 0x15u:
        return HandleIndexedState15();
    case 0x16u:
        return HandleIndexedState16();
    case 0x17u:
        return HandleIndexedState17();
    case 0x18u:
        return HandleIndexedState18();
    case 0x19u:
        return HandleIndexedState19();
    case 0x1au:
        return HandleIndexedState1A();
    case 0x1bu:
        return HandleIndexedState1B();
    case 0x1cu:
        return HandleIndexedState1C();
    case 0x1du:
        return HandleIndexedState1D();
    case 0x1eu:
        return HandleIndexedState1E();
    case 0x1fu:
        return HandleIndexedState1F();
    default:
        return 0u;
    }
}
#define State04Random() NextPseudoRandom15Bit()
#define State04Track(position) FindTrackNumberForFramePosition(position)
#define State04Restart(entry) RestartCoroutine1AndYield(entry)
#define IndexedStateTimingWindow(enabled) IsControllerReadPathWithinOperatingWindow(enabled)
#endif
#endif
