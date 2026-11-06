#include "indexed_state0f_coroutine.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/indexed_state0f_coroutine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadIndexedU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteIndexedU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void WaitForIndexedMediaReady(void)
{
    INDEXED_YIELD();
    INDEXED_YIELD();
    INDEXED_YIELD();
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
        INDEXED_YIELD();
}

static void EnterIndexedTimedFailureLoop(u8 status)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B8, status);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) & 0xf3u) | 4u));
    INDEXED_TIMED_LOOP();
}

/* RAM:fb3ed6 */
void RunIndexedState0FOperationCoroutine0(void)
{
    u8 strategy;
    u32 block_count = 180000ul;
    u32 packet_overhead;

    WaitForIndexedMediaReady();
    strategy = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D);
    if(strategy == 0x91u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
    else if(strategy == 0xb1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 10u);
    else if(strategy == 0xd1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 24u);
    else
    {
        EnterIndexedTimedFailureLoop(1u);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C44, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C45, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) | 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C47, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C48, 10u);
    WriteIndexedU32(FW_NEAR_UNKNOWN_0C4B, 32ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E5FB, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E5FA, 0u);
    WriteIndexedU32(FW_NEAR_UNKNOWN_E5FC, block_count);
    packet_overhead = INDEXED_MULTIPLY(7ul, INDEXED_DIVIDE(block_count, ReadIndexedU32(FW_NEAR_UNKNOWN_0C4B))) + 0x91ul;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 8u);
    WriteIndexedU32(FW_NEAR_UNKNOWN_E612, ReadIndexedU32(FW_NEAR_UNKNOWN_114F));
    WriteIndexedU32(FW_NEAR_UNKNOWN_E616, 0xfffffffful);
    WriteIndexedU32(FW_NEAR_UNKNOWN_E61A, block_count + packet_overhead);
    WriteIndexedU32(FW_NEAR_UNKNOWN_E61E, ReadIndexedU32(FW_NEAR_UNKNOWN_E61A) + 0x1a5eul);
    INDEXED_RESET_TRACKS(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, 1u);
    INDEXED_LOAD_TRACK(1u);

    if(INDEXED_EXECUTE() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        EnterIndexedTimedFailureLoop(4u);
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    INDEXED_INIT_COROUTINE1(FW_ENTRY_PROCESS_DRIVE_STATE_MACHINE);
    WaitForIndexedMediaReady();
    if(INDEXED_VALIDATE() == 0u)
    {
        EnterIndexedTimedFailureLoop(6u);
        return;
    }
    INDEXED_INIT_COROUTINE1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
    INDEXED_YIELD_LOOP();
}
