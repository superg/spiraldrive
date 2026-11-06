#include "opcode_f0_configuration_update.h"
#include "../platform/configuration_range_update.h"
#include "../platform/coroutine_initialization.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

extern void YieldCoroutine0(void);
static u16 ReadBigEndianWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u8 StagedIdentityMatches(void)
{
    u8 index;

    for(index = 0u; index < 8u; ++index)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A + index) != FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF95F0) + index))
            return 0u;
    return 1u;
}

static u8 ProtectedConfigurationFieldsAreValid(u16 offset, u16 end)
{
    u8 value;

    if((s16)end > 0x40 && offset < 0x41u)
    {
        value = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B32 + (u16)(0x41u - offset));
        if(value != 0u && value != 0xffu)
            return 0u;
    }
    if((s16)end > 0x4b && offset < 0x4cu)
    {
        value = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B32 + (u16)(0x4cu - offset));
        if((value & 0x3fu) != 0x3fu)
            return 0u;
    }
    return 1u;
}

static u8 ConfigurationOffsetRequiresVerification(u16 offset)
{
    return (u8)(offset < 0x5eu || (offset > 0x5fu && offset < 0x7eu));
}

/* RAM:fd5dd7 */
void HandleScsiOpcodeF0(void)
{
    u16 offset;
    u16 parameter_length;
    u16 end;
    u16 payload_length;
    u16 verification_count;
    u16 index;
    u16 absolute_offset;

    while((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 4u) != 0u)
        YieldCoroutine0();

    offset = ReadBigEndianWord(FW_NEAR_UNKNOWN_0B0E);
    parameter_length = ReadBigEndianWord(FW_NEAR_UNKNOWN_0B12);
    if((u32)parameter_length + offset - 8ul > 0xa0ul)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if(ExecuteDataOutAndCopyFromControllerBuffer(FW_NEAR_UNKNOWN_0B2A, parameter_length) == 4u)
        return;
    if(!StagedIdentityMatches())
    {
        SetSenseData(5u, 0x2600u);
        return;
    }

    end = (u16)(parameter_length + offset - 9u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 2u) != 0u && !ProtectedConfigurationFieldsAreValid(offset, end))
    {
        SetSenseData(5u, 0x2600u);
        return;
    }

    payload_length = (u16)(parameter_length - 8u);
    if(UpdateAndProgramDriveConfigurationRange(FW_NEAR_UNKNOWN_0B32, offset, payload_length) == 0u)
    {
        InitializeCoroutine1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
        SetSenseData(4u, 0x4000u);
        return;
    }

    verification_count = (u16)(end - offset);
    ReadDriveConfigurationBytes(offset, FW_NEAR_UNKNOWN_E4FA, verification_count);
    for(index = 0u; index < verification_count; ++index)
    {
        absolute_offset = (u16)(offset + index);
        if(ConfigurationOffsetRequiresVerification(absolute_offset) && FirmwareReadNear8(FW_NEAR_UNKNOWN_E4FA + index) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0B32 + index))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
            InitializeCoroutine1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
            return;
        }
    }
    SetNoSenseData();
}
