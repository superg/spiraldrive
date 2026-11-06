#include "encoding.h"
#include "arithmetic.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/encoding_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fa824f */
/* ACC input/result. 0xff is a sentinel for zero; 0x80 is negative zero. */
s8 DecodeControllerEncodedSignedOffset(u8 encoded_offset)
{
    if(encoded_offset == 0xffu)
        return 0;
    if((encoded_offset & 0x80u) != 0u)
        return (s8)(128 - (s16)encoded_offset);
    return (s8)encoded_offset;
}

/* Reconstruction helper; no direct Ghidra function address.
 * Restoring division is explicit to avoid importing the C251 long-division
 * runtime routine into the source-only image. */
static u32 DivideU32ByU16(u32 dividend, u16 divisor, u16 *remainder)
{
    u32 quotient = 0;
    u16 partial = 0;
    u8 bit_count = 32;

    do
    {
        partial = (u16)((partial << 1) | (u16)(dividend >> 31));
        dividend <<= 1;
        quotient <<= 1;
        if(partial >= divisor)
        {
            partial = (u16)(partial - divisor);
            quotient |= 1ul;
        }
        bit_count--;
    } while(bit_count != 0);
    *remainder = partial;
    return quotient;
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 FirmwareReadBigEndianS32(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* RAM:fee831 */
u8 PackedBcdByteToBinary(u8 value)
{
    return (u8)(((value >> 4) * 10u) + (value & 0x0fu));
}

/* RAM:feee69 */
u8 BinaryByteToPackedBcd(u8 value)
{
    return (u8)(value + ((value / 10u) * 6u));
}

/* RAM:fe3243 */
u8 PLEXTOR_NEAR_FUNCTION DecodeAsciiHexNibbleOrZero(u8 character)
{
    if(character >= '0' && character <= '9')
        return (u8)(character - '0');
    if(character >= 'A' && character <= 'F')
        return (u8)(character - ('A' - 10));
    if(character >= 'a' && character <= 'f')
        return (u8)(character - ('a' - 10));
    return 0;
}

/* RAM:fda6c7 */
u16 DecodeUppercaseAsciiHexNibbleOrZero(u8 character)
{
    if(character >= 'A' && character <= 'F')
        return (u16)(character - ('A' - 10));
    if(character >= '0' && character <= '9')
        return (u16)(character - '0');
    return 0;
}

/* RAM:fa823a */
u8 EncodeSignedByteAsSignMagnitude(s8 value)
{
    if(value < 0)
        return (u8)((u8)(0u - (u8)value) | 0x80u);
    return (u8)value;
}

/* RAM:fd4de2 */
u8 IsBinaryMsfTupleValid(fw_addr_t tuple_address)
{
    return (u8)(FirmwareRead8(tuple_address) < 150u && FirmwareRead8(FirmwareAddressIncrementLow16(tuple_address, 1)) < 60u && FirmwareRead8(FirmwareAddressIncrementLow16(tuple_address, 2)) < 75u);
}

/* RAM:fee617 */
u8 PLEXTOR_NEAR_FUNCTION MapOneHotByteToOneBasedIndexOrFF(u8 one_hot_value)
{
    u8 index = 0;
    u8 result;

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/encoding_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(one_hot_value == 0)
    {
        result = 0u;
    }
    else
    {
        while((one_hot_value & 1u) == 0u)
        {
            one_hot_value >>= 1;
            index++;
        }
        result = one_hot_value == 1u ? (u8)(index + 1u) : 0xffu;
    }
#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/encoding_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    return result;
}

/* RAM:fee5e0 */
/* table fee607..fee616.
 * The original returns the high-nibble pattern in R2 and the low-nibble
 * pattern in R3, which together form the ordinary big-endian WR2 value. */
u16 MapByteNibblesToSevenSegmentPatterns(u8 value)
{
    static const u8 patterns[16] = { 0x3fu, 0x06u, 0x5bu, 0x4fu, 0x66u, 0x6du, 0x7du, 0x27u, 0x7fu, 0x6fu, 0x77u, 0x7cu, 0x33u, 0x5eu, 0x79u, 0x71u };

    return (u16)(((u16)patterns[value >> 4] << 8) | patterns[value & 0x0fu]);
}

/* RAM:fdb4b6 */
void PLEXTOR_NEAR_FUNCTION SwapByteOrderInWords(fw_addr_t buffer_address, u16 word_count)
{
    u8 first;
    u8 second;

    while(word_count != 0)
    {
        first = FirmwareRead8(buffer_address);
        second = FirmwareRead8(FirmwareAddressIncrementLow16(buffer_address, 1));
        FirmwareWrite8(buffer_address, second);
        FirmwareWrite8(FirmwareAddressIncrementLow16(buffer_address, 1), first);
        buffer_address = FirmwareAddressIncrementLow16(buffer_address, 2);
        word_count--;
    }
}

/* RAM:feeb12 */
void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address)
{
    u32 position;
    u16 frames;
    u16 seconds;
    u32 minutes;
    u16 output_address = (u16)output_tuple_address;

    if(frame_position < 0)
        frame_position += 450000l;
    position = (u32)frame_position;
    minutes = DivideU32ByU16(position, 75u, &frames);
    minutes = DivideU32ByU16(minutes, 60u, &seconds);
    FirmwareWriteNear8(output_address, (u8)minutes);
    FirmwareWriteNear8((u16)(output_address + 1u), (u8)seconds);
    FirmwareWriteNear8((u16)(output_address + 2u), (u8)frames);
}

/* RAM:feeb80 */
void ConvertFramePositionToBinaryMsfTupleFar(s32 frame_position, fw_addr_t output_tuple_address)
{
    u32 position;
    u16 frames;
    u16 seconds;
    u32 minutes;

    if(frame_position < 0)
        frame_position += 450000l;
    position = (u32)frame_position;
    minutes = DivideU32ByU16(position, 75u, &frames);
    minutes = DivideU32ByU16(minutes, 60u, &seconds);

    /* The original stores frame, minute, then second.  Its DR0 increments
     * preserve a full 24-bit address rather than a bank-fixed low word. */
    FirmwareWrite8(FirmwareAddressIncrement24(output_tuple_address, 2), (u8)frames);
    FirmwareWrite8(output_tuple_address, (u8)minutes);
    FirmwareWrite8(FirmwareAddressIncrement24(output_tuple_address, 1), (u8)seconds);
}

/* RAM:feec14 */
void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address)
{
    FirmwareWrite8(tuple_address, BinaryByteToPackedBcd(FirmwareRead8(tuple_address)));
    FirmwareWriteNear8((u16)(tuple_address + 1u), BinaryByteToPackedBcd(FirmwareReadNear8((u16)(tuple_address + 1u))));
    FirmwareWriteNear8((u16)(tuple_address + 2u), BinaryByteToPackedBcd(FirmwareReadNear8((u16)(tuple_address + 2u))));
}

