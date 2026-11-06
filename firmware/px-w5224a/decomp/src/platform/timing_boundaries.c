#include "mcs251.h"

sbit Timer1RunBit = 0x8e;
sbit Timer1OverflowBit = 0x8f;

void PLEXTOR_NEAR_FUNCTION ClearTimer1RunAtomic(void)
{
    Timer1RunBit = 0;
}

void PLEXTOR_NEAR_FUNCTION SetTimer1RunAtomic(void)
{
    Timer1RunBit = 1;
}

void PLEXTOR_NEAR_FUNCTION ClearTimer1OverflowAtomic(void)
{
    Timer1OverflowBit = 0;
}

u8 PLEXTOR_NEAR_FUNCTION ReadTimer1OverflowAtomic(void)
{
    return Timer1OverflowBit;
}
