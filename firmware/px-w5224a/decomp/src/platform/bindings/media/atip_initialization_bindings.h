#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveAtipInitializationResetAtip(void);
extern void ObserveAtipInitializationResetDriveControl(void);
extern u8 ObserveAtipInitializationIsIndexed(u8 selector);
extern void ObserveAtipInitializationProcessIndexed(void);
extern u8 ObserveAtipInitializationInitializeController(void);
extern void ObserveAtipInitializationResetPorts(void);
extern void ObserveAtipInitializationResetHardware(void);
extern void ObserveAtipInitializationResetOperatingState(void);
extern void ObserveAtipInitializationWriteController(u32 address, u8 value);
extern u8 ObserveAtipInitializationAcquirePosition(void);
extern void ObserveAtipInitializationApplyOperatingCode(void);
extern void ObserveAtipInitializationArmPacket(void);
extern u8 ObserveAtipInitializationParsePacket(void);
extern void ObserveAtipInitializationYield(void);
extern u8 ObserveAtipInitializationValidatePacket(u8 mode);
extern void ObserveAtipInitializationAcquireSupplemental(void);
#define ATIP_RESET() ObserveAtipInitializationResetAtip()
#define ATIP_RESET_DRIVE() ObserveAtipInitializationResetDriveControl()
#define ATIP_IS_INDEXED(selector) ObserveAtipInitializationIsIndexed(selector)
#define ATIP_PROCESS_INDEXED() ObserveAtipInitializationProcessIndexed()
#define ATIP_INITIALIZE_CONTROLLER() ObserveAtipInitializationInitializeController()
#define ATIP_RESET_PORTS() ObserveAtipInitializationResetPorts()
#define ATIP_RESET_HARDWARE() ObserveAtipInitializationResetHardware()
#define ATIP_RESET_OPERATING() ObserveAtipInitializationResetOperatingState()
#define ATIP_WRITE_CONTROLLER(address, value) ObserveAtipInitializationWriteController((address), (value))
#define ATIP_ACQUIRE_POSITION() ObserveAtipInitializationAcquirePosition()
#define ATIP_APPLY_OPERATING() ObserveAtipInitializationApplyOperatingCode()
#define ATIP_ARM_PACKET() ObserveAtipInitializationArmPacket()
#define ATIP_PARSE_PACKET() ObserveAtipInitializationParsePacket()
#define ATIP_YIELD() ObserveAtipInitializationYield()
#define ATIP_VALIDATE(mode) ObserveAtipInitializationValidatePacket(mode)
#define ATIP_ACQUIRE_SUPPLEMENTAL() ObserveAtipInitializationAcquireSupplemental()
#else
extern void ResetAtipState(void);
extern void ResetDriveControlStateToEight(void);
extern u8 IsIndexedDriveStateActive(u8 selector);
extern void ProcessIndexedStateOperation(void);
extern u8 InitializeControllerOperatingStateAndRunCalibration(void);
extern void ResetControllerPorts8087Through808B(void);
extern void ResetControllerHardwareRegisters(void);
extern void ResetControllerOperatingState(void);
extern void WaitControllerPortReadyAndWriteByte(u32 address, u8 value);
extern u8 AcquireDiscReadPosition(void);
extern void ApplyCurrentOperatingCodeToControllerChannels0Through3(void);
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);
extern void YieldCoroutine1(void);
extern u8 PLEXTOR_NEAR_FUNCTION ValidateAndApplyAtipPacketForMode(u8 mode);
extern u8 PLEXTOR_NEAR_FUNCTION AcquireSupplementalAtipCapacityState(void);
#define ATIP_RESET() ResetAtipState()
#define ATIP_RESET_DRIVE() ResetDriveControlStateToEight()
#define ATIP_IS_INDEXED(selector) IsIndexedDriveStateActive(selector)
#define ATIP_PROCESS_INDEXED() ProcessIndexedStateOperation()
#define ATIP_INITIALIZE_CONTROLLER() InitializeControllerOperatingStateAndRunCalibration()
#define ATIP_RESET_PORTS() ResetControllerPorts8087Through808B()
#define ATIP_RESET_HARDWARE() ResetControllerHardwareRegisters()
#define ATIP_RESET_OPERATING() ResetControllerOperatingState()
#define ATIP_WRITE_CONTROLLER(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define ATIP_ACQUIRE_POSITION() AcquireDiscReadPosition()
#define ATIP_APPLY_OPERATING() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define ATIP_ARM_PACKET() ArmControllerThreeBytePacketCapture()
#define ATIP_PARSE_PACKET() ParseControllerThreeBytePacket()
#define ATIP_YIELD() YieldCoroutine1()
#define ATIP_VALIDATE(mode) ValidateAndApplyAtipPacketForMode(mode)
#define ATIP_ACQUIRE_SUPPLEMENTAL() AcquireSupplementalAtipCapacityState()
#endif
#endif
