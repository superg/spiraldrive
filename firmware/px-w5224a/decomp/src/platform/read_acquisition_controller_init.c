#include "read_acquisition_controller_init.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/read_acquisition_controller_init_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void PublishReadAcquisitionControllerShadow(u8 mask, u8 bits)
{
    u8 value = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & mask) | bits);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, value);
}

/* RAM:fdbc83 */
void InitializeReadAcquisitionControllerRegisters(void)
{
    DISABLE_GLOBAL_INTERRUPT();
    PublishReadAcquisitionControllerShadow(0xe7u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x7fu);
    PublishReadAcquisitionControllerShadow(0xffu, 0x08u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1158) == 5u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018031, 0x40u);

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, 0x41u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, 0xa0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0x83u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806E, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800F, 4u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800C, 0x7fu);

    PublishReadAcquisitionControllerShadow(0xe7u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 0u);
    PublishReadAcquisitionControllerShadow(0xffu, 0x08u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800B, 0xd2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018001, 0x71u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018023, 0x10u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800C, 0xffu);

    PublishReadAcquisitionControllerShadow(0xe7u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFD, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018007, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, 6u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, 0x60u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0x60u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801C, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801D, 0u);

    ENABLE_GLOBAL_INTERRUPT();
    ENABLE_CONTROLLER_INTERRUPT();
    SET_SFR_A9_BIT2();
}
