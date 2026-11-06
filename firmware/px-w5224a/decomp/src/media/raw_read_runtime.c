#include "raw_read_runtime.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_parameters.h"
#include "../platform/controller_record_bank_scan.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/read_measurement_wait.h"
#include "../platform/status_mapping.h"
#include "../platform/track_state.h"
#include "../platform/type2_detected_layout_prepare.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_runtime_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_runtime_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/raw_read_runtime_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 4
#include "../platform/bindings/media/raw_read_runtime_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 5
#include "../platform/bindings/media/raw_read_runtime_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 6
#include "../platform/bindings/media/raw_read_runtime_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 7
#include "../platform/bindings/media/raw_read_runtime_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRawReadRuntimeDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static u16 ReadRawReadRuntimeWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteRawReadRuntimeWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteRawReadRuntimeDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fdcd1e */
void InitializeMediaReadControlState(void)
{
    u8 last_track_number;
    fw_addr_t last_track_tuple;
    s32 last_track_position;

    READ_CONTROL_RESET_QUEUE();
    while((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x20u) != 0u)
        READ_CONTROL_YIELD();

    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_12C5, ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_12C1) - 150ul);
    last_track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
    last_track_tuple = (fw_addr_t)(FW_HUGE_UNKNOWN_2BB03E + (u16)last_track_number * 10u);
    last_track_position = READ_CONTROL_CONVERT_MSF(last_track_tuple);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_157A, (u32)last_track_position);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_157E, (u32)last_track_position - 150ul);

    READ_CONTROL_CLEAR_BITS0017_0020();
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_0F3B, 0u);
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_0F3D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E4, 0u);
    READ_CONTROL_CLEAR_BIT003F();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F14, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F63, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E9, FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1147) > 6u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1148) & 0xbfu) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F63, 1u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) != 0xffu && FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) <= FirmwareReadNear8(FW_NEAR_UNKNOWN_F2BD))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E9, FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE));
    }

    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_15E5, 0x7ffffffful);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x77u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xf5u));
    READ_CONTROL_INITIALIZE_COROUTINE3(FW_ENTRY_RUN_MEDIA_READ_CONTROL_COROUTINE3);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74C, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E622, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E751, 0u);
}

/* RAM:f90b91 */
void PLEXTOR_NEAR_FUNCTION ThrottleRawReadProducerForQueueCapacity(void)
{
    u32 inherited_dr12 = CAPTURE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER();
    u16 threshold;

    INITIALIZE_CONTROLLER_CURSORS_FOR_PRODUCER_THROTTLE();
    if(ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292) != 0u)
    {
        if(FirmwareReadBit000E() != 0u)
            threshold = 0x80u;
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 0u)
            threshold = 0x10u;
        else
            threshold = 0x40u;

        if((u16)((u16)(ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A) - 6u) - ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292)) < threshold)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F72, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 0u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) != 0u)
            {
                INITIALIZE_RAW_READ_SPEED_FOR_PRODUCER_THROTTLE();
                WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_0F10, 0xfffffff6ul);
            }
            DISPATCH_READ_ACQUISITION_FOR_PRODUCER_THROTTLE();
            do
            {
                YIELD_COROUTINE2_FOR_PRODUCER_THROTTLE();
                if(ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292) == 0u)
                    break;
            } while((u16)((u16)(ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A) - 6u) - ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292)) < threshold);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1591) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1591, 0u);
                UPDATE_RAW_READ_SPEED_FOR_PRODUCER_THROTTLE();
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 0u);
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F72, 0u);
    RESTORE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER(inherited_dr12);
}

/* RAM:fa905b */
void InitializeRawReadPipelineRuntimeState(void)
{
    u32 slot_count;

    RESET_RAW_READ_PIPELINE_BUFFERS_AND_STATE();
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1298, 0u);
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1294, 0u);
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1296, 0u);
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292, 0u);

    slot_count = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A);
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1u));
    slot_count = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A);
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6u));
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1262, 0u);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1266, 0u);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_126A, ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1562));
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_126E, 0u);

    INITIALIZE_RAW_READ_SECTOR_CONTROL_STATE();
    CLEAR_RAW_READ_PIPELINE_STATUS_STATE();
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1488, 0u);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_148C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 0x0au);
}

