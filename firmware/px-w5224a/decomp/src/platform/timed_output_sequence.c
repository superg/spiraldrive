#include "timed_output_sequence.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/timed_output_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fa826b */
void ServiceIndexedState0FTimedOutputSequence(void)
{
    u8 repetition_limit;
    u8 repetition_count;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) & 1u) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 30u);
        SET_TIMED_OUTPUT_LINES_DIRECT_HIGH();
        while((s8)READ_TIMED_OUTPUT_COUNTDOWN() > 0)
        {
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B9, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) & 0xfdu));
    }

    if((s8)READ_TIMED_OUTPUT_COUNTDOWN() >= 0)
        return;

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) >> 1) & 1u) == 0u)
    {
        APPLY_TIMED_OUTPUT_STATE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) | 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 30u);
    }
    else
    {
        SET_TIMED_OUTPUT_LINES_HIGH();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B9) + 1u));
        repetition_limit = FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B8);
        repetition_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B9);
        if(repetition_limit > repetition_count)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 30u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B9, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 0x7fu);
        }
    }
}
