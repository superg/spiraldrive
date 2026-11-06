#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u16 ObserveDriveConfigurationState(u8 event, u32 argument0, u32 argument1);
#define READ_CONFIGURATION_WORD(index) ObserveDriveConfigurationState(DRIVE_CONFIGURATION_READ_WORD, (index), 0ul)
#define VALIDATE_STARTUP_INPUTS(enabled) ((void)ObserveDriveConfigurationState(DRIVE_CONFIGURATION_VALIDATE_STARTUP_INPUTS, (enabled), 0ul))
#define DELAY_TIMER_UNITS(units) ((void)ObserveDriveConfigurationState(DRIVE_CONFIGURATION_DELAY, (units), 0ul))

static u8 ReadSfrBit(u8 address, u8 mask)
{
    return (u8)((FirmwareReadSfr8(address) & mask) != 0u);
}

static void WriteSfrBit(u8 address, u8 mask, u8 value)
{
    u8 current = FirmwareReadSfr8(address);

    if(value != 0u)
        current = (u8)(current | mask);
    else
        current = (u8)(current & (u8)~mask);
    FirmwareWriteSfr8(address, current);
}

#define READ_P0_BIT1() ReadSfrBit(0x80u, 0x02u)
#define READ_P0_BIT2() ReadSfrBit(0x80u, 0x04u)
#define READ_P0_BIT6() ReadSfrBit(0x80u, 0x40u)
#define READ_P1_BIT6() ReadSfrBit(0x90u, 0x40u)
#define SET_P3_BIT0(value) WriteSfrBit(0xb0u, 0x01u, (value))
#define SET_P3_BIT1(value) WriteSfrBit(0xb0u, 0x02u, (value))
#else
sbit DriveConfigurationP0Bit1 = 0x81;
sbit DriveConfigurationP0Bit2 = 0x82;
sbit DriveConfigurationP0Bit6 = 0x86;
sbit DriveConfigurationP1Bit6 = 0x96;
sbit DriveConfigurationP3Bit0 = 0xb0;
sbit DriveConfigurationP3Bit1 = 0xb1;

#define READ_CONFIGURATION_WORD(index) ReadDriveConfigurationWord(index)
#define VALIDATE_STARTUP_INPUTS(enabled) ValidateStartupP0InputCombinationStable(enabled)
#define DELAY_TIMER_UNITS(units) DelayTimerUnits(units)
#define READ_P0_BIT1() DriveConfigurationP0Bit1
#define READ_P0_BIT2() DriveConfigurationP0Bit2
#define READ_P0_BIT6() DriveConfigurationP0Bit6
#define READ_P1_BIT6() DriveConfigurationP1Bit6
#define SET_P3_BIT0(value) (DriveConfigurationP3Bit0 = (value))
#define SET_P3_BIT1(value) (DriveConfigurationP3Bit1 = (value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(address, (u8)(value >> 8));
FirmwareWriteNear8((u16)(address + 1u), (u8)value);
#else
*((volatile u16 near *)address) = value;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
return ((u32)FirmwareReadNear8(address) << 24) | ((u32)FirmwareReadNear8((u16)(address + 1u)) << 16) | ((u32)FirmwareReadNear8((u16)(address + 2u)) << 8) | FirmwareReadNear8((u16)(address + 3u));
#else
return *((volatile u32 near *)address);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(address, (u8)(value >> 24));
FirmwareWriteNear8((u16)(address + 1u), (u8)(value >> 16));
FirmwareWriteNear8((u16)(address + 2u), (u8)(value >> 8));
FirmwareWriteNear8((u16)(address + 3u), (u8)value);
#else
*((volatile u32 near *)address) = value;
#endif
#endif
