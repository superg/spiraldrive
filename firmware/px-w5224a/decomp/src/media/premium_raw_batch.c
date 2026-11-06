#include "premium_raw_batch.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/premium_raw_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPremiumBatchWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadPremiumBatchDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WritePremiumBatchDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fc691d */
u32 AcquirePlextorPremiumRawSectorBatch(s32 start_frame_position, u16 sector_count)
{
    u32 main_buffer_address;

    PREPARE_BATCH_BITS();
    RESET_BATCH_QUEUE();
    INITIALIZE_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
    WritePremiumBatchDword(FW_NEAR_UNKNOWN_1598, (u32)start_frame_position + 0x96ul);

    do
    {
        YIELD_COROUTINE0();
        if(sector_count == ReadPremiumBatchWord(FW_NEAR_UNKNOWN_1292))
            break;
    } while(READ_PRODUCER_ACTIVE() != 0u);

    if(READ_PRODUCER_ACTIVE() != 0u)
    {
        u16 descriptor_address = GET_CONSUMER_DESCRIPTOR();

        main_buffer_address = ReadPremiumBatchDword((fw_addr_t)(u16)(descriptor_address + 4u));
    }
    else
    {
        main_buffer_address = 0xfffffffful;
    }

    SERVICE_HOST_COMMAND();
    RESET_BATCH_QUEUE();
    INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    DISPATCH_READ_REQUEST();
    YIELD_COROUTINE0();
    return main_buffer_address;
}

static void CopyPremiumPayloadBytes(fw_addr_t destination, fw_addr_t source, u32 byte_count)
{
    while(byte_count != 0ul)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        ++destination;
        ++source;
        --byte_count;
    }
}

/* RAM:fd6936 */
u8 PLEXTOR_NEAR_FUNCTION ValidatePlextorPremiumDiscSignatureSector(void)
{
    fw_addr_t acquired_address;
    fw_addr_t signature_address;
    fw_addr_t checksum_payload_address;
    u16 checksum = 0u;
    u16 word_index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x04u) == 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) != 1u)
        return 0u;

    acquired_address = SIGNATURE_ACQUIRE_BATCH(0x15l, 2u);
    if(acquired_address == 0xfffffffful)
        return 0u;

    signature_address = FirmwareAddressIncrement24(acquired_address, FW_HUGE_UNKNOWN_100010);
    if(AreByteRangesEqual(signature_address, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9608), 14u) == 0u)
        return 0u;

    checksum_payload_address = FirmwareAddressIncrement24(acquired_address, FW_HUGE_UNKNOWN_100940);
    for(word_index = 0u; word_index < 0x400u; ++word_index)
        checksum = (u16)(checksum + ReadPremiumBatchWord(FirmwareAddressIncrement24(checksum_payload_address, (fw_addr_t)word_index * 2u)));
    if(ReadPremiumBatchWord(FirmwareAddressIncrement24(signature_address, 14u)) != checksum)
        return 0u;

    if(AreByteRangesEqual(FirmwareAddressIncrement24(signature_address, 20u), 0x0000dcd5ul, 16u) == 0u)
        return 0u;

    SIGNATURE_SERVICE_HOST_COMMAND();
    return 1u;
}

/* RAM:fd6a30 */
u8 PLEXTOR_NEAR_FUNCTION AcquirePlextorPremiumFirmwarePayloadFromDisc(void)
{
    u32 acquisition_address;
    u16 attempt;
    u16 sector_index;

    PAYLOAD_RESET_QUEUE();
    PAYLOAD_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    PAYLOAD_INITIALIZE_COROUTINE3(FW_ENTRY_IDLE_COROUTINE3);
    PAYLOAD_YIELD_COROUTINE0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 4u);

    for(attempt = 0u; attempt < 3u; ++attempt)
    {
        acquisition_address = PAYLOAD_ACQUIRE_BATCH(0x17l, 0x100u);
        if(acquisition_address == 0xfffffffful)
            return 0u;
        if(acquisition_address != 0ul)
            continue;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) | 0x02u));
        PAYLOAD_RESET_MEDIA_COROUTINES();
        PAYLOAD_SET_P3_RX_LOW_TX_HIGH();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C3) == 0u)
        {
            PAYLOAD_YIELD_COROUTINE0();
            PAYLOAD_SERVICE_HOST_COMMAND();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 1u);
        }
        PAYLOAD_CLEAR_P1_TIMER_PINS();

        CopyPremiumPayloadBytes(FW_HUGE_PREMIUM_PAYLOAD_SOURCE, FW_HUGE_BUFFER_BANK_BASE, 0x00093000ul);
        for(sector_index = 0u; sector_index < 0x100u; ++sector_index)
            CopyPremiumPayloadBytes(FW_HUGE_BUFFER_BANK_BASE + (fw_addr_t)sector_index * 0x800ul, FW_HUGE_PREMIUM_PAYLOAD_SECTOR_SOURCE + MultiplyU32Low(0x930ul, sector_index), 0x800ul);
        PAYLOAD_SERVICE_HOST_COMMAND();
        return 1u;
    }
    return 0u;
}
