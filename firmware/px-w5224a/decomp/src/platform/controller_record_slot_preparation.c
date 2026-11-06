#include "controller_record_slot_preparation.h"
#include "arithmetic.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_slot_preparation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadControllerRecordSlotU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteControllerRecordSlotU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static s32 TransformPrimaryControllerRecordPosition(s32 source_position)
{
    s32 quotient = DivideControllerRecordSlotPosition((s32)((u32)source_position + 150ul), 32l);

    return (s32)MultiplyU32Low((u32)quotient, 39ul);
}

static s32 TransformSecondaryControllerRecordPosition(s32 source_position)
{
    s32 quotient = DivideControllerRecordSlotPosition(source_position, 32l);

    return (s32)(MultiplyU32Low((u32)quotient, 39ul) + 145ul);
}

/* RAM:fa099c */
u8 PreparePrimaryType2ControllerRecordSlots(u8 acquisition_mode)
{
    u8 slot_index;
    s32 candidate;
    s32 source_position;

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_slot_preparation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    for(slot_index = 0u; slot_index < 8u; ++slot_index)
    {
        if(acquisition_mode == 0u)
            source_position = (s32)ReadControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA537 + (u16)slot_index * 4u);
        else
            source_position = (s32)ReadControllerRecordSlotU32(FW_NEAR_UNKNOWN_FB04 + (u16)slot_index * 5u);
        candidate = TransformPrimaryControllerRecordPosition(source_position);

        while(AcquirePrimaryType2ControllerRecordSlot(acquisition_mode, slot_index, candidate) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB03 + (u16)slot_index * 5u, 1u);
            source_position = (s32)(ReadControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA577) - 32ul);
            WriteControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA577, (u32)source_position);
            WriteControllerRecordSlotU32(FW_NEAR_UNKNOWN_FB04 + (u16)slot_index * 5u, (u32)source_position);
            WriteControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA537 + (u16)slot_index * 4u, (u32)source_position);
            candidate = TransformPrimaryControllerRecordPosition(source_position);
            if(candidate < (s32)ReadControllerRecordSlotU32(FW_NEAR_UNKNOWN_114F))
                return 0u;
        }
    }
    return 1u;
}

/* RAM:fa1227 */
u8 PrepareSecondaryType2ControllerRecordSlots(u8 acquisition_mode)
{
    u8 slot_index;
    s32 candidate;
    s32 source_position;
    s32 lower_bound;

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_record_slot_preparation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    for(slot_index = 0u; slot_index < 8u; ++slot_index)
    {
        source_position = (s32)ReadControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA557 + (u16)slot_index * 4u);
        candidate = TransformSecondaryControllerRecordPosition(source_position);
        WriteControllerRecordSlotU32(FW_NEAR_UNKNOWN_FB2C + (u16)slot_index * 5u, (u32)source_position);

        while(AcquireSecondaryType2ControllerRecordSlot(acquisition_mode, slot_index, candidate) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB2B + (u16)slot_index * 5u, 1u);
            source_position = (s32)(ReadControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA57B) - 32ul);
            WriteControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA57B, (u32)source_position);
            WriteControllerRecordSlotU32(FW_NEAR_UNKNOWN_FB2C + (u16)slot_index * 5u, (u32)source_position);
            WriteControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA557 + (u16)slot_index * 4u, (u32)source_position);
            candidate = TransformSecondaryControllerRecordPosition(source_position);
            lower_bound = (s32)(ReadControllerRecordSlotU32(FW_HUGE_UNKNOWN_2BA595) - 0x4e0ul);
            if(candidate < lower_bound)
                return 0u;
        }
    }
    return 1u;
}
