#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveMediaStateCoroutineState0(void);
extern void ObserveMediaStateCoroutineState1(void);
extern void ObserveMediaStateCoroutineReadAcquisition(void);
extern void ObserveMediaStateCoroutineFinalizeState2(void);
extern void ObserveMediaStateCoroutineState3(void);
extern void ObserveMediaStateCoroutineState5(void);
extern void ObserveMediaStateCoroutineFinalizeState5(void);
extern void ObserveMediaStateCoroutineState6(void);
extern void ObserveMediaStateCoroutineTerminalTrap(void);
#define DispatchMediaState0() ObserveMediaStateCoroutineState0()
#define DispatchMediaState1() ObserveMediaStateCoroutineState1()
#define DispatchReadAcquisitionState() ObserveMediaStateCoroutineReadAcquisition()
#define DispatchFinalizeMediaState2() ObserveMediaStateCoroutineFinalizeState2()
#define DispatchMediaState3() ObserveMediaStateCoroutineState3()
#define DispatchMediaState5() ObserveMediaStateCoroutineState5()
#define DispatchFinalizeMediaState5() ObserveMediaStateCoroutineFinalizeState5()
#define DispatchMediaState6() ObserveMediaStateCoroutineState6()
#define EnterMediaStateTerminalTrap() ObserveMediaStateCoroutineTerminalTrap()
#else
extern void PLEXTOR_NEAR_FUNCTION ProcessMediaCoroutineState0(void);
extern void PLEXTOR_NEAR_FUNCTION ProcessMediaCoroutineState6(void);
extern void PLEXTOR_NEAR_FUNCTION FinalizeMediaCoroutineState2NearAdapter(void);
extern void PLEXTOR_NEAR_FUNCTION FinalizeMediaCoroutineState5NearAdapter(void);
#define DispatchMediaState0() ProcessMediaCoroutineState0()
#define DispatchMediaState1() ProcessMediaCoroutineState1()
#define DispatchReadAcquisitionState() ProcessReadAcquisitionState()
#define DispatchFinalizeMediaState2() FinalizeMediaCoroutineState2NearAdapter()
#define DispatchMediaState3() ProcessMediaCoroutineState3()
#define DispatchMediaState5() ProcessMediaCoroutineState5()
#define DispatchFinalizeMediaState5() FinalizeMediaCoroutineState5NearAdapter()
#define DispatchMediaState6() ProcessMediaCoroutineState6()
#define EnterMediaStateTerminalTrap()                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        for(;;)                                                                                                                                                                                        \
        {                                                                                                                                                                                              \
        }                                                                                                                                                                                              \
    } while(0)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveMediaState0SetIndexedP3Pattern(void);
extern void ObserveMediaState0ResetDriveControl(void);
extern void ObserveMediaState0WriteD6Value7E(void);
extern void ObserveMediaState0SetP1TimerPinsLow(void);
extern void ObserveMediaState0YieldCoroutine1(void);
extern void ObserveMediaState0ServiceIndexedWait(void);
extern void ObserveMediaState0TryAdvanceState(void);
extern void ObserveMediaState0SetP1TimerPinsHigh(void);
extern void ObserveMediaState0WriteD6Value00(void);
extern u8 ObserveMediaState0InitializeControllerCalibration(void);
#define MediaState0SetIndexedP3Pattern() ObserveMediaState0SetIndexedP3Pattern()
#define MediaState0ResetDriveControl() ObserveMediaState0ResetDriveControl()
#define MediaState0WriteD6Value7E() ObserveMediaState0WriteD6Value7E()
#define MediaState0SetP1TimerPinsLow() ObserveMediaState0SetP1TimerPinsLow()
#define MediaState0Yield() ObserveMediaState0YieldCoroutine1()
#define MediaState0ServiceIndexedWait() ObserveMediaState0ServiceIndexedWait()
#define MediaState0TryAdvanceState() ObserveMediaState0TryAdvanceState()
#define MediaState0SetP1TimerPinsHigh() ObserveMediaState0SetP1TimerPinsHigh()
#define MediaState0WriteD6Value00() ObserveMediaState0WriteD6Value00()
#define MediaState0InitializeControllerCalibration() ObserveMediaState0InitializeControllerCalibration()
#else
extern void ResetDriveControlStateToEight(void);
extern u8 InitializeControllerOperatingStateAndRunCalibration(void);
extern void YieldCoroutine1(void);
extern void PLEXTOR_NEAR_FUNCTION SetMediaState0IndexedP3PatternAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetMediaState0P1TimerPinsLowAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetMediaState0P1TimerPinsHighAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ServiceMediaState0IndexedWaitNearAdapter(void);
extern void PLEXTOR_NEAR_FUNCTION TryAdvanceMediaState0NearAdapter(void);
#define MediaState0SetIndexedP3Pattern() SetMediaState0IndexedP3PatternAtomic()
#define MediaState0ResetDriveControl() ResetDriveControlStateToEight()
#define MediaState0WriteD6Value7E() WriteControllerParameterD6Value7E()
#define MediaState0SetP1TimerPinsLow() SetMediaState0P1TimerPinsLowAtomic()
#define MediaState0Yield() YieldCoroutine1()
#define MediaState0ServiceIndexedWait() ServiceMediaState0IndexedWaitNearAdapter()
#define MediaState0TryAdvanceState() TryAdvanceMediaState0NearAdapter()
#define MediaState0SetP1TimerPinsHigh() SetMediaState0P1TimerPinsHighAtomic()
#define MediaState0WriteD6Value00() WriteControllerParameterD6Value00()
#define MediaState0InitializeControllerCalibration() InitializeControllerOperatingStateAndRunCalibration()
#endif
#endif
