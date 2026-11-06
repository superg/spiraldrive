#include "vendor_e9.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define VENDOR_E9_CDB FW_NEAR_HOST_COMMAND_OPCODE
#define VENDOR_E9_RESPONSE FW_NEAR_UNKNOWN_0B2A

static void InitializeVendorE9Response(void)
{
    u8 offset;

    FirmwareWrite8(VENDOR_E9_RESPONSE + 1u, 6u);
    for(offset = 2u; offset < 8u; ++offset)
        FirmwareWrite8(VENDOR_E9_RESPONSE + offset, 0u);
}

static void BuildVendorE9Selector1Response(u8 allow_updates)
{
    u8 state;

    InitializeVendorE9Response();
    if(allow_updates != 0u)
    {
        state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
        state &= 0xfeu;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
        if((FirmwareRead8(VENDOR_E9_CDB + 3u) & 1u) != 0u)
        {
            state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
            state |= 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
        }

        state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
        state &= 0xfdu;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
        if((FirmwareRead8(VENDOR_E9_CDB + 3u) & 2u) != 0u)
        {
            state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
            state |= 2u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 1u) != 0u)
        FirmwareWrite8(VENDOR_E9_RESPONSE + 2u, (u8)(FirmwareRead8(VENDOR_E9_RESPONSE + 2u) | 1u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 2u) != 0u)
        FirmwareWrite8(VENDOR_E9_RESPONSE + 2u, (u8)(FirmwareRead8(VENDOR_E9_RESPONSE + 2u) | 2u));
}

static void BuildVendorE9Selector2Response(u8 allow_updates)
{
    u8 flags;
    u8 adjustment;

    InitializeVendorE9Response();
    if(allow_updates != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u))
    {
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_158E);
        flags &= 0xfdu;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, flags);
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_158E);
        flags &= 0xfeu;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, flags);
        adjustment = 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158D, 0u);

        if(FirmwareRead8(VENDOR_E9_CDB + 3u) == 1u)
        {
            flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_158E);
            flags |= 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, flags);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158D, FirmwareRead8(VENDOR_E9_CDB + 3u));
            adjustment = (u8)(FirmwareRead8(VENDOR_E9_CDB + 4u) & 0x7fu);
            if(adjustment > 2u)
                adjustment = 2u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158B, adjustment);
        if((FirmwareRead8(VENDOR_E9_CDB + 4u) & 0x80u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158B, (u8)(0u - adjustment));
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 1u) != 0u)
        FirmwareWrite8(VENDOR_E9_RESPONSE + 2u, (u8)(FirmwareRead8(VENDOR_E9_RESPONSE + 2u) | 1u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0x10u) != 0u)
        FirmwareWrite8(VENDOR_E9_RESPONSE + 2u, (u8)(FirmwareRead8(VENDOR_E9_RESPONSE + 2u) | 2u));

    adjustment = FirmwareReadNear8(FW_NEAR_UNKNOWN_158B);
    if((s8)adjustment < 0)
        adjustment = (u8)((u8)(0u - adjustment) | 0x80u);
    FirmwareWrite8(VENDOR_E9_RESPONSE + 3u, adjustment);
    FirmwareWrite8(VENDOR_E9_RESPONSE + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_158D));
    FirmwareWrite8(VENDOR_E9_RESPONSE + 5u, 0u);
}

static u8 ValidateVendorE9SelectorBbReservedBytes(void)
{
    u8 offset;

    for(offset = 4u; offset < 9u; ++offset)
        if(FirmwareRead8(VENDOR_E9_CDB + offset) != 0u)
            return 0u;
    return 1u;
}

static void BuildVendorE9SelectorBbResponse(u8 allow_updates)
{
    u8 state;

    InitializeVendorE9Response();
    if(allow_updates != 0u)
    {
        if((FirmwareRead8(VENDOR_E9_CDB + 3u) & 1u) != 0u)
        {
            state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
            state |= 4u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C43, 0x34u);
        }
        else
        {
            state = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED);
            state &= 0xfbu;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, state);
            state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
            state &= 0xfbu;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C43, 0x28u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C43) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37))
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C43));
        }

        state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
        state &= 0xf7u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
        if((FirmwareRead8(VENDOR_E9_CDB + 3u) & 2u) != 0u)
        {
            state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
            state |= 8u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, state);
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 4u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 4u) != 0u)
        FirmwareWrite8(VENDOR_E9_RESPONSE + 2u, (u8)(FirmwareRead8(VENDOR_E9_RESPONSE + 2u) | 1u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 8u) != 0u)
        FirmwareWrite8(VENDOR_E9_RESPONSE + 2u, (u8)(FirmwareRead8(VENDOR_E9_RESPONSE + 2u) | 2u));
}

/* RAM:fca870 */
void HandleScsiOpcodeE9(void)
{
    u8 allow_updates = (u8)((FirmwareRead8(VENDOR_E9_CDB + 1u) & 0x10u) != 0u);
    u16 allocation_length = ReadBigEndianU16(VENDOR_E9_CDB + 9u);
    u8 selector = FirmwareRead8(VENDOR_E9_CDB + 2u);
    u16 response_length;

    FirmwareWrite8(VENDOR_E9_RESPONSE, selector);
    if(selector == 1u)
    {
        BuildVendorE9Selector1Response(allow_updates);
    }
    else if(selector == 2u)
    {
        BuildVendorE9Selector2Response(allow_updates);
    }
    else if(selector == 0xbbu)
    {
        if(ValidateVendorE9SelectorBbReservedBytes() == 0u)
        {
            HandleUnsupportedScsiCommand();
            return;
        }
        BuildVendorE9SelectorBbResponse(allow_updates);
    }
    else
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    response_length = (u16)(FirmwareRead8(VENDOR_E9_RESPONSE + 1u) + 2u);
    if(allocation_length < response_length)
        response_length = allocation_length;
    if(CopyResponseToControllerBufferAndExecuteDataIn(VENDOR_E9_RESPONSE, response_length) != 4u)
        SetNoSenseData();
}
