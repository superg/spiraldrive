#include "disc_recognition.h"
#include "../platform/controller_q_packet_process.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"

extern void PLEXTOR_NEAR_FUNCTION ReturnFromFullTocAppendNoOp(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/disc_recognition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/disc_recognition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static s32 ReadInitialTocFramePosition(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static void WriteInitialTocFramePosition(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u16 ReadDiscRecognitionWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteDiscRecognitionWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fba102 */
u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoInitialToc(void)
{
    u8 point = FirmwareReadNear8(FW_NEAR_UNKNOWN_1247);
    u8 presence_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18);

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) >= 0)
    {
        if(point == 0xa1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
            CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F532, 10u);
            presence_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 2u);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_F53A) == 0x10u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F53C) & 0x40u) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, 1u);
        }
        else if(point == 0xa2u)
        {
            s32 lead_out_position;

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_124C) > 0x5eu)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_102E, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
            }
            lead_out_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_124C);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C1, (u8)((u32)lead_out_position >> 24));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C2, (u8)((u32)lead_out_position >> 16));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C3, (u8)((u32)lead_out_position >> 8));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C4, (u8)lead_out_position);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
            CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F53C, 10u);
            presence_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 4u);
        }
        else if(point == 0xa0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BF, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
            CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F546, 10u);
            presence_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0xf7u));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_124D) == 0x10u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F2BC, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F2BD, 0x41u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F2BF, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F2C0, 2u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F2C1, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BF, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | 0x0au));
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 2u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F53C) & 0x40u) != 0u)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, 1u);
            }
        }
        else if(point != 0u)
        {
            fw_addr_t record = FW_NEAR_UNKNOWN_F2B6 + (fw_addr_t)((u16)point * 6u);

            FirmwareWrite8(record, point);
            FirmwareWrite8(record + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1245));
            FirmwareWrite8(record + 3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
            FirmwareWrite8(record + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_124D));
            FirmwareWrite8(record + 5u, FirmwareReadNear8(FW_NEAR_UNKNOWN_124E));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1245) & 0x40u) == 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | 1u));
            else
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | 2u));
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, presence_flags);
    return 1u;
}

/* RAM:fb9e9d */
u8 PLEXTOR_NEAR_FUNCTION AcquireInitialDiscTocState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 4u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    RESET_ALL_INITIAL_TOC_STATE();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u
        && (FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 13u || INITIAL_TOC_OPERATION_CODE_IS_ELIGIBLE() != 0u))
    {
        MARK_INITIAL_TOC_UNAVAILABLE();
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 15u);
    WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_0E3B, 0x10000000ul);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_0E3B, (u32)ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_114F) + 0x1194ul);

    for(;;)
    {
        u8 negative_packet_count;
        u8 pulse_failure_count;

        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) != 0u)
            if(ACQUIRE_INITIAL_TOC_POSITION() == 1u)
                break;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            {
                RESET_ALL_INITIAL_TOC_STATE();
                MARK_INITIAL_TOC_UNAVAILABLE();
                return 0u;
            }
            RESET_INITIAL_TOC_DRIVE_CONTROL();
            return 4u;
        }

        APPLY_INITIAL_TOC_OPERATING_CODE();
        REARM_INITIAL_TOC_Q_CAPTURE();
        negative_packet_count = 0u;
        pulse_failure_count = 0u;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        {
            while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) != 0u)
            {
                if(PROCESS_INITIAL_TOC_Q_PACKET() != 0xfeu)
                {
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) != 0u)
                    {
                        if(RESET_INITIAL_TOC_ACQUISITION(1u) == 0u)
                            return 4u;
                        break;
                    }
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 8u) == 0u)
                    {
                        (void)RESET_INITIAL_TOC_ACQUISITION(0u);
                        break;
                    }
                    REARM_INITIAL_TOC_Q_CAPTURE();
                    if(ACCUMULATE_INITIAL_TOC_Q_PACKET() == 0u)
                    {
                        if(RESET_INITIAL_TOC_ACQUISITION(1u) == 0u)
                            return 4u;
                        break;
                    }
                    if(INITIAL_TOC_IS_COMPLETE() != 0u)
                        return 0u;
                }
                YIELD_INITIAL_TOC();
            }
            continue;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
        ARM_INITIAL_TOC_PACKET_CAPTURE();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) != 0u)
        {
            if(PARSE_INITIAL_TOC_PACKET() != 0xfeu)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
                {
                    if(RESET_INITIAL_TOC_ACQUISITION(1u) == 0u)
                        return 4u;
                    break;
                }

                ARM_INITIAL_TOC_PACKET_CAPTURE();
                if(PROCESS_INITIAL_TOC_Q_PACKET() != 0xfeu)
                {
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
                    {
                        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x80u) != 0u)
                            ++negative_packet_count;
                        else
                            negative_packet_count = 0u;
                        if(negative_packet_count > 5u)
                        {
                            MARK_INITIAL_TOC_UNAVAILABLE();
                            return 0u;
                        }
                    }
                    REARM_INITIAL_TOC_Q_CAPTURE();
                    if(ACCUMULATE_INITIAL_TOC_Q_PACKET() == 0u)
                    {
                        if(RESET_INITIAL_TOC_ACQUISITION(1u) == 0u)
                            return 4u;
                        break;
                    }
                    if(INITIAL_TOC_IS_COMPLETE() != 0u)
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xbfu));
                        return 0u;
                    }
                }

                if(TEST_INITIAL_TOC_PULSE_THRESHOLD(10u) != 0u)
                {
                    pulse_failure_count = 0u;
                }
                else
                {
                    ++pulse_failure_count;
                    if(pulse_failure_count > 20u)
                    {
                        MARK_INITIAL_TOC_UNAVAILABLE();
                        return 0u;
                    }
                }
            }

            if(ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_1235) <= ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_114F))
            {
                (void)RESET_INITIAL_TOC_ACQUISITION(0u);
                break;
            }
            if(ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_1235) >= 0)
            {
                if(INITIAL_TOC_IS_COMPLETE() != 0u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xbfu));
                    return 0u;
                }
                (void)RESET_INITIAL_TOC_ACQUISITION(0u);
                break;
            }
            YIELD_INITIAL_TOC();
        }
    }
}

