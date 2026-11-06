#include "read_acquisition_request.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/read_acquisition_request_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/read_acquisition_request_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadNativeDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteNativeDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fb23c8 */
void PrepareAndDispatchReadOrSeekAcquisitionAtLba(s32 requested_lba)
{
    s32 target = (s32)((u32)requested_lba + 150ul);
    s32 disc_end;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
    disc_end = (s32)ReadNativeDword(FW_NEAR_UNKNOWN_12C1);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if((s32)((u32)disc_end + 75ul) < target)
            goto invalid_target;
    }
    else if((s32)ReadNativeDword(FW_NEAR_CURRENT_FRAME_POSITION) < disc_end)
    {
        if((s32)((u32)disc_end + 75ul) < target)
            goto invalid_target;
    }
    else if((s32)ReadNativeDword(FW_NEAR_CURRENT_FRAME_POSITION) < target)
    {
        goto invalid_target;
    }

    if(target < 1)
        target = 1;

    if((((target < (s32)ReadNativeDword(FW_NEAR_UNKNOWN_103D)) || ((s32)((u32)ReadNativeDword(FW_NEAR_UNKNOWN_103D) + 10ul) < target))
           && (((s32)ReadNativeDword(FW_NEAR_UNKNOWN_103D) < target) || ((s32)((u32)target + 3ul) < (s32)ReadNativeDword(FW_NEAR_UNKNOWN_103D))))
        || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
        WriteNativeDword(FW_NEAR_UNKNOWN_0C8E, (u32)target);
        READ_OR_SEEK_DISPATCH_AND_WAIT();
    }
    else
    {
        READ_OR_SEEK_RESET_QUEUE();
        READ_OR_SEEK_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        READ_OR_SEEK_SET_NO_SENSE();
    }
    return;

invalid_target:
    READ_OR_SEEK_SET_SENSE(5u, 0x2100u);
    READ_OR_SEEK_SET_INFORMATION((u32)requested_lba);
}

/* RAM:fdccc1 */
u8 RequestReadAcquisitionAndWait(s32 base_target, s32 signed_adjustment)
{
    s32 target = (s32)((u32)base_target + (u32)signed_adjustment);

    if(target <= 0 && READ_BIT_001B() == 0u && READ_BIT_0021() == 0u)
        target = 1;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 1u);
    WriteNativeDword(FW_NEAR_UNKNOWN_0C8E, (u32)target);
    DISPATCH_READ_ACQUISITION_REQUEST();
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u)
        YIELD_COROUTINE2();
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41);
}
