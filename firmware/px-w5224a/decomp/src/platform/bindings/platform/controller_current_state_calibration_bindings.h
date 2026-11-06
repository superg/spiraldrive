#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCurrentStateCalibrationYield(void);
extern u8 ObserveCurrentStateCalibrationApplyStoredWord(void);
extern u8 ObserveCurrentStateCalibrationPulseTest(u8 sample_count);
extern u8 ObserveCurrentStateCalibrationRunFull(void);
extern u8 ObserveCurrentStateCalibrationCaptureReferences(void);
extern void ObserveCurrentStateCalibrationSetP3High(void);
#define CURRENT_CALIBRATION_YIELD() ObserveCurrentStateCalibrationYield()
#define CURRENT_CALIBRATION_APPLY_STORED_WORD() ObserveCurrentStateCalibrationApplyStoredWord()
#define CURRENT_CALIBRATION_PULSE_TEST(count) ObserveCurrentStateCalibrationPulseTest(count)
#define CURRENT_CALIBRATION_RUN_FULL() ObserveCurrentStateCalibrationRunFull()
#define CURRENT_CALIBRATION_CAPTURE_REFERENCES() ObserveCurrentStateCalibrationCaptureReferences()
#define CURRENT_CALIBRATION_SET_P3_HIGH() ObserveCurrentStateCalibrationSetP3High()
#else
#include "platform/controller_command_c0_calibration.h"
#include "platform/controller_parameters.h"
#include "platform/leaf_workers.h"
#include "platform/media_interrupt_leaves.h"
extern void YieldCoroutine1(void);
extern void SetP3Bit0Atomic(void);
extern void SetP3Bit1Atomic(void);

static u16 ReadCurrentCalibrationWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteCurrentCalibrationWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u8 ApplyStoredWordForCurrentStateCalibration(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x20u) == 0u)
    {
        SubmitControllerWordPair(ReadCurrentCalibrationWord(FW_NEAR_UNKNOWN_FAB8), 0x0204u);
        WriteCurrentCalibrationWord(FW_NEAR_UNKNOWN_F990, ReadCurrentCalibrationWord(FW_NEAR_UNKNOWN_FAB8));
    }
    return LoadParameterFCAndCalibrateWord0319ForState(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008));
}

#define CURRENT_CALIBRATION_YIELD() YieldCoroutine1()
#define CURRENT_CALIBRATION_APPLY_STORED_WORD() ApplyStoredWordForCurrentStateCalibration()
#define CURRENT_CALIBRATION_PULSE_TEST(count) TestP1Cex0PulseSuccessThreshold(count)
#define CURRENT_CALIBRATION_RUN_FULL() CalibrateControllerWords0204And0319Offset()
#define CURRENT_CALIBRATION_CAPTURE_REFERENCES() CalibrateControllerWords030A031AAndCaptureReferences()
#define CURRENT_CALIBRATION_SET_P3_HIGH()                                                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        SetP3Bit0Atomic();                                                                                                                                                                             \
        SetP3Bit1Atomic();                                                                                                                                                                             \
    } while(0)
#endif
#endif
