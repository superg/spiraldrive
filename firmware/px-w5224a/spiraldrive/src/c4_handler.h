#ifndef SPIRALDRIVE_C4_HANDLER_H
#define SPIRALDRIVE_C4_HANDLER_H

#include "firmware_memory_map.h"



typedef enum
{
    SPIRALDRIVE_OPERATION_SEEK = 1,
    /* after draining a full stopped buffer, report CHECK CONDITION once; the next READ starts a new stream */
    SPIRALDRIVE_OPERATION_READ = 2
} SpiralDriveOperation;

typedef struct
{
    u8 operation_code;
    u8 operation;
    /* READ stall timeout; ignored by SEEK */
    u16 timeout_ms;
    /* raw controller displacement in -32767..32767 for SEEK, positive frame count for READ */
    s32 value;
    u32 reserved;
} SpiralDriveCDB;

#define SPIRALDRIVE_C4_STREAM_MARKER 0x04

#define SPIRALDRIVE_C4_DATA_SIZE 2352
#define SPIRALDRIVE_C4_C2_SIZE 294
#define SPIRALDRIVE_C4_SUB_SIZE 96
#define SPIRALDRIVE_C4_SUBQ_SIZE 12
#define SPIRALDRIVE_C4_FRAME_SIZE (SPIRALDRIVE_C4_DATA_SIZE + SPIRALDRIVE_C4_C2_SIZE + SPIRALDRIVE_C4_SUB_SIZE + SPIRALDRIVE_C4_SUBQ_SIZE)

/* READ repeats this layout for each captured frame, without a header or padding */
typedef struct
{
    u8 main_data[SPIRALDRIVE_C4_DATA_SIZE];
    u8 c2[SPIRALDRIVE_C4_C2_SIZE];
    u8 sub[SPIRALDRIVE_C4_SUB_SIZE];
    /* unchanged 12-byte Q packet including CRC, excluding the four trailing slot bytes */
    /* the last completed Q slot observed at capture time, not a guaranteed DATA-sector match */
    /* unavailable SUB/SUBQ are zero-filled independently; invalid or repeated records are retained */
    u8 subq[SPIRALDRIVE_C4_SUBQ_SIZE];
} SpiralDriveReadFrame;

/* successful SEEK returns one DATA IN byte with GOOD; failures return CHECK CONDITION */
typedef enum
{
    SPIRALDRIVE_SEEK_NONE = 0,
    SPIRALDRIVE_SEEK_INNER_LIMIT = 1
} SpiralDriveSeekResult;

void handle_scsi_opcode_c4(void) reentrant;
void call_stock_no_sense(void);
/* stock ABI takes byte_count in DR4 and source_address in DR0; 5 means transfer completed */
u8 call_stock_response_data_in(u32 byte_count, u32 source_address);
void call_stock_yield0(void);
/* magnitude is 1..32767; inward selects inward (1) or outward (0); the result is valid only with no sense */
u8 spiraldrive_move_pickup(u16 magnitude, u8 inward) reentrant;

#endif
