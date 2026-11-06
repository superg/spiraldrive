#include "raw_read_transfer_layout.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_transfer_layout_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadLayoutU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadLayoutU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteLayoutU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteLayoutU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void SetLayoutLengths(u16 transfer_length, u32 component_length)
{
    WriteLayoutU16(FW_NEAR_UNKNOWN_146D, transfer_length);
    WriteLayoutU32(FW_NEAR_UNKNOWN_1470, component_length);
}

static void PublishLayoutDescriptor(const u8 descriptor[4])
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, descriptor[0]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, descriptor[1]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, descriptor[2]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, descriptor[3]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
}

static void BuildLayoutDescriptor(u8 descriptor[4], u8 tag)
{
    LAYOUT_BUILD_TUPLE((s32)ReadLayoutU32(FW_NEAR_UNKNOWN_1598), descriptor);
    descriptor[3] = tag;
}

static void ApplyType2AuxiliaryFlags(u8 record, u8 substate, u8 substate_one_value, u8 substate_three_value)
{
    if((record & 0xc0u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 2u);
        WriteLayoutU32(FW_NEAR_UNKNOWN_1474, 0x60ul);
        WriteLayoutU16(FW_NEAR_UNKNOWN_146D, (u16)(ReadLayoutU16(FW_NEAR_UNKNOWN_146D) + 0x60u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc2u);
    }
    if(substate == 3u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, substate_three_value);
    else if(substate == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, substate_one_value);
}

static void ConfigureType8(u8 write_type, u8 record, u8 descriptor[4])
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    if(write_type == 2u)
    {
        switch(record & 0x3fu)
        {
        case 0x11u:
        case 0x13u:
            SetLayoutLengths(0x0930u, 0x930ul);
            break;
        case 0x10u:
        case 0x12u:
            SetLayoutLengths(0x0800u, 0x800ul);
            break;
        }
        ApplyType2AuxiliaryFlags(record, FirmwareReadNear8(FW_NEAR_UNKNOWN_146F), 0x0du, 8u);
    }
    else
    {
        SetLayoutLengths(0x0800u, 0x800ul);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0x46u);
    BuildLayoutDescriptor(descriptor, 1u);
}

static void ConfigureType9(u8 write_type, u8 record, u8 descriptor[4])
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0x4eu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    if(write_type == 2u)
    {
        switch(record & 0x3fu)
        {
        case 0x31u:
        case 0x33u:
            SetLayoutLengths(0x0930u, 0x930ul);
            break;
        case 0x30u:
        case 0x32u:
            SetLayoutLengths(0x0920u, 0x920ul);
            break;
        }
        ApplyType2AuxiliaryFlags(record, FirmwareReadNear8(FW_NEAR_UNKNOWN_146F), 0x0cu, 8u);
    }
    else
    {
        SetLayoutLengths(0x0920u, 0x920ul);
    }
    BuildLayoutDescriptor(descriptor, 2u);
}

static void ConfigureType13(u8 write_type, u8 record, u8 descriptor[4])
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0x46u);
    if(write_type == 2u)
    {
        switch(record & 0x3fu)
        {
        case 0x21u:
        case 0x23u:
            SetLayoutLengths(0x0930u, 0x930ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
            break;
        case 0x20u:
        case 0x22u:
            SetLayoutLengths(0x0920u, 0x920ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 1u);
            break;
        }
        ApplyType2AuxiliaryFlags(record, FirmwareReadNear8(FW_NEAR_UNKNOWN_146F), 0x0du, 0x0eu);
    }
    else
    {
        SetLayoutLengths(0x091cu, 0x91cul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0x20u);
    }
    LAYOUT_UPDATE_Q_TYPE();
    BuildLayoutDescriptor(descriptor, 2u);
}

/* RAM:fed50b */
void PLEXTOR_NEAR_FUNCTION ConfigureRawReadTransferLayoutFromQTypeCode(void)
{
    u8 descriptor[4];
    u8 q_type;
    u8 record = 0u;
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    u16 record_index;

    if(write_type == 2u)
    {
        record_index = ReadLayoutU16(FW_NEAR_UNKNOWN_149B);
        q_type = LAYOUT_DECODE(record_index);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, q_type);
        record = FirmwareRead8(FW_HUGE_UNKNOWN_00B4C0 + (u16)(record_index << 3));
    }
    else
    {
        q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);
    }

    descriptor[0] = 0u;
    descriptor[1] = 0u;
    descriptor[2] = 0u;
    descriptor[3] = 0u;

    switch(q_type)
    {
    case 0u:
        SetLayoutLengths(0x0930u, 0x930ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0u);
        break;
    case 1u:
        WriteLayoutU16(FW_NEAR_UNKNOWN_146D, 0x0940u);
        WriteLayoutU32(FW_NEAR_UNKNOWN_1470, 0x930ul);
        WriteLayoutU32(FW_NEAR_UNKNOWN_1474, 0x10ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0u);
        break;
    case 2u:
    case 3u:
        WriteLayoutU16(FW_NEAR_UNKNOWN_146D, 0x0990u);
        WriteLayoutU32(FW_NEAR_UNKNOWN_1470, 0x930ul);
        WriteLayoutU32(FW_NEAR_UNKNOWN_1474, 0x60ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0u);
        break;
    case 8u:
        ConfigureType8(write_type, record, descriptor);
        break;
    case 9u:
        ConfigureType9(write_type, record, descriptor);
        break;
    case 10u:
        SetLayoutLengths(0x0800u, 0x800ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0x47u);
        LAYOUT_UPDATE_Q_TYPE();
        BuildLayoutDescriptor(descriptor, 2u);
        break;
    case 11u:
        SetLayoutLengths(0x0808u, 0x808ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0x46u);
        LAYOUT_UPDATE_Q_TYPE();
        BuildLayoutDescriptor(descriptor, 2u);
        break;
    case 12u:
        SetLayoutLengths(0x0914u, 0x914ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0x47u);
        LAYOUT_UPDATE_Q_TYPE();
        BuildLayoutDescriptor(descriptor, 2u);
        break;
    case 13u:
        ConfigureType13(write_type, record, descriptor);
        break;
    case 0x41u:
        SetLayoutLengths(0x0060u, 0x60ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0u);
        break;
    default:
        return;
    }
    PublishLayoutDescriptor(descriptor);
}