/* RAM:fab60f */
void HandleReadMeasurementTargetPacketMatch(void)
{
    u8 controller_flags;
    u8 expected_byte;
    u8 mode;
    u8 profile_value;

    if(READ_MEASUREMENT_TARGET_PENDING() == 0u)
        return;
    expected_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_154E);
    if(expected_byte != FirmwareReadNear8(FW_NEAR_UNKNOWN_1133))
        return;
    expected_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_154F);
    if(expected_byte != FirmwareReadNear8(FW_NEAR_UNKNOWN_1134))
        return;
    expected_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_1550);
    if(expected_byte != FirmwareReadNear8(FW_NEAR_UNKNOWN_1135))
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E);
    if(mode <= 3u)
        profile_value = 0x47u;
    else
        profile_value = 0x40u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, profile_value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, profile_value);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFD, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x30u);
    SET_READ_MEASUREMENT_CONTROLLER_ACTIVE();
    controller_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_flags);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_flags);
    CLEAR_READ_MEASUREMENT_TARGET_PENDING();
}

/* RAM:fd9afd */
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForHighDescriptorMode(void)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E);
    u32 target;
    u8 target_offset;

    if(mode == 0u || mode == 9u)
    {
        target_offset = 2u;
    }
    else if(mode == 8u || (mode >= 10u && mode <= 13u))
    {
        target_offset = 3u;
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 7u);
        return;
    }

    target = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_FBA2);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_FBA6, target - (u32)target_offset);
    PrepareReadMeasurementAtAdjustedTarget();
}

/* RAM:fe2abe */
u8 RestartReadAcquisitionCoroutineForProducerWait(void)
{
    return RestartReadAcquisitionCoroutineForProducerWaitBody();
}

/* RAM:fe2dad */
u32 AdvanceRawReadRingPosition(u32 main_data_cursor)
{
    u32 ring_span = MultiplyU32Low(ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A), 0x930ul);
    u32 next_cursor = main_data_cursor + 0x930ul;

    if(next_cursor > ring_span - 1ul)
        next_cursor = 0ul;
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_126E, next_cursor);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 1u)
    {
        u32 next_index = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1266) + 1ul;
        WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1266, next_index);
        if(next_index == ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A))
            WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1266, 0ul);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_14A6) == 1u)
    {
        u32 next_index = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1262) + 1ul;
        WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1262, next_index);
        if(next_index == ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A))
            WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1262, 0ul);
    }
    return next_cursor;
}

/* RAM:fb7f5b */
void PLEXTOR_NEAR_FUNCTION LoadControllerBufferSetupFromCurrentRawDescriptor(void)
{
    u16 queued_count = ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292);
    u16 descriptor_address = 0u;
    u32 descriptor_value;
    u16 descriptor_word;
    u8 descriptor_flags;
    u8 descriptor_mode;

    if(queued_count != 0u)
    {
        u16 consumer_index = ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1294);
        u16 descriptor_offset = (u16)(consumer_index * 0x16u);
        u16 descriptor_base = (u16)ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_129E);

        descriptor_address = (u16)(descriptor_base + descriptor_offset);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9B, FirmwareReadNear8(FW_NEAR_UNKNOWN_1281));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1282));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1283));

    descriptor_mode = FirmwareReadNearDisplaced8(descriptor_address, 0x15u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9E, descriptor_mode);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E) == 0x41u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9E, 0x0eu);

    descriptor_value = (u32)(((u16)FirmwareReadNearDisplaced8(descriptor_address, 2u) << 8) | FirmwareReadNearDisplaced8(descriptor_address, 3u));
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_FBA2, descriptor_value);
    descriptor_value = ((u32)FirmwareReadNearDisplaced8(descriptor_address, 4u) << 24) | ((u32)FirmwareReadNearDisplaced8(descriptor_address, 5u) << 16)
                     | ((u32)FirmwareReadNearDisplaced8(descriptor_address, 6u) << 8) | FirmwareReadNearDisplaced8(descriptor_address, 7u);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_FBAA, descriptor_value);

    descriptor_word = (u16)(((u16)FirmwareReadNearDisplaced8(descriptor_address, 8u) << 8) | FirmwareReadNearDisplaced8(descriptor_address, 9u));
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_FBAE, descriptor_word);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_FBB0, (u32)(u16)(descriptor_address + 10u));

    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_FBB6, 0u);
    descriptor_flags = FirmwareReadNearDisplaced8(descriptor_address, 0x14u);
    if((descriptor_flags & 0x80u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBB6, 1u);
    if((descriptor_flags & 0x40u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBB7, 1u);

    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_FBB4, ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292));
}

