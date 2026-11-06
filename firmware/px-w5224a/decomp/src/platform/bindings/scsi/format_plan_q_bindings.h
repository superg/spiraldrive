#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveFormatPlanPackedBcd(u8 value);
extern void ObserveFormatPlanFrameToMsf(s32 frame_position, fw_addr_t output_address);
extern u8 ObserveFormatPlanLastTrack(void);
extern u8 ObserveFormatPlanBuildPma(fw_addr_t destination);
extern u8 ObserveFormatPlanPackedBcdToBinary(u8 value);
#define FORMAT_PLAN_BCD(value) ObserveFormatPlanPackedBcd(value)
#define FORMAT_PLAN_FRAME_TO_MSF(frame, output) ObserveFormatPlanFrameToMsf((frame), (output))
#define FORMAT_PLAN_LAST_TRACK() ObserveFormatPlanLastTrack()
#define FORMAT_PLAN_BUILD_PMA(destination, count) ((count) = ObserveFormatPlanBuildPma(destination))
#define FORMAT_PLAN_UNPACK_BCD(value) ObserveFormatPlanPackedBcdToBinary(value)
#else
#include "platform/encoding.h"
#include "platform/status_mapping.h"
#include "platform/track_state.h"
#define FORMAT_PLAN_BCD(value) BinaryByteToPackedBcd(value)
#define FORMAT_PLAN_FRAME_TO_MSF(frame, output) ConvertFramePositionToBinaryMsfTupleFar((frame), (output))
#define FORMAT_PLAN_LAST_TRACK() GetEffectiveLastTrackNumber()
#define FORMAT_PLAN_BUILD_PMA(destination, count) BuildPmaAdr3Adr5RawQRecordSequence((destination), (fw_addr_t) & (count))
#define FORMAT_PLAN_UNPACK_BCD(value) PackedBcdByteToBinary(value)
#endif
#endif
