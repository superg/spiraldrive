#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveSetCdSpeedValidate(fw_addr_t mask_descriptor);
extern void ObserveSetCdSpeedUnsupported(void);
extern u8 ObserveSetCdSpeedBit0008(void);
extern void ObserveSetCdSpeedClearBit000E(void);
extern void ObserveSetCdSpeedResetProgress(void);
extern void ObserveSetCdSpeedResetQueue(void);
extern void ObserveSetCdSpeedInitializeCoroutine2(u32 entry_point);
extern void ObserveSetCdSpeedDispatchRequest5(void);
extern void ObserveSetCdSpeedYield(void);
extern void ObserveSetCdSpeedNoSense(void);
#define SET_CD_SPEED_VALIDATE(mask) ObserveSetCdSpeedValidate(mask)
#define SET_CD_SPEED_UNSUPPORTED() ObserveSetCdSpeedUnsupported()
#define READ_SET_CD_SPEED_BIT0008() ObserveSetCdSpeedBit0008()
#define CLEAR_SET_CD_SPEED_BIT000E() ObserveSetCdSpeedClearBit000E()
#define RESET_SET_CD_SPEED_PROGRESS() ObserveSetCdSpeedResetProgress()
#define RESET_SET_CD_SPEED_QUEUE() ObserveSetCdSpeedResetQueue()
#define INITIALIZE_SET_CD_SPEED_COROUTINE(entry) ObserveSetCdSpeedInitializeCoroutine2(entry)
#define DISPATCH_SET_CD_SPEED_REQUEST5() ObserveSetCdSpeedDispatchRequest5()
#define YIELD_SET_CD_SPEED() ObserveSetCdSpeedYield()
#define SET_CD_SPEED_NO_SENSE() ObserveSetCdSpeedNoSense()
#else
u8 bdata SetCdSpeedBitByte21 _at_ 0x21;
sbit SetCdSpeedBit0008 = SetCdSpeedBitByte21 ^ 0;
sbit SetCdSpeedBit000E = SetCdSpeedBitByte21 ^ 6;
#define SET_CD_SPEED_VALIDATE(mask) ValidateScsiCdbFieldMasks(mask)
#define SET_CD_SPEED_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define READ_SET_CD_SPEED_BIT0008() ((u8)SetCdSpeedBit0008)
#define CLEAR_SET_CD_SPEED_BIT000E() (SetCdSpeedBit000E = 0)
#define RESET_SET_CD_SPEED_PROGRESS() ResetRawReadProgressCountersAndPhaseFlags()
#define RESET_SET_CD_SPEED_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_SET_CD_SPEED_COROUTINE(entry) InitializeCoroutine2(entry)
#define DISPATCH_SET_CD_SPEED_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define YIELD_SET_CD_SPEED() YieldCoroutine0()
#define SET_CD_SPEED_NO_SENSE() SetNoSenseData()
extern void YieldCoroutine0(void);
#endif
#endif