/* RAM:fba5c8 */
u8 PLEXTOR_NEAR_FUNCTION BuildFullTocForAvailableSessions(void)
{
    s32 next_lead_in;
    u8 retry_count;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC19, FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1A, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
    WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4, (u32)ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12C1));
    WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_12D8, (u32)ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12C1));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1A, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
    APPEND_FULL_TOC_SESSION();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 1u) != 0u)
        goto finalize;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 8u) == 0u)
        goto finalize;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == 0xffu)
        goto finalize;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 2u) != 0u)
        WriteInitialTocFramePosition(FW_NEAR_CURRENT_FRAME_POSITION, (u32)ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_FB8C) + (u32)ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_FB90) - 0x8caul);
    next_lead_in = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_F553);
    WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_12CC, (u32)next_lead_in);
    if(next_lead_in == 0)
        goto finalize;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && next_lead_in >= ReadInitialTocFramePosition(FW_NEAR_CURRENT_FRAME_POSITION))
        goto finalize;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | 4u));
    FirmwareWrite8(FW_HUGE_UNKNOWN_00F2BC + (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) * 6u), 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1B, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, 2u);
    retry_count = 3u;

    for(;;)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
        if(ACQUIRE_NEXT_FULL_TOC_BOUNDARY() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) - 1u));
            break;
        }
        if(ACQUIRE_NEXT_FULL_TOC_LEAD_IN() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
            --retry_count;
            if(retry_count != 0u)
                continue;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) - 1u));
            CLEAR_FAILED_FULL_TOC_SESSION_POINTS();
            break;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        retry_count = 3u;
        APPEND_FULL_TOC_SESSION();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 8u) == 0u)
            break;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == 0xffu)
            break;
        next_lead_in = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_F553);
        WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_12CC, (u32)next_lead_in);
        if(next_lead_in == 0)
            break;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && next_lead_in >= ReadInitialTocFramePosition(FW_NEAR_CURRENT_FRAME_POSITION))
            break;
        if(REACQUIRE_NEXT_FULL_TOC_BOUNDARY() == 0u)
            break;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) + 1u));
    }

finalize:
    FINALIZE_FULL_TOC_LEAD_OUT();
    return 0u;
}

