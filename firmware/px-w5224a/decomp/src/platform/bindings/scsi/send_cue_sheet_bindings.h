#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveSendCueValidateCdb(fw_addr_t mask_descriptor);
extern void ObserveSendCueUnsupported(void);
extern void ObserveSendCueSequenceError(void);
extern u8 ObserveSendCueReceive(fw_addr_t destination, u32 byte_count);
extern void ObserveSendCueNoSense(void);
#define SEND_CUE_VALIDATE(mask) ObserveSendCueValidateCdb(mask)
#define SEND_CUE_UNSUPPORTED() ObserveSendCueUnsupported()
#define SEND_CUE_SEQUENCE_ERROR() ObserveSendCueSequenceError()
#define SEND_CUE_RECEIVE(destination, count) ObserveSendCueReceive((destination), (count))
#define SEND_CUE_NO_SENSE() ObserveSendCueNoSense()
#else
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#define SEND_CUE_VALIDATE(mask) ValidateScsiCdbFieldMasks(mask)
#define SEND_CUE_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define SEND_CUE_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define SEND_CUE_RECEIVE(destination, count) ExecuteDataOutAndCopyFromControllerBuffer((destination), (count))
#define SEND_CUE_NO_SENSE() SetNoSenseData()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u32 value = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F) << 24;
value |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10) << 16;
value |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0B11) << 8;
value |= FirmwareReadNear8(FW_NEAR_UNKNOWN_0B12);
return value;
#else
return *((volatile u32 near *)0x0b0f);
#endif
#endif