/* RAM:feec78 */
void PLEXTOR_NEAR_FUNCTION ConvertThreePackedBcdBytesToBinary(u16 tuple_address)
{
    FirmwareWrite8(tuple_address, PackedBcdByteToBinary(FirmwareRead8(tuple_address)));
    FirmwareWriteNear8((u16)(tuple_address + 1u), PackedBcdByteToBinary(FirmwareReadNear8((u16)(tuple_address + 1u))));
    FirmwareWriteNear8((u16)(tuple_address + 2u), PackedBcdByteToBinary(FirmwareReadNear8((u16)(tuple_address + 2u))));
}

/* RAM:feecc1 */
void IncrementPackedBcdMsf(u16 packed_bcd_msf_address)
{
    fw_addr_t address = packed_bcd_msf_address;
    u8 raw_frame = FirmwareRead8(address + 2ul);
    u8 raw_second = FirmwareRead8(address + 1ul);
    u8 minute = FirmwareRead8(address);
    u8 frame = (u8)(PackedBcdByteToBinary(raw_frame) + 1u);
    u8 second;

    if(frame != 75u)
    {
        FirmwareWrite8(address + 2ul, BinaryByteToPackedBcd(frame));
        return;
    }
    FirmwareWrite8(address + 2ul, 0u);
    second = (u8)(PackedBcdByteToBinary(raw_second) + 1u);
    if(second != 60u)
    {
        FirmwareWrite8(address + 1ul, BinaryByteToPackedBcd(second));
        return;
    }
    FirmwareWrite8(address + 1ul, 0u);
    if(minute == 0x99u)
    {
        FirmwareWrite8(address, FirmwareReadNear8(FW_NEAR_UNKNOWN_1033) == 0u ? 0u : 0xa0u);
        return;
    }
    FirmwareWrite8(address, BinaryByteToPackedBcd((u8)(PackedBcdByteToBinary(minute) + 1u)));
}

