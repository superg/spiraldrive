#ifndef PLEXTOR_FIRMWARE_MEMORY_H
#define PLEXTOR_FIRMWARE_MEMORY_H

#include "firmware_code_addresses.h"
#include "firmware_external_addresses.h"
#include "firmware_map.h"
#include "firmware_rom_data.h"
#include "mcs251.h"

/* Reconstruction helper; no direct Ghidra function address.
 * Wrap only the low sixteen address bits, preserving the 24-bit bank. */
#define FirmwareAddressIncrementLow16(address, increment) (((fw_addr_t)(address) & ~(fw_addr_t)0xffffu) | (fw_addr_t)(u16)((u16)(address) + (u16)(increment)))

/* Reconstruction helper; no direct Ghidra function address.
 * Native DR increments retain 24 bits and wrap at the CPU address-space edge. */
#define FirmwareAddressIncrement24(address, increment) (((fw_addr_t)(address) + (fw_addr_t)(increment)) & FW_24BIT_ADDRESS_MASK)

/* Zero-cost typed and read/modify/write helpers.  The address argument of a
 * bit helper must be a stable named field because it is evaluated twice. */
#define FirmwareReadS32(address) ((s32)FirmwareReadU32(address))
#define FirmwareSetNearBits8(address, bits) FirmwareWriteNear8((address), (u8)(FirmwareReadNear8(address) | (bits)))
#define FirmwareClearNearBits8(address, clear_mask) FirmwareWriteNear8((address), (u8)(FirmwareReadNear8(address) & (clear_mask)))

