#include "controller_buffer_chunk_fill.h"
#include "firmware_memory.h"

static void WriteControllerBufferFillAddress(fw_addr_t buffer_address)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)buffer_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(buffer_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(buffer_address >> 16));
}

static void WriteControllerBufferFillLength(u16 byte_count)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)byte_count);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(byte_count >> 8));
}

/* RAM:facc3b */
void FillControllerBufferChunkAndWait(fw_addr_t buffer_address, u16 byte_count, u8 value)
{
    u8 repetition;
    u8 controller_shadow;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0xe5u);
    for(repetition = 0u; repetition < 5u; ++repetition)
        WriteControllerBufferFillAddress(buffer_address);
    for(repetition = 0u; repetition < 5u; ++repetition)
        WriteControllerBufferFillLength(byte_count);

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0xe0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018029, value);
    WriteControllerBufferFillAddress(buffer_address);
    WriteControllerBufferFillLength(byte_count);

    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA0) == 1u ? 0x88u : 0x80u);

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_14DA) == 0u)
    {
    }

    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xbfu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_shadow);
}
