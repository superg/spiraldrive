#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadControllerBufferByteAtomic(fw_addr_t buffer_offset);
#define ReadControllerBufferByteAtomic ObserveReadControllerBufferByteAtomic
#else
extern u8 ReadControllerBufferByteAtomic(fw_addr_t buffer_offset);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveCaptureInheritedDR12ForControllerRecordWorkspacePageCopy(void);
extern void ObserveRestoreInheritedDR12ForControllerRecordWorkspacePageCopy(u32 value);
#define CAPTURE_CONTROLLER_RECORD_PAGE_COPY_DR12() ObserveCaptureInheritedDR12ForControllerRecordWorkspacePageCopy()
#define RESTORE_CONTROLLER_RECORD_PAGE_COPY_DR12(value) ObserveRestoreInheritedDR12ForControllerRecordWorkspacePageCopy(value)
#else
extern u32 PLEXTOR_NEAR_FUNCTION CaptureInheritedDR12ForControllerRecordWorkspacePageCopy(void);
extern void PLEXTOR_NEAR_FUNCTION RestoreInheritedDR12ForControllerRecordWorkspacePageCopy(u32 value);
#define CAPTURE_CONTROLLER_RECORD_PAGE_COPY_DR12() CaptureInheritedDR12ForControllerRecordWorkspacePageCopy()
#define RESTORE_CONTROLLER_RECORD_PAGE_COPY_DR12(value) RestoreInheritedDR12ForControllerRecordWorkspacePageCopy(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
static u16 FirmwareReadBigEndianU16Full(fw_addr_t source_address)
{
    return FirmwareReadU16(source_address);
}
#else
#define FirmwareReadBigEndianU16Full(source_address) (*((volatile u16 huge *)(fw_addr_t)(source_address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU16Full(fw_addr_t destination_address, u16 value)
{
    FirmwareWriteU16(destination_address, value);
}
#else
#define FirmwareWriteBigEndianU16Full(destination_address, value) (*((volatile u16 huge *)(fw_addr_t)(destination_address)) = (u16)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if !defined(PLEXTOR_HOST_TEST)
extern u8 CaptureR7ForControllerBufferPageCopy(void);
extern u8 CaptureR6ForControllerBufferPageCopy(void);
extern fw_addr_t CaptureDR0ForControllerBufferPageCopy(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if !defined(PLEXTOR_HOST_TEST)
/* Native callers use DR0/ACC/R7/R6. C251 assigns the equivalent C
 * parameters to DR4/ACC/R3/R2, so capture every differing register before
 * ordinary C expression evaluation can reuse
 * it. */
write_marker = CaptureR6ForControllerBufferPageCopy();
page_index = CaptureR7ForControllerBufferPageCopy();
source_address = CaptureDR0ForControllerBufferPageCopy();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
/* Host models for native inherited-register/private-stack primitives. */
void PLEXTOR_NEAR_FUNCTION StoreR3AtSegmentRelativeE789PlusSelf(u8 value_and_index)
{
    FirmwareWrite8(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFE789) + value_and_index, value_and_index);
}

void CopyWordRangeAtOneMiBOffsets(fw_addr_t source, fw_addr_t destination, u32 word_count)
{
    fw_addr_t source_cursor = FirmwareAddressIncrement24(source, FW_ADDRESS_BANK_STRIDE);
    fw_addr_t destination_cursor = FirmwareAddressIncrement24(destination, FW_ADDRESS_BANK_STRIDE);

    while(word_count != 0)
    {
        FirmwareWriteBigEndianU16Full(destination_cursor, FirmwareReadBigEndianU16Full(source_cursor));
        source_cursor = FirmwareAddressIncrement24(source_cursor, 2);
        destination_cursor = FirmwareAddressIncrement24(destination_cursor, 2);
        word_count--;
    }
}
#endif
#endif
