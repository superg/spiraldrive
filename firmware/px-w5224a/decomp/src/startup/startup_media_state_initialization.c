#include "startup_media_state_initialization.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/startup_media_state_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadStartupMediaU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteStartupMediaU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteStartupMediaU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ClearStartupMediaByteMask(fw_addr_t address, u8 mask)
{
    FirmwareWrite8(address, (u8)(FirmwareRead8(address) & mask));
}

static void ClearStartupMediaWordMask(fw_addr_t address, u16 mask)
{
    WriteStartupMediaU16(address, (u16)(ReadStartupMediaU16(address) & mask));
}

/* RAM:f85583 */
void PLEXTOR_NEAR_FUNCTION InitializeStartupMediaState(void)
{
    u8 index;

    for(index = 0u; index < 16u; ++index)
        WriteStartupMediaU16(FW_NEAR_UNKNOWN_100A + (u16)(index * 2u), 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102C, 0u);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_102A, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_102D, 0xfeu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, 0u);
    STARTUP_MEDIA_WRITE_BIT(FW_BIT_UNKNOWN_0028, 0u);
    WriteStartupMediaU32(FW_NEAR_UNKNOWN_0CF5, 0x00014cccul);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_0CF9, 0x0640u);
    WriteStartupMediaU32(FW_NEAR_UNKNOWN_FC37, 0u);
    WriteStartupMediaU32(FW_NEAR_UNKNOWN_FC33, 0u);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_1000, ReadStartupMediaU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F848FE)));
    WriteStartupMediaU32(FW_NEAR_UNKNOWN_0EF1, 0u);
    STARTUP_MEDIA_REBUILD_RADIAL();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED5, 0u);

    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1301, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1301, 0xdfu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1302, 0xfeu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1302, 0xf7u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1302, 0x7fu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1302, 0xefu);
    STARTUP_MEDIA_WRITE_BIT(FW_BIT_UNKNOWN_0037, 0u);
    STARTUP_MEDIA_WRITE_BIT(FW_BIT_UNKNOWN_0038, 0u);
    STARTUP_MEDIA_WRITE_BIT(FW_BIT_UNKNOWN_003A, 0u);
    STARTUP_MEDIA_WRITE_BIT(FW_BIT_UNKNOWN_0036, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1303, 0xdfu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1303, 0xefu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1303, 0x7fu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1303, 0xfeu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1303, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1303, 0xbfu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1304, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1304, 0x7fu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1304, 0xefu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1305, 0xbfu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1305, 0xfbu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1305, 0xfeu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1307, 0xfeu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1307, 0xfdu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF1, 0u);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_0FF5, 0xffbfu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_0FF5, 0xfff7u);
    STARTUP_MEDIA_CLEAR_STATE_FLAGS();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_1305, 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB98, 0u);
    WriteStartupMediaU32(FW_NEAR_UNKNOWN_14D6, 0u);
    STARTUP_MEDIA_WRITE_BIT(FW_BIT_UNKNOWN_0031, 0u);
    STARTUP_MEDIA_WRITE_E8E9(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4C, 0x10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F971, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_0FB9, 0xfeu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xfffbu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xfff7u);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xffefu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xffdfu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xfeffu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xdfffu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xbfffu);
    ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0x7fffu);
    STARTUP_MEDIA_CLEAR_SPEED_REDUCTION();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79B, 0u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11DC) & 2u) == 0u)
    {
        ClearStartupMediaWordMask(FW_NEAR_UNKNOWN_FAAC, 0xf9ffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE9, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11DC, 0u);
        WriteStartupMediaU16(FW_NEAR_UNKNOWN_11DA, 0x0960u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11DC, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11DC) | 2u));
    }

    STARTUP_MEDIA_INITIALIZE_DISC_CONTROL();
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_FAD1, 0u);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_FACF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 0x34u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FC21, 0xf7u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FC21, 0xfbu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FC21, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FC21, 0xfeu);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_FC22, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9A8, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_158E, 0xfbu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_158E, 0xf7u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_158E, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_158E, 0xfeu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158B, 0u);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_1589, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB99, 0u);
    for(index = 0u; index < 6u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCB + index, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEA, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFE, 0u);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_FB96, 0u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x10u) == 0u)
    {
        ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAED, 0xfbu);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 4u) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C43, 0x28u);
    }
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAED, 0xefu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAED, 0xf7u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAED, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAED, 0xfeu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEE, 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x40u) == 0u)
        ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAEF, 0xfeu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAEF, 0xbfu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAEF, 0xdfu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAEF, 0xefu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAEF, 0xf7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF0, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FAEF, 0xfdu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102E, 0u);
    for(index = 0u; index < 0xa0u; ++index)
        WriteStartupMediaU16(FW_NEAR_UNKNOWN_0CFB + (u16)(index * 2u), 0u);
    WriteStartupMediaU32(FW_NEAR_UNKNOWN_FBED, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E8, 100u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E7, 100u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_F9E9, 0xfbu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_F9E9, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_F9E9, 0xfeu);
    if(STARTUP_MEDIA_INDEXED_STATE(4u) != 0u)
        STARTUP_MEDIA_CAPTURE_POSITION();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEC, 0u);
    WriteStartupMediaU16(FW_NEAR_UNKNOWN_FC30, 0u);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FC32, 0xfdu);
    ClearStartupMediaByteMask(FW_NEAR_UNKNOWN_FC32, 0xfeu);
}
