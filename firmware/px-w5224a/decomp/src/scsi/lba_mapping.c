#include "lba_mapping.h"
#include "../platform/firmware_memory.h"
#include "track_packet_lba_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/lba_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ReadBigEndianS32(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* RAM:fd3b0d */
s32 LbaMappingFunction(s32 logical_block_address)
{
    ClearLbaMappingAlternateBit();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u)
        return MapThroughTrackPacketLayout(logical_block_address);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
    {
        if(logical_block_address < 0l)
        {
            SetLbaMappingAlternateBit();
            return MapNegativeThroughThirtyTwoPlusSevenLayout(logical_block_address);
        }
        return MapThroughTrackPacketLayout(logical_block_address);
    }

    if(logical_block_address < 0l || logical_block_address > ReadBigEndianS32(FW_HUGE_UNKNOWN_2BA529))
        return MapThroughTrackPacketLayout(logical_block_address);
    return MapThroughAlternateRecordLayout(logical_block_address);
}
