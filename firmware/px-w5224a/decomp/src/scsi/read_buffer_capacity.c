#include "read_buffer_capacity.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_buffer_capacity_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra addresses. */
static u16 ReadNativeWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteNativeDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fafd77 */
void HandleReadBufferCapacityCommand(void)
{
    u16 high_water_count;
    u16 available_count;
    u16 allocation_length;

    if(VALIDATE_READ_BUFFER_CAPACITY(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D16)) == 0u)
    {
        HANDLE_UNSUPPORTED_READ_BUFFER_CAPACITY();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 10u);

    high_water_count = ReadNativeWord(FW_NEAR_UNKNOWN_129C);
    WriteNativeDword(FW_NEAR_UNKNOWN_0B2E, (u32)high_water_count * 0x0930ul);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) == 0u)
    {
        available_count = ReadNativeWord(FW_NEAR_UNKNOWN_129C);
    }
    else
    {
        u16 queued_count = ReadNativeWord(FW_NEAR_UNKNOWN_1292);

        if(queued_count > ReadNativeWord(FW_NEAR_UNKNOWN_129C))
        {
            available_count = 0u;
        }
        else
        {
            u16 current_high_water = ReadNativeWord(FW_NEAR_UNKNOWN_129C);
            u16 current_queued = ReadNativeWord(FW_NEAR_UNKNOWN_1292);

            available_count = (u16)(current_high_water - current_queued);
        }
    }
    WriteNativeDword(FW_NEAR_UNKNOWN_0B32, (u32)available_count * 0x0930ul);

    allocation_length = MinU16(ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11), 12u);
    if(TRANSFER_READ_BUFFER_CAPACITY_RESPONSE(FW_NEAR_UNKNOWN_0B2A, (u32)allocation_length) != 4u)
        SET_NO_SENSE_FOR_READ_BUFFER_CAPACITY();
}
