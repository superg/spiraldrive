#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCalibrationConfigurationRead(fw_addr_t offset, fw_addr_t destination, u16 count);
extern void ObserveCalibrationConfigurationWriteE8E9(u8 value);
extern u8 ObserveCalibrationConfigurationIndexedState(u8 state_code);
extern void ObserveCalibrationConfigurationWriteBit44(u8 value);
#define CALIBRATION_CONFIGURATION_READ(offset, destination, count) ObserveCalibrationConfigurationRead((offset), (destination), (count))
#define CALIBRATION_CONFIGURATION_WRITE_E8E9(value) ObserveCalibrationConfigurationWriteE8E9(value)
#define CALIBRATION_CONFIGURATION_INDEXED_STATE(value) ObserveCalibrationConfigurationIndexedState(value)
#define CALIBRATION_CONFIGURATION_WRITE_BIT44(value) ObserveCalibrationConfigurationWriteBit44(value)
#else
#include "platform/controller_parameters.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
u8 bdata CalibrationConfigurationBitByte28 _at_ 0x28;
sbit CalibrationConfigurationBit0044 = CalibrationConfigurationBitByte28 ^ 4;
#define CALIBRATION_CONFIGURATION_READ(offset, destination, count) ReadDriveConfigurationBytes((offset), (destination), (count))
#define CALIBRATION_CONFIGURATION_WRITE_E8E9(value) WriteControllerParametersE8E9ForValue(value)
#define CALIBRATION_CONFIGURATION_INDEXED_STATE(value) IsIndexedDriveStateActive(value)
#define CALIBRATION_CONFIGURATION_WRITE_BIT44(value) (CalibrationConfigurationBit0044 = (value))
#endif
#endif
