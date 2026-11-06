#include "firmware_memory.h"
#include "leaf_workers.h"
#include "mcs251.h"
#include "memory.h"

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32ControllerMapping(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u16 ReadBigEndianU16ControllerMapping(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteBigEndianU16ControllerMapping(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:f8f47a */
u8 MapOperatingCodeToProfileIndex(u8 operating_code)
{
    if(operating_code == 0)
        return 0;
    if(operating_code < 2u)
        return 1;
    if(operating_code < 4u)
        return 2;
    if(operating_code < 8u)
        return 3;
    if(operating_code < 10u)
        return 4;
    if(operating_code < 12u)
        return 5;
    if(operating_code < 16u)
        return 6u;
    if(operating_code < 20u)
        return 7u;
    if(operating_code < 24u)
        return 8u;
    if(operating_code < 32u)
        return 9u;
    if(operating_code < 40u)
        return 10u;
    if(operating_code < 48u)
        return 11u;
    return 12u;
}

/* RAM:f8f4e9 */
u8 MapOperatingCodeToTransitionClass(u8 operating_code)
{
    if(operating_code < 4u)
        return 0u;
    if(operating_code < 8u)
        return 1u;
    if(operating_code < 0x18u)
        return 2u;
    if(operating_code < 0x20u)
        return 3u;
    if(operating_code < 0x28u)
        return 4u;
    return 5u;
}

/* RAM:f8b6dc */
u8 MapOperatingCodeToProfileIndexMinusSeven(u8 operating_code)
{
    return (u8)(MapOperatingCodeToProfileIndex(operating_code) - 7u);
}

/* RAM:f8b995 */
void UpdateState103ABit6ForMediaState(void)
{
    u8 state_103a;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xbfu));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 5u)
    {
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 6u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xbfu));
            return;
        }
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 3u)
            return;
    }

    state_103a = FirmwareReadNear8(FW_NEAR_UNKNOWN_103A);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x20u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(state_103a & 0xbfu));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(state_103a | 0x40u));
}

/* RAM:f8b420 */
u8 PLEXTOR_NEAR_FUNCTION MapOperatingCodeToControllerChannel3Value(u8 operating_code)
{
    u8 enabled;

    if(operating_code == 2u)
    {
        enabled = FirmwareReadNear8(FW_NEAR_UNKNOWN_1006);
        return enabled == 0 ? 0xa5u : 0xa6u;
    }

    if(operating_code == 4u || operating_code == 8u || operating_code == 10u || operating_code == 12u || operating_code == 16u)
    {
        enabled = FirmwareReadNear8(FW_NEAR_UNKNOWN_1006);
        if(enabled == 0)
            return 0xadu;
        if(operating_code == 10u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x40u) != 0)
            return 0xaau;
        return 0xa6u;
    }

    enabled = FirmwareReadNear8(FW_NEAR_UNKNOWN_1006);
    if(enabled == 0)
        return 0xaau;
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0xffu ? 0xaau : 0xaeu;
}

/* RAM:f8b73d */
u8 IsSignedByteLessThanNegative45(u8 value)
{
    return (u8)(value < 0xd3u);
}

/* RAM:f9cd4a */
u8 PLEXTOR_NEAR_FUNCTION MapControllerRegisterIndexSkippingGap(u8 index)
{
    if(index > 0x0fu)
        index = (u8)(index + 5u);
    if(index > 0x1au)
        index = 0x1au;
    return index;
}

/* RAM:f94d9c */
u8 ClearControllerMeasurementSeriesFromIndex(u8 first_word_index)
{
    while(first_word_index < 15u)
    {
        u16 offset = (u16)first_word_index << 1;

        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F0F7, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F0F7, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F0D9, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F0D9, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F0BB, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F0BB, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EFE3, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EFE3, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EFC5, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EFC5, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EFA7, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EFA7, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F09D, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F09D, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F03F, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F03F, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F01F, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F01F, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F001, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_F001, (u16)(offset + 1u)), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EF89, offset), 0u);
        FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_EF89, (u16)(offset + 1u)), 0u);
        first_word_index++;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F115, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF85, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F117, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF87, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 2u);
    return first_word_index;
}

/* RAM:f94d1d */
u8 PLEXTOR_NEAR_FUNCTION ClearControllerMeasurementSeries(void)
{
    (void)ClearControllerMeasurementSeriesFromIndex(0u);
    return 2u;
}

/* RAM:f9829c */
void CopyControllerMeasurementSeries00Through05To07Through12(void)
{
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EF89, FW_NEAR_UNKNOWN_F09D, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFA7, FW_NEAR_UNKNOWN_F0BB, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFC5, FW_NEAR_UNKNOWN_F0D9, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFE3, FW_NEAR_UNKNOWN_F0F7, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F001, FW_NEAR_UNKNOWN_F11B, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F01F, FW_NEAR_UNKNOWN_F139, 30u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_F115, ReadBigEndianU16(FW_NEAR_UNKNOWN_EF85));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_F117, ReadBigEndianU16(FW_NEAR_UNKNOWN_EF87));
}

