#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateReadFormatCapacitiesCdb(fw_addr_t mask_descriptor);
extern void ObserveUnsupportedReadFormatCapacitiesCommand(void);
extern void ObserveLoadReadFormatCapacitiesTrack(u8 track_number);
extern s32 ObserveComputeReadFormatCapacitiesNextWritablePosition(void);
extern s32 ObserveConvertReadFormatCapacitiesMsf(u8 minute, u8 second, u8 frame);
extern u32 ObserveMultiplyReadFormatCapacities(u32 factor_a, u32 factor_b);
extern u32 ObserveDivideReadFormatCapacities(u32 dividend, u32 divisor);
extern u32 ObserveComputeReadFormatCapacitiesGeometry(void);
extern u8 ObserveTransferReadFormatCapacitiesResponse(fw_addr_t source_address, u32 byte_count);
extern void ObserveSetNoSenseForReadFormatCapacities(void);
#define READ_FORMAT_VALIDATE(mask) ObserveValidateReadFormatCapacitiesCdb(mask)
#define READ_FORMAT_UNSUPPORTED() ObserveUnsupportedReadFormatCapacitiesCommand()
#define READ_FORMAT_LOAD_TRACK(number) ObserveLoadReadFormatCapacitiesTrack(number)
#define READ_FORMAT_NEXT_WRITABLE() ObserveComputeReadFormatCapacitiesNextWritablePosition()
#define READ_FORMAT_CONVERT_MSF(minute, second, frame) ObserveConvertReadFormatCapacitiesMsf((minute), (second), (frame))
#define READ_FORMAT_MULTIPLY(left, right) ObserveMultiplyReadFormatCapacities((left), (right))
#define READ_FORMAT_DIVIDE(dividend, divisor) ObserveDivideReadFormatCapacities((dividend), (divisor))
#define READ_FORMAT_GEOMETRY() ObserveComputeReadFormatCapacitiesGeometry()
#define READ_FORMAT_TRANSFER(source, count) ObserveTransferReadFormatCapacitiesResponse((source), (count))
#define READ_FORMAT_NO_SENSE() ObserveSetNoSenseForReadFormatCapacities()
#else
#define READ_FORMAT_VALIDATE(mask) ValidateScsiCdbFieldMasks(mask)
#define READ_FORMAT_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define READ_FORMAT_LOAD_TRACK(number) LoadTrackInformationByNumber(number)
#define READ_FORMAT_NEXT_WRITABLE() ComputeWorkingTrackNextWritableFramePosition()
#define READ_FORMAT_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define READ_FORMAT_DIVIDE(dividend, divisor) DivideU32Quotient((dividend), (divisor))
#define READ_FORMAT_GEOMETRY() ComputeAndPublishMediaCapacityGeometry()
#define READ_FORMAT_TRANSFER(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define READ_FORMAT_NO_SENSE() SetNoSenseData()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return READ_FORMAT_CONVERT_MSF(minute, second, frame);
#else
u8 tuple[3];

tuple[0] = minute;
tuple[1] = second;
tuple[2] = frame;
return ConvertBinaryMsfToAdjustedFramePosition((fw_addr_t)tuple);
#endif
#endif
