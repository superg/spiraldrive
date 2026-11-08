#include "c4_handler.h"
#include "c4_media_coroutine.h"
#include "c4_sense.h"
#include "firmware_memory_map.h"
#include "firmware_timing.h"
#include "raw_capture.h"
#include "raw_sector_range.h"



static u8 wait_for_controller_wake(void) reentrant
{
    u8 last_tick = firmware_timer_ticks;
    u16 elapsed = 0;

    /* SET CD SPEED can return while the native coroutine is still waking the controller */
    while(((firmware_raw_read_options & SPIRALDRIVE_C4_STREAM_MARKER) == 0 || firmware_raw_read_state != 3) && (firmware_media_state == 2 || firmware_media_state == 3)
          && (firmware_controller_channel1_value == 0 || firmware_controller_channel2_value == 0 || firmware_drive_control_state == 6))
    {
        u8 now_tick = firmware_timer_ticks;
        elapsed += (u8)(now_tick - last_tick);
        last_tick = now_tick;
        if(elapsed >= 5000000 / FIRMWARE_TIMER_TICK_US)
            return 0;
        /* preserve the native wake-up stack and its saved controller registers until setup completes */
        call_stock_yield0();
    }
    return 1;
}


/* keep stream identity after GOOD clears the command-in-progress bit by using RAM:000f0b bit 2 */
/* stock raw transfer selectors leave bit 2 unused because their option index starts at bit 3 and the C2 selector uses bit 1 */
static void stop_capture(void) reentrant
{
    if((firmware_raw_read_options & SPIRALDRIVE_C4_STREAM_MARKER) != 0)
    {
        /* finish pending C4 setup or idle speed changes before stopping the stream */
        firmware_read_command_active = 1;
        while((firmware_raw_read_options & SPIRALDRIVE_C4_STREAM_MARKER) != 0 && (firmware_media_state == 2 || firmware_media_state == 3) && firmware_acquisition_in_progress != 0)
            call_stock_yield0();
        call_alternate_helper_initialize_cursors();
    }
    firmware_raw_read_options &= (u8)~SPIRALDRIVE_C4_STREAM_MARKER;
    firmware_raw_read_state = 0;
    firmware_read_lead_in = 0;
    firmware_read_command_active = 0;
}


static u8 wait_for_seek_speed(void) reentrant
{
    u8 last_tick = firmware_timer_ticks;
    u16 elapsed = 0;

    for(;;)
    {
        u8 now_tick = firmware_timer_ticks;
        elapsed += (u8)(now_tick - last_tick);
        last_tick = now_tick;
        if(firmware_sense_key != 0)
            return 0;
        if((firmware_raw_read_options & SPIRALDRIVE_C4_STREAM_MARKER) == 0 || (firmware_media_state != 2 && firmware_media_state != 3))
        {
            spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_NOT_READY, SPIRALDRIVE_ASC_NOT_READY);
            return 0;
        }
        if(firmware_acquisition_cancel_requested != 0 || elapsed >= 5000000 / FIRMWARE_TIMER_TICK_US)
        {
            spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_ABORTED_COMMAND, firmware_acquisition_cancel_requested != 0 ? SPIRALDRIVE_ASC_IO_PROCESS_TERMINATED : SPIRALDRIVE_ASC_INSUFFICIENT_TIME);
            firmware_acquisition_cancel_requested = 1;
            return 0;
        }
        if(firmware_acquisition_in_progress == 0)
        {
            if(firmware_operating_code <= firmware_operating_code_limit)
                return 1;
            spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_HARDWARE_ERROR, SPIRALDRIVE_ASC_INTERNAL_TARGET_FAILURE);
            return 0;
        }
        call_stock_yield0();
    }
}


static void fail(u8 sense_key, u16 sense_code) reentrant
{
    stop_capture();
    spiraldrive_report_c4_failure(sense_key, sense_code);
}


static void fail_transfer(void) reentrant
{
    /* snapshot native sense before cleanup can yield to another coroutine */
    if(firmware_sense_key != 0)
        fail(firmware_sense_key, ((u16)firmware_sense_asc << 8) | firmware_sense_ascq);
    else
        fail(SPIRALDRIVE_SENSE_ABORTED_COMMAND, SPIRALDRIVE_ASC_DATA_PHASE_ERROR);
}


