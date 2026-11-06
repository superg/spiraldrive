#include "write_buffer_snapshot_validation.h"
#include "../platform/controller_buffer_atomic.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

static u8 SnapshotBytesMatch(fw_addr_t buffer_offset, fw_addr_t firmware_address, u16 byte_count)
{
    u16 index;

    for(index = 0u; index < byte_count; ++index)
    {
        u8 snapshot_byte = ReadControllerBufferByteAtomic(buffer_offset);

        buffer_offset = FirmwareAddressIncrement24(buffer_offset, 1u);
        if(FirmwareRead8(firmware_address) != snapshot_byte)
            return 0u;
        firmware_address = FirmwareAddressIncrement24(firmware_address, 1u);
    }
    return 1u;
}

/* RAM:fd56b6 */
u8 ValidateWriteBufferSnapshot(u32 expected_length)
{
    u16 staged_length;
    u16 staged_checksum;

    staged_length = (u16)((u16)ReadControllerBufferByteAtomic(0u) << 8);
    staged_length = (u16)(staged_length | ReadControllerBufferByteAtomic(1u));
    if((u32)staged_length != expected_length)
        return 0u;
    if(staged_length != 0x28e2u)
        return 0u;
    if(SnapshotBytesMatch(2u, FW_NEAR_UNKNOWN_DCCD, 8u) == 0u)
        return 0u;
    if(SnapshotBytesMatch(10u, FW_NEAR_UNKNOWN_DCD5, 0x10u) == 0u)
        return 0u;

    staged_checksum = (u16)((u16)ReadControllerBufferByteAtomic(0x28e0u) << 8);
    staged_checksum = (u16)(staged_checksum | ReadControllerBufferByteAtomic(0x28e1u));
    if(staged_checksum != CalculateControllerBufferChecksum28E0(0u))
        return 0u;

    SetWriteBufferSnapshotValid();
    return 1u;
}
