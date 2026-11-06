#include "format_plan_q.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/format_plan_q_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadFormatPlanU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u8 SelectFormatPlanTrackNumber(u8 effective_last_track)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) != 0u)
        return effective_last_track;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u)
        return 1u;
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u);
}

static void ClearFormatPlanRecordFields(fw_addr_t record)
{
    FirmwareWrite8(record + 1u, 0u);
    FirmwareWrite8(record + 3u, 0u);
    FirmwareWrite8(record + 4u, 0u);
    FirmwareWrite8(record + 5u, 0u);
    FirmwareWrite8(record + 6u, 0u);
}

static void PackFormatPlanMsf(fw_addr_t tuple)
{
    FirmwareWrite8(tuple, FORMAT_PLAN_BCD(FirmwareRead8(tuple)));
    FirmwareWrite8(tuple + 1u, FORMAT_PLAN_BCD(FirmwareRead8(tuple + 1u)));
    FirmwareWrite8(tuple + 2u, FORMAT_PLAN_BCD(FirmwareRead8(tuple + 2u)));
}

/* RAM:fb3863 */
void PLEXTOR_NEAR_FUNCTION BuildFormatPlanPrimaryStagedRawQRecordSequence(void)
{
    fw_addr_t record = FW_NEAR_UNKNOWN_2BC11B;
    u8 control_adr = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0xfdu) << 4) | 1u);
    u8 effective_last_track = FORMAT_PLAN_LAST_TRACK();

    FirmwareWrite8(record, control_adr);
    FirmwareWrite8(record + 2u, 0xa0u);
    ClearFormatPlanRecordFields(record);
    FirmwareWrite8(record + 7u, FORMAT_PLAN_BCD(SelectFormatPlanTrackNumber(effective_last_track)));
    FirmwareWrite8(record + 8u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A));
    FirmwareWrite8(record + 9u, 0u);

    record += 10u;
    FirmwareWrite8(record, control_adr);
    FirmwareWrite8(record + 2u, 0xa1u);
    ClearFormatPlanRecordFields(record);
    FirmwareWrite8(record + 7u, FORMAT_PLAN_BCD(SelectFormatPlanTrackNumber(effective_last_track)));
    FirmwareWrite8(record + 8u, 0u);
    FirmwareWrite8(record + 9u, 0u);

    record += 10u;
    FirmwareWrite8(record, control_adr);
    FirmwareWrite8(record + 2u, 0xa2u);
    ClearFormatPlanRecordFields(record);
    FORMAT_PLAN_FRAME_TO_MSF((s32)ReadFormatPlanU32(FW_NEAR_UNKNOWN_E61A), record + 7u);
    PackFormatPlanMsf(record + 7u);

    record += 10u;
    FirmwareWrite8(record, control_adr);
    FirmwareWrite8(record + 2u, FORMAT_PLAN_BCD(SelectFormatPlanTrackNumber(effective_last_track)));
    ClearFormatPlanRecordFields(record);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u)
        FORMAT_PLAN_FRAME_TO_MSF((s32)(ReadFormatPlanU32(FW_NEAR_UNKNOWN_E616) + 0x97ul), record + 7u);
    else
        FORMAT_PLAN_FRAME_TO_MSF((s32)ReadFormatPlanU32(FW_NEAR_UNKNOWN_DD3D), record + 7u);
    PackFormatPlanMsf(record + 7u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC11A, 4u);
}

/* RAM:fb3b5c */
void PLEXTOR_NEAR_FUNCTION BuildFormatPlanSecondaryStagedRawQRecordSequence(void) PLEXTOR_REENTRANT
{
    fw_addr_t destination = FW_HUGE_UNKNOWN_2BC519;
    u8 operation = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45);
    u8 track_mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0xfdu);
    u8 control = (u8)((track_mode << 4) | 5u);
    u8 emit_b0 = 1u;
    u8 suppress_c = 0u;
    u8 sequence_count = 0u;
    u8 pma_count;

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, 0u);
    FirmwareWrite8(destination, control);
    FirmwareWrite8(destination + 2u, 0xb0u);
    if(operation == 0x40u)
    {
        FirmwareWrite8(destination + 3u, 0xffu);
        FirmwareWrite8(destination + 4u, 0xffu);
        FirmwareWrite8(destination + 5u, 0xffu);
    }
    else if(operation == 0xc0u)
    {
        FORMAT_PLAN_FRAME_TO_MSF((s32)(ReadFormatPlanU32(FW_NEAR_UNKNOWN_E61E) + 0x1194ul), destination + 3u);
        PackFormatPlanMsf(destination + 3u);
    }
    else if(operation == 0u)
    {
        emit_b0 = 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            FirmwareWrite8(destination + 3u, 0xffu);
            FirmwareWrite8(destination + 4u, 0xffu);
            FirmwareWrite8(destination + 5u, 0xffu);
            emit_b0 = 1u;
        }
    }
    if(emit_b0 != 0u)
    {
        FirmwareWrite8(destination + 7u, FORMAT_PLAN_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_113B)));
        FirmwareWrite8(destination + 8u, FORMAT_PLAN_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_113C)));
        FirmwareWrite8(destination + 9u, FORMAT_PLAN_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_113D)));
        FirmwareWrite8(destination + 6u, 1u);
        FirmwareWrite8(destination + 1u, 0u);
        destination += 10u;
        sequence_count = 1u;
    }

    if(operation == 0x40u)
    {
        if(emit_b0 != 0u && track_mode == 0u)
            suppress_c = 1u;
    }
    else if(operation == 0u)
    {
        suppress_c = emit_b0;
    }

    FORMAT_PLAN_BUILD_PMA(destination, pma_count);
    destination += ((fw_addr_t)pma_count << 3) + ((fw_addr_t)pma_count << 1);
    sequence_count = (u8)(sequence_count + pma_count);

    if(emit_b0 != 0u && (suppress_c == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u) && (s32)ReadFormatPlanU32(FW_NEAR_UNKNOWN_DD69) < 0)
    {
        FirmwareWrite8(destination, control);
        FirmwareWrite8(destination + 2u, 0xc0u);
        FirmwareWrite8(destination + 7u, FORMAT_PLAN_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_113F)));
        FirmwareWrite8(destination + 8u, FORMAT_PLAN_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_1140)));
        FirmwareWrite8(destination + 9u, FORMAT_PLAN_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_1141)));
        FirmwareWrite8(destination + 3u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7A) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7B)) << 1));
        FirmwareWrite8(destination + 4u, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) << 1));
        FirmwareWrite8(destination + 5u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7D) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E)) << 1));
        FirmwareWrite8(destination + 1u, 0u);
        FirmwareWrite8(destination + 6u, 0u);
        sequence_count++;
        destination += 10u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            FirmwareWrite8(destination, control);
            FirmwareWrite8(destination + 2u, 0xc1u);
            FirmwareWrite8(destination + 3u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7F) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80)) << 1));
            FirmwareWrite8(destination + 4u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB81) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB82)) << 1));
            FirmwareWrite8(destination + 5u, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB83) << 1));
            FirmwareWrite8(destination + 7u, 0u);
            FirmwareWrite8(destination + 8u, 0u);
            FirmwareWrite8(destination + 9u, 0u);
            FirmwareWrite8(destination + 1u, 0u);
            FirmwareWrite8(destination + 6u, 0u);
            sequence_count++;
        }
    }
    if(emit_b0 != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BC51F, FORMAT_PLAN_UNPACK_BCD(sequence_count));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, sequence_count);
}
