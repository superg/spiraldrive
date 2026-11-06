#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveControllerOperatingAdjustment(u8 event, u32 argument);
#define OBSERVE(event, argument) ObserveControllerOperatingAdjustment((event), (argument))
#define READ_BIT31() ((u8)OBSERVE(CONTROLLER_ADJUSTMENT_READ_BIT31, 0ul))
#define READ_BIT33() ((u8)OBSERVE(CONTROLLER_ADJUSTMENT_READ_BIT33, 0ul))
#define WRITE_E8E9(value) ((void)OBSERVE(CONTROLLER_ADJUSTMENT_WRITE_E8E9, (value)))
#define WRITE_EAEB(value) ((void)OBSERVE(CONTROLLER_ADJUSTMENT_WRITE_EAEB, (value)))
#define DIVIDE_BY_SEVEN(value) ((s16)OBSERVE(CONTROLLER_ADJUSTMENT_DIVIDE_BY_SEVEN, (u16)(value)))
#define WRITE_ECED(value) ((void)OBSERVE(CONTROLLER_ADJUSTMENT_WRITE_ECED, (value)))
#define WRITE_EF() ((void)OBSERVE(CONTROLLER_ADJUSTMENT_WRITE_EF, 0ul))
#define SET_EE_BIT7() ((void)OBSERVE(CONTROLLER_ADJUSTMENT_SET_EE_BIT7, 0ul))
#define SCALE_83_POINT_5(value) OBSERVE(CONTROLLER_ADJUSTMENT_SCALE_83_POINT_5, (value))
#define WRITE_EF00(value) ((void)OBSERVE(CONTROLLER_ADJUSTMENT_WRITE_EF00, (value)))
#define MAP_OPERATING_CODE(value) ((u8)OBSERVE(CONTROLLER_ADJUSTMENT_MAP_OPERATING_CODE, (value)))
#else
extern s16 DivideS16Quotient(s16 dividend, s16 divisor);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0033Atomic(void);
#define READ_BIT31() FirmwareReadBit0031()
#define READ_BIT33() ReadBit0033Atomic()
#define WRITE_E8E9(value) WriteControllerParametersE8E9ForValue(value)
#define WRITE_EAEB(value) WriteControllerParametersEAEBFromScaledValue(value)
#define DIVIDE_BY_SEVEN(value) DivideS16Quotient((value), 7)
#define WRITE_ECED(value) WriteControllerParametersECEDFromScaledValue(value)
#define WRITE_EF() WriteControllerParameterEFFromOperatingState()
#define SET_EE_BIT7() SetControllerParameterEEShadowBit7()
#define SCALE_83_POINT_5(value) ScaleUnsignedWordBy83Point5ToU32(value)
#define WRITE_EF00(value) WriteControllerParameterEF00AndConditionalD0Value(value)
#define MAP_OPERATING_CODE(value) MapOperatingCodeToProfileIndex(value)
#endif
#endif
