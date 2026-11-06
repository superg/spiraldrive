#include "memory.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/memory_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/memory_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/memory_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/memory_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/memory_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 7
#include "bindings/platform/memory_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fde0d5 */
u16 ReadBigEndianU16(fw_addr_t source_address)
{
    return ((u16)FirmwareRead8(source_address) << 8) | FirmwareRead8(FirmwareAddressIncrementLow16(source_address, 1));
}

/* RAM:fde0e8 */
void WriteBigEndianU16(fw_addr_t destination_address, u16 value)
{
    FirmwareWrite8(destination_address, (u8)(value >> 8));
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, 1), (u8)value);
}

/* RAM:fee823 */
void ZeroMemoryWords(fw_addr_t destination, u16 byte_count)
{
    u16 word_count = byte_count >> 1;

    /* The source uses a do/while. byte_count == 0 therefore wraps to 65536
     * word stores; verified callers supply an even nonzero count. */
    do
    {
        FirmwareWrite8(destination, 0);
        FirmwareWrite8(destination + 1ul, 0);
        destination += 2ul;
        word_count--;
    } while(word_count != 0);
}

/* RAM:fd9638 */
void CopyBytes16(u16 destination, u16 source, u8 byte_count)
{
    u8 word_count;
    u8 high;
    u8 low;

    if(destination == 0)
        return;
    word_count = byte_count >> 1;
    while(word_count != 0)
    {
        high = FirmwareRead8(source);
        low = FirmwareReadNear8((u16)(source + 1u));

        FirmwareWrite8(destination, high);
        FirmwareWriteNear8((u16)(destination + 1u), low);
        source = (u16)(source + 2u);
        destination = (u16)(destination + 2u);
        word_count--;
    }
    if((byte_count & 1u) != 0)
        FirmwareWrite8(destination, FirmwareRead8(source));
}

/* RAM:f849cc */
void CopyBytesWith8BitCount(fw_addr_t destination, fw_addr_t source, u8 byte_count)
{
    CopyBytesWithIndexRange(destination, source, 0, byte_count);
}

/* RAM:f849de */
void CopyBytesWithIndexRange(fw_addr_t destination, fw_addr_t source, u8 current_index, u8 exclusive_end)
{
    while(current_index < exclusive_end)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        destination = FirmwareAddressIncrementLow16(destination, 1);
        source = FirmwareAddressIncrementLow16(source, 1);
        current_index++;
    }
}

/* RAM:fa30fc */
void CopySixBytesFar(fw_addr_t destination, fw_addr_t source)
{
    u16 first = FirmwareReadBigEndianU16Full(source);
    u16 second = FirmwareReadBigEndianU16Full(FirmwareAddressIncrement24(source, 2));

    FirmwareWriteBigEndianU16Full(FirmwareAddressIncrement24(destination, 2), second);
    FirmwareWriteBigEndianU16Full(destination, first);
    FirmwareWriteBigEndianU16Full(FirmwareAddressIncrement24(destination, 4), FirmwareReadBigEndianU16Full(FirmwareAddressIncrement24(source, 4)));
}

/* RAM:faddb5 */
void PLEXTOR_NEAR_FUNCTION CopyTenBytesFar(fw_addr_t destination, fw_addr_t source)
{
    u16 first = FirmwareReadBigEndianU16Full(source);
    u16 second = FirmwareReadBigEndianU16Full(FirmwareAddressIncrementLow16(source, 2));

    FirmwareWriteBigEndianU16Full(FirmwareAddressIncrementLow16(destination, 2), second);
    FirmwareWriteBigEndianU16Full(destination, first);
    source = FirmwareAddressIncrementLow16(source, 4);
    destination = FirmwareAddressIncrementLow16(destination, 4);
    first = FirmwareReadBigEndianU16Full(source);
    second = FirmwareReadBigEndianU16Full(FirmwareAddressIncrementLow16(source, 2));
    FirmwareWriteBigEndianU16Full(FirmwareAddressIncrementLow16(destination, 2), second);
    FirmwareWriteBigEndianU16Full(destination, first);
    FirmwareWriteBigEndianU16Full(FirmwareAddressIncrementLow16(destination, 4), FirmwareReadBigEndianU16Full(FirmwareAddressIncrementLow16(source, 4)));
}

/* RAM:faca48 */
void PLEXTOR_NEAR_FUNCTION CopyBinaryTocPayloadToFullTocDescriptorBody(fw_addr_t source_packet_address, fw_addr_t destination_descriptor_body_address)
{
    u8 index;

    FirmwareWrite8(destination_descriptor_body_address, FirmwareRead8(source_packet_address));
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination_descriptor_body_address, 1u), 0u);
    for(index = 2u; index < 10u; ++index)
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination_descriptor_body_address, index), FirmwareRead8(FirmwareAddressIncrementLow16(source_packet_address, index)));
}

/* RAM:fa5263 */
void CopyBytesFromAddressPlus1MiB(fw_addr_t source_base, fw_addr_t destination, u8 byte_count)
{
    u8 index = 0;
    fw_addr_t source = FirmwareAddressIncrement24(source_base, FW_ADDRESS_BANK_STRIDE);

    while(index < byte_count)
    {
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination, index), FirmwareRead8(FirmwareAddressIncrement24(source, index)));
        index++;
    }
}

