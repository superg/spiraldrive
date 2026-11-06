#include "primary_raw_read_producer_position.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_ring_alignment.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/primary_raw_read_producer_position_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/primary_raw_read_producer_position_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadProducerPositionWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadProducerPositionDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteProducerPositionDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ServicePrimaryProducerStatus(u16 status)
{
    ServiceParsedQRawReadCountdownAndEnqueueDescriptor(status);
}

static void ProgramPrimaryProducerControllerSlot(void)
{
    u16 producer_index = ReadProducerPositionWord(FW_NEAR_UNKNOWN_1296);
    u32 controller_address;
    u8 metadata_length;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) != 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801C, (u8)producer_index);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801D, (u8)(producer_index >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, 0x80u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0x80u);
    }

    if(READ_PRODUCER_BIT_0020() != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x4cu);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x4cu);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0xccu);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0xccu);
    }

    controller_address = (u32)producer_index * 0x0930ul;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 1u)
        controller_address += ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F7F) + 0x2df0ul;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018007, (u8)controller_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, (u8)(controller_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, (u8)(controller_address >> 16));

    metadata_length = ReadProducerPositionWord(FW_NEAR_UNKNOWN_0B00) == 0x0a56u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u ? 12u : 8u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, metadata_length);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, metadata_length);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 2u);
}

/* RAM:f91655 */
void PLEXTOR_NEAR_FUNCTION ProcessPrimaryRawReadProducerPosition(void)
{
    u8 old_countdown;
    s32 target_position;
    s32 position_delta;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 0x80u) != 0u)
    {
        old_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, (u8)(old_countdown - 1u));
        if((s8)old_countdown <= 0)
            ServicePrimaryProducerStatus(0x0081u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F21) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F21, 0u);
        if(READ_PRODUCER_BIT_0020() == 0u)
            return;
    }

    if(DecodeControllerPositionFields(1u) == 0u)
    {
        old_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, (u8)(old_countdown - 1u));
        if((s8)old_countdown <= 0)
            ServicePrimaryProducerStatus(0x0082u);
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 0x40u) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E) <= 15)
            ServicePrimaryProducerStatus(0x0082u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFC) & 0x80u) != 0u)
    {
        u8 mode = READ_PRODUCER_BIT_0020() != 0u ? 0x4cu : 0xccu;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, mode);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, mode);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);

    WriteProducerPositionDword(FW_NEAR_UNKNOWN_0F83, ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F87));
    WriteProducerPositionDword(FW_NEAR_UNKNOWN_0F87, ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F8B));
    WriteProducerPositionDword(FW_NEAR_UNKNOWN_0F8B, ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F8F));
    WriteProducerPositionDword(FW_NEAR_UNKNOWN_0F8F, ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F93));
    WriteProducerPositionDword(FW_NEAR_UNKNOWN_0F93, ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F6E));

    position_delta = (s32)(ReadProducerPositionDword(FW_NEAR_UNKNOWN_1235) - ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F6E));
    if(position_delta >= -170l && position_delta <= 170l)
        WriteProducerPositionDword(FW_NEAR_UNKNOWN_159C, (u32)position_delta);

    target_position = (s32)(ReadProducerPositionDword(FW_NEAR_UNKNOWN_1598) + ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F10));
    if(READ_PRODUCER_BIT_0026() != 0u && READ_PRODUCER_BIT_001B() == 0u)
    {
        if((s32)ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F6E) >= (s32)ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F97))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9B) - 1u));
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9B) <= 1)
        {
            s32 probe_delta = (s32)((u32)target_position - ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F97));

            if(probe_delta < 0l || probe_delta >= 32l)
            {
                WRITE_PRODUCER_BIT_0026(0u);
                ServicePrimaryProducerStatus(0x0083u);
                return;
            }
            WriteProducerPositionDword(FW_NEAR_UNKNOWN_0F6E, (u32)target_position);
        }
    }

    position_delta = (s32)((u32)target_position - ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F6E));
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
                ServicePrimaryProducerStatus(0x0083u);
            }
            else if(position_delta >= 150l)
            {
                ServicePrimaryProducerStatus(0x0083u);
            }
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u || position_delta >= 8l)
        {
            u8 old_retry_count;

            if(READ_PRODUCER_BIT_0026() != 0u)
                return;
            old_retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2C);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, (u8)(old_retry_count + 1u));
            if((s8)old_retry_count <= 75)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
            ServicePrimaryProducerStatus(0x0083u);
            return;
        }
    }
    else if(position_delta < 0l && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u || position_delta < -4l))
    {
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u && READ_PRODUCER_BIT_0020() != 0u)
        {
            if(READ_PRODUCER_BIT_0026() != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9B) > 0)
                return;
            if((s32)ReadProducerPositionDword(FW_NEAR_UNKNOWN_0F6E) == (s32)ReadProducerPositionDword(FW_NEAR_UNKNOWN_1598))
            {
                WRITE_PRODUCER_BIT_0026(1u);
                SelectRecentReadAcquisitionProbeTarget();
            }
            else
            {
                u8 old_speed_retry = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9C);

                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, (u8)(old_speed_retry + 1u));
                if((s8)old_speed_retry > 5)
                    UpdateRawReadSpeedForAcquisitionPhase();
            }
        }
        ServicePrimaryProducerStatus(0x0082u);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
    WRITE_PRODUCER_BIT_0026(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, 0u);
    ProgramPrimaryProducerControllerSlot();
}
