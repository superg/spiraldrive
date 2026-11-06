#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ReadBit0033Atomic(void);
extern void ObserveWriteBit0031ForReadMeasurementWait(u8 value);
#define READ_MEASUREMENT_BIT33() ReadBit0033Atomic()
#define WRITE_MEASUREMENT_BIT31(value) ObserveWriteBit0031ForReadMeasurementWait(value)
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0033Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION WriteBit0031Atomic(u8 value);
#define READ_MEASUREMENT_BIT33() ReadBit0033Atomic()
#define WRITE_MEASUREMENT_BIT31(value) WriteBit0031Atomic(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrepareReadMeasurementForDescriptorModeEntry(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
if(ObservePrepareReadMeasurementForDescriptorModeEntry() != 0u)
    return;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveAdjustedTargetReadBit(u8 bit_address);
extern void ObserveAdjustedTargetWriteBit(u8 bit_address, u8 value);
extern void ObserveAdjustedTargetInitializeWait(void);
extern void ObserveAdjustedTargetYield(void);
extern u8 ObserveAdjustedTargetOperatingWindow(u8 use_wide_window);
extern u8 ObserveAdjustedTargetAccumulateWriteSpeedFlags(void);
extern void ObserveAdjustedTargetInitializeControllerPhase(void);
extern void ObserveAdjustedTargetWriteParameterEf(u8 value);
extern void ObserveAdjustedTargetWriteParametersEaeb(u16 value);
extern void ObserveAdjustedTargetAcquireRawReadPrefix(void);
extern void ObserveAdjustedTargetAccumulateWindow(u8 sample_divisor);
extern u8 ObservePrepareReadMeasurementAtAdjustedTargetEntry(void);
extern void PLEXTOR_NEAR_FUNCTION ClearRxdatBit3Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetRxdatBit3Atomic(void);
#define ADJUSTED_READ_BIT(address) ObserveAdjustedTargetReadBit(address)
#define ADJUSTED_WRITE_BIT(address, value) ObserveAdjustedTargetWriteBit((address), (value))
#define ADJUSTED_INITIALIZE_WAIT() ObserveAdjustedTargetInitializeWait()
#define ADJUSTED_YIELD() ObserveAdjustedTargetYield()
#define ADJUSTED_OPERATING_WINDOW(wide) ObserveAdjustedTargetOperatingWindow(wide)
#define ADJUSTED_ACCUMULATE_FLAGS() ObserveAdjustedTargetAccumulateWriteSpeedFlags()
#define ADJUSTED_INITIALIZE_PHASE() ObserveAdjustedTargetInitializeControllerPhase()
#define ADJUSTED_WRITE_EF(value) ObserveAdjustedTargetWriteParameterEf(value)
#define ADJUSTED_WRITE_EAEB(value) ObserveAdjustedTargetWriteParametersEaeb(value)
#define ADJUSTED_ACQUIRE_PREFIX() ObserveAdjustedTargetAcquireRawReadPrefix()
#define ADJUSTED_ACCUMULATE_WINDOW(divisor) ObserveAdjustedTargetAccumulateWindow(divisor)
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0037ForAdjustedTarget(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0037ForAdjustedTarget(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0037ForAdjustedTarget(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit002EForAdjustedTarget(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit002EForAdjustedTarget(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit002BForAdjustedTarget(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0038Atomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0044ForReadAcquisitionTarget(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit003AForOperatingProfileAfterResponse(void);
extern void PLEXTOR_NEAR_FUNCTION ClearRxdatBit3Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetRxdatBit3Atomic(void);
extern u8 IsControllerReadPathWithinOperatingWindow(u8 use_wide_window);
extern u8 TryAccumulateReadMeasurementWriteSpeedFlags(void);
extern void InitializeReadMeasurementControllerPhase(void);
extern void WriteControllerParameterEF00AndConditionalD0Value(u8 value);
extern void WriteControllerParametersEAEBFromScaledValue(u16 value);
extern void AcquireRawReadPrefixAtCurrentPipelineTarget(void);
extern void AccumulateReadMeasurementWindowResult(u8 sample_divisor);

static u8 ReadAdjustedTargetBit(u8 bit_address)
{
    switch(bit_address)
    {
    case 0x28u:
        return FirmwareReadBit0028();
    case 0x2bu:
        return FirmwareReadBit002B();
    case 0x2eu:
        return ReadBit002EForAdjustedTarget();
    case 0x30u:
        return FirmwareReadBit0030();
    case 0x32u:
        return FirmwareReadBit0032();
    case 0x33u:
        return ReadBit0033Atomic();
    case 0x34u:
        return FirmwareReadBit0034();
    case 0x36u:
        return FirmwareReadBit0036();
    case 0x37u:
        return ReadBit0037ForAdjustedTarget();
    case 0x38u:
        return ReadBit0038Atomic();
    case 0x44u:
        return ReadBit0044ForReadAcquisitionTarget();
    default:
        return 0u;
    }
}

static void WriteAdjustedTargetBit(u8 bit_address, u8 value)
{
    switch(bit_address)
    {
    case 0x2bu:
        if(value == 0u)
            ClearBit002BForAdjustedTarget();
        break;
    case 0x2eu:
        if(value != 0u)
            SetBit002EForAdjustedTarget();
        break;
    case 0x34u:
        FirmwareWriteBit0034(value);
        break;
    case 0x37u:
        if(value != 0u)
            SetBit0037ForAdjustedTarget();
        else
            ClearBit0037ForAdjustedTarget();
        break;
    case 0x3au:
        if(value == 0u)
            ClearBit003AForOperatingProfileAfterResponse();
        break;
    }
}

#define ADJUSTED_READ_BIT(address) ReadAdjustedTargetBit(address)
#define ADJUSTED_WRITE_BIT(address, value) WriteAdjustedTargetBit((address), (value))
#define ADJUSTED_INITIALIZE_WAIT() InitializeReadMeasurementTargetPacketWait()
#define ADJUSTED_YIELD() YieldCoroutine3()
#define ADJUSTED_OPERATING_WINDOW(wide) IsControllerReadPathWithinOperatingWindow(wide)
#define ADJUSTED_ACCUMULATE_FLAGS() TryAccumulateReadMeasurementWriteSpeedFlags()
#define ADJUSTED_INITIALIZE_PHASE() InitializeReadMeasurementControllerPhase()
#define ADJUSTED_WRITE_EF(value) WriteControllerParameterEF00AndConditionalD0Value(value)
#define ADJUSTED_WRITE_EAEB(value) WriteControllerParametersEAEBFromScaledValue(value)
#define ADJUSTED_ACQUIRE_PREFIX() AcquireRawReadPrefixAtCurrentPipelineTarget()
#define ADJUSTED_ACCUMULATE_WINDOW(divisor) AccumulateReadMeasurementWindowResult(divisor)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
if(ObservePrepareReadMeasurementAtAdjustedTargetEntry() != 0u)
    return;
#endif
#endif
