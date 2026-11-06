#include "vendor_f2_shared.h"
#include "platform/firmware_memory.h"
#include "platform/memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f2_shared_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadVendorF2Word(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadVendorF2Dword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static float ReadVendorF2Float(fw_addr_t address)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.bits = ReadVendorF2Dword(address);
    return converted.value;
}

static void WriteVendorF2Word(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:fbdcde */
void PLEXTOR_NEAR_FUNCTION BuildVendorF2SharedMeasurementResponseFields(void)
{
    s8 table_value;
    float measurement_value;
    float narrowed;
    double value;
    s32 rounded;
    u16 reference_word;
    s16 reference_delta;

    VENDOR_F2_COPY(FW_NEAR_UNKNOWN_F11B, FW_NEAR_UNKNOWN_0BB0, 0x1eu);
    VENDOR_F2_COPY(FW_NEAR_UNKNOWN_F139, FW_NEAR_UNKNOWN_0BCE, 0x1eu);
    WriteVendorF2Word(FW_NEAR_UNKNOWN_0BEE, ReadVendorF2Word(FW_NEAR_UNKNOWN_F060));
    WriteVendorF2Word(FW_NEAR_UNKNOWN_0BF2, ReadVendorF2Word(FW_NEAR_UNKNOWN_F062));
    WriteVendorF2Word(FW_NEAR_UNKNOWN_0BF6, ReadVendorF2Word(FW_NEAR_UNKNOWN_F064));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 3u)
    {
        WriteVendorF2Word(FW_NEAR_UNKNOWN_0BFA, ReadVendorF2Word(FW_NEAR_UNKNOWN_EF85));
        WriteVendorF2Word(FW_NEAR_UNKNOWN_0BFC, ReadVendorF2Word(FW_NEAR_UNKNOWN_EF87));
        return;
    }

    table_value = (s8)FirmwareRead8(FirmwareAddressIncrementLow16(ReadVendorF2Dword(FW_NEAR_UNKNOWN_F1BB), 4u));
    measurement_value = ReadVendorF2Float(FW_NEAR_UNKNOWN_F06E);

    value = (double)table_value * 0.001;
    value *= (double)measurement_value;
    narrowed = (float)value;
    value = (double)narrowed + 4.6592;
    narrowed = (float)value;
    value = (double)narrowed / 0.0043;
    rounded = (s32)VENDOR_F2_CEIL(value);
    reference_word = (u16)((u16)rounded + ReadVendorF2Word(FW_NEAR_UNKNOWN_F074));
    reference_delta = (s16)(reference_word - ReadVendorF2Word(FW_NEAR_UNKNOWN_F074));

    value = ((double)measurement_value - (double)reference_delta * 0.001) + 12.0;
    value /= ((double)reference_delta * 0.0089 + 14.3) * 0.01;
    value += (double)FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A);
    narrowed = (float)value;
    rounded = (s32)VENDOR_F2_CEIL((double)narrowed);

    WriteVendorF2Word(FW_NEAR_UNKNOWN_0BFA, (u16)(u8)rounded);
    WriteVendorF2Word(FW_NEAR_UNKNOWN_0BFC, reference_word);
}
