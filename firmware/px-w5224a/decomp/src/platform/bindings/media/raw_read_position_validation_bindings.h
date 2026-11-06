#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveRawReadPositionValidation(u8 event, u32 argument0, u32 argument1);
#define POSITION_EVENT(event, argument0, argument1) ObserveRawReadPositionValidation((event), (argument0), (argument1))
#define POSITION_CHECK_P1() ((u8)POSITION_EVENT(RAW_READ_POSITION_CHECK_P1, 0ul, 0ul))
#define POSITION_DECODE(value) ((u8)POSITION_EVENT(RAW_READ_POSITION_DECODE, (value), 0ul))
#define POSITION_ENQUEUE_TIMING(status, mode) ((void)POSITION_EVENT(RAW_READ_POSITION_ENQUEUE_TIMING_STATUS, (status), (mode)))
#define POSITION_INITIALIZE_CURSORS() ((void)POSITION_EVENT(RAW_READ_POSITION_INITIALIZE_CURSORS, 0ul, 0ul))
#define POSITION_LIMIT_BACKLOG() ((void)POSITION_EVENT(RAW_READ_POSITION_LIMIT_BACKLOG, 0ul, 0ul))
#define POSITION_BUILD_DESCRIPTOR(status, mode) ((void)POSITION_EVENT(RAW_READ_POSITION_BUILD_DESCRIPTOR, (status), (mode)))
#define POSITION_ENQUEUE_DESCRIPTOR() ((void)POSITION_EVENT(RAW_READ_POSITION_ENQUEUE_DESCRIPTOR, 0ul, 0ul))
#define POSITION_SET_TERMINAL() ((void)POSITION_EVENT(RAW_READ_POSITION_SET_TERMINAL, 1ul, 0ul))
#else
extern void PLEXTOR_NEAR_FUNCTION SetBit0009Atomic(void);

#define POSITION_CHECK_P1() CheckP1Bit3StableHighForCurrentMediaState()
#define POSITION_DECODE(value) DecodeControllerPositionFields(value)
#define POSITION_ENQUEUE_TIMING(status, mode) EnqueueRawSectorDescriptorWithTimingGate((status), (mode))
#define POSITION_INITIALIZE_CURSORS() InitializeControllerBufferCursorRegistersIfNeeded()
#define POSITION_LIMIT_BACKLOG() LimitRawReadSecondaryCursorBacklog()
#define POSITION_BUILD_DESCRIPTOR(status, mode) BuildRawSectorDescriptorScratch((status), (mode))
#define POSITION_ENQUEUE_DESCRIPTOR() EnqueueRawSectorDescriptor()
#define POSITION_SET_TERMINAL() SetBit0009Atomic()
#endif
#endif
