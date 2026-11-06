#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/full_toc_track_span.h"

static u8 low_memory[1u << 16];
static u8 toc_memory[1u << 16];
static fw_addr_t convert_addresses[4];
static unsigned convert_count;
static fw_addr_t lead_out_source;
static unsigned lead_out_count;
static s32 start_result;
static s32 end_result;
static s32 lead_out_result;

u8 FirmwareRead8(fw_addr_t address)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        return toc_memory[(u16)address];
    return low_memory[(u16)address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        toc_memory[(u16)address] = value;
    else
        low_memory[(u16)address] = value;
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

s32 ObserveFullTocSpanConvertMsf(fw_addr_t tuple_address)
{
    convert_addresses[convert_count++] = tuple_address;
    return convert_count == 1u ? start_result : end_result;
}

s32 ObserveFullTocSpanCopyAndConvertLeadOut(fw_addr_t descriptor_msf_address)
{
    lead_out_source = descriptor_msf_address;
    ++lead_out_count;
    return lead_out_result;
}

static void ResetFixture(void)
{
    memset(low_memory, 0, sizeof(low_memory));
    memset(toc_memory, 0, sizeof(toc_memory));
    memset(convert_addresses, 0, sizeof(convert_addresses));
    convert_count = 0u;
    lead_out_source = 0ul;
    lead_out_count = 0u;
    start_result = 1000l;
    end_result = 1400l;
    lead_out_result = 1700l;
}

static void SetDescriptorCount(u16 count)
{
    u16 byte_count = (u16)(count * 11u);

    low_memory[0x12cau] = (u8)(byte_count >> 8);
    low_memory[0x12cbu] = (u8)byte_count;
}

static void SetDescriptor(u16 index, u8 session, u8 point, u8 pmin)
{
    u16 address = (u16)(0x8000u + index * 11u);

    toc_memory[address] = session;
    toc_memory[(u16)(address + 3u)] = point;
    toc_memory[(u16)(address + 8u)] = pmin;
}

static void TestNonfinalTrackUsesFollowingDecodedBoundary(void)
{
    ResetFixture();
    SetDescriptorCount(4u);
    SetDescriptor(1u, 1u, 3u, 0u);
    SetDescriptor(2u, 1u, 4u, 0u);
    SetDescriptor(3u, 1u, 0xa1u, 4u);
    SetDescriptor(4u, 1u, 0xa2u, 20u);

    assert(ComputeFullTocTrackSpanFrames(3u) == 400l);
    assert(lead_out_count == 1u && lead_out_source == 0x2b8034ul);
    assert(convert_count == 2u);
    assert(convert_addresses[0] == 0x00f2cbul);
    assert(convert_addresses[1] == 0x00f2d1ul);
}

static void TestLastTrackUsesMatchingSessionLeadOut(void)
{
    ResetFixture();
    SetDescriptorCount(6u);
    SetDescriptor(1u, 1u, 1u, 0u);
    SetDescriptor(2u, 1u, 0xa1u, 1u);
    SetDescriptor(3u, 1u, 0xa2u, 10u);
    SetDescriptor(4u, 2u, 5u, 0u);
    SetDescriptor(5u, 2u, 0xa1u, 5u);
    SetDescriptor(6u, 2u, 0xa2u, 30u);
    start_result = 1200l;
    lead_out_result = 2000l;

    assert(ComputeFullTocTrackSpanFrames(5u) == 800l);
    assert(lead_out_count == 1u && lead_out_source == 0x2b804au);
    assert(convert_count == 1u && convert_addresses[0] == 0x00f2d7ul);
}

static void TestSignedSpanArithmeticWrapsLikeNativeDwordSubtraction(void)
{
    ResetFixture();
    SetDescriptorCount(3u);
    SetDescriptor(1u, 1u, 2u, 0u);
    SetDescriptor(2u, 1u, 0xa1u, 2u);
    SetDescriptor(3u, 1u, 0xa2u, 10u);
    start_result = (s32)0x7ffffff0ul;
    lead_out_result = (s32)0x80000010ul;

    assert((u32)ComputeFullTocTrackSpanFrames(2u) == 0x20ul);
}

int main(void)
{
    TestNonfinalTrackUsesFollowingDecodedBoundary();
    TestLastTrackUsesMatchingSessionLeadOut();
    TestSignedSpanArithmeticWrapsLikeNativeDwordSubtraction();
    return 0;
}
