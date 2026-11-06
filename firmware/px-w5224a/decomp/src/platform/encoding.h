#ifndef PLEXTOR_ENCODING_H
#define PLEXTOR_ENCODING_H

#include "mcs251.h"

u8 PackedBcdByteToBinary(u8 value);
u8 BinaryByteToPackedBcd(u8 value);
u8 PLEXTOR_NEAR_FUNCTION DecodeAsciiHexNibbleOrZero(u8 character);
u16 DecodeUppercaseAsciiHexNibbleOrZero(u8 character);
u8 EncodeSignedByteAsSignMagnitude(s8 value);
s8 DecodeControllerEncodedSignedOffset(u8 encoded_offset);
u8 IsBinaryMsfTupleValid(fw_addr_t tuple_address);
u8 PLEXTOR_NEAR_FUNCTION MapOneHotByteToOneBasedIndexOrFF(u8 one_hot_value);
u16 MapByteNibblesToSevenSegmentPatterns(u8 value);
void PLEXTOR_NEAR_FUNCTION SwapByteOrderInWords(fw_addr_t buffer_address, u16 word_count);
void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
void ConvertFramePositionToBinaryMsfTupleFar(s32 frame_position, fw_addr_t output_tuple_address);
void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
void PLEXTOR_NEAR_FUNCTION ConvertThreePackedBcdBytesToBinary(u16 tuple_address);
void IncrementPackedBcdMsf(u16 packed_bcd_msf_address);
void DecrementPackedBcdMsf(u16 packed_bcd_msf_address);
s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address);
s32 ConvertBinaryMsfValuesToAdjustedFramePosition(u8 minute, u8 second, u8 frame);
s32 ConvertBinaryMsfToAdjustedFramePositionAndCaptureFrameByte(fw_addr_t tuple_address, u8 *frame_byte);
void ComputeBinaryMsfTupleDifference(u16 minuend_address, u16 subtrahend_address, u16 destination);
u8 CompareBinaryMsfTuples(u16 left_tuple_address, u16 right_tuple_address);
u32 DecodeThreeBytePackedBcdBase100Value(u32 packed_bcd_value);
u32 PLEXTOR_NEAR_FUNCTION ParseUppercaseAsciiHexU32(u8 character_count, fw_addr_t source_address);
void BuildCrc16TablePolynomial1021(void);

void AdvanceRawReadQPacketMsfTuples(u8 reverse_relative);

#endif
