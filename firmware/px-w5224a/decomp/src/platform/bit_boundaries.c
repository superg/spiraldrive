#include "mcs251.h"

/* Addressless C251 direct-bit and atomic boundaries. */
u8 bdata BoundaryBitByte20 _at_ 0x20;
u8 bdata BoundaryBitByte21 _at_ 0x21;
u8 bdata BoundaryBitByte22 _at_ 0x22;
u8 bdata BoundaryBitByte23 _at_ 0x23;
u8 bdata BoundaryBitByte24 _at_ 0x24;
u8 bdata BoundaryBitByte25 _at_ 0x25;
u8 bdata BoundaryBitByte26 _at_ 0x26;
u8 bdata BoundaryBitByte27 _at_ 0x27;
u8 bdata BoundaryBitByte28 _at_ 0x28;
sbit BoundaryBit0001 = BoundaryBitByte20 ^ 1;
sbit BoundaryBit0002 = BoundaryBitByte20 ^ 2;
sbit BoundaryBit0003 = BoundaryBitByte20 ^ 3;
sbit BoundaryBit0004 = BoundaryBitByte20 ^ 4;
sbit BoundaryBit0005 = BoundaryBitByte20 ^ 5;
sbit BoundaryBit0007 = BoundaryBitByte20 ^ 7;
sbit BoundaryBit0008 = BoundaryBitByte21 ^ 0;
sbit BoundaryBit0009 = BoundaryBitByte21 ^ 1;
sbit BoundaryBit000A = BoundaryBitByte21 ^ 2;
sbit BoundaryBit000B = BoundaryBitByte21 ^ 3;
sbit BoundaryBit000C = BoundaryBitByte21 ^ 4;
sbit BoundaryBit000D = BoundaryBitByte21 ^ 5;
sbit BoundaryBit000E = BoundaryBitByte21 ^ 6;
sbit BoundaryBit000F = BoundaryBitByte21 ^ 7;
sbit BoundaryBit0010 = BoundaryBitByte22 ^ 0;
sbit BoundaryBit0011 = BoundaryBitByte22 ^ 1;
sbit BoundaryBit0012 = BoundaryBitByte22 ^ 2;
sbit BoundaryBit0013 = BoundaryBitByte22 ^ 3;
sbit BoundaryBit0014 = BoundaryBitByte22 ^ 4;
sbit BoundaryBit0015 = BoundaryBitByte22 ^ 5;
sbit BoundaryBit0016 = BoundaryBitByte22 ^ 6;
sbit BoundaryBit0017 = BoundaryBitByte22 ^ 7;
sbit BoundaryBit0018 = BoundaryBitByte23 ^ 0;
sbit BoundaryBit0019 = BoundaryBitByte23 ^ 1;
sbit BoundaryBit001A = BoundaryBitByte23 ^ 2;
sbit BoundaryBit001B = BoundaryBitByte23 ^ 3;
sbit BoundaryBit001C = BoundaryBitByte23 ^ 4;
sbit BoundaryBit001D = BoundaryBitByte23 ^ 5;
sbit BoundaryBit001E = BoundaryBitByte23 ^ 6;
sbit BoundaryBit001F = BoundaryBitByte23 ^ 7;
sbit BoundaryBit0020 = BoundaryBitByte24 ^ 0;
sbit BoundaryBit0021 = BoundaryBitByte24 ^ 1;
sbit BoundaryBit0022 = BoundaryBitByte24 ^ 2;
sbit BoundaryBit0023 = BoundaryBitByte24 ^ 3;
sbit BoundaryBit0024 = BoundaryBitByte24 ^ 4;
sbit BoundaryBit0026 = BoundaryBitByte24 ^ 6;
sbit BoundaryBit0027 = BoundaryBitByte24 ^ 7;
sbit BoundaryBit0028 = BoundaryBitByte25 ^ 0;
sbit BoundaryBit0029 = BoundaryBitByte25 ^ 1;
sbit BoundaryBit002B = BoundaryBitByte25 ^ 3;
sbit BoundaryBit002C = BoundaryBitByte25 ^ 4;
sbit BoundaryBit002E = BoundaryBitByte25 ^ 6;
sbit BoundaryBit002F = BoundaryBitByte25 ^ 7;
sbit BoundaryBit0030 = BoundaryBitByte26 ^ 0;
sbit BoundaryBit0031 = BoundaryBitByte26 ^ 1;
sbit BoundaryBit0032 = BoundaryBitByte26 ^ 2;
sbit BoundaryBit0033 = BoundaryBitByte26 ^ 3;
sbit BoundaryBit0034 = BoundaryBitByte26 ^ 4;
sbit BoundaryBit0035 = BoundaryBitByte26 ^ 5;
sbit BoundaryBit0036 = BoundaryBitByte26 ^ 6;
sbit BoundaryBit0037 = BoundaryBitByte26 ^ 7;
sbit BoundaryBit0038 = BoundaryBitByte27 ^ 0;
sbit BoundaryBit0039 = BoundaryBitByte27 ^ 1;
sbit BoundaryBit003A = BoundaryBitByte27 ^ 2;
sbit BoundaryBit003B = BoundaryBitByte27 ^ 3;
sbit BoundaryBit003C = BoundaryBitByte27 ^ 4;
sbit BoundaryBit003D = BoundaryBitByte27 ^ 5;
sbit BoundaryBit003E = BoundaryBitByte27 ^ 6;
sbit BoundaryBit003F = BoundaryBitByte27 ^ 7;
sbit BoundaryBit0040 = BoundaryBitByte28 ^ 0;
sbit BoundaryBit0043 = BoundaryBitByte28 ^ 3;
sbit BoundaryBit0044 = BoundaryBitByte28 ^ 4;
sbit BoundaryBit0046 = BoundaryBitByte28 ^ 6;
sbit BoundaryBit0080 = 0x80;
sbit BoundaryBit0081 = 0x81;
sbit BoundaryBit0082 = 0x82;
sbit BoundaryBit0083 = 0x83;
sbit BoundaryBit0085 = 0x85;
sbit BoundaryBit0086 = 0x86;
sbit BoundaryBit0087 = 0x87;
sbit BoundaryBit0089 = 0x89;
sbit BoundaryBit0090 = 0x90;
sbit BoundaryBit0091 = 0x91;
sbit BoundaryBit0093 = 0x93;
sbit BoundaryBit00A8 = 0xa8;
sbit BoundaryBit00A9 = 0xa9;
sbit BoundaryBit00AA = 0xaa;
sbit BoundaryBit00AB = 0xab;
sbit BoundaryBit00AE = 0xae;
sbit BoundaryBit00AF = 0xaf;
sbit BoundaryBit00B0 = 0xb0;
sbit BoundaryBit00B1 = 0xb1;
extern void SetSfrA9Bit2Extended(void);