/* RAM:fba802 */
u8 PLEXTOR_NEAR_FUNCTION AcquireNextSessionLeadInToc(void)
{
    u8 negative_packet_count = 0u;
    u8 pulse_failure_count = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 0xfeu));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) | 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) | 1u));
    }
    WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_0E3B, 0x10000000ul);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_0E3B, (u32)ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12CC) - 0x465ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 15u);

    for(;;)
    {
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) != 0u)
            if(ACQUIRE_INITIAL_TOC_POSITION() == 1u)
                break;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) == 0u)
            return 0u;

        APPLY_INITIAL_TOC_OPERATING_CODE();
        REARM_INITIAL_TOC_Q_CAPTURE();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        {
            while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) != 0u)
            {
                if(PROCESS_INITIAL_TOC_Q_PACKET() != 0xfeu)
                {
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 8u) == 0u)
                    {
                        WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_0E3B, 0x10000000ul);
                        break;
                    }
                    REARM_INITIAL_TOC_Q_CAPTURE();
                    if(ACCUMULATE_CURRENT_SESSION_TOC_Q_PACKET() == 0u)
                        return 0u;
                    if(CURRENT_SESSION_TOC_IS_COMPLETE() == 1u)
                        return 1u;
                }
                YIELD_INITIAL_TOC();
            }
            continue;
        }

        ARM_INITIAL_TOC_PACKET_CAPTURE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) != 0u)
        {
            if(PARSE_INITIAL_TOC_PACKET() != 0xfeu)
            {
                ARM_INITIAL_TOC_PACKET_CAPTURE();
                if(TEST_INITIAL_TOC_PULSE_THRESHOLD(10u) != 0u)
                {
                    pulse_failure_count = 0u;
                }
                else
                {
                    ++pulse_failure_count;
                    if(pulse_failure_count > 10u)
                        return 0u;
                }
                if(ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_1235) >= (s32)((u32)ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12CC) + 150ul))
                    return 0u;
            }

            {
                u8 process_result = PROCESS_INITIAL_TOC_Q_PACKET();

                if(process_result != 0xfeu)
                {
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x80u) != 0u)
                    {
                        ++negative_packet_count;
                    }
                    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 8u) != 0u)
                    {
                        negative_packet_count = 0u;
                    }
                    else
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 0x80u));
                        ++negative_packet_count;
                    }
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && negative_packet_count > 5u) || process_result == 0u)
                        return 0u;
                    REARM_INITIAL_TOC_Q_CAPTURE();
                    if(ACCUMULATE_CURRENT_SESSION_TOC_Q_PACKET() == 0u)
                        return 0u;
                    if(CURRENT_SESSION_TOC_IS_COMPLETE() == 1u)
                        return 1u;
                }
            }
            YIELD_INITIAL_TOC();
        }
    }
}

/* RAM:fba9e0 */
u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoSessionToc(void)
{
    u8 point;
    s32 frame_position;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x80u) != 0u)
        return 1u;

    point = FirmwareReadNear8(FW_NEAR_UNKNOWN_1247);
    if(point == 0xa0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 1u) == 0u)
        {
            if((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_124C) != (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1B) + 1u)
                return 0u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC19, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
            CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F532, 10u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 1u));
        }
    }
    else if(point == 0xa1u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 2u) == 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 2u) != 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_124C) != FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A))
                    return 0u;
            }
            else
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_124C) < FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A))
                    return 0u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1A, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
                CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F53C, 10u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 2u));
            }
        }
    }
    else if(point == 0xa2u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 1u) == 0u)
        {
            frame_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_124C);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 4u) != 0u)
            {
                if(frame_position != ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4))
                    return 0u;
            }
            else
            {
                if(frame_position < ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4))
                    return 0u;
                frame_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_124C);
                WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4, (u32)frame_position);
                CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F546, 10u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 4u));
            }
        }
    }
    else
    {
        fw_addr_t record;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 1u) != 0u && (u16)point == (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1B) + 1u
            && ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12CC) != (s32)((u32)ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_124C) - 150ul))
        {
            return 0u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC24) & 0xfdu));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 1u) != 0u && point < FirmwareReadNear8(FW_NEAR_UNKNOWN_FC19))
            return 0u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 2u) != 0u && point > FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A))
            return 0u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 1u) != 0u)
        {
            if(point < FirmwareReadNear8(FW_NEAR_UNKNOWN_FC19))
                goto append_retained_payload;
        }
        else if((u16)point < (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1B) + 1u)
        {
            goto append_retained_payload;
        }

        record = FW_NEAR_UNKNOWN_F2B6 + (fw_addr_t)((u16)point * 6u);
        FirmwareWrite8(record, point);
        FirmwareWrite8(record + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1245));
        FirmwareWrite8(record + 3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
        FirmwareWrite8(record + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_124D));
        FirmwareWrite8(record + 5u, FirmwareReadNear8(FW_NEAR_UNKNOWN_124E));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1245) & 0x40u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | 2u));
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | 1u));
    }

