#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateStagedFirmwareRevisionPolicy(u8 buffer_id);
extern u8 ObserveValidateStagedMainFirmwareHeader(void);
extern u8 ObserveValidateStagedFirmwareChecksum(u32 start_offset, u32 end_offset);
extern void ObserveServicePendingHostCommandBeforeProgramming(void);
extern void ObserveInvokeRuntimeMainFlashProgrammerOperation3(void);
#define VALIDATE_REVISION(buffer_id) ObserveValidateStagedFirmwareRevisionPolicy(buffer_id)
#define VALIDATE_HEADER() ObserveValidateStagedMainFirmwareHeader()
#define VALIDATE_CHECKSUM(start_offset, end_offset) ObserveValidateStagedFirmwareChecksum((start_offset), (end_offset))
#define SERVICE_PENDING_HOST_COMMAND() ObserveServicePendingHostCommandBeforeProgramming()
#define INVOKE_RUNTIME_PROGRAMMER() ObserveInvokeRuntimeMainFlashProgrammerOperation3()
#define SET_GLOBAL_INTERRUPT(value) FirmwareWriteBit0547(value)
#define SET_CONTROLLER_INTERRUPT(value) FirmwareWriteBit0546(value)
#define SET_A9_BIT2() FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 4u))
#else
sbit MainProgrammerGlobalInterruptEnable = 0xaf;
sbit MainProgrammerControllerInterruptEnable = 0xae;
extern void SetSfrA9Bit2Atomic(void);
extern void InvokeRuntimeMainFlashProgrammerOperation3(void);
#define VALIDATE_REVISION(buffer_id) ValidateStagedFirmwareRevisionPolicy(buffer_id)
#define VALIDATE_HEADER() ValidateStagedMainFirmwareHeader()
#define VALIDATE_CHECKSUM(start_offset, end_offset) ValidateStagedFirmwareChecksum((end_offset), (start_offset))
#define SERVICE_PENDING_HOST_COMMAND() ServicePendingHostCommandIfAny()
#define INVOKE_RUNTIME_PROGRAMMER() InvokeRuntimeMainFlashProgrammerOperation3()
#define SET_GLOBAL_INTERRUPT(value) (MainProgrammerGlobalInterruptEnable = (value))
#define SET_CONTROLLER_INTERRUPT(value) (MainProgrammerControllerInterruptEnable = (value))
#define SET_A9_BIT2() SetSfrA9Bit2Atomic()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(FW_NEAR_UNKNOWN_1312, (u8)(entry >> 24));
FirmwareWriteNear8(FW_NEAR_UNKNOWN_1313, (u8)(entry >> 16));
FirmwareWriteNear8(FW_NEAR_UNKNOWN_1314, (u8)(entry >> 8));
FirmwareWriteNear8(FW_NEAR_UNKNOWN_1315, (u8)entry);
#else
*((volatile fw_addr_t near *)0x1312) = entry;
#endif
#endif