/* RAM:f93e5a */
void PLEXTOR_NEAR_FUNCTION ResetControllerEventBit7TransferState(void)
{
    u8 metadata[10];
    u8 controller_flags;
    u8 index;
    u8 slot_index;
    u32 cursor;
    u32 inherited_dr12 = CAPTURE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER();

    controller_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0x7fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_flags);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_flags);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 1u);
    CLEAR_EVENT_BIT7_TRANSFER_BITS_ATOMIC();
    RESET_CONTROLLER_PORTS_FOR_EVENT_BIT7_TRANSFER();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018079, 0x44u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807D, 0x54u);

    slot_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F5C);
    cursor = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156E) + ((u32)slot_index << 4);
    WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_15D8, cursor);
    for(index = 0u; index < 10u; ++index)
    {
        cursor = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_15D8);
        WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_15D8, cursor + 1u);
        metadata[index] = READ_CONTROLLER_BUFFER_BYTE_FOR_EVENT_BIT7_TRANSFER(cursor);
    }
    for(index = 0u; index < 10u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF2 + index, metadata[index]);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 7u)
    {
        SET_RAW_READ_TERMINAL_BIT_FOR_EVENT_BIT7_TRANSFER();
        RESET_RAW_READ_QUEUE_FOR_EVENT_BIT7_TRANSFER();
        WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1598, ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_0F32));
        DISPATCH_REQUEST5_FOR_EVENT_BIT7_TRANSFER();
    }
    SET_P3_RX_HIGH_TX_LOW_FOR_EVENT_BIT7_TRANSFER();
    RESTORE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER(inherited_dr12);
}

/* RAM:fbcade */
void InitializeMediaReadTrackModelAndPipelineState(void)
{
    u32 atip_last_lead_out;
    u16 scan_result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD55, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 0u);
    MEDIA_READ_INIT_CLEAR_WORKING_TRACK();
    MEDIA_READ_INIT_RESET_TRACK_TABLE();
    MEDIA_READ_INIT_UPDATE_SESSION_REFERENCE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
    MEDIA_READ_INIT_POPULATE_FULL_TOC_TRACKS();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
    {
        MEDIA_READ_INIT_SESSION_BOUNDS();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD63) == 0u)
                MEDIA_READ_INIT_WORKING_TRACK();
            else
                MEDIA_READ_INIT_SESSION_RAW_Q_TRACKS();
        }
    }

    MEDIA_READ_INIT_RESET_TYPE2_PIPELINE();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        (void)MEDIA_READ_INIT_PREPARE_TYPE2_PIPELINE();

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E625) & 0xc0u) != 0u)
        {
            u8 default_value = FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A0C));

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E625, default_value);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3C, FirmwareReadNear8(FW_NEAR_UNKNOWN_E625));
        }
    }
    else
    {
        atip_last_lead_out = ReadRawReadRuntimeDword(FW_NEAR_CURRENT_FRAME_POSITION);
        scan_result = MEDIA_READ_INIT_SCAN_BANKS(1u, atip_last_lead_out, 0l);
        if(scan_result == 0u)
        {
            WriteRawReadRuntimeDword(FW_NEAR_SCAN_POSITION_SECOND, atip_last_lead_out);
            WriteRawReadRuntimeDword(FW_NEAR_SCAN_POSITION_FIRST, atip_last_lead_out);
        }
        else
        {
            WriteRawReadRuntimeDword(FW_NEAR_SCAN_POSITION_FIRST, ReadRawReadRuntimeDword(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT));
            WriteRawReadRuntimeDword(FW_NEAR_SCAN_POSITION_SECOND, ReadRawReadRuntimeDword(FW_HUGE_UNKNOWN_2B970C));
        }
    }

    MEDIA_READ_INIT_SET_LAYOUT_BIT(MEDIA_READ_INIT_LAYOUT_RESULT() != 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 0x40u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0xf7u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
    {
        MEDIA_READ_INIT_RESET_RAW_READ();
        MEDIA_READ_INIT_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        MEDIA_READ_INIT_YIELD3();
        MEDIA_READ_INIT_YIELD3();
        MEDIA_READ_INIT_SNAPSHOT_TRACK_TAIL();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_0E3B, 150ul);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158B, 0u);
        WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1589, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E8, 100u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E7, 100u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0xfeu));
    }
}

