#include "raw_read_acquisition_plan.h"
#include "../platform/firmware_memory.h"
#include "configured_raw_read_preparation_segments.h"
#include "raw_read_preparation_range.h"
#include "raw_read_transfer_layout.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRawReadPlanU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u8 InitializeRawReadPlanRange(u8 alternate_plan)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 TestRawReadPlanEligibility(u8 alternate_state)
{
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void SetRawReadPlanSequenceError(void)
{
#define PLEXTOR_BINDING_SECTION 4
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 MapRawReadPlanStatus(u8 field_r7, u8 field_r6, u8 flags)
{
#define PLEXTOR_BINDING_SECTION 5
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 ExecuteRawReadPlanPendingRange(u8 state_eligible)
{
#define PLEXTOR_BINDING_SECTION 6
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 SelectRawReadPlanSectorClass(u8 state_eligible)
{
#define PLEXTOR_BINDING_SECTION 7
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 AcquireRawReadPlanSegments(u8 state_eligible, u8 alternate_plan)
{
#define PLEXTOR_BINDING_SECTION 8
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void SetRawReadPlanMsfTuples(s32 relative_position, s32 absolute_position)
{
#define PLEXTOR_BINDING_SECTION 9
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void ConfigureRawReadPlanLayout(void)
{
#define PLEXTOR_BINDING_SECTION 10
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void DecrementRawReadPlanMsf(u16 address)
{
#define PLEXTOR_BINDING_SECTION 11
#include "../platform/bindings/media/raw_read_acquisition_plan_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 IsRawReadPlanTypeZeroOrOne(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    return (u8)(write_type == 0u || write_type == 1u);
}

static void InitializeRawReadPlanQType(void)
{
    u8 field_r6;
    u8 field_r7;
    u8 flags;

    if(IsRawReadPlanTypeZeroOrOne() == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    }
    else
    {
        field_r6 = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD33);
        field_r7 = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD32);
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, MapRawReadPlanStatus(field_r7, field_r6, flags));
    }
}

static void RefreshRawReadPlanQTypeAndSectorClass(u8 state_eligible)
{
    if(IsRawReadPlanTypeZeroOrOne() == 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, SelectRawReadPlanSectorClass(state_eligible));
}

static void PublishRawReadPlanMsfTuples(void)
{
    s32 target;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        s32 start = (s32)ReadRawReadPlanU32(FW_NEAR_UNKNOWN_DD3D);

        target = (s32)ReadRawReadPlanU32(FW_NEAR_UNKNOWN_1598);
        SetRawReadPlanMsfTuples((s32)((u32)target - (u32)start), (s32)ReadRawReadPlanU32(FW_NEAR_UNKNOWN_1598));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_14B4) == 1u)
            SetRawReadPlanMsfTuples(0l, (s32)ReadRawReadPlanU32(FW_NEAR_UNKNOWN_1598));
    }
    else
    {
        SetRawReadPlanMsfTuples(0l, (s32)ReadRawReadPlanU32(FW_NEAR_UNKNOWN_1598));
    }
}

static void BuildRawReadPlanControllerQPacket(void)
{
    u8 index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 3u) == 1u)
    {
        for(index = 0u; index < 10u; ++index)
            FirmwareWrite8((fw_addr_t)(FW_NEAR_UNKNOWN_88AA + index), FirmwareReadNear8((fw_addr_t)(FW_NEAR_UNKNOWN_1272 + index)));
    }
    else
    {
        for(index = 0u; index < 10u; ++index)
            FirmwareWrite8((fw_addr_t)(FW_NEAR_UNKNOWN_88AA + index), FirmwareReadNear8((fw_addr_t)(FW_NEAR_UNKNOWN_1491 + index)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88AF, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88AE, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88AD, 0u);
        DecrementRawReadPlanMsf(FW_NEAR_UNKNOWN_88AD);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88AC, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
}

/* RAM:fe9af2 */
u8 PLEXTOR_NEAR_FUNCTION PrepareRawReadAcquisitionPlan(u8 alternate_plan)
{
    u8 state_eligible;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1286) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u && alternate_plan == 0u)
        return 1u;
    if(InitializeRawReadPlanRange(alternate_plan) == 0u)
        return 1u;

    state_eligible = TestRawReadPlanEligibility(1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) == 0u && state_eligible == 1u)
    {
        SetRawReadPlanSequenceError();
        return 0u;
    }

    InitializeRawReadPlanQType();
    if(ExecuteRawReadPlanPendingRange(state_eligible) == 0u)
        return 0u;

    state_eligible = TestRawReadPlanEligibility(0u);
    RefreshRawReadPlanQTypeAndSectorClass(state_eligible);
    if(AcquireRawReadPlanSegments(state_eligible, alternate_plan) == 0u)
        return 0u;

    PublishRawReadPlanMsfTuples();
    ConfigureRawReadPlanLayout();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AB, 1u);
    BuildRawReadPlanControllerQPacket();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1286, 1u);
    return 1u;
}
