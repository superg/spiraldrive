#include "c4_media_coroutine.h"
#include "c4_acquisition.h"
#include "c4_handler.h"
#include "c4_sense.h"
#include "firmware_memory_map.h"
#include "raw_sector_range.h"



void call_select_operating_code_for_position(u32 position);
void call_restore_controller_d6(void);
u8 call_initialize_controller_operating_state(void);


void initialize_c4_speed(void) reentrant
{
    firmware_operating_code_selection = 1;
    call_stock_initialize_raw_read_speed();
    /* retain D8's lead-in speed ceiling without enabling its positioning or subcode policy */
    if(firmware_operating_code_limit > 4)
        firmware_operating_code_limit = 4;
    firmware_operating_code_candidate = firmware_operating_code_limit;
    firmware_raw_read_speed_limit = firmware_operating_code_limit;
    firmware_read_operating_code_override = 0;
}


/* return zero on success or the standard ASC/ASCQ explaining why startup failed */
static u16 wake_controller(void) reentrant
{
    if(firmware_controller_channel1_value != 0 && firmware_controller_channel2_value != 0)
        return 0;

    /* only use D8's warm wake-up path; cold calibration can home the pickup */
    if((firmware_drive_control_flags & 1) != 0 || (firmware_controller_calibration_status & 0x8000) != 0)
        return SPIRALDRIVE_ASC_INITIALIZATION_REQUIRED;
    call_position_set_p1_bits();
    call_restore_controller_d6();
    if(call_initialize_controller_operating_state() == 0)
        return SPIRALDRIVE_ASC_INTERNAL_TARGET_FAILURE;
    /* native wake-up restores controller profiles before C4 applies its retained ceiling */
    initialize_c4_speed();
    return 0;
}


static u8 idle_capture(void) reentrant
{
    firmware_eight_channel_capture_flags &= 0xf7;
    firmware_command_0e_capture_flags &= 0xef;
    /* retain the normal servo controls after DMA stops, without discarding queued frames */
    /* reuse stock state-3 speed selection and its timer, without its position acquisition worker */
    firmware_media_state = 3;
    firmware_media_idle_countdown = firmware_media_idle_timeout;
    firmware_media_idle_speed_countdown = 10;
    while(firmware_media_state == 3 && firmware_acquisition_cancel_requested == 0)
    {
        if(firmware_read_command_active == 0)
        {
            firmware_acquisition_in_progress = 1;
            call_acq_select_operating_code();
            if(firmware_controller_operating_mode == 0)
                call_position_select_operating_code(1);
            else
                call_select_operating_code_for_position(firmware_operating_code_reference_position);
            firmware_acquisition_in_progress = 0;
            if(firmware_media_state == 3 && firmware_acquisition_cancel_requested == 0 && firmware_media_idle_inhibited == 0 && firmware_media_idle_timeout != 0
                && (s32)firmware_media_idle_countdown < 0)
            {
                /* use the stock idle shutdown, including its flag that prevents homing */
                firmware_media_state = 0;
                firmware_controller_calibration_status |= 0x10;
                firmware_p3_rxd = 1;
                firmware_p3_txd = 0;
                firmware_operating_code_transition_flags |= 2;
                return 1;
            }
        }
        call_acq_yield1();
    }
    return 0;
}


/* custom C4 state-2 route through stock 1.04 RunMediaStateCoroutine1 at RAM:f85d74 */
/* the entry adapter returns other states to the unchanged stock scheduler and keeps C4 position requests out of its state-2 worker */
void spiraldrive_run_c4_media_coroutine1(void) reentrant
{
    u8 previous_state = 0xff;
    u16 startup_error;

    firmware_acquisition_distance_retry_count = 0;
    firmware_controller_calibration_flags &= 0xdf;
    firmware_media_state1_retry_count = 0;
    if(firmware_startup_status != 0)
        firmware_media_capture_flags |= 8;

    startup_error = wake_controller();
    if(startup_error != 0)
    {
        if(firmware_sense_key == 0)
            spiraldrive_report_c4_failure(startup_error == SPIRALDRIVE_ASC_INITIALIZATION_REQUIRED ? SPIRALDRIVE_SENSE_NOT_READY : SPIRALDRIVE_SENSE_HARDWARE_ERROR, startup_error);
        firmware_raw_read_state = 0;
        firmware_acquisition_cancel_requested = 1;
        firmware_acquisition_in_progress = 0;
        firmware_operating_code_transition_flags |= 2;
        firmware_media_state = 0;
        return;
    }

    for(;;)
    {
        u8 state;

        if(firmware_media_state == 2 && firmware_raw_read_state == 0)
        {
            /* finish SEEK speed preparation on the coroutine used by native settling waits */
            if(firmware_acquisition_in_progress != 0 && firmware_acquisition_cancel_requested == 0)
            {
                call_acq_select_operating_code();
                call_position_select_operating_code(1);
                firmware_acquisition_in_progress = 0;
            }
            if(firmware_media_state == 2 && firmware_acquisition_cancel_requested == 0)
                if(idle_capture() != 0)
                    return;
        }
        firmware_operating_code_transition_flags &= 0xfe;
        firmware_controller_profile_flags &= 0xfe;
        state = firmware_media_state;
        if(previous_state != 2 || state != 2)
            firmware_acquisition_distance_retry_count = 0;
        previous_state = state;

        if(state != 2 || firmware_raw_read_state != 3 || (firmware_raw_read_options & 4) == 0)
        {
            /* end C4 acquisition on a state change so the C4 range path sees stopped capture and fails before a later stock state-2 iteration can move the pickup */
            firmware_raw_read_state = 0;
            firmware_raw_read_options &= (u8)~SPIRALDRIVE_C4_STREAM_MARKER;
            return;
        }

        firmware_controller_calibration_flags |= 0x20;
        spiraldrive_process_c4_acquisition();
        call_c4_finalize_controller_capture();
        firmware_media_capture_flags &= 0x7f;
        firmware_controller_mode_flags &= 0xfd;
        firmware_controller_calibration_flags &= 0xdf;
        firmware_media_capture_flags |= 8;

        if(firmware_media_state == 2 && (firmware_raw_read_state == 0 || (firmware_raw_queue_control == 4 && firmware_raw_read_state == 3)))
            if(idle_capture() != 0)
                return;
    }
}
