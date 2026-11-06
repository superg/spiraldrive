#include "spc.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

/* Fixed data: RAM:ff95f8..RAM:ff9607.
 * The native loop reads these fixed-ROM bytes through DR0.  Keep a source
 * representation local to the function rather than conflating this expected
 * value with the mutable INQUIRY response buffer at RAM:00dcd5. */
static const u8 kExpectedInquiryProductIdentification[16] = { 0x43u, 0x44u, 0x2du, 0x52u, 0x20u, 0x20u, 0x20u, 0x50u, 0x58u, 0x2du, 0x57u, 0x34u, 0x38u, 0x32u, 0x34u, 0x41u };

/* RAM:fd6882 */
/* At its native entry, R7 < 16 branches into the shared comparison loop at
 * RAM:fd685e.  The only direct caller, the thunk at RAM:fd685a, clears R7 and
 * therefore compares all sixteen bytes.  A mismatch clears B and returns ACC
 * zero; a complete match returns ACC six. */
u8 CompareInquiryProductIdentificationFromIndex(u8 starting_index, u8 prior_match)
{
    u8 index = starting_index;
    u8 matched = prior_match;

    while(index < 16u)
    {
        if(FirmwareReadNear8((fw_addr_t)(FW_NEAR_UNKNOWN_DCD5 + index)) != kExpectedInquiryProductIdentification[index])
            return 0u;
        matched = 1u;
        index = (u8)(index + 1u);
    }

    return matched != 0u ? 6u : 0u;
}

/* RAM:fd685a */
u8 CompareFullInquiryProductIdentification(void)
{
    return CompareInquiryProductIdentificationFromIndex(0u, 0u);
}

/* RAM:f9b9e0 */
u8 PLEXTOR_NEAR_FUNCTION AppendDriveSerialNumberFeaturePayload(fw_addr_t destination)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD25) == 0x20u)
        return 0u;
    CopyBytesWith8BitCount(destination, FW_NEAR_UNKNOWN_DD25, 6u);
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 7u), 0x20u);
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 6u), 0x20u);
    return 8u;
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteGetPerformanceU32LowWordFirst(u16 address, u32 value)
{
    FirmwareWriteNearU16((u16)(address + 2u), (u16)value);
    FirmwareWriteNearU16(address, (u16)(value >> 16));
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadGetPerformanceU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; low-word pointer increments match native DR0 use. */
static u16 ReadGetPerformanceU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; preserves the native low-word-first dword load. */
static u32 ReadReadCapacityU32LowWordFirst(u16 address)
{
    u16 low_word = FirmwareReadNearU16((u16)(address + 2u));
    u16 high_word = FirmwareReadNearU16(address);

    return ((u32)high_word << 16) | low_word;
}

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/spc_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd2635 */
void HandleReadCapacity10Command(void)
{
    u32 last_logical_block_address;
    u32 media_capacity;
    u16 logical_block_size;

    if(VALIDATE_READ_CAPACITY_10(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DB2)) == 0u)
    {
        HANDLE_UNSUPPORTED_READ_CAPACITY_10();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
    {
        media_capacity = ReadReadCapacityU32LowWordFirst(FW_NEAR_UNKNOWN_12C1);
        if(media_capacity == 0ul)
            last_logical_block_address = 1ul;
        else
            last_logical_block_address = ReadReadCapacityU32LowWordFirst(FW_NEAR_UNKNOWN_12C1) - 150ul;
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
    {
        last_logical_block_address = 0x400ul;
    }
    else
    {
        last_logical_block_address = COMPUTE_READ_CAPACITY_10_GEOMETRY();
    }

    last_logical_block_address--;
    WriteGetPerformanceU32LowWordFirst(FW_NEAR_UNKNOWN_0B2A, last_logical_block_address);
    logical_block_size = ReadGetPerformanceU16(FW_NEAR_UNKNOWN_0B00);
    WriteGetPerformanceU32LowWordFirst(FW_NEAR_UNKNOWN_0B2E, (u32)logical_block_size);
    if(TRANSFER_READ_CAPACITY_10_RESPONSE(FW_NEAR_UNKNOWN_0B2A, 8ul) != 4u)
        SET_NO_SENSE_FOR_READ_CAPACITY_10();
}

/* RAM:fc0000 */
/* The native cursor and four values use a mixed register/stack ABI. */
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceDescriptorFromFourU32Values(u16 response_length, u32 value0, u32 value1, u32 value2, u32 value3)
{
    u16 address = (u16)(0x0b2au + response_length);

    WriteGetPerformanceU32LowWordFirst(address, value0);
    address = (u16)(address + 4u);
    WriteGetPerformanceU32LowWordFirst(address, value1);
    address = (u16)(address + 4u);
    WriteGetPerformanceU32LowWordFirst(address, value2);
    address = (u16)(address + 4u);
    WriteGetPerformanceU32LowWordFirst(address, value3);
    return (u16)(response_length + 16u);
}

/* RAM:fc005d */
/* ACC, WR6, and WR4 are explicit C parameters. */
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceNominalSpeedDescriptor(u8 nominal_speed_code, u16 remaining_descriptor_count, u16 response_length)
{
    u32 start_rate;
    u32 end_rate;

    if(remaining_descriptor_count == 0u)
        return response_length;

    switch(nominal_speed_code)
    {
    case 4u:
        start_rate = 0x02c2ul;
        end_rate = 0x02c2ul;
        break;
    case 8u:
        start_rate = 0x0583ul;
        end_rate = 0x0583ul;
        break;
    case 24u:
        start_rate = 0x06e4ul;
        end_rate = 0x108aul;
        break;
    case 32u:
        start_rate = 0x09a6ul;
        end_rate = 0x160dul;
        break;
    case 40u:
        start_rate = 0x0bb7ul;
        end_rate = 0x1b90ul;
        break;
    case 52u:
        start_rate = 0x0f29ul;
        end_rate = 0x23d5ul;
        break;
    default:
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x04u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x04u) == 0u)
        {
            start_rate = 0x0bb7ul;
            end_rate = 0x1b90ul;
        }
        else
        {
            start_rate = 0x0f29ul;
            end_rate = 0x23d5ul;
        }
        break;
    }

    return AppendGetPerformanceDescriptorFromFourU32Values(response_length, 0ul, start_rate, 359999ul, end_rate);
}

