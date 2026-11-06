#include "arithmetic.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/indexed_state_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define DEFINE_INACTIVE_INDEXED_STATE_HANDLER(name, address)                                                                                                                                           \
    u8 name(void)                                                                                                                                                                                      \
    {                                                                                                                                                                                                  \
        return 0u;                                                                                                                                                                                     \
    }

/* RAM:fd7aa4 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState01, fd7aa4)
/* RAM:fd7aa6 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState02, fd7aa6)
/* RAM:fd7aa8 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState03, fd7aa8)
/* RAM:fd7bed */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState05, fd7bed)
/* RAM:fd7bef */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState06, fd7bef)
/* RAM:fd7bf1 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState07, fd7bf1)
/* RAM:fd7cff */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState08, fd7cff)
/* RAM:fd7de4 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState0B, fd7de4)
/* RAM:fd7e4e */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState0D, fd7e4e)
/* RAM:fd7e50 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState0E, fd7e50)
/* RAM:fd7e52 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState0F, fd7e52)
/* RAM:fd7e54 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState10, fd7e54)
/* RAM:fd7e56 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState11, fd7e56)
/* RAM:fd7e58 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState12, fd7e58)
/* RAM:fd7e5a */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState13, fd7e5a)
/* RAM:fd7e5c */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState14, fd7e5c)
/* RAM:fd7e5e */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState15, fd7e5e)
/* RAM:fd7e60 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState16, fd7e60)
/* RAM:fd7e72 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState17, fd7e72)
/* RAM:fd7e62 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState18, fd7e62)
/* RAM:fd7e64 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState19, fd7e64)
/* RAM:fd7e66 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState1A, fd7e66)
/* RAM:fd7e68 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState1B, fd7e68)
/* RAM:fd7e6a */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState1C, fd7e6a)
/* RAM:fd7e6c */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState1D, fd7e6c)
/* RAM:fd7e6e */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState1E, fd7e6e)
/* RAM:fd7e70 */
DEFINE_INACTIVE_INDEXED_STATE_HANDLER(HandleIndexedState1F, fd7e70)

#undef DEFINE_INACTIVE_INDEXED_STATE_HANDLER

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/indexed_state_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadState04Word(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteState04Word(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fd7bf3 */
u8 PLEXTOR_NEAR_FUNCTION ProcessIndexedStateGroup(u8 terminal_handoff_flag)
{
    u8 selected_track = CAPTURE_INDEXED_STATE_GROUP_R15();
    u8 layout = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    u8 cursor;
    u32 target_position = 0ul;
    u8 write_target_position = 1u;

    if(layout == 0x0fu)
    {
        cursor = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);
        if(cursor == 1u)
        {
            selected_track = 0x0fu;
            target_position = 0x0002df5ful;
        }
        else if(terminal_handoff_flag == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            return 1u;
        }
        else
        {
            selected_track = 1u;
            target_position = 0x00000096ul;
        }
    }
    else if(layout == 0x10u)
    {
        cursor = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);
        if(cursor == 1u)
        {
            selected_track = 0x0du;
            target_position = 0x00030525ul;
        }
        else if(terminal_handoff_flag == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            return 1u;
        }
        else
        {
            selected_track = 1u;
            target_position = 0x00000096ul;
        }
    }
    else if(layout == 0x18u)
    {
        cursor = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);
        if(cursor == 0x0fu)
        {
            selected_track = 0x13u;
            target_position = 0x00031eb1ul;
        }
        else if(cursor == 0x13u)
        {
            if(terminal_handoff_flag == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
                return 1u;
            }
            selected_track = 9u;
            target_position = 0x00016839ul;
        }
        else if(cursor == 9u)
        {
            selected_track = 0x0fu;
            target_position = 0x000281bful;
        }
        else
        {
            selected_track = 9u;
            target_position = 0x00016839ul;
        }
    }
    else
    {
        write_target_position = 0u;
    }

    if(write_target_position != 0u)
        WriteState04Word(0xfbf8u, target_position);
    WriteState04Word(FW_NEAR_UNKNOWN_0F36, 0x000002eeul);
    CLEAR_INDEXED_STATE_GROUP_BIT0018();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, selected_track);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, selected_track);
    INITIALIZE_INDEXED_STATE_GROUP_COROUTINE0(FW_ENTRY_RUN_CONFIGURED_RAW_SECTOR_READ_COROUTINE0);
    return 0u;
}

/* RAM:fd79fd */
u8 DispatchIndexedState(void)
{
    u8 mode;

    if(TestIndexedStateDispatchActivity(6u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBE, 2u);
    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1);
    if(mode != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBE, 0u);
    if(mode >= 3u)
        return CallIndexedStateHandler(4u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 1u)
        return 0u;
    return CallIndexedStateHandler(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A));
}

/* RAM:fd7aaa */
u8 HandleIndexedState04(void)
{
    u8 state = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1);
    u32 scaled_source;
    u32 target;

    if(state < 3u || (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_130B) > 0)
        return 0u;

    do
    {
        scaled_source = (u32)(State04Random() >> 2);
        target = MultiplyU32Low(ReadState04Word(FW_NEAR_UNKNOWN_12C1), scaled_source) >> 13;
        target += scaled_source & 0x3ful;
        WriteState04Word(FW_NEAR_UNKNOWN_0E3B, target);
    } while(target < 150ul || target >= ReadState04Word(FW_NEAR_UNKNOWN_12C1) - 3750ul);

    WriteState04Word(FW_NEAR_UNKNOWN_0E3B, target);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, State04Track((s32)target));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0x67u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        State04Restart(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) + 1u));
    }
    return 1u;
}

/* RAM:fd7e74 */
u8 ProcessIndexedStateTiming(void)
{
    u32 upper_bound;
    u32 scaled_source;
    u32 target;

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1202) <= 0)
    {
        switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47))
        {
        case 1u:
            upper_bound = ReadState04Word(FW_NEAR_CURRENT_FRAME_POSITION) - 9000ul;
            break;
        case 2u:
            upper_bound = ReadState04Word(FW_NEAR_CURRENT_FRAME_POSITION) - 18000ul;
            break;
        case 4u:
            upper_bound = ReadState04Word(FW_NEAR_CURRENT_FRAME_POSITION) - 36000ul;
            break;
        case 8u:
            upper_bound = ReadState04Word(FW_NEAR_CURRENT_FRAME_POSITION) - 72000ul;
            break;
        default:
            for(;;)
            {
            }
        }

        target = 0ul;
        do
        {
            scaled_source = (u32)(State04Random() >> 2);
            target = MultiplyU32Low(upper_bound, scaled_source) >> 13;
            target += scaled_source & 0x3ful;
            WriteState04Word(FW_NEAR_UNKNOWN_0E3B, target);
        } while(target < 150ul || target >= upper_bound - 5ul);
        WriteState04Word(FW_NEAR_UNKNOWN_0E3B, target);
        return 1u;
    }

    if(IndexedStateTimingWindow(1u) != 0u)
        return 0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
    return 1u;
}
