#include "timer_phase2.h"
#include "firmware_memory.h"
#include "timer_debounce.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/timer_phase2_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void DecrementTimerByte(fw_addr_t address)
{
    FirmwareWrite8(address, (u8)(FirmwareRead8(address) - 1u));
}

static u16 ReadTimerU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteTimerU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadTimerU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteTimerU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ServiceTimerSerialHandshakes(void)
{
    u8 value;

    DecrementTimerByte(FW_NEAR_UNKNOWN_1210);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x08u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x10u) == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1210) < 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1210, 2u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x04u) != 0u)
        {
            WRITE_P3_BIT0(1u);
            if(READ_P3_BIT1() != 0u)
                WRITE_P3_BIT1(0u);
            else
                WRITE_P3_BIT1(1u);
        }
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEE) + 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEE, value);
        if(value >= 6u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) | 0x10u));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x10u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) & 7u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1211, 30u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) | 0x10u));
            }
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x20u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x04u) != 0u)
                ServiceP1Bit6DriveStateTransition();
        }
    }

    DecrementTimerByte(FW_NEAR_UNKNOWN_1211);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x20u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x40u) == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1211) < 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1211, 2u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 1u) != 0u)
        {
            if(READ_P3_BIT0() != 0u)
                WRITE_P3_BIT0(0u);
            else
                WRITE_P3_BIT0(1u);
            WRITE_P3_BIT1(1u);
        }
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAF0) + 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAF0, value);
        if(value >= 6u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) | 0x40u));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x20u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 1u) != 0u)
                ServiceP1Bit6DriveStateTransition();
        }
    }
}

static void ServiceSlowerTimerCountdowns(void)
{
    u8 old_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_120D);
    u16 signed_countdown;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120D, (u8)(old_value - 1u));
    if((s8)old_value <= 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_120D, 9u);
        DecrementTimerByte(FW_NEAR_UNKNOWN_1204);
        DecrementTimerByte(FW_NEAR_UNKNOWN_1203);
        DecrementTimerByte(FW_NEAR_UNKNOWN_1202);
        DecrementTimerByte(FW_NEAR_UNKNOWN_0FB8);
        if(READ_BIT003E() != 0u)
            WRITE_BIT003E(0u);
        else
            WRITE_BIT003D(1u);
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x08u) == 0x08u)
            DecrementTimerByte(FW_NEAR_HOST_FAILURE_COUNTDOWN);
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 1u) == 1u)
            DecrementTimerByte(FW_NEAR_HOST_ACQUISITION_COUNTDOWN);
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_122D) > 0)
        {
            DecrementTimerByte(FW_NEAR_UNKNOWN_122D);
        }
        else if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x10u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 4u));
            FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xefu));
        }
        WriteTimerU32(FW_NEAR_UNKNOWN_11F4, ReadTimerU32(FW_NEAR_UNKNOWN_11F4) + 1ul);
        signed_countdown = (u16)(ReadTimerU16(FW_NEAR_UNKNOWN_11DA) - 1u);
        WriteTimerU16(FW_NEAR_UNKNOWN_11DA, signed_countdown);
        if((s16)signed_countdown < 0)
            WriteTimerU16(FW_NEAR_UNKNOWN_11DA, 0xffffu);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 1u)
            WriteTimerU32(FW_NEAR_UNKNOWN_11F0, ReadTimerU32(FW_NEAR_UNKNOWN_11F0) + 1ul);
    }

    old_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_120E);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120E, (u8)(old_value - 1u));
    if((s8)old_value <= 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_120E, 4u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1226) != 0u)
    {
        DecrementTimerByte(FW_NEAR_UNKNOWN_1226);
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1226) <= 0)
        {
            WRITE_P3_BIT0(1u);
            WRITE_P3_BIT1(0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
        }
    }
}

