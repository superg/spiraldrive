#include "controller_buffer_setup.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_buffer_setup_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadSetupU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteSetupU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadSetupU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteSetupU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void WriteControllerMode(u8 mode, u8 control)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, mode);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, mode);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, control);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, control);
}

static void WriteControllerAddress3(u32 address)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018007, (u8)address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, (u8)(address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, (u8)(address >> 16));
}

static void WriteControllerAddress3Plus12(void)
{
    u32 address = ReadSetupU32(FW_NEAR_UNKNOWN_FBAA) + 12ul;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, (u8)address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800B, (u8)(address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800C, (u8)(address >> 16));
}

static void WriteControllerAddress4(u16 address)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801C, (u8)address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801D, (u8)(address >> 8));
}

static void CacheCurrentDescriptorContext(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBC2, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBB6));
    WriteSetupU32(FW_NEAR_UNKNOWN_FBBE, ReadSetupU32(FW_NEAR_UNKNOWN_FBB0));
}

static void AdvanceRawReadConsumer(void)
{
    WriteSetupU16(FW_NEAR_UNKNOWN_1294, (u16)((ReadSetupU16(FW_NEAR_UNKNOWN_1294) + 1u) & ReadSetupU16(FW_NEAR_UNKNOWN_129A)));
    WriteSetupU16(FW_NEAR_UNKNOWN_1292, (u16)(ReadSetupU16(FW_NEAR_UNKNOWN_1292) - 1u));
}

static void ConsumeCurrentDescriptor(void)
{
    AdvanceRawReadConsumer();
    SETUP_LOAD_CURRENT();
}

static void StartReadMeasurementSubstate(void)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 1u);
}

static void PublishAddress2(u8 value)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, value);
}

static void PublishAddress3(u8 value)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, value);
}

static u8 FinishDescriptorIfRequired(void)
{
    if(SETUP_READ_BIT(0x28u) != 0u && (SETUP_READ_BIT(0x30u) == 1u || (SETUP_READ_BIT(0x36u) != 0u && SETUP_READ_BIT(0x35u) != 0u)) && ReadSetupU16(FW_NEAR_UNKNOWN_FBB4) < 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 12u);
        SETUP_WRITE_BIT(0x32u, 1u);
        if(SETUP_READ_BIT(0x36u) != 1u)
            WriteSetupU16(FW_NEAR_UNKNOWN_FBD7, (u16)(ReadSetupU16(FW_NEAR_UNKNOWN_FBD7) + 1u));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 15u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14C0) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 10u);
        SETUP_WRITE_BIT(0x30u, 0u);
        SETUP_WRITE_BIT(0x33u, 0u);
        SETUP_WRITE_BIT(0x32u, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 15u);
        return 1u;
    }

    if(ReadSetupU16(FW_NEAR_UNKNOWN_FBB4) == 1u || ReadSetupU32(FW_NEAR_UNKNOWN_FBA2) == ReadSetupU32(FW_NEAR_UNKNOWN_14BC))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C0, 1u);
        WriteSetupU32(FW_NEAR_UNKNOWN_14DE, ReadSetupU32(FW_NEAR_UNKNOWN_FBA2));
        return 1u;
    }
    return 0u;
}

static u8 ConfigureComplexDescriptorMode(u8 mode, u8 publish_address)
{
    if(mode == 8u)
        WriteControllerMode(0x52u, 0x40u);
    else if(mode == 10u || mode == 11u)
        WriteControllerMode(0x52u, 0x42u);
    else if(mode == 12u)
        WriteControllerMode(0x12u, 0x43u);
    else if(mode == 13u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBB7) == 0u)
            WriteControllerMode(0x52u, 0x42u);
        else
            WriteControllerMode(0x12u, 0x43u);
    else
        return 0u;
    if(publish_address != 0u)
        WriteControllerAddress3Plus12();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 10u);
    return 1u;
}

static void RouteDescriptorMode(u8 route_zero, u8 route_one, u8 route_three, u8 route_nine, u8 publish_simple_mode, u8 publish_complex_address)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E);

    if(mode == 0u && route_zero != 0u)
    {
        if(publish_simple_mode != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u), FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 1u);
    }
    else if(mode == 1u && route_one != 0u)
    {
        if(publish_simple_mode != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u), FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 2u);
    }
    else if(mode == 3u && route_three != 0u)
    {
        if(publish_simple_mode != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u), FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 4u);
    }
    else if(mode == 9u && route_nine != 0u)
    {
        if(publish_simple_mode != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u), FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 1u);
    }
    else
    {
        (void)ConfigureComplexDescriptorMode(mode, publish_complex_address);
    }
}

static void LoadLookaheadAndSelectPhase(u8 next_phase)
{
    SETUP_LOAD_LOOKAHEAD();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBBA) == 2u)
    {
        WriteControllerAddress4(ReadSetupU16(FW_NEAR_UNKNOWN_FBBB));
        PublishAddress2((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0xe4u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, next_phase);
    }
}