append_retained_payload:
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 8u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x10u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC18, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) | 8u));
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F528, FW_NEAR_UNKNOWN_F550, 10u);
    }
    return 1u;
}

static fw_addr_t FullTocDescriptorAddress(u16 cursor_index)
{
    u16 offset = (u16)(0x8000u + (u16)(cursor_index * 11u));

    return FW_HUGE_UNKNOWN_2B0000 | (fw_addr_t)offset;
}

static void AppendFullTocPayloadDescriptor(fw_addr_t source, u8 clear_position_fields)
{
    u16 cursor = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_12DC);
    fw_addr_t destination = FullTocDescriptorAddress(cursor);

    FirmwareWrite8(destination, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
    CopyBytesWithinFarBanks(source, destination + 1u, 10u);
    if(clear_position_fields != 0u)
    {
        FirmwareWrite8(destination + 6u, 0u);
        FirmwareWrite8(destination + 5u, 0u);
        FirmwareWrite8(destination + 4u, 0u);
    }
    cursor = (u16)(cursor + 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12DC, (u8)(cursor >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12DD, (u8)cursor);
}

static void AddFullTocDescriptorBytes(u16 byte_count)
{
    u16 bytes_used = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_12CA);

    bytes_used = (u16)(bytes_used + byte_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12CA, (u8)(bytes_used >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12CB, (u8)bytes_used);
}

/* RAM:fbad8a */
void PLEXTOR_NEAR_FUNCTION AppendFullTocSessionFromDecodedTocState(void)
{
    u8 first_track;
    u8 last_track;
    u8 track;
    u8 payload_index;
    u8 payload_count;

    AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F532, 1u);
    AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F53C, 1u);
    AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F546, 1u);
    AddFullTocDescriptorBytes(33u);

    first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC19);
    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A);
    if(first_track <= last_track)
    {
        track = first_track;
        do
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 8u) == 0u || last_track < 2u || track != 1u)
            {
                u16 cursor = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_12DC);
                fw_addr_t destination = FullTocDescriptorAddress(cursor);
                fw_addr_t record = (u16)(0xf2b6u + (u16)((u16)track * 6u));

                FirmwareWrite8(destination, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
                FirmwareWrite8(destination + 1u, FirmwareRead8(record + 1u));
                FirmwareWrite8(destination + 2u, FirmwareRead8(record + 2u));
                FirmwareWrite8(destination + 3u, FirmwareRead8(record));
                FirmwareWrite8(destination + 8u, FirmwareRead8(record + 3u));
                FirmwareWrite8(destination + 9u, FirmwareRead8(record + 4u));
                FirmwareWrite8(destination + 10u, FirmwareRead8(record + 5u));
                FirmwareWrite8(destination + 6u, 0u);
                FirmwareWrite8(destination + 5u, 0u);
                FirmwareWrite8(destination + 4u, 0u);
                cursor = (u16)(cursor + 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_12DC, (u8)(cursor >> 8));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_12DD, (u8)cursor);
                AddFullTocDescriptorBytes(11u);
            }
            if(track == last_track)
                break;
            track = (u8)(track + 1u);
        } while(1);
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 8u) != 0u)
    {
        AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F550, 0u);
        AddFullTocDescriptorBytes(11u);
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x10u) != 0u)
    {
        AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F564, 0u);
        AddFullTocDescriptorBytes(11u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F566, 0u);

        payload_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_F56C);
        if(payload_count != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1D) != payload_count)
        {
            payload_count = (u8)(((u16)payload_count + 6u) / 7u);
            payload_index = 1u;
            do
            {
                AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F79C + (fw_addr_t)((u16)payload_index * 10u), 0u);
                AddFullTocDescriptorBytes(11u);
                if(payload_index == payload_count)
                    break;
                payload_index = (u8)(payload_index + 1u);
            } while(1);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1D, FirmwareReadNear8(FW_NEAR_UNKNOWN_F56C));
        }

        payload_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_F56B);
        if(payload_count != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1C) != payload_count)
        {
            payload_index = 1u;
            do
            {
                AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F7C4 + (fw_addr_t)((u16)payload_index * 10u), 0u);
                AddFullTocDescriptorBytes(11u);
                if(payload_index == payload_count)
                    break;
                payload_index = (u8)(payload_index + 1u);
            } while(1);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1C, payload_count);
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x80u) != 0u)
    {
        AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F55A, 0u);
        AddFullTocDescriptorBytes(11u);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0x40u) != 0u)
    {
        AppendFullTocPayloadDescriptor(FW_NEAR_UNKNOWN_F56E, 0u);
        AddFullTocDescriptorBytes(11u);
    }

    ReturnFromFullTocAppendNoOp();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1E, 0u);
}

