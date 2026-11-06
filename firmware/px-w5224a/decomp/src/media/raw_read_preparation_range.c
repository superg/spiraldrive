#include "raw_read_preparation_range.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_read_preparation_range_end.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_preparation_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPreparationDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePreparationDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fe9d02 */
u8 PLEXTOR_NEAR_FUNCTION InitializeRawReadPreparationRangeAndPacket(u8 alternate_range)
{
    s32 range_end;
    u8 write_type;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
        return 0u;

    INITIALIZE_PREPARATION_SEGMENTS(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD62));
    range_end = SELECT_PREPARATION_RANGE_END(alternate_range);

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type == 0u || write_type == 1u)
    {
        if(alternate_range == 1u)
        {
            WritePreparationDword(FW_NEAR_UNKNOWN_1598, ReadPreparationDword(FW_NEAR_UNKNOWN_DD51));
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B4) == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B3) == 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_14B6) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_14B7) != 1u)
                    WritePreparationDword(FW_NEAR_UNKNOWN_1598, ReadPreparationDword(FW_NEAR_UNKNOWN_DD3D) - (u32)range_end);
                else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B7) != 1u)
                    WritePreparationDword(FW_NEAR_UNKNOWN_1598, ReadPreparationDword(FW_NEAR_UNKNOWN_1598) - (u32)range_end);
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B6) == 0u)
            {
                WritePreparationDword(FW_NEAR_UNKNOWN_1598, ReadPreparationDword(FW_NEAR_UNKNOWN_1598) - (u32)range_end);
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, SELECT_PREPARATION_SECTOR_CLASS(1u, alternate_range));
    BUILD_PREPARATION_Q_PACKET((s32)((u32)range_end - 1ul), FW_NEAR_UNKNOWN_1272);
    BUILD_PREPARATION_TDI_PREFIX(FW_NEAR_UNKNOWN_143C);
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u);
}
