#include "pma_q_record_reset.h"
#include "../platform/firmware_memory.h"

/* RAM:fb8c0e */
void ResetPmaQRecordTablesFromTrackIndex(u8 first_track_index)
{
    u8 record_index;
    u8 byte_index;
    fw_addr_t record_address;

    for(record_index = first_track_index; record_index < 100u; ++record_index)
    {
        record_address = FW_HUGE_UNKNOWN_2BB03B + (fw_addr_t)record_index * 10ul;
        for(byte_index = 0u; byte_index < 10u; ++byte_index)
            FirmwareWrite8(record_address + byte_index, 0u);
    }

    for(byte_index = 0u; byte_index < 10u; ++byte_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BB5DB + byte_index, 0u);

    for(record_index = 0u; record_index < 22u; ++record_index)
    {
        for(byte_index = 0u; byte_index < 10u; ++byte_index)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BB423 + (fw_addr_t)record_index * 10ul + byte_index, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BB4FF + (fw_addr_t)record_index * 10ul + byte_index, 0u);
        }
    }

    for(record_index = 0u; record_index < 41u; ++record_index)
    {
        for(byte_index = 0u; byte_index < 10u; ++byte_index)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BB5E5 + (fw_addr_t)record_index * 10ul + byte_index, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BB77F + (fw_addr_t)record_index * 10ul + byte_index, 0u);
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
    for(byte_index = 0u; byte_index < 4u; ++byte_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1143 + byte_index, 0u);
    for(byte_index = 0u; byte_index < 8u; ++byte_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147 + byte_index, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, 0xffu);
}
