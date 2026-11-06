#ifndef PLEXTOR_MEMORY_H
#define PLEXTOR_MEMORY_H

#include "mcs251.h"

u16 ReadBigEndianU16(fw_addr_t source_address);
void WriteBigEndianU16(fw_addr_t destination_address, u16 value);
void ZeroMemoryWords(fw_addr_t destination, u16 byte_count);
void CopyBytes16(u16 destination, u16 source, u8 byte_count);
void CopyBytesWith8BitCount(fw_addr_t destination, fw_addr_t source, u8 byte_count);
void CopyBytesWithIndexRange(fw_addr_t destination, fw_addr_t source, u8 current_index, u8 exclusive_end);
void CopySixBytesFar(fw_addr_t destination, fw_addr_t source);
void PLEXTOR_NEAR_FUNCTION CopyTenBytesFar(fw_addr_t destination, fw_addr_t source);
void PLEXTOR_NEAR_FUNCTION CopyBinaryTocPayloadToFullTocDescriptorBody(fw_addr_t source_packet_address, fw_addr_t destination_descriptor_body_address);
void CopyBytesFromAddressPlus1MiB(fw_addr_t source_base, fw_addr_t destination, u8 byte_count);
void CopyBytesToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u8 byte_count);
void ClearControllerBufferRecordZonePresenceBit(u16 group_index, u16 zone_index);
void CopyFarWordWindowsWithByteStride(fw_addr_t source_address, fw_addr_t destination_address, u16 window_count);
void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
u32 CopyFromControllerBufferWindow(u32 controller_offset, fw_addr_t destination, u16 byte_count);
u32 CopyToControllerBufferWindow(fw_addr_t source, u32 controller_offset, u32 byte_count);
u8 AreByteRangesEqual(fw_addr_t left, fw_addr_t right, u16 byte_count);
void CopyWordRangeAtOneMiBOffsets(fw_addr_t source, fw_addr_t destination, u32 word_count);
void CopyDwordsToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u16 dword_count);
void PLEXTOR_NEAR_FUNCTION CopyBytes8(fw_addr_t destination, fw_addr_t source, u8 byte_count);
void CopyBytes8LoopCondition(fw_addr_t destination, fw_addr_t source, u8 current_index, u8 exclusive_end);
void CopyThreeBytes16(u16 destination, u16 source);
void FillFarBytesNonzeroCount(u8 byte_value, u32 byte_count, fw_addr_t destination);
void PLEXTOR_NEAR_FUNCTION StoreR3AtSegmentRelativeE789PlusSelf(u8 value_and_index);
void WriteControllerBufferWindowByteAtomic(fw_addr_t buffer_offset, u8 value);
void CopyRamBytesToReadBufferSnapshot(fw_addr_t controller_offset, fw_addr_t ram_source, u16 byte_count);
void CopyReadBufferSnapshotBytesToRam(fw_addr_t controller_offset, fw_addr_t ram_destination, u16 byte_count);
void CopyControllerBuffer512BytePageToUpperBankedRegion(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker);
void CopyControllerBufferPageToUpperBankedRegionForC(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker);
void CopyControllerRecordWorkspacePageToBankedRegion(u8 bank_index, u8 page_index);

#endif
