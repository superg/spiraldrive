#include "configuration_range_update.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/configuration_range_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void DisableGlobalInterrupts(void)
{
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/configuration_range_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 ConfigurationFieldPairIsValid(void)
{
    u8 third;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1640) != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_164C) == 0xffu)
        return 1u;
    third = FirmwareReadNear8(FW_NEAR_UNKNOWN_164D);
    return (u8)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_164C) == (u16)third + 0x40u);
}

static u8 FailConfigurationUpdate(u8 failure_code)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12F7, 1u);
    PersistDriveConfigurationValidationFailureCode(failure_code);
    return 0u;
}

/* RAM:fd60f2 */
u8 UpdateAndProgramDriveConfigurationRange(fw_addr_t source, u16 destination_offset, u16 byte_count)
{
    u8 all_primary_bytes_erased = 1u;
    u8 extended_only = 0u;
    u16 index;
    u16 checksum;
    u16 stored_checksum;
    fw_addr_t source_cursor;
    fw_addr_t destination_cursor;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12F7, 0u);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_12F8, ReadBigEndianU32(FW_NEAR_UNKNOWN_12F8) + 1ul);
    DisableGlobalInterrupts();
    ReadDriveConfigurationBytes(0ul, FW_NEAR_UNKNOWN_1620, 0x00a0u);

    for(index = 0u; index < 0x60u; ++index)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1620 + index) != 0xffu)
        {
            all_primary_bytes_erased = 0u;
            break;
        }
    }
    if(!all_primary_bytes_erased)
    {
        checksum = 0u;
        for(index = 0u; index < 0x5eu; ++index)
            checksum = (u16)(checksum + FirmwareReadNear8(FW_NEAR_UNKNOWN_1620 + index));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12FC, (u8)(checksum >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12FD, (u8)checksum);
        stored_checksum = ReadBigEndianU16(FW_NEAR_UNKNOWN_167E);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12FE, (u8)(stored_checksum >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12FF, (u8)stored_checksum);
        if(stored_checksum != checksum)
            return FailConfigurationUpdate(1u);
    }

    if(!ConfigurationFieldPairIsValid())
        return FailConfigurationUpdate(2u);
    if(destination_offset > 0x60u)
        extended_only = 1u;
    if(destination_offset > 0x9fu)
        return FailConfigurationUpdate(3u);
    if(byte_count > 0xa0u)
        return FailConfigurationUpdate(5u);

    source_cursor = source;
    destination_cursor = FW_NEAR_UNKNOWN_1620 + destination_offset;
    for(index = 0u; index < byte_count; ++index)
    {
        FirmwareWrite8(destination_cursor, FirmwareRead8(source_cursor));
        source_cursor = FirmwareAddressIncrement24(source_cursor, 1u);
        destination_cursor = FirmwareAddressIncrement24(destination_cursor, 1u);
    }

    source_cursor = source;
    destination_cursor = FW_NEAR_UNKNOWN_1620 + destination_offset;
    for(index = 0u; index < byte_count; ++index)
    {
        u8 source_value = FirmwareRead8(source_cursor);

        if(FirmwareRead8(destination_cursor) != source_value)
            return FailConfigurationUpdate(6u);
        source_cursor = FirmwareAddressIncrement24(source_cursor, 1u);
        destination_cursor = FirmwareAddressIncrement24(destination_cursor, 1u);
    }

    if(!ConfigurationFieldPairIsValid())
        return FailConfigurationUpdate(7u);
    RecalculateDriveConfigurationChecksums();
    if(!ConfigurationFieldPairIsValid())
        return FailConfigurationUpdate(8u);
    if(ProgramAndVerifyDriveConfiguration(extended_only) == 0u)
        return FailConfigurationUpdate(9u);
    if(ReadDriveConfigurationWord(0x2fu) != ReadBigEndianU16(FW_NEAR_UNKNOWN_12FC))
        return FailConfigurationUpdate(10u);
    if(ReadDriveConfigurationWord(0x3fu) != ReadBigEndianU16(FW_NEAR_UNKNOWN_12FE))
        return FailConfigurationUpdate(10u);
    return 1u;
}
