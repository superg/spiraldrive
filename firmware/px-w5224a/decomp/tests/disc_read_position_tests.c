#include <assert.h>
#include <string.h>
#include "media/disc_read_position.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 packet_results[8];
static u8 reduce_results[8];
static unsigned packet_result_index;
static unsigned reduce_result_index;
static u8 pulse_result;
static u8 position_result;
static u8 nudge_result;
static u8 calibration_result;
static unsigned disabled_packet_calls;
static unsigned enabled_packet_calls;
static unsigned apply_d1_calls;
static unsigned radial_sample_calls;
static unsigned recovery_reset_calls;
static unsigned drive_reset_calls;
static unsigned finalizer_calls;
static unsigned move_calls;
static unsigned strobe_calls;
static unsigned yield_calls;
static unsigned p1_helper_calls;
static unsigned p1_direct_calls;
static fw_addr_t last_strobe_port;
static u8 last_strobe_value;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

void SetP1Bits0And1(void)
{
    ++p1_helper_calls;
}
void ObserveDiscReadPositionSetP1TimerPins(void)
{
    ++p1_direct_calls;
}
void ResetControllerPorts8087Through808B(void) {}
void SelectAndApplyOperatingCodeForMediaState(u8 wait)
{
    (void)wait;
}
void SelectAndConfigureOperatingCodeForThreshold(u32 threshold)
{
    (void)threshold;
}
void InitializeReadAcquisitionRadialState(void) {}
void WaitControllerPortReadyAndWriteByte(fw_addr_t port, u8 value)
{
    (void)port;
    (void)value;
}
u8 IsRadialPositionEstimateAtMost3542(void)
{
    return 1u;
}
u8 AcquireAndValidateReadPositionPacket(void)
{
    ++disabled_packet_calls;
    return packet_results[packet_result_index++];
}
u8 AcquireReadPositionPacketWithChannelSetup(void)
{
    ++enabled_packet_calls;
    return packet_results[packet_result_index++];
}
void ApplyControllerD1ModeBitsAfterPulseTest(void)
{
    ++apply_d1_calls;
}
void AccumulateRadialStepScaleCalibrationSample(void)
{
    ++radial_sample_calls;
}
u8 ReduceReadSpeedForAcquisitionRecovery(u8 force)
{
    (void)force;
    return reduce_results[reduce_result_index++];
}
void SetControllerChannel0ValueForOperatingCode(u8 code)
{
    (void)code;
}
void ResetReadAcquisitionRecoveryState(void)
{
    ++recovery_reset_calls;
}
void ApplyControllerParameterD1ModeBits(void)
{
    ++apply_d1_calls;
}
u8 PositionPickupForAcquisition(void)
{
    return position_result;
}
u8 ApplyPendingReadAcquisitionTargetNudge(void)
{
    return nudge_result;
}
void MovePickupByByteDistance(u8 distance, u8 direction)
{
    assert(distance == 4u && direction == 1u);
    ++move_calls;
}
void StrobeControllerPortWriteByteFromC(fw_addr_t port, u8 value)
{
    ++strobe_calls;
    last_strobe_port = port;
    last_strobe_value = value;
}
void YieldCoroutine1(void)
{
    ++yield_calls;
}
u8 TestP1Cex0PulseSuccessThreshold(u8 count)
{
    assert(count == 20u);
    return pulse_result;
}
void ResetDriveControlStateToEight(void)
{
    ++drive_reset_calls;
}
u8 InitializeControllerOperatingStateAndRunCalibration(void)
{
    return calibration_result;
}
void ConfigureControllerPorts80D0D1D3ForCurrentState(void)
{
    ++finalizer_calls;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(packet_results, 0, sizeof(packet_results));
    memset(reduce_results, 0, sizeof(reduce_results));
    packet_result_index = reduce_result_index = 0u;
    pulse_result = position_result = nudge_result = 0u;
    calibration_result = 1u;
    disabled_packet_calls = enabled_packet_calls = 0u;
    apply_d1_calls = radial_sample_calls = recovery_reset_calls = 0u;
    drive_reset_calls = finalizer_calls = move_calls = strobe_calls = 0u;
    yield_calls = p1_helper_calls = p1_direct_calls = 0u;
    last_strobe_port = 0ul;
    last_strobe_value = 0u;
    WriteU32(0x000e3bul, 100ul);
    WriteU32(0x001235ul, 200ul);
    memory[0x001207ul] = 0u;
    memory[0x00fc3dul] = 3u;
}