static void ConfigureOuterDiscProbeController(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        OUTER_DISC_CONFIGURE_DISABLED();
    else
        OUTER_DISC_CONFIGURE_ENABLED(0u);
    OUTER_DISC_APPLY_OPERATING_CODE();
}

static void PublishOuterDiscCountdownFailure(void)
{
    WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FAAC) | 0x2000u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 8u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 24u);
        WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7) | 0x0100u));
    }
}

static void PublishOuterDiscAcquisitionFailure(void)
{
    WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FAAC) | 0x2000u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 8u);
    WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7) | 0x0100u));
}

static void CaptureOuterDiscControllerSamples(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);
    OUTER_DISC_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
    OUTER_DISC_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0x14u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 15u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) > 0)
        OUTER_DISC_YIELD();
    OUTER_DISC_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9A9, OUTER_DISC_READ_PORT(FW_CONTROLLER_UNKNOWN_0180C7));

    OUTER_DISC_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
    OUTER_DISC_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0x16u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 15u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) > 0)
        OUTER_DISC_YIELD();
    OUTER_DISC_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9AA, OUTER_DISC_READ_PORT(FW_CONTROLLER_UNKNOWN_0180C7));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));
}

static void FinishOuterDiscProbeFromSamples(u8 ordinary_mode)
{
    u8 first_class = 0u;
    u8 second_class = 0u;
    u8 first_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A9);
    u8 second_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_F9AA);

    if(ordinary_mode != 0u)
    {
        if(first_sample >= 80u || second_sample >= 96u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 4u) != 0u)
            WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7) | 0x0040u));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if(first_sample >= 61u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
            first_class = first_sample >= 179u ? 2u : 1u;
        }
        if(second_sample >= 64u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
            second_class = second_sample >= 112u ? 2u : 1u;
        }
    }
    else
    {
        if(first_sample >= 84u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
            first_class = first_sample >= 201u ? 2u : 1u;
        }
        if(second_sample >= 80u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
            second_class = second_sample >= 132u ? 2u : 1u;
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 4u) == 0u)
        return;
    if(second_class == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 8u);
    else if(first_class == 2u || (u8)(first_class + second_class) > 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 24u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 32u);
}

