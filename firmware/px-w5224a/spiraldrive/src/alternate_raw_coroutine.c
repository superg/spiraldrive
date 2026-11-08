#include "alternate_raw_coroutine.h"
#include "raw_capture.h"
#include "raw_sector_range.h"



void spiraldrive_alternate_raw_read_coroutine2(void) reentrant
{
    firmware_raw_q_target_probe_enabled = 1; /* select the raw-buffer descriptor layout */
    firmware_raw_read_terminal = 0;
    firmware_raw_queue_control = 6;
    firmware_raw_controller_slot = 0;
    firmware_raw_controller_secondary_slot = 0;
    firmware_raw_auxiliary_buffer_cursor = firmware_raw_auxiliary_buffer_base;
    firmware_raw_main_buffer_cursor = 0;

    while(firmware_acquisition_in_progress != 0)
        call_alternate_raw_read_yield2();
    if(firmware_raw_read_state == 3 && firmware_raw_producer_error == 0)
    {
        start_raw_capture();
        while(firmware_raw_pipeline_state != 0 && firmware_raw_read_state == 3)
            call_alternate_raw_read_yield2();
    }
    call_alternate_helper_initialize_cursors();
    /* release the final buffered frame, but never restart capture across an unreported gap */
    firmware_alternate_read_active = 0;
    for(;;)
        call_alternate_raw_read_yield2();
}