/* RAM:feed99 */
void DecrementPackedBcdMsf(u16 packed_bcd_msf_address)
{
    fw_addr_t address = packed_bcd_msf_address;
    u8 raw_frame = FirmwareRead8(address + 2ul);
    u8 raw_second = FirmwareRead8(address + 1ul);
    u8 minute = FirmwareRead8(address);
    u8 frame = (u8)(PackedBcdByteToBinary(raw_frame) - 1u);
    u8 second;

    if(frame != 0xffu)
    {
        FirmwareWrite8(address + 2ul, BinaryByteToPackedBcd(frame));
        return;
    }
    frame = 74u;
    second = (u8)(PackedBcdByteToBinary(raw_second) - 1u);
    if(second == 0xffu)
    {
        second = 59u;
        if(minute == 0u)
        {
            FirmwareWrite8(address + 2ul, 0x74u);
            FirmwareWrite8(address + 1ul, 0x59u);
            FirmwareWrite8(address, 0x99u);
            return;
        }
        minute = (u8)(PackedBcdByteToBinary(minute) - 1u);
        FirmwareWrite8(address, BinaryByteToPackedBcd(minute));
    }
    FirmwareWrite8(address + 1ul, BinaryByteToPackedBcd(second));
    FirmwareWrite8(address + 2ul, BinaryByteToPackedBcd(frame));
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ConvertBinaryMsfToAdjustedFramePositionInternal(fw_addr_t tuple_address, u8 *frame_byte)
{
    u8 minute = FirmwareRead8(tuple_address);
    u8 second = FirmwareRead8(FirmwareAddressIncrementLow16(tuple_address, 1));
    u8 frame = FirmwareRead8(FirmwareAddressIncrementLow16(tuple_address, 2));

    if(frame_byte != 0)
        *frame_byte = frame;
    return ConvertBinaryMsfValuesToAdjustedFramePosition(minute, second, frame);
}

/* Reconstruction helper for native callers that stage MSF bytes on their
 * private stack before applying the common conversion policy. */
s32 ConvertBinaryMsfValuesToAdjustedFramePosition(u8 minute, u8 second, u8 frame)
{
    u16 total_seconds = (u16)((u16)minute * 60u + second);
    s32 frame_position = (s32)(((u32)total_seconds << 6) + ((u32)total_seconds << 3) + ((u32)total_seconds << 1) + total_seconds + frame);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_102F) == 1 && ReadSfrD1Bit5ForMsfAdjustment() == 0u)
    {
        frame_position -= 450000l;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) != 0)
        {
            s32 lead_in_start = FirmwareReadBigEndianS32(FW_NEAR_UNKNOWN_114F);
            if(lead_in_start != 0 && frame_position < lead_in_start - 0x47el)
                frame_position += 0x8cal;
        }
    }
    return frame_position;
}

