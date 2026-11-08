#include "raw_sector_range.h"
#include "alternate_raw_coroutine.h"
#include "c4_handler.h"
#include "c4_media_coroutine.h"
#include "c4_sense.h"
#include "firmware_memory_map.h"
#include "firmware_timing.h"



typedef enum
{
    SPIRALDRIVE_C4_POLL_FAILED = 0,
    SPIRALDRIVE_C4_POLL_COMPLETE = 1,
    SPIRALDRIVE_C4_POLL_CAPTURE_STOPPED = 0xfa,
    SPIRALDRIVE_C4_POLL_DISCONTINUITY = 0xfb,
    SPIRALDRIVE_C4_POLL_INTERNAL_ERROR = 0xfc,
    SPIRALDRIVE_C4_POLL_TIMEOUT = 0xfd,
    SPIRALDRIVE_C4_POLL_PENDING = 0xfe
} SpiralDrivePollStatus;


static void configure_c4_transfer(u8 components) reentrant
{
    call_configure_ata_data_transfer_status();
    firmware_controller_port_801f = 0;
    firmware_controller_port_8021 = 0;
    firmware_controller_port_8020 = components;
}


static void append_c4_transfer_component(u32 address, u16 byte_count) reentrant
{
    firmware_controller_port_800d = (u8)byte_count;
    firmware_controller_port_800e = (u8)(byte_count >> 8);
    firmware_controller_port_8010 = (u8)address;
    firmware_controller_port_8011 = (u8)(address >> 8);
    firmware_controller_port_8012 = (u8)(address >> 16);
}


static u8 transfer_c4_components(u16 byte_count, u16 completed_frames) reentrant
{
    /* stock completion advances both the descriptor cursor and frame count by this value */
    firmware_raw_host_batch_count = completed_frames;
    firmware_raw_transfer_block_length = byte_count;
    firmware_raw_host_transfer_active = 1;
    firmware_controller_port_800d = (u8)byte_count;
    firmware_controller_port_800e = (u8)(byte_count >> 8);
    firmware_controller_port_8056 = 0x50;
    firmware_controller_port_8022 = 0x80;
    firmware_controller_port_8051 = 2;
    firmware_controller_port_805e = 0x97;
    while(firmware_raw_host_transfer_active != 0)
    {
        if((firmware_host_completion_flags & 0x10) != 0 || firmware_sense_key != 0)
            return 0;
        call_stock_yield0();
    }
    return (firmware_host_completion_flags & 0x10) == 0 && firmware_sense_key == 0;
}


static u8 poll_c4_host_transfer(void) reentrant
{
    volatile FirmwareRawDescriptor near *descriptor;
    u32 q_address;

    /* retain stock abort and producer-error handling */
    if((firmware_host_completion_flags & 0x10) != 0 || firmware_raw_producer_error != 0)
        return call_raw_range_poll_host_transfer();

    firmware_raw_batch_threshold = 1;
    if(call_prepare_raw_batch() == 0)
        return SPIRALDRIVE_C4_POLL_PENDING;
    if(call_count_raw_batch() == 0)
        return SPIRALDRIVE_C4_POLL_FAILED;

    descriptor = (volatile FirmwareRawDescriptor near *)(u16)firmware_raw_descriptor_ring_base + firmware_raw_secondary_cursor;
    if(descriptor->c4_subq_slot == 0xffff)
        q_address = firmware_raw_subcode_buffer_base + firmware_raw_ring_size * SPIRALDRIVE_C4_SUB_SIZE;
    else
        /* controller slots remain 16 bytes apart even though only the 12-byte Q packet is returned */
        q_address = firmware_raw_q_buffer_base + (u32)descriptor->c4_subq_slot * 16;

    /* use the stock three-component mode and offsets, retaining the descriptor until SUBQ completes */
    configure_c4_transfer(0x83);
    append_c4_transfer_component(descriptor->firmware_main_cursor + 0x110, SPIRALDRIVE_C4_DATA_SIZE);
    append_c4_transfer_component(descriptor->firmware_auxiliary_cursor + 0x22, SPIRALDRIVE_C4_C2_SIZE);
    append_c4_transfer_component(firmware_raw_subcode_buffer_base + (u32)descriptor->firmware_subcode_slot * SPIRALDRIVE_C4_SUB_SIZE, SPIRALDRIVE_C4_SUB_SIZE);
    if(transfer_c4_components(SPIRALDRIVE_C4_FRAME_SIZE - SPIRALDRIVE_C4_SUBQ_SIZE, 0) == 0)
        return SPIRALDRIVE_C4_POLL_FAILED;

    /* the second DATA IN phase appends the unmodified Q packet and commits exactly one frame */
    configure_c4_transfer(0x80);
    append_c4_transfer_component(q_address, SPIRALDRIVE_C4_SUBQ_SIZE);
    if(transfer_c4_components(SPIRALDRIVE_C4_SUBQ_SIZE, 1) == 0)
        return SPIRALDRIVE_C4_POLL_FAILED;
    return firmware_read_completed_count == firmware_read_requested_count ? SPIRALDRIVE_C4_POLL_COMPLETE : SPIRALDRIVE_C4_POLL_PENDING;
}


