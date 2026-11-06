#include "primary_raw_read_pipeline_coroutine.h"
#include "../platform/coroutine_entries.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_sector_descriptor_scratch.h"
#include "primary_raw_read_operating_code.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_speed_update.h"

extern void YieldCoroutine2(void);
extern u8 CheckP1Bit3StableHighForCurrentMediaState(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/primary_raw_read_pipeline_coroutine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/primary_raw_read_pipeline_coroutine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPrimaryCoroutineDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePrimaryCoroutineWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WritePrimaryCoroutineDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:f90000 */
void RunPrimaryRawReadPipelineCoroutine2(void)
{
    u32 ring_slot_count;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    }
    else
    {
        if(READ_PRIMARY_COROUTINE_BIT_0020() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        }
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, 10u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, 0u);
            WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_0F00, ReadPrimaryCoroutineDword(FW_NEAR_SCAN_POSITION_FIRST));
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, 2u);
            WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_0F00, ReadPrimaryCoroutineDword(FW_NEAR_UNKNOWN_1598) + 0x20ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
        }
        WritePrimaryCoroutineDword(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT, 0u);
        WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_0F7F, 0u);
        WRITE_PRIMARY_COROUTINE_BIT_0025(0u);
        WRITE_PRIMARY_COROUTINE_BIT_0026(0u);
        WRITE_PRIMARY_COROUTINE_BIT_001F(0u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F08, 0x32u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 0u);
    if(READ_PRIMARY_COROUTINE_BIT_000C() == 0u)
        WritePrimaryCoroutineWord(FW_NEAR_UNKNOWN_0F3B, 0u);
    WRITE_PRIMARY_COROUTINE_BIT_0024(0u);
    WRITE_PRIMARY_COROUTINE_BIT_000E(0u);
    WRITE_PRIMARY_COROUTINE_BIT_0009(0u);
    WRITE_PRIMARY_COROUTINE_BIT_0004(0u);
    WRITE_PRIMARY_COROUTINE_BIT_0007(0u);
    WRITE_PRIMARY_COROUTINE_BIT_0018(0u);
    WRITE_PRIMARY_COROUTINE_BIT_0008(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 1u);

    UpdateRawReadSpeedForCurrentPipelineState();
    WritePrimaryCoroutineWord(FW_NEAR_UNKNOWN_1298, 0u);
    WritePrimaryCoroutineWord(FW_NEAR_UNKNOWN_1294, 0u);
    WritePrimaryCoroutineWord(FW_NEAR_UNKNOWN_1296, 0u);
    WritePrimaryCoroutineWord(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slot_count = ReadPrimaryCoroutineDword(FW_NEAR_UNKNOWN_156A);
    WritePrimaryCoroutineWord(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    ring_slot_count = ReadPrimaryCoroutineDword(FW_NEAR_UNKNOWN_156A);
    WritePrimaryCoroutineWord(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_1262, 0u);
    WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_1266, 0u);
    WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_126A, ReadPrimaryCoroutineDword(FW_NEAR_UNKNOWN_1562));
    WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_126E, 0u);

    while(READ_PRIMARY_COROUTINE_BIT_0009() == 0u)
    {
        if(RUN_PRIMARY_PIPELINE_STEP() != 0u)
        {
            WRITE_PRIMARY_COROUTINE_BIT_0005(0u);
            do
            {
                YIELD_PRIMARY_COROUTINE2();
                if(READ_PRIMARY_COROUTINE_BIT_0005() != 0u)
                {
                    if(CHECK_PRIMARY_COROUTINE_P1() != 0u)
                    {
                        WRITE_PRIMARY_COROUTINE_BIT_0005(0u);
                    }
                    else
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
                        BuildRawSectorDescriptorScratch(0x2100u, 5u);
                        EnqueueRawSectorDescriptor();
                        WRITE_PRIMARY_COROUTINE_BIT_0009(1u);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 0u);
                    }
                }
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F76) <= 0)
                    ServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x0281u);
            } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) == 4u);
            UpdatePrimaryRawReadOperatingCodeSelector();
        }
    }

    WritePrimaryCoroutineDword(FW_NEAR_UNKNOWN_1598, ReadPrimaryCoroutineDword(FW_NEAR_UNKNOWN_1598) + 1ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1591, 0u);
    FINALIZE_PRIMARY_COROUTINE2();
}
