#include "c4_acquisition.h"



static void initialize_radial_state(void) reentrant
{
    u16 radial;
    firmware_move_failure_count = 0;
    firmware_acquisition_position_match_count = 0;
    firmware_radial_calibration_flags &= 0x7f;
    firmware_media_control_flags &= 0xfb;
    firmware_move_status40_count = 0;
    firmware_acquisition_previous_error = 0;
    firmware_acquisition_repeated_error_flags = 0;
    call_position_ramp_controller_word();
    /* match both stock model calls, including the overwritten first result */
    (void)call_position_map_signed(0x00369d03);
    radial = call_position_map_zero();
    firmware_acquisition_radial_target = radial;
    firmware_acquisition_radial_target_high = (u8)(radial >> 8);
    firmware_acquisition_retry_count = (firmware_host_command_status & 1) != 0 ? 1 : 3;
    firmware_acquisition_packet_countdown = firmware_media_mode == 0 ? 0x3c : 0x50;
    firmware_acquisition_position_matched = 0;
    firmware_acquisition_target_nudge_pending = 0;
}


/* configure tracking without waiting for or interpreting a position packet */
static void configure_tracking(void) reentrant
{
    firmware_acquisition_error = 0;
    call_position_set_p1_bits();
    firmware_p1_t1 = 1;
    firmware_p1_t0 = 1;
    call_position_reset_controller_ports();
    call_position_select_operating_code(1);
    if(firmware_radial_model_refined_scale != 0 && (firmware_radial_model_flags & 4) != 0)
        firmware_radial_model_scale = firmware_radial_model_refined_scale;
    initialize_radial_state();
    firmware_acquisition_low_position_adjusted = 0;
    firmware_acquisition_negative_position = 0;
    call_position_write_port(0x80d0, 0);
    call_position_write_port(0x80d3, 0);

    call_position_apply_channels1_through3();
    firmware_drive_control_state = 1;
    firmware_acquisition_target_nudge_pending = 0;
    firmware_b = 0;
    firmware_acquisition_tolerance_flags &= 0xfe;
    call_position_configure_ports();
}


static void apply_conditional_ports(void)
{
    /* stock RAM:f868f8 and its callee at RAM:f86909 both return near */
    if((firmware_controller_port_setup_flags & 0x40) == 0)
    {
        call_acq_strobe_port(0x8079, firmware_media_mode == 2 ? 0x94 : 0x14);
        firmware_controller_port_809d_shadow = 0x40;
        call_position_write_port(0x809d, firmware_controller_port_809d_shadow);
        firmware_controller_port_809f_shadow = 0xb0;
        call_position_write_port(0x809f, firmware_controller_port_809f_shadow);
    }
}


void spiraldrive_process_c4_acquisition(void) reentrant
{
    firmware_startup_acquisition_pending = 0;
    call_acq_update_measurement();
    call_acq_reset_eight_channel();
    if(firmware_controller_operating_mode != 0)
        firmware_controller_mode_flags &= 0xfb;
    firmware_acquisition_tolerance_flags &= 0xfc;
    if(firmware_controller_operating_mode == 1)
        call_acq_set_operating_enable(0);

    firmware_acquisition_in_progress = 1;
    firmware_acquisition_alternate_path = 1;
    call_acq_select_operating_code();
    configure_tracking();
    call_acq_configure_alternate();
    call_acq_apply_operating_code();
    call_acq_reset_ports();
    apply_conditional_ports();
    call_acq_update_measurement();
    call_acq_prepare_command_0e();
    firmware_eight_channel_capture_flags |= 8;
    firmware_command_0e_capture_flags |= 0x10;
    firmware_acquisition_in_progress = 0;

    /* keep controller calibration serviced while the raw DMA producer runs */
    while(firmware_acquisition_cancel_requested == 0 && firmware_raw_read_state == 3 && firmware_raw_queue_control != 4)
    {
        call_acq_service_eight_channel();
        call_acq_service_command_0e();
        call_acq_yield1();
    }
    firmware_acquisition_cancel_requested = 0;
    call_acq_finalize_eight_channel();
}
