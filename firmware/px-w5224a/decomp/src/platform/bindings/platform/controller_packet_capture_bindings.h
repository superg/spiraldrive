#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveControllerPacketIndexedState(u8 state_code);
extern void ObserveControllerPacketFlagConsumers(void);
extern void ObserveControllerPacketMeasurementConsumer(void);
#define PACKET_INDEXED_STATE(state) ObserveControllerPacketIndexedState(state)
#define PACKET_FLAG_CONSUMERS() ObserveControllerPacketFlagConsumers()
#define PACKET_MEASUREMENT_CONSUMER() ObserveControllerPacketMeasurementConsumer()
#else
#include "media/decoder.h"
extern u8 IsIndexedDriveStateActive(u8 state_code);
extern void PLEXTOR_NEAR_FUNCTION CaptureControllerPacketsFromFlagWord(void);
#define PACKET_INDEXED_STATE(state) IsIndexedDriveStateActive(state)
#define PACKET_FLAG_CONSUMERS() CaptureControllerPacketsFromFlagWord()
#define PACKET_MEASUREMENT_CONSUMER() CaptureControllerD8D9WindowIntoMeasurementSeries()
#endif
#endif
