#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/working_track_raw_read_range.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x300000];
static u16 reset_calls, fill_calls, msf_calls, sequence_calls;
static u16 plan_calls, report_calls, acquire_calls, prefix_calls;
static u8 sector_class, plan_result, acquire_result, prefix_result;
static s32 acquired_count, msf_relative, msf_absolute;

static u16 ReadU16(fw_addr_t a)
{
    return (u16)(((u16)memory_image[a] << 8) | memory_image[a + 1u]);
}
static u32 ReadU32(fw_addr_t a)
{
    return ((u32)memory_image[a] << 24) | ((u32)memory_image[a + 1u] << 16) | ((u32)memory_image[a + 2u] << 8) | memory_image[a + 3u];
}
static void WriteU32(fw_addr_t a, u32 v)
{
    memory_image[a] = (u8)(v >> 24);
    memory_image[a + 1u] = (u8)(v >> 16);
    memory_image[a + 2u] = (u8)(v >> 8);
    memory_image[a + 3u] = (u8)v;
}
static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    reset_calls = fill_calls = msf_calls = sequence_calls = 0u;
    plan_calls = report_calls = acquire_calls = prefix_calls = 0u;
    sector_class = 2u;
    plan_result = acquire_result = prefix_result = 1u;
    acquired_count = msf_relative = msf_absolute = 0l;
    WriteU32(0x156au, 0x200ul);
    WriteU32(0x1562u, 0x126100ul);
    WriteU32(0xdd3du, 100ul);
    WriteU32(0xdd41u, 1000ul);
    WriteU32(0xdd51u, 120ul);
    WriteU32(0xdd57u, 10ul);
}
u8 FirmwareRead8(fw_addr_t a)
{
    assert(a < sizeof(memory_image));
    return memory_image[a];
}
void FirmwareWrite8(fw_addr_t a, u8 v)
{
    assert(a < sizeof(memory_image));
    memory_image[a] = v;
}
void ObserveWorkingTrackRangeReset(void)
{
    ++reset_calls;
}
void ObserveWorkingTrackRangeFill(fw_addr_t a, u32 c, u8 v)
{
    ++fill_calls;
    assert(a == 0ul && c == 0x1b0000ul && v == 0u);
}
u8 ObserveWorkingTrackRangeSectorClass(u8 p, u8 f)
{
    assert(p == 0u && f == 0u);
    return sector_class;
}
void ObserveWorkingTrackRangeSetMsf(s32 r, s32 a)
{
    ++msf_calls;
    msf_relative = r;
    msf_absolute = a;
}
void ObserveWorkingTrackRangeSequenceError(void)
{
    ++sequence_calls;
}
u8 ObserveWorkingTrackRangePreparePlan(u8 a)
{
    ++plan_calls;
    assert(a == 1u);
    return plan_result;
}
void ObserveWorkingTrackRangeReportFailure(void)
{
    ++report_calls;
}
u8 ObserveWorkingTrackRangeAcquire(s32 c)
{
    ++acquire_calls;
    acquired_count = c;
    return acquire_result;
}
u8 ObserveWorkingTrackRangePrefix(u8 o, u8 w)
{
    ++prefix_calls;
    assert(o == 0u && w == 1u);
    return prefix_result;
}

static void AssertInitialization(void)
{
    assert(reset_calls == 1u && fill_calls == 1u);
    assert(ReadU16(0x129au) == 0x1ffu && ReadU16(0x129cu) == 0x1fau);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x126au) == 0x126100ul);
    assert(memory_image[0xc2au] == 6u && memory_image[0x1281u] == 2u && memory_image[0x142bu] == sector_class);
}

static void TestWriteTypeAndPlanFailures(void)
{
    ResetHarness();
    memory_image[0xc44u] = 1u;
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 0u);
    AssertInitialization();
    assert(sequence_calls == 1u && acquire_calls == 0u);

    ResetHarness();
    WriteU32(0xdd51u, 80ul);
    plan_result = 0u;
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 0u);
    assert(plan_calls == 1u && report_calls == 1u && acquire_calls == 0u);
}

static void TestFixedPacketRoutes(void)
{
    ResetHarness();
    memory_image[0xc46u] = 2u;
    WriteU32(0xdd51u, 111ul);
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 1u);
    assert(acquired_count == 16l && ReadU32(0x8000u) == 130ul);
    assert(msf_calls == 2u && msf_relative == 11l && msf_absolute == 111l);
    assert(ReadU32(0x1488u) == 110ul && prefix_calls == 1u);

    ResetHarness();
    memory_image[0xc46u] = 2u;
    WriteU32(0xdd51u, 111ul);
    WriteU32(0xdd41u, 129ul);
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 0u);
    assert(report_calls == 1u && acquire_calls == 0u);
}

static void TestOrdinaryRoutes(void)
{
    ResetHarness();
    WriteU32(0x1484u, 0x80000000ul);
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 1u);
    assert(acquired_count == 280l && ReadU32(0x8000u) == 123ul);

    ResetHarness();
    memory_image[0xdd34u] = 5u;
    WriteU32(0xdd51u, 120ul);
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 1u);
    assert(acquired_count == 878l);

    ResetHarness();
    WriteU32(0x1484u, 1ul);
    prefix_result = 0u;
    WriteU32(0x1488u, 0xdeadbeeful);
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 0u);
    assert(acquired_count == 10l && ReadU32(0x1488u) == 0xdeadbeeful);

    ResetHarness();
    WriteU32(0x1484u, 1ul);
    acquire_result = 0u;
    assert(PrepareAndAcquireWorkingTrackRawReadRange() == 0u);
    assert(report_calls == 1u && prefix_calls == 0u);
}

int main(void)
{
    TestWriteTypeAndPlanFailures();
    TestFixedPacketRoutes();
    TestOrdinaryRoutes();
    puts("working-track raw-read range tests passed");
    return 0;
}
