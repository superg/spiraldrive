#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDecodeControllerQPacketBcdTuplesForAdr0(void);
extern void ObserveUpdateControllerWord035FFromAdr0Progress(void);
extern void ObserveUpdateDecodedFramePositionFromAdr0(u8 radial_table_index, fw_addr_t tuple_address);
extern void ObserveDecodeControllerQPacketBcdTuplesForAdr1(void);
extern void ObserveUpdateControllerWord035FFromAdr1Progress(void);
extern void ObserveUpdateDecodedFramePositionFromAdr1(u8 radial_table_index, fw_addr_t tuple_address);

#define DispatchDecodeControllerQPacketBcdTuples() ObserveDecodeControllerQPacketBcdTuplesForAdr0()
#define DispatchUpdateControllerWord035FFromProgressBucket() ObserveUpdateControllerWord035FFromAdr0Progress()
#define DispatchUpdateDecodedFramePositionAndRadialEstimate(index, address) ObserveUpdateDecodedFramePositionFromAdr0((index), (address))
#define DispatchDecodeControllerQPacketBcdTuplesForAdr1() ObserveDecodeControllerQPacketBcdTuplesForAdr1()
#define DispatchUpdateControllerWord035FFromAdr1Progress() ObserveUpdateControllerWord035FFromAdr1Progress()
#define DispatchUpdateDecodedFramePositionFromAdr1(index, address) ObserveUpdateDecodedFramePositionFromAdr1((index), (address))
#else
#define DispatchDecodeControllerQPacketBcdTuples() DecodeControllerQPacketBcdTuples()
#define DispatchUpdateControllerWord035FFromProgressBucket() UpdateControllerWord035FFromProgressBucket()
#define DispatchUpdateDecodedFramePositionAndRadialEstimate(index, address) UpdateDecodedFramePositionAndRadialEstimate((index), (address))
#define DispatchDecodeControllerQPacketBcdTuplesForAdr1() DecodeControllerQPacketBcdTuples()
#define DispatchUpdateControllerWord035FFromAdr1Progress() UpdateControllerWord035FFromProgressBucket()
#define DispatchUpdateDecodedFramePositionFromAdr1(index, address) UpdateDecodedFramePositionAndRadialEstimate((index), (address))
#endif
#endif
