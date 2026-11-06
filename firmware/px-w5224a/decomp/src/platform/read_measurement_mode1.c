#include "read_measurement_mode1.h"
#include "firmware_memory.h"
#include "media/toc_session_append.h"
#include "media/toc_session_removal.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/read_measurement_mode1_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void AppendAndFinalizeMode1TocSession(void)
{
    Mode1AppendSession();
    Mode1FinalizeC3();
    Mode1FinalizeC4();
}

/* RAM:fd987d */
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode1(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 1u)
    {
        Mode1PrepareDescriptor();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B8) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_88B8) == 1u)
                Mode1ResetToc();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B8) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_88B8) == 6u)
                Mode1RemoveSession();
        }
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        Mode1PrepareDescriptor();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        {
            Mode1ResetToc();
            Mode1AppendSession();
        }
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 1u)
        {
            Mode1PrepareDescriptor();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
                return;
            Mode1RemoveSession();
            AppendAndFinalizeMode1TocSession();
            return;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u)
        {
            Mode1PrepareDescriptor();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
                return;
            Mode1ResetToc();
            Mode1AppendSession();
            return;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9C) == 1u)
    {
        Mode1PrepareDescriptor();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
            AppendAndFinalizeMode1TocSession();
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 3u)
    {
        Mode1PrepareLowDescriptor();
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 2u)
    {
        Mode1PrepareDescriptor();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && Mode1ReadBit0032() != 1u)
            AppendAndFinalizeMode1TocSession();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 6u);
    }
}