/* RAM:fee841 */
s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address)
{
    return ConvertBinaryMsfToAdjustedFramePositionInternal(tuple_address, 0);
}

/* Reconstruction helper; no direct Ghidra function address. */
s32 ConvertBinaryMsfToAdjustedFramePositionAndCaptureFrameByte(fw_addr_t tuple_address, u8 *frame_byte)
{
    return ConvertBinaryMsfToAdjustedFramePositionInternal(tuple_address, frame_byte);
}

/* RAM:fe17e1 */
void ComputeBinaryMsfTupleDifference(u16 minuend_address, u16 subtrahend_address, u16 destination)
{
    s32 difference = ConvertBinaryMsfToAdjustedFramePosition(minuend_address) - ConvertBinaryMsfToAdjustedFramePosition(subtrahend_address);
    ConvertFramePositionToBinaryMsfTuple(difference, destination);
}

/* RAM:fe1818 */
u8 CompareBinaryMsfTuples(u16 left_tuple_address, u16 right_tuple_address)
{
    s32 left = ConvertBinaryMsfToAdjustedFramePosition(left_tuple_address);
    s32 right = ConvertBinaryMsfToAdjustedFramePosition(right_tuple_address);

    if(left == right)
        return 0;
    return left < right ? 1 : 2;
}

/* RAM:fc49c9 */
u32 DecodeThreeBytePackedBcdBase100Value(u32 packed_bcd_value)
{
    u32 low = PackedBcdByteToBinary((u8)packed_bcd_value);
    u32 middle = PackedBcdByteToBinary((u8)(packed_bcd_value >> 8));
    u32 high = PackedBcdByteToBinary((u8)(packed_bcd_value >> 16));

    return low + MultiplyU32Low(100ul, middle) + MultiplyU32Low(10000ul, high);
}

/* RAM:fda6f0 */
u32 PLEXTOR_NEAR_FUNCTION ParseUppercaseAsciiHexU32(u8 character_count, fw_addr_t source_address)
{
    u32 result = 0;
    u8 remaining = character_count;

    while(remaining != 0)
    {
        u32 digit;
        u8 shift_count;

        u8 byte_offset = (u8)(remaining - 1u);
        shift_count = (u8)((character_count - remaining) << 2);
        digit = DecodeUppercaseAsciiHexNibbleOrZero(FirmwareRead8(FirmwareAddressIncrementLow16(source_address, byte_offset)));
        while(shift_count != 0)
        {
            digit <<= 1;
            shift_count--;
        }
        result += digit;
        remaining--;
    }
    return result;
}

/* RAM:fdc385 */
void BuildCrc16TablePolynomial1021(void)
{
    u16 table_index;

    for(table_index = 0; table_index != 0x100u; ++table_index)
    {
        u16 value = (u16)(table_index << 8);
        u8 bit_index;
        fw_addr_t entry_address;

        for(bit_index = 0; bit_index != 8u; ++bit_index)
            if((value & 0x8000u) != 0)
                value = (u16)((value << 1) ^ 0x1021u);
            else
                value <<= 1;
        entry_address = (fw_addr_t)(FW_NEAR_UNKNOWN_F596 + ((u32)table_index << 1));
        FirmwareWrite8(entry_address, (u8)(value >> 8));
        FirmwareWrite8(FirmwareAddressIncrementLow16(entry_address, 1), (u8)value);
    }
}

/* RAM:fe9ecc */
/* ACC exact-byte selector; ERET, preserves R15. Absolute tuple first. */
void AdvanceRawReadQPacketMsfTuples(u8 reverse_relative)
{
    IncrementPackedBcdMsf(FW_NEAR_UNKNOWN_1279);
    if(reverse_relative == 1u)
        DecrementPackedBcdMsf(0x1275u);
    else
        IncrementPackedBcdMsf(FW_NEAR_UNKNOWN_1275);
}
