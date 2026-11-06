#include "reserve_track.h"
#include "../media/reserve_track_raw_read_pipeline.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/reserve_track_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:faf878 */
void HandleReserveTrackCommand(void)
{
    u8 track_number;
    u8 track_type;

    if(VALIDATE_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E64)) == 0u)
    {
        REJECT_COMMAND();
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        SET_SEQUENCE_ERROR();
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 3u)
    {
        SET_SEQUENCE_ERROR();
        return;
    }

    NORMALIZE_WRITE_SPEED();
    if(VALIDATE_WRITE_PARAMETERS() == 0u || VALIDATE_MEDIA() == 0u)
        return;

    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    track_type = FirmwareRead8(FW_HUGE_UNKNOWN_00DD87 + (u16)((u16)track_number * 19u));
    if(track_type == 1u)
    {
        SET_SENSE(5u, 0x7203u);
        return;
    }
    track_type = FirmwareRead8(FW_HUGE_UNKNOWN_00DD87 + (u16)((u16)track_number * 19u));
    if(track_type != 0u)
    {
        SET_SEQUENCE_ERROR();
        return;
    }

    CLEAR_WORKING_TRACK();
    if(LOAD_TRACK(track_number, (u8)(((u16)track_number * 19u) >> 8)) == 0u)
        return;
    if(VALIDATE_PMA_START() == 0u)
        return;

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC119, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC11A, 0u);
    if(VALIDATE_END_FRAME(ReadBigEndianU32(FW_NEAR_UNKNOWN_0B0F)) == 0u)
    {
        REJECT_COMMAND();
        return;
    }

    BUILD_ADR1_RECORD();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    EXECUTE_PREPARATION();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/reserve_track_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:faf946 */
void ExecuteReserveTrackRawReadPreparation(void)
{
    u8 prepared;
    u16 sense_code;

    RESET_RAW_READ_QUEUE();
    INITIALIZE_COROUTINE2();
    YIELD_COROUTINE0();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && ENSURE_OPC() == 0u)
        return;

    WriteBigEndianU32(FW_NEAR_UNKNOWN_1455, 0ul);
    prepared = PREPARE_RAW_READ_PIPELINE();
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC119, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC11A, 0u);

    if(prepared == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3A, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B24));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3B, FirmwareReadNear8(FW_NEAR_UNKNOWN_0B25));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
            return;
        sense_code = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_0C3A);
        if(sense_code != 0x0c07u && sense_code != 0x0200u)
            return;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x80u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 5u);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_DD51, ReadBigEndianU32(FW_NEAR_UNKNOWN_1484));
        WriteBigEndianU32(FW_NEAR_UNKNOWN_DD4D, ReadBigEndianU32(FW_NEAR_UNKNOWN_1484));
        WriteBigEndianU32(FW_NEAR_UNKNOWN_DD5B, 0ul);
        UPDATE_TRACK_TABLE();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 2u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 3u);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_DD45, ReadBigEndianU32(FW_NEAR_UNKNOWN_DD3D) - 5ul);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_DD5B, 0ul);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
    {
        UPDATE_TRACK_TABLE();
        SNAPSHOT_TRACK_TAIL();
    }
    CLEAR_PREPARATION_WORKING_TRACK();
}
