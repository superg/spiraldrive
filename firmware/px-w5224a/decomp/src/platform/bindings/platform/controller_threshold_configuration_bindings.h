#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveThresholdConfigurationSetP1Bits(void);
extern void ObserveThresholdConfigurationWriteRegister01(u8 value);
extern u8 ObserveThresholdConfigurationMapProfile(u8 operating_code);
extern void ObserveThresholdConfigurationProfile(u8 profile_code);
extern void ObserveThresholdConfigurationSetHighThresholds(u8 operating_code);
extern void ObserveThresholdConfigurationSelectSecondary(s32 threshold_value);
extern void ObserveThresholdConfigurationSubmitWord(u16 value, u16 selector);
extern void ObserveThresholdConfigurationStrobe(fw_addr_t address, u8 value);
extern void ObserveThresholdConfigurationCommonPorts(u8 enabled);
extern void ObserveThresholdConfigurationWritePort(fw_addr_t address, u8 value);
extern void ObserveThresholdConfigurationLoadProfile(u8 operating_code);
extern void ObserveThresholdConfigurationFinalPorts(u8 operating_code);
extern void ObserveThresholdConfigurationFinalize(s32 threshold_value);
extern void ObserveThresholdConfigurationUpdateState103A(void);
#define SET_P1_BITS() ObserveThresholdConfigurationSetP1Bits()
#define WRITE_REGISTER_01(value) ObserveThresholdConfigurationWriteRegister01(value)
#define MAP_PROFILE(code) ObserveThresholdConfigurationMapProfile(code)
#define APPLY_PROFILE(code) ObserveThresholdConfigurationProfile(code)
#define SET_HIGH_THRESHOLDS(code) ObserveThresholdConfigurationSetHighThresholds(code)
#define SELECT_SECONDARY(value) ObserveThresholdConfigurationSelectSecondary(value)
#define SUBMIT_WORD(value, selector) ObserveThresholdConfigurationSubmitWord((value), (selector))
#define STROBE_PORT(address, value) ObserveThresholdConfigurationStrobe((address), (value))
#define CONFIGURE_COMMON_PORTS(enabled) ObserveThresholdConfigurationCommonPorts(enabled)
#define WRITE_PORT(address, value) ObserveThresholdConfigurationWritePort((address), (value))
#define LOAD_PROFILE(code) ObserveThresholdConfigurationLoadProfile(code)
#define CONFIGURE_FINAL_PORTS(code) ObserveThresholdConfigurationFinalPorts(code)
#define FINALIZE_TRANSITION(value) ObserveThresholdConfigurationFinalize(value)
#define UPDATE_STATE_103A() ObserveThresholdConfigurationUpdateState103A()
#else
#define SET_P1_BITS() SetP1Bits0And1()
#define WRITE_REGISTER_01(value) WriteControllerRegister01Clamped(value)
#define MAP_PROFILE(code) MapOperatingCodeToProfileIndex(code)
#define SET_HIGH_THRESHOLDS(code) SetHighOperatingCodeThresholds(code)
#define SELECT_SECONDARY(value) SelectSecondaryOperatingCodeForThresholdValue(value)
#define SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define STROBE_PORT(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define CONFIGURE_COMMON_PORTS(enabled) ConfigureControllerPorts8046And804EAnd80C9Through80CB(enabled)
#define WRITE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define LOAD_PROFILE(code) LoadControllerOperatingProfile(code)
#define CONFIGURE_FINAL_PORTS(code) ConfigureControllerPortsForOperatingCode(code)
#define FINALIZE_TRANSITION(value) FinalizeControllerOperatingCodeTransition(value)
#define UPDATE_STATE_103A() UpdateState103ABit6ForMediaState()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
APPLY_PROFILE(1u);
#else
ConfigureControllerParameterProfileForCode01();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
APPLY_PROFILE(4u);
#else
ConfigureControllerParameterProfileForCode04();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
APPLY_PROFILE(8u);
#else
ConfigureControllerParameterProfileForCode08();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
APPLY_PROFILE(10u);
#else
ConfigureControllerParameterProfileForCode0A();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
APPLY_PROFILE(12u);
#else
ConfigureControllerParameterProfileForCode0C();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
APPLY_PROFILE(16u);
#else
ConfigureControllerParameterProfileForCode10();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 8
#if defined(PLEXTOR_HOST_TEST)
APPLY_PROFILE(0x28u);
#else
ConfigureControllerParameterProfileCode28F125();
#endif
#endif
