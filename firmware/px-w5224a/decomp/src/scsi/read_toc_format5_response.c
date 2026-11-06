#include "read_toc_format5_response.h"
#include "../platform/firmware_memory.h"

extern u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source_address, u32 requested_count);
extern void SetSenseData(u8 sense_key, u16 asc_ascq);

/* RAM:fd79bd */
void PLEXTOR_NEAR_FUNCTION ReturnEmptyCdTextResponse(u16 requested_count)
{
    u8 result;

    if(requested_count > 4u)
        requested_count = 4u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);

    result = CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, requested_count);
    if(result != 4u)
        SetSenseData(0u, 0u);
}