/* RAM:f98418 */
void SelectMediaStrategyRecordIndexForAtipClass1(void)
{
    s32 lead_in_start = ((s32)(u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_114F) << 24) | ((s32)(u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1150) << 16) | ((s32)(u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1151) << 8)
                      | (s32)(u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1152);
    u32 lead_out_end = FirmwareReadNearU32(FW_NEAR_CURRENT_FRAME_POSITION);
    u8 record_index;

    if(lead_in_start == (s32)-0x2cddL)
        record_index = lead_out_end == 0x0005265cul ? 0x84u : 0x85u;
    else if(lead_in_start == (s32)-0x2cd3L)
        record_index = lead_out_end == 0x0005184cul ? 0x82u : 0x86u;
    else if(lead_in_start == (s32)-0x2cc9L)
        record_index = 0x83u;
    else if(lead_in_start == (s32)-0x2cb7L)
        record_index = 0x87u;
    else if(lead_in_start == (s32)-0x2ab2L)
        record_index = 0x81u;
    else
        record_index = 0x80u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, record_index);
}

/* RAM:f98499 */
void SelectMediaStrategyRecordIndexForAtipClass2(void)
{
    s32 lead_in_start = (s32)ReadBigEndianU32ControllerMapping(FW_NEAR_UNKNOWN_114F);
    u32 lead_out_end = ReadBigEndianU32ControllerMapping(FW_NEAR_CURRENT_FRAME_POSITION);
    u8 selector = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB81);

    if(lead_in_start == (s32)-0x31ceL)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x93u);
    else if(lead_in_start == (s32)-0x3036L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa8u);
    else if(lead_in_start == (s32)-0x2fc1L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa9u);
    else if(lead_in_start == (s32)-0x2e38L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x9au);
    else if(lead_in_start == (s32)-0x2e37L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa4u);
    else if(lead_in_start == (s32)-0x2e0eL)
    {
        if(lead_out_end == 0x000520fdul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x95u);
        else if(lead_out_end == 359999ul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x9eu);
    }
    else if(lead_in_start == (s32)-0x2da5L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa2u);
    else if(lead_in_start == (s32)-0x2d78L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x97u);
    else if(lead_in_start == (s32)-0x2d4bL)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa7u);
    else if(lead_in_start == (s32)-0x2d1eL)
    {
        if(lead_out_end == 0x00051cfcul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa3u);
        else if(lead_out_end == 359999ul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa0u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x90u);
    }
    else if(lead_in_start == (s32)-0x2d13L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x9du);
    else if(lead_in_start == (s32)-0x2d0aL)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa6u);
    else if(lead_in_start == (s32)-0x2cecL)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xaau);
    else if(lead_in_start == (s32)-0x2cddL)
    {
        if(lead_out_end == 0x0005265cul || lead_out_end == 359999ul || lead_out_end == 98999ul || lead_out_end == 0x0001aeaau)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x94u);
        else if(lead_out_end == 0x00051dddul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x96u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x90u);
    }
    else if(lead_in_start == (s32)-0x2cd3L)
    {
        if(lead_out_end == 0x00051d92ul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x98u);
        else if(lead_out_end == 0x00052423ul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x9cu);
        else if(lead_out_end == 0x0005184cul || lead_out_end == 359999ul)
        {
            if(selector == 0x40u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x92u);
            else if(selector == 0x30u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x9fu);
        }
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x90u);
    }
    else if(lead_in_start == (s32)-0x2cc9L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x93u);
    else if(lead_in_start == (s32)-0x2cbeL || lead_in_start == (s32)-0x2cb7L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa1u);
    else if(lead_in_start == (s32)-0x2c79L)
    {
        if(lead_out_end == 0x00052f26ul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x9bu);
        else if(lead_out_end == 0x00018b82ul)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xa5u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x90u);
    }
    else if(lead_in_start == (s32)-0x2b51L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x99u);
    else if(lead_in_start == (s32)-0x2ab0L)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x91u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x90u);
}

