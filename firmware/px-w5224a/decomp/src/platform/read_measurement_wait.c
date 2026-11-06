#include "read_measurement_wait.h"
#include "encoding.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/read_measurement_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern void YieldCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementAtAdjustedTarget(void);

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/read_measurement_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadMeasurementWaitU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteMeasurementWaitU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadMeasurementWaitU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteMeasurementWaitU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fab563 */
void InitializeReadMeasurementTargetPacketWait(void)
{
    s32 target_position;
    u8 value;

    if(READ_MEASUREMENT_BIT33() == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 1u)
        WRITE_MEASUREMENT_BIT31(0u);
    else
        WRITE_MEASUREMENT_BIT31(1u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) > 0x28u && (s32)ReadMeasurementWaitU32(FW_NEAR_UNKNOWN_FBA6) > 0x00047694l)
    {
        WriteMeasurementWaitU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadMeasurementWaitU16(FW_NEAR_UNKNOWN_FAD7) | 0x0004u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD3, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 0x28u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x28u);
        WriteMeasurementWaitU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadMeasurementWaitU16(FW_NEAR_UNKNOWN_FAD7) | 0x0020u));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
    target_position = (s32)ReadMeasurementWaitU32(FW_NEAR_UNKNOWN_FBA6);
    WriteMeasurementWaitU32(FW_NEAR_UNKNOWN_0E3B, (u32)target_position);
    target_position = (s32)ReadMeasurementWaitU32(FW_NEAR_UNKNOWN_FBA6);
    ConvertFramePositionToBinaryMsfTuple(target_position - 1l, 0x154eul);
    value = FirmwareReadNear8(FW_NEAR_UNKNOWN_154E);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_154E, BinaryByteToPackedBcd(value));
    value = FirmwareReadNear8(FW_NEAR_UNKNOWN_154F);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_154F, BinaryByteToPackedBcd(value));
    value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1550);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1550, BinaryByteToPackedBcd(value));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 5u);
    YieldCoroutine3();
}

/* RAM:fd99dd */
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForDescriptorMode(void)
{
    u32 target_position;
    u8 mode;
    u8 offset;

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/read_measurement_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E);
    switch(mode)
    {
    case 0u:
    case 1u:
    case 3u:
    case 9u:
    case 14u:
        offset = 2u;
        break;
    case 2u:
        offset = 4u;
        break;
    case 8u:
    case 10u:
    case 11u:
    case 12u:
    case 13u:
        offset = 3u;
        break;
    default:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 7u);
        return;
    }
    target_position = ReadMeasurementWaitU32(FW_NEAR_UNKNOWN_FBA2) - offset;
    WriteMeasurementWaitU32(FW_NEAR_UNKNOWN_FBA6, target_position);
    PrepareReadMeasurementAtAdjustedTarget();
}

#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/read_measurement_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd9b81 */
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementAtAdjustedTarget(void)
{
    u8 retry_count;
    u8 saved_scale;
    u8 status;
    u8 controller_sample;
    u8 classification;
    u16 parameter_value;

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/read_measurement_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DD, 0u);
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0037, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEC, 0u);

    for(;;)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
        ADJUSTED_INITIALIZE_WAIT();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) == 1u)
            ADJUSTED_YIELD();

        ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0034, 1u);
        while(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0034) != 0u)
        {
            status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41);
            if(status != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEC, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBBD, status);
                ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0034, 0u);
                break;
            }

            if(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0033) == 0u)
            {
                if(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0037) != 1u)
                {
                    controller_sample = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018032);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_150A, controller_sample);
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_150A) & 0x20u) != 0u)
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 2u);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEC, 2u);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_14DD) + 1u));
                        ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0034, 0u);
                        break;
                    }
                }
            }
            else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0037) != 1u && ADJUSTED_OPERATING_WINDOW(1u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 2u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEC, 2u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_14DD) + 1u));
                ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0034, 0u);
                break;
            }

            if((s32)ReadMeasurementWaitU32(FW_NEAR_UNKNOWN_FBA6) < (s32)ReadMeasurementWaitU32(FW_NEAR_UNKNOWN_1235))
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 3u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEC, 3u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_14DD) + 1u));
                ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0034, 0u);
                break;
            }
            ADJUSTED_YIELD();
        }

        retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_14DD);
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && retry_count > 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 10u)
            ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0037, 1u);

        status = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0);
        if(retry_count > 3u || (FirmwareRead8(FW_HUGE_UNKNOWN_2BA5D8) == 2u && retry_count != 0u) || status == 1u)
        {
            if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 1u || ADJUSTED_ACCUMULATE_FLAGS() == 0u)
            {
                if(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0033) != 1u)
                    return;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x80u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 6u);
                WriteMeasurementWaitU32(FW_NEAR_UNKNOWN_DD51, ReadMeasurementWaitU32(FW_NEAR_UNKNOWN_FBA2));
                return;
            }
            ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_003A, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DD, 0u);
        }

        status = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0);
        if(status == 2u || status == 3u)
            continue;
        break;
    }

    saved_scale = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51);
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 10u)
        ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_0037, 1u);

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9F) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 0u)
            ADJUSTED_INITIALIZE_PHASE();

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 0u)
        {
            if(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_002B) != 0u && ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0044) != 0u && ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0038) != 0u)
            {
                FirmwareWriteSfr8(FW_SFR_UNKNOWN_E1, (u8)(FirmwareReadDirectSfr8(E1) & 0x7fu));
                FirmwareWriteDirectSfr8(E1, 1u);
                FirmwareWriteDirectSfr8(E4, 8u);
                FirmwareWriteDirectSfr8(F1, 0u);
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_102D) & 1u) == 1u)
                    ClearRxdatBit3Atomic();
                else
                    SetRxdatBit3Atomic();
                FirmwareWriteSfr8(FW_SFR_UNKNOWN_E1, (u8)(FirmwareReadDirectSfr8(E1) | 0x80u));
            }

            if(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0028) != 0u)
            {
                parameter_value = ReadMeasurementWaitU16(FW_NEAR_UNKNOWN_FBC3);
                if(parameter_value != ReadMeasurementWaitU16(FW_NEAR_UNKNOWN_FBC5) || ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_002E) != 1u)
                {
                    WriteMeasurementWaitU16(FW_NEAR_UNKNOWN_FBC5, parameter_value);
                    ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_002E, 1u);
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
                        ADJUSTED_WRITE_EF((u8)parameter_value);
                    else
                        ADJUSTED_WRITE_EAEB(parameter_value);
                }
            }
        }

        if(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0030) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) == 0u && ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0036) != 1u
            && (((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_1554) == 1u
            && ReadMeasurementWaitU16(FW_NEAR_UNKNOWN_1292) < 10u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1555, 1u);
            ADJUSTED_ACQUIRE_PREFIX();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1555, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
        }
        ADJUSTED_YIELD();
    }

    ADJUSTED_ACCUMULATE_WINDOW((u8)(saved_scale + FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51)));
    classification = FirmwareReadNear8(FW_NEAR_UNKNOWN_88B8);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 1u)
    {
        if(((classification == 0u || classification == 1u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 1u) || (classification == 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1147) == 0u)
            || (classification == 5u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1147) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF) == 0u)
            || (classification == 6u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF) == 0u))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x10u));
        }
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xefu));
    }

    ADJUSTED_WRITE_BIT(FW_BIT_UNKNOWN_002B, 0u);
    status = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9F);
    if(status == 0u)
    {
        if(ADJUSTED_READ_BIT(FW_BIT_UNKNOWN_0032) != 1u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
    }
    else if(status == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 4u);
    }
}
