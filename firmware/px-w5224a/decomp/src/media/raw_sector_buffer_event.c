#include "raw_sector_buffer_event.h"
#include "../platform/firmware_memory.h"
#include "../platform/raw_sector_descriptor_scratch.h"
#include "../platform/status_mapping.h"
#include "raw_read_descriptor_completion.h"

extern void EnqueueRawSectorDescriptor(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0018ForRawSectorBufferEvent(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0019ForRawSectorBufferEvent(void);

static u16 ReadBufferEventWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteBufferEventWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadBufferEventDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBufferEventDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void ServiceBufferEventAndReturn(void)
{
    ServiceParsedQDescriptorCountdownAndPositionAdvance();
}

static u8 IsBufferEventPositionOutsideWindow(void)
{
    s32 current = (s32)ReadBufferEventDword(FW_NEAR_UNKNOWN_1235);
    s32 target = (s32)ReadBufferEventDword(FW_NEAR_UNKNOWN_1598);

    if(target < current)
        return (s32)((u32)current - (u32)target) >= 8l ? 1u : 0u;
    return (s32)((u32)target - (u32)current) >= 3l ? 1u : 0u;
}

static void ProcessBufferEventMetadata(void)
{
    u8 metadata[10];
    u8 index;
    u32 cursor = ReadBufferEventDword(FW_NEAR_UNKNOWN_156E) + ReadBufferEventDword(FW_NEAR_UNKNOWN_1262) * 16ul + FW_HUGE_BUFFER_BANK_BASE;

    WriteBufferEventDword(FW_NEAR_UNKNOWN_15D8, cursor);
    for(index = 0u; index < 10u; ++index)
    {
        metadata[index] = FirmwareRead8(cursor);
        ++cursor;
        WriteBufferEventDword(FW_NEAR_UNKNOWN_15D8, cursor);
    }

    if((metadata[0] & 0x0fu) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) != 0u)
    {
        for(index = 0u; index < 10u; ++index)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A4 + index, metadata[index]);
    }
    else
    {
        AdvanceRawSectorQPositionByOneFrame();
        cursor -= 10ul;
        WriteBufferEventDword(FW_NEAR_UNKNOWN_15D8, cursor);
        for(index = 0u; index < 10u; ++index)
        {
            FirmwareWrite8(cursor, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A4 + index));
            ++cursor;
            WriteBufferEventDword(FW_NEAR_UNKNOWN_15D8, cursor);
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F30, (FirmwareReadNear8(FW_NEAR_UNKNOWN_15A4) & 0x40u) != 0u ? 1u : 0u);
    if(FirmwareReadBit001A() != 0u && (s16)ReadBufferEventWord(FW_NEAR_UNKNOWN_1292) > 0)
    {
        FirmwareWriteBit001A(0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807C, 0u);
    }
}

/* RAM:f92d0b */
void PLEXTOR_NEAR_FUNCTION ProcessRawSectorBufferEvent(void)
{
    u8 retry_count;
    u32 target;

    if((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 7) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0F75))
    {
        ServiceBufferEventAndReturn();
        return;
    }

    if(FirmwareReadBit0013() == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F62) == 0u)
    {
        u8 format = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);

        retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22);
        if(FirmwareReadBit000E() == 0u && format >= 1u && format <= 3u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2E) & 0x40u) == 0u)
            && (s8)retry_count < 3)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, (u8)(retry_count + 1u));
            ServiceBufferEventAndReturn();
            return;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F27) & 0x40u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u)
        {
            retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22);
            if((s8)retry_count < 2)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, (u8)(retry_count + 1u));
                ServiceBufferEventAndReturn();
                return;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) + 1u));
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 1u) == 0u)
        {
            ServiceBufferEventAndReturn();
            return;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < 0x34u && ReadBufferEventDword(FW_NEAR_UNKNOWN_126E) % 0x0930ul != 0ul)
        {
            ServiceBufferEventAndReturn();
            return;
        }

        retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22);
        if((s8)retry_count < 10 && IsBufferEventPositionOutsideWindow() != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, (u8)(retry_count + 1u));
            ServiceBufferEventAndReturn();
            return;
        }

        target = ReadBufferEventDword(FW_NEAR_UNKNOWN_1598);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 4u) != 0u && (s32)target >= (s32)(ReadBufferEventDword(FW_NEAR_UNKNOWN_12C1) + 100ul))
        {
            BuildRawSectorDescriptorScratch(0x6300u, 5u);
            EnqueueRawSectorDescriptor();
            InitializeControllerBufferCursorRegistersIfNeeded();
            return;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 2u)
    {
        ServiceBufferEventAndReturn();
        return;
    }

    if(FirmwareReadBit000E() != 0u)
        ProcessBufferEventMetadata();

    BuildRawSectorDescriptorScratch(0u, 0u);
    EnqueueRawSectorDescriptor();
    target = ReadBufferEventDword(FW_NEAR_UNKNOWN_1598) + 1ul;
    WriteBufferEventDword(FW_NEAR_UNKNOWN_1598, target);

    if(ReadBit0018ForRawSectorBufferEvent() != 0u)
    {
        u16 old_scan_count = ReadBufferEventWord(FW_NEAR_UNKNOWN_0F5E);

        WriteBufferEventWord(FW_NEAR_UNKNOWN_0F5E, (u16)(old_scan_count + 1u));
        if(old_scan_count > 0x12u)
        {
            WriteBufferEventWord(FW_NEAR_UNKNOWN_0F5E, 0u);
            if(ReadBit0019ForRawSectorBufferEvent() != 0u)
                target -= 0x80ul;
            else
                target += 0x6eul;
            WriteBufferEventDword(FW_NEAR_UNKNOWN_1598, target);
            WriteBufferEventDword(FW_NEAR_UNKNOWN_0F36, ReadBufferEventDword(FW_NEAR_UNKNOWN_0F36) - 0x6eul);
            InitializeControllerBufferCursorRegistersIfNeeded();
            return;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0x19u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) >= 10u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x28u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) >= 0x28u
        && (u16)(ReadBufferEventWord(FW_NEAR_UNKNOWN_129C) - 75u) < ReadBufferEventWord(FW_NEAR_UNKNOWN_1292))
    {
        InitializeControllerBufferCursorRegistersIfNeeded();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 8u);
    }
    if(IsRawReadQueueAtHighWater() != 0u)
    {
        InitializeControllerBufferCursorRegistersIfNeeded();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F61, 4u);
    }
}