static void report_read_failure(u8 status) reentrant
{
    /* retain native failure details, but discard its synthesized LBA information */
    if(firmware_sense_key != 0)
        spiraldrive_report_c4_failure(firmware_sense_key, ((u16)firmware_sense_asc << 8) | firmware_sense_ascq);
    else if(status == SPIRALDRIVE_C4_POLL_DISCONTINUITY)
        spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_ABORTED_COMMAND, SPIRALDRIVE_ASC_LOSS_OF_STREAMING);
    else if(status == SPIRALDRIVE_C4_POLL_TIMEOUT)
        spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_ABORTED_COMMAND, SPIRALDRIVE_ASC_INSUFFICIENT_TIME);
    else if(status == SPIRALDRIVE_C4_POLL_CAPTURE_STOPPED || (firmware_host_completion_flags & 0x10) != 0)
        spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_ABORTED_COMMAND, SPIRALDRIVE_ASC_IO_PROCESS_TERMINATED);
    else
        spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_HARDWARE_ERROR, SPIRALDRIVE_ASC_INTERNAL_TARGET_FAILURE);
}


/* transfer a frame count from the current C4 stream, with a deadline for stalled capture */
u8 spiraldrive_execute_raw_sector_read_range(u32 frame_count, u16 timeout_ms) reentrant
{
    u8 status;
    u8 last_tick;
    u32 elapsed = 0;
    u32 completed;
    u32 elapsed_budget = ((u32)timeout_ms * 1000 + FIRMWARE_TIMER_TICK_US - 1) / FIRMWARE_TIMER_TICK_US;

    /* only failures from this command may supply the final sense */
    call_stock_no_sense();

    /* use host progress as a count rather than an acquisition address */
    firmware_raw_read_start_position = 0;
    firmware_raw_read_end_position = frame_count;
    firmware_read_negative_range_30000 = 0;
    firmware_read_negative_range_20000 = 0;
    firmware_unknown_bit_001b = 0;
    firmware_raw_read_sequence_state = 0;

    firmware_raw_read_phase = 0;
    firmware_p3_rxd = 0;
    firmware_p3_txd = 0;
    firmware_read_requested_count = frame_count;
    call_raw_range_update_progress();
    firmware_track_packet_mapping_active = 0;
    firmware_raw_transfer_position = 0;

    if(firmware_raw_read_state != 3 || firmware_raw_read_format != 4)
    {
        firmware_raw_transfer_progress_state = 0;
        call_raw_range_reset_queue();
        firmware_raw_read_state = 3;
        firmware_raw_read_format = 4;
        firmware_raw_producer_error = 0;
        firmware_alternate_read_active = 1;
        firmware_raw_queue_control = 6;
        firmware_raw_producer_throttled = 0;
        firmware_raw_pipeline_target = 0; /* a descriptor sequence, not a disc address */
        /* initialize speed before coroutine 1 can apply it, including when setup finishes without yielding to coroutine 2 */
        firmware_read_acquisition_packet_flags &= 0xf9;
        initialize_c4_speed();
        firmware_media_state = 2;
        firmware_acquisition_cancel_requested = 0;
        firmware_acquisition_in_progress = 1;
        call_raw_range_initialize_coroutine1((u32)spiraldrive_c4_media_coroutine1_entry);
        call_raw_range_initialize_coroutine2((u32)spiraldrive_alternate_raw_read_coroutine2);
    }

    firmware_read_completed_count = 0;
    firmware_read_cd_alternate_length = 0;
    firmware_read_transfer_length = SPIRALDRIVE_C4_FRAME_SIZE;
    firmware_unknown_bit_0001 = 0;

    /* use the free-running tick byte, since controller setup can reload stock countdowns */
    /* the stock poll completes active ATAPI batches before the next timeout check */
    last_tick = firmware_timer_ticks;
    completed = 0;

    do
    {
        u16 secondary_offset;
        u16 available;
        /* guard stock RAM:fc683c, which resets an empty stopped queue and installs the stock producer, to preserve the C4 cursor and keep acquisition from restarting behind the host */
        secondary_offset = (u16)((firmware_raw_secondary_cursor - firmware_raw_consumer_cursor) & firmware_raw_ring_mask);
        available = firmware_raw_queue_count;
        /* the stock poll indexes a fixed producer-error table without a bounds check */
        if(firmware_raw_producer_error >= sizeof(firmware_raw_read_sense_keys) || (firmware_raw_producer_error == 0 && available < secondary_offset))
        {
            status = SPIRALDRIVE_C4_POLL_INTERNAL_ERROR;
            break;
        }
        if(firmware_raw_producer_error == 0 && firmware_alternate_read_active == 0 && firmware_raw_queue_control != 4)
        {
            status = SPIRALDRIVE_C4_POLL_CAPTURE_STOPPED;
            break;
        }
        if(firmware_raw_producer_error == 0 && firmware_raw_queue_control == 4 && firmware_alternate_read_active == 0)
        {
            /* drain the stopped buffer, including a short final batch, before reporting the gap */
            available -= secondary_offset;
            if(available == 0)
            {
                status = SPIRALDRIVE_C4_POLL_DISCONTINUITY;
                break;
            }
            if(firmware_read_requested_count - firmware_read_completed_count > available)
                firmware_read_requested_count = firmware_read_completed_count + available;
        }
        status = poll_c4_host_transfer();
        if(status == SPIRALDRIVE_C4_POLL_PENDING)
        {
            u32 now_completed = firmware_read_completed_count;
            u8 now_tick = firmware_timer_ticks;
            elapsed += (u8)(now_tick - last_tick);
            last_tick = now_tick;
            if(now_completed != completed)
            {
                completed = now_completed;
                elapsed = 0;
            }
            else if(elapsed >= elapsed_budget)
            {
                status = SPIRALDRIVE_C4_POLL_TIMEOUT;
                break;
            }
            call_stock_yield0();
        }
    } while(status == SPIRALDRIVE_C4_POLL_PENDING);

    firmware_read_cd_alternate_length = 0;
    firmware_raw_read_phase = 4;
    firmware_p3_rxd = 1;
    firmware_p3_txd = 1;
    if(status == SPIRALDRIVE_C4_POLL_COMPLETE && firmware_read_completed_count == frame_count && firmware_sense_key == 0)
    {
        call_stock_no_sense();
        return 1;
    }
    if(status == SPIRALDRIVE_C4_POLL_COMPLETE && firmware_read_completed_count == firmware_read_requested_count && firmware_raw_queue_control == 4)
        status = SPIRALDRIVE_C4_POLL_DISCONTINUITY;

    /* treat incomplete transfers as errors even when the stock poll reports success */
    /* report the failure before queue reset so the caller retains CHECK CONDITION and clears read mode */
    /* the next READ starts fresh acquisition at the current pickup */
    /* keep the stock idle coroutine but skip stock recovery request 5, which can reposition the pickup after a failed descriptor */
    report_read_failure(status);
    call_raw_range_reset_queue();
    call_raw_range_initialize_coroutine2(0xf84a00);
    return 0;
}