u8 FirmwareReadBit0014(void)
{
    return BoundaryBit0014 != 0 ? 1u : 0u;
}

u8 FirmwareReadBit0017(void)
{
    return BoundaryBit0017 != 0 ? 1u : 0u;
}

u8 FirmwareReadBit0021(void)
{
    return BoundaryBit0021 != 0 ? 1u : 0u;
}

void FirmwareWriteBit0008(u8 value)
{
    BoundaryBit0008 = value != 0u;
}

void FirmwareWriteBit000E(u8 value)
{
    BoundaryBit000E = value != 0u;
}

void FirmwareWriteBit000F(u8 value)
{
    BoundaryBit000F = value != 0u;
}

void FirmwareWriteBit0022(u8 value)
{
    BoundaryBit0022 = value != 0u;
}

void FirmwareWriteBit0580(u8 value)
{
    BoundaryBit00B0 = value != 0u;
}

void FirmwareWriteBit0581(u8 value)
{
    BoundaryBit00B1 = value != 0u;
}

void SetSfrA9Bit2Atomic(void)
{
    SetSfrA9Bit2Extended();
}

void PLEXTOR_NEAR_FUNCTION ClearBit0036Atomic(void)
{
    BoundaryBit0036 = 0;
    return;
}

void ClearBit0030Atomic(void)
{
    BoundaryBit0030 = 0;
    return;
}

void SetSyntheticPatternActiveBitsAtomic(void)
{
    BoundaryBit0030 = 1;
    BoundaryBit0035 = 1;
    return;
}

void ClearSyntheticPatternActiveBitsAtomic(void)
{
    BoundaryBit0030 = 0;
    BoundaryBit0035 = 0;
    return;
}

u8 ReadBit0038ForSecondaryControllerProfileUpdate(void)
{
    u8 accumulator = 0u;
    if(BoundaryBit0038 == 0)
        goto ReadBit0038ForSecondaryControllerProfileUpdate_Clear;
    accumulator = 1u;
    return accumulator;
ReadBit0038ForSecondaryControllerProfileUpdate_Clear:;
    accumulator = 0u;
    return accumulator;
}

