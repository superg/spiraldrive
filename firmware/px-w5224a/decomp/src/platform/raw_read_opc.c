#include "raw_read_opc.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/raw_read_opc_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static fw_addr_t ReadOpcStatusPointer(void)
{
    return ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_15B7) << 24) | ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_15B8) << 16) | ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_15B9) << 8)
         | FirmwareReadNear8(FW_NEAR_UNKNOWN_15BA);
}

/* RAM:fec05d */
u8 PLEXTOR_NEAR_FUNCTION EnsureOpcCalibrationForRawReadOperation(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x08u));

    if(IS_OPC_REQUIRED(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38)) == 1u)
    {
        RESET_RAW_READ_QUEUE();
        INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        YIELD_COROUTINE0();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15B2, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 6u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E74F) == 1u)
    {
        do
        {
            YIELD_COROUTINE0();
        } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15B2) == 1u);

        while(FirmwareRead8(ReadOpcStatusPointer()) == 0x11u)
            YIELD_COROUTINE0();

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15B2) != 1u && FirmwareRead8(ReadOpcStatusPointer()) != 0x11u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74F, 0u);
            SET_SENSE_FROM_PENDING_RESULT();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B1A) != 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 1u);
            UPDATE_OPC_SPEED_CLASS(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    return 1u;
}
