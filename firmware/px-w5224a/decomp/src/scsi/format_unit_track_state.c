#include "format_unit_track_state.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/format_unit_track_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadFormatTrackU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteFormatTrackU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void CopyFormatBoundaries(void)
{
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD69, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E612));
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD6D, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E616) + 1ul);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD71, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61A));
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD75, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E));
}

static void UpdateFormatTrackDataBlockType(u8 inherited_r7, u8 inherited_b)
{
    FORMAT_TRACK_UPDATE_DATA_BLOCK(inherited_r7, inherited_b);
}

static void MarkFormatTrackActiveWithoutExtension(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, 0u);
}

static void CreateFormatTrackExtension(u8 next_track, u8 clear_active_flag)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, next_track);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, next_track);
    if(clear_active_flag != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 0u);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD7A, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E) + 0x1194ul);
    FORMAT_TRACK_COMMIT();
    FORMAT_TRACK_CLEAR();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, next_track);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD69, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E));
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD6D, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E) + 0x1194ul);
}

static void PopulateNewFormatTrack(u8 track_number, u8 session_number, u32 start_frame, u8 track_mode, u8 inherited_r7, u8 inherited_b)
{
    UpdateFormatTrackDataBlockType(inherited_r7, inherited_b);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, (u8)((track_mode << 4) | 1u));
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD3D, start_frame);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD41, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61A));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 1u);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD57, ReadFormatTrackU32(FW_NEAR_UNKNOWN_0C4B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, session_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, session_number);
    CopyFormatBoundaries();
}

static void FinishFormatTrackRecord(u8 next_track, u8 clear_active_flag)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) == 0xc0u)
        CreateFormatTrackExtension(next_track, clear_active_flag);
    else
        MarkFormatTrackActiveWithoutExtension();
}

/* RAM:fb32e0 */
void PLEXTOR_NEAR_FUNCTION UpdateFormatUnitTrackStateAfterSuccessfulPreparation(void)
{
    u8 track_mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0xfdu);

    FORMAT_TRACK_CLEAR();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u)
        {
            FORMAT_TRACK_RESET_TABLE();
            /* ResetTrackInformationTableState exits its 1..99 record loop with
             * R7/B holding 19 * 99 and 100, which RAM:fc4e93 consumes. */
            PopulateNewFormatTrack(1u, 1u, 0x96ul, track_mode, 0xebu, 100u);
            FinishFormatTrackRecord(2u, 0u);
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u && FORMAT_TRACK_TAIL_UNINITIALIZED() != 0u)
        {
            FORMAT_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
            PopulateNewFormatTrack(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0), FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9), ReadFormatTrackU32(FW_NEAR_UNKNOWN_E616) + 0x97ul, track_mode,
                FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37), FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38));
            FinishFormatTrackRecord((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u), 1u);
        }
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
        {
            if(FORMAT_TRACK_TAIL_UNINITIALIZED() != 0u)
            {
                FORMAT_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
                WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD7A, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E) + 0x1194ul);
            }
        }
        else
        {
            FORMAT_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD45, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E));
        WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD41, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61A));
        CopyFormatBoundaries();
        FinishFormatTrackRecord((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u), 1u);
    }
    FORMAT_TRACK_SET_ACTIVE_BIT();
    FORMAT_TRACK_COMMIT();
}

static void PopulateFailedFormatTrack(u8 track_number, u8 session_number, u32 start_frame, u8 track_mode, u8 update_data_block)
{
    if(update_data_block != 0u)
        UpdateFormatTrackDataBlockType(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37), FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, track_number);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD51, start_frame);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, (u8)((track_mode << 4) | 1u));
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD3D, start_frame);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD41, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61A));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 1u);
    WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD57, ReadFormatTrackU32(FW_NEAR_UNKNOWN_0C4B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, session_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, session_number);
    CopyFormatBoundaries();
}

/* RAM:fb3598 */
void PLEXTOR_NEAR_FUNCTION UpdateFormatUnitTrackStateAfterFailedPreparation(void)
{
    u8 track_mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0xfdu);

    FORMAT_TRACK_CLEAR();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u)
        {
            FORMAT_TRACK_RESET_TABLE();
            PopulateFailedFormatTrack(1u, 1u, 0x96ul, track_mode, 0u);
            FinishFormatTrackRecord(2u, 0u);
        }
        else
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u && FORMAT_TRACK_TAIL_UNINITIALIZED() != 0u)
            {
                FORMAT_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E));
                PopulateFailedFormatTrack(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E), FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9), ReadFormatTrackU32(FW_NEAR_UNKNOWN_E616) + 0x97ul, track_mode, 1u);
                FinishFormatTrackRecord((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u), 1u);
            }
            WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD51, ReadFormatTrackU32(FW_NEAR_UNKNOWN_DD3D));
        }
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
        {
            if(FORMAT_TRACK_TAIL_UNINITIALIZED() != 0u)
            {
                FORMAT_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
                WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD7A, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E) + 0x1194ul);
            }
        }
        else
        {
            FORMAT_TRACK_LOAD(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 5u);
        WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD51, ReadFormatTrackU32(FW_NEAR_UNKNOWN_DD3D));
        WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD45, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61E));
        WriteFormatTrackU32(FW_NEAR_UNKNOWN_DD41, ReadFormatTrackU32(FW_NEAR_UNKNOWN_E61A));
        CopyFormatBoundaries();
        FinishFormatTrackRecord((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u), 1u);
    }
    FORMAT_TRACK_SET_ACTIVE_BIT();
    FORMAT_TRACK_COMMIT();
}
