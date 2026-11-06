#include "controller_event_dispatch.h"
#include "../media/controller_event_bit7_progress.h"
#include "../media/controller_read_event.h"
#include "../media/raw_read_runtime.h"
#include "controller_event_bit2_progress.h"
#include "controller_packet_capture.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_event_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd10e9 */
void HandleControllerEventInterrupt(void)
{
    u8 event_mask;
    u8 source_status;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807E, 0u);
    event_mask = (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018003) & 0xfeu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_122A, event_mask);
    source_status = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01807F);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1097, source_status);

    if((source_status & 0x08u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802E, 8u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) & 0x08u) != 0u)
            CompleteControllerOperatingResponse();
    }
    if((source_status & 0x04u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806F, 0x20u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) & 0x04u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) & 0xfbu));
            FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C0, 1u);
        }
    }
    if((source_status & 0x02u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806F, 0x10u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) & 0x02u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) & 0xfdu));
            FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C1, 1u);
        }
    }

    if((event_mask & 0x40u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x40u);
        HandleControllerEvent40Completion();
    }
    if((event_mask & 0x08u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F78, 0x50u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x08u);
        HandleControllerEventBit3Completion();
    }
    if((event_mask & 0x20u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x20u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0x20u) != 0u)
            ProcessControllerReadEvent();
    }
    if((event_mask & 0x10u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x10u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0x10u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xefu));
            CLEAR_CONTROLLER_EVENT_BIT43();
            PrepareControllerEventBit4OperatingUpdate();
            FinalizeControllerEventBit4OperatingUpdate();
        }
    }
    if((event_mask & 0x04u) == 0u)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) < 0)
            PublishControllerOperatingResponse();
        if(READ_CONTROLLER_EVENT_BIT43() != 0u)
            HandleControllerEventBit2ReadMeasurementProgress();
        ApplyControllerRegisterBlock34And36Through3F();
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 4u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 4u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xfbu));
            HandleReadMeasurementTargetPacketMatch();
            CaptureControllerThreeBytePacket();
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 1u) == 1u)
                HandleControllerEventBit2MeasurementService();
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 1u) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 4u));
        }
    }
    if((event_mask & 0x02u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 2u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 2u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xfdu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x20u));
        }
    }
    if((event_mask & 0x80u) == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 1u);
        HandleControllerEventBit7TransferProgress();
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807E, FirmwareReadNear8(FW_NEAR_UNKNOWN_1038));
}