#if defined(PLEXTOR_HOST_TEST)
u8 FirmwareReadBit0004(void);
u8 FirmwareReadBit000B(void);
u8 FirmwareReadBit0011(void);
u8 FirmwareReadBit0013(void);
u8 FirmwareReadBit001B(void);
u8 FirmwareReadBit0008(void);
u8 FirmwareReadBit000E(void);
u8 FirmwareReadBit001A(void);
u8 FirmwareReadBit0407(void);
u8 FirmwareReadBit0020(void);
u8 FirmwareReadBit0024(void);
u8 FirmwareReadBit002B(void);
u8 FirmwareReadBit0027(void);
void FirmwareWriteBit0026(u8 value);
void FirmwareWriteBit0027(u8 value);
u8 FirmwareReadBit0031(void);
u8 FirmwareReadBit0030(void);
u8 FirmwareReadBit0028(void);
u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0032(void);
void FirmwareWriteBit0032(u8 value);
u8 FirmwareReadBit0034(void);
u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0036(void);
u8 FirmwareReadBit0014(void);
u8 FirmwareReadBit0580(void);
u8 FirmwareReadBit0581(void);
u8 FirmwareReadBit0546(void);
u8 FirmwareReadBit0547(void);
u8 FirmwareReadBit0015(void);
void FirmwareWriteBit0012(u8 value);
void FirmwareWriteBit000A(u8 value);
void FirmwareWriteBit000C(u8 value);
void FirmwareWriteBit000E(u8 value);
void FirmwareWriteBit001A(u8 value);
void FirmwareWriteBit0014(u8 value);
void FirmwareWriteBit0580(u8 value);
void FirmwareWriteBit0581(u8 value);
void FirmwareWriteBit0546(u8 value);
void FirmwareWriteBit0547(u8 value);
void FirmwareWriteBit0480(u8 value);
void FirmwareWriteBit0481(u8 value);
void FirmwareWriteBit0015(u8 value);
void FirmwareWriteBit0016(u8 value);
void FirmwareWriteBit0017(u8 value);
void FirmwareWriteBit0009(u8 value);
void FirmwareWriteBit0004(u8 value);
void FirmwareWriteBit0024(u8 value);
void FirmwareWriteBit0008(u8 value);
void FirmwareWriteBit003D(u8 value);
void FirmwareWriteBit003E(u8 value);
void FirmwareWriteBit003F(u8 value);
void FirmwareWriteBit0040(u8 value);
void FirmwareWriteBit0542(u8 value);
void FirmwareWriteBit0036(u8 value);
void FirmwareWriteBit0034(u8 value);
void FirmwareWriteBit0043(u8 value);
void FirmwareWriteBit0039(u8 value);
void FirmwareWriteBit003A(u8 value);
void FirmwareWriteBit003B(u8 value);
void FirmwareWriteBit000F(u8 value);
void FirmwareWriteBit0022(u8 value);
void FirmwareWriteBit0023(u8 value);
u8 FirmwareRead8(fw_addr_t address);
void FirmwareWrite8(fw_addr_t address, u8 value);
u8 FirmwareReadSfr8(u8 address);
void FirmwareWriteSfr8(u8 address, u8 value);
#define FirmwareReadController8(address) FirmwareRead8((fw_addr_t)(address))
#define FirmwareWriteController8(address, value) FirmwareWrite8((fw_addr_t)(address), (value))
#define FirmwareReadNear8(address) FirmwareRead8((fw_addr_t)(address))
#define FirmwareWriteNear8(address, value) FirmwareWrite8((fw_addr_t)(address), (value))
#define FirmwareReadNearDisplaced8(address, displacement) FirmwareRead8((fw_addr_t)(u16)(address) + (fw_addr_t)(displacement))
static inline u16 FirmwareReadNearU16(u16 address)
{
    u16 value = (u16)FirmwareRead8(address) << 8;

    value |= FirmwareRead8((u16)(address + 1u));
    return value;
}
static inline u32 FirmwareReadNearU32(u16 address)
{
    u32 value = (u32)FirmwareRead8(address) << 24;

    value |= (u32)FirmwareRead8((u16)(address + 1u)) << 16;
    value |= (u32)FirmwareRead8((u16)(address + 2u)) << 8;
    value |= FirmwareRead8((u16)(address + 3u));
    return value;
}
static inline void FirmwareWriteNearU16(u16 address, u16 value)
{
    FirmwareWrite8(address, (u8)(value >> 8));
    FirmwareWrite8((u16)(address + 1u), (u8)value);
}
static inline void FirmwareWriteNearU32(u16 address, u32 value)
{
    FirmwareWrite8(address, (u8)(value >> 24));
    FirmwareWrite8((u16)(address + 1u), (u8)(value >> 16));
    FirmwareWrite8((u16)(address + 2u), (u8)(value >> 8));
    FirmwareWrite8((u16)(address + 3u), (u8)value);
}
static inline u16 FirmwareReadU16(fw_addr_t address)
{
    u16 value = (u16)FirmwareRead8(address) << 8;

    value |= FirmwareRead8(FirmwareAddressIncrement24(address, 1u));
    return value;
}
static inline u32 FirmwareReadU32(fw_addr_t address)
{
    u32 value = (u32)FirmwareRead8(address) << 24;

    value |= (u32)FirmwareRead8(FirmwareAddressIncrement24(address, 1u)) << 16;
    value |= (u32)FirmwareRead8(FirmwareAddressIncrement24(address, 2u)) << 8;
    value |= FirmwareRead8(FirmwareAddressIncrement24(address, 3u));
    return value;
}
static inline void FirmwareWriteU16(fw_addr_t address, u16 value)
{
    FirmwareWrite8(address, (u8)(value >> 8));
    FirmwareWrite8(FirmwareAddressIncrement24(address, 1u), (u8)value);
}
static inline void FirmwareWriteU32(fw_addr_t address, u32 value)
{
    FirmwareWrite8(address, (u8)(value >> 24));
    FirmwareWrite8(FirmwareAddressIncrement24(address, 1u), (u8)(value >> 16));
    FirmwareWrite8(FirmwareAddressIncrement24(address, 2u), (u8)(value >> 8));
    FirmwareWrite8(FirmwareAddressIncrement24(address, 3u), (u8)value);
}
#define PLEXTOR_SFR_ADDRESS_(address) 0x##address
#define PLEXTOR_SFR_ADDRESS(address) PLEXTOR_SFR_ADDRESS_(address)
#define FirmwareReadDirectSfr8(address) FirmwareReadSfr8(PLEXTOR_SFR_ADDRESS(address))
#define FirmwareWriteDirectSfr8(address, value) FirmwareWriteSfr8(PLEXTOR_SFR_ADDRESS(address), (value))
#else
/* Reconstruction helper; no direct Ghidra function address.
 * The target build needs the LC898095 full 24-bit HDATA access form. */