static void ServiceDividedTimerCountdowns(void)
{
    u16 word_value;

    DecrementTimerByte(FW_NEAR_UNKNOWN_120F);
    DecrementTimerByte(FW_NEAR_UNKNOWN_1208);
    DecrementTimerByte(FW_NEAR_UNKNOWN_120A);
    DecrementTimerByte(FW_NEAR_UNKNOWN_1209);
    DecrementTimerByte(FW_NEAR_UNKNOWN_1207);
    DecrementTimerByte(FW_NEAR_UNKNOWN_1205);
    DecrementTimerByte(FW_NEAR_UNKNOWN_1206);
    DecrementTimerByte(FW_NEAR_UNKNOWN_120B);
    DecrementTimerByte(FW_NEAR_UNKNOWN_1224);
    DecrementTimerByte(FW_NEAR_UNKNOWN_130B);
    DecrementTimerByte(FW_NEAR_UNKNOWN_120C);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_122E) > 0)
        DecrementTimerByte(FW_NEAR_UNKNOWN_122E);
    word_value = ReadTimerU16(FW_NEAR_UNKNOWN_1004);
    if(word_value != 0u)
        WriteTimerU16(FW_NEAR_UNKNOWN_1004, (u16)(word_value - 1u));

    ServiceTimerSerialHandshakes();
    ServiceSlowerTimerCountdowns();
}

static void ServicePeriodicP3Pattern(void)
{
    u8 mode = 0u;
    u8 trigger = 0u;
    u8 tick;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x10u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u))
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x20u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51) >= 0x29u)
                mode = 1u;
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51) >= 0x1fu)
                mode = 2u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
            mode = 0u;
    }
    else
    {
        u8 state = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88);

        if(state == 2u || state == 3u)
            mode = 1u;
        else if(state == 1u)
            mode = 2u;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 8u) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u)
        mode = 0u;

    tick = FirmwareReadNear8(FW_NEAR_UNKNOWN_121D);
    if((mode == 2u && tick >= 8u) || (mode == 1u && tick >= 4u) || (mode == 0u && tick >= 15u))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121D, 0u);
        trigger = 1u;
    }
    if(trigger == 0u)
        return;

    if(READ_P3_BIT0() != 0u)
    {
        WRITE_P3_BIT0(0u);
        WRITE_P3_BIT1(1u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 8u) != 0u)
        {
            WRITE_P3_BIT0(0u);
            WRITE_P3_BIT1(0u);
        }
    }
    else
    {
        WRITE_P3_BIT0(1u);
        WRITE_P3_BIT1(1u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u)
        {
            WRITE_P3_BIT0(1u);
            WRITE_P3_BIT1(0u);
        }
    }
}

/* RAM:fd1693 */
void TimerDispatchPhase2(void)
{
    static const u16 byte_countdowns[] = { 0x121eu, 0x1216u, 0x1217u, 0x1215u, 0x1213u, 0x1214u, 0x121au, 0x0fd1u, 0x0fbdu, 0x1219u };
    u8 index;
    u8 old_value;
    u8 value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_121D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_121D) + 1u));
    for(index = 0u; index < sizeof(byte_countdowns) / sizeof(byte_countdowns[0]); ++index)
        DecrementTimerByte(byte_countdowns[index]);
    WriteTimerU16(FW_NEAR_UNKNOWN_1200, (u16)(ReadTimerU16(FW_NEAR_UNKNOWN_1200) - 1u));
    DecrementTimerByte(FW_NEAR_UNKNOWN_121B);
    DecrementTimerByte(FW_NEAR_UNKNOWN_121C);
    WriteTimerU16(FW_NEAR_UNKNOWN_1034, (u16)(ReadTimerU16(FW_NEAR_UNKNOWN_1034) - 1u));
    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1218) - 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1218, value);
    if((s8)value <= 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1218, 0u);
    DecrementTimerByte(FW_NEAR_UNKNOWN_FBD5);

    old_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1212);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1212, (u8)(old_value - 1u));
    if((s8)old_value <= 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1212, 9u);
        ServiceDividedTimerCountdowns();
    }

    ServicePeriodicP3Pattern();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11DC) & 1u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1212) == 9u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1212) == 4u))
    {
        if(READ_P3_BIT1() != 0u)
        {
            WRITE_P3_BIT0(1u);
            WRITE_P3_BIT1(0u);
        }
        else
        {
            WRITE_P3_BIT0(0u);
            WRITE_P3_BIT1(1u);
        }
    }
}