static void TestNearTargetSuccess(void)
{
    Reset();
    packet_results[0] = 1u;
    memory[0x000ee2ul] = 1u;
    assert(AcquireDiscReadPosition() == 1u);
    assert(disabled_packet_calls == 1u && enabled_packet_calls == 0u);
    assert(radial_sample_calls == 1u && finalizer_calls == 1u);
    assert(memory[0x000e41ul] == 0u);
    assert(memory[0x001030ul] == 1u);
    assert(p1_helper_calls == 1u && p1_direct_calls == 1u);
}

static void TestControllerAbort(void)
{
    Reset();
    memory[0x001006ul] = 1u;
    memory[0x000e3ful] = 1u;
    memory[0x0011eful] = 0xffu;
    packet_results[0] = 1u;
    assert(AcquireDiscReadPosition() == 0xffu);
    assert(enabled_packet_calls == 1u && apply_d1_calls == 1u);
    assert(memory[0x0011eful] == 0x7fu);
    assert(memory[0x001300ul] == 1u);
    assert(memory[0x01802bul] == 0x80u);
}

static void TestLowTargetPositioningNudge(void)
{
    Reset();
    memory[0x001006ul] = 1u;
    memory[0x001207ul] = 1u;
    memory[0x000ee4ul] = 2u;
    WriteU32(0x000e3bul, 10ul);
    packet_results[0] = 1u;
    position_result = 1u;
    assert(AcquireDiscReadPosition() == 1u);
    assert(ReadU32(0x000e3bul) == 16ul);
    assert(move_calls == 1u && strobe_calls == 1u);
    assert(last_strobe_port == 0x018007ul && last_strobe_value == 8u);
    assert(apply_d1_calls == 1u);
}

static void TestPacketFailureCanRetryAtReducedSpeed(void)
{
    Reset();
    packet_results[0] = 0u;
    packet_results[1] = 1u;
    reduce_results[0] = 1u;
    pulse_result = 1u;
    memory[0x000e41ul] = 2u;
    memory[0x000ee2ul] = 1u;
    assert(AcquireDiscReadPosition() == 1u);
    assert(disabled_packet_calls == 2u);
    assert(recovery_reset_calls == 1u && drive_reset_calls == 0u);
    assert(memory[0x00fc3bul] == 0u);
}

static void TestCalibrationFailurePublishesStatusSeven(void)
{
    Reset();
    packet_results[0] = 0u;
    pulse_result = 0u;
    calibration_result = 0u;
    memory[0x000e41ul] = 2u;
    memory[0x00fc3dul] = 2u;
    assert(AcquireDiscReadPosition() == 0u);
    assert(drive_reset_calls == 2u);
    assert(memory[0x000e41ul] == 7u);
    assert(memory[0x00fc3dul] == 1u);
    assert(reduce_result_index == 0u);
}

static void TestLeadInPacketIsRejected(void)
{
    Reset();
    memory[0x000ed8ul] = 1u;
    memory[0x0012c0ul] = 2u;
    memory[0x001233ul] = 3u;
    WriteU32(0x001235ul, 99ul);
    WriteU32(0x0012d8ul, 100ul);
    packet_results[0] = 1u;
    assert(AcquireDiscReadPosition() == 0u);
    assert(memory[0x000e41ul] == 5u);
    assert(radial_sample_calls == 0u);
}

int main(void)
{
    TestNearTargetSuccess();
    TestControllerAbort();
    TestLowTargetPositioningNudge();
    TestPacketFailureCanRetryAtReducedSpeed();
    TestCalibrationFailurePublishesStatusSeven();
    TestLeadInPacketIsRejected();
    return 0;
}