/* RAM:fbb40f */
void PLEXTOR_NEAR_FUNCTION ProbeOuterDiscAndDetermineReadSpeedLimit(void)
{
    u8 probe_enabled = 0u;
    u8 ordinary_mode = 1u;
    u8 retry_budget;
    u8 restart_tolerance = 0u;
    u8 expected_speed;
    s32 outer_reference;

    if(ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4) >= 270000l
        && (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) == 1u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) >= 7u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F2BD) & 0xbfu) != 0u)))
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
            || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4) <= ReadInitialTocFramePosition(FW_NEAR_CURRENT_FRAME_POSITION) + 6750l))
        {
            probe_enabled = 1u;
        }
        ordinary_mode = 0u;
    }

    if((OUTER_DISC_INDEXED_STATE_ACTIVE(4u) != 0u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 1u && (ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FAAC) & 0x0800u) == 0u)
        || (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x20u) == 0u)
    {
        probe_enabled = 1u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 24u);
    }
    if(probe_enabled == 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x10u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
    WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_0E3B, 150ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 2u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        if(OUTER_DISC_ACQUIRE_POSITION() == 1u)
            break;
        if(OUTER_DISC_RESET_CONTROLLER() == 0u)
            return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 40u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u && ordinary_mode == 0u)
        OUTER_DISC_SET_OPERATING_ENABLE(0u);

    outer_reference = ReadInitialTocFramePosition(FW_NEAR_CURRENT_FRAME_POSITION);
    if(ordinary_mode == 0u)
    {
        outer_reference = ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12D8);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 2u && ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4) - outer_reference >= 16950l)
            outer_reference = ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_12D4);
    }
    WriteInitialTocFramePosition(FW_NEAR_UNKNOWN_0E3B, (u32)(outer_reference - 5600l));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 52u);
    expected_speed = 52u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xfeu));
    WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FC22, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 2u));
    retry_budget = 3u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 14u);

    for(;;)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
        {
            PublishOuterDiscCountdownFailure();
            return;
        }
        if(OUTER_DISC_ACQUIRE_POSITION() != 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xfdu));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == 7u)
            {
                PublishOuterDiscAcquisitionFailure();
                return;
            }
            if(OUTER_DISC_RESET_CONTROLLER() == 0u)
                return;
            continue;
        }

        if(OUTER_DISC_CHECK_TOLERANCE(restart_tolerance == 0u) == 0u && ordinary_mode == 1u)
        {
            restart_tolerance = 1u;
            continue;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xfdu));
        ConfigureOuterDiscProbeController();

        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
        {
            u8 packet_status;

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
            {
                OUTER_DISC_REARM_Q_CAPTURE();
                do
                {
                    packet_status = OUTER_DISC_PROCESS_Q_PACKET();
                    if(packet_status == 0xfeu)
                        OUTER_DISC_YIELD();
                } while(packet_status == 0xfeu);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB94, 0u);
            }
            else
            {
                OUTER_DISC_ARM_PACKET_CAPTURE();
                do
                {
                    packet_status = OUTER_DISC_PARSE_PACKET();
                    if(packet_status == 0xfeu)
                        OUTER_DISC_YIELD();
                } while(packet_status == 0xfeu);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79A, 0u);
            }

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xfeu));
                if(retry_budget == 0u)
                    return;
                retry_budget = (u8)(retry_budget - 1u);
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) == 7u)
                    PublishOuterDiscAcquisitionFailure();
                if(OUTER_DISC_RESET_CONTROLLER() == 0u)
                    return;
                ConfigureOuterDiscProbeController();
                break;
            }

            if(ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_1235) <= outer_reference - 5500l)
                WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FC22, 0u);
            if(ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_1235) > outer_reference - 5500l)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 1u));
            if(ReadInitialTocFramePosition(FW_NEAR_UNKNOWN_1235) >= outer_reference - 1000l)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xfeu));
                if(ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FC22) >= 100u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) | 4u));
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1F, 32u);
                    WriteDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadDiscRecognitionWord(FW_NEAR_UNKNOWN_FAD7) | 0x40u));
                }
                CaptureOuterDiscControllerSamples();
                FinishOuterDiscProbeFromSamples(ordinary_mode);
                return;
            }
        }

        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
            PublishOuterDiscCountdownFailure();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F) != expected_speed)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xfeu));
            return;
        }
    }
}

#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/disc_recognition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fb9d58 */
u8 RunDiscRecognitionAndTocInitialization(void)
{
    u8 status;

    if(VALIDATE_DISC_SNAPSHOT() == 1u)
        return INITIALIZE_DISC_POST_TOC();

    status = INITIALIZE_DISC_CONTROLLER_ATIP();
    if(status != 0u)
        return status;

    status = ACQUIRE_DISC_INITIAL_TOC();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
    if(status != 0u)
        return status;

    status = CALIBRATE_DISC_RADIAL_MODEL();
    if(status != 0u)
        return status;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u)
    {
        status = BUILD_DISC_FULL_TOC();
        if(status != 0u)
            return status;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        status = ACQUIRE_DISC_PMA();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        if(status != 0u)
            return status;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 4u) == 0u)
        PROBE_DISC_OUTER_LIMIT();
    FINALIZE_DISC_POST_TOC();
    CONSTRAIN_DISC_READ_SPEED();
    APPLY_DISC_READ_SPEED();

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 4u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 4u) != 0u) && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 4u) == 0u)
    {
        u8 speed_limit = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, speed_limit);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed_limit);
    }
    return 0u;
}
