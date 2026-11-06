#include "play_cd.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

/* The native MSF worker receives end_tuple_address in DR4, then
 * start_tuple_address in DR0, followed by playback_mode in ACC. */
extern void ValidateAndStartPlayAudioMsfRangeFromCdbTuples(fw_addr_t end_tuple_address, fw_addr_t start_tuple_address, u8 playback_mode);
extern void ValidateAndStartPlayAudioFrameRange(s32 start_frame_position, s32 frame_count, u8 playback_mode);

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fd4a5d */
void HandlePlayCdCommand(void)
{
    u8 playback_mode;
    s32 start_frame_position;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D7D)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1cu) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1cu) != 4u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    playback_mode = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) & 1u) == 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) == 2u)
    {
        ValidateAndStartPlayAudioMsfRangeFromCdbTuples(FW_NEAR_UNKNOWN_0B10, FW_NEAR_UNKNOWN_0B0D, playback_mode);
        return;
    }

    start_frame_position = (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_0B0C);
    if(start_frame_position != -1l)
        start_frame_position = (s32)((u32)start_frame_position + 150ul);
    ValidateAndStartPlayAudioFrameRange(start_frame_position, (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_0B10), playback_mode);
}
