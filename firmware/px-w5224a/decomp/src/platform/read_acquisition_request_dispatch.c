#include "read_acquisition_request_dispatch.h"
#include "coroutine_initialization.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

extern void ResumeCoroutine1(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/read_acquisition_request_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRequestDispatchU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRequestDispatchU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fde12d */
void DispatchReadAcquisitionRequestFlags(void)
{
    u8 state;
    u8 track_number;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_11DD) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xdfu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
            return;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 1u || FirmwareReadBit0008() != 0u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F72) == 0u)
            return;

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0x04u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xfbu));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
                return;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 5u)
                return;

            SET_READ_ACQUISITION_P3_RX_HIGH_TX_LOW();
            track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
            LoadReadAcquisitionTargetForTrackNumber();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 0u)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            InitializeCoroutine1(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1);
            ResumeCoroutine1();
            return;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0x01u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xfeu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBE, 2u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 1u)
                return;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
                return;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 5u)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
            ResumeCoroutine1();
            return;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0x02u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xfdu));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 3u)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
            ResumeCoroutine1();
            return;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0x08u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xf7u));
            AdvanceReadAcquisitionTrackCursor();
            LoadReadAcquisitionTargetForTrackNumber();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 0u)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
            ResumeCoroutine1();
            return;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0x20u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xdfu));
            state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF);
            if(state == 0u)
            {
                RetreatReadAcquisitionTrackCursor();
                LoadReadAcquisitionTargetForTrackNumber();
                return;
            }
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1218) > 0)
                RetreatReadAcquisitionTrackCursor();
            LoadReadAcquisitionTargetForTrackNumber();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
            ResumeCoroutine1();
            return;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0x10u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) & 0xefu));
            if((s32)ReadRequestDispatchU32(FW_NEAR_UNKNOWN_12C1) < 0x00017124l)
                return;
            WriteRequestDispatchU32(FW_NEAR_UNKNOWN_0E3B, 90000ul);
            track_number = FindTrackNumberForFramePosition(90000l);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
            ResumeCoroutine1();
            return;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E1) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
            return;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E1) & 0x04u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 1u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x20u) == 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBD, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x20u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xbfu));
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11E1) & 0x10u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 1u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x20u) == 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBD, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x20u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x40u));
            return;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xdfu));
}
