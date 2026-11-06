#include "get_event_status_notification.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/get_event_status_notification_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadGetEventStatusWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadGetEventStatusDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteGetEventStatusWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteGetEventStatusDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void SetGetEventStatusNoSense(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B18, 0x70u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B24, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B25, 0u);
    WriteGetEventStatusDword(FW_NEAR_UNKNOWN_0B1B, 0ul);
}

/* RAM:f9b78a */
/* Builds the MMC GET EVENT STATUS NOTIFICATION response for the first pending
 * requested class, or identifies the first requested supported class when all
 * of its descriptors are empty. */
void PLEXTOR_NEAR_FUNCTION HandleGetEventStatusNotificationCommand(void)
{
    u8 requested_classes;
    u8 fallback_class = 0u;
    u8 response_class = 0u;
    u16 allocation_length;
    u16 response_length = 4u;
    u16 transfer_length;
    u32 descriptor = 0ul;

    if(VALIDATE_GET_EVENT_STATUS_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E97)) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 1u) == 0u)
    {
        REJECT_GET_EVENT_STATUS();
        return;
    }

    requested_classes = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E);
    allocation_length = ReadGetEventStatusWord(FW_NEAR_UNKNOWN_0B11);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);

    if((requested_classes & 0x02u) != 0u)
    {
        response_length = (u16)(response_length + 4u);
        descriptor = BuildOperationalChangeEventStatusDescriptor();
        if((u8)(descriptor >> 24) != 0u || requested_classes == 0x02u)
            response_class = 1u;
        else
            fallback_class = 0x02u;
    }
    if(response_class == 0u && (requested_classes & 0x04u) != 0u)
    {
        response_length = (u16)(response_length + 4u);
        descriptor = BuildMediaEventStatusDescriptor();
        if((u8)(descriptor >> 24) != 0u || requested_classes == 0x04u)
            response_class = 2u;
        else if(fallback_class == 0u)
            fallback_class = 0x04u;
    }
    if(response_class == 0u && (requested_classes & 0x08u) != 0u)
    {
        response_length = (u16)(response_length + 4u);
        descriptor = BuildEmptyExternalRequestEventStatusDescriptor();
        if((u8)(descriptor >> 24) != 0u || requested_classes == 0x08u)
            response_class = 3u;
        else if(fallback_class == 0u)
            fallback_class = 0x08u;
    }
    if(response_class == 0u && (requested_classes & 0x10u) != 0u)
    {
        response_length = (u16)(response_length + 4u);
        descriptor = BuildGetEventStatusMediaEventDescriptor(0x10u);
        if((u8)(descriptor >> 24) != 0u || requested_classes == 0x10u)
            response_class = 4u;
        else if(fallback_class == 0u)
            fallback_class = 0x10u;
    }
    if(response_class == 0u && (requested_classes & 0x40u) != 0u)
    {
        response_length = (u16)(response_length + 4u);
        descriptor = BuildDeviceBusyEventStatusDescriptor();
        if((u8)(descriptor >> 24) != 0u || requested_classes == 0x40u)
            response_class = 6u;
        else if(fallback_class == 0u)
            fallback_class = 0x40u;
    }

    if(response_class == 0u)
    {
        if(fallback_class == 0u)
        {
            response_class = 0x80u;
            response_length = 4u;
        }
        else
        {
            response_class = fallback_class;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, response_class);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0x5eu);
    WriteGetEventStatusWord(FW_NEAR_UNKNOWN_0B2A, (u16)(response_length - 2u));
    if(response_class != 0x80u)
        WriteGetEventStatusDword(FW_NEAR_UNKNOWN_0B2E, descriptor);

    STAGE_GET_EVENT_STATUS_RESPONSE(FW_NEAR_UNKNOWN_0B2A, ReadGetEventStatusDword(FW_NEAR_UNKNOWN_155A), (u32)response_length);
    transfer_length = GET_EVENT_STATUS_MIN_U16(allocation_length, response_length);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 0x10u));
    if(EXECUTE_GET_EVENT_STATUS_DATA_IN((u32)transfer_length) != 4u)
        SetGetEventStatusNoSense();
}