void ClearReadTrackInformationRawModeBitsAtomic(void)
{
    BoundaryBit001B = 0;
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0038Atomic(void)
{
    u8 accumulator = 0u;
    if(BoundaryBit0038 == 0)
        goto ReadBit0038Atomic_Clear;
    accumulator = 1u;
    return accumulator;
ReadBit0038Atomic_Clear:;
    accumulator = 0u;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0033Atomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0033;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0037ForAdjustedTarget(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0037;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit0037ForAdjustedTarget(void)
{
    BoundaryBit0037 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0037ForAdjustedTarget(void)
{
    BoundaryBit0037 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit002EForAdjustedTarget(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit002E;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit002EForAdjustedTarget(void)
{
    BoundaryBit002E = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit002BForAdjustedTarget(void)
{
    BoundaryBit002B = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0029ForReadAcquisitionCoroutine3(void)
{
    BoundaryBit0029 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0029ForReadAcquisitionCoroutine3(void)
{
    BoundaryBit0029 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP3RxHighTxLowForReadAcquisitionCoroutine3(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP3RxHighTxLowForReadAcquisitionDispatch(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP3RxLowTxHighForReadAcquisitionCoroutine3(void)
{
    BoundaryBit00B0 = 0;
    BoundaryBit00B1 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0030ForReadAcquisitionCoroutine3(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0030;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0030ForReadAcquisitionCoroutine3(void)
{
    BoundaryBit0030 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0031ForReadAcquisitionCoroutine3(void)
{
    BoundaryBit0031 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0032ForReadAcquisitionCoroutine3(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0032;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0032ForReadAcquisitionCoroutine3(void)
{
    BoundaryBit0032 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0033ForReadAcquisitionCoroutine3(void)
{
    BoundaryBit0033 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0035ForReadAcquisitionCoroutine3(void)
{
    BoundaryBit0035 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0035ForActiveWriteFinalizer(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0035;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0039Atomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0039;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit0039Atomic(void)
{
    BoundaryBit0039 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0010ForReadCdCoverage(void)
{
    BoundaryBit0010 = 0;
    return;
}

void PreparePremiumRawSectorBatchBitsAtomic(void)
{
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    BoundaryBit000C = 0;
    return;
}

u8 ReadBit003AForProcessedSfrDbWindow(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit003A == 0)
        goto ReadBit003AForProcessedSfrDbWindowReturn;
    ++accumulator;
ReadBit003AForProcessedSfrDbWindowReturn:;
    return accumulator;
}

void PrepareWritePreservationHandshakeLowAtomic(void)
{
    BoundaryBit00B0 = 0;
    BoundaryBit00B1 = 0;
    return;
}

void SetWritePreservationActiveBitsAtomic(void)
{
    BoundaryBit0002 = 1;
    BoundaryBit001B = 1;
    return;
}

void ClearWritePreservationActiveBitsIfSetAtomic(void)
{
    if(BoundaryBit001B == 0)
        goto ClearWritePreservationActiveBitsIfSetReturn;
    BoundaryBit001B = 0;
    BoundaryBit0002 = 0;
ClearWritePreservationActiveBitsIfSetReturn:;
    return;
}

void SetWritePreservationHandshakeHighAtomic(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 1;
    return;
}

void PrepareOpcHandshakePinsAtomic(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0044ForReadAcquisitionTarget(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0044;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void ClearFormat2RawReadModeBitsAtomic(void)
{
    BoundaryBit0013 = 0;
    BoundaryBit000D = 0;
    BoundaryBit000E = 0;
    return;
}

void SetFormat2RawReadModeBitAtomic(void)
{
    BoundaryBit000E = 1;
    return;
}

void ClearFormat2RawReadModeBitAtomic(void)
{
    BoundaryBit000E = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0038Atomic(void)
{
    BoundaryBit0038 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearMediaReadControlBits0017And0020(void)
{
    BoundaryBit0017 = 0;
    BoundaryBit0020 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0038ForOperatingProfileAfterResponse(void)
{
    BoundaryBit0038 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit003AForOperatingProfileAfterResponse(void)
{
    BoundaryBit003A = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0035ForLeadInOutSequenceAtomic(void)
{
    BoundaryBit0035 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0406(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0086 == 0)
        goto FirmwareReadBit0406Return;
    accumulator = 1u;
FirmwareReadBit0406Return:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0402(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0082 == 0)
        goto FirmwareReadBit0402Return;
    accumulator = 1u;
FirmwareReadBit0402Return:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0013ForReadTargetAdjustment(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0013 == 0)
        goto ReadBit0013Done;
    ++accumulator;
ReadBit0013Done:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit001bForReadTargetAdjustment(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit001B == 0)
        goto ReadBit001bDone;
    ++accumulator;
ReadBit001bDone:;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBit001bForLbaMapping(void)
{
    BoundaryBit001B = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit001bForLbaMapping(void)
{
    BoundaryBit001B = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit001cForReadTargetAdjustment(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit001C == 0)
        goto ReadBit001cDone;
    ++accumulator;
ReadBit001cDone:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit001dForReadTargetAdjustment(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit001D == 0)
        goto ReadBit001dDone;
    ++accumulator;
ReadBit001dDone:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0016ForRangeValidation(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0016 == 0)
        goto ReadBit0016ForRangeValidationDone;
    ++accumulator;
ReadBit0016ForRangeValidationDone:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0017ForRangeValidation(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0017 == 0)
        goto ReadBit0017ForRangeValidationDone;
    ++accumulator;
ReadBit0017ForRangeValidationDone:;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBits0016And0017ForRangeValidationAtomic(void)
{
    BoundaryBit0017 = 0;
    BoundaryBit0016 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0013ForRangeValidationAtomic(void)
{
    BoundaryBit0013 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit001CForRangeValidationAtomic(void)
{
    BoundaryBit001C = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit001DForRangeValidationAtomic(void)
{
    BoundaryBit001D = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0021ForAlternateRangeValidation(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0021 == 0)
        goto ReadBit0021ForAlternateRangeValidationDone;
    ++accumulator;
ReadBit0021ForAlternateRangeValidationDone:;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBits0016Then0017ForAlternateRangeValidationAtomic(void)
{
    BoundaryBit0016 = 0;
    BoundaryBit0017 = 0;
    return;
}

u8 FirmwareReadBit000E(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000E == 0)
        goto FirmwareReadBit000EReturn;
    accumulator = 1u;
FirmwareReadBit000EReturn:;
    return accumulator;
}

u8 FirmwareReadBit001A(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit001A == 0)
        goto FirmwareReadBit001AReturn;
    accumulator = 1u;
FirmwareReadBit001AReturn:;
    return accumulator;
}

void FirmwareWriteBit001A(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit001AClear;
    BoundaryBit001A = 1;
    return;
FirmwareWriteBit001AClear:;
    BoundaryBit001A = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0018ForRawSectorBufferEvent(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0018 == 0)
        goto ReadBit0018ForRawSectorBufferEventReturn;
    accumulator = 1u;
ReadBit0018ForRawSectorBufferEventReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0019ForRawSectorBufferEvent(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0019 == 0)
        goto ReadBit0019ForRawSectorBufferEventReturn;
    accumulator = 1u;
ReadBit0019ForRawSectorBufferEventReturn:;
    return accumulator;
}

u8 FirmwareReadBit0004(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0004 == 0)
        goto FirmwareReadBit0004Return;
    accumulator = 1u;
FirmwareReadBit0004Return:;
    return accumulator;
}

u8 FirmwareReadBit0034(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0034 == 0)
        goto FirmwareReadBit0034Return;
    accumulator = 1u;
FirmwareReadBit0034Return:;
    return accumulator;
}

u8 FirmwareReadBit000B(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000B == 0)
        goto FirmwareReadBit000BReturn;
    accumulator = 1u;
FirmwareReadBit000BReturn:;
    return accumulator;
}

u8 FirmwareReadBit0011(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0011 == 0)
        goto FirmwareReadBit0011Return;
    accumulator = 1u;
FirmwareReadBit0011Return:;
    return accumulator;
}

u8 FirmwareReadBit0013(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0013 == 0)
        goto FirmwareReadBit0013Return;
    accumulator = 1u;
FirmwareReadBit0013Return:;
    return accumulator;
}

void ClearBit000EForRawSectorTransferSetupAtomic(void)
{
    BoundaryBit000E = 0;
    return;
}

u8 ReadBit000FForControllerPositionDecode(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000F == 0)
        goto ReadBit000FForControllerPositionDecodeReturn;
    accumulator = 1u;
ReadBit000FForControllerPositionDecodeReturn:;
    return accumulator;
}

u8 ReadBit0013ForControllerPositionDecode(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0013 == 0)
        goto ReadBit0013ForControllerPositionDecodeReturn;
    accumulator = 1u;
ReadBit0013ForControllerPositionDecodeReturn:;
    return accumulator;
}

u8 ReadBit001CForControllerPositionDecode(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit001C == 0)
        goto ReadBit001CForControllerPositionDecodeReturn;
    accumulator = 1u;
ReadBit001CForControllerPositionDecodeReturn:;
    return accumulator;
}

u8 ReadBit001DForControllerPositionDecode(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit001D == 0)
        goto ReadBit001DForControllerPositionDecodeReturn;
    accumulator = 1u;
ReadBit001DForControllerPositionDecodeReturn:;
    return accumulator;
}

u8 FirmwareReadBit001B(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit001B == 0)
        goto FirmwareReadBit001BReturn;
    accumulator = 1u;
FirmwareReadBit001BReturn:;
    return accumulator;
}

u8 FirmwareReadBit0024(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0024 == 0)
        goto FirmwareReadBit0024Return;
    accumulator = 1u;
FirmwareReadBit0024Return:;
    return accumulator;
}

u8 FirmwareReadBit0008(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0008 == 0)
        goto FirmwareReadBit0008Return;
    accumulator = 1u;
FirmwareReadBit0008Return:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0005ForRawReadFinalizer(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0005;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearRawReadFinalizerBitsAtomic(void)
{
    BoundaryBit0008 = 0;
    BoundaryBit0022 = 0;
    BoundaryBit001F = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearEventBit7TransferBitsAtomic(void)
{
    BoundaryBit000E = 0;
    BoundaryBit0018 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetRawReadTerminalBitForEventBit7Transfer(void)
{
    BoundaryBit0009 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP3RxHighTxLowForEventBit7Transfer(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 0;
    return;
}

u8 FirmwareReadBit0407(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0087 == 0)
        goto FirmwareReadBit0407Return;
    accumulator = 1u;
FirmwareReadBit0407Return:;
    return accumulator;
}

void FirmwareWriteBit000A(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit000AClear;
    BoundaryBit000A = 1;
    return;
FirmwareWriteBit000AClear:;
    BoundaryBit000A = 0;
    return;
}

void FirmwareWriteBit000C(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit000CClear;
    BoundaryBit000C = 1;
    return;
FirmwareWriteBit000CClear:;
    BoundaryBit000C = 0;
    return;
}

void FirmwareWriteBit0016(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0016Clear;
    BoundaryBit0016 = 1;
    return;
FirmwareWriteBit0016Clear:;
    BoundaryBit0016 = 0;
    return;
}

void FirmwareWriteBit0017(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0017Clear;
    BoundaryBit0017 = 1;
    return;
FirmwareWriteBit0017Clear:;
    BoundaryBit0017 = 0;
    return;
}

void FirmwareWriteBit0009(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0009Clear;
    BoundaryBit0009 = 1;
    return;
FirmwareWriteBit0009Clear:;
    BoundaryBit0009 = 0;
    return;
}

void FirmwareWriteBit0004(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0004Clear;
    BoundaryBit0004 = 1;
    return;
FirmwareWriteBit0004Clear:;
    BoundaryBit0004 = 0;
    return;
}

void FirmwareWriteBit0024(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0024Clear;
    BoundaryBit0024 = 1;
    return;
FirmwareWriteBit0024Clear:;
    BoundaryBit0024 = 0;
    return;
}

void FirmwareWriteBit0026(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0026Clear;
    BoundaryBit0026 = 1;
    return;
FirmwareWriteBit0026Clear:;
    BoundaryBit0026 = 0;
    return;
}

void FirmwareWriteBit003D(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit003DClear;
    BoundaryBit003D = 1;
    return;
FirmwareWriteBit003DClear:;
    BoundaryBit003D = 0;
    return;
}

void FirmwareWriteBit0034(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0034Clear;
    BoundaryBit0034 = 1;
    return;
FirmwareWriteBit0034Clear:;
    BoundaryBit0034 = 0;
    return;
}

void FirmwareWriteBit0043(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0043Clear;
    BoundaryBit0043 = 1;
    return;
FirmwareWriteBit0043Clear:;
    BoundaryBit0043 = 0;
    return;
}

void FirmwareWriteBit003E(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit003EClear;
    BoundaryBit003E = 1;
    return;
FirmwareWriteBit003EClear:;
    BoundaryBit003E = 0;
    return;
}

void FirmwareWriteBit0040(u8 value)
{
    if(value == 0u)
        goto FirmwareWriteBit0040Clear;
    BoundaryBit0040 = 1;
    return;
FirmwareWriteBit0040Clear:;
    BoundaryBit0040 = 0;
    return;
}

u8 ReadBit003DForRawReadProgress(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit003D == 0)
        goto ReadBit003DForRawReadProgressReturn;
    accumulator = 1u;
ReadBit003DForRawReadProgressReturn:;
    return accumulator;
}

u8 ReadBit003EForRawReadProgress(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit003E == 0)
        goto ReadBit003EForRawReadProgressReturn;
    accumulator = 1u;
ReadBit003EForRawReadProgressReturn:;
    return accumulator;
}

u8 ReadBit003FForRawReadProgress(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit003F == 0)
        goto ReadBit003FForRawReadProgressReturn;
    accumulator = 1u;
ReadBit003FForRawReadProgressReturn:;
    return accumulator;
}

u8 ReadBit0040ForRawReadProgress(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0040 == 0)
        goto ReadBit0040ForRawReadProgressReturn;
    accumulator = 1u;
ReadBit0040ForRawReadProgressReturn:;
    return accumulator;
}

u8 FirmwareReadBit0020(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0020 == 0)
        goto FirmwareReadBit0020Return;
    accumulator = 1u;
FirmwareReadBit0020Return:;
    return accumulator;
}

u8 FirmwareReadBit002B(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit002B == 0)
        goto FirmwareReadBit002BReturn;
    accumulator = 1u;
FirmwareReadBit002BReturn:;
    return accumulator;
}

void ClearBit0026Atomic(void)
{
    BoundaryBit0026 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0027Atomic(void)
{
    BoundaryBit0027 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0027Atomic(void)
{
    BoundaryBit0027 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0027ForBuild(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0027;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 FirmwareReadBit0031(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0031 == 0)
        goto FirmwareReadBit0031Return;
    accumulator = 1u;
FirmwareReadBit0031Return:;
    return accumulator;
}

u8 FirmwareReadBit0030(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0030 == 0)
        goto FirmwareReadBit0030Return;
    accumulator = 1u;
FirmwareReadBit0030Return:;
    return accumulator;
}

u8 FirmwareReadBit0028(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0028 == 0)
        goto FirmwareReadBit0028Return;
    accumulator = 1u;
FirmwareReadBit0028Return:;
    return accumulator;
}

void SetP0Bit3Atomic(void)
{
    BoundaryBit0083 = 1;
    return;
}

void ClearP0Bit3Atomic(void)
{
    BoundaryBit0083 = 0;
    return;
}

void ClearBit0547Atomic(void)
{
    BoundaryBit00AF = 0;
    return;
}

void SetBit0547Atomic(void)
{
    BoundaryBit00AF = 1;
    return;
}

void SetBit0546Atomic(void)
{
    BoundaryBit00AE = 1;
    return;
}

void SetTimer0InterruptEnableAtomic(void)
{
    BoundaryBit00A9 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit0ForRuntimeDriveState(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0080 == 0)
        goto ReadP0Bit0ForRuntimeDriveStateReturn;
    accumulator = 1u;
ReadP0Bit0ForRuntimeDriveStateReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit1ForRuntimeDriveState(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0081 == 0)
        goto ReadP0Bit1ForRuntimeDriveStateReturn;
    accumulator = 1u;
ReadP0Bit1ForRuntimeDriveStateReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit2ForRuntimeDriveState(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0082 == 0)
        goto ReadP0Bit2ForRuntimeDriveStateReturn;
    accumulator = 1u;
ReadP0Bit2ForRuntimeDriveStateReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit5ForRuntimeDriveState(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0085 == 0)
        goto ReadP0Bit5ForRuntimeDriveStateReturn;
    accumulator = 1u;
ReadP0Bit5ForRuntimeDriveStateReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit6ForRuntimeDriveState(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0086 == 0)
        goto ReadP0Bit6ForRuntimeDriveStateReturn;
    accumulator = 1u;
ReadP0Bit6ForRuntimeDriveStateReturn:;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit000BForRuntimeDriveStateAtomic(void)
{
    BoundaryBit000B = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearReadMeasurementBit0028Atomic(void)
{
    BoundaryBit0028 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0028ForEventProgress(void)
{
    BoundaryBit0028 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0033ForEventProgress(void)
{
    BoundaryBit0033 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0028ForBufferSetup(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0028 == 0)
        goto ReadBit0028ForBufferSetupReturn;
    accumulator = 1u;
ReadBit0028ForBufferSetupReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit002BForBufferSetup(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit002B == 0)
        goto ReadBit002BForBufferSetupReturn;
    accumulator = 1u;
ReadBit002BForBufferSetupReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0030ForBufferSetup(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0030 == 0)
        goto ReadBit0030ForBufferSetupReturn;
    accumulator = 1u;
ReadBit0030ForBufferSetupReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0032ForBufferSetup(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0032 == 0)
        goto ReadBit0032ForBufferSetupReturn;
    accumulator = 1u;
ReadBit0032ForBufferSetupReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0035ForBufferSetup(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0035 == 0)
        goto ReadBit0035ForBufferSetupReturn;
    accumulator = 1u;
ReadBit0035ForBufferSetupReturn:;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0036ForBufferSetup(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0036 == 0)
        goto ReadBit0036ForBufferSetupReturn;
    accumulator = 1u;
ReadBit0036ForBufferSetupReturn:;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit002BForBufferSetup(void)
{
    BoundaryBit002B = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit002BForBufferSetup(void)
{
    BoundaryBit002B = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0032ForBufferSetup(void)
{
    BoundaryBit0032 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0032ForBufferSetup(void)
{
    BoundaryBit0032 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0030ForBufferSetup(void)
{
    BoundaryBit0030 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0033ForBufferSetup(void)
{
    BoundaryBit0033 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearReadMeasurementWindowBitsAtomic(void)
{
    BoundaryBit002B = 0;
    BoundaryBit002C = 0;
    BoundaryBit002E = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0032ForReadMeasurementReset(void)
{
    u8 accumulator = 0u;
    if(BoundaryBit0032 == 0)
        goto ReadBit0032ForReadMeasurementResetClear;
    accumulator = 1u;
    return accumulator;
ReadBit0032ForReadMeasurementResetClear:;
    accumulator = 0u;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0032ForReadMeasurementResetAtomic(void)
{
    BoundaryBit0032 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0542Atomic(void)
{
    BoundaryBit00AA = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0542Atomic(void)
{
    BoundaryBit00AA = 1;
    return;
}

void ClearBit0018Atomic(void)
{
    BoundaryBit0018 = 0;
    return;
}

void SetBit0018Atomic(void)
{
    BoundaryBit0018 = 1;
    return;
}

void WriteBit0019FromAcc(u8 value)
{
    if(value == 0u)
        goto WriteBit0019Clear;
    BoundaryBit0019 = 1;
    return;
WriteBit0019Clear:;
    BoundaryBit0019 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit003FAtomic(void)
{
    BoundaryBit003F = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit003FAtomic(void)
{
    BoundaryBit003F = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit002FForModePage05Atomic(void)
{
    BoundaryBit002F = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit002FForModePage05Atomic(void)
{
    BoundaryBit002F = 0;
    return;
}

void SetBit0046ForReadCommandAtomic(void)
{
    BoundaryBit0046 = 1;
    return;
}

void ClearBit0046ForReadCommandAtomic(void)
{
    BoundaryBit0046 = 0;
    return;
}

void SetBit0021ForReadCdAtomic(void)
{
    BoundaryBit0021 = 1;
    return;
}

void SetReadCdMsfNegativeRangeBitsAtomic(void)
{
    BoundaryBit0021 = 1;
    BoundaryBit0002 = 1;
    BoundaryBit001B = 1;
    return;
}

void ClearReadCdMsfTemporaryBitsIfSetAtomic(void)
{
    if(BoundaryBit001B == 0)
        goto ClearReadCdMsfTemporaryBitsDone;
    BoundaryBit001B = 0;
    BoundaryBit0002 = 0;
ClearReadCdMsfTemporaryBitsDone:;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit001EAtomic(void)
{
    BoundaryBit001E = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearCdTextRawReadSetupBitsAtomic(void)
{
    BoundaryBit000D = 0;
    BoundaryBit0003 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0013ForCdTextRawReadAtomic(void)
{
    BoundaryBit0013 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION FinalizeCdTextRawReadModeBitsAtomic(void)
{
    BoundaryBit001B = 0;
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0032(void)
{
    u8 accumulator = 0u;
    if(BoundaryBit0032 == 0)
        goto FirmwareReadBit0032_Clear;
    accumulator = 1u;
    return accumulator;
FirmwareReadBit0032_Clear:;
    accumulator = 0u;
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION FirmwareReadBit0036(void)
{
    u8 accumulator = 0u;
    if(BoundaryBit0036 == 0)
        goto FirmwareReadBit0036_Clear;
    accumulator = 1u;
    return accumulator;
FirmwareReadBit0036_Clear:;
    accumulator = 0u;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit054AAtomic(void)
{
    BoundaryBit00AB = 1;
    return;
}

void ClearBit0023Atomic(void)
{
    BoundaryBit0023 = 0;
    return;
}

void SetBit0023Atomic(void)
{
    BoundaryBit0023 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0023Atomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0023;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0012Atomic(void)
{
    BoundaryBit0012 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0014Atomic(void)
{
    BoundaryBit0014 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0015Atomic(void)
{
    BoundaryBit0015 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0007Atomic(void)
{
    BoundaryBit0007 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0009Atomic(void)
{
    BoundaryBit0009 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0009Atomic(void)
{
    BoundaryBit0009 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION InitializeRawReadPositionProbeBitsAtomic(void)
{
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    BoundaryBit001B = 0;
    BoundaryBit0018 = 0;
    BoundaryBit000E = 0;
    BoundaryBit0009 = 0;
    BoundaryBit0008 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0009Atomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0009;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit0017AndClearBit0016ForPositionProbeAtomic(void)
{
    BoundaryBit0017 = 1;
    BoundaryBit0016 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0003ForPackedSubchannelBatch(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0003;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit0003ForPackedSubchannelBatch(void)
{
    BoundaryBit0003 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit001EForPackedSubchannelBatch(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit001E;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit001CAtomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit001C;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit001DAtomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit001D;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearExternalInterrupt0PendingAndEnableAtomic(void)
{
    BoundaryBit0089 = 0;
    BoundaryBit00A8 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetExternalInterrupt0EnableAtomic(void)
{
    BoundaryBit00A8 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearExternalInterrupt0EnableAtomic(void)
{
    BoundaryBit00A8 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadP1Bit3Atomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0093;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBit000EForReadRequestAtomic(void)
{
    BoundaryBit000E = 0;
    return;
}

void ClearReadMasterCueEntryBitsAtomic(void)
{
    BoundaryBit000E = 0;
    BoundaryBit0018 = 0;
    return;
}

void ClearReadMasterCueModeBitsAtomic(void)
{
    BoundaryBit000C = 0;
    BoundaryBit000D = 0;
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    BoundaryBit001B = 0;
    return;
}

void SetReadMasterCueHandshakePinsLowAtomic(void)
{
    BoundaryBit00B0 = 0;
    BoundaryBit00B1 = 0;
    return;
}

void SetReadMasterCueHandshakePinsHighAtomic(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 1;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0014Atomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0014;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit0015Atomic(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit0015;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION SetBit0014Atomic(void)
{
    BoundaryBit0014 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0015Atomic(void)
{
    BoundaryBit0015 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0015ForCursorAtomic(void)
{
    BoundaryBit0015 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit000EForBuild(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit000E;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit000DForWriteRecordQuery(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit000D;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

u8 ReadBit000DForVendorD8(void)
{
    u8 accumulator = 0u;
    u8 carry = 0u;
    carry = BoundaryBit000D;
    accumulator = 0u;
    accumulator = (u8)((accumulator << 1) | carry);
    return accumulator;
}

void ClearVendorD8RawReadModeBitsAtomic(void)
{
    BoundaryBit000D = 0;
    BoundaryBit0013 = 0;
    BoundaryBit000E = 0;
    BoundaryBit0018 = 0;
    return;
}

void SetP1Bit0Atomic(void)
{
    BoundaryBit0090 = 1;
    return;
}

void SetP1Bit1Atomic(void)
{
    BoundaryBit0091 = 1;
    return;
}

void SetBit003BAtomic(void)
{
    BoundaryBit003B = 1;
    return;
}

void ClearBit003BAtomic(void)
{
    BoundaryBit003B = 0;
    return;
}

void ClearSyntheticRawReadPipelineBitsAtomic(void)
{
    BoundaryBit002F = 0;
    BoundaryBit0030 = 0;
    BoundaryBit0031 = 0;
    BoundaryBit0032 = 0;
    BoundaryBit0033 = 0;
    BoundaryBit0034 = 0;
    BoundaryBit0035 = 0;
    return;
}

void ClearP1Bits0And1Atomic(void)
{
    BoundaryBit0090 = 0;
    BoundaryBit0091 = 0;
    return;
}

void SelectP1Bit0OnlyAtomic(void)
{
    BoundaryBit0091 = 0;
    BoundaryBit0090 = 1;
    return;
}

void SelectP1Bit1OnlyAtomic(void)
{
    BoundaryBit0091 = 1;
    BoundaryBit0090 = 0;
    return;
}

void ClearControllerDataPathBits31And33Atomic(void)
{
    BoundaryBit0031 = 0;
    BoundaryBit0033 = 0;
    return;
}

void ClearReadMeasurementPhaseBits2BAnd43Atomic(void)
{
    BoundaryBit002B = 0;
    BoundaryBit0043 = 0;
    return;
}

void SetP3Bit0Atomic(void)
{
    BoundaryBit00B0 = 1;
    return;
}

void ClearP3Bit0Atomic(void)
{
    BoundaryBit00B0 = 0;
    return;
}

void SetP3Bit1Atomic(void)
{
    BoundaryBit00B1 = 1;
    return;
}

void SetP3Bits0And1HighForTimedOutputSequence(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 1;
    return;
}

u8 ReadBit000AForReadVerifyRange(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000A == 0)
        goto ReadBit000AForReadVerifyRangeReturn;
    ++accumulator;
ReadBit000AForReadVerifyRangeReturn:;
    return accumulator;
}

void ClearBit000CForReadVerifyRange(void)
{
    BoundaryBit000C = 0;
    return;
}

u8 ReadBit000CForReadVerifyDescriptorPoll(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000C == 0)
        goto ReadBit000CForReadVerifyDescriptorPollReturn;
    ++accumulator;
ReadBit000CForReadVerifyDescriptorPollReturn:;
    return accumulator;
}

void SetP3RxLowTxHighForPremiumPayload(void)
{
    BoundaryBit00B0 = 0;
    BoundaryBit00B1 = 1;
    return;
}

void ClearP1TimerPinsForPremiumPayload(void)
{
    BoundaryBit0091 = 0;
    BoundaryBit0090 = 0;
    return;
}

void ClearReadVerifyRawModeBitsAtomic(void)
{
    BoundaryBit000D = 0;
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    BoundaryBit001B = 0;
    return;
}

void SetReadVerifyHandshakePinsLowAtomic(void)
{
    BoundaryBit00B0 = 0;
    BoundaryBit00B1 = 0;
    return;
}

void SetReadVerifyHandshakePinsHighAtomic(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 1;
    return;
}

void PrepareReadHeaderRangeValidationBitsAtomic(void)
{
    BoundaryBit000C = 0;
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    BoundaryBit001B = 0;
    BoundaryBit000D = 1;
    return;
}

void ClearBit000DForReadHeaderAtomic(void)
{
    BoundaryBit000D = 0;
    return;
}

void SetWriteAndVerifyCommandActiveBitAtomic(void)
{
    BoundaryBit0002 = 1;
    return;
}

void ClearWriteAndVerifyCommandActiveBitAtomic(void)
{
    BoundaryBit0002 = 0;
    return;
}

void ClearP3Bit1Atomic(void)
{
    BoundaryBit00B1 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetMediaState5LeadOutTransitionBitsAtomic(void)
{
    BoundaryBit003B = 1;
    BoundaryBit0039 = 1;
    BoundaryBit003A = 1;
    BoundaryBit0036 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION CopyBit002FTo0030Atomic(void)
{
    if(BoundaryBit002F == 0)
        goto CopyBit002FTo0030Clear;
    BoundaryBit0030 = 1;
    return;
CopyBit002FTo0030Clear:;
    BoundaryBit0030 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetBit0035Atomic(void)
{
    BoundaryBit0035 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION WriteBit0031Atomic(u8 value)
{
    if(value == 0u)
        goto WriteBit0031AtomicClear;
    BoundaryBit0031 = 1;
    return;
WriteBit0031AtomicClear:;
    BoundaryBit0031 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit000CForHostTransferDescriptorCount(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000C == 0)
        goto ReadBit000CForHostTransferDescriptorCountReturn;
    ++accumulator;
ReadBit000CForHostTransferDescriptorCountReturn:;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION WriteBit003CForReadCommandRawSectorTransferBatch(u8 value)
{
    if(value == 0u)
        goto WriteBit003CForReadCommandRawSectorTransferBatchClear;
    BoundaryBit003C = 1;
    return;
WriteBit003CForReadCommandRawSectorTransferBatchClear:;
    BoundaryBit003C = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION InitializeReadCommandRawSectorBitsAtomic(void)
{
    BoundaryBit001B = 0;
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION InitializeReadCdRawSectorBitsAtomic(void)
{
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearReadCommandRawSectorValidationBitsAtomic(void)
{
    BoundaryBit000D = 0;
    BoundaryBit000E = 0;
    BoundaryBit0018 = 0;
    return;
}

u8 PLEXTOR_NEAR_FUNCTION ReadBit000AForReadCommandRawSector(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000A == 0)
        goto ReadBit000AForReadCommandRawSectorReturn;
    ++accumulator;
ReadBit000AForReadCommandRawSectorReturn:;
    return accumulator;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0001ForReadCommandRawSectorAtomic(void)
{
    BoundaryBit0001 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0017ForStartupSequenceAtomic(void)
{
    BoundaryBit0017 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION ClearBit0017ForRawQPositionProbeAtomic(void)
{
    BoundaryBit0017 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP3PinsHighForStartupSequenceAtomic(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP1TimerPinsLowForStartupSequenceAtomic(void)
{
    BoundaryBit0091 = 0;
    BoundaryBit0090 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP1TimerPinsHighForStartupSequenceAtomic(void)
{
    BoundaryBit0091 = 1;
    BoundaryBit0090 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetP3PinsHighForDriveInitializationCompletionAtomic(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 1;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetMediaState0IndexedP3PatternAtomic(void)
{
    BoundaryBit00B0 = 1;
    BoundaryBit00B1 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetMediaState0P1TimerPinsLowAtomic(void)
{
    BoundaryBit0091 = 0;
    BoundaryBit0090 = 0;
    return;
}

void PLEXTOR_NEAR_FUNCTION SetMediaState0P1TimerPinsHighAtomic(void)
{
    BoundaryBit0091 = 1;
    BoundaryBit0090 = 1;
    return;
}

u8 ReadFirmwareMainBit0029(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0029 == 0)
        goto ReadFirmwareMainBit0029Return;
    ++accumulator;
ReadFirmwareMainBit0029Return:;
    return accumulator;
}

u8 ReadFirmwareMainBit0046(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit0046 == 0)
        goto ReadFirmwareMainBit0046Return;
    ++accumulator;
ReadFirmwareMainBit0046Return:;
    return accumulator;
}

u8 ReadBit000AForFormatUnitCode11Scan(void)
{
    u8 accumulator = 0u;
    accumulator = 0u;
    if(BoundaryBit000A == 0)
        goto ReadBit000AForFormatUnitCode11ScanDone;
    ++accumulator;
ReadBit000AForFormatUnitCode11ScanDone:;
    return accumulator;
}

void ClearFormatUnitCode11ScanModeBitsAtomic(void)
{
    BoundaryBit001B = 0;
    BoundaryBit001D = 0;
    BoundaryBit001C = 0;
    BoundaryBit0013 = 0;
    BoundaryBit000D = 0;
    BoundaryBit000E = 0;
    BoundaryBit0018 = 0;
    return;
}

void ClearBit000CForFormatUnitCode11Scan(void)
{
    BoundaryBit000C = 0;
    return;
}
