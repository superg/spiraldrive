#ifndef SPIRALDRIVE_C4_MEDIA_COROUTINE_H
#define SPIRALDRIVE_C4_MEDIA_COROUTINE_H

#include "firmware_memory_map.h"



/* entry stub for the coroutine-1 stack initialized by the stock helper */
void spiraldrive_c4_media_coroutine1_entry(void);

void spiraldrive_run_c4_media_coroutine1(void) reentrant;
void initialize_c4_speed(void) reentrant;
void call_c4_finalize_controller_capture(void);

#endif