static void ServiceActiveDescriptorPhase(u8 phase)
{
    u8 lookahead_phase;

    if(phase == 1u)
    {
        PublishAddress3((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu));
        SETUP_WRITE_BIT(0x2bu, 1u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E) == 0u)
            WriteControllerMode(2u, 0x47u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E) == 9u)
            WriteControllerMode(2u, 0x40u);
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        if(FinishDescriptorIfRequired() != 0u)
            return;
        RouteDescriptorMode(0u, 1u, 1u, 0u, 0u, 1u);
        lookahead_phase = 10u;
    }
    else if(phase == 3u)
    {
        SETUP_WRITE_BIT(0x2bu, 1u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) != 3u)
            CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        if(FinishDescriptorIfRequired() != 0u)
            return;
        RouteDescriptorMode(1u, 0u, 1u, 1u, 0u, 1u);
        lookahead_phase = 11u;
    }
    else if(phase == 5u)
    {
        SETUP_WRITE_BIT(0x2bu, 1u);
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        if(FinishDescriptorIfRequired() != 0u)
            return;
        RouteDescriptorMode(1u, 1u, 0u, 1u, 0u, 1u);
        lookahead_phase = 15u;
    }
    else if(phase == 10u)
    {
        PublishAddress3((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu));
        SETUP_WRITE_BIT(0x2bu, 1u);
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        if(FinishDescriptorIfRequired() != 0u)
            return;
        RouteDescriptorMode(1u, 1u, 1u, 1u, 1u, 0u);
        lookahead_phase = 13u;
    }
    else if(phase == 14u)
    {
        PublishAddress3((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu));
        SETUP_WRITE_BIT(0x2bu, 1u);
        WriteControllerMode(2u, 0x47u);
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        if(FinishDescriptorIfRequired() != 0u)
            return;
        RouteDescriptorMode(1u, 1u, 1u, 1u, 0u, 1u);
        return;
    }
    else
    {
        SETUP_WRITE_BIT(0x2bu, 1u);
        WriteControllerMode(2u, 0x47u);
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        if(FinishDescriptorIfRequired() != 0u)
            return;
        RouteDescriptorMode(1u, 1u, 1u, 1u, 0u, 1u);
        lookahead_phase = 10u;
    }
    LoadLookaheadAndSelectPhase(lookahead_phase);
}

static void ServiceInitialPhase(void)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E);
    u16 ring_address;

    if(mode == 1u || mode == 3u || mode == 9u || mode == 0u)
    {
        WriteControllerMode(2u, mode == 9u ? 0x40u : 0x47u);
        WriteControllerAddress3(ReadSetupU32(FW_NEAR_UNKNOWN_FBAA));
        if(mode == 1u)
        {
            WriteControllerAddress4(ReadSetupU16(FW_NEAR_UNKNOWN_FBAE));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 3u)
            {
                PublishAddress2((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0x84u));
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBC2) == 0u ? 6u : 7u);
            }
            else
            {
                CacheCurrentDescriptorContext();
            }
        }
        else if(mode == 3u)
        {
            ring_address = (u16)(ReadSetupU16(FW_NEAR_UNKNOWN_FBAE) + 2u);
            if((u32)ring_address >= ReadSetupU32(FW_NEAR_UNKNOWN_156A))
                ring_address = (u16)(ring_address - (u16)ReadSetupU32(FW_NEAR_UNKNOWN_156A));
            WriteSetupU16(FW_NEAR_UNKNOWN_FBAE, ring_address);
            WriteControllerAddress4(ring_address);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 3u)
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 8u);
            PublishAddress2((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0x84u));
            CacheCurrentDescriptorContext();
        }
        else
        {
            CacheCurrentDescriptorContext();
        }
        StartReadMeasurementSubstate();
        ConsumeCurrentDescriptor();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, mode == 1u ? 3u : mode == 3u ? 5u : 1u);
    }
    else if(mode == 2u)
    {
        WriteControllerMode(2u, 0x47u);
        PublishAddress2((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0xe4u));
        WriteControllerAddress4(ReadSetupU16(FW_NEAR_UNKNOWN_FBAE));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 11u);
    }
    else if(mode == 8u || mode == 10u || mode == 11u || mode == 12u || mode == 13u)
    {
        (void)ConfigureComplexDescriptorMode(mode, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 6u);
    }
    else if(mode == 14u)
    {
        WriteControllerAddress3(ReadSetupU32(FW_NEAR_UNKNOWN_FBAA));
        WriteControllerMode(2u, 0x47u);
        CacheCurrentDescriptorContext();
        WriteControllerAddress4(ReadSetupU16(FW_NEAR_UNKNOWN_FBAE));
        PublishAddress2((u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) & 0x9fu) | 0x84u));
        PublishAddress3((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) | 0x10u));
        StartReadMeasurementSubstate();
        ConsumeCurrentDescriptor();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 16u);
    }
}

