#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveUpdateRawReadQPacketControlBit(void);
extern u8 ObserveIsIsrcAvailableForCurrentTrack(void);
extern u8 ObserveIsMediaCatalogNumberAvailable(void);
extern u8 ObserveBuildIsrcQPacket(void);
extern u8 ObserveBuildMediaCatalogNumberQPacket(void);
extern void ObserveRestoreCurrentQPacketAfterIsrc(u8 decrement_relative_msf);
extern void ObserveRestoreCurrentQPacketAfterMediaCatalog(u8 decrement_relative_msf);
extern void ObserveAdvanceRawReadQPacketMsfTuples(u8 reverse_relative);
#define UPDATE_Q_CONTROL() ObserveUpdateRawReadQPacketControlBit()
#define IS_ISRC_AVAILABLE() ObserveIsIsrcAvailableForCurrentTrack()
#define IS_MEDIA_CATALOG_AVAILABLE() ObserveIsMediaCatalogNumberAvailable()
#define BUILD_ISRC_PACKET() ObserveBuildIsrcQPacket()
#define BUILD_MEDIA_CATALOG_PACKET() ObserveBuildMediaCatalogNumberQPacket()
#define RESTORE_AFTER_ISRC(decrement) ObserveRestoreCurrentQPacketAfterIsrc(decrement)
#define RESTORE_AFTER_MEDIA_CATALOG(decrement) ObserveRestoreCurrentQPacketAfterMediaCatalog(decrement)
#define ADVANCE_Q_MSF_TUPLES(reverse) ObserveAdvanceRawReadQPacketMsfTuples(reverse)
#else
extern void UpdateRawReadQPacketControlBit(void);
extern u8 PLEXTOR_NEAR_FUNCTION IsIsrcAvailableForCurrentTrack(void);
extern u8 PLEXTOR_NEAR_FUNCTION IsMediaCatalogNumberAvailable(void);
extern u8 BuildIsrcQPacket(void);
extern u8 BuildMediaCatalogNumberQPacket(void);
extern void AdvanceRawReadQPacketMsfTuples(u8 reverse_relative);
#define UPDATE_Q_CONTROL() UpdateRawReadQPacketControlBit()
#define IS_ISRC_AVAILABLE() IsIsrcAvailableForCurrentTrack()
#define IS_MEDIA_CATALOG_AVAILABLE() IsMediaCatalogNumberAvailable()
#define BUILD_ISRC_PACKET() BuildIsrcQPacket()
#define BUILD_MEDIA_CATALOG_PACKET() BuildMediaCatalogNumberQPacket()
#define RESTORE_AFTER_ISRC(decrement) RestoreCurrentQPacketAfterIsrc(decrement)
#define RESTORE_AFTER_MEDIA_CATALOG(decrement) RestoreCurrentQPacketAfterMediaCatalog(decrement)
#define ADVANCE_Q_MSF_TUPLES(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#endif
#endif
