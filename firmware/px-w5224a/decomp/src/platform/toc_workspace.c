#include "toc_workspace.h"
#include "firmware_memory.h"

/* RAM:fad958 */
void PLEXTOR_NEAR_FUNCTION ResetDecodedTocSummaryTables(void)
{
    u16 index;
    fw_addr_t record;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1C, 0u);

    for(index = 0u; index <= 100u; ++index)
    {
        record = FW_NEAR_UNKNOWN_F2B6 + (fw_addr_t)(index * 6u);
        FirmwareWrite8(record, 0u);
        FirmwareWrite8(record + 1u, 0u);
        FirmwareWrite8(record + 2u, 0u);
        FirmwareWrite8(record + 5u, 0u);
        FirmwareWrite8(record + 4u, 0u);
        FirmwareWrite8(record + 3u, 0u);
    }

    for(index = 0u; index <= 40u; ++index)
    {
        record = FW_NEAR_UNKNOWN_F7C4 + (fw_addr_t)(index * 10u);
        FirmwareWrite8(record + 2u, 0u);
        FirmwareWrite8(record + 1u, 0u);
        FirmwareWrite8(record, 0u);
        FirmwareWrite8(record + 5u, 0u);
        FirmwareWrite8(record + 4u, 0u);
        FirmwareWrite8(record + 3u, 0u);
        FirmwareWrite8(record + 8u, 0u);
        FirmwareWrite8(record + 7u, 0u);
        FirmwareWrite8(record + 6u, 0u);
        FirmwareWrite8(record + 9u, 0u);
    }

    for(index = 0u; index <= 3u; ++index)
    {
        record = FW_NEAR_UNKNOWN_F79C + (fw_addr_t)(index * 10u);
        FirmwareWrite8(record + 2u, 0u);
        FirmwareWrite8(record + 1u, 0u);
        FirmwareWrite8(record, 0u);
        FirmwareWrite8(record + 5u, 0u);
        FirmwareWrite8(record + 4u, 0u);
        FirmwareWrite8(record + 3u, 0u);
        FirmwareWrite8(record + 8u, 0u);
        FirmwareWrite8(record + 7u, 0u);
        FirmwareWrite8(record + 6u, 0u);
        FirmwareWrite8(record + 9u, 0u);
    }
}

/* RAM:fad287 */
void ResetFullTocDescriptorWorkspace(void)
{
    static const u8 field_order[11] = { 0u, 3u, 2u, 1u, 6u, 5u, 4u, 7u, 10u, 9u, 8u };
    u16 descriptor_index;
    u8 field_index;
    fw_addr_t descriptor;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, 0u);
    ResetDecodedTocSummaryTables();
    for(descriptor_index = 1u; descriptor_index < 500u; ++descriptor_index)
    {
        descriptor = FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)(descriptor_index * 11u);
        for(field_index = 0u; field_index < 11u; ++field_index)
            FirmwareWrite8(descriptor + field_order[field_index], 0u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12DC, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12DD, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12CA, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12CB, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C4, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x40u));
}
