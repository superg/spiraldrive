#include "raw_read_position_probe.h"
#include "../platform/firmware_memory.h"
#include "../platform/raw_sector_descriptor_scratch.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_position_probe_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f90c46 */
void PLEXTOR_NEAR_FUNCTION InitializeRawReadControllerForPositionProbe(void)
{
    u8 value;
    s8 format_index;

    CLEAR_POSITION_PROBE_BIT0014();
    CLEAR_POSITION_PROBE_BIT0015();
    CLEAR_POSITION_PROBE_BIT0007();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F21, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 0u);

    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFD) | 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFD, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, value);
    if(READ_POSITION_PROBE_BIT0013() != 0u || READ_POSITION_PROBE_BIT001C() != 0u || READ_POSITION_PROBE_BIT001D() != 0u)
        value = 0xccu;
    else
        value = 0x4cu;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x28u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0u);

    format_index = (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);
    if(format_index > 0)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) | 0x60u));
        if(format_index == 1)
        {
            value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) & 0x1fu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, value);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, value);
            value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0x86u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, value);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, value);
        }
        else
        {
            value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0xe6u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, value);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, value);
        }
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x28u);
    }
    else
    {
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x20u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, value);
    ENABLE_POSITION_PROBE_EXTERNAL_INTERRUPT0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F76, 0x7du);
}

/* RAM:f92c27 */
void PLEXTOR_NEAR_FUNCTION ArmRawSectorBufferCapture(void)
{
    u8 value;
    u8 format_index;
    u16 producer_index;
    u32 buffer_offset;

    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xdfu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x20u);
    CLEAR_RAW_SECTOR_CAPTURE_BIT0009();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F48, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 6u);

    format_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);
    if(READ_POSITION_PROBE_BIT0013() != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F20, 5u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F20, FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF95B5) + format_index));

    producer_index = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_1296);
    buffer_offset = (u32)producer_index * 0x0930ul;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801C, (u8)producer_index);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801D, (u8)(producer_index >> 8));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) == 0u)
        value = 8u;
    else
        value = 0x0cu;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018007, (u8)buffer_offset);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, (u8)(buffer_offset >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, (u8)(buffer_offset >> 16));

    if(format_index == 4u)
        value = 0x81u;
    else
        value = 0x80u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x20u);
    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x20u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, value);
}

static void WritePositionProbeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WritePositionProbeU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u32 ReadPositionProbeU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u16 ReadPositionProbeU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* RAM:f92a69 */
u8 PLEXTOR_NEAR_FUNCTION ProcessAlternateRawReadPipelineStep(void)
{
    s8 countdown = CAPTURE_ALTERNATE_RAW_READ_STEP_COUNTDOWN();
    u8 acquisition_result;
    u32 comparison_bound;
    u32 pipeline_target;
    u16 descriptor_status;
    u8 descriptor_mode;
    s8 previous_countdown;

    CLEAR_ALTERNATE_RAW_READ_STEP_TERMINAL();
    for(;;)
    {
        if(READ_ALTERNATE_RAW_READ_STEP_TERMINAL() != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F48, 0u);
            return 0u;
        }

        THROTTLE_ALTERNATE_RAW_READ_STEP();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F48, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4A, 0u);
        if(READ_ALTERNATE_RAW_READ_STEP_SPECIAL_MODE() != 0u)
            acquisition_result = REQUEST_ALTERNATE_RAW_READ_STEP((s32)0x10000000ul, 0l);
        else
            acquisition_result = REQUEST_ALTERNATE_RAW_READ_STEP((s32)ReadPositionProbeU32(FW_NEAR_UNKNOWN_1598), -10l);

        if(acquisition_result != 0u)
        {
            if(CHECK_ALTERNATE_RAW_READ_STEP_P1() != 0u)
            {
                comparison_bound = ReadPositionProbeU32(FW_NEAR_UNKNOWN_12C1);
                pipeline_target = ReadPositionProbeU32(FW_NEAR_UNKNOWN_1598);
                if((s32)pipeline_target < (s32)comparison_bound)
                {
                    descriptor_status = ReadPositionProbeU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + ((u32)acquisition_result << 1));
                    descriptor_mode = FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + acquisition_result);
                    BUILD_ALTERNATE_RAW_READ_STEP_DESCRIPTOR(descriptor_status, descriptor_mode);
                }
                else
                {
                    BUILD_ALTERNATE_RAW_READ_STEP_DESCRIPTOR(0x2100u, 5u);
                }
            }
            else
            {
                BUILD_ALTERNATE_RAW_READ_STEP_DESCRIPTOR(0x2100u, 5u);
            }
            ENQUEUE_ALTERNATE_RAW_READ_STEP_DESCRIPTOR();
            SET_ALTERNATE_RAW_READ_STEP_TERMINAL();
            return 0u;
        }

        INITIALIZE_ALTERNATE_RAW_READ_STEP_CONTROLLER();
        do
        {
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F76) <= 0)
            {
                previous_countdown = countdown;
                countdown = (s8)(countdown - 1);
                if(previous_countdown <= 0)
                {
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 0x14u)
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0u);
                    SERVICE_ALTERNATE_RAW_READ_STEP_COUNTDOWN();
                }
            }
            else
            {
                countdown = 10;
            }
            YIELD_ALTERNATE_RAW_READ_STEP();
        } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) == 6u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) != 0u)
            return 1u;
    }
}

