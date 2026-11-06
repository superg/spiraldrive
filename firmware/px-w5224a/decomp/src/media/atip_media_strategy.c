#include "atip_media_strategy.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/atip_media_strategy_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadAtipStrategyU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:f9f5a1 */
void SelectAndApplyMediaStrategyFromAtipLeadInTime(void)
{
    u8 low_nibble;
    u8 record_index;
    u8 alternate_family;
    u8 bcd_second;
    u8 bcd_frame;
    u16 identifier;
    s32 lead_in;
    u32 lead_out;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD7, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD7));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD8, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD8) & 0xefu));
    if(VALIDATE_ATIP_STRATEGY() == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0x80u);
        RESET_ATIP_STRATEGY_DRIVE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfdu));
        RESTART_ATIP_STRATEGY(FW_ENTRY_PROCESS_DRIVE_STATE_MACHINE);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        INITIALIZE_ATIP_STRATEGY();
        return;
    }

    bcd_second = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB54);
    bcd_frame = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB55);
    low_nibble = (u8)(bcd_frame & 0x0fu);
    record_index = low_nibble >= 5u ? 1u : 0u;
    alternate_family = record_index;
    identifier = (u16)(0xa914u + (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1140) * 75u) + (u16)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1141) / 10u) * 10u));
    lead_in = (s32)ReadAtipStrategyU32(FW_NEAR_UNKNOWN_114F);
    lead_out = ReadAtipStrategyU32(FW_NEAR_CURRENT_FRAME_POSITION);

    switch(identifier)
    {
    case 0xa914u:
    case 0xb067u:
        record_index = 0x30u;
        break;
    case 0xad79u:
    case 0xb274u:
    case 0xb76fu:
        record_index = 0x06u;
        if(low_nibble != 0u)
            record_index = 0x18u;
        if(low_nibble >= 5u)
            record_index = 0x37u;
        break;
    case 0xad83u:
    case 0xb229u:
    case 0xb70bu:
        if(low_nibble < 5u)
        {
            record_index = 0x0eu;
            if(lead_out == 0x00057e3eul || lead_out == 0x000521f8ul)
                record_index = 0x48u;
        }
        else if(lead_out == 0x000521f8ul || lead_out == 0x00057e3eul)
        {
            record_index = 0x31u;
        }
        else if(lead_out == 0x000521f9ul || lead_out == 0x00057e3bul)
        {
            record_index = 0x4bu;
        }
        else
        {
            record_index = 0x1au;
            if(bcd_second == 0x15u)
            {
                if(bcd_frame == 0x17u)
                    record_index = 0x28u;
                if(bcd_frame == 0x19u)
                    record_index = 0x2bu;
            }
            else if(bcd_second == 0x31u && bcd_frame == 0x09u)
            {
                record_index = 0x2bu;
            }
        }
        break;
    case 0xad97u:
        record_index = low_nibble == 7u ? 0x42u : 0x3au;
        break;
    case 0xae0fu:
        record_index = low_nibble == 6u ? 0x40u : 0x2eu;
        break;
    case 0xae19u:
        if(low_nibble >= 5u)
            record_index = 0x1eu;
        break;
    case 0xae64u:
        if(low_nibble >= 7u)
            record_index = 0x21u;
        break;
    case 0xae96u:
        if(low_nibble >= 5u)
            record_index = 0x0bu;
        break;
    case 0xaf4fu:
    case 0xaf9au:
        if(low_nibble < 5u)
        {
            if(low_nibble >= 2u)
                record_index = 0x49u;
        }
        else
        {
            if(low_nibble >= 6u)
                record_index = 0x23u;
            if(lead_out == 0x00057e38ul)
                record_index = 0x0fu;
        }
        break;
    case 0xaf90u:
        record_index = low_nibble < 5u ? 0u : 1u;
        break;
    case 0xafc2u:
        if(low_nibble < 5u)
            record_index = 0x11u;
        else if(low_nibble < 7u)
            record_index = 0x1cu;
        else
            record_index = 0x34u;
        break;
    case 0xb01cu:
    case 0xb68eu:
        record_index = 0x3eu;
        if(lead_in == (s32)-0x2db4L || lead_in == (s32)-0x2742L)
            record_index = 0x06u;
        else if(lead_out == 0x00052161ul || lead_out == 359999ul)
            record_index = 0x04u;
        else if(lead_out == 0x00052162ul || lead_out == 0x00057e3eul)
            record_index = 0x24u;
        break;
    case 0xb026u:
    case 0xb698u:
        if(low_nibble < 5u)
            record_index = 0x36u;
        else if(bcd_frame == 0x15u)
            record_index = 0x3fu;
        else
            record_index = 0x15u;
        break;
    case 0xb030u:
    case 0xb6acu:
        record_index = 0x51u;
        if(lead_in == (s32)-0x2d9bL)
            record_index = 0x38u;
        break;
    case 0xb03au:
    case 0xb64du:
        record_index = 0x39u;
        break;
    case 0xb07bu:
    case 0xb6e3u:
        if(low_nibble < 5u)
            record_index = bcd_frame < 0x24u ? 0x08u : 0x27u;
        else
            record_index = bcd_frame == 0x28u ? 0x01u : 0x2cu;
        break;
    case 0xb085u:
        if(low_nibble >= 5u)
            record_index = 0x05u;
        break;
    case 0xb0a3u:
    case 0xb67fu:
        record_index = 0x16u;
        break;
    case 0xb0b2u:
    case 0xb643u:
        record_index = 0x50u;
        if(lead_in == (s32)-0x2d18L)
            record_index = 0x2au;
        break;
    case 0xb0bcu:
    case 0xb701u:
        if(low_nibble < 5u)
        {
            record_index = 0x1fu;
            if(lead_out == 0x00057e3eul)
                record_index = 0x46u;
        }
        else
        {
            record_index = 0x22u;
        }
        break;
    case 0xb0c6u:
        if(low_nibble < 5u)
        {
            record_index = 0x14u;
        }
        else
        {
            record_index = 0x2fu;
            if(lead_out == 0x00057e3eul || lead_out == 0x0005265aul)
                record_index = 0x45u;
            if(bcd_frame == 0x25u)
                record_index = 0x01u;
        }
        break;
    case 0xb0dau:
    case 0xb6b6u:
        if(low_nibble < 5u)
            record_index = 0x0cu;
        else if(lead_out == 359999ul || lead_out == 0x000521f7ul)
            record_index = 0x29u;
        else
            record_index = 0x4cu;
        break;
    case 0xb0e4u:
    case 0xb760u:
        if(low_nibble < 5u)
            record_index = bcd_frame == 0x54u ? 0x20u : 0x3bu;
        else
            record_index = 0x3cu;
        break;
    case 0xb0eeu:
        if(low_nibble < 5u)
        {
            record_index = 0u;
        }
        else
        {
            record_index = 0x1du;
            if(lead_out == 0x00057e3eul || lead_out == 0x00057e3cul || lead_out == 0x0005294aul)
                record_index = 0x47u;
        }
        break;
    case 0xb0fdu:
        if(low_nibble < 5u)
        {
            if(low_nibble == 0u)
            {
                alternate_family = 1u;
                record_index = 1u;
            }
            else if(low_nibble == 1u)
            {
                record_index = 0x12u;
            }
        }
        else
        {
            record_index = lead_out == 359999ul ? 0x3du : 0x03u;
        }
        break;
    case 0xb107u:
    case 0xb738u:
        record_index = bcd_frame == 0x18u ? 0x4du : 0x12u;
        break;
    case 0xb111u:
    case 0xb6edu:
        record_index = 0x10u;
        break;
    case 0xb125u:
    case 0xb72eu:
        record_index = bcd_frame == 0x45u ? 0x09u : 0x2du;
        break;
    case 0xb139u:
    case 0xb724u:
        if(lead_out == 0x0005184cul || lead_out == 359999ul)
            record_index = 0x0au;
        else
            record_index = 0x35u;
        break;
    case 0xb152u:
    case 0xb779u:
        if(low_nibble < 5u)
            record_index = 0x1bu;
        else
            record_index = lead_out == 0x00057e3dul ? 0x17u : 0x41u;
        break;
    case 0xb15cu:
    case 0xb6a2u:
        record_index = lead_in == (s32)-0x2c6eL ? 0x44u : 0x13u;
        break;
    case 0xb1e8u:
        record_index = 0x0du;
        break;
    case 0xb27eu:
        if(low_nibble < 5u)
        {
            record_index = 0u;
        }
        else
        {
            record_index = 0x19u;
            if(low_nibble == 9u)
            {
                if(lead_out == 0x00057e3eul || lead_out == 0x00051d93ul)
                    record_index = 0x43u;
                else if(lead_out == 0x00057e3dul || lead_out == 0x00057e3cul)
                    record_index = 0x4fu;
            }
        }
        break;
    case 0xb31eu:
    case 0xb7ceu:
        if(low_nibble == 0u)
            record_index = 0u;
        else if(low_nibble == 1u)
            record_index = 0x07u;
        else if(low_nibble == 2u)
            record_index = 0x25u;
        else if(lead_out == 359999ul || lead_out == 0x00052161ul)
            record_index = 0x33u;
        else
            record_index = 0x4eu;
        break;
    case 0xb5cbu:
        if(low_nibble == 3u)
        {
            record_index = 0x19u;
            alternate_family = 1u;
        }
        if(low_nibble == 7u)
            record_index = 0x1au;
        break;
    case 0xb12fu:
    case 0xb756u:
        record_index = 0x4au;
        if(low_nibble == 6u)
            record_index = 0x26u;
        else if(low_nibble == 7u)
            record_index = 0x32u;
        else if(low_nibble == 5u)
            record_index = 0x02u;
        break;
    default:
        break;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, record_index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5E, alternate_family);
    if(alternate_family == 0u)
    {
        if(FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9491) + (fw_addr_t)record_index) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0u);
    }
    else if(FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9491) + (fw_addr_t)record_index) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 1u);
    }
    INITIALIZE_ATIP_STRATEGY();
}