/* RAM:fe01d3 */
void CopyBytesToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u8 byte_count)
{
    fw_addr_t destination = FirmwareAddressIncrement24(destination_base, FW_ADDRESS_BANK_STRIDE);
    u8 index = 0;

    while(index < byte_count)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        source = FirmwareAddressIncrement24(source, 1);
        destination = FirmwareAddressIncrement24(destination, 1);
        index++;
    }
}

/* RAM:fa5286 */
void ClearControllerBufferRecordZonePresenceBit(u16 group_index, u16 zone_index)
{
    u32 group_base;
    fw_addr_t record_page;
    u8 page_index;

    if(zone_index < 8u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA17, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA17) & (u8) ~((u8)1u << (u8)zone_index)));

    group_base = (u32)group_index << 14;
    record_page = ((fw_addr_t)((u16)(group_base >> 16) + 0x30u) << 16) | (fw_addr_t)(u16)group_base;
    for(page_index = 0; page_index < 8u; ++page_index)
        CopyBytesToAddressPlus1MiB(FirmwareAddressIncrement24(record_page, ((fw_addr_t)page_index << 11) + 0x0ful), FW_NEAR_UNKNOWN_FA17, 1u);
}

/* RAM:fee676 */
void CopyFarWordWindowsWithByteStride(fw_addr_t source_address, fw_addr_t destination_address, u16 window_count)
{
    while(window_count != 0)
    {
        FirmwareWriteBigEndianU16Full(destination_address, FirmwareReadBigEndianU16Full(source_address));
        source_address = FirmwareAddressIncrement24(source_address, 1);
        destination_address = FirmwareAddressIncrement24(destination_address, 1);
        window_count--;
    }
}

/* RAM:fe02bd */
/* RAM:feebf7 */
void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT
{
    u16 index = 0;

    while(index < byte_count)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        source = FirmwareAddressIncrementLow16(source, 1);
        destination = FirmwareAddressIncrementLow16(destination, 1);
        index++;
    }
}

/* RAM:fde0a6 */
/* Source ABI: DR0 destination, DR4 controller offset, pushed word count;
 * returns offset+count in DR4 (used by MODE SELECT), ERET. The five caller
 * bodies keep the count slot outside the destination window. C models that
 * count as a value, with no claim for caller-stack overwrite aliases. */
u32 CopyFromControllerBufferWindow(u32 controller_offset, fw_addr_t destination, u16 byte_count)
{
    fw_addr_t source = FirmwareAddressIncrement24(controller_offset, FW_ADDRESS_BANK_STRIDE);
    u32 end_offset = controller_offset + byte_count;
    u16 index;
    destination = FirmwareAddressIncrement24(destination, 0u);
    for(index = 0u; index < byte_count; ++index)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        destination = FirmwareAddressIncrementLow16(destination, 1u);
        source = FirmwareAddressIncrement24(source, 1u);
    }
    return end_offset;
}

/* RAM:fddd78 */
/* Source ABI: DR0 source, DR4 controller offset, pushed dword count;
 * returns offset+initial count in DR4, ERET, R15 preserved. The original
 * rereads its count slot each iteration; this C value interface requires
 * that the destination does not overwrite the caller's count slot. */
u32 CopyToControllerBufferWindow(fw_addr_t source, u32 controller_offset, u32 byte_count)
{
    fw_addr_t destination = FirmwareAddressIncrement24(controller_offset, FW_ADDRESS_BANK_STRIDE);
    u32 end_offset = controller_offset + byte_count;
    u32 index;
    source = FirmwareAddressIncrement24(source, 0u);
    for(index = 0ul; index < byte_count; ++index)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        source = FirmwareAddressIncrementLow16(source, 1u);
        destination = FirmwareAddressIncrement24(destination, 1u);
    }
    return end_offset;
}

/* RAM:fef103 */
u8 AreByteRangesEqual(fw_addr_t left, fw_addr_t right, u16 byte_count)
{
    u16 index = 0;

    while(index < byte_count)
    {
        if(FirmwareRead8(FirmwareAddressIncrement24(left, index)) != FirmwareRead8(FirmwareAddressIncrement24(right, index)))
            return 0;
        index++;
    }
    return 1;
}

/* RAM:fa30cf */
void CopyDwordsToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u16 dword_count)
{
    fw_addr_t destination = FirmwareAddressIncrement24(destination_base, FW_ADDRESS_BANK_STRIDE);

    while(dword_count != 0)
    {
        u16 high_word = FirmwareReadBigEndianU16Full(FirmwareAddressIncrement24(source, 2));
        u16 low_word = FirmwareReadBigEndianU16Full(source);

        FirmwareWriteBigEndianU16Full(FirmwareAddressIncrement24(destination, 2), high_word);
        FirmwareWriteBigEndianU16Full(destination, low_word);
        source = FirmwareAddressIncrement24(source, 4);
        destination = FirmwareAddressIncrement24(destination, 4);
        dword_count--;
    }
}

