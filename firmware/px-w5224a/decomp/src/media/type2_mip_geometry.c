#include "type2_mip_geometry.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"

static u32 ReadMipGeometryU24(fw_addr_t address)
{
    u32 value = (u32)FirmwareRead8(address) << 16;

    value |= (u32)FirmwareRead8(address + 1u) << 8;
    value |= FirmwareRead8(address + 2u);
    return value;
}

static u32 ReadMipGeometryU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteMipGeometryU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u32 MapMipCoordinateToControllerFrame(u32 coordinate)
{
    return (MultiplyU32Low(39ul, coordinate) >> 5) - 0xa7ul;
}

/* RAM:fcd42a */
void PLEXTOR_NEAR_FUNCTION DeriveMipControllerRecordGeometryFromPayload(void)
{
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        u32 primary_coordinate = ReadMipGeometryU24(FW_HUGE_UNKNOWN_2BB931);
        u32 secondary_coordinate = ReadMipGeometryU24(FW_HUGE_UNKNOWN_2BB934);
        u32 base = ReadMipGeometryU32(FW_HUGE_UNKNOWN_2BA57F);
        u8 row;
        u8 column;
        u8 found = 0u;

        WriteMipGeometryU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION, primary_coordinate);
        for(row = 0u; row != 3u && found == 0u; ++row)
        {
            for(column = 0u; column != 8u; ++column)
            {
                u32 candidate = base + ((u32)row << 8) + ((u32)(column + 1u) << 5);

                if(primary_coordinate == candidate)
                {
                    found = 1u;
                    break;
                }
            }
        }
        if(found == 0u)
        {
            primary_coordinate = base + 0x20ul;
            WriteMipGeometryU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION, primary_coordinate);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC4C, 1u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC4C, 0u);
        }

        WriteMipGeometryU32(FW_HUGE_UNKNOWN_2BA59F, MapMipCoordinateToControllerFrame(primary_coordinate));
        if(secondary_coordinate == 0ul)
        {
            WriteMipGeometryU32(FW_HUGE_UNKNOWN_2BA5A3, 0ul);
            WriteMipGeometryU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, 0ul);
        }
        else
        {
            WriteMipGeometryU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, secondary_coordinate);
            WriteMipGeometryU32(FW_HUGE_UNKNOWN_2BA5A3, MapMipCoordinateToControllerFrame(secondary_coordinate));
        }

        WriteMipGeometryU32(FW_HUGE_UNKNOWN_2BA58D, ReadMipGeometryU32(FW_HUGE_UNKNOWN_2BA59F) + 0x15ful);
        WriteMipGeometryU32(FW_HUGE_UNKNOWN_2BA595, ReadMipGeometryU32(FW_HUGE_UNKNOWN_2BA58D) - 0x27ul);
        WriteMipGeometryU32(FW_HUGE_UNKNOWN_2BA599, ReadMipGeometryU32(FW_HUGE_UNKNOWN_2BA595) - 0x138ul);
    }

    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) == 0u)
        FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 0x14u);
    else
        FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 0x3cu);
}
