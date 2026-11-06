#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_position_probe.h"
#include "raw_read_runtime.h"
#include "read_acquisition_request.h"

extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern void YieldCoroutine2(void);
extern u8 CheckP1Bit3StableHighForCurrentMediaState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0009Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0017AndClearBit0016ForPositionProbeAtomic(void);

static u32 ReadPositionProbeDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePositionProbeDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void MarkShortLeadInProbeIfApplicable(void)
{
    s32 target = (s32)ReadPositionProbeDword(FW_NEAR_UNKNOWN_1598);

    if(target >= 0 && target <= 0x1c2L)
        SetBit0017AndClearBit0016ForPositionProbeAtomic();
}

static u8 ConsumeSignedRetry(u8 *remaining)
{
    s8 previous = (s8)*remaining;

    *remaining = (u8)(*remaining - 1u);
    return previous > 0 ? 1u : 0u;
}

/* RAM:f9360f */
u8 PLEXTOR_NEAR_FUNCTION ProcessRawReadPositionProbeStep(void)
{
    u8 remaining;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0x19u);
    WritePositionProbeDword(FW_NEAR_UNKNOWN_0F10, 0u);

    for(;;)
    {
        if(ReadBit0009Atomic() != 0u)
            return 0u;

        ThrottleRawReadProducerForQueueCapacity();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) | 4u));
            remaining = 5u;
            for(;;)
            {
                s32 position_delta;
                u8 format_code;

                DispatchReadAcquisitionRequest5ForPipelineTarget();
                YieldCoroutine2();
                while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u)
                    YieldCoroutine2();
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 0u)
                {
                    if(ConsumeSignedRetry(&remaining) != 0u)
                        continue;
                    EnqueueRawSectorDescriptorWithTimingGate(0x0200u, 3u);
                    return 0u;
                }

                if(CheckP1Bit3StableHighForCurrentMediaState() == 0u)
                {
                    if(ConsumeSignedRetry(&remaining) != 0u)
                        continue;
                    MarkShortLeadInProbeIfApplicable();
                    EnqueueRawSectorDescriptorWithTimingGate(0x800bu, 9u);
                    return 0u;
                }

                CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_0F64, FW_NEAR_UNKNOWN_1245, 10u);
                position_delta = (s32)ReadPositionProbeDword(FW_NEAR_UNKNOWN_1598) - ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_0F6B);
                if(position_delta > 150L || position_delta < -150L)
                {
                    if(ConsumeSignedRetry(&remaining) != 0u)
                        continue;
                    EnqueueRawSectorDescriptorWithTimingGate(0x0283u, 3u);
                    return 0u;
                }

                format_code = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F64) & 0x0fu);
                if(format_code == 1u)
                    break;
                if(ConsumeSignedRetry(&remaining) != 0u)
                    continue;
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && format_code == 0u)
                {
                    MarkShortLeadInProbeIfApplicable();
                    EnqueueRawSectorDescriptorWithTimingGate(0x800bu, 9u);
                }
                else
                {
                    EnqueueRawSectorDescriptorWithTimingGate(0x0283u, 3u);
                }
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F20, 0u);
        remaining = 5u;
        for(;;)
        {
            u16 adjustment;
            u32 adjusted_position;

            YieldCoroutine2();
            adjustment = (u16)(0x001bu - FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23));
            adjusted_position = ReadPositionProbeDword(FW_NEAR_UNKNOWN_159C) - (u32)adjustment;
            WritePositionProbeDword(FW_NEAR_UNKNOWN_159C, adjusted_position);
            if(RequestReadAcquisitionAndWait((s32)ReadPositionProbeDword(FW_NEAR_UNKNOWN_1598), (s32)adjusted_position) == 0u)
                break;
            if(ConsumeSignedRetry(&remaining) == 0u)
            {
                EnqueueRawSectorDescriptorWithTimingGate(0x0280u, 3u);
                return 0u;
            }
        }

        InitializeRawReadControllerForPositionProbe();
        do
        {
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F76) <= 0)
            {
                if(ConsumeSignedRetry(&remaining) == 0u)
                {
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 0x14u)
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0u);
                    ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0082u);
                }
            }
            else
            {
                remaining = 6u;
            }
            YieldCoroutine2();
        } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) >= 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) <= 3u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F04) != 0u)
            return 1u;
    }
}
