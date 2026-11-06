#include "send_opc_information.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/send_opc_information_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:faface */
void HandleSendOpcInformationCommand(void)
{
    u16 parameter_length;
    u8 opc_result;

    if(VALIDATE_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D00)) == 0u)
    {
        REJECT_COMMAND();
        return;
    }
    if(VALIDATE_MEDIA() == 0u)
        return;

    NORMALIZE_WRITE_SPEED();
    parameter_length = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_0B11);
    if(parameter_length > 32u || (parameter_length & 7u) != 0u)
    {
        SET_SENSE(5u, 0x2600u);
        return;
    }
    if(EXECUTE_DATA_OUT(FW_NEAR_UNKNOWN_E72B, parameter_length) == 4u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74B, (u8)(parameter_length >> 3));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 1u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E74E) != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) > 100u)
            {
                SET_SENSE(3u, 0x7302u);
                return;
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) > 95u)
            {
                SET_SENSE(1u, 0x7301u);
                return;
            }
        }

        RESET_RAW_READ_QUEUE();
        INITIALIZE_COROUTINE2();
        YIELD_COROUTINE0();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
        opc_result = RUN_OPC();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        if(opc_result == 0u)
            return;
    }
    SET_NO_SENSE();
}

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/send_opc_information_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadBigEndianU16At(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU16At(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fafbd0 */
u8 RunOpcForCurrentWriteSpeed(void)
{
    u8 result_code;
    u8 write_speed;

    PREPARE_HANDSHAKE_PINS();
    result_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    if(result_code != 0u)
    {
        SET_DIRECT_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + result_code), ReadBigEndianU16At(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + ((u16)result_code << 1)));
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15B2, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 6u);
    do
    {
        OPC_YIELD();
    } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u);

    result_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41);
    if(result_code != 0u)
    {
        SET_SENSE_FROM_STATUS(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
        return 0u;
    }

    do
    {
        SERVICE_PENDING_HOST();
        OPC_YIELD();
    } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15B2) == 1u);
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAAB) == 0x11u)
    {
        SERVICE_PENDING_HOST();
        OPC_YIELD();
    }

    SET_SENSE_FROM_PENDING();
    result_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAAB);
    if(result_code == 0x12u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 1u);
        UPDATE_OPC_SPEED_CLASS(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
        return 1u;
    }

    write_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEA, write_speed < 0x10u ? write_speed : 0x10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 1u));
    WriteBigEndianU16At(FW_NEAR_UNKNOWN_0C3A, ReadBigEndianU16At(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84416) + ((u16)result_code << 1)));
    return 0u;
}