/* RAM:fb65d8 */
void PLEXTOR_NEAR_FUNCTION ServiceControllerBufferSetupStateMachine(void)
{
    u8 phase;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
        WriteSetupU32(FW_NEAR_UNKNOWN_150B, ReadSetupU32(FW_NEAR_UNKNOWN_150B) + 1ul);
    phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA1);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_154C, phase);
    if(SETUP_READ_BIT(0x2bu) != 0u)
        WriteSetupU16(FW_NEAR_UNKNOWN_FBC7, (u16)(ReadSetupU16(FW_NEAR_UNKNOWN_FBC7) + 1u));

    if(phase == 0u)
    {
        ServiceInitialPhase();
    }
    else if(phase == 1u || phase == 3u || phase == 5u || phase == 10u || phase == 14u || phase == 16u)
    {
        ServiceActiveDescriptorPhase(phase);
    }
    else if(phase == 2u)
    {
        PublishAddress3((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu));
        WriteControllerMode(2u, 0x47u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 3u)
        {
            PublishAddress2((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0x84u));
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBC2) == 0u ? 6u : 7u);
        }
        else
        {
            CacheCurrentDescriptorContext();
        }
        WriteControllerAddress4(ReadSetupU16(FW_NEAR_UNKNOWN_FBAE));
        ConsumeCurrentDescriptor();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 3u);
    }
    else if(phase == 4u)
    {
        PublishAddress3((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu));
        WriteControllerMode(2u, 0x47u);
        {
            u16 address = (u16)(ReadSetupU16(FW_NEAR_UNKNOWN_FBAE) + 2u);
            if((u32)address >= ReadSetupU32(FW_NEAR_UNKNOWN_156A))
                address = (u16)(address - (u16)ReadSetupU32(FW_NEAR_UNKNOWN_156A));
            WriteSetupU16(FW_NEAR_UNKNOWN_FBAE, address);
            WriteControllerAddress4(address);
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 3u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 8u);
        PublishAddress2((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) | 0x84u));
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 5u);
    }
    else if(phase == 6u || phase == 9u)
    {
        WriteControllerAddress3(ReadSetupU32(FW_NEAR_UNKNOWN_FBAA));
        CacheCurrentDescriptorContext();
        StartReadMeasurementSubstate();
        ConsumeCurrentDescriptor();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, phase == 6u ? 10u : 14u);
    }
    else if(phase == 11u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 9u);
    }
    else if(phase == 12u || phase == 13u)
    {
        if(phase == 13u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u), FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA1, 14u);
    }

    if(phase == 15u)
    {
        SETUP_WRITE_BIT(0x2bu, 0u);
        WriteControllerMode(2u, 0u);
        SETUP_WRITE_BIT(0x43u, 0u);
        if(SETUP_READ_BIT(0x32u) == 1u)
            WriteSetupU32(FW_NEAR_UNKNOWN_14DE, ReadSetupU32(FW_NEAR_UNKNOWN_1235) - 2ul);
    }
}

/* RAM:fb7d5a */
void PLEXTOR_NEAR_FUNCTION AdvanceControllerBufferSetupPhase(void)
{
    u8 phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_1531);

    if(phase == 0u)
    {
        u32 address = ReadSetupU32(FW_NEAR_UNKNOWN_1525);

        WriteControllerMode(0x52u, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, (u8)address);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800B, (u8)(address >> 8));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800C, (u8)(address >> 16));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1531, 1u);
    }
    else if(phase == 1u)
    {
        WriteControllerAddress3(ReadSetupU32(FW_NEAR_UNKNOWN_FBAA));
        CacheCurrentDescriptorContext();
        StartReadMeasurementSubstate();
        ConsumeCurrentDescriptor();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1529, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_152A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_152B, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018036, 0x41u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018037, 1u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018038, 1u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018039, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803B, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803C, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803D, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802B));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803E, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802C));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803F, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802D));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1531, 2u);
    }
    else if(phase == 2u)
    {
        CacheCurrentDescriptorContext();
        ConsumeCurrentDescriptor();
        SETUP_INCREMENT_MSF(FW_NEAR_UNKNOWN_1529);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_152D, SETUP_BINARY_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_1529)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_152E, SETUP_BINARY_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_152A)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_152F, SETUP_BINARY_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_152B)));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018039, FirmwareReadNear8(FW_NEAR_UNKNOWN_152D));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, FirmwareReadNear8(FW_NEAR_UNKNOWN_152E));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803B, FirmwareReadNear8(FW_NEAR_UNKNOWN_152F));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803D, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802B));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803E, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802C));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803F, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802D));
    }
}
