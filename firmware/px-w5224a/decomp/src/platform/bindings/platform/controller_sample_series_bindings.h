#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteD4E2ForControllerSampleSeries(u8 code);
extern void ObserveRaiseF4ForControllerSampleSeries(void);
extern void ObserveWaitForControllerSampleSeries(u16 duration_units, u8 yield_while_waiting);
extern u8 ObserveReadSfrForControllerSampleSeries(u8 address);
extern void ObserveWriteSfrForControllerSampleSeries(u8 address, u8 value);
extern void ObserveRestoreD4E2ForControllerSampleSeries(void);
extern void ObserveLowerF4ForControllerSampleSeries(void);
#define WRITE_D4_E2(code) ObserveWriteD4E2ForControllerSampleSeries(code)
#define RAISE_F4() ObserveRaiseF4ForControllerSampleSeries()
#define WAIT_TIMER(duration, yield) ObserveWaitForControllerSampleSeries((duration), (yield))
#define READ_SFR_E1() ObserveReadSfrForControllerSampleSeries(FW_SFR_UNKNOWN_E1)
#define READ_SFR_E3() ObserveReadSfrForControllerSampleSeries(FW_SFR_UNKNOWN_E3)
#define READ_SFR_F1() ObserveReadSfrForControllerSampleSeries(FW_SFR_UNKNOWN_F1)
#define READ_SFR_D8() ObserveReadSfrForControllerSampleSeries(FW_SFR_UNKNOWN_D8)
#define READ_SFR_D9() ObserveReadSfrForControllerSampleSeries(FW_SFR_UNKNOWN_D9)
#define READ_SFR_DB() ObserveReadSfrForControllerSampleSeries(FW_SFR_UNKNOWN_DB)
#define WRITE_SFR_E1(value) ObserveWriteSfrForControllerSampleSeries(FW_SFR_UNKNOWN_E1, (value))
#define WRITE_SFR_E3(value) ObserveWriteSfrForControllerSampleSeries(FW_SFR_UNKNOWN_E3, (value))
#define RESTORE_D4_E2() ObserveRestoreD4E2ForControllerSampleSeries()
#define LOWER_F4() ObserveLowerF4ForControllerSampleSeries()
#else
#define WRITE_D4_E2(code) WriteControllerParametersD4AndE2ForCode(code)
#define RAISE_F4() WriteControllerParameterF4WithUpAdjustment()
#define WAIT_TIMER(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define READ_SFR_E1() FirmwareReadDirectSfr8(E1)
#define READ_SFR_E3() FirmwareReadSfr8(FW_SFR_UNKNOWN_E3)
#define READ_SFR_F1() FirmwareReadDirectSfr8(F1)
#define READ_SFR_D8() FirmwareReadDirectSfr8(D8)
#define READ_SFR_D9() FirmwareReadDirectSfr8(D9)
#define READ_SFR_DB() FirmwareReadDirectSfr8(DB)
#define WRITE_SFR_E1(value) FirmwareWriteDirectSfr8(E1, (value))
#define WRITE_SFR_E3(value) FirmwareWriteSfr8(FW_SFR_UNKNOWN_E3, (value))
#define RESTORE_D4_E2() WriteControllerParametersD412AndE2Value58()
#define LOWER_F4() WriteControllerParameterF4WithDownAdjustment()
#endif
#endif
