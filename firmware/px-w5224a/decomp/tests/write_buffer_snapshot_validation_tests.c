#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/write_buffer_snapshot_validation.h"

static u8 firmware[0x10000u];
static u8 controller_buffer[0x28e2u];
static fw_addr_t read_offsets[32];
static u8 read_count;
static u16 checksum_result;
static u8 checksum_call_count;
static fw_addr_t checksum_start;
static u8 valid_set_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(firmware));
    return firmware[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware));
    firmware[address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

u8 ReadControllerBufferByteAtomic(fw_addr_t buffer_offset)
{
    assert(buffer_offset < sizeof(controller_buffer));
    assert(read_count < sizeof(read_offsets) / sizeof(read_offsets[0]));
    read_offsets[read_count++] = buffer_offset;
    return controller_buffer[buffer_offset];
}

u16 CalculateControllerBufferChecksum28E0(fw_addr_t start_offset)
{
    ++checksum_call_count;
    checksum_start = start_offset;
    return checksum_result;
}

void SetWriteBufferSnapshotValid(void)
{
    ++valid_set_count;
}

static void InitializeValidSnapshot(void)
{
    u8 index;

    memset(firmware, 0, sizeof(firmware));
    memset(controller_buffer, 0, sizeof(controller_buffer));
    memset(read_offsets, 0, sizeof(read_offsets));
    read_count = 0u;
    checksum_result = 0x6a31u;
    checksum_call_count = 0u;
    checksum_start = 0xfffffful;
    valid_set_count = 0u;

    controller_buffer[0] = 0x28u;
    controller_buffer[1] = 0xe2u;
    for(index = 0u; index < 8u; ++index)
    {
        firmware[0xdccdu + index] = (u8)(0x20u + index);
        controller_buffer[2u + index] = firmware[0xdccdu + index];
    }
    for(index = 0u; index < 0x10u; ++index)
    {
        firmware[0xdcd5u + index] = (u8)(0x40u + index);
        controller_buffer[10u + index] = firmware[0xdcd5u + index];
    }
    controller_buffer[0x28e0u] = 0x6au;
    controller_buffer[0x28e1u] = 0x31u;
}

static void AssertNoPublication(void)
{
    assert(valid_set_count == 0u);
}

static void TestExpectedLengthMismatchStopsAfterHeader(void)
{
    InitializeValidSnapshot();
    assert(ValidateWriteBufferSnapshot(0x010028e2ul) == 0u);
    assert(read_count == 2u);
    assert(read_offsets[0] == 0u);
    assert(read_offsets[1] == 1u);
    assert(checksum_call_count == 0u);
    AssertNoPublication();
}

static void TestRequiredLengthMismatchStopsAfterHeader(void)
{
    InitializeValidSnapshot();
    controller_buffer[0] = 0x12u;
    controller_buffer[1] = 0x34u;
    assert(ValidateWriteBufferSnapshot(0x1234ul) == 0u);
    assert(read_count == 2u);
    assert(checksum_call_count == 0u);
    AssertNoPublication();
}

static void TestVendorMismatchShortCircuitsAtFirstDifference(void)
{
    InitializeValidSnapshot();
    controller_buffer[5] ^= 1u;
    assert(ValidateWriteBufferSnapshot(0x28e2ul) == 0u);
    assert(read_count == 6u);
    assert(read_offsets[5] == 5u);
    assert(checksum_call_count == 0u);
    AssertNoPublication();
}

static void TestProductMismatchShortCircuitsAtFirstDifference(void)
{
    InitializeValidSnapshot();
    controller_buffer[15] ^= 1u;
    assert(ValidateWriteBufferSnapshot(0x28e2ul) == 0u);
    assert(read_count == 16u);
    assert(read_offsets[15] == 15u);
    assert(checksum_call_count == 0u);
    AssertNoPublication();
}

static void TestChecksumMismatchDoesNotPublishValidity(void)
{
    InitializeValidSnapshot();
    checksum_result ^= 1u;
    assert(ValidateWriteBufferSnapshot(0x28e2ul) == 0u);
    assert(read_count == 28u);
    assert(read_offsets[26] == 0x28e0u);
    assert(read_offsets[27] == 0x28e1u);
    assert(checksum_call_count == 1u);
    assert(checksum_start == 0u);
    AssertNoPublication();
}

static void TestValidSnapshotPublishesAndReturnsTrue(void)
{
    InitializeValidSnapshot();
    assert(ValidateWriteBufferSnapshot(0x28e2ul) == 1u);
    assert(read_count == 28u);
    assert(checksum_call_count == 1u);
    assert(checksum_start == 0u);
    assert(valid_set_count == 1u);
}

int main(void)
{
    TestExpectedLengthMismatchStopsAfterHeader();
    TestRequiredLengthMismatchStopsAfterHeader();
    TestVendorMismatchShortCircuitsAtFirstDifference();
    TestProductMismatchShortCircuitsAtFirstDifference();
    TestChecksumMismatchDoesNotPublishValidity();
    TestValidSnapshotPublishesAndReturnsTrue();
    puts("write buffer snapshot validation tests passed");
    return 0;
}
