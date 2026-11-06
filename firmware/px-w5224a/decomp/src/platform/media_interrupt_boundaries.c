#include "mcs251.h"
#include "raw_read_q_control.h"

u8 bdata SubChannelCaptureBitByte21 _at_ 0x21;
sbit SubChannelCaptureBit = SubChannelCaptureBitByte21 ^ 6;

u8 PLEXTOR_NEAR_FUNCTION ReadSubChannelCaptureBit(void)
{
    return SubChannelCaptureBit;
}

void PLEXTOR_NEAR_FUNCTION ClearReadSubChannelCaptureBit(void)
{
    SubChannelCaptureBit = 0;
}

void PLEXTOR_NEAR_FUNCTION UpdateRawReadQPacketControlAndIdentificationForBuild(void)
{
    UpdateRawReadQPacketControlAndIdentification(1u, 1u);
}
