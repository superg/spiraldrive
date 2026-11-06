#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawTrackSelectionInitializeState(void);
extern void ObserveRawTrackSelectionLoadMediaCatalog(void);
extern void ObserveRawTrackSelectionLoadIsrc(void);
#define InitializeTrackSelectionState() ObserveRawTrackSelectionInitializeState()
#define LoadSelectedMediaCatalog() ObserveRawTrackSelectionLoadMediaCatalog()
#define LoadSelectedIsrc() ObserveRawTrackSelectionLoadIsrc()
#else
extern void PLEXTOR_NEAR_FUNCTION LoadMediaCatalogNumberNearAdapter(void);
#define InitializeTrackSelectionState() InitializeRawReadTrackSelectionState()
#define LoadSelectedMediaCatalog() LoadMediaCatalogNumberNearAdapter()
#define LoadSelectedIsrc() LoadIsrcCharactersForCurrentTrack()
#endif
#endif
