#include "read_disc_information.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "read_disc_information_recordable.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_disc_information_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadDiscInformationExtent(void)
{
    return (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_E74B) * 8u + 0x22u);
}

/* RAM:fae02a */
void HandleReadDiscInformationCommand(void)
{
    u16 allocation_length;
    u8 response_offset;

    if(VALIDATE_READ_DISC_INFORMATION(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CEA)) == 0u)
    {
        HANDLE_UNSUPPORTED_READ_DISC_INFORMATION();
        return;
    }

    allocation_length = ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11);
    response_offset = 0u;
    while((u16)response_offset <= ReadDiscInformationExtent())
    {
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_0B2A + response_offset), 0u);
        response_offset++;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        BUILD_BASIC_READ_DISC_INFORMATION();
    else if(BUILD_RECORDABLE_READ_DISC_INFORMATION() == 0u)
        return;

    if(allocation_length == 0u)
    {
        SET_NO_SENSE_FOR_READ_DISC_INFORMATION();
        return;
    }

    allocation_length = MinU16(allocation_length, ReadDiscInformationExtent());
    if(TRANSFER_READ_DISC_INFORMATION(FW_NEAR_UNKNOWN_0B2A, (u32)allocation_length) != 4u)
        SET_NO_SENSE_FOR_READ_DISC_INFORMATION();
}