/* RAM:f98765 */
void SelectMediaStrategyRecordIndexForAtipSelector1(void)
{
    s32 lead_in_start = (s32)ReadBigEndianU32ControllerMapping(FW_NEAR_UNKNOWN_114F);
    u32 lead_out_end = ReadBigEndianU32ControllerMapping(FW_NEAR_CURRENT_FRAME_POSITION);
    u8 record_index;

    if(lead_in_start == (s32)-0x31ceL || lead_in_start == (s32)-0x2cc9L)
        record_index = 0xb3u;
    else if(lead_in_start == (s32)-0x31cdL)
        record_index = 0xc0u;
    else if(lead_in_start == (s32)-0x3034L)
        record_index = 0xbcu;
    else if(lead_in_start == (s32)-0x2fc0L)
        record_index = 0xc7u;
    else if(lead_in_start == (s32)-0x2e0eL)
        record_index = 0xb4u;
    else if(lead_in_start == (s32)-0x2e0cL)
        record_index = 0xc1u;
    else if(lead_in_start == (s32)-0x2da5L)
        record_index = 0xbau;
    else if(lead_in_start == (s32)-0x2d4bL)
        record_index = 0xb9u;
    else if(lead_in_start == (s32)-0x2d1eL)
        record_index = 0xc2u;
    else if(lead_in_start == (s32)-0x2d1dL)
        record_index = 0xc5u;
    else if(lead_in_start == (s32)-0x2d13L)
        record_index = 0xbeu;
    else if(lead_in_start == (s32)-0x2d09L)
        record_index = 0xbbu;
    else if(lead_in_start == (s32)-0x2cebL)
        record_index = 0xb6u;
    else if(lead_in_start == (s32)-0x2cddL)
    {
        if(lead_out_end == 0x0005265cul || lead_out_end == 359999ul)
            record_index = 0xb5u;
        else if(lead_out_end == 0x00051dddul)
            record_index = 0xb0u;
        else if(lead_out_end == 0x00052651ul)
            record_index = 0xbdu;
        else
            record_index = 0xb9u;
    }
    else if(lead_in_start == (s32)-0x2cd3L)
        record_index = 0xb2u;
    else if(lead_in_start == (s32)-0x2cc8L)
        record_index = 0xc0u;
    else if(lead_in_start == (s32)-0x2cb7L || lead_in_start == (s32)-0x2cb6L)
        record_index = lead_out_end == 359999ul ? 0xc4u : 0xb7u;
    else if(lead_in_start == (s32)-0x2c79L)
        record_index = 0xbfu;
    else if(lead_in_start == (s32)-0x2b52L || lead_in_start == (s32)-0x2b50L)
        record_index = 0xb8u;
    else if(lead_in_start == (s32)-0x2b4fL)
        record_index = 0xc3u;
    else if(lead_in_start == (s32)-0x2aafL)
        record_index = 0xb1u;
    else
        record_index = 0xb0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, record_index);
    RefineMediaStrategyRecordIndexFromAtipIdentification();
}

/* RAM:f983b1 */
/* The native final store uses incoming R15 if no classifier route is selected. */
void SelectAndPublishMediaStrategyOffsetFromAtipClass(u8 inherited_offset)
{
    u8 record_index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0xfeu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 1u)
            {
                SelectMediaStrategyRecordIndexForAtipClass1();
                inherited_offset = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) - 0x80u);
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u)
            {
                SelectMediaStrategyRecordIndexForAtipClass2();
                inherited_offset = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) - 0x90u);
            }
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
        {
            SelectMediaStrategyRecordIndexForAtipSelector1();
            inherited_offset = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) - 0xb0u);
        }
        else
        {
            SelectMediaStrategyRecordIndexFromAtipLeadIn();
            inherited_offset = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) - 0xd0u);
        }
    }
    record_index = inherited_offset;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F05F, record_index);
}

/* RAM:fadbe3 */
void ApplySfrDaStrategyAdjustment(void)
{
    fw_addr_t table_base;
    fw_addr_t record;
    s16 direction;
    s16 first_delta;
    s16 limit_delta;
    u16 candidate_first;
    u16 candidate_limit;
    u16 current_limit;
    u8 apply = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x18u)
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82F7A);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x14u)
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82E84);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x10u)
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82D8E);
    else
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82C98);

    record = table_base + (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) * 3u);
    direction = (s16)((s16)(s8)FirmwareRead8(record) * 9);
    first_delta = (s16)(s8)FirmwareRead8(record + 1u);
    limit_delta = (s16)((s16)(s8)FirmwareRead8(record + 2u) * 18);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x28u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x20u)
    {
        direction = 0;
        first_delta = 0;
        limit_delta = 0;
    }

    candidate_limit = (u16)(ReadBigEndianU16ControllerMapping(FW_NEAR_UNKNOWN_FACF) + limit_delta);
    candidate_first = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD1) + first_delta);
    current_limit = ReadBigEndianU16ControllerMapping(FW_NEAR_UNKNOWN_FBC3);
    if(direction > 0)
    {
        if((s16)candidate_limit > (s16)current_limit && (s16)candidate_first <= (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD3))
            apply = 1u;
    }
    else if(direction < 0)
    {
        if(candidate_limit < current_limit && candidate_first >= (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FBD3))
            apply = 1u;
    }
    if(apply != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD1, (u8)candidate_first);
        WriteBigEndianU16ControllerMapping(FW_NEAR_UNKNOWN_FBC3, (u16)(current_limit + direction));
    }
}

/* RAM:fccf89 */
void ClearType2ControllerRecordWorkingState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA4, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CBB, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CB7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CB3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CAF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CAB, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CC0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CBF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CC5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CC1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CCA, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CC9, 0u);
}

/* RAM:fa832d */
void ResetEightChannelMeasurementCaptureState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1062, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1063, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1064, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1065, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1066, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_107E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_107F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_107C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_107D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_107A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_107B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1078, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1079, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1076, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1077, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1074, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1075, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1072, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1073, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1070, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1071, 0u);
}
