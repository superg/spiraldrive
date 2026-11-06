#include "controller_record_boundaries.h"
#include "arithmetic.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "mcs251.h"
#include "memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_boundaries_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_boundaries_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_record_boundaries_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteBigEndianU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fcdf6e */
u8 PLEXTOR_NEAR_FUNCTION LocateAndAdjustType2ControllerRecordWindow(void)
{
    u8 candidate_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC4C) == 0u ? 1u : 25u;
    u8 candidate_index;

    for(candidate_index = 0u; candidate_index < candidate_count; ++candidate_index)
    {
        u32 candidate_base = ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA595) + MultiplyU32Low(39ul, candidate_index);
        u8 offset_index;

        for(offset_index = 0u; offset_index < 4u; ++offset_index)
        {
            u8 status = PROBE_TYPE2_CONTROLLER_RECORD_WINDOW((s32)(candidate_base + 10ul + MultiplyU32Low(3ul, offset_index)));

            if(status == 5u)
                return 0u;
            if(status == 0x11u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC4C) != 0u)
                {
                    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BA595, candidate_base);
                    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BA58D, ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA595) + 39ul);
                    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BA59F, ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA58D) - 351ul);
                    WriteBigEndianU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION, ReadBigEndianU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION) + (u32)candidate_index * 32ul);
                    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BA599, ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA595) - 312ul);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC4C, 0u);
                }
                return 1u;
            }
        }
    }
    return 0u;
}

static u32 ReadBigEndianU32LowWordFirst(fw_addr_t address)
{
    u16 low = ((u16)FirmwareRead8(address + 2u) << 8) | FirmwareRead8(address + 3u);
    u16 high = ((u16)FirmwareRead8(address) << 8) | FirmwareRead8(address + 1u);

    return ((u32)high << 16) | low;
}

static void WriteBigEndianU32LowWordFirst(fw_addr_t address, u32 value)
{
    FirmwareWriteU16(FirmwareAddressIncrement24(address, 2u), (u16)value);
    FirmwareWriteU16(address, (u16)(value >> 16));
}

static u32 ReadPrimaryType2PackedPosition(u8 descriptor_index)
{
    fw_addr_t address = FW_HUGE_UNKNOWN_2BB938 + (u16)descriptor_index * 3u;
    u32 value = ReadBigEndianU32LowWordFirst(address) & 0x00fffffful;

    return value | 0xfff00000ul;
}

/* RAM:fcd65f */
void InitializePrimaryType2ControllerRecordDescriptorsAndBoundsFromIndex(u8 first_descriptor_index)
{
    u8 descriptor_index;
    u32 candidate;
    u32 selected;

    for(descriptor_index = first_descriptor_index; descriptor_index < 8u; ++descriptor_index)
    {
        candidate = ReadPrimaryType2PackedPosition(descriptor_index);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB04 + (u16)descriptor_index * 5u, candidate);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA537 + (u16)descriptor_index * 4u, candidate);
    }

    candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB04);
    selected = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB09);
    if((s32)candidate <= (s32)selected)
        selected = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB04);
    else
        selected = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB09);
    WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA577, selected);

    for(descriptor_index = 1u; descriptor_index < 8u; ++descriptor_index)
    {
        candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB04 + (u16)descriptor_index * 5u);
        selected = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA577);
        if((s32)selected > (s32)candidate)
        {
            candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB04 + (u16)descriptor_index * 5u);
            WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA577, candidate);
        }
    }
}

/* RAM:fcd610 */
void InitializePrimaryType2ControllerRecordDescriptorsAndBounds(void)
{
    InitializePrimaryType2ControllerRecordDescriptorsAndBoundsFromIndex(0u);
}

static u32 ReadSecondaryType2PackedPosition(u8 descriptor_index)
{
    fw_addr_t address = FW_HUGE_UNKNOWN_2BB950 + (u16)descriptor_index * 3u;

    return ReadBigEndianU32LowWordFirst(address) & 0x00fffffful;
}

/* RAM:fcd755 */
void InitializeType2ControllerRecordDescriptorTablesAndBoundsFromIndex(u8 first_descriptor_index)
{
    u8 descriptor_index;
    u32 candidate;
    u32 lower_bound;
    u32 upper_bound;

    for(descriptor_index = first_descriptor_index; descriptor_index < 8u; ++descriptor_index)
    {
        candidate = ReadSecondaryType2PackedPosition(descriptor_index);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C + (u16)descriptor_index * 5u, candidate);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA557 + (u16)descriptor_index * 4u, candidate);
    }

    candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB31);
    lower_bound = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C);
    if((s32)lower_bound > (s32)candidate)
    {
        lower_bound = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB31);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57B, lower_bound);
        upper_bound = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57F, upper_bound);
    }
    else
    {
        lower_bound = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57B, lower_bound);
        upper_bound = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB31);
        WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57F, upper_bound);
    }

    for(descriptor_index = 1u; descriptor_index < 8u; ++descriptor_index)
    {
        candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C + (u16)descriptor_index * 5u);
        lower_bound = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57B);
        if((s32)lower_bound > (s32)candidate)
        {
            candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C + (u16)descriptor_index * 5u);
            WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57B, candidate);
        }
        else
        {
            candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C + (u16)descriptor_index * 5u);
            upper_bound = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57F);
            if((s32)upper_bound < (s32)candidate)
            {
                candidate = ReadBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BFB2C + (u16)descriptor_index * 5u);
                WriteBigEndianU32LowWordFirst(FW_HUGE_UNKNOWN_2BA57F, candidate);
            }
        }
    }
}

