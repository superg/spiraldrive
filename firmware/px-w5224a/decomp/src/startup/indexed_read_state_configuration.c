#include "indexed_read_state_configuration.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/indexed_read_state_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadIndexedReadU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteIndexedReadU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteIndexedReadU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void SelectLeadInReadTarget(void)
{
    u8 track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
    WriteIndexedReadU32(FW_NEAR_UNKNOWN_0E3B, (u32)INDEXED_READ_CONVERT(FW_NEAR_UNKNOWN_F2B9 + (fw_addr_t)((u16)track_number * 6u)));
}

static void SetDiscEndpointRadialValue(fw_addr_t frame_position_address)
{
    s32 frame_position = (s32)ReadIndexedReadU32(frame_position_address);
    s32 seconds = INDEXED_READ_DIVIDE(frame_position, 75l);

    WriteIndexedReadU16(FW_NEAR_UNKNOWN_0FF7, INDEXED_READ_MAP((u16)seconds));
}

static void DispatchIndexedReadState(u8 state)
{
    u8 index;

    switch(state)
    {
    case 0x00u:
    {
        u8 configuration_byte;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 2u);
        INDEXED_READ_CONFIGURATION(0x003eul, &configuration_byte, 1u);
        WriteIndexedReadU32(FW_NEAR_UNKNOWN_0E3B, (configuration_byte & 0x0fu) == 0u ? 0x0004ce78ul : 0x00047694ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0u);
        break;
    }
    case 0x05u:
        WriteIndexedReadU32(FW_NEAR_UNKNOWN_0E3B, 45000ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
        break;
    case 0x06u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E1, 1u);
        WriteIndexedReadU16(FW_NEAR_UNKNOWN_F9DF, 0x001eu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfbu));
        break;
    case 0x0au:
        WriteIndexedReadU32(FW_NEAR_UNKNOWN_11F0, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
        break;
    case 0x0cu:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, 0u);
        break;
    case 0x11u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 4u));
        break;
    case 0x12u:
        INDEXED_READ_INITIALIZE_SUBSYSTEM();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, 0u);
        break;
    case 0x13u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE6, 0u);
        break;
    case 0x1fu:
        for(index = 0u; index < 6u; ++index)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9AC + (u16)(index * 2u), 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9AB + (u16)(index * 2u), 0u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F977, (FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 8u) != 0u ? 3u : 0u);
        INDEXED_READ_WRITE_BIT(FW_BIT_UNKNOWN_0037, 1u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
        {
            INDEXED_READ_ADVANCE();
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F977, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E1, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfbu));
            WriteIndexedReadU16(FW_NEAR_UNKNOWN_F9DF, 0x001eu);
        }
        break;
    case 0xf6u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 5u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 12u : 2u);
        break;
    case 0xf9u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 0xc0u));
        INDEXED_READ_WRITE_BIT(FW_BIT_UNKNOWN_0580, 0u);
        INDEXED_READ_WRITE_BIT(FW_BIT_UNKNOWN_0581, 1u);
        break;
    case 0x01u:
    case 0x02u:
    case 0x03u:
    case 0x04u:
    case 0x07u:
    case 0x09u:
    case 0x0eu:
    case 0x14u:
    case 0x15u:
    case 0x16u:
    case 0x17u:
    case 0xf2u:
    case 0xf3u:
    case 0xf4u:
    case 0xfdu:
    case 0xffu:
        break;
    default:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
        break;
    }
}

/* RAM:f859f0 */
void PLEXTOR_NEAR_FUNCTION ConfigureMediaReadStateForIndexedDriveState(void)
{
    u8 media_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    u8 flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xefu);

    if(media_type != 2u)
        flags &= 0xf7u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, flags);

    if(media_type == 0u)
    {
        SelectLeadInReadTarget();
    }
    else if((flags & 0x40u) != 0u)
    {
        if((flags & 0x80u) != 0u)
        {
            if((flags & 0x20u) == 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(flags | 0x10u));
            WriteIndexedReadU32(FW_NEAR_UNKNOWN_0E3B, 0u);
        }
        else
        {
            u8 track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
            WriteIndexedReadU32(FW_NEAR_UNKNOWN_0E3B, (u32)INDEXED_READ_CONVERT(FW_HUGE_UNKNOWN_2BB042 + (fw_addr_t)((u16)track_number * 10u)));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xf7u));
        }
    }
    else
    {
        SelectLeadInReadTarget();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xf7u));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    if(media_type == 0u)
    {
        SetDiscEndpointRadialValue(FW_NEAR_UNKNOWN_12C1);
    }
    else
    {
        SetDiscEndpointRadialValue(FW_NEAR_CURRENT_FRAME_POSITION);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u)
            SetDiscEndpointRadialValue(FW_NEAR_UNKNOWN_12C1);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) == 1u ? 4u : 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xbfu));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        DispatchIndexedReadState(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1309) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);

    INDEXED_READ_RESET(1u);
}
