#include "blank_format_raw_read_plan.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/blank_format_raw_read_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPlanU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u8 AcquireLeadInPrelude(u8 operation)
{
    u32 lead_in;
    u32 terminal_count;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8010, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8011, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    lead_in = ReadPlanU32(FW_NEAR_UNKNOWN_114F);
    if(PLAN_ACQUIRE((s32)(lead_in - 1000ul), 1000ul, 0u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 0u);
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 1u);
    lead_in = ReadPlanU32(FW_NEAR_UNKNOWN_114F);
    if(PLAN_ACQUIRE((s32)lead_in, 0ul - lead_in, 1u) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    if(operation == 0u)
        terminal_count = ReadPlanU32(FW_NEAR_CURRENT_FRAME_POSITION) + 2250ul;
    else
        terminal_count = ReadPlanU32(FW_NEAR_UNKNOWN_E606) + 2ul;
    return PLAN_ACQUIRE(0l, terminal_count, 2u);
}

static u8 AcquireTenRecordQSequence(void)
{
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8010, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8011, 10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    result = PLAN_Q_SEQUENCE(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 0u);
    return (u8)(result != 0u);
}

/* RAM:fe7e4b */
u8 ExecuteBlankOrFormatRawReadPlan(void)
{
    u8 operation;
    s32 start;
    u32 count;
    u32 end;
    u32 reference;

    PLAN_RESET();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B6, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B8, FirmwareReadNear8(FW_NEAR_UNKNOWN_E600));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 1u);
    operation = FirmwareReadNear8(FW_NEAR_UNKNOWN_E600);

    switch(operation)
    {
    case 0u:
        return AcquireLeadInPrelude(0u);

    case 1u:
        return AcquireLeadInPrelude(1u);

    case 2u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x10u) != 0u)
        {
            reference = ReadPlanU32(FW_NEAR_UNKNOWN_DD39);
            start = (s32)(reference + 145ul);
            reference = ReadPlanU32(FW_NEAR_UNKNOWN_DD39);
            end = ReadPlanU32(FW_NEAR_UNKNOWN_DD41);
            count = end - reference - 145ul;
        }
        else
        {
            reference = ReadPlanU32(FW_NEAR_UNKNOWN_DD39);
            start = (s32)reference;
            reference = ReadPlanU32(FW_NEAR_UNKNOWN_DD39);
            end = ReadPlanU32(FW_NEAR_UNKNOWN_DD41);
            count = end - reference;
        }
        return PLAN_ACQUIRE(start, count, 2u);

    case 3u:
        if(AcquireTenRecordQSequence() == 0u)
            return 0u;
        operation = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
        if(operation == 0u || (operation == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x10u) == 0u))
            return 1u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
        reference = ReadPlanU32(FW_NEAR_UNKNOWN_DD39);
        start = (s32)reference;
        reference = ReadPlanU32(FW_NEAR_UNKNOWN_DD39);
        end = ReadPlanU32(FW_NEAR_UNKNOWN_DD41);
        count = end - reference;
        return PLAN_ACQUIRE(start, count, 2u);

    case 4u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
        start = (s32)(ReadPlanU32(FW_NEAR_UNKNOWN_E602) + 145ul);
        count = ReadPlanU32(FW_NEAR_UNKNOWN_DD41) - (u32)start;
        return PLAN_ACQUIRE(start, count, 2u);

    case 5u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 1u);
        start = (s32)ReadPlanU32(FW_NEAR_UNKNOWN_E612);
        count = 0ul - ReadPlanU32(FW_NEAR_UNKNOWN_E612);
        count += ReadPlanU32(FW_NEAR_UNKNOWN_E616) + 1ul;
        if(PLAN_ACQUIRE(start, count, 1u) == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 3u);
        start = (s32)ReadPlanU32(FW_NEAR_UNKNOWN_E61A);
        count = 0ul - ReadPlanU32(FW_NEAR_UNKNOWN_E61A);
        count += ReadPlanU32(FW_NEAR_UNKNOWN_E61E) + 1ul;
        return PLAN_ACQUIRE(start, count, 2u);

    case 6u:
        operation = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD63);
        if(operation != 1u && operation != 2u)
            return 1u;
        if(AcquireTenRecordQSequence() == 0u)
            return 0u;
        if(operation == 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 1u);
            start = (s32)ReadPlanU32(FW_NEAR_UNKNOWN_E612);
            count = ReadPlanU32(FW_NEAR_UNKNOWN_E616) - (u32)start + 1ul;
            if(PLAN_ACQUIRE(start, count, 1u) == 0u)
                return 0u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
        start = (s32)(ReadPlanU32(FW_NEAR_UNKNOWN_E616) + 1ul);
        count = ReadPlanU32(FW_NEAR_UNKNOWN_E61E) - (u32)start;
        return PLAN_ACQUIRE(start, count, 2u);

    default:
        return 1u;
    }
}
