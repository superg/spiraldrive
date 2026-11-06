#include "mode_page_initialization.h"
#include "../media/raw_read_runtime.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/mode_page_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fc2619 */
void PLEXTOR_NEAR_FUNCTION ApplyModePage05RuntimeImageToWriteState(void)
{
    u8 page_byte;
    u8 option_flags;

    MODE_PAGE_05_WRITE_TEST_BIT((u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_E641) & 0x40u) != 0u));

    page_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_E641);
    if((page_byte & 0x10u) != 0u)
    {
        option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(option_flags | 0x01u));
    }
    else
    {
        option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(option_flags & 0xfeu));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x02u) != 0u)
            MODE_PAGE_05_SNAPSHOT_TRACK_TAIL();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C44, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E641) & 0x0fu));
    page_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_E642);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C45, (u8)(page_byte & 0xc0u));

    page_byte = FirmwareReadNear8(FW_NEAR_UNKNOWN_E642);
    if((page_byte & 0x20u) != 0u)
    {
        option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(option_flags | 0x02u));
    }
    else
    {
        option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(option_flags & 0xfdu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
            MODE_PAGE_05_WRITE_TEST_BIT(1u);
    }

    page_byte = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E642) & 0x0fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C47, page_byte);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0x02u) != 0u)
    {
        option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(option_flags & 0xfbu));
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E642) & 0x10u) != 0u)
    {
        option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(option_flags | 0x04u));
    }
    else
    {
        option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(option_flags & 0xfbu));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C48, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E643) & 0x0fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C49, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E646) & 0x3fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4A, FirmwareReadNear8(FW_NEAR_UNKNOWN_E647));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4B, FirmwareReadNear8(FW_NEAR_UNKNOWN_E649));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4C, FirmwareReadNear8(FW_NEAR_UNKNOWN_E64A));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4D, FirmwareReadNear8(FW_NEAR_UNKNOWN_E64B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4E, FirmwareReadNear8(FW_NEAR_UNKNOWN_E64C));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4F, FirmwareReadNear8(FW_NEAR_UNKNOWN_E64D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C50, FirmwareReadNear8(FW_NEAR_UNKNOWN_E64E));
    MODE_PAGE_05_COPY(FW_NEAR_UNKNOWN_0C51, FW_NEAR_UNKNOWN_E64F, 0x10u);
    MODE_PAGE_05_COPY(FW_NEAR_UNKNOWN_0C61, FW_NEAR_UNKNOWN_E65F, 0x10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C71, FirmwareReadNear8(FW_NEAR_UNKNOWN_E66F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C72, FirmwareReadNear8(FW_NEAR_UNKNOWN_E670));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C73, FirmwareReadNear8(FW_NEAR_UNKNOWN_E671));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C74, FirmwareReadNear8(FW_NEAR_UNKNOWN_E672));
}

/* RAM:fc2a37 */
void InitializeModePageStateFromFirmwareDefaults(void)
{
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E623, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A0A), 0x0cu);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E62F, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A32), 0x10u);
    INITIALIZE_MODE_PAGE_COPY(FW_HUGE_UNKNOWN_2BC6EF, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A5A), 0x08u);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E63F, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8A9E), 0x34u);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E687, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B4A), 0x08u);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E68F, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B6A), 0x10u);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E673, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B0E), 0x08u);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E67B, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B2A), 0x0cu);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E69F, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B96), 0x0cu);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E6AB, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8BCC), 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6B7, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6B8, 0u);
    INITIALIZE_MODE_PAGE_COPY(FW_NEAR_UNKNOWN_E6EB, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8BCC), 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6F7, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6F8, 0u);

    INITIALIZE_MODE_PAGE_APPLY(0x01u, ApplyModePage01RuntimeState());
    INITIALIZE_MODE_PAGE_APPLY(0x02u, ApplyModePage02RuntimeTimingParameters());
    INITIALIZE_MODE_PAGE_APPLY(0x05u, ApplyModePage05RuntimeImageToWriteState());
    INITIALIZE_MODE_PAGE_APPLY(0x0du, UpdateMediaStateCountdownFromModePage0D());
    INITIALIZE_MODE_PAGE_APPLY(0x0eu, ApplyModePage0ERuntimeImageToRawReadState());
    INITIALIZE_MODE_PAGE_APPLY(0x07u, ApplyModePage07RuntimeState());
    INITIALIZE_MODE_PAGE_APPLY(0x1au, ApplyModePage1ASelectionState());
}
