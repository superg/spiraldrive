#ifndef RAW_CAPTURE_H
#define RAW_CAPTURE_H

#include "firmware_memory_map.h"



void start_raw_capture(void) reentrant;
void call_clear_controller_buffer(u32 buffer_address, u32 byte_count);
void call_alternate_helper_initialize_cursors(void);
void call_alternate_step_build_descriptor(u16 status, u8 mode);
void call_alternate_step_enqueue_descriptor(void);
void process_c4_read_event(void) reentrant;
void call_stock_read_event(void);

#endif
