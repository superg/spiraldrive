#include "raw_read_operation_mode_speed.h"
#include "../platform/coroutine_initialization.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/noop_handlers.h"
#include "../platform/raw_read_opc.h"
#include "../platform/status_mapping.h"
#include "repeated_raw_read_range_windows.h"
#include "synthetic_raw_read_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_operation_mode_speed_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRawOperationU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadRawOperationU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRawOperationU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRawOperationU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void ResetRawOperationQueue(void)
{
    u32 ring_slot_count;

    WriteRawOperationU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteRawOperationU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteRawOperationU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteRawOperationU16(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slot_count = ReadRawOperationU32(FW_NEAR_UNKNOWN_156A);
    WriteRawOperationU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    ring_slot_count = ReadRawOperationU32(FW_NEAR_UNKNOWN_156A);
    WriteRawOperationU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteRawOperationU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteRawOperationU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteRawOperationU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteRawOperationU32(FW_NEAR_UNKNOWN_126A, ReadRawOperationU32(FW_NEAR_UNKNOWN_1562));
    WriteRawOperationU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CB, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
}

static void WaitForRawOperationReadiness(void)
{
    if(RAW_OPERATION_INDEXED(0x1fu) == 1u)
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 2) & 1u) != 1u)
            RAW_OPERATION_YIELD();
    else
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u)
            RAW_OPERATION_YIELD();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 10u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) > 0)
        RAW_OPERATION_YIELD();
    RAW_OPERATION_NOOP();
}

static u8 SelectRawOperationSpeed(u8 operation_mode, u8 requested_speed_x)
{
    u8 state;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xdfu));
        if(operation_mode == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            {
                requested_speed_x = 24u;
            }
            else
            {
                state = FirmwareReadNear8(FW_NEAR_UNKNOWN_130A);
                if(state == 3u || state == 4u)
                    requested_speed_x = 10u;
                else
                    requested_speed_x = 4u;
            }
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) == 0u)
                requested_speed_x = 4u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
            requested_speed_x = 4u;
    }

    if(operation_mode == 4u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
        {
            requested_speed_x = 24u;
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
        {
            if(RAW_OPERATION_BIT0406() != 0u)
                requested_speed_x = 10u;
            else if(RAW_OPERATION_BIT0402() != 0u)
                requested_speed_x = 8u;
            else
                requested_speed_x = 4u;
        }
        else
        {
            requested_speed_x = 4u;
        }
    }
    return requested_speed_x;
}

static u8 FailRawOperationOpc(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 8u);
    if(((ReadRawOperationU16(FW_NEAR_UNKNOWN_FAD7) >> 3) & 1u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 7u);
    return 0u;
}

/* RAM:feb8b4 */
u8 PLEXTOR_NEAR_FUNCTION ExecuteRawReadOperationForModeAndSpeed(u8 operation_mode, u8 requested_speed_x)
{
    u8 indexed_state;

    ResetRawOperationQueue();
    WaitForRawOperationReadiness();
    requested_speed_x = SelectRawOperationSpeed(operation_mode, requested_speed_x);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, requested_speed_x);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, requested_speed_x);
    RAW_OPERATION_NORMALIZE();

    indexed_state = RAW_OPERATION_INDEXED(0x1fu);
    if(indexed_state == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) | 0x08u));
        indexed_state = RAW_OPERATION_OPC();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xf7u));
        if(indexed_state == 0u)
            return FailRawOperationOpc();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
    }

    if(RAW_OPERATION_OPC() == 0u)
        return FailRawOperationOpc();
    RAW_OPERATION_BUILD_TDI(FW_NEAR_UNKNOWN_143C);

    if(operation_mode == 0u)
        return (u8)(RAW_OPERATION_PREPARE() != 0u);
    if(operation_mode == 3u)
    {
        while(RAW_OPERATION_REPEAT((u32)ReadRawOperationU16(FW_NEAR_UNKNOWN_F9DF)) != 0u)
        {
        }
        return 0u;
    }
    if(operation_mode == 0x1fu)
    {
        while(RAW_OPERATION_REPEAT(10ul) != 0u)
        {
        }
        return 0u;
    }
    if(operation_mode == 4u)
    {
        if(RAW_OPERATION_REPEAT(30ul) == 0u)
            return 0u;
        RAW_OPERATION_INITIALIZE(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
    }
    return 1u;
}
