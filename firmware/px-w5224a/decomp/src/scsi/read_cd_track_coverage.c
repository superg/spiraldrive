#include "read_cd_track_coverage.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_cd_track_coverage_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PLEXTOR_NEAR_FUNCTION ClearBit0010ForReadCdCoverage(void);

static s32 ReadCoverageS32(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static void WriteCoverageS32(fw_addr_t address, s32 value)
{
    u32 raw_value = (u32)value;

    FirmwareWrite8(address, (u8)(raw_value >> 24));
    FirmwareWrite8(address + 1u, (u8)(raw_value >> 16));
    FirmwareWrite8(address + 2u, (u8)(raw_value >> 8));
    FirmwareWrite8(address + 3u, (u8)raw_value);
}

static u8 MatchCoveragePosition(s32 frame_position, u8 first_track, u8 last_track, u16 frame_bias)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_cd_track_coverage_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:fd365b */
u8 ClassifyReadCdFrameRangeTrackCoverage(s32 start_frame_position, s32 end_frame_position)
{
    u8 first_track;
    u8 last_track;
    u8 current_track;

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        return 1u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) != 3u)
        ClearBit0010ForReadCdCoverage();

    first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x13u) == 1u)
            return 2u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1147) != 0xffu && FirmwareReadNear8(FW_NEAR_UNKNOWN_1148) >= FirmwareReadNear8(FW_NEAR_UNKNOWN_1147))
    {
        first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
        last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1148);
    }

    current_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_15E9);
    if(ReadCoverageS32(FW_NEAR_UNKNOWN_15E5) <= start_frame_position && first_track <= current_track && current_track <= last_track)
        first_track = current_track;

    WriteCoverageS32(FW_NEAR_UNKNOWN_15E5, start_frame_position);
    if(MatchCoveragePosition(start_frame_position, first_track, last_track, 0x0096u) != 0u)
        return 2u;

    (void)MatchCoveragePosition(end_frame_position, first_track, last_track, 0x0095u);
    return 1u;
}
