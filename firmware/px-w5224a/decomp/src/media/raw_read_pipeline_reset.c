#include "raw_read_pipeline_reset.h"
#include "../platform/firmware_memory.h"

static void ClearRawReadResetWord(fw_addr_t address)
{
    FirmwareWrite8(address, 0u);
    FirmwareWrite8(address + 1u, 0u);
}

static void ClearRawReadResetDword(fw_addr_t address)
{
    ClearRawReadResetWord(address);
    ClearRawReadResetWord(address + 2u);
}

/* RAM:fe2e9d */
void ResetRawReadPipelineBuffersAndState(void)
{
    u16 record_index;
    u8 byte_index;

    for(byte_index = 0u; byte_index < 10u; ++byte_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272 + byte_index, 0u);
    ClearRawReadResetDword(FW_NEAR_UNKNOWN_1262);
    ClearRawReadResetDword(FW_NEAR_UNKNOWN_1266);
    ClearRawReadResetDword(FW_NEAR_UNKNOWN_126A);
    ClearRawReadResetDword(FW_NEAR_UNKNOWN_126E);
    for(byte_index = 0u; byte_index < 18u; ++byte_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C + byte_index, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);

    ClearRawReadResetDword(FW_NEAR_UNKNOWN_8000);
    ClearRawReadResetDword(FW_NEAR_UNKNOWN_8000);
    ClearRawReadResetWord(FW_NEAR_UNKNOWN_8008);
    ClearRawReadResetWord(FW_NEAR_UNKNOWN_800A);
    ClearRawReadResetWord(FW_NEAR_UNKNOWN_800C);
    ClearRawReadResetWord(FW_NEAR_UNKNOWN_800E);
    ClearRawReadResetWord(FW_NEAR_UNKNOWN_8010);

    for(record_index = 0u; record_index < 220u; ++record_index)
    {
        fw_addr_t record_address = FW_NEAR_UNKNOWN_8012 + (fw_addr_t)(record_index * 10u);

        for(byte_index = 0u; byte_index < 10u; ++byte_index)
            FirmwareWrite8(record_address + byte_index, 0u);
    }

    for(byte_index = 0u; byte_index < 16u; ++byte_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88AA + byte_index, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CC, 0xbbu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1555, 0u);
}
