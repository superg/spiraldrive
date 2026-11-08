#ifndef SPIRALDRIVE_ALTERNATE_RAW_COROUTINE_H
#define SPIRALDRIVE_ALTERNATE_RAW_COROUTINE_H

#include "firmware_memory_map.h"



void spiraldrive_alternate_raw_read_coroutine2(void) reentrant;

/* bridges to unchanged stock 1.04 callees */
void call_alternate_raw_read_yield2(void);

#endif
