#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/read_acquisition_request_dispatch.h"

static u8 memory[0x20000ul];
static fw_addr_t write_addresses[64];
static u8 write_values[64];
static unsigned write_count;
static u8 bit_0008;
static u8 bit_0580;
static u8 bit_0581;
static unsigned advance_count;
static unsigned retreat_count;
static unsigned load_count;
static unsigned resume_count;
static unsigned initialize_count;
static u32 initialize_entry;
static u8 load_mutates_state;
static u8 load_state;
static u8 find_result;
static s32 find_argument;
static unsigned find_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    assert(write_count < sizeof(write_addresses) / sizeof(write_addresses[0]));
    write_addresses[write_count] = address;
    write_values[write_count] = value;
    ++write_count;
    memory[address] = value;
}

u8 FirmwareReadBit0008(void)
{
    return bit_0008;
}
void FirmwareWriteBit0580(u8 value)
{
    bit_0580 = (u8)(value != 0u);
}
void FirmwareWriteBit0581(u8 value)
{
    bit_0581 = (u8)(value != 0u);
}

void AdvanceReadAcquisitionTrackCursor(void)
{
    ++advance_count;
}
void RetreatReadAcquisitionTrackCursor(void)
{
    ++retreat_count;
}

void LoadReadAcquisitionTargetForTrackNumber(void)
{
    ++load_count;
    if(load_mutates_state != 0u)
        memory[0x0faful] = load_state;
}

u8 FindTrackNumberForFramePosition(s32 frame_position)
{
    ++find_count;
    find_argument = frame_position;
    return find_result;
}

void InitializeCoroutine1(u32 entry_point)
{
    ++initialize_count;
    initialize_entry = entry_point;
}

void ResumeCoroutine1(void)
{
    ++resume_count;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(write_addresses, 0, sizeof(write_addresses));
    memset(write_values, 0, sizeof(write_values));
    write_count = 0u;
    bit_0008 = 0u;
    bit_0580 = 0u;
    bit_0581 = 1u;
    advance_count = 0u;
    retreat_count = 0u;
    load_count = 0u;
    resume_count = 0u;
    initialize_count = 0u;
    initialize_entry = 0u;
    load_mutates_state = 0u;
    load_state = 0u;
    find_result = 7u;
    find_argument = 0;
    find_count = 0u;
}

static void EnablePrimaryRequests(u8 request_flags, u8 state)
{
    memory[0x11ddul] = 1u;
    memory[0x11e5ul] = request_flags;
    memory[0x0faful] = state;
    memory[0x0fbaul] = 0xffu;
    memory[0x0f72ul] = 1u;
}

static void TestAdmissionGates(void)
{
    ResetFixture();
    memory[0x0fbaul] = 0xffu;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0fbaul] == 0xdfu);

    ResetFixture();
    EnablePrimaryRequests(0xffu, 0u);
    memory[0x0faeul] = 1u;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0fbaul] == 0xdfu && memory[0x11e5ul] == 0xffu);

    ResetFixture();
    EnablePrimaryRequests(0xffu, 0u);
    memory[0x0f46ul] = 1u;
    memory[0x0f72ul] = 0u;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x11e5ul] == 0xffu && memory[0x0fbaul] == 0xdfu);

    ResetFixture();
    EnablePrimaryRequests(0xffu, 0u);
    bit_0008 = 1u;
    memory[0x0f72ul] = 0u;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x11e5ul] == 0xffu);
}

static void TestBit2Request(void)
{
    ResetFixture();
    EnablePrimaryRequests(0x3fu, 6u);
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x11e5ul] == 0x3bu && load_count == 0u);

    ResetFixture();
    EnablePrimaryRequests(0x04u, 2u);
    memory[0x12bful] = 0x12u;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x11e5ul] == 0u && bit_0580 == 1u && bit_0581 == 0u);
    assert(memory[0x0fbbul] == 0x12u && memory[0x0e42ul] == 0x12u);
    assert(load_count == 1u && memory[0x0faful] == 0u);
    assert(initialize_count == 1u && initialize_entry == 0x00f85d74ul);
    assert(resume_count == 1u);

    ResetFixture();
    EnablePrimaryRequests(0x04u, 2u);
    load_mutates_state = 1u;
    load_state = 0u;
    DispatchReadAcquisitionRequestFlags();
    assert(load_count == 1u && initialize_count == 0u && resume_count == 0u);
}

