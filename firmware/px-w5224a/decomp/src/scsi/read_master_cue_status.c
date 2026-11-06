#include "read_master_cue_status.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_master_cue_status_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra addresses. */
static u16 ReadReadMasterCueWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteReadMasterCueWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadReadMasterCueDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fc64f2 */
u8 PLEXTOR_NEAR_FUNCTION PollReadMasterCueRawReadStatus(void)
{
    u8 status;
    u16 descriptor_address;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0u;

    status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    if(status != 0u)
    {
        u8 sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        u16 sense_code = ReadReadMasterCueWord(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (fw_addr_t)((u16)sense_index * 2u));

        sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        SET_READ_MASTER_CUE_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + sense_index), sense_code);
        return 0u;
    }

    SET_READ_MASTER_CUE_SENSE(2u, 0x3000u);
    descriptor_address = GET_LATEST_READ_MASTER_CUE_DESCRIPTOR();
    if(descriptor_address == 0u)
        return 0xfeu;

    if(FirmwareRead8((fw_addr_t)(descriptor_address + 0x13u)) == 9u)
    {
        u16 descriptor_index = 0u;

        while(descriptor_index < ReadReadMasterCueWord(FW_NEAR_UNKNOWN_1292))
        {
            u16 secondary_address = GET_READ_MASTER_CUE_SECONDARY_DESCRIPTOR();

            if(FirmwareRead8((fw_addr_t)(secondary_address + 0x14u)) >= 0xaau && FirmwareRead8((fw_addr_t)(secondary_address + 0x14u)) <= 0xacu)
                return 1u;

            WriteReadMasterCueWord(FW_NEAR_UNKNOWN_1298, (u16)((ReadReadMasterCueWord(FW_NEAR_UNKNOWN_1298) + 1u) & ReadReadMasterCueWord(FW_NEAR_UNKNOWN_129A)));
            descriptor_index++;
        }
        SET_READ_MASTER_CUE_SENSE(5u, 0x6400u);
        return 0u;
    }

    if(VALIDATE_READ_MASTER_CUE_DESCRIPTOR(descriptor_address) == 0u)
    {
        u16 status_address = ReadReadMasterCueWord((fw_addr_t)(descriptor_address + 0x0eu));

        SET_READ_MASTER_CUE_SENSE(FirmwareRead8((fw_addr_t)(status_address + 0x10u)), descriptor_address);
        return 0u;
    }

    return READ_MASTER_CUE_QUEUE_HIGH_WATER() != 0u ? 0u : 0xfeu;
}

extern u8 CompareReadWindowBounds(u32 upper_bound, u32 lower_bound);

/* RAM:fc65c3 */
u8 PLEXTOR_NEAR_FUNCTION IsReadWindowOutsideAcceptedRange(void)
{
    u32 upper_bound;
    u32 lower_bound;

    if((s32)ReadReadMasterCueDword(FW_NEAR_UNKNOWN_0B02) > 150l)
        return 1u;
    if((s32)ReadReadMasterCueDword(FW_NEAR_UNKNOWN_0B02) < 0l)
        return 1u;

    upper_bound = ReadReadMasterCueDword(FW_NEAR_UNKNOWN_0B02) + 151ul;
    lower_bound = ReadReadMasterCueDword(FW_NEAR_UNKNOWN_0B02) + 150ul;
    return CompareReadWindowBounds(upper_bound, lower_bound) == 2u ? 0u : 1u;
}
