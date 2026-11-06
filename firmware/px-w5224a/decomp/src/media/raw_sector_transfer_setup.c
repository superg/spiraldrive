#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "../scsi/raw_sector_range.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_sector_transfer_setup_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRawSectorTransferDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fd3311 */
void DispatchRawSectorTransferSetup(void)
{
    u32 start_frame_address;

    if(READ_PENDING_RAW_SECTOR_COMMAND() != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    start_frame_address = ReadRawSectorTransferDword(FW_NEAR_UNKNOWN_0B02) + 150ul;
    CLEAR_PENDING_RAW_SECTOR_COMMAND();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, 0u);
    CLEAR_RAW_SECTOR_BIT000E();
    CLEAR_RAW_SECTOR_BIT0018();
    SET_RAW_SECTOR_SCOPE();
    ExecuteRawSectorReadRange(start_frame_address, ReadRawSectorTransferDword(FW_NEAR_UNKNOWN_0B06) + start_frame_address, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C34));
    CLEAR_RAW_SECTOR_SCOPE();
}
