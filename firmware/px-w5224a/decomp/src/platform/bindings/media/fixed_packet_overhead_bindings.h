#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveFixedPacketStateValid(void);
extern u8 ObserveFixedPacketLastTrackNumber(void);
extern s32 ObserveFixedPacketMsfConversion(fw_addr_t tuple_address);
#define FIXED_PACKET_STATE_VALID() ObserveFixedPacketStateValid()
#define FIXED_PACKET_LAST_TRACK() ObserveFixedPacketLastTrackNumber()
#define FIXED_PACKET_CONVERT_MSF(address) ObserveFixedPacketMsfConversion(address)
#define FIXED_PACKET_REMAINDER(dividend, divisor) ((dividend) % (divisor))
#else
extern u32 DivideU32RemainderNative(u32 dividend, u32 divisor);
u8 bdata FixedPacketStateBitByte _at_ 0x21;
sbit FixedPacketStateValid = FixedPacketStateBitByte ^ 4;
#define FIXED_PACKET_STATE_VALID() FixedPacketStateValid
#define FIXED_PACKET_LAST_TRACK() GetEffectiveLastTrackNumber()
#define FIXED_PACKET_CONVERT_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define FIXED_PACKET_REMAINDER(dividend, divisor) DivideU32RemainderNative((dividend), (divisor))
#endif
#endif