/* RAM:fc0106 */
/* ACC, WR4, and WR6 are explicit C parameters. */
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceWriteSpeedDescriptorsForSpeed(u8 speed_x, u16 current_response_length, u16 descriptor_limit)
{
    u32 start_rate;
    u32 end_rate;

    if(descriptor_limit == 0u)
        return current_response_length;

    if(speed_x == 24u || speed_x == 32u)
    {
        u16 threshold_address = speed_x == 24u ? 0xf9eeu : 0xf9f2u;
        u32 upper_rate = speed_x == 24u ? 0x108aul : 0x160dul;
        u32 threshold = ReadGetPerformanceU32(threshold_address);

        current_response_length = AppendGetPerformanceDescriptorFromFourU32Values(current_response_length, 0ul, 0x0e78ul, threshold - 1ul, upper_rate);
        if(descriptor_limit > 1u)
        {
            threshold = ReadGetPerformanceU32(threshold_address);
            current_response_length = AppendGetPerformanceDescriptorFromFourU32Values(current_response_length, threshold, upper_rate, 359999ul, upper_rate);
        }
        return current_response_length;
    }

    switch(speed_x)
    {
    case 4u:
        start_rate = end_rate = 0x02c2ul;
        break;
    case 8u:
        start_rate = end_rate = 0x0583ul;
        break;
    case 10u:
        start_rate = end_rate = 0x06e4ul;
        break;
    case 12u:
        start_rate = end_rate = 0x0845ul;
        break;
    case 16u:
        start_rate = end_rate = 0x0b06ul;
        break;
    default:
        start_rate = 0x0f29ul;
        end_rate = 0x23d5ul;
        break;
    }

    return AppendGetPerformanceDescriptorFromFourU32Values(current_response_length, 0ul, start_rate, 359999ul, end_rate);
}

/* RAM:fc02b2 */
/* Native DR0 and WR6 inputs plus inherited WR14 are explicit here. */
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceDescriptorFromTableRecord(fw_addr_t performance_table_record_address, u16 response_length, u16 inherited_speed_ceiling)
{
    fw_addr_t flags_address = FirmwareAddressIncrementLow16(performance_table_record_address, 2u);
    fw_addr_t speed_address = FirmwareAddressIncrementLow16(performance_table_record_address, 3u);
    u16 record_speed = ReadGetPerformanceU16(speed_address);
    u8 flags = (FirmwareRead8(flags_address) & 0x03u) == 1u ? 1u : 0u;
    u16 current_speed = GetCdSpeedKilobytesPerSecond(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C43));
    u16 speed_ceiling = inherited_speed_ceiling;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x02u) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0x01u) != 0u)
        {
            speed_ceiling = 0x02c2u;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_158D) == 2u)
                speed_ceiling = 0x0583u;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0x01u) != 0u)
            speed_ceiling = 0x02c2u;
        if(speed_ceiling < record_speed)
            return response_length;
    }

    return AppendGetPerformanceDescriptorWithFlagsAndThreeU32Values(flags, response_length, ReadGetPerformanceU32(FW_NEAR_CURRENT_FRAME_POSITION), (u32)record_speed, (u32)current_speed);
}

/* RAM:fc024e */
/* The native cursor and final value use a mixed register/stack ABI. */
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceDescriptorWithFlagsAndThreeU32Values(u8 flags, u16 response_length, u32 value0, u32 value1, u32 value2)
{
    u16 address = (u16)(0x0b2au + response_length);

    WriteGetPerformanceU32LowWordFirst(address, 0ul);
    FirmwareWrite8(address, (u8)(flags << 3));
    address = (u16)(address + 4u);
    WriteGetPerformanceU32LowWordFirst(address, value0);
    address = (u16)(address + 4u);
    WriteGetPerformanceU32LowWordFirst(address, value1);
    address = (u16)(address + 4u);
    WriteGetPerformanceU32LowWordFirst(address, value2);
    return (u16)(response_length + 16u);
}

/* RAM:fae461 */
u8 PLEXTOR_NEAR_FUNCTION FindFirstTrackNumberForLastFullTocSession(void)
{
    u16 descriptor_count = (u16)(FirmwareReadNearU16(FW_NEAR_UNKNOWN_12CA) / 11u);
    u16 descriptor_index;

    for(descriptor_index = 1u; descriptor_index <= descriptor_count; ++descriptor_index)
    {
        u16 low_address = (u16)(0x8000u + (u16)(descriptor_index * 11u));
        fw_addr_t descriptor = FW_HUGE_UNKNOWN_2B0000 | low_address;
        fw_addr_t track_address = FW_HUGE_UNKNOWN_2B0000 | (u16)(low_address + 3u);

        if(FirmwareRead8(descriptor) == FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) && FirmwareRead8(track_address) <= 99u)
            return FirmwareRead8(track_address);
    }
    return 1u;
}
