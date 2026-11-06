#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_pass.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 lead_in_status;
static u8 calibration_result;
static u8 sequence_statuses[4];
static u8 sequence_count;
static u8 recovery_statuses[2];
static u8 recovery_count;
static u8 retry_eligible;
static u8 enable_count;
static u8 lead_in_count;
static u8 calibration_count;
static u8 select_count;
static u8 copy_count;
static u8 centered_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

void SetControllerOperatingEnableState(u8 enabled)
{
    assert(enabled == 1u);
    ++enable_count;
}

void AcquireMediaState6LeadInMeasurementReference(void)
{
    ++lead_in_count;
    memory[0x00ef7ful] = lead_in_status;
}

u8 ExecuteReadAcquisitionCalibrationForMode2(void)
{
    ++calibration_count;
    return calibration_result;
}

void SelectControllerMeasurementPassByEf72Bit5(void)
{
    ++select_count;
}

void ExecuteControllerMeasurementSequenceAtComputedTarget(void)
{
    memory[0x00ef7ful] = sequence_statuses[sequence_count++];
}

void RetryControllerMeasurementEvaluationWithReadPositionRecovery(void)
{
    memory[0x00ef7ful] = recovery_statuses[recovery_count++];
}

u8 IsControllerMeasurementSeriesRetryEligible(void)
{
    return retry_eligible;
}

void CopyControllerMeasurementSeries00Through05To07Through12(void)
{
    ++copy_count;
}

void InitializeControllerMeasurementSeries00And01FromCenteredSlopes(void)
{
    ++centered_count;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(sequence_statuses, 1, sizeof(sequence_statuses));
    memset(recovery_statuses, 1, sizeof(recovery_statuses));
    lead_in_status = 1u;
    calibration_result = 1u;
    retry_eligible = 0u;
    sequence_count = recovery_count = 0u;
    enable_count = lead_in_count = calibration_count = select_count = 0u;
    copy_count = centered_count = 0u;
    memory[0x00ef80ul] = 1u;
}

static void CheckLeadInFailureAndInitialization(void)
{
    Reset();
    memory[0x00ef72ul] = 0xffu;
    memory[0x00103bul] = 0xffu;
    memory[0x00103aul] = 0x80u;
    lead_in_status = 4u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 2u);
    assert(memory[0x00ef72ul] == 0xfbu);
    assert(memory[0x00ef7eul] == 0u);
    assert(memory[0x00103bul] == 0xfdu);
    assert(memory[0x00103aul] == 0x81u);
    assert(enable_count == 1u && lead_in_count == 1u);
    assert(sequence_count == 0u && select_count == 0u);
}

static void CheckCalibrationRoutes(void)
{
    Reset();
    memory[0x00ef74ul] = 0x28u;
    memory[0x00fb5dul] = 0xb9u;
    calibration_result = 0u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 2u);
    assert(memory[0x00ef74ul] == 0x60u && calibration_count == 1u);

    Reset();
    memory[0x001132ul] = 0x88u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 1u);
    assert(calibration_count == 1u && memory[0x00ef74ul] == 2u);
    assert(memory[0x001132ul] == 0x80u);
}

static void CheckPrimaryTerminalFailure(void)
{
    Reset();
    sequence_statuses[0] = 4u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 2u);
    assert(select_count == 1u && sequence_count == 1u);
    assert(memory[0x00ef79ul] == 4u && memory[0x00ef7eul] == 5u);
}

static void CheckOrdinaryClassification(void)
{
    Reset();
    sequence_statuses[0] = 1u;
    sequence_statuses[1] = 1u;
    recovery_statuses[0] = 1u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 1u);
    assert(sequence_count == 2u && recovery_count == 1u);
    assert(memory[0x00ef79ul] == 1u && memory[0x00ef7aul] == 1u && memory[0x00ef7dul] == 1u);
    assert(memory[0x00ef74ul] == 1u);

    Reset();
    memory[0x00ef80ul] = 0u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 3u);

    Reset();
    sequence_statuses[1] = 2u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 2u);
}

static void CheckSecondaryPassAndStoredFinalStatus(void)
{
    Reset();
    retry_eligible = 1u;
    sequence_statuses[0] = 1u;
    sequence_statuses[1] = 1u;
    sequence_statuses[2] = 1u;
    recovery_statuses[0] = 1u;
    recovery_statuses[1] = 1u;
    assert(ExecuteControllerMeasurementPassAndClassifyResult() == 1u);
    assert(sequence_count == 3u && recovery_count == 2u);
    assert(copy_count == 1u && centered_count == 1u);
    assert((memory[0x00ef72ul] & 4u) != 0u);
    assert((memory[0x00ef72ul] & 2u) == 0u);
    assert(memory[0x00ef74ul] == 2u);
    assert(memory[0x00ef7bul] == 1u && memory[0x00ef7cul] == 1u && memory[0x00ef7dul] == 1u);
}

int main(void)
{
    CheckLeadInFailureAndInitialization();
    CheckCalibrationRoutes();
    CheckPrimaryTerminalFailure();
    CheckOrdinaryClassification();
    CheckSecondaryPassAndStoredFinalStatus();
    return 0;
}
