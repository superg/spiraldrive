#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawReadTransferLayoutDecode(u16 record_index);
extern void ObserveRawReadTransferLayoutUpdateQType(void);
extern void ObserveRawReadTransferLayoutTuple(s32 frame_position, u8 *tuple);
#define LAYOUT_DECODE(index) ObserveRawReadTransferLayoutDecode(index)
#define LAYOUT_UPDATE_Q_TYPE() ObserveRawReadTransferLayoutUpdateQType()
#define LAYOUT_BUILD_TUPLE(frame, tuple) ObserveRawReadTransferLayoutTuple((frame), (tuple))
#else
#define LAYOUT_DECODE(index) DecodeRawReadQRecordTypeCode(index)
#define LAYOUT_UPDATE_Q_TYPE() UpdateRawReadQTypeStateFlag()
#define LAYOUT_BUILD_TUPLE(frame, tuple)                                                                                                                                                               \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ConvertFramePositionToBinaryMsfTuple((frame), (fw_addr_t)(tuple));                                                                                                                             \
        ConvertThreeBinaryBytesToPackedBcd((u16)(tuple));                                                                                                                                              \
    } while(0)
#endif
#endif