#define FirmwareRead8(address) (*((volatile u8 huge *)(address)))
#define FirmwareWrite8(address, value) (*((volatile u8 huge *)(address)) = (value))
#define FirmwareReadSfr8(address) (*((volatile u8 near *)(address)))
#define FirmwareWriteSfr8(address, value) (*((volatile u8 near *)(address)) = (value))
#define PLEXTOR_SFR_NAME_(address) FirmwareSfr##address
#define PLEXTOR_SFR_NAME(address) PLEXTOR_SFR_NAME_(address)
#define FirmwareReadDirectSfr8(address) PLEXTOR_SFR_NAME(address)
#define FirmwareWriteDirectSfr8(address, value) (PLEXTOR_SFR_NAME(address) = (value))
sfr FirmwareSfr80 = 0x80;
sfr FirmwareSfr85 = 0x85;
sfr FirmwareSfr86 = 0x86;
sfr FirmwareSfr88 = 0x88;
sfr FirmwareSfr8A = 0x8a;
sfr FirmwareSfr8B = 0x8b;
sfr FirmwareSfr8C = 0x8c;
sfr FirmwareSfr8D = 0x8d;
sfr FirmwareSfr90 = 0x90;
sfr FirmwareSfr91 = 0x91;
sfr FirmwareSfr94 = 0x94;
sfr FirmwareSfr96 = 0x96;
sfr FirmwareSfrA8 = 0xa8;
sfr FirmwareSfrA9 = 0xa9;
sfr FirmwareSfrB0 = 0xb0;
sfr FirmwareSfrD1 = 0xd1;
sfr FirmwareSfrD2 = 0xd2;
sfr FirmwareSfrD5 = 0xd5;
sfr FirmwareSfrD6 = 0xd6;
sfr FirmwareSfrD7 = 0xd7;
sfr FirmwareSfrD8 = 0xd8;
sfr FirmwareSfrD9 = 0xd9;
sfr FirmwareSfrDA = 0xda;
sfr FirmwareSfrDB = 0xdb;
sfr FirmwareSfrE1 = 0xe1;
sfr FirmwareSfrE4 = 0xe4;
sfr FirmwareSfrF1 = 0xf1;
/* LC898095 ports occupy external-data offsets 0x8000+. The original uses
 * MOVX @DPTR for this window; DPX supplies the established 0x01 bank. */
#define FirmwareReadController8(address) (*((volatile u8 xdata *)(u16)(address)))
#define FirmwareWriteController8(address, value) (*((volatile u8 xdata *)(u16)(address)) = (value))
/* Main working RAM occupies physical bank 0x00. The allocation-free absolute
 * base exported by firmware_ram_window.a51 lets constant indices compile to
 * the original direct-address form; variable indices retain near semantics. */
extern volatile u8 near FirmwareRamWindow[0x10000ul];
#define FirmwareReadNear8(address) (FirmwareRamWindow[(u16)(address)])
#define FirmwareWriteNear8(address, value) (FirmwareRamWindow[(u16)(address)] = (value))
#define FirmwareReadNearDisplaced8(address, displacement) (*((volatile u8 near *)(u16)(address) + (displacement)))
#define FirmwareReadNearU16(address) (*((volatile u16 near *)(u16)(address)))
#define FirmwareReadNearU32(address) (*((volatile u32 near *)(u16)(address)))
#define FirmwareWriteNearU16(address, value) (*((volatile u16 near *)(u16)(address)) = (value))
#define FirmwareWriteNearU32(address, value) (*((volatile u32 near *)(u16)(address)) = (value))
#define FirmwareReadU16(address) (*((volatile u16 huge *)(fw_addr_t)(address)))
#define FirmwareReadU32(address) (*((volatile u32 huge *)(fw_addr_t)(address)))
#define FirmwareWriteU16(address, value) (*((volatile u16 huge *)(fw_addr_t)(address)) = (value))
#define FirmwareWriteU32(address, value) (*((volatile u32 huge *)(fw_addr_t)(address)) = (value))
u8 FirmwareReadBit0004(void);
u8 FirmwareReadBit000B(void);
u8 FirmwareReadBit0011(void);
u8 FirmwareReadBit0013(void);
u8 FirmwareReadBit001B(void);
u8 FirmwareReadBit0008(void);
u8 FirmwareReadBit000E(void);
u8 FirmwareReadBit001A(void);
u8 FirmwareReadBit0407(void);
u8 FirmwareReadBit0020(void);
u8 FirmwareReadBit0024(void);
u8 FirmwareReadBit002B(void);
u8 FirmwareReadBit0027(void);
u8 FirmwareReadBit0031(void);
u8 FirmwareReadBit0030(void);
u8 FirmwareReadBit0028(void);
u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0032(void);
u8 FirmwareReadBit0034(void);
u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0036(void);
u8 FirmwareReadBit0014(void);
u8 FirmwareReadBit0017(void);
u8 FirmwareReadBit0021(void);
void FirmwareWriteBit000A(u8 value);
void FirmwareWriteBit0008(u8 value);
void FirmwareWriteBit000E(u8 value);
void FirmwareWriteBit000F(u8 value);
void FirmwareWriteBit001A(u8 value);
void FirmwareWriteBit000C(u8 value);
void FirmwareWriteBit003D(u8 value);
void FirmwareWriteBit0034(u8 value);
void FirmwareWriteBit0043(u8 value);
void FirmwareWriteBit003E(u8 value);
void FirmwareWriteBit0040(u8 value);
void FirmwareWriteBit0016(u8 value);
void FirmwareWriteBit0017(u8 value);
void FirmwareWriteBit0009(u8 value);
void FirmwareWriteBit0004(u8 value);
void FirmwareWriteBit0024(u8 value);
void FirmwareWriteBit0022(u8 value);
void FirmwareWriteBit0026(u8 value);
void FirmwareWriteBit0580(u8 value);
void FirmwareWriteBit0581(u8 value);
#endif

#endif