/* RAM:f9352d */
void RunRawReadPositionProbeCoroutine2(void)
{
    u32 ring_slot_count;

    WritePositionProbeU32(FW_NEAR_UNKNOWN_159C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F06, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
    INITIALIZE_POSITION_PROBE_COROUTINE_BITS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) | 2u));
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1298, 0u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1294, 0u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1296, 0u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slot_count = ReadPositionProbeU32(FW_NEAR_UNKNOWN_156A);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    ring_slot_count = ReadPositionProbeU32(FW_NEAR_UNKNOWN_156A);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WritePositionProbeU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WritePositionProbeU32(FW_NEAR_UNKNOWN_1262, 0u);
    WritePositionProbeU32(FW_NEAR_UNKNOWN_1266, 0u);
    WritePositionProbeU32(FW_NEAR_UNKNOWN_126A, ReadPositionProbeU32(FW_NEAR_UNKNOWN_1562));
    WritePositionProbeU32(FW_NEAR_UNKNOWN_126E, 0u);

    while(READ_POSITION_PROBE_COROUTINE_TERMINAL() == 0u)
    {
        if(RUN_POSITION_PROBE_COROUTINE_STEP() != 0u)
        {
            do
            {
                YIELD_POSITION_PROBE_COROUTINE2();
            } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) == 9u);
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfdu));
    YIELD_POSITION_PROBE_COROUTINE2();
    FINALIZE_POSITION_PROBE_COROUTINE2();
}

/* RAM:f929ab */
void RunAlternateRawReadPipelineCoroutine2(void)
{
    u32 ring_slot_count;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F46, 1u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_0F5E, 0u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_0F5A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
    CLEAR_ALTERNATE_RAW_READ_TERMINAL();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0x19u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F61, 6u);
    INITIALIZE_ALTERNATE_RAW_READ_SPEED();
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1298, 0u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1294, 0u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1296, 0u);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slot_count = ReadPositionProbeU32(FW_NEAR_UNKNOWN_156A);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    ring_slot_count = ReadPositionProbeU32(FW_NEAR_UNKNOWN_156A);
    WritePositionProbeU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WritePositionProbeU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WritePositionProbeU32(FW_NEAR_UNKNOWN_1262, 0u);
    WritePositionProbeU32(FW_NEAR_UNKNOWN_1266, 0u);
    WritePositionProbeU32(FW_NEAR_UNKNOWN_126A, ReadPositionProbeU32(FW_NEAR_UNKNOWN_1562));
    WritePositionProbeU32(FW_NEAR_UNKNOWN_126E, 0u);

    do
    {
        if(RUN_ALTERNATE_RAW_READ_STEP() == 0u)
            FINALIZE_ALTERNATE_RAW_READ_COROUTINE2();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) == 7u)
            YIELD_ALTERNATE_RAW_READ_COROUTINE2();
    } while(READ_ALTERNATE_RAW_READ_TERMINAL() == 0u);
    FINALIZE_ALTERNATE_RAW_READ_COROUTINE2();
}
