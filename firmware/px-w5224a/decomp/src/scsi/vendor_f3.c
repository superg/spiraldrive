#include "vendor_f3.h"
#include "../platform/controller_mapping.h"
#include "../platform/controller_parameters.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "mmc_write.h"
#include "vendor_f3_controller_mode.h"
#include "vendor_f3_measurement_response.h"
#include "vendor_f3_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f3_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern void YieldCoroutine0(void);

#define VENDOR_F3_CDB FW_NEAR_HOST_COMMAND_OPCODE

static u16 ReadVendorF3U16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteVendorF3U16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void StartVendorF3ReadAcquisitionBetweenMsfValues(u8 start_minute, u8 end_minute)
{
    s32 end_position = ConvertBinaryMsfValuesToAdjustedFramePosition(end_minute, 0u, 0u);
    s32 start_position = ConvertBinaryMsfValuesToAdjustedFramePosition(start_minute, 0u, 0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x04u));
    if(end_position == start_position)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8E, (u8)((u32)end_position >> 24));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8F, (u8)((u32)end_position >> 16));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C90, (u8)((u32)end_position >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C91, (u8)end_position);
        (void)DispatchAndWaitForReadAcquisitionResult();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8E, (u8)((u32)end_position >> 24));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8F, (u8)((u32)end_position >> 16));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C90, (u8)((u32)end_position >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C91, (u8)end_position);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C92, (u8)((u32)start_position >> 24));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C93, (u8)((u32)start_position >> 16));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C94, (u8)((u32)start_position >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C95, (u8)start_position);
        ExecuteReadAcquisitionAndWaitForCompletion(1u);
    }
}

static void ExecuteVendorF3CalibrationStart(void)
{
    u8 value = FirmwareRead8(VENDOR_F3_CDB + 2u);

    if(IsSignedByteLessThanNegative45(value) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1582, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1583, value);
    ApplyControllerParametersE8E9ForScsiF3(FirmwareReadNear8(FW_NEAR_UNKNOWN_1583));
    EnableControllerPort8098WritePath();
    SetNoSenseData();
}

static void ExecuteVendorF3CalibrationCommit(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1582) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    PersistVendorF3CalibrationHeaderBytes(FirmwareReadNear8(FW_NEAR_UNKNOWN_1583));
    DisableControllerPort8098WritePath();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1582, 0u);
    SetNoSenseData();
}

static void ExecuteVendorF3OpcStateUpdate(void)
{
    u16 value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
    {
        SetSenseData(2u, 0x0401u);
        return;
    }

    value = ReadVendorF3U16(VENDOR_F3_CDB + 3u);
    if(value >= 0x5dc0u)
        value = 0x5dc0u;
    SetVendorF3StateWordsAndFlag(value);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        StoreVendorF3MeasurementOperands(FirmwareRead8(VENDOR_F3_CDB + 5u), value);
        NormalizeWriteSpeedForCurrentMedia();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 1u);
    UpdateOpcCalibrationSpeedClass(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    SetNoSenseData();
}

static void ExecuteVendorF3AcquisitionRange(u8 start_minute, u8 end_minute)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
    else
        StartVendorF3ReadAcquisitionBetweenMsfValues(start_minute, end_minute);
}

static void ReportVendorF3DriveCycleResult(void)
{
    u8 result;

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB0) == 0u)
        YieldCoroutine0();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB1) == 1u)
    {
        SetNoSenseData();
        return;
    }

    result = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB2);
    if(result >= 1u && result <= 7u)
        SetSenseData(4u, (u16)(0x4080u + result));
    else if(result == 0u)
        SetSenseData(4u, 0x4080u);
    else
        SetSenseData(0u, 0u);
}

static void UpdateVendorF3ConfigurationBit(u16 mask, u8 shift)
{
    u16 value = ReadVendorF3U16(FW_NEAR_UNKNOWN_FAAC);

    value = (u16)(value & (u16)~mask);
    value |= (u16)((u16)(FirmwareRead8(VENDOR_F3_CDB + 2u) & 1u) << shift);
    WriteVendorF3U16(FW_NEAR_UNKNOWN_FAAC, value);
    SetNoSenseData();
}

static void ConfigureVendorF3PortSetup(void)
{
    u8 selector;
    u8 shadow;

    if(FirmwareRead8(VENDOR_F3_CDB + 2u) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    selector = FirmwareRead8(VENDOR_F3_CDB + 3u);
    if(selector > 1u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, selector == 0u ? 0u : 0x80u);
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
}

/* RAM:fcb050 */
void HandleScsiOpcodeF3(void)
{
    u8 selector = FirmwareRead8(VENDOR_F3_CDB + 1u);
    u16 value;

    switch(selector)
    {
    case 0x00u:
        ExecuteVendorF3CalibrationStart();
        break;
    case 0x01u:
        ExecuteVendorF3CalibrationCommit();
        break;
    case 0x02u:
        ExecuteVendorF3OpcStateUpdate();
        break;
    case 0x03u:
    case 0x1cu:
        SetNoSenseData();
        break;
    case 0x04u:
        ExecuteVendorF3AcquisitionRange(0x05u, 0x46u);
        break;
    case 0x05u:
        ExecuteVendorF3AcquisitionRange(0x46u, 0x4au);
        break;
    case 0x06u:
        CLEAR_VENDOR_F3_BIT_000B();
        SetNoSenseData();
        break;
    case 0x09u:
        ExecuteVendorF3AcquisitionRange(0x17u, 0x46u);
        break;
    case 0x0au:
        ExecuteVendorF3DriveCycleCommand();
        SetNoSenseData();
        break;
    case 0x0bu:
        ReportVendorF3DriveCycleResult();
        break;
    case 0x0du:
        ExecuteVendorF3Selector0DControllerMode(FirmwareRead8(VENDOR_F3_CDB + 2u));
        break;
    case 0x0eu:
        FUN_RAM_fbe56d(FirmwareRead8(VENDOR_F3_CDB + 2u));
        break;
    case 0x0fu:
        ResetControllerAndCompleteVendorF3Command();
        break;
    case 0x10u:
        UpdateVendorF3ConfigurationBit(0x4000u, 14u);
        break;
    case 0x11u:
        UpdateControllerPortSetupFlag6FromBoolean(FirmwareRead8(VENDOR_F3_CDB + 2u));
        break;
    case 0x12u:
        UpdateVendorF3ConfigurationBit(0x8000u, 15u);
        break;
    case 0x13u:
        if(PersistVendorF3ControllerWordIfPending() == 0u)
            InitializeCoroutine1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
        break;
    case 0x14u:
        value = ReadVendorF3U16(VENDOR_F3_CDB + 2u);
        if(value >= 0x5dc0u)
            value = 0x5dc0u;
        ApplyVendorF3ControllerWordInRange(value);
        break;
    case 0x15u:
        DispatchVendorF3ReadAcquisitionMode();
        break;
    case 0x16u:
        BuildVendorF3MeasurementResultResponse();
        break;
    case 0x17u:
        FUN_RAM_fbe98f();
        break;
    case 0x1du:
        if((FirmwareRead8(VENDOR_F3_CDB + 2u) & 1u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_131A, 1u);
            SetNoSenseData();
        }
        else
        {
            HandleUnsupportedScsiCommand();
        }
        break;
    case 0x1fu:
        ConfigureVendorF3PortSetup();
        break;
    default:
        HandleUnsupportedScsiCommand();
        break;
    }
}
