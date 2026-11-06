#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrimaryRawReadCaptureBusy(void);
extern void ObservePrimaryRawReadCaptureYield(void);
extern void ObservePrimaryRawReadCaptureConvert(s32 frame_position, fw_addr_t destination);
extern void ObservePrimaryRawReadCapturePackBcd(u16 tuple_address);
extern void ObservePrimaryRawReadCaptureCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObservePrimaryRawReadCaptureWriteControllerBlock(fw_addr_t source_address);
extern void ObservePrimaryRawReadCaptureSubmit(fw_addr_t controller_buffer_address, u8 command_parameter);
extern u32 ObservePrimaryRawReadCaptureMultiply(u32 factor_a, u32 factor_b);
extern void ObservePrimaryRawReadCaptureIncrement(u16 tuple_address);
#define CAPTURE_BUSY() ObservePrimaryRawReadCaptureBusy()
#define CAPTURE_YIELD() ObservePrimaryRawReadCaptureYield()
#define CAPTURE_CONVERT(frame, destination) ObservePrimaryRawReadCaptureConvert((frame), (destination))
#define CAPTURE_PACK_BCD(address) ObservePrimaryRawReadCapturePackBcd(address)
#define CAPTURE_COPY(source, destination, count) ObservePrimaryRawReadCaptureCopy((source), (destination), (count))
#define CAPTURE_WRITE_CONTROLLER_BLOCK(source) ObservePrimaryRawReadCaptureWriteControllerBlock(source)
#define CAPTURE_SUBMIT(address, parameter) ObservePrimaryRawReadCaptureSubmit((address), (parameter))
#define CAPTURE_MULTIPLY(left, right) ObservePrimaryRawReadCaptureMultiply((left), (right))
#define CAPTURE_INCREMENT(address) ObservePrimaryRawReadCaptureIncrement(address)
#else
extern void YieldCoroutine2(void);
u8 bdata PrimaryRawReadCaptureStateBitByte _at_ 0x27;
sbit PrimaryRawReadCaptureBusy = PrimaryRawReadCaptureStateBitByte ^ 4;
#define CAPTURE_BUSY() PrimaryRawReadCaptureBusy
#define CAPTURE_YIELD() YieldCoroutine2()
#define CAPTURE_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define CAPTURE_PACK_BCD(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define CAPTURE_COPY(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define CAPTURE_WRITE_CONTROLLER_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define CAPTURE_SUBMIT(address, parameter) SubmitControllerCommand80AtBufferAddressAndWait((address), (parameter))
#define CAPTURE_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define CAPTURE_INCREMENT(address) IncrementPackedBcdMsf(address)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t temporary_tuple = FW_NEAR_UNKNOWN_7000;
#else
u8 tuple[3];
fw_addr_t temporary_tuple = (fw_addr_t)tuple;
#endif
#endif