/* RAM:fc2a18 */
void PLEXTOR_NEAR_FUNCTION CopyBytes8(fw_addr_t destination, fw_addr_t source, u8 byte_count)
{
    CopyBytes8LoopCondition(destination, source, 0, byte_count);
}

/* RAM:fc2a32 */
void CopyBytes8LoopCondition(fw_addr_t destination, fw_addr_t source, u8 current_index, u8 exclusive_end)
{
    while(current_index < exclusive_end)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        destination = FirmwareAddressIncrementLow16(destination, 1);
        source = FirmwareAddressIncrementLow16(source, 1);
        current_index++;
    }
}

/* RAM:fe02da */
void CopyThreeBytes16(u16 destination, u16 source)
{
    FirmwareWrite8(destination, FirmwareRead8(source));
    destination++;
    source++;
    FirmwareWrite8(destination, FirmwareRead8(source));
    destination++;
    source++;
    FirmwareWrite8(destination, FirmwareRead8(source));
}

/* RAM:fee66c */
void FillFarBytesNonzeroCount(u8 byte_value, u32 byte_count, fw_addr_t destination)
{
    /* The original do/while relies on the caller's nonzero-count precondition. */
    do
    {
        FirmwareWrite8(destination, byte_value);
        destination++;
        byte_count--;
    } while(byte_count != 0);
}


/* RAM:fbf736 */
void WriteControllerBufferWindowByteAtomic(fw_addr_t buffer_offset, u8 value)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    FirmwareWrite8(FirmwareAddressIncrement24(buffer_offset, FW_ADDRESS_BANK_STRIDE), value);
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:fd5012 */
void CopyRamBytesToReadBufferSnapshot(fw_addr_t controller_offset, fw_addr_t ram_source, u16 byte_count)
{
    u16 index;

    for(index = 0u; index < byte_count; ++index)
    {
        u8 value = FirmwareRead8(ram_source);

        WriteControllerBufferWindowByteAtomic(controller_offset, value);
        controller_offset = FirmwareAddressIncrement24(controller_offset, 1u);
        ram_source = FirmwareAddressIncrement24(ram_source, 1u);
    }
}

/* RAM:fd5060 */
void CopyReadBufferSnapshotBytesToRam(fw_addr_t controller_offset, fw_addr_t ram_destination, u16 byte_count)
{
    u16 index;

    for(index = 0u; index < byte_count; ++index)
    {
        u8 value = ReadControllerBufferByteAtomic(controller_offset);

        FirmwareWrite8(ram_destination, value);
        controller_offset = FirmwareAddressIncrement24(controller_offset, 1u);
        ram_destination = FirmwareAddressIncrement24(ram_destination, 1u);
    }
}

static void CopyControllerBufferPageToUpperBankedRegion(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker)
{
    fw_addr_t destination_base;

    if(write_marker != 0u)
        FirmwareWrite8(source_address, 0x4du);

    destination_base = FW_NEAR_UNKNOWN_4009 + ((fw_addr_t)bank_index << 14) + ((fw_addr_t)page_index << 11);
    CopyDwordsToAddressPlus1MiB(destination_base, source_address, 0x0200u);
}

/* RAM:fcd87e */
/* The historical symbol says 512 bytes, but the native caller supplies
 * 0x200 four-byte units to RAM:fa30cf, so each page is 0x800 bytes.  The two
 * known callers bound both indices to zero through seven. */
void CopyControllerBuffer512BytePageToUpperBankedRegion(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker)
{

#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/memory_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    CopyControllerBufferPageToUpperBankedRegion(source_address, bank_index, page_index, write_marker);
}

/* C251's ordinary four-argument ABI overlaps DR4 with R6/R7, so a C caller
 * cannot populate the native DR0/ACC/R7/R6 tuple through an assembly bridge
 * without destroying page_index and write_marker.  Keep the canonical entry
 * above for native callers and use this ordinary C entry for reconstructed
 * callers. */
void CopyControllerBufferPageToUpperBankedRegionForC(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker)
{
    CopyControllerBufferPageToUpperBankedRegion(source_address, bank_index, page_index, write_marker);
}

/* RAM:faa80c */
/* Despite the historical name, RAM:fde0a6 proves that this copies one
 * controller-window bank/page into the fixed CPU workspace at RAM:2ba62f.
 * Bank values outside zero through seven retain the
 * inherited DR12 base. */
void CopyControllerRecordWorkspacePageToBankedRegion(u8 bank_index, u8 page_index)
{
    u32 inherited_dr12 = CAPTURE_CONTROLLER_RECORD_PAGE_COPY_DR12();
    u32 controller_offset = inherited_dr12;

    if(bank_index < 8u)
        controller_offset = (u32)bank_index << 14;
    controller_offset += FW_HUGE_UNKNOWN_094000 + ((u32)page_index << 11);
    (void)CopyFromControllerBufferWindow(controller_offset, FW_HUGE_UNKNOWN_2BA62F, 0x0800u);
    RESTORE_CONTROLLER_RECORD_PAGE_COPY_DR12(inherited_dr12);
}
