#include <assert.h>
#include <string.h>
#include "platform/controller_q_packet_process.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static unsigned advance_count, capture2_count, capture3_count;
static unsigned adr0_count, adr1_count, decode_count;
static unsigned point_record_count, track_record_count;
static unsigned rearm_count, dispatch8_count, dispatch0e_count, ack_count;
static unsigned bcd_count, copy_count;
static u8 advance_result;
static u8 bcd_result;
static fw_addr_t copy_source, copy_destination;
static u16 copy_length;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 AdvanceDecodedFramePositionOutsideLeadInLeadOut(void)
{
    ++advance_count;
    return advance_result;
}

void CaptureControllerQAdr2PacketIfEnabled(void)
{
    ++capture2_count;
}
void CaptureControllerQAdr3PacketIfEnabled(void)
{
    ++capture3_count;
}
void ProcessControllerQAdr0PacketForState2(void)
{
    ++adr0_count;
}
void ProcessControllerQAdr1Packet(void)
{
    ++adr1_count;
}
void DecodeControllerQPacketBcdTuples(void)
{
    ++decode_count;
}
void CopyControllerQAdr5PointB2ThroughB4Record(void)
{
    ++point_record_count;
}
void CopyControllerQAdr5TrackPointRecord(void)
{
    ++track_record_count;
}
void RearmControllerQPacketCaptureWithTimeout(void)
{
    ++rearm_count;
}
void DispatchEightChannelMeasurementCaptureForCurrentQ(void)
{
    ++dispatch8_count;
}
void DispatchControllerCommand0EMeasurementCaptureForCurrentQ(void)
{
    ++dispatch0e_count;
}
void AcknowledgeControllerQPacketCapture(void)
{
    ++ack_count;
}

u8 PackedBcdByteToBinary(u8 value)
{
    (void)value;
    ++bcd_count;
    return bcd_result;
}

void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 length)
{
    unsigned index;

    ++copy_count;
    copy_source = source;
    copy_destination = destination;
    copy_length = length;
    for(index = 0u; index < length; ++index)
        memory[destination + index] = memory[source + index];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    advance_count = capture2_count = capture3_count = 0u;
    adr0_count = adr1_count = decode_count = 0u;
    point_record_count = track_record_count = 0u;
    rearm_count = dispatch8_count = dispatch0e_count = ack_count = 0u;
    bcd_count = copy_count = 0u;
    advance_result = 1u;
    bcd_result = 1u;
    copy_source = copy_destination = 0ul;
    copy_length = 0u;
    memory[0x0011eful] = 2u;
    memory[0x001205ul] = 1u;
    memory[0x00125aul] = 0x80u;
    memory[0x001305ul] = 0xffu;
}

static void SetPacket(u8 adr, u8 point)
{
    memory[0x00124ful] = (u8)(0xa0u | adr);
    memory[0x001250ul] = 0x22u;
    memory[0x001251ul] = point;
    memory[0x001252ul] = 0x11u;
    memory[0x001253ul] = 0x12u;
    memory[0x001254ul] = 0x13u;
    memory[0x001255ul] = 0u;
    memory[0x001256ul] = 0x21u;
    memory[0x001257ul] = 0x22u;
    memory[0x001258ul] = 0x23u;
}

static void TestPreconditionsAndPending(void)
{
    ResetState();
    memory[0x0011eful] = 0u;
    assert(ProcessCapturedControllerQPacket() == 0u);
    assert(memory[0x00f79aul] == 7u);
    assert(memory[0x0011eful] == 4u);
    assert(memory[0x001305ul] == 0x7fu);

    ResetState();
    memory[0x001205ul] = 0x80u;
    assert(ProcessCapturedControllerQPacket() == 0u);
    assert(memory[0x00f79aul] == 6u);

    ResetState();
    memory[0x00125aul] = 0u;
    assert(ProcessCapturedControllerQPacket() == 0xfeu);
    assert(rearm_count == 0u);
}

static void TestCompletedCaptureDispatch(void)
{
    ResetState();
    memory[0x00125aul] = 0xc1u;
    assert(ProcessCapturedControllerQPacket() == 0xfeu);
    assert(memory[0x00125aul] == 0x81u);
    assert(advance_count == 1u);
    assert(dispatch8_count == 1u && dispatch0e_count == 1u);
    assert(ack_count == 1u);

    ResetState();
    memory[0x00125aul] = 0xc1u;
    memory[0x001304ul] = 2u;
    assert(ProcessCapturedControllerQPacket() == 0xfeu);
    assert(advance_count == 0u && ack_count == 1u);
}

static void TestOrdinaryAdrDispatch(void)
{
    ResetState();
    SetPacket(1u, 1u);
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(adr1_count == 1u);
    assert(memory[0x00125aul] == 0x81u);

    ResetState();
    SetPacket(2u, 1u);
    memory[0x00125aul] = 0x81u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(capture2_count == 1u && advance_count == 1u);

    ResetState();
    SetPacket(3u, 1u);
    memory[0x00125aul] = 0x81u;
    memory[0x000ff9ul] = 4u;
    assert(ProcessCapturedControllerQPacket() == 0xfeu);
    assert(capture3_count == 1u && advance_count == 1u);
    assert(rearm_count == 1u);

    ResetState();
    SetPacket(0u, 1u);
    memory[0x001008ul] = 2u;
    memory[0x001132ul] = 0xffu;
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(adr0_count == 1u && memory[0x001132ul] == 0xf7u);

    ResetState();
    SetPacket(0u, 1u);
    memory[0x00125aul] = 0x81u;
    memory[0x000ff9ul] = 4u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(advance_count == 1u && rearm_count == 0u);

    ResetState();
    SetPacket(7u, 1u);
    memory[0x00125aul] = 0x81u;
    advance_result = 0u;
    assert(ProcessCapturedControllerQPacket() == 0xfeu);
    assert(rearm_count == 1u);
}

