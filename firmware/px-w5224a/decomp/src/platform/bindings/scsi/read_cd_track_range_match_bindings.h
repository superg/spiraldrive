#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadCdTrackRangeBit(u8 bit_address);
extern void ObserveWriteReadCdTrackRangeBit(u8 bit_address, u8 value);
#define READ_TRACK_RANGE_BIT(address) ObserveReadCdTrackRangeBit((address))
#define WRITE_TRACK_RANGE_BIT(address, value) ObserveWriteReadCdTrackRangeBit((address), (value))
#else
u8 bdata ReadCdTrackRangeBitByte22 _at_ 0x22;
sbit ReadCdTrackRangeBit0010 = ReadCdTrackRangeBitByte22 ^ 0;
sbit ReadCdTrackRangeBit0011 = ReadCdTrackRangeBitByte22 ^ 1;
#define READ_TRACK_RANGE_BIT(address) ((u8)ReadCdTrackRangeBit0011)
#define WRITE_TRACK_RANGE_BIT(address, value)                                                                                                                                                          \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if((address) == 0x10u)                                                                                                                                                                         \
            ReadCdTrackRangeBit0010 = (value);                                                                                                                                                         \
        else                                                                                                                                                                                           \
            ReadCdTrackRangeBit0011 = (value);                                                                                                                                                         \
    } while(0)
#endif
#endif
