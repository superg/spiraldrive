#include "coroutine_entries.h"
#include "firmware_memory.h"

extern void YieldCoroutine0(void);
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/coroutine_entries_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern void InitializeCoroutine1(u32 entry_point);

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/coroutine_entries_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadCoroutineEntryBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fbfbb9 */
/* ERET after idle handoff. */
void RunIndexedState04MediaReadCoroutine0(void)
{
    u32 lead_out;
    u32 first_start;
    s8 acquisition_result;
    u8 saved_flag_bit6;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    ClearIndexedState04P3Bit0();
    SetIndexedState04P3Bit1();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    saved_flag_bit6 = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xbfu));

    lead_out = ReadCoroutineEntryBigEndianU32(FW_NEAR_CURRENT_FRAME_POSITION);
    first_start = ((u32)(u16)((u16)(lead_out >> 16) - 1u) << 16) | (lead_out & 0x0000fffful);
    acquisition_result = AcquireIndexedState04PatternRange((s32)(first_start - 0x00003cfeul), 10000ul);
    if(acquisition_result < 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x3cu);
    }
    else
    {
        lead_out = ReadCoroutineEntryBigEndianU32(FW_NEAR_CURRENT_FRAME_POSITION);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD6) & 0x0fu) == 0u)
            acquisition_result = AcquireIndexedState04PatternRange((s32)(lead_out - 0x00006a0eul), 0x57e4ul);
        else
            acquisition_result = AcquireIndexedState04PatternRange((s32)0x00048792ul, 0x57e4ul);
        if(acquisition_result < 0)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x3eu);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 0u);
    do
    {
        YieldIndexedState04Coroutine0();
    } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C3) == 0u);

    SetIndexedState04P3Bit0();
    SetIndexedState04P3Bit1();
    if(acquisition_result < 0)
    {
        InitializeCoroutine1(FW_ENTRY_PUBLISH_READ_SETUP_FAILURE_COROUTINE1);
    }
    else
    {
        SetIndexedState04P3Bit0();
        ClearIndexedState04P3Bit1();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xbfu) | saved_flag_bit6));
    YieldCoroutine0Forever();
}

/* RAM:fbc9e0 */
void RunMediaReadControlCoroutine3(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9A, 0u);
    FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x10u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        u8 selected_read_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, selected_read_speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        if(selected_read_speed <= 8u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1591, 0u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
    }

    YieldMediaReadControlCoroutine3();
    if(IsMediaReadControlSnapshotValid() != 0u)
        ClearMediaReadControlSnapshot();
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        ResetMediaReadControlType2Pipeline();
    else
        InitializeMediaReadControlTrackModel();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x02u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) | 0x02u));
        UpdateMediaReadControlTrackFlag();
        if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x08u) != 0u)
            FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xf7u));
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x80u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1584, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1584) | 1u));
        FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) & 0xfbu));
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
            InitializeMediaReadControlAcquisition();
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    }

    CopyMediaReadControlBytes(FW_NEAR_UNKNOWN_FAD3, 0xe4f2u, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xbfu));
    for(;;)
        YieldMediaReadControlCoroutine3();
}

/* RAM:f84a00 */
void InitializeThenIdleCoroutine2(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1591, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4A, 0u);
    ResetRawReadQueueForCoroutine2Entry();

    for(;;)
        YieldCoroutine2ForIdleEntry();
}

/* RAM:fdc225 */
void YieldCoroutine0Forever(void)
{
    for(;;)
        YieldCoroutine0();
}

/* RAM:f84b00 */
void IdleCoroutine3(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9A, 0u);

    for(;;)
        YieldCoroutine3ForIdleEntry();
}

/* RAM:fb7ffa */
void LoadControllerBufferSetupCoroutineTail(void)
{
    LOAD_CONTROLLER_BUFFER_SETUP_FROM_CURRENT_DESCRIPTOR();
}

/* RAM:f92815 */
void PLEXTOR_NEAR_FUNCTION FinalizeRawReadPipelineAndIdleCoroutine2(void)
{
    InitializeControllerBufferCursorForRawReadFinalizer();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F3B, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F10, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F11, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F12, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F13, 0xf6u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
    }
    else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45) > 0)
    {
        u8 speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, speed);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F48, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F46, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F07, 0u);
    ClearRawReadFinalizerBitsAtomic();

    if(ReadBit0005ForRawReadFinalizer() != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) | 0x04u));
        DispatchReadAcquisitionRequest5ForRawReadFinalizer();
        YieldCoroutine2ForIdleEntry();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u)
            YieldCoroutine2ForIdleEntry();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
    }
    else
    {
        DispatchReadAcquisitionRequest5ForRawReadFinalizer();
    }

    for(;;)
        YieldCoroutine2ForIdleEntry();
}