static void AssertCopiedRecord(u8 point, u8 kinds, fw_addr_t destination, unsigned expected_decode)
{
    assert(memory[0x001259ul] == kinds);
    assert(decode_count == expected_decode);
    assert(memory[0x001247ul] == point);
    assert(memory[0x001245ul] == 0xa5u);
    assert(memory[0x001246ul] == 0x22u);
    assert(copy_count == 1u);
    assert(copy_source == 0x001245ul);
    assert(copy_destination == destination && copy_length == 10u);
}

static void TestOrdinaryAdr5FixedRecords(void)
{
    ResetState();
    SetPacket(5u, 0xb0u);
    memory[0x001252ul] = memory[0x001253ul] = memory[0x001254ul] = 0xffu;
    memory[0x00125aul] = 0x81u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    AssertCopiedRecord(0xb0u, 0x10u, 0x00f528ul, 1u);
    assert(memory[0x00f52bul] == 0xffu);
    assert(memory[0x00f52cul] == 0xffu);
    assert(memory[0x00f52dul] == 0xffu);

    ResetState();
    SetPacket(5u, 0xb1u);
    memory[0x00125aul] = 0x81u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    AssertCopiedRecord(0xb1u, 0x20u, 0x00f564ul, 1u);

    ResetState();
    SetPacket(5u, 0xc0u);
    memory[0x00125aul] = 0x81u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    AssertCopiedRecord(0xc0u, 0x80u, 0x00f55aul, 1u);
    assert(memory[0x001248ul] == 0x11u);
    assert(memory[0x001249ul] == 0x12u);
    assert(memory[0x00124aul] == 0x13u);

    ResetState();
    SetPacket(5u, 0xc1u);
    memory[0x00125aul] = 0x81u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    AssertCopiedRecord(0xc1u, 0x40u, 0x00f56eul, 0u);
    assert(memory[0x001248ul] == 0x11u);
    assert(memory[0x00124cul] == 0x21u);
    assert(memory[0x00124dul] == 0x22u);
    assert(memory[0x00124eul] == 0x23u);
}

static void TestOrdinaryAdr5IndexedRecords(void)
{
    ResetState();
    SetPacket(5u, 0xb3u);
    memory[0x00125aul] = 0x81u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(point_record_count == 1u && track_record_count == 0u);
    assert(decode_count == 1u && memory[0x001259ul] == 0x20u);

    ResetState();
    SetPacket(5u, 0x20u);
    memory[0x00125aul] = 0x81u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(point_record_count == 0u && track_record_count == 1u);
    assert(decode_count == 1u && memory[0x001259ul] == 0x20u);
}

static void TestAlternateAdr1And2(void)
{
    ResetState();
    memory[0x001304ul] = 2u;
    SetPacket(1u, 0x12u);
    memory[0x001252ul] = 0xffu;
    bcd_result = 12u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(bcd_count == 1u && decode_count == 1u);
    assert(memory[0x001247ul] == 12u);
    assert(memory[0x00125bul] == 1u);
    assert(memory[0x001248ul] == 0xffu);
    assert(memory[0x001249ul] == 0x12u);

    ResetState();
    memory[0x001304ul] = 2u;
    SetPacket(2u, 0x12u);
    bcd_result = 12u;
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(memory[0x00125bul] == 2u);
    assert(memory[0x00124dul] == 0x22u);
}

static void TestAlternateBoundedAdrPackets(void)
{
    static const u8 adrs[4] = { 3u, 4u, 5u, 6u };
    static const u8 maxima[4] = { 0x15u, 0x15u, 0x28u, 0x28u };
    static const u8 masks[4] = { 4u, 8u, 0x10u, 0x20u };
    unsigned index;

    for(index = 0u; index < 4u; ++index)
    {
        ResetState();
        memory[0x001304ul] = 2u;
        SetPacket(adrs[index], 0x12u);
        bcd_result = maxima[index];
        assert(ProcessCapturedControllerQPacket() == 1u);
        assert(bcd_count == 2u && decode_count == 1u);
        assert(memory[0x001247ul] == maxima[index]);
        assert(memory[0x00125bul] == masks[index]);

        ResetState();
        memory[0x001304ul] = 2u;
        SetPacket(adrs[index], 0x12u);
        bcd_result = (u8)(maxima[index] + 1u);
        assert(ProcessCapturedControllerQPacket() == 0u);
        assert(bcd_count == 1u && decode_count == 0u);
        assert(rearm_count == 1u && memory[0x001246ul] == 0x22u);
    }
}

static void TestAlternateAdr0AndUnsupported(void)
{
    ResetState();
    memory[0x001304ul] = 2u;
    memory[0x001008ul] = 2u;
    SetPacket(0u, 0u);
    assert(ProcessCapturedControllerQPacket() == 1u);
    assert(adr0_count == 1u && rearm_count == 0u);

    ResetState();
    memory[0x001304ul] = 2u;
    SetPacket(7u, 0u);
    assert(ProcessCapturedControllerQPacket() == 0xfeu);
    assert(rearm_count == 1u && memory[0x00125bul] == 0u);
}

int main(void)
{
    TestPreconditionsAndPending();
    TestCompletedCaptureDispatch();
    TestOrdinaryAdrDispatch();
    TestOrdinaryAdr5FixedRecords();
    TestOrdinaryAdr5IndexedRecords();
    TestAlternateAdr1And2();
    TestAlternateBoundedAdrPackets();
    TestAlternateAdr0AndUnsupported();
    return 0;
}
