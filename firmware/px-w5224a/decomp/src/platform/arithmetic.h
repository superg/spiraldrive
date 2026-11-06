#ifndef PLEXTOR_ARITHMETIC_H
#define PLEXTOR_ARITHMETIC_H

#include "mcs251.h"

u8 MinU8(u8 value, u8 limit);
u16 MinU16(u16 value, u16 limit);
u32 MinU32(u32 value, u32 limit);
s16 AbsoluteValueI16(s16 value);
float PLEXTOR_NEAR_FUNCTION NaturalLogFloat32(float value);
s32 Compute2000TimesLog10OfWordOver32767(u16 value);
u32 MultiplyU32Low(u32 factor_a, u32 factor_b);
u32 ScaleUnsignedWordBy83Point5ToU32(u16 value);
u32 PLEXTOR_NEAR_FUNCTION PowerOfTenU32(u8 exponent);
u32 IntegerSquareRootU32(u32 value);
void UpdateFourByteBitConsensusState(fw_addr_t state_address, u8 new_value);
void ResetPseudoRandomGenerator(u16 seed);
u16 NextPseudoRandom15Bit(void);
void SortWordArrayAscendingInPlace(fw_addr_t array_address, u16 element_count);
void SortByteArrayAscendingInPlace(fw_addr_t array_address, u16 element_count);
u16 ScaleUnsignedByteBy176Point4Rounded(u8 value);
void WriteCdSpeedKilobytesPerSecond(u8 speed_x, fw_addr_t output_address);
u16 GetCdSpeedKilobytesPerSecond(u8 speed_x);

u8 PLEXTOR_NEAR_FUNCTION ToggleR3Bit7IfSignedR3Negative(s8 value);
u8 PLEXTOR_NEAR_FUNCTION ToggleR3Bit7IfSignedWR2Negative(s16 value);
u8 PLEXTOR_NEAR_FUNCTION ToggleR3Bit7IfSignedDR0Negative(s32 value);


void WaitTimer1ScaledDuration(u16 duration_units, u8 yield_while_waiting);

#endif
