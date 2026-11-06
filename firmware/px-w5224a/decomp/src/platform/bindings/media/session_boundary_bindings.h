#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveAcquireNextSessionDiscPosition(void);
extern void ObserveApplyNextSessionOperatingChannels(void);
extern void ObserveRearmNextSessionQCapture(void);
extern u8 ObserveProcessNextSessionQPacket(void);
extern u8 ObserveTestNextSessionPulseThreshold(u8 sample_count);
extern void ObserveYieldNextSessionBoundary(void);
#define ACQUIRE_DISC_POSITION() ObserveAcquireNextSessionDiscPosition()
#define APPLY_OPERATING_CHANNELS() ObserveApplyNextSessionOperatingChannels()
#define REARM_Q_CAPTURE() ObserveRearmNextSessionQCapture()
#define PROCESS_Q_PACKET() ObserveProcessNextSessionQPacket()
#define TEST_PULSE_THRESHOLD(sample_count) ObserveTestNextSessionPulseThreshold(sample_count)
#define YIELD_BOUNDARY_WAIT() ObserveYieldNextSessionBoundary()
#else
extern u8 AcquireDiscReadPosition(void);
extern void ApplyCurrentOperatingCodeToControllerChannels0Through3(void);
extern void RearmControllerQPacketCaptureWithTimeout(void);
extern u8 TestP1Cex0PulseSuccessThreshold(u8 sample_count);
extern void YieldCoroutine1(void);
#define ACQUIRE_DISC_POSITION() AcquireDiscReadPosition()
#define APPLY_OPERATING_CHANNELS() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define REARM_Q_CAPTURE() RearmControllerQPacketCaptureWithTimeout()
#define PROCESS_Q_PACKET() ProcessCapturedControllerQPacket()
#define TEST_PULSE_THRESHOLD(sample_count) TestP1Cex0PulseSuccessThreshold(sample_count)
#define YIELD_BOUNDARY_WAIT() YieldCoroutine1()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveQBudgetPulseThreshold(u8 sample_count);
extern void ObserveQBudgetRearmCapture(void);
extern s8 ObserveQBudgetDivide120ByOperatingCode(u8 operating_code);
extern u8 ObserveQBudgetProcessPacket(void);
extern void ObserveQBudgetYield(void);
#define Q_BUDGET_PULSE(count) ObserveQBudgetPulseThreshold(count)
#define Q_BUDGET_REARM() ObserveQBudgetRearmCapture()
#define Q_BUDGET_DIVIDE(code) ObserveQBudgetDivide120ByOperatingCode(code)
#define Q_BUDGET_PROCESS() ObserveQBudgetProcessPacket()
#define Q_BUDGET_YIELD() ObserveQBudgetYield()
#else
extern s8 PLEXTOR_NEAR_FUNCTION DivideS8OperatingBudgetNative(u8 operating_code);
#define Q_BUDGET_PULSE(count) TestP1Cex0PulseSuccessThreshold(count)
#define Q_BUDGET_REARM() RearmControllerQPacketCaptureWithTimeout()
#define Q_BUDGET_DIVIDE(code) DivideS8OperatingBudgetNative(code)
#define Q_BUDGET_PROCESS() ProcessCapturedControllerQPacket()
#define Q_BUDGET_YIELD() YieldCoroutine1()
#endif
#endif
