#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 FirmwareReadBit0021(void);
extern s32 ObserveTrackPacketConvertMsf(fw_addr_t address);
extern u8 ObserveTrackPacketLastTrack(void);
extern u32 ObserveTrackPacketDivide(u32 dividend, u32 divisor);
extern u32 ObserveTrackPacketMultiply(u32 left, u32 right);
#define CONVERT_TRACK_PACKET_MSF(address) ObserveTrackPacketConvertMsf(address)
#define GET_TRACK_PACKET_LAST_TRACK() ObserveTrackPacketLastTrack()
#define READ_TRACK_PACKET_EXCEPTION_BIT() FirmwareReadBit0021()
#define DIVIDE_TRACK_PACKET(value, divisor) ObserveTrackPacketDivide((value), (divisor))
#define MULTIPLY_TRACK_PACKET(left, right) ObserveTrackPacketMultiply((left), (right))
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0021ForAlternateRangeValidation(void);
extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
#define CONVERT_TRACK_PACKET_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define GET_TRACK_PACKET_LAST_TRACK() GetEffectiveLastTrackNumber()
#define READ_TRACK_PACKET_EXCEPTION_BIT() ReadBit0021ForAlternateRangeValidation()
#define DIVIDE_TRACK_PACKET(value, divisor) DivideU32QuotientNative((value), (divisor))
#define MULTIPLY_TRACK_PACKET(left, right) MultiplyU32Low((left), (right))
#endif
#endif
