#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_f3.h"

static u8 memory[0x1000000ul];
static unsigned unsupported_calls;
static unsigned no_sense_calls;
static unsigned sense_calls;
static u8 sense_key;
static u16 sense_code;
static unsigned status_sense_calls;
static u8 status_sense_value;
static u8 signed_limit_result;
static unsigned apply_e8_calls;
static u8 apply_e8_value;
static unsigned enable_calls;
static unsigned disable_calls;
static unsigned persist_header_calls;
static u8 persist_header_value;
static unsigned state_word_calls;
static u16 state_word_value;
static unsigned operand_calls;
static u8 operand_byte;
static u16 operand_word;
static unsigned normalize_calls;
static unsigned speed_class_calls;
static u8 speed_class_value;
static u8 converted_minutes[8];
static unsigned conversion_count;
static unsigned acquisition_wait_calls;
static u8 acquisition_wait_argument;
static unsigned dispatch_wait_calls;
static unsigned clear_bit_calls;
static unsigned drive_cycle_calls;
static unsigned yield_calls;
static unsigned mode_calls;
static u8 mode_value;
static unsigned ee_calls;
static u8 ee_value;
static unsigned reset_calls;
static unsigned port_flag_calls;
static u8 port_flag_value;
static u8 persist_word_result;
static unsigned persist_word_calls;
static unsigned coroutine_calls;
static u32 coroutine_entry;
static unsigned apply_word_calls;
static u16 apply_word_value;
static unsigned acquisition_mode_calls;
static unsigned measurement_response_calls;
static unsigned cleanup_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void HandleUnsupportedScsiCommand(void)
{
    ++unsupported_calls;
}
void SetNoSenseData(void)
{
    ++no_sense_calls;
}
void SetSenseData(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}
void SetSenseDataFromStatusCode(u8 status)
{
    ++status_sense_calls;
    status_sense_value = status;
}
u8 IsSignedByteLessThanNegative45(u8 value)
{
    (void)value;
    return signed_limit_result;
}
void ApplyControllerParametersE8E9ForScsiF3(u8 value)
{
    ++apply_e8_calls;
    apply_e8_value = value;
}
void EnableControllerPort8098WritePath(void)
{
    ++enable_calls;
}
void DisableControllerPort8098WritePath(void)
{
    ++disable_calls;
}
void PersistVendorF3CalibrationHeaderBytes(u8 value)
{
    ++persist_header_calls;
    persist_header_value = value;
}
void SetVendorF3StateWordsAndFlag(u16 value)
{
    ++state_word_calls;
    state_word_value = value;
}
void StoreVendorF3MeasurementOperands(u8 value, u16 word_value)
{
    ++operand_calls;
    operand_byte = value;
    operand_word = word_value;
}
void NormalizeWriteSpeedForCurrentMedia(void)
{
    ++normalize_calls;
}
void UpdateOpcCalibrationSpeedClass(u8 value)
{
    ++speed_class_calls;
    speed_class_value = value;
}
s32 ConvertBinaryMsfValuesToAdjustedFramePosition(u8 minute, u8 second, u8 frame)
{
    assert(second == 0u && frame == 0u);
    converted_minutes[conversion_count++] = minute;
    return (s32)minute * 100l;
}
u8 DispatchAndWaitForReadAcquisitionResult(void)
{
    ++dispatch_wait_calls;
    return 1u;
}
void ExecuteReadAcquisitionAndWaitForCompletion(u8 argument)
{
    ++acquisition_wait_calls;
    acquisition_wait_argument = argument;
}
void ObserveVendorF3ClearBit000B(void)
{
    ++clear_bit_calls;
}
void ExecuteVendorF3DriveCycleCommand(void)
{
    ++drive_cycle_calls;
}
void YieldCoroutine0(void)
{
    ++yield_calls;
    memory[0x00fab0ul] = 1u;
}
void ExecuteVendorF3Selector0DControllerMode(u8 value)
{
    ++mode_calls;
    mode_value = value;
}
void FUN_RAM_fbe56d(u8 value)
{
    ++ee_calls;
    ee_value = value;
}
void ResetControllerAndCompleteVendorF3Command(void)
{
    ++reset_calls;
}
void UpdateControllerPortSetupFlag6FromBoolean(u8 value)
{
    ++port_flag_calls;
    port_flag_value = value;
}
u8 PersistVendorF3ControllerWordIfPending(void)
{
    ++persist_word_calls;
    return persist_word_result;
}
void InitializeCoroutine1(u32 entry_point)
{
    ++coroutine_calls;
    coroutine_entry = entry_point;
}
void ApplyVendorF3ControllerWordInRange(u16 value)
{
    ++apply_word_calls;
    apply_word_value = value;
}
void DispatchVendorF3ReadAcquisitionMode(void)
{
    ++acquisition_mode_calls;
}
void BuildVendorF3MeasurementResultResponse(void)
{
    ++measurement_response_calls;
}
void FUN_RAM_fbe98f(void)
{
    ++cleanup_calls;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    unsupported_calls = no_sense_calls = sense_calls = 0u;
    status_sense_calls = apply_e8_calls = enable_calls = disable_calls = 0u;
    persist_header_calls = state_word_calls = operand_calls = normalize_calls = 0u;
    speed_class_calls = conversion_count = acquisition_wait_calls = 0u;
    dispatch_wait_calls = clear_bit_calls = drive_cycle_calls = yield_calls = 0u;
    mode_calls = ee_calls = reset_calls = port_flag_calls = 0u;
    persist_word_calls = coroutine_calls = apply_word_calls = 0u;
    acquisition_mode_calls = measurement_response_calls = cleanup_calls = 0u;
    sense_key = status_sense_value = apply_e8_value = persist_header_value = 0u;
    operand_byte = speed_class_value = acquisition_wait_argument = 0u;
    mode_value = ee_value = port_flag_value = 0u;
    sense_code = state_word_value = operand_word = apply_word_value = 0u;
    coroutine_entry = 0ul;
    signed_limit_result = 1u;
    persist_word_result = 1u;
}

