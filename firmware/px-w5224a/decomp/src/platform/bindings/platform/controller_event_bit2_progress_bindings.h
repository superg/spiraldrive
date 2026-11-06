#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveEventBit2ReadBit37(void);
extern u8 ObserveEventBit2ReadBit33(void);
extern u8 ObserveEventBit2ReadBit28(void);
extern void ObserveEventBit2SetBit28(void);
extern void ObserveEventBit2SetBit33(void);
extern void ObserveEventBit2ClearBit2B(void);
extern void ObserveEventBit2ClearBit43(void);
extern void ObserveEventBit2AdvanceSetupPhase(void);
extern void ObserveEventBit2ServiceSetupState(void);
extern void ObserveEventBit2InitializeMeasurementPhase(void);
extern u16 ObserveEventBit2LookupAdjustment(void);
extern u8 ObserveEventBit2WithinOperatingWindow(u8 use_wide_window);
extern void ObserveEventBit2FinalizeOperatingUpdate(void);
#define EVENT2_READ_BIT37() ObserveEventBit2ReadBit37()
#define EVENT2_READ_BIT33() ObserveEventBit2ReadBit33()
#define EVENT2_READ_BIT28() ObserveEventBit2ReadBit28()
#define EVENT2_SET_BIT28() ObserveEventBit2SetBit28()
#define EVENT2_SET_BIT33() ObserveEventBit2SetBit33()
#define EVENT2_CLEAR_BIT2B() ObserveEventBit2ClearBit2B()
#define EVENT2_CLEAR_BIT43() ObserveEventBit2ClearBit43()
#define EVENT2_ADVANCE_SETUP() ObserveEventBit2AdvanceSetupPhase()
#define EVENT2_SERVICE_SETUP() ObserveEventBit2ServiceSetupState()
#define EVENT2_INITIALIZE_PHASE() ObserveEventBit2InitializeMeasurementPhase()
#define EVENT2_LOOKUP_ADJUSTMENT() ObserveEventBit2LookupAdjustment()
#define EVENT2_WITHIN_WINDOW(wide) ObserveEventBit2WithinOperatingWindow(wide)
#define EVENT2_FINALIZE_UPDATE() ObserveEventBit2FinalizeOperatingUpdate()
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0037ForAdjustedTarget(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0033Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0028ForEventProgress(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0033ForEventProgress(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit002BForAdjustedTarget(void);
extern void PLEXTOR_NEAR_FUNCTION AdvanceControllerBufferSetupPhase(void);
extern void PLEXTOR_NEAR_FUNCTION ServiceControllerBufferSetupStateMachine(void);
#define EVENT2_READ_BIT37() ReadBit0037ForAdjustedTarget()
#define EVENT2_READ_BIT33() ReadBit0033Atomic()
#define EVENT2_READ_BIT28() FirmwareReadBit0028()
#define EVENT2_SET_BIT28() SetBit0028ForEventProgress()
#define EVENT2_SET_BIT33() SetBit0033ForEventProgress()
#define EVENT2_CLEAR_BIT2B() ClearBit002BForAdjustedTarget()
#define EVENT2_CLEAR_BIT43() FirmwareWriteBit0043(0u)
#define EVENT2_ADVANCE_SETUP() AdvanceControllerBufferSetupPhase()
#define EVENT2_SERVICE_SETUP() ServiceControllerBufferSetupStateMachine()
#define EVENT2_INITIALIZE_PHASE() InitializeReadMeasurementControllerPhase()
#define EVENT2_LOOKUP_ADJUSTMENT() LookupControllerOperatingAdjustmentForCurrentPosition()
#define EVENT2_WITHIN_WINDOW(wide) IsControllerReadPathWithinOperatingWindow(wide)
#define EVENT2_FINALIZE_UPDATE() FinalizeControllerEventBit4OperatingUpdate()
#endif
#endif
