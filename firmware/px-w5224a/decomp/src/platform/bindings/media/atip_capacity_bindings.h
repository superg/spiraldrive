#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveAcquireSupplementalAtipDiscPosition(void);
extern void ObserveApplySupplementalAtipOperatingCode(void);
extern void ObserveArmSupplementalAtipPacketCapture(void);
extern u8 ObserveParseSupplementalAtipPacket(void);
extern void ObserveYieldSupplementalAtip(void);
extern u8 ObserveValidateSupplementalAtipPacket(u8 validation_mode);
extern u32 ObserveMultiplySupplementalAtipCapacity(u32 factor_a, u32 factor_b);
extern void ObserveResetSupplementalAtipDriveControl(void);
#define ACQUIRE_SUPPLEMENTAL_ATIP_POSITION() ObserveAcquireSupplementalAtipDiscPosition()
#define APPLY_SUPPLEMENTAL_ATIP_OPERATING_CODE() ObserveApplySupplementalAtipOperatingCode()
#define ARM_SUPPLEMENTAL_ATIP_PACKET() ObserveArmSupplementalAtipPacketCapture()
#define PARSE_SUPPLEMENTAL_ATIP_PACKET() ObserveParseSupplementalAtipPacket()
#define YIELD_SUPPLEMENTAL_ATIP() ObserveYieldSupplementalAtip()
#define VALIDATE_SUPPLEMENTAL_ATIP_PACKET(mode) ObserveValidateSupplementalAtipPacket(mode)
#define MULTIPLY_SUPPLEMENTAL_ATIP_CAPACITY(a, b) ObserveMultiplySupplementalAtipCapacity((a), (b))
#define RESET_SUPPLEMENTAL_ATIP_DRIVE_CONTROL() ObserveResetSupplementalAtipDriveControl()
#else
extern u8 AcquireDiscReadPosition(void);
extern void ApplyCurrentOperatingCodeToControllerChannels0Through3(void);
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);
extern void YieldCoroutine1(void);
extern u8 PLEXTOR_NEAR_FUNCTION ValidateAndApplyAtipPacketForMode(u8 packet_validation_mode);
extern u32 MultiplyU32Low(u32 factor_a, u32 factor_b);
extern void ResetDriveControlStateToEight(void);
#define ACQUIRE_SUPPLEMENTAL_ATIP_POSITION() AcquireDiscReadPosition()
#define APPLY_SUPPLEMENTAL_ATIP_OPERATING_CODE() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define ARM_SUPPLEMENTAL_ATIP_PACKET() ArmControllerThreeBytePacketCapture()
#define PARSE_SUPPLEMENTAL_ATIP_PACKET() ParseControllerThreeBytePacket()
#define YIELD_SUPPLEMENTAL_ATIP() YieldCoroutine1()
#define VALIDATE_SUPPLEMENTAL_ATIP_PACKET(mode) ValidateAndApplyAtipPacketForMode(mode)
#define MULTIPLY_SUPPLEMENTAL_ATIP_CAPACITY(a, b) MultiplyU32Low((a), (b))
#define RESET_SUPPLEMENTAL_ATIP_DRIVE_CONTROL() ResetDriveControlStateToEight()
#endif
#endif
