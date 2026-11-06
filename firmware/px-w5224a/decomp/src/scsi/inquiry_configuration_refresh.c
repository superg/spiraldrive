#include "inquiry_configuration_refresh.h"
#include "../boot/image.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "spc.h"

extern u32 DivideU32Quotient(u32 dividend, u32 divisor);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/inquiry_configuration_refresh_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fda4eb */
void RefreshInquiryResponseFromDriveConfiguration(void)
{
    u8 index;
    u8 serial_valid = 1u;
    u32 serial_value;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/inquiry_configuration_refresh_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    READ_CONFIGURATION(0u, configuration_address, 0x20u);
    InitializeInquiryResponseBuffer();

    if(FirmwareRead8(configuration_address + 2u) >= (u8)'A' && FirmwareRead8(configuration_address + 2u) <= (u8)'Z')
        CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_DCCD, configuration_address + 2u, 8u);

    if(FirmwareRead8(configuration_address + 10u) >= (u8)'A' && FirmwareRead8(configuration_address + 10u) <= (u8)'Z')
        CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_DCD5, configuration_address + 10u, 16u);
    else if(FirmwareRead8(configuration_address + 10u) == (u8)' ' && FirmwareRead8(configuration_address + 11u) >= (u8)'A' && FirmwareRead8(configuration_address + 11u) <= (u8)'Z')
        CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_DCD5, configuration_address + 10u, 16u);

    for(index = 0u; index < 6u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD25 + index, (u8)' ');

    for(index = 0u; index < 5u; ++index)
    {
        if(FirmwareRead8(configuration_address + 26u + index) == 0xffu)
        {
            serial_valid = 0u;
            break;
        }
    }

    if(serial_valid != 0u)
    {
        serial_value = ParseUppercaseAsciiHexU32(5u, configuration_address + 26u);
        for(index = 0u; index < 6u; ++index)
        {
            u32 factor = PowerOfTenU32((u8)(5u - index));
            u32 quotient = DivideU32Quotient(serial_value, factor);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD25 + index, (u8)((u8)quotient + (u8)'0'));
            serial_value -= MultiplyU32Low(factor, quotient);
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x04u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7F, CompareFullInquiryProductIdentification());
    AppendConditionalInquiryExtensionBytes();

    READ_CONFIGURATION(0x46u, validity_address, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88BA, FirmwareRead8(validity_address) != 0u ? 1u : 0u);
}
