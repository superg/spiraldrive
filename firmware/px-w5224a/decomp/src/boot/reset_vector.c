#include "vector_entries.h"

extern void FirmwareMainLoop(void);

/* RAM:f80050 */
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/boot/reset_vector_bindings.h"
#undef PLEXTOR_BINDING_SECTION