static void Select(u8 selector)
{
    memory[0x000b0bul] = selector;
    HandleScsiOpcodeF3();
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void TestCalibrationStartAndCommit(void)
{
    Reset();
    memory[0x000b0cul] = 0x90u;
    Select(0u);
    assert(memory[0x001582ul] == 1u && memory[0x001583ul] == 0x90u);
    assert(apply_e8_calls == 1u && apply_e8_value == 0x90u);
    assert(enable_calls == 1u && no_sense_calls == 1u);

    Reset();
    signed_limit_result = 0u;
    Select(0u);
    assert(unsupported_calls == 1u && apply_e8_calls == 0u);

    Reset();
    Select(1u);
    assert(unsupported_calls == 1u);

    Reset();
    memory[0x001582ul] = 1u;
    memory[0x001583ul] = 0x91u;
    Select(1u);
    assert(persist_header_calls == 1u && persist_header_value == 0x91u);
    assert(disable_calls == 1u && memory[0x001582ul] == 0u);
    assert(no_sense_calls == 1u);
}

static void TestOpcStateUpdate(void)
{
    Reset();
    memory[0x000faeul] = 7u;
    Select(2u);
    assert(status_sense_calls == 1u && status_sense_value == 7u);

    Reset();
    Select(2u);
    assert(sense_calls == 1u && sense_key == 2u && sense_code == 0x0401u);

    Reset();
    memory[0x000c85ul] = 2u;
    memory[0x001008ul] = 2u;
    memory[0x000b0dul] = 0x60u;
    memory[0x000b0eul] = 0u;
    memory[0x000b0ful] = 9u;
    memory[0x000e47ul] = 24u;
    Select(2u);
    assert(state_word_calls == 1u && state_word_value == 0x5dc0u);
    assert(operand_calls == 1u && operand_byte == 9u && operand_word == 0x5dc0u);
    assert(normalize_calls == 1u && memory[0x000c38ul] == 24u);
    assert(memory[0x00e74eul] == 1u);
    assert(speed_class_calls == 1u && speed_class_value == 24u);
    assert(no_sense_calls == 1u);
}

static void TestAcquisitionRanges(void)
{
    Reset();
    Select(4u);
    assert(conversion_count == 2u && converted_minutes[0] == 0x46u && converted_minutes[1] == 0x05u);
    assert(memory[0x000c2aul] == 5u && memory[0x000c8cul] == 2u);
    assert(ReadU32(0x000c8eul) == 0x46u * 100ul);
    assert(ReadU32(0x000c92ul) == 0x05u * 100ul);
    assert(acquisition_wait_calls == 1u && acquisition_wait_argument == 1u);

    Reset();
    Select(5u);
    assert(converted_minutes[0] == 0x4au && converted_minutes[1] == 0x46u);

    Reset();
    Select(9u);
    assert(converted_minutes[0] == 0x46u && converted_minutes[1] == 0x17u);

    Reset();
    memory[0x000faeul] = 3u;
    Select(4u);
    assert(status_sense_calls == 1u && conversion_count == 0u);
}

static void TestSimpleDispatchSelectors(void)
{
    Reset();
    Select(3u);
    assert(no_sense_calls == 1u);
    Reset();
    Select(6u);
    assert(clear_bit_calls == 1u && no_sense_calls == 1u);
    Reset();
    Select(0x0au);
    assert(drive_cycle_calls == 1u && no_sense_calls == 1u);
    Reset();
    memory[0x000b0cul] = 7u;
    Select(0x0du);
    assert(mode_calls == 1u && mode_value == 7u);
    Reset();
    memory[0x000b0cul] = 8u;
    Select(0x0eu);
    assert(ee_calls == 1u && ee_value == 8u);
    Reset();
    Select(0x0fu);
    assert(reset_calls == 1u);
    Reset();
    memory[0x000b0cul] = 1u;
    Select(0x11u);
    assert(port_flag_calls == 1u && port_flag_value == 1u);
    Reset();
    Select(0x15u);
    assert(acquisition_mode_calls == 1u);
    Reset();
    Select(0x16u);
    assert(measurement_response_calls == 1u);
    Reset();
    Select(0x17u);
    assert(cleanup_calls == 1u);
    Reset();
    Select(0x1cu);
    assert(no_sense_calls == 1u);
    Reset();
    Select(0x08u);
    assert(unsupported_calls == 1u);
}

static void TestDriveCycleResult(void)
{
    Reset();
    memory[0x00fab2ul] = 7u;
    Select(0x0bu);
    assert(yield_calls == 1u);
    assert(sense_calls == 1u && sense_key == 4u && sense_code == 0x4087u);

    Reset();
    memory[0x00fab0ul] = 1u;
    memory[0x00fab1ul] = 1u;
    Select(0x0bu);
    assert(no_sense_calls == 1u && sense_calls == 0u);

    Reset();
    memory[0x00fab0ul] = 1u;
    memory[0x00fab2ul] = 0x20u;
    Select(0x0bu);
    assert(sense_calls == 1u && sense_key == 0u && sense_code == 0u);
}

static void TestConfigurationAndPersistenceSelectors(void)
{
    Reset();
    memory[0x00faacul] = 0xffu;
    memory[0x00faadul] = 0xffu;
    memory[0x000b0cul] = 0u;
    Select(0x10u);
    assert(memory[0x00faacul] == 0xbfu && memory[0x00faadul] == 0xffu);
    memory[0x000b0cul] = 1u;
    Select(0x10u);
    assert(memory[0x00faacul] == 0xffu && memory[0x00faadul] == 0xffu);

    memory[0x000b0cul] = 0u;
    Select(0x12u);
    assert(memory[0x00faacul] == 0x7fu);
    memory[0x000b0cul] = 1u;
    Select(0x12u);
    assert(memory[0x00faacul] == 0xffu);

    Reset();
    persist_word_result = 0u;
    Select(0x13u);
    assert(persist_word_calls == 1u && coroutine_calls == 1u);
    assert(coroutine_entry == 0x00f853cful);

    Reset();
    memory[0x000b0cul] = 0x60u;
    Select(0x14u);
    assert(apply_word_calls == 1u && apply_word_value == 0x5dc0u);
}

static void TestFinalFlagsAndPortSetup(void)
{
    Reset();
    Select(0x1du);
    assert(unsupported_calls == 1u);
    Reset();
    memory[0x000b0cul] = 3u;
    Select(0x1du);
    assert(memory[0x00131aul] == 1u && no_sense_calls == 1u);

    Reset();
    memory[0x000fcaul] = 0x20u;
    memory[0x000b0dul] = 1u;
    Select(0x1fu);
    assert(memory[0x018003ul] == 0x80u);
    assert(memory[0x000fcaul] == 0x20u && memory[0x018002ul] == 0x20u);
    assert(no_sense_calls == 0u);

    Reset();
    memory[0x000b0cul] = 1u;
    Select(0x1fu);
    assert(unsupported_calls == 1u);
}

int main(void)
{
    TestCalibrationStartAndCommit();
    TestOpcStateUpdate();
    TestAcquisitionRanges();
    TestSimpleDispatchSelectors();
    TestDriveCycleResult();
    TestConfigurationAndPersistenceSelectors();
    TestFinalFlagsAndPortSetup();
    return 0;
}
