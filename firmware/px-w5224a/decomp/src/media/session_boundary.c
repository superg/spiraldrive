#include "session_boundary.h"
#include "../platform/controller_q_packet_process.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/session_boundary_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/session_boundary_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f89194 */
void PLEXTOR_NEAR_FUNCTION ServiceMediaQPacketCaptureWithOperatingCodeBudget(void)
{
    u8 capture_flags;

    capture_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9);
    if(((capture_flags >> 2) & 1u) == 1u)
    {
        u8 pulse_result = Q_BUDGET_PULSE(10u);

        capture_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)((capture_flags & 0xfeu) | (pulse_result & 1u)));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
        return;

    capture_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9);
    if((capture_flags & 1u) == 1u || ((capture_flags >> 2) & 1u) != 1u)
    {
        u8 consecutive_failures = 0u;

        Q_BUDGET_REARM();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, (u8)Q_BUDGET_DIVIDE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43)));
        do
        {
            u8 packet_result = Q_BUDGET_PROCESS();

            if(packet_result != 0xfeu || (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) < 1)
                break;
            if(Q_BUDGET_PULSE(10u) != 0u)
            {
                consecutive_failures = 0u;
            }
            else
            {
                consecutive_failures = (u8)(consecutive_failures + 1u);
                if(consecutive_failures >= 3u)
                {
                    capture_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9);
                    if(((capture_flags >> 2) & 1u) == 1u)
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfeu));
                    break;
                }
            }
        } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
            return;
        if(((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) < 0 || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) < 0))
            && (((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) >> 2) & 1u) != 0u))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfeu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    }
    Q_BUDGET_YIELD();
}

/* Reconstruction helpers; no direct Ghidra function addresses. */
static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fba73c */
u8 PLEXTOR_NEAR_FUNCTION AcquireNextSessionBoundaryPosition(void)
{
    u8 consecutive_pulse_failures = 0u;
    u32 target;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    target = ReadBigEndianU32(FW_NEAR_UNKNOWN_12CC) + 150ul;
    WriteBigEndianU32(FW_NEAR_UNKNOWN_12D0, target);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0E3B, target);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED8, 1u);
    if(ACQUIRE_DISC_POSITION() != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED8, 0u);
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED8, 0u);
    APPLY_OPERATING_CHANNELS();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 4u) == 0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 2u);
    for(;;)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
            return 0u;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        {
            if(TEST_PULSE_THRESHOLD(10u) == 1u)
                return 1u;
            continue;
        }

        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
        {
            if(TEST_PULSE_THRESHOLD(10u) == 1u)
                return 1u;
            continue;
        }

        REARM_Q_CAPTURE();
        for(;;)
        {
            u8 packet_result = PROCESS_Q_PACKET();

            if(packet_result != 0xfeu)
            {
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x80u) != 0u)
                    return 0u;
                return 1u;
            }

            if(TEST_PULSE_THRESHOLD(10u) != 0u)
                consecutive_pulse_failures = 0u;
            else
                ++consecutive_pulse_failures;
            if(consecutive_pulse_failures > 20u)
                return 0u;
            YIELD_BOUNDARY_WAIT();
        }
    }
}