/* RAM:fc6721 */
void UpdateRawReadProgressCountersAndPhaseFlags(void)
{
    u32 command_position_delta;
    u32 transfer_block_count;
    u16 progress_count;
    u8 phase_count;

    if(READ_RAW_READ_PROGRESS_BIT003F() != 0u)
        return;

    command_position_delta = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_0B02) + 150ul - ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_0F14);
    if((s32)command_position_delta > 100l && FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) < 10u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);

    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) != FirmwareReadNear8(FW_NEAR_UNKNOWN_15E4))
    {
        RESET_RAW_READ_PROGRESS_STATE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E4, FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE));
        return;
    }
    if(READ_RAW_READ_PROGRESS_BIT003E() != 0u)
        return;

    if(READ_RAW_READ_PROGRESS_BIT003D() == 0u)
    {
        transfer_block_count = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1455);
        if(transfer_block_count < 0x023aul)
            WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_15E0, (u16)(ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_15E0) + (u16)transfer_block_count));
        else
            RESET_RAW_READ_PROGRESS_STATE();
        return;
    }

    progress_count = ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_15E0);
    if((s16)progress_count > 570)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1591) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1591, 0u);
            RESET_RAW_READ_QUEUE_FOR_PROGRESS();
            return;
        }
        phase_count = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E3) + 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E3, phase_count);
        if(phase_count > 2u || progress_count >= 825u)
        {
            RESET_RAW_READ_PROGRESS_STATE();
            return;
        }
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E3, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) < 10u)
        {
            if(progress_count < 150u && READ_RAW_READ_PROGRESS_BIT0040() != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) + 1u));
            else
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
        }
    }

    FirmwareWriteBit003D(0u);
    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_15E0, 0u);
    FirmwareWriteBit003E(1u);
    FirmwareWriteBit0040(1u);
}

/* RAM:fc683c */
u8 PrepareRawReadDescriptorBatchForInspection(void)
{
    u16 secondary_offset;
    u16 available_count;
    u16 descriptor_address;
    u16 retained_capacity;
    u8 alternate_offset;
    u32 threshold;
    u32 remaining_blocks;

    secondary_offset = (u16)((ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1298) - ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1294)) & ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_129A));
    available_count = (u16)(ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292) - secondary_offset);
    alternate_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46);

    if(available_count <= (u16)alternate_offset)
    {
        if(FirmwareReadBit0008() != 1u && alternate_offset == 0u)
        {
            DESCRIPTOR_BATCH_RESET_QUEUE();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 2u)
                DESCRIPTOR_BATCH_INITIALIZE_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
            else
                DESCRIPTOR_BATCH_INITIALIZE_COROUTINE2(FW_ENTRY_RUN_ALTERNATE_RAW_READ_PIPELINE_COROUTINE2);
        }
        FirmwareWriteBit0040(0u);
        return 0u;
    }

    threshold = (u32)alternate_offset + (u32)ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_145F);
    if((u32)available_count >= threshold)
        return 1u;

    remaining_blocks = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1455) - ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1459);
    if((u32)available_count >= (u32)alternate_offset + remaining_blocks)
        return 1u;
    if(available_count >= 23u)
        return 1u;

    DESCRIPTOR_BATCH_SET_EX0(0u);
    descriptor_address = DESCRIPTOR_BATCH_LATEST_ADDRESS();
    DESCRIPTOR_BATCH_SET_EX0(1u);
    if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) != 0u)
    {
        if(DESCRIPTOR_BATCH_VALIDATE(descriptor_address) == 0u)
            return 1u;
        if(FirmwareReadBit0008() != 1u)
            return 1u;
    }

    secondary_offset = (u16)((ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1298) - ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1294)) & ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_129A));
    retained_capacity = (u16)(ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_129C) - secondary_offset);
    if(retained_capacity < ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_145F))
        DESCRIPTOR_BATCH_CONSUME(secondary_offset);
    FirmwareWriteBit0040(0u);
    return 0u;
}

