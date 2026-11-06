#ifndef PLEXTOR_FIRMWARE_MAP_H
#define PLEXTOR_FIRMWARE_MAP_H

#include "firmware_map_controller.h"
#include "firmware_map_huge.h"
#include "firmware_map_near.h"
#include "firmware_map_rom.h"
#include "firmware_map_sfr_bits.h"
#include "mcs251.h"

#define FW_24BIT_ADDRESS_MASK ((fw_addr_t)0xfffffful)
#define FW_HUGE_BUFFER_BANK_BASE ((fw_addr_t)0x100000ul)
#define FW_ADDRESS_BANK_STRIDE FW_HUGE_BUFFER_BANK_BASE

/* Known buffer-region entry points; substructure remains unresolved. */
#define FW_HUGE_RAW_PAYLOAD_SIGNATURE ((fw_addr_t)0x100018ul)
#define FW_HUGE_PREMIUM_PAYLOAD_SOURCE ((fw_addr_t)0x193000ul)
#define FW_HUGE_PREMIUM_PAYLOAD_SECTOR_SOURCE ((fw_addr_t)0x193010ul)

/*
 * Plextor PX-W5224A firmware 1.04 physical storage map.
 *
 * Keep numeric physical addresses in this layer.  Names encode the Ghidra
 * address space used as evidence: FW_NEAR/FW_HUGE are RAM, FW_CONTROLLER is
 * the LC898095 external-data window, FW_SFR is SFR space, and FW_BIT is BITS
 * space.  A name containing UNKNOWN deliberately makes no semantic claim.
 *
 * Width and byte-order notes describe the access observed in the original
 * instructions, not a guessed C object layout.
 */

/* RAM:000b0a, u8, current twelve-byte ATA packet opcode. Confirmed. */
#define FW_NEAR_HOST_COMMAND_OPCODE 0x0b0aul

/* RAM:000b0a..000b15, twelve-byte ATA packet buffer. Confirmed. */
#define FW_NEAR_HOST_COMMAND_PACKET 0x0b0aul

/* RAM:000c32, u8, pending host packet state. Confirmed. */
#define FW_NEAR_HOST_PACKET_STATE 0x0c32ul

/* RAM:000c78, u8, zero selects ATA task-file dispatch. Strong inference. */
#define FW_NEAR_HOST_COMMAND_PROTOCOL 0x0c78ul

/* RAM:000c83, u8 bit field, host-command completion state. Confirmed. */
#define FW_NEAR_HOST_COMPLETION_FLAGS 0x0c83ul

/* RAM:000c84, RAM:000c87 and RAM:000c88, u8 bit fields; roles unresolved. */
#define FW_NEAR_UNKNOWN_0C84 0x0c84ul
#define FW_NEAR_UNKNOWN_0C87 0x0c87ul
#define FW_NEAR_UNKNOWN_0C88 0x0c88ul

/* RAM:000c89, u8 bit field, host-command status. Confirmed. */
#define FW_NEAR_HOST_COMMAND_STATUS 0x0c89ul

/* RAM:000c8a, u8 bit field, host-command event state. Strong inference. */
#define FW_NEAR_HOST_COMMAND_EVENTS 0x0c8aul

/* RAM:000c8b, u8 bit field, media-operation event state. Strong inference. */
#define FW_NEAR_MEDIA_OPERATION_EVENTS 0x0c8bul

/* RAM:000c9e, u8, active format/media state selector. Confirmed. */
#define FW_NEAR_FORMAT_STATE 0x0c9eul

/* RAM:000ce1/000ce5, native big-endian u32 scan snapshots. Confirmed. */
#define FW_NEAR_SCAN_POSITION_FIRST 0x0ce1ul
#define FW_NEAR_SCAN_POSITION_SECOND 0x0ce5ul

/* RAM:001153, native big-endian u32 current frame position. Confirmed. */
#define FW_NEAR_CURRENT_FRAME_POSITION 0x1153ul

/* RAM:00122b/00122c, u8 host-command countdowns. Confirmed. */
#define FW_NEAR_HOST_FAILURE_COUNTDOWN 0x122bul
#define FW_NEAR_HOST_ACQUISITION_COUNTDOWN 0x122cul

/* RAM:001588, u8 bit field, active operation state. Strong inference. */
#define FW_NEAR_ACTIVE_OPERATION_FLAGS 0x1588ul

/* RAM:00e750, u8 bit field; physical role is not yet established. */
#define FW_NEAR_UNKNOWN_E750 0xe750ul

/* RAM:00faef, u8 bit field; physical role is not yet established. */
#define FW_NEAR_UNKNOWN_FAEF 0xfaeful

/* RAM:00fc48/00fc49, u8 format-operation state. Strong inference. */
#define FW_NEAR_FORMAT_OPERATION_ACTIVE 0xfc48ul
#define FW_NEAR_FORMAT_OPERATION_RESULT 0xfc49ul

/* RAM:2b9704, native big-endian u32 controller scan snapshot. Confirmed. */
#define FW_HUGE_CONTROLLER_SCAN_SNAPSHOT ((fw_addr_t)0x2b9704ul)

/* RAM:2ba52e, native big-endian s32 mapped record position. Confirmed. */
#define FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION ((fw_addr_t)0x2ba52eul)

/* RAM:2ba532, native big-endian s32 active-record upper boundary. Confirmed. */
#define FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY ((fw_addr_t)0x2ba532ul)

/* RAM:2ba5af..2ba5b1, u8 host-operation latches. Strong inference. */
#define FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH ((fw_addr_t)0x2ba5aful)
#define FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH ((fw_addr_t)0x2ba5b0ul)
#define FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH ((fw_addr_t)0x2ba5b1ul)

/* RAM:018000, u8 controller command-data window. Confirmed. */
#define FW_CONTROLLER_COMMAND_WINDOW ((fw_addr_t)0x018000ul)

/* RAM:01804a/018050/018051/018056, u8 controller command registers. */
#define FW_CONTROLLER_COMMAND_CODE ((fw_addr_t)0x01804aul)
#define FW_CONTROLLER_COMMAND_ARGUMENT0 ((fw_addr_t)0x018050ul)
#define FW_CONTROLLER_COMMAND_ARGUMENT1 ((fw_addr_t)0x018051ul)
#define FW_CONTROLLER_COMMAND_TRIGGER ((fw_addr_t)0x018056ul)

/* FW_NEAR_HOST_COMMAND_STATUS masks. */
#define HOST_COMMAND_STATUS_ACTIVE_RECORD 0x01u
#define HOST_COMMAND_STATUS_FAILURE 0x08u
#define HOST_COMMAND_STATUS_RETRYABLE_READ 0x10u
#define HOST_COMMAND_STATUS_RETRYABLE_READ_CLEAR_MASK 0xefu
#define HOST_COMMAND_STATUS_BOUNDARY_REACHED 0x80u

/* FW_NEAR_HOST_COMPLETION_FLAGS masks. */
#define HOST_COMPLETION_PREPARED 0x01u
#define HOST_COMPLETION_PREPARED_CLEAR_MASK 0xfeu
#define HOST_COMPLETION_DEFERRED 0x02u
#define HOST_COMPLETION_DEFERRED_CLEAR_MASK 0xfdu
#define HOST_COMPLETION_CLEAR_REQUESTED 0x10u

/* FW_NEAR_HOST_COMMAND_EVENTS masks. */
#define HOST_COMMAND_EVENT_PACKET_PENDING 0x02u
#define HOST_COMMAND_EVENT_ACTIVE_RECORD 0x01u
#define HOST_COMMAND_EVENT_FORMAT_TRANSITION 0x80u

#endif
