#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDiscControlConfigurationReadToMemory(fw_addr_t offset, fw_addr_t destination, u16 count);
extern void ObserveDiscControlConfigurationReadToLocal(fw_addr_t offset, u8 *destination, u16 count);
extern s16 ObserveDiscControlInitializationHelper(u8 event, s16 argument);
#define READ_CONFIGURATION_TO_MEMORY(offset, destination, count) ObserveDiscControlConfigurationReadToMemory((offset), (destination), (count))
#define READ_CONFIGURATION_TO_LOCAL(offset, destination, count) ObserveDiscControlConfigurationReadToLocal((offset), (destination), (count))
#define RESET_MEASUREMENT_CAPTURE() ((void)ObserveDiscControlInitializationHelper(DISC_CONTROL_RESET_MEASUREMENT_CAPTURE, 0))
#define DIVIDE_ADJUSTED_BYTE(value) ((s8)ObserveDiscControlInitializationHelper(DISC_CONTROL_DIVIDE_ADJUSTED_BYTE, (value)))
#define TEST_INDEXED_STATE(value) ((u8)ObserveDiscControlInitializationHelper(DISC_CONTROL_TEST_INDEXED_STATE, (value)))
#define DECODE_SIGNED_OFFSET(value) ((s8)ObserveDiscControlInitializationHelper(DISC_CONTROL_DECODE_SIGNED_OFFSET, (value)))
#else
extern s8 PLEXTOR_NEAR_FUNCTION DivideS8ByTwoNative(s8 value);
#define READ_CONFIGURATION_TO_MEMORY(offset, destination, count) ReadDriveConfigurationBytes((offset), (destination), (count))
#define READ_CONFIGURATION_TO_LOCAL(offset, destination, count) ReadDriveConfigurationBytes((offset), (fw_addr_t)(destination), (count))
#define RESET_MEASUREMENT_CAPTURE() ResetControllerCommand0EMeasurementCaptureState()
#define DIVIDE_ADJUSTED_BYTE(value) DivideS8ByTwoNative(value)
#define TEST_INDEXED_STATE(value) IsIndexedDriveStateActive(value)
#define DECODE_SIGNED_OFFSET(value) DecodeControllerEncodedSignedOffset(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(address, (u8)(value >> 8));
FirmwareWriteNear8((u16)(address + 1u), (u8)value);
#else
*((volatile u16 near *)address) = value;
#endif
#endif
