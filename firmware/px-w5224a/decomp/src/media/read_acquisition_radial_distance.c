#include "read_acquisition_radial_distance.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/read_acquisition_radial_distance_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteBigEndianU16ForRadialDistance(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadBigEndianU32ForRadialDistance(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32ForRadialDistance(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u16 MapSignedFramePositionToRadialValue(s32 frame_position)
{
    if(frame_position < 0)
    {
        s32 magnitude = (s32)(0ul - (u32)frame_position);
        return MapReadAcquisitionLeadInSeconds((u16)DivideReadAcquisitionFramePositionBy75(magnitude));
    }
    return MapReadAcquisitionFrameSeconds((u16)DivideReadAcquisitionFramePositionBy75(frame_position));
}

/* RAM:fc928f */
u16 PLEXTOR_NEAR_FUNCTION CalculateRadialModelDistanceToReadTarget(void)
{
    s32 target_position = (s32)ReadBigEndianU32ForRadialDistance(FW_NEAR_UNKNOWN_0E3B);
    s32 current_position;
    u16 current_value;
    u16 target_value;

    WriteBigEndianU32ForRadialDistance(FW_NEAR_UNKNOWN_0EED, (u32)target_position);
    current_position = (s32)ReadBigEndianU32ForRadialDistance(FW_NEAR_UNKNOWN_1235);
    if(current_position < 0)
    {
        WriteBigEndianU16ForRadialDistance(FW_NEAR_UNKNOWN_0ED3, 0xffffu);
        current_value = MapSignedFramePositionToRadialValue(current_position);
    }
    else
    {
        current_value = MapSignedFramePositionToRadialValue(current_position);
        WriteBigEndianU16ForRadialDistance(FW_NEAR_UNKNOWN_0ED3, current_value);
    }

    if(target_position < 0)
        WriteBigEndianU16ForRadialDistance(FW_NEAR_UNKNOWN_0ED3, 0xffffu);
    target_value = MapSignedFramePositionToRadialValue(target_position);

    if((current_position < 0) != (target_position < 0))
        return (u16)(current_value + target_value);
    return target_value < current_value ? (u16)(current_value - target_value) : (u16)(target_value - current_value);
}
