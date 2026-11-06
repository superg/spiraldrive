#include "read_disc_information_recordable.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_disc_information_recordable_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRecordableDiscDword(u16 address)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_disc_information_recordable_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteRecordableDiscDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void AddRecordableDiscStatusBits(u8 bits)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C) | bits));
}

/* RAM:fae0bd */
u8 PLEXTOR_NEAR_FUNCTION BuildRecordableMediaReadDiscInformationResponse(void)
{
    u8 opc_record_count;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    opc_record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74B);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, (u8)((u8)(opc_record_count << 3) + 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u ? 0x10u : 0u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u || RECORDABLE_TAIL_UNINITIALIZED() == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E) != FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC))
            AddRecordableDiscStatusBits(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u ? 1u : 2u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
        {
            if(RECORDABLE_TAIL_UNINITIALIZED() == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E) != FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC))
                AddRecordableDiscStatusBits(4u);
        }
        else
        {
            AddRecordableDiscStatusBits(0x0cu);
        }
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u || RECORDABLE_TAIL_UNINITIALIZED() == 0u)
            AddRecordableDiscStatusBits(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u ? 1u : 2u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
        {
            if(RECORDABLE_TAIL_UNINITIALIZED() == 0u)
                AddRecordableDiscStatusBits(4u);
        }
        else
        {
            AddRecordableDiscStatusBits(0x0cu);
        }
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0x1eu);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u ? 1u : FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u ? (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) + 1u) : FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 1u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
    {
        u8 last_track;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u)
            last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC);
        else
            last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, last_track == 100u ? 99u : last_track);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, RECORDABLE_FIRST_TRACK_LAST_SESSION());
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
    }
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, 1u);
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) & 0x40u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0x20u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        u8 media_status = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);

        if(media_status != 0u)
        {
            if(media_status == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B31) | 1u));
            }
            else if(media_status == 2u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B31) | 2u));
            }
            else if(media_status == 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B31) | 3u));
            }
            else
            {
                for(;;)
                {
                    /* Canonical fail-stop for an impossible media status. */
                }
            }
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B31) | FirmwareReadNear8(FW_NEAR_UNKNOWN_0C9F)));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, RECORDABLE_DISC_TYPE());
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) == 1u)
    {
        u32 disc_id;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B31) | 0x80u));
        disc_id = RECORDABLE_MULTIPLY(100ul, FirmwareRead8(FW_HUGE_UNKNOWN_2BB5DF));
        disc_id += RECORDABLE_MULTIPLY(10000ul, FirmwareRead8(FW_HUGE_UNKNOWN_2BB5DE));
        disc_id += FirmwareRead8(FW_HUGE_UNKNOWN_2BB5E0);
        WriteRecordableDiscDword(FW_NEAR_UNKNOWN_0B36, disc_id);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
    {
        RECORDABLE_CONVERT_MSF((s32)ReadRecordableDiscDword(FW_NEAR_UNKNOWN_DD69), FW_NEAR_UNKNOWN_0B3B);
    }
    else if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3D, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3C, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3B, 0xffu);
    }
    else
    {
        RECORDABLE_CONVERT_MSF((s32)ReadRecordableDiscDword(FW_NEAR_UNKNOWN_DD69), FW_NEAR_UNKNOWN_0B3B);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
    {
        RECORDABLE_CONVERT_MSF((s32)ReadRecordableDiscDword(FW_NEAR_CURRENT_FRAME_POSITION), FW_NEAR_UNKNOWN_0B3F);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B42, 0u);
    }
    else if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B41, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B40, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3F, 0xffu);
    }
    else
    {
        RECORDABLE_CONVERT_MSF((s32)ReadRecordableDiscDword(FW_NEAR_CURRENT_FRAME_POSITION), FW_NEAR_UNKNOWN_0B3F);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B42, 0u);
    }

    opc_record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74B);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B4B, opc_record_count);
    opc_record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_E74B);
    RECORDABLE_COPY(FW_NEAR_UNKNOWN_E72B, FW_NEAR_UNKNOWN_0B4C, (u16)((u16)opc_record_count * 8u));
    return 1u;
}