/* RAM:fcd70e */
void InitializeType2ControllerRecordDescriptorTablesAndBounds(void)
{
    InitializeType2ControllerRecordDescriptorTablesAndBoundsFromIndex(0u);
}

/* RAM:fcd998 */
void InitializeType2ControllerRecordRuntimeStateFromWorkingTrack(void)
{
    u32 value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA2, FirmwareReadNear8(FW_NEAR_FORMAT_STATE));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA3, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA4, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA5, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA6, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38));
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CA7, 0x00000096ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CAB, 0ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CAF, ReadBigEndianU32(FW_NEAR_UNKNOWN_0CA7) + 0x618ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CB3, ReadBigEndianU32(FW_NEAR_UNKNOWN_0CAB) + 0x500ul);
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CB7, ReadBigEndianU32(FW_NEAR_UNKNOWN_DD41));

    value = ReadBigEndianU32(FW_NEAR_UNKNOWN_DD41) << 5;
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CBB, (u32)(DivideType2RuntimePosition((s32)value, 39l) - 150l));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CBF, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD55));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CC0, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56));
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CC1, ReadBigEndianU32(FW_NEAR_UNKNOWN_DD57));
    WriteBigEndianU32(FW_NEAR_UNKNOWN_0CC5, ReadBigEndianU32(FW_NEAR_UNKNOWN_DD5B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CC9, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD5F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CCA, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD60));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CD6, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD93));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CD7, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD94));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CD8, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD95));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CD9, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD96));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CDA, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD97));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CDB, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD98));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CDC, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD99));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CDD, FirmwareReadNear8(FW_NEAR_FORMAT_STATE));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CDE, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD9B));
    value = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD9C) & 1u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CDF, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0CDF) & 0xfeu) | value));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CE0, FirmwareReadNear8(FW_NEAR_UNKNOWN_DDA5));
}

/* RAM:fa9910 */
void IncrementType2ControllerRecordSlotValue(u8 record_slot_index)
{
    u32 controller_offset = 0x00094004ul + ((u32)record_slot_index << 14);
    u32 value;

    (void)CopyFromControllerBufferWindow(controller_offset, FW_HUGE_UNKNOWN_2BA633, 4u);
    value = ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA633) + 1ul;
    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BA584, value);
    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BA633, value);
    CopyBytesToAddressPlus1MiB(controller_offset, FW_HUGE_UNKNOWN_2BA633, 4u);
}

/* RAM:fccd59 */
void BuildActiveControllerRecordBoundaryTable(void)
{
    u8 index;
    u8 segment_count = CAPTURE_INHERITED_R7();

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u)
    {
        segment_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA588);
    }
    else
    {
        for(index = 0u; index < 64u; ++index)
        {
            if(ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA59F) < ReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84690) + (u16)index * 4u))
            {
                segment_count = index;
                break;
            }
        }
    }

    for(index = 0u; index < segment_count; ++index)
    {
        u32 boundary;

        if(index == (u8)(segment_count - 1u))
            boundary = ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA58D);
        else
            boundary = ReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490) + (u16)(index + 1u) * 4u) - 5ul;
        WriteBigEndianU32(FW_HUGE_UNKNOWN_2BAE33 + (u16)index * 4u, boundary);
    }

    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BAE2F, ReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490)) - 5ul);
}

static void BuildMipType2ControllerRecordBoundaryTableBody(u8 inherited_segment_count)
{
    u8 index;
    u8 segment_count = inherited_segment_count;

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u)
    {
        segment_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA588);
    }
    else if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 1u)
    {
        for(index = 0u; index < 64u; ++index)
        {
            if(ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA59F) < ReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84690) + (u16)index * 4u))
            {
                segment_count = index;
                break;
            }
        }
    }

    for(index = 0u; index < segment_count; ++index)
    {
        u32 boundary;

        if(index == (u8)(segment_count - 1u))
            boundary = ReadBigEndianU32(FW_HUGE_UNKNOWN_2BA58D);
        else
            boundary = ReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490) + (u16)(index + 1u) * 4u) - 5ul;
        WriteBigEndianU32(FW_HUGE_UNKNOWN_2BAE33 + (u16)index * 4u, boundary);
    }

    WriteBigEndianU32(FW_HUGE_UNKNOWN_2BAE2F, ReadBigEndianU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490)) - 5ul);
}

/* RAM:fcda9d */
void BuildMipType2ControllerRecordBoundaryTable(void)
{
    BuildMipType2ControllerRecordBoundaryTableBody(CAPTURE_INHERITED_R7());
}

/* Reconstruction helper; no direct Ghidra function address. */
void BuildMipType2ControllerRecordBoundaryTableWithInheritedCount(u8 inherited_segment_count)
{
    BuildMipType2ControllerRecordBoundaryTableBody(inherited_segment_count);
}