void handle_scsi_opcode_c4(void) reentrant
{
    volatile SpiralDriveCDB near *const cdb = (volatile SpiralDriveCDB near *)firmware_command_buffer;

    if(cdb->reserved != 0)
    {
        fail(SPIRALDRIVE_SENSE_ILLEGAL_REQUEST, SPIRALDRIVE_ASC_INVALID_FIELD_IN_CDB);
        return;
    }

    if(cdb->operation == SPIRALDRIVE_OPERATION_SEEK)
    {
        u8 result = SPIRALDRIVE_SEEK_NONE;

        if(cdb->value < -32767 || cdb->value > 32767)
        {
            fail(SPIRALDRIVE_SENSE_ILLEGAL_REQUEST, SPIRALDRIVE_ASC_INVALID_FIELD_IN_CDB);
            return;
        }

        /* keep the next READ from reusing descriptors captured before this move */
        stop_capture();
        call_stock_no_sense();

        if(cdb->value != 0 && wait_for_controller_wake() == 0)
        {
            fail(SPIRALDRIVE_SENSE_NOT_READY, SPIRALDRIVE_ASC_BECOMING_READY);
            return;
        }

        /* replace the stock producer and position-holding worker before yielding during a raw move */
        firmware_read_command_active = 1;
        call_raw_range_reset_queue();
        call_raw_range_initialize_coroutine2(0xf84a00);
        /* wake the stopped controller for movement, while SEEK(0) leaves standby intact */
        if((firmware_media_state == 0 && cdb->value != 0)
            || ((firmware_media_state == 2 || firmware_media_state == 3) && firmware_controller_channel1_value != 0 && firmware_controller_channel2_value != 0))
        {
            firmware_raw_read_options |= SPIRALDRIVE_C4_STREAM_MARKER;
            firmware_acquisition_cancel_requested = 0;
            firmware_acquisition_in_progress = 0;
            firmware_media_state = 2;
            if(cdb->value != 0)
            {
                initialize_c4_speed();
                firmware_acquisition_in_progress = 1;
            }
            call_raw_range_initialize_coroutine1((u32)spiraldrive_c4_media_coroutine1_entry);
        }

        if(cdb->value != 0)
        {
            u16 magnitude = (u16)(cdb->value < 0 ? -cdb->value : cdb->value);
            u8 direction = cdb->value < 0;

            if(wait_for_seek_speed() != 0)
                result = spiraldrive_move_pickup(magnitude, direction);
            if(firmware_sense_key != 0)
            {
                fail(firmware_sense_key, ((u16)firmware_sense_asc << 8) | firmware_sense_ascq);
                return;
            }
        }
        firmware_read_command_active = 0;

        firmware_response_buffer[0] = (u8)result;
        if(call_stock_response_data_in(1, (u32)firmware_response_buffer) != 5 || firmware_sense_key != 0)
            fail_transfer();
    }
    else if(cdb->operation == SPIRALDRIVE_OPERATION_READ)
    {
        if(cdb->timeout_ms == 0 || cdb->value <= 0)
        {
            fail(SPIRALDRIVE_SENSE_ILLEGAL_REQUEST, SPIRALDRIVE_ASC_INVALID_FIELD_IN_CDB);
            return;
        }

        if(wait_for_controller_wake() == 0)
        {
            spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_NOT_READY, SPIRALDRIVE_ASC_BECOMING_READY);
            return;
        }

        /* force fresh C4 acquisition unless the previous command retained the C4 stream marker, since mode/format cannot distinguish a leftover D8 producer from a continuing C4 stream */
        if(firmware_read_cd_mode != 0 || (firmware_raw_read_options & SPIRALDRIVE_C4_STREAM_MARKER) == 0)
            firmware_raw_read_state = 7;
        firmware_raw_read_options = 2 | SPIRALDRIVE_C4_STREAM_MARKER;
        firmware_read_cd_mode = 0;
        firmware_read_lead_in = 0;
        firmware_raw_event7_transfer_enabled = 0;
        firmware_raw_scan_enabled = 0;
        firmware_read_command_active = 1;
        if(spiraldrive_execute_raw_sector_read_range((u32)cdb->value, cdb->timeout_ms) != 1)
        {
            /* preserve the range failure while stopping capture */
            fail_transfer();
            return;
        }
        if(firmware_read_completed_count != (u32)cdb->value)
        {
            /* guard against the completed count changing after the range returns */
            fail(SPIRALDRIVE_SENSE_HARDWARE_ERROR, SPIRALDRIVE_ASC_INTERNAL_TARGET_FAILURE);
            return;
        }
        firmware_read_command_active = 0;
    }
    /* reject unexpected operations */
    else
    {
        fail(SPIRALDRIVE_SENSE_ILLEGAL_REQUEST, SPIRALDRIVE_ASC_INVALID_FIELD_IN_CDB);
    }
}
