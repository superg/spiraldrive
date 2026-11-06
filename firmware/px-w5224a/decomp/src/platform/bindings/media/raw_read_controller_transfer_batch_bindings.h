#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveProgramRawReadControllerTransferAddressRegisters(void);
extern u32 ObserveSelectRawReadBufferAddressForTransferBatch(u32 component_length, u32 fallback_address);
#define PROGRAM_TRANSFER_ADDRESS_REGISTERS() ObserveProgramRawReadControllerTransferAddressRegisters()
#define SELECT_BUFFER_ADDRESS(length, fallback) ObserveSelectRawReadBufferAddressForTransferBatch((length), (fallback))
#else
#define PROGRAM_TRANSFER_ADDRESS_REGISTERS() ProgramRawReadControllerTransferAddressRegisters()
#define SELECT_BUFFER_ADDRESS(length, fallback) SelectRawReadBufferAddressForLength((length), (fallback))
#endif
#endif
