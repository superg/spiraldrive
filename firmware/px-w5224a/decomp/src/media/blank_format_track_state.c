#include "blank_format_track_state.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/blank_format_track_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBlankTrackU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBlankTrackU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void InitializeEmptyBlankTrackState(void)
{
    BLANK_TRACK_RESET_TABLE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, 0x41u);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD3D, 0ul);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD41, 0ul);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD65, 0ul);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD69, ReadBlankTrackU32(FW_NEAR_UNKNOWN_114F));
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD6D, 0ul);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD71, 0ul);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD75, 0ul);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD7A, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, 1u);
}

static void SetBlankTrackSessionBounds(void)
{
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD69, ReadBlankTrackU32(FW_NEAR_UNKNOWN_E612));
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD6D, ReadBlankTrackU32(FW_NEAR_UNKNOWN_E616));
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD71, 0ul);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD75, 0ul);
}

static void ResetBlankTrackTailForSession(u8 session)
{
    u8 reference = BLANK_TRACK_FIND_ENTRY(session);
    u8 first_reset;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, reference);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, reference);
    first_reset = BLANK_TRACK_FIND_ENTRY(session);
    BLANK_TRACK_RESET_RECORDS(first_reset);
}

static void UpdateBlankTrackType2(void)
{
    u8 selected = FirmwareReadNear8(FW_NEAR_UNKNOWN_E601);

    BLANK_TRACK_LOAD(selected);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) == selected)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 1u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 3u);
            WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD45, ReadBlankTrackU32(FW_NEAR_UNKNOWN_DD3D) - 5ul);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 2u);
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) - 1u));
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 3u);
        WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD45, (u32)BLANK_TRACK_CONVERT((fw_addr_t)((u16)FW_NEAR_UNKNOWN_DD81 + (u16)selected * 19u)) - 5ul);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 2u);
    }
}

static void UpdateBlankTrackType3(void)
{
    u8 high = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    fw_addr_t type_address = (fw_addr_t)((u16)FW_NEAR_UNKNOWN_DD87 + (u16)high * 19u);

    if(FirmwareRead8(type_address) != 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, (u8)(high - 1u));
    BLANK_TRACK_CLEAR_RECORD(BLANK_TRACK_LAST_TRACK());
}

static void UpdateBlankTrackType4(void)
{
    BLANK_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_E601));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 3u);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD45, ReadBlankTrackU32(FW_NEAR_UNKNOWN_E602) + 0x91ul);
}

static void UpdateBlankTrackType5(void)
{
    u8 session = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64) - 1u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, session);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 0u);
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD69, ReadBlankTrackU32(FW_NEAR_UNKNOWN_E612));
    WriteBlankTrackU32(FW_NEAR_UNKNOWN_DD6D, ReadBlankTrackU32(FW_NEAR_UNKNOWN_E616));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, BLANK_TRACK_FIND_ENTRY(session));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, (u8)(BLANK_TRACK_FIND_LAST(session) + 1u));
}

static void UpdateBlankTrackType6(void)
{
    u8 session;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 0u);
        session = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64);
        if(session == 1u)
        {
            InitializeEmptyBlankTrackState();
        }
        else
        {
            SetBlankTrackSessionBounds();
            ResetBlankTrackTailForSession(session);
        }
        return;
    }

    if(BLANK_TRACK_TAIL_UNINITIALIZED() != 0u)
    {
        session = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64);
        if(session == 2u)
        {
            InitializeEmptyBlankTrackState();
        }
        else
        {
            session = (u8)(session - 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, session);
            SetBlankTrackSessionBounds();
            ResetBlankTrackTailForSession(session);
        }
    }
    else
    {
        u8 reference = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E);
        BLANK_TRACK_RESET_RECORDS(reference);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, reference);
    }
}

/* RAM:fc166b */
void UpdateTrackStateAfterBlankOrFormatOperation(void)
{
    u8 operation;

    BLANK_TRACK_CLEAR_WORKING();
    operation = FirmwareReadNear8(FW_NEAR_UNKNOWN_E600);
    if(operation == 0u || operation == 1u)
        InitializeEmptyBlankTrackState();
    else if(operation == 2u)
        UpdateBlankTrackType2();
    else if(operation == 3u)
        UpdateBlankTrackType3();
    else if(operation == 4u)
        UpdateBlankTrackType4();
    else if(operation == 5u)
        UpdateBlankTrackType5();
    else if(operation == 6u)
        UpdateBlankTrackType6();

    BLANK_TRACK_COMMIT();
    BLANK_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F));
}
