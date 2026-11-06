#include "controller_asic_initialization.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "timing.h"

static void WriteBigEndianU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void WriteControllerAddress24(fw_addr_t port, u32 address)
{
    FirmwareWriteController8(port, (u8)address);
    FirmwareWriteController8(port + 1u, (u8)(address >> 8));
    FirmwareWriteController8(port + 2u, (u8)(address >> 16));
}

static void PublishControllerShadow(u8 mask, u8 bits)
{
    u8 shadow = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & mask) | bits);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
}

/* RAM:fdb787 */
void InitializeControllerAsic(void)
{
    u8 saved_port_8040 = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018040);
    u8 saved_port_803f;
    u32 region_b_base;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1157, saved_port_8040);
    saved_port_803f = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01803F);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1158, saved_port_803f);

    WriteBigEndianU32(FW_NEAR_UNKNOWN_155E, 0x00126000ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1562, 0x00126100ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1566, 0x0014ae00ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_156A, 0x00000200ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_156E, 0x00156f00ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_155A, 0x00160000ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1556, FW_HUGE_UNKNOWN_1B0000);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_1552, 0x0082u);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1572, 0x000b5000ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_1576, 0x000d9c00ul);

    PublishControllerShadow(0xffu, 0x08u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018031, saved_port_803f == 5u ? 0x40u : 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800B, 0xd2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018001, 0x71u);
    if(saved_port_8040 == 2u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803C, 0x0du);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803D, 0x60u);
    }
    else
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803C, 1u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803D, 0x10u);
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, 0x41u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, 0xa0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0x42u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806E, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x50u);
    DelayTimerUnits(12u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803E, 0x27u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018023, 0x10u);
    DelayTimerUnits(9u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803B, 0x88u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, 0x62u);

    PublishControllerShadow(0xe7u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x7fu);
    WriteControllerRegister01Clamped(1u);
    PublishControllerShadow(0xffu, 0x08u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, 0xa0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0x83u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806E, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800B, 0xd2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018001, 0x71u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800F, 4u);

    WriteControllerAddress24(FW_CONTROLLER_UNKNOWN_018010, 0u);
    WriteControllerAddress24(FW_CONTROLLER_UNKNOWN_018013, 0x00126000ul);
    WriteControllerAddress24(FW_CONTROLLER_UNKNOWN_01802A, 0x000b5000ul);
    region_b_base = 0x000d9c00ul;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, (u8)region_b_base);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802F, (u8)(region_b_base >> 16));
    WriteControllerAddress24(FW_CONTROLLER_UNKNOWN_018016, 0x00126100ul);
    WriteControllerAddress24(FW_CONTROLLER_UNKNOWN_018019, 0x0014ae00ul);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801C, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801D, 2u);
    WriteControllerAddress24(FW_CONTROLLER_UNKNOWN_01801F, 0x00156f00ul);

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0xe0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018023, 0x10u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018048, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018040, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018043, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0xffu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018042, 1u);

    PublishControllerShadow(0xe7u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFD, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802A, 3u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    WriteControllerAddress24(FW_CONTROLLER_UNKNOWN_018007, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, 6u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, 0x60u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0x60u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801C, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801D, 0u);

    PublishControllerShadow(0xffu, 0x08u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01804B, 0x2au);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, 8u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x82u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018007, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, 0x88u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BB, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018078, 0x20u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018079, 0x10u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807A, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807B, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807C, 0x57u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807D, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807E, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, 0u);

    PublishControllerShadow(0xe7u, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802E, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807E, 0u);

    PublishControllerShadow(0xffu, 0x08u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C1, 0x14u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C2, 0xe1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, 0x60u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C6, 0x23u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, saved_port_8040 == 2u ? 4u : 0u);
    PublishControllerShadow(0xe7u, 0u);
}
