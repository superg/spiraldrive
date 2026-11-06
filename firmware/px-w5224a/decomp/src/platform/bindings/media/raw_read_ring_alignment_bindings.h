#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveConvertControllerPositionTuple(fw_addr_t tuple_address);
extern u8 ObserveReadControllerPositionBit000F(void);
extern u8 ObserveReadControllerPositionBit0013(void);
extern u8 ObserveReadControllerPositionBit001C(void);
extern u8 ObserveReadControllerPositionBit001D(void);
#define CONVERT_CONTROLLER_POSITION(address) ObserveConvertControllerPositionTuple(address)
#define READ_CONTROLLER_POSITION_BIT000F() ObserveReadControllerPositionBit000F()
#define READ_CONTROLLER_POSITION_BIT0013() ObserveReadControllerPositionBit0013()
#define READ_CONTROLLER_POSITION_BIT001C() ObserveReadControllerPositionBit001C()
#define READ_CONTROLLER_POSITION_BIT001D() ObserveReadControllerPositionBit001D()
extern void ObserveDecodeControllerPositionForRingAlignment(u8 check_status);
extern void ObserveInitializeControllerBufferCursorsForRingAlignment(void);
#define DECODE_CONTROLLER_POSITION(check_status) ObserveDecodeControllerPositionForRingAlignment(check_status)
#define INITIALIZE_CONTROLLER_CURSORS() ObserveInitializeControllerBufferCursorsForRingAlignment()
#else
extern u8 ReadBit000FForControllerPositionDecode(void);
extern u8 ReadBit0013ForControllerPositionDecode(void);
extern u8 ReadBit001CForControllerPositionDecode(void);
extern u8 ReadBit001DForControllerPositionDecode(void);
#define CONVERT_CONTROLLER_POSITION(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define READ_CONTROLLER_POSITION_BIT000F() ReadBit000FForControllerPositionDecode()
#define READ_CONTROLLER_POSITION_BIT0013() ReadBit0013ForControllerPositionDecode()
#define READ_CONTROLLER_POSITION_BIT001C() ReadBit001CForControllerPositionDecode()
#define READ_CONTROLLER_POSITION_BIT001D() ReadBit001DForControllerPositionDecode()
extern void InitializeControllerBufferCursorRegistersIfNeeded(void);
#define DECODE_CONTROLLER_POSITION(check_status) DecodeControllerPositionFields(check_status)
#define INITIALIZE_CONTROLLER_CURSORS() InitializeControllerBufferCursorRegistersIfNeeded()
#endif
#endif