/* RAM:fc65f4 */
u16 CountValidatedRawReadDescriptorsForHostTransfer(void)
{
    u16 saved_secondary_cursor = ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1298);
    u32 remaining_blocks = ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1455) - ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1459);
    u16 scan_limit = remaining_blocks < 23ul ? (u16)remaining_blocks : 22u;
    u16 scan_index;
    u16 accepted_count = 0u;

    for(scan_index = 1u; scan_index <= scan_limit; ++scan_index)
    {
        u16 descriptor_address;
        u16 secondary_cursor;
        u16 backlog;
        u16 available_after_cursor;

        ++accepted_count;
        descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
        if(FirmwareRead8((fw_addr_t)descriptor_address + 0x10u) != 0u)
        {
            if(HOST_TRANSFER_COUNT_VALIDATE(descriptor_address) == 0u)
            {
                --accepted_count;
                break;
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1468) != 0u)
                break;
        }

        if(HOST_TRANSFER_COUNT_MAPPING_BIT() != 0u)
        {
            s32 logical_block = (s32)(ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_15DC) + ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_1459) + (u32)accepted_count - 1ul);
            s32 expected_frame;

            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xb9u)
                expected_frame = logical_block + 150l;
            else
                expected_frame = HOST_TRANSFER_COUNT_MAP_LBA(logical_block);
            if((s32)ReadRawReadRuntimeDword((fw_addr_t)descriptor_address + 2u) != expected_frame)
            {
                --accepted_count;
                break;
            }
        }

        secondary_cursor = (u16)((ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1298) + 1u) & ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_129A));
        WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1298, secondary_cursor);
        backlog = (u16)((secondary_cursor - ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1294)) & ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_129A));
        available_after_cursor = (u16)(ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292) - backlog);
        if(available_after_cursor <= (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46))
            break;
    }

    WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1298, saved_secondary_cursor);
    return accepted_count;
}

/* RAM:fc28c0 */
void PLEXTOR_NEAR_FUNCTION ApplyModePage0ERuntimeImageToRawReadState(void)
{
    u8 mode_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_E691);

    if((mode_flags & 4u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x10u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xefu));

    if((mode_flags & 2u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x20u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xdfu));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F95E, FirmwareReadNear8(FW_NEAR_UNKNOWN_E697));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F95F, FirmwareReadNear8(FW_NEAR_UNKNOWN_E698));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F960, FirmwareReadNear8(FW_NEAR_UNKNOWN_E699));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F961, FirmwareReadNear8(FW_NEAR_UNKNOWN_E69A));
    MODE_PAGE_0E_BUILD_SHADOWS();

    if(FirmwareReadBit000E() != 0u && ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292) > 40u)
    {
        u16 descriptor_address;
        u16 linked_address;
        u16 target_high_word;
        u16 producer_index;
        u16 ring_slot_count;

        MODE_PAGE_0E_INITIALIZE_CURSORS();
        MODE_PAGE_0E_SET_EX0(0u);
        WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1292, 21u);
        producer_index = (u16)(ReadRawReadRuntimeWord(FW_NEAR_UNKNOWN_1294) + 20u);
        WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1296, producer_index);
        ring_slot_count = (u16)ReadRawReadRuntimeDword(FW_NEAR_UNKNOWN_156A);
        if(producer_index > (u16)(ring_slot_count - 1u))
        {
            producer_index = (u16)(producer_index - ring_slot_count);
            WriteRawReadRuntimeWord(FW_NEAR_UNKNOWN_1296, producer_index);
        }
        descriptor_address = MODE_PAGE_0E_LATEST_DESCRIPTOR();
        linked_address = ReadRawReadRuntimeWord((fw_addr_t)(u16)(descriptor_address + 2u));
        target_high_word = ReadRawReadRuntimeWord((fw_addr_t)linked_address);
        WriteRawReadRuntimeDword(FW_NEAR_UNKNOWN_1598, ((u32)target_high_word << 16) | (u32)descriptor_address);
        MODE_PAGE_0E_SET_EX0(1u);
        MODE_PAGE_0E_YIELD0();
    }

    MODE_PAGE_0E_APPLY_SHADOWS();
}
