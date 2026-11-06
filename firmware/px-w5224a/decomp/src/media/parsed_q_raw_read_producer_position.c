#include "parsed_q_raw_read_producer_position.h"
#include "../platform/firmware_memory.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_ring_alignment.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/parsed_q_raw_read_producer_position_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadParsedQProducerWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadParsedQProducerDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteParsedQProducerDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ProgramParsedQProducerControllerSlot(void)
{
    u16 producer_index = ReadParsedQProducerWord(FW_NEAR_UNKNOWN_1296);
    u32 controller_address;
    u8 metadata_length;

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) > 0)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801C, (u8)producer_index);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801D, (u8)(producer_index >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, 0x80u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0x80u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0xccu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0xccu);

    controller_address = (u32)producer_index * 0x0930ul;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018007, (u8)controller_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, (u8)(controller_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, (u8)(controller_address >> 16));

    metadata_length = ReadParsedQProducerWord(FW_NEAR_UNKNOWN_0B00) == 0x0a56u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u ? 12u : 8u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, metadata_length);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, metadata_length);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 2u);
}

/* RAM:f9142d */
void PLEXTOR_NEAR_FUNCTION ProcessParsedQRawReadProducerPosition(void)
{
    u8 old_countdown;
    s32 position_delta;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 0x80u) != 0u)
    {
        old_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, (u8)(old_countdown - 1u));
        if((s8)old_countdown <= 0)
            ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0081u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F21) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F21, 0u);
        return;
    }

    if(DecodeControllerPositionFields(1u) == 0u)
    {
        old_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, (u8)(old_countdown - 1u));
        if((s8)old_countdown <= 0)
            ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0082u);
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 0x40u) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E) <= 15)
            ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0082u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFC) & 0x80u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0xccu);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0xccu);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);

    if(READ_PARSED_Q_PRODUCER_BIT_0013() == 0u && READ_PARSED_Q_PRODUCER_BIT_001C() == 0u && READ_PARSED_Q_PRODUCER_BIT_001D() == 0u)
    {
        position_delta = (s32)(ReadParsedQProducerDword(FW_NEAR_UNKNOWN_1235) - ReadParsedQProducerDword(FW_NEAR_UNKNOWN_0F6E));
        if(position_delta >= -170l && position_delta <= 170l)
            WriteParsedQProducerDword(FW_NEAR_UNKNOWN_159C, (u32)position_delta);

        position_delta = (s32)(ReadParsedQProducerDword(FW_NEAR_UNKNOWN_1598) + ReadParsedQProducerDword(FW_NEAR_UNKNOWN_0F10) - ReadParsedQProducerDword(FW_NEAR_UNKNOWN_0F6E));
        if(position_delta > 0l)
        {
            u16 tolerance = (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4) * 5u;

            if(position_delta >= (s32)(u32)tolerance)
            {
                if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) > 15)
                {
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u)
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) + 1u));
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 3u);
                    }
                    ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0083u);
                }
                else if(position_delta >= 150l)
                {
                    ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0083u);
                }
            }

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u || position_delta >= 8l)
            {
                u8 old_retry_count;

                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
                    return;
                old_retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2C);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, (u8)(old_retry_count + 1u));
                if((s8)old_retry_count <= 30)
                    return;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
                if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 17)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 1u);
                ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0083u);
                return;
            }
        }
        else if(position_delta < 0l && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u || position_delta < -4l))
        {
            ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0082u);
            return;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
    ProgramParsedQProducerControllerSlot();
}
