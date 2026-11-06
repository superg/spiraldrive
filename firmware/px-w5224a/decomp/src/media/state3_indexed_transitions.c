#include "state3_indexed_transitions.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/state3_indexed_transitions_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadState3IndexedU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteState3IndexedU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadState3IndexedU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteState3IndexedU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:f86b2a */
u8 PLEXTOR_NEAR_FUNCTION ServiceMediaState3IndexedDriveTransitions(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x0cu)
        MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_DISPATCH, 0ul);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x0eu)
    {
        s32 countdown = (s32)ReadState3IndexedU32(FW_NEAR_UNKNOWN_0FB0);
        s32 threshold = (s32)(ReadState3IndexedU32(FW_NEAR_UNKNOWN_0FB4) - 0x1d4cul);

        if(countdown < threshold)
            MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x13u)
        MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_ADVANCE, 0ul);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x04u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E1) != 0u)
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
                MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_SERVICE_COUNTDOWN, 0ul);
            else
                MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_SERVICE_CALIBRATION, 0ul);
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x04u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
            MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_INITIALIZE_RANGE, 0ul);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB0) != 0u)
            MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 6u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E1) == 0xffu)
            MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_INITIALIZE_RANGE, 0ul);
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x04u) == 0u)
            MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_SERVICE_COUNTDOWN, 0ul);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 4u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x20u) == 0u)
        {
            FirmwareWrite8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F), 0x11u);
            if(((ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) >> 9) & 3u) == 2u && (s32)ReadState3IndexedU32(FW_NEAR_CURRENT_FRAME_POSITION) >= 0x00056cacl)
            {
                WriteState3IndexedU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) | 0x0600u));
                MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_INITIALIZE_COROUTINE0, FW_ENTRY_RUN_INDEXED_STATE04_MEDIA_READ_COROUTINE0);
            }
        }
        else if((ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) & 0x0800u) == 0u)
        {
            WriteState3IndexedU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) | 0x0800u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x28u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 0x28u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, 9u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11DC, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11DC) | 1u));
            if((s16)ReadState3IndexedU16(FW_NEAR_UNKNOWN_11DA) > 0)
                WriteState3IndexedU16(FW_NEAR_UNKNOWN_11DA, 0x03c0u);
        }
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE9) > 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE9) - 1u));
            MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, FW_ENTRY_RUN_STARTUP_SEQUENCE_COROUTINE1);
        }
        else
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE9) != 0xffu && ((ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) >> 9) & 3u) == 0u)
            {
                WriteState3IndexedU16(FW_NEAR_UNKNOWN_FAAC, (u16)((ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) & 0xf9ffu) | 0x0200u));
                MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_INITIALIZE_COROUTINE0, FW_ENTRY_RUN_DRIVE_OPERATION_RESULT_COROUTINE0);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE9, 0xffu);
            }
            if(((ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) >> 9) & 3u) == 2u)
            {
                WriteState3IndexedU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) | 0x0600u));
                MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_INITIALIZE_COROUTINE0, FW_ENTRY_RUN_INDEXED_STATE04_MEDIA_READ_COROUTINE0);
            }
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 9u)
    {
        if((ReadState3IndexedU16(FW_NEAR_UNKNOWN_FAAC) & 0x0800u) != 0u && (s16)ReadState3IndexedU16(FW_NEAR_UNKNOWN_11DA) <= 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11DC, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11DC) & 0xfeu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, 4u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x34u);
            MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, FW_ENTRY_RUN_STARTUP_SEQUENCE_COROUTINE1);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB8, 10u);
        WriteState3IndexedU32(FW_NEAR_UNKNOWN_0FB4, 0ul);
        MEDIA_STATE3_INDEXED_CALL(MEDIA_STATE3_INDEXED_DISPATCH, 0ul);
    }

    return 0u;
}