static void TestStateRequests(void)
{
    ResetFixture();
    EnablePrimaryRequests(0x01u, 5u);
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x11e5ul] == 0u && memory[0x0fbeul] == 2u);
    assert(memory[0x0faful] == 5u && resume_count == 0u);

    ResetFixture();
    EnablePrimaryRequests(0x01u, 0u);
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0faful] == 1u && memory[0x0e3ful] == 1u);
    assert(resume_count == 1u);

    ResetFixture();
    EnablePrimaryRequests(0x02u, 0u);
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0faful] == 3u && memory[0x0e3ful] == 1u);
    assert(resume_count == 1u);

    ResetFixture();
    EnablePrimaryRequests(0x02u, 3u);
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0faful] == 3u && resume_count == 0u);
}

static void TestCursorRequests(void)
{
    ResetFixture();
    EnablePrimaryRequests(0x08u, 2u);
    DispatchReadAcquisitionRequestFlags();
    assert(advance_count == 1u && load_count == 1u && resume_count == 1u);

    ResetFixture();
    EnablePrimaryRequests(0x08u, 2u);
    load_mutates_state = 1u;
    load_state = 0u;
    DispatchReadAcquisitionRequestFlags();
    assert(advance_count == 1u && load_count == 1u && resume_count == 0u);

    ResetFixture();
    EnablePrimaryRequests(0x20u, 0u);
    DispatchReadAcquisitionRequestFlags();
    assert(retreat_count == 1u && load_count == 1u && resume_count == 0u);

    ResetFixture();
    EnablePrimaryRequests(0x20u, 2u);
    memory[0x1218ul] = 1u;
    DispatchReadAcquisitionRequestFlags();
    assert(retreat_count == 1u && load_count == 1u && resume_count == 1u);

    ResetFixture();
    EnablePrimaryRequests(0x20u, 2u);
    memory[0x1218ul] = 0xffu;
    DispatchReadAcquisitionRequestFlags();
    assert(retreat_count == 0u && load_count == 1u && resume_count == 1u);
}

static void TestFixedTargetRequest(void)
{
    ResetFixture();
    EnablePrimaryRequests(0x10u, 0u);
    WriteU32(0x12c1ul, 94499ul);
    DispatchReadAcquisitionRequestFlags();
    assert(find_count == 0u && resume_count == 0u);

    ResetFixture();
    EnablePrimaryRequests(0x10u, 0u);
    WriteU32(0x12c1ul, 94500ul);
    DispatchReadAcquisitionRequestFlags();
    assert(ReadU32(0x0e3bul) == 90000ul);
    assert(find_count == 1u && find_argument == 90000l);
    assert(memory[0x0fbbul] == 7u && memory[0x0e42ul] == 7u);
    assert(memory[0x0faful] == 1u && memory[0x0e3ful] == 1u);
    assert(resume_count == 1u);
}

static void TestSecondaryFlags(void)
{
    ResetFixture();
    memory[0x11e1ul] = 0x04u;
    memory[0x0faful] = 1u;
    memory[0x0fbaul] = 0x40u;
    memory[0x0fbdul] = 0xa5u;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0fbdul] == 0u && memory[0x0fbaul] == 0x20u);
    assert(write_count == 3u);
    assert(write_addresses[1] == 0x0fbaul && write_values[1] == 0x60u);
    assert(write_addresses[2] == 0x0fbaul && write_values[2] == 0x20u);

    ResetFixture();
    memory[0x11e1ul] = 0x10u;
    memory[0x0faful] = 1u;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0fbdul] == 0u && memory[0x0fbaul] == 0x60u);
    assert(write_count == 3u);
    assert(write_addresses[1] == 0x0fbaul && write_values[1] == 0x20u);
    assert(write_addresses[2] == 0x0fbaul && write_values[2] == 0x60u);

    ResetFixture();
    memory[0x11e1ul] = 0x10u;
    memory[0x0faeul] = 1u;
    memory[0x0fbaul] = 0xffu;
    DispatchReadAcquisitionRequestFlags();
    assert(memory[0x0fbaul] == 0xffu && write_count == 0u);
}

int main(void)
{
    TestAdmissionGates();
    TestBit2Request();
    TestStateRequests();
    TestCursorRequests();
    TestFixedTargetRequest();
    TestSecondaryFlags();
    puts("read-acquisition request-dispatch tests passed");
    return 0;
}
