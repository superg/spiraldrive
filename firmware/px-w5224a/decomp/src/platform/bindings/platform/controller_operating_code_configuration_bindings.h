#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCurrentOperatingCodeStrobe(fw_addr_t port_address, u8 value);
extern void ObserveCurrentOperatingCodeSetP1Bits(void);
extern void ObserveCurrentOperatingCodeProfile(u8 profile_kind, u8 alternate_profile);
extern void ObserveCurrentOperatingCodeCommonPorts(u8 enabled);
extern void ObserveCurrentOperatingCodeWords(u8 operating_code);
extern void ObserveCurrentOperatingCodeLoadProfile(u8 operating_code);
extern u8 ObserveCurrentOperatingCodeMapChannel3(u8 operating_code);
extern void ObserveCurrentOperatingCodeSetChannel(u8 channel_index, u8 value);
extern void ObserveCurrentOperatingCodeFinalPorts(u8 operating_code);
#define DispatchCurrentOperatingCodeStrobe(address, value) ObserveCurrentOperatingCodeStrobe((address), (value))
#define DispatchCurrentOperatingCodeSetP1Bits() ObserveCurrentOperatingCodeSetP1Bits()
#define DispatchCurrentOperatingCodeProfile(kind, alternate) ObserveCurrentOperatingCodeProfile((kind), (alternate))
#define DispatchCurrentOperatingCodeCommonPorts(enabled) ObserveCurrentOperatingCodeCommonPorts(enabled)
#define DispatchCurrentOperatingCodeWords(code) ObserveCurrentOperatingCodeWords(code)
#define DispatchCurrentOperatingCodeLoadProfile(code) ObserveCurrentOperatingCodeLoadProfile(code)
#define DispatchCurrentOperatingCodeMapChannel3(code) ObserveCurrentOperatingCodeMapChannel3(code)
#define DispatchCurrentOperatingCodeSetChannel(index, value) ObserveCurrentOperatingCodeSetChannel((index), (value))
#define DispatchCurrentOperatingCodeFinalPorts(code) ObserveCurrentOperatingCodeFinalPorts(code)
#else
#define DispatchCurrentOperatingCodeStrobe(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define DispatchCurrentOperatingCodeSetP1Bits() SetP1Bits0And1()
#define DispatchCurrentOperatingCodeCommonPorts(enabled) ConfigureControllerPorts8046And804EAnd80C9Through80CB(enabled)
#define DispatchCurrentOperatingCodeWords(code) ConfigureControllerWords035F036F037FForOperatingCode(code)
#define DispatchCurrentOperatingCodeLoadProfile(code) LoadControllerOperatingProfile(code)
#define DispatchCurrentOperatingCodeMapChannel3(code) MapOperatingCodeToControllerChannel3Value(code)
#define DispatchCurrentOperatingCodeSetChannel(index, value) SetIndexedControllerChannelValue((index), (value))
#define DispatchCurrentOperatingCodeFinalPorts(code) ConfigureControllerPortsForOperatingCode(code)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u8 profile_kind;

if(operating_code == 4u)
    profile_kind = 4u;
else if(operating_code == 8u)
    profile_kind = 8u;
else if(operating_code == 0x18u)
    profile_kind = 0x18u;
else if(operating_code == 0x20u)
    profile_kind = 0x20u;
else if(operating_code == 0x28u)
    profile_kind = 0x28u;
else if(operating_code == 1u)
    profile_kind = 1u;
else
    profile_kind = 0x30u;
DispatchCurrentOperatingCodeProfile(profile_kind, 0u);
#else
if(operating_code == 4u)
    ConfigureControllerParameterProfileForCode04();
else if(operating_code == 8u)
    ConfigureControllerParameterProfileForCode08();
else if(operating_code == 0x18u)
    ConfigureControllerParameterProfileForCode18(0u);
else if(operating_code == 0x20u)
    ConfigureControllerParameterProfileForCode20(0u);
else if(operating_code == 0x28u)
    ConfigureControllerParameterProfileForCode28(0u);
else if(operating_code == 1u)
    ConfigureControllerParameterProfileForCode01();
else
    ConfigureControllerParameterProfileForCode30(0u);
#endif
#endif
