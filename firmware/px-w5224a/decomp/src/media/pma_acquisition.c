#include "pma_acquisition.h"
#include "platform/controller_q_packet_process.h"
#include "platform/encoding.h"
#include "platform/firmware_memory.h"
#include "platform/memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/pma_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPmaU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePmaU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 FinishPmaIfComplete(void)
{
    if(PMA_STATE_COMPLETE() == 1u)
    {
        PMA_FINALIZE_START();
        return 1u;
    }
    (void)PMA_RESET_ACQUISITION(0u);
    return 0u;
}

/* RAM:fb8497 */
u8 AcquirePmaQRecordState(void)
{
    s32 lead_in = (s32)ReadPmaU32(FW_NEAR_UNKNOWN_114F);
    s32 anchor = lead_in - 1000l;
    u8 wrap_count = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
    PMA_RESET_TABLES();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 30u);
    WritePmaU32(FW_NEAR_UNKNOWN_0E3B, (u32)(lead_in - 1015l));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 13u || PMA_OPERATION_IN_TOC_SET() != 0u))
    {
        WritePmaU32(FW_NEAR_UNKNOWN_1143, (u32)anchor);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x80u));
        return 0u;
    }

    for(;;)
    {
        s32 first_position = 0l;
        u8 pulse_failures = 0u;
        u8 control_count = 0u;

        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
        {
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 4u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) != 0u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) != 0xffu
                && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
            {
                PMA_RESET_DRIVE();
                return 4u;
            }
            PMA_RESET_TABLES();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x80u));
            WritePmaU32(FW_NEAR_UNKNOWN_1143, (u32)anchor);
            return 0u;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        if(PMA_ACQUIRE_POSITION() != 1u)
        {
            if(PMA_RESET_ACQUISITION(1u) == 0u)
                return 4u;
            continue;
        }

        PMA_APPLY_OPERATING();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
        PMA_ARM_PACKET();

        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
        {
            u8 packet = PMA_PARSE_PACKET();
            s32 position = (s32)ReadPmaU32(FW_NEAR_UNKNOWN_1235);

            if(packet != 0xfeu)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
                {
                    if(PMA_RESET_ACQUISITION(1u) == 0u)
                        return 4u;
                    break;
                }
                if(first_position == 0l)
                    first_position = position;
                if(position < first_position)
                {
                    wrap_count++;
                    if(wrap_count > 9u
                        && (((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 4u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u) || FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == 0xffu))
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 0u);
                }
                PMA_ARM_PACKET();
                if(position == anchor - 10l)
                    PMA_REARM_PACKET();

                if(position >= anchor - 2l)
                {
                    if(PMA_PROCESS_PACKET() != 0xfeu)
                    {
                        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) < 0)
                            control_count++;
                        PMA_REARM_PACKET();
                        if(PMA_ACCUMULATE_PACKET() != 1u)
                        {
                            if(PMA_RESET_ACQUISITION(1u) == 0u)
                                return 4u;
                            break;
                        }
                    }
                    PMA_WAIT_TIMER(PMA_DIVIDE_TIMER());
                    if(PMA_PULSE_THRESHOLD(10u) != 0u)
                        pulse_failures = 0u;
                    else
                        pulse_failures++;
                    if(pulse_failures == 1u || control_count == 1u)
                    {
                        WritePmaU32(FW_NEAR_UNKNOWN_1143, (u32)position);
                    }
                    else if(pulse_failures > 10u || control_count > 5u)
                    {
                        if(FinishPmaIfComplete() != 0u)
                            return 0u;
                        break;
                    }
                }

                if(position <= anchor - 40l || position >= anchor + 1010l)
                {
                    (void)PMA_RESET_ACQUISITION(0u);
                    break;
                }
                if(position >= anchor + 999l)
                {
                    if(FinishPmaIfComplete() != 0u)
                        return 0u;
                    break;
                }
            }
            PMA_YIELD();
        }
    }
}

/* RAM:fb89c5 */
u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoPmaState(void)
{
    u8 accepted_mask = FirmwareReadNear8(FW_NEAR_UNKNOWN_125B);
    u8 record_index;
    fw_addr_t record_address;

    if((accepted_mask & 0x01u) != 0u)
    {
        record_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1247);
        if(record_index > FirmwareReadNear8(FW_NEAR_UNKNOWN_1149))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1149, record_index);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1148, record_index);
        }
        if(record_index < FirmwareReadNear8(FW_NEAR_UNKNOWN_1147))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, record_index);

        record_address = FW_HUGE_UNKNOWN_2BB03B + (fw_addr_t)record_index * 10ul;
        if(PMA_CONVERT_MSF(record_address + 7ul) <= PMA_CONVERT_MSF(FW_NEAR_UNKNOWN_124C))
            PMA_COPY_BYTES(record_address, FW_NEAR_UNKNOWN_1245, 10u);
        if(FirmwareRead8(record_address + 3ul) >= 0x5fu)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_102E, 1u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) != 0u)
        {
            u8 summary = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1245) & 0x40u) != 0u)
                summary |= 0x02u;
            else
                summary |= 0x01u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, summary);
        }
    }

    if((accepted_mask & 0x02u) != 0u)
    {
        PMA_COPY_BYTES(FW_NEAR_UNKNOWN_1245, FW_HUGE_UNKNOWN_2BB5DB, 10u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_114E, 1u);
    }

    if((accepted_mask & 0x04u) != 0u)
    {
        record_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1247);
        if(record_index > FirmwareReadNear8(FW_NEAR_UNKNOWN_114A))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_114A, record_index);
        PMA_COPY_BYTES(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_2BB423 + (fw_addr_t)record_index * 10ul, 10u);
    }

    if((accepted_mask & 0x08u) != 0u)
    {
        record_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1247);
        if(record_index > FirmwareReadNear8(FW_NEAR_UNKNOWN_114B))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_114B, record_index);
        PMA_COPY_BYTES(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_2BB4FF + (fw_addr_t)record_index * 10ul, 10u);
    }

    if((accepted_mask & 0x10u) != 0u)
    {
        record_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1247);
        if(record_index > FirmwareReadNear8(FW_NEAR_UNKNOWN_114C))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_114C, record_index);
        PMA_COPY_BYTES(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_2BB5E5 + (fw_addr_t)record_index * 10ul, 10u);
    }

    if((accepted_mask & 0x20u) != 0u)
    {
        record_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1247);
        if(record_index > FirmwareReadNear8(FW_NEAR_UNKNOWN_114D))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_114D, record_index);
        PMA_COPY_BYTES(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_2BB77F + (fw_addr_t)record_index * 10ul, 10u);
    }
    return 1u;
}
