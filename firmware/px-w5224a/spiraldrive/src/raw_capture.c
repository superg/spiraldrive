#include "raw_capture.h"
#include "c4_handler.h"



void start_raw_capture(void) reentrant
{
    /* reserve a zero slot for unavailable SUB or SUBQ in the gap before the Q ring */
    call_clear_controller_buffer(firmware_raw_subcode_buffer_base + firmware_raw_ring_size * SPIRALDRIVE_C4_SUB_SIZE, SPIRALDRIVE_C4_SUB_SIZE);
    firmware_auxiliary_cursor_initialized = 0;
    firmware_main_cursor_initialized = 0;
    firmware_raw_pipeline_state = 5;
    firmware_controller_port_8002_shadow = 0;
    firmware_controller_port_8002 = 0;
    firmware_controller_port_8004_shadow = 0;
    firmware_controller_port_8004 = 0;
    firmware_controller_port_8006_shadow = 0;
    firmware_controller_port_8006 = 0;
    firmware_controller_port_8005_shadow = 0x27;
    firmware_controller_port_8005 = 0x27;
    firmware_controller_port_8019 = 0x60;
    firmware_controller_port_8019_shadow = 0;
    firmware_controller_port_8019 = 0;

    firmware_controller_port_8018_shadow &= 0x1f;
    firmware_controller_port_8018 = firmware_controller_port_8018_shadow;
    firmware_controller_port_8018_shadow |= 0x86;
    firmware_controller_port_8018 = firmware_controller_port_8018_shadow;
    firmware_controller_port_8003_shadow |= 0x20;
    firmware_controller_port_8003 = firmware_controller_port_8003_shadow;
    firmware_controller_port_802d = 0x29;
    firmware_external_interrupt0_pending = 0;
    firmware_external_interrupt0_enabled = 1;
    firmware_controller_countdown = 0x7d;
}


/* arm the format-4 DMA buffers on the next controller event, without waiting for a Q address */
static void arm_capture(void) reentrant
{
    u32 buffer_offset = (u32)firmware_raw_producer_cursor * 2352;

    firmware_controller_port_8003_shadow &= 0xdf;
    firmware_controller_port_8003 = firmware_controller_port_8003_shadow;
    firmware_controller_port_802d = 0x20;
    firmware_raw_read_terminal = 0;
    firmware_raw_pipeline_state = 6;
    firmware_raw_capture_warmup = 4;
    firmware_controller_port_801c = (u8)firmware_raw_producer_cursor;
    firmware_controller_port_801d = (u8)(firmware_raw_producer_cursor >> 8);
    firmware_controller_port_8004_shadow = 0x0c;
    firmware_controller_port_8004 = 0x0c;
    firmware_controller_port_8007 = (u8)buffer_offset;
    firmware_controller_port_8008 = (u8)(buffer_offset >> 8);
    firmware_controller_port_8009 = (u8)(buffer_offset >> 16);
    firmware_controller_port_8019_shadow = 0x81;
    firmware_controller_port_8019 = 0x81;
    firmware_controller_port_802d = 0x20;
    firmware_controller_port_8003_shadow |= 0x20;
    firmware_controller_port_8003 = firmware_controller_port_8003_shadow;
}


void process_c4_read_event(void) reentrant
{
    u16 slot;
    u16 q_slot;

    if((firmware_raw_read_options & SPIRALDRIVE_C4_STREAM_MARKER) == 0 || firmware_raw_read_state != 3)
    {
        call_stock_read_event();
        return;
    }

    firmware_controller_countdown = 0x7d;
    if(firmware_main_cursor_initialized != 0)
    {
        firmware_controller_port_8005_shadow = 0;
        firmware_controller_port_8005 = 0;
        firmware_main_cursor_initialized = 0;
        return;
    }
    if(firmware_raw_pipeline_state == 5)
    {
        arm_capture();
        return;
    }
    if(firmware_raw_pipeline_state == 6)
    {
        firmware_controller_port_8005_shadow = 0x87;
        firmware_controller_port_8005 = 0x87;
        firmware_raw_pipeline_state = 7;
        return;
    }
    if(firmware_raw_pipeline_state != 7)
        return;
    if(firmware_raw_capture_warmup != 0)
    {
        --firmware_raw_capture_warmup;
        return;
    }

    /* the controller reports the next subchannel slot; 0xffff means no completed slot */
    slot = (u16)firmware_controller_port_801d << 8;
    slot |= firmware_controller_port_801c;
    if(slot == 0xffff)
        firmware_raw_controller_slot = firmware_raw_ring_size;
    else
        firmware_raw_controller_slot = (u16)(slot - 1) & firmware_raw_ring_mask;
    q_slot = (u16)firmware_controller_port_8020 << 8;
    q_slot |= firmware_controller_port_801f;
    if(q_slot != 0xffff)
        q_slot = (u16)(q_slot - 1) & firmware_raw_ring_mask;
    firmware_raw_auxiliary_buffer_cursor = firmware_controller_port_8019;
    firmware_raw_auxiliary_buffer_cursor |= (u32)firmware_controller_port_801a << 8;
    firmware_raw_auxiliary_buffer_cursor |= (u32)firmware_controller_port_801b << 16;
    firmware_raw_main_buffer_cursor = firmware_controller_port_800b;
    firmware_raw_main_buffer_cursor |= (u32)firmware_controller_port_800c << 8;
    firmware_raw_main_buffer_cursor |= (u32)firmware_controller_port_800d << 16;

    /* retain main/C2 even when the descriptor must select the zero subcode slot */
    call_alternate_step_build_descriptor(0, 0);
    /* queue the independently observed Q slot without decoding or aligning its contents */
    firmware_raw_descriptor_scratch.c4_subq_slot = q_slot;
    call_alternate_step_enqueue_descriptor();
    ++firmware_raw_pipeline_target;
    if(firmware_raw_queue_count >= firmware_raw_ring_high_water)
    {
        call_alternate_helper_initialize_cursors();
        firmware_raw_queue_control = 4;
    }
}
