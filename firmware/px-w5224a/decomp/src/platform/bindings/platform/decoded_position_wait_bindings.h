#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDecodedPositionWaitArm(void);
extern u8 ObserveDecodedPositionWaitParse(void);
extern void ObserveDecodedPositionWaitYield(void);
extern void ObserveDecodedPositionWaitRestore(u8 full_restore);
extern void ObserveDecodedPositionWaitReset(void);
extern u8 ObserveDecodedPositionWaitInitialize(void);
extern void ObserveDecodedPositionWaitEnable(u8 enabled);
#define WaitPositionArm() ObserveDecodedPositionWaitArm()
#define WaitPositionParse() ObserveDecodedPositionWaitParse()
#define WaitPositionYield() ObserveDecodedPositionWaitYield()
#define WaitPositionRestore(value) ObserveDecodedPositionWaitRestore(value)
#define WaitPositionReset() ObserveDecodedPositionWaitReset()
#define WaitPositionInitialize() ObserveDecodedPositionWaitInitialize()
#define WaitPositionEnable(value) ObserveDecodedPositionWaitEnable(value)
#else
extern u8 ParseControllerThreeBytePacket(void);
extern void YieldCoroutine1(void);
extern void ResetDriveControlStateToEight(void);
extern u8 InitializeControllerOperatingStateAndRunCalibration(void);
#define WaitPositionArm() ArmControllerThreeBytePacketCapture()
#define WaitPositionParse() ParseControllerThreeBytePacket()
#define WaitPositionYield() YieldCoroutine1()
#define WaitPositionRestore(value) RestoreControllerReadPathConfiguration(value)
#define WaitPositionReset() ResetDriveControlStateToEight()
#define WaitPositionInitialize() InitializeControllerOperatingStateAndRunCalibration()
#define WaitPositionEnable(value) SetControllerOperatingEnableState(value)
#endif
#endif
