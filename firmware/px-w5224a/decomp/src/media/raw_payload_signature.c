#include "raw_payload_signature.h"
#include "../platform/firmware_memory.h"

extern u16 GetLatestQueuedRawSectorDescriptorAddress(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_payload_signature_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPayloadDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static u16 ReadPayloadWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WritePayloadDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 PayloadPrefixMatches(u8 first, u8 second, u8 third)
{
    return (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_2BB919) == first && FirmwareRead8(FW_HUGE_UNKNOWN_2BB91A) == second && FirmwareRead8(FW_HUGE_UNKNOWN_2BB91B) == third);
}

/* RAM:fc6247 */
u8 PLEXTOR_NEAR_FUNCTION ClassifyLatestQueuedRawSectorDescriptor(void)
{
    u16 descriptor_address;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        return 0u;
    descriptor_address = GetLatestQueuedRawSectorDescriptorAddress();
    if(descriptor_address == 0u)
        return 0xfeu;
    if(ReadPayloadWord((fw_addr_t)(u16)(descriptor_address + 0x0eu)) != 0u)
        return 0u;
    if(ReadPayloadWord(FW_NEAR_UNKNOWN_1296) == 0u)
        return 0xfeu;
    return 1u;
}

/* RAM:fc6019 */
u8 ClassifyRawSectorPayloadSignatureAtPipelineTarget(void)
{
    s32 logical_block_address = (s32)ReadPayloadDword(FW_NEAR_UNKNOWN_1598) - 150l;
    s32 mapped_address;
    u8 acquisition_status;
    u8 result;

    WritePayloadDword(FW_NEAR_UNKNOWN_15DC, (u32)logical_block_address);
    WritePayloadDword(FW_NEAR_UNKNOWN_0B02, (u32)logical_block_address);
    mapped_address = MAP_NEGATIVE_PAYLOAD_ADDRESS(logical_block_address);
    WritePayloadDword(FW_NEAR_UNKNOWN_0B02, (u32)(mapped_address - 150l));
    WritePayloadDword(FW_NEAR_UNKNOWN_1598, (u32)mapped_address);

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB91C, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB91B, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB91A, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB919, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    SET_PAYLOAD_HANDSHAKE_PINS(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
    RESET_PAYLOAD_QUEUE();
    INITIALIZE_PAYLOAD_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F07, 0u);
    SET_PAYLOAD_BIT001B(1u);
    do
    {
        YIELD_PAYLOAD_COROUTINE3();
        acquisition_status = POLL_PAYLOAD_DESCRIPTOR();
    } while(acquisition_status == 0xfeu);
    SET_PAYLOAD_BIT001B(0u);

    if(acquisition_status == 0u)
    {
        result = 0xffu;
    }
    else
    {
        u16 descriptor_address = GET_PAYLOAD_CONSUMER_DESCRIPTOR();
        fw_addr_t source_address = (ReadPayloadDword((fw_addr_t)(u16)(descriptor_address + 4u)) + FW_HUGE_RAW_PAYLOAD_SIGNATURE) & 0x00fffffful;
        u16 offset;

        for(offset = 0u; offset < 0x0800u; ++offset)
            FirmwareWrite8(FW_HUGE_UNKNOWN_2BB919 + offset, FirmwareRead8(FirmwareAddressIncrement24(source_address, offset)));

        if(PayloadPrefixMatches('M', 'I', 'P'))
        {
            result = 0x0fu;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 1u);
        }
        else if(PayloadPrefixMatches('M', 'D', 'T'))
        {
            result = 0x10u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 1u);
        }
        else if(PayloadPrefixMatches('S', 'I', 'P'))
        {
            result = 0x11u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 1u);
        }
        else if(PayloadPrefixMatches('S', 'D', 'T'))
        {
            result = 0x12u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 1u);
        }
        else
        {
            result = 0xfeu;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 0u);
        }
    }

    RESET_PAYLOAD_QUEUE();
    INITIALIZE_PAYLOAD_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_PAYLOAD_HANDSHAKE_PINS(1u);
    return result;
}
