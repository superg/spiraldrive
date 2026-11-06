#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawQPopulationReadConfiguration(fw_addr_t offset, u8 *destination, u16 count);
extern void ObserveRawQPopulationResetRandom(u16 seed);
extern u16 ObserveRawQPopulationNextRandom(void);
extern u32 ObserveRawQPopulationMultiply(u32 left, u32 right);
extern u8 ObserveRawQPopulationSelectTrack(void);
extern u8 ObserveRawQPopulationBinaryToBcd(u8 value);
extern void ObserveRawQPopulationFrameToMsf(s32 frame_position, fw_addr_t destination);
extern void ObserveRawQPopulationPackTuple(u16 address);
extern void ObserveRawQPopulationUnpackTuple(u16 address);
#define READ_CONFIGURATION(offset, destination, count) ObserveRawQPopulationReadConfiguration((offset), (destination), (count))
#define RESET_RANDOM(seed) ObserveRawQPopulationResetRandom(seed)
#define NEXT_RANDOM() ObserveRawQPopulationNextRandom()
#define MULTIPLY_U32(left, right) ObserveRawQPopulationMultiply((left), (right))
#define SELECT_TRACK() ObserveRawQPopulationSelectTrack()
#define BINARY_TO_BCD(value) ObserveRawQPopulationBinaryToBcd(value)
#define FRAME_TO_MSF(frame, destination) ObserveRawQPopulationFrameToMsf((frame), (destination))
#define PACK_TUPLE(address) ObserveRawQPopulationPackTuple(address)
#define UNPACK_TUPLE(address) ObserveRawQPopulationUnpackTuple(address)
#else
#define READ_CONFIGURATION(offset, destination, count) ReadDriveConfigurationBytes((offset), (fw_addr_t)(destination), (count))
#define RESET_RANDOM(seed) ResetPseudoRandomGenerator(seed)
#define NEXT_RANDOM() NextPseudoRandom15Bit()
#define MULTIPLY_U32(left, right) MultiplyU32Low((left), (right))
#define SELECT_TRACK() SelectRawReadQTrackNumberForCurrentOperation()
#define BINARY_TO_BCD(value) BinaryByteToPackedBcd(value)
#define FRAME_TO_MSF(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define PACK_TUPLE(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define UNPACK_TUPLE(address) ConvertThreePackedBcdBytesToBinary(address)
#endif
#endif
