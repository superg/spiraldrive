#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveVendorF3CyclePortWrite(fw_addr_t address, u8 value);
extern void ObserveVendorF3CycleProfile(u8 set_range_high_bit, u16 range_value);
extern void ObserveVendorF3CycleDrive(u8 drive_code, u8 enabled);
extern u8 ObserveVendorF3CycleCountdownRead(void);
extern u8 ObserveVendorF3CycleInputBit2(void);
#define WRITE_VENDOR_F3_CYCLE_PORT(address, value) ObserveVendorF3CyclePortWrite((address), (value))
#define APPLY_VENDOR_F3_CYCLE_PROFILE(enabled, value) ObserveVendorF3CycleProfile((enabled), (value))
#define CONFIGURE_VENDOR_F3_CYCLE_DRIVE(code, enabled) ObserveVendorF3CycleDrive((code), (enabled))
#define READ_VENDOR_F3_CYCLE_COUNTDOWN() ObserveVendorF3CycleCountdownRead()
#define READ_VENDOR_F3_CYCLE_INPUT_BIT2() ObserveVendorF3CycleInputBit2()
#else
#define WRITE_VENDOR_F3_CYCLE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define APPLY_VENDOR_F3_CYCLE_PROFILE(enabled, value) ConfigureControllerStateProfileFromFirmwareDefaults((enabled), (value))
#define CONFIGURE_VENDOR_F3_CYCLE_DRIVE(code, enabled) ConfigureControllerDrivePorts8496Through849D((code), (enabled))
#define READ_VENDOR_F3_CYCLE_COUNTDOWN() FirmwareReadNear8(FW_NEAR_UNKNOWN_1216)
#define READ_VENDOR_F3_CYCLE_INPUT_BIT2() ((u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 0x04u) != 0u))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define SetVendorF3StateFlag() FirmwareWriteBit0039(1u)
#else
u8 bdata VendorF3StateBitByte _at_ 0x27;
sbit VendorF3StateFlag = VendorF3StateBitByte ^ 1;
#define SetVendorF3StateFlag() (VendorF3StateFlag = 1)
#endif
#endif
