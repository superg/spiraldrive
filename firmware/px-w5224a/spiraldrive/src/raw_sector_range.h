#ifndef SPIRALDRIVE_RAW_SECTOR_RANGE_H
#define SPIRALDRIVE_RAW_SECTOR_RANGE_H

#include "firmware_memory_map.h"



u8 spiraldrive_execute_raw_sector_read_range(u32 frame_count, u16 timeout_ms) reentrant;

/* adapters to unchanged stock 1.04 routines using the stock ABI */
void call_stock_initialize_raw_read_speed(void);
void call_raw_range_update_progress(void);
void call_raw_range_reset_queue(void);
void call_raw_range_initialize_coroutine2(u32 entry);
void call_raw_range_initialize_coroutine1(u32 entry);
u8 call_raw_range_poll_host_transfer(void);
u8 call_prepare_raw_batch(void);
u16 call_count_raw_batch(void);
void call_configure_ata_data_transfer_status(void);
void call_stock_no_sense(void);
void call_stock_yield0(void);

#endif
