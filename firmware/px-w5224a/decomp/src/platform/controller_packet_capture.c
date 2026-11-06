#include "controller_packet_capture.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_packet_capture_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadControllerPacketU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteControllerPacketU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadControllerPacketU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteControllerPacketU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fb94e1 */
void CaptureControllerThreeBytePacket(void)
{
    u8 accepted = 1u;

    if(FirmwareReadBit0407() == 1u)
    {
        if((s8)FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802A) >= 0)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0x01u) != 0u)
                WriteControllerPacketU16(FW_NEAR_UNKNOWN_FC22, (u16)(ReadControllerPacketU16(FW_NEAR_UNKNOWN_FC22) + 1u));

            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) & 0x01u) == 1u)
                WriteControllerPacketU32(FW_NEAR_UNKNOWN_FB6A, ReadControllerPacketU32(FW_NEAR_UNKNOWN_FB6A) + 1ul);

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
            {
                s32 window_boundary;
                s32 decoded_position;

                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB95) != 0xffu)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB95, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB95) + 1u));

                if(PACKET_INDEXED_STATE(4u) != 0u)
                {
                    window_boundary = (s32)(ReadControllerPacketU32(FW_NEAR_CURRENT_FRAME_POSITION) - 0x34bcul);
                    decoded_position = (s32)ReadControllerPacketU32(FW_NEAR_UNKNOWN_1235);
                    if(decoded_position >= window_boundary)
                    {
                        window_boundary = (s32)(ReadControllerPacketU32(FW_NEAR_CURRENT_FRAME_POSITION) - 9000ul);
                        decoded_position = (s32)ReadControllerPacketU32(FW_NEAR_UNKNOWN_1235);
                        if(decoded_position <= window_boundary && ReadControllerPacketU16(FW_NEAR_UNKNOWN_FB96) != 0xffffu)
                            WriteControllerPacketU16(FW_NEAR_UNKNOWN_FB96, (u16)(ReadControllerPacketU16(FW_NEAR_UNKNOWN_FB96) + 1u));
                    }
                }
            }

            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0x01u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 1u)
                accepted = 0u;

            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0x02u) != 0u)
                WriteControllerPacketU32(FW_NEAR_UNKNOWN_1235, ReadControllerPacketU32(FW_NEAR_UNKNOWN_1235) + 1ul);
        }

        if(accepted != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1133, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802B));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1134, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802C));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1135, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802D));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1136, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) | 0x80u));
        }
    }
    else
    {
        accepted = 0u;
    }

    if(accepted == 0u)
    {
        u8 shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x04u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, shadow);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, shadow);
    }

    if((ReadControllerPacketU16(FW_NEAR_UNKNOWN_0FF5) & 0x0002u) != 0u || (ReadControllerPacketU16(FW_NEAR_UNKNOWN_0FF5) & 0x0004u) != 0u)
        PACKET_FLAG_CONSUMERS();

    if((ReadControllerPacketU16(FW_NEAR_UNKNOWN_0FF5) & 0x0008u) == 0x0008u)
        PACKET_MEASUREMENT_CONSUMER();
}
