#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/mode_select.h"

static u8 memory[1u << 24];
static u8 controller_data[512];
static u32 controller_base;
static u8 data_out_status;
static u32 data_out_length;
static unsigned data_out_calls;
static unsigned copy_calls;
static u8 length_pages[16];
static u8 length_values[16];
static unsigned length_calls;
static u8 validated_pages[16];
static unsigned validate_calls;
static u8 rejected_page;
static fw_addr_t runtime_destinations[16];
static u8 runtime_counts[16];
static unsigned runtime_copy_calls;
static u8 applied_pages[16];
static unsigned apply_calls;
static unsigned sense_calls;
static u8 sense_key;
static u16 sense_code;
static unsigned no_sense_calls;
static unsigned reset_calls;
static unsigned initialize_calls;
static u32 initialized_entry;
static unsigned dispatch_calls;
static unsigned yield_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveModeSelectDataOut(u32 transfer_length)
{
    ++data_out_calls;
    data_out_length = transfer_length;
    return data_out_status;
}

u32 ObserveModeSelectControllerCopy(fw_addr_t destination, u32 controller_offset, u16 byte_count)
{
    u16 index;
    u32 source = controller_offset - controller_base;

    ++copy_calls;
    for(index = 0u; index < byte_count; ++index)
        FirmwareWrite8(destination + index, controller_data[source + index]);
    return controller_offset + byte_count;
}

u8 ObserveModeSelectLengthMatch(u8 page_code, u8 payload_length)
{
    length_pages[length_calls] = page_code;
    length_values[length_calls++] = payload_length;
    return 1u;
}

u8 ObserveModeSelectValidate(u8 page_code)
{
    validated_pages[validate_calls++] = page_code;
    return (u8)(page_code != rejected_page);
}

void ObserveModeSelectRuntimeCopy(fw_addr_t destination, fw_addr_t source, u8 byte_count)
{
    u8 index;

    runtime_destinations[runtime_copy_calls] = destination;
    runtime_counts[runtime_copy_calls++] = byte_count;
    for(index = 0u; index < byte_count; ++index)
        FirmwareWrite8(destination + index, FirmwareRead8(source + index));
}

void ObserveModeSelectApply(u8 page_code)
{
    applied_pages[apply_calls++] = page_code;
}

void ObserveModeSelectSense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}

void ObserveModeSelectNoSense(void)
{
    ++no_sense_calls;
}
void ObserveModeSelectResetRawRead(void)
{
    ++reset_calls;
}
void ObserveModeSelectInitializeCoroutine2(u32 entry_point)
{
    ++initialize_calls;
    initialized_entry = entry_point;
}
void ObserveModeSelectDispatchRequest5(void)
{
    ++dispatch_calls;
}
void ObserveModeSelectYield(void)
{
    ++yield_calls;
}

static void WriteBe16(u8 *destination, u16 value)
{
    destination[0] = (u8)(value >> 8);
    destination[1] = (u8)value;
}

static void WriteBe32Memory(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadBe32Memory(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(controller_data, 0, sizeof(controller_data));
    memset(length_pages, 0, sizeof(length_pages));
    memset(length_values, 0, sizeof(length_values));
    memset(validated_pages, 0, sizeof(validated_pages));
    memset(runtime_destinations, 0, sizeof(runtime_destinations));
    memset(runtime_counts, 0, sizeof(runtime_counts));
    memset(applied_pages, 0, sizeof(applied_pages));
    controller_base = 0x200ul;
    WriteBe32Memory(0x00155aul, controller_base);
    data_out_status = 5u;
    data_out_length = 0u;
    data_out_calls = 0u;
    copy_calls = 0u;
    length_calls = 0u;
    validate_calls = 0u;
    rejected_page = 0xffu;
    runtime_copy_calls = 0u;
    apply_calls = 0u;
    sense_calls = 0u;
    sense_key = 0u;
    sense_code = 0u;
    no_sense_calls = 0u;
    reset_calls = 0u;
    initialize_calls = 0u;
    initialized_entry = 0u;
    dispatch_calls = 0u;
    yield_calls = 0u;
}

static void AssertInvalidField(void)
{
    assert(sense_calls == 1u);
    assert(sense_key == 5u && sense_code == 0x2600u);
    assert(no_sense_calls == 0u);
}

static u16 BuildModeSelect6BlockDescriptor(u8 density, u32 block_size)
{
    controller_data[3] = 8u;
    controller_data[4] = density;
    controller_data[8] = (u8)(block_size >> 24);
    controller_data[9] = (u8)(block_size >> 16);
    controller_data[10] = (u8)(block_size >> 8);
    controller_data[11] = (u8)block_size;
    return 12u;
}

static void TestTransferAndHeaderExits(void)
{
    ResetState();
    data_out_status = 4u;
    ProcessModeSelectParameterList(6u, 0x1234u);
    assert(data_out_calls == 1u && data_out_length == 0x1234ul);
    assert(copy_calls == 0u && no_sense_calls == 0u);

    ResetState();
    ProcessModeSelectParameterList(6u, 3u);
    assert(copy_calls == 0u && no_sense_calls == 1u);
    assert(sense_calls == 0u);

    ResetState();
    ProcessModeSelectParameterList(10u, 7u);
    assert(copy_calls == 0u && no_sense_calls == 1u);

    ResetState();
    controller_data[3] = 7u;
    ProcessModeSelectParameterList(6u, 4u);
    AssertInvalidField();

    ResetState();
    WriteBe16(&controller_data[6], 9u);
    ProcessModeSelectParameterList(10u, 8u);
    AssertInvalidField();
}

static void TestBlockDescriptorSizesAndPublication(void)
{
    static const u16 accepted[] = { 0x0800u, 0x0804u, 0x0808u, 0x0914u, 0x091cu, 0x0920u, 0x0924u, 0x0930u, 0x0940u, 0x0990u, 0x0a56u };
    unsigned index;

    for(index = 0u; index < sizeof(accepted) / sizeof(accepted[0]); ++index)
    {
        ResetState();
        memory[0x000c83u] = 0xffu;
        ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0u, accepted[index]));
        assert(sense_calls == 0u && no_sense_calls == 1u);
        assert(memory[0x000b00u] == (u8)(accepted[index] >> 8));
        assert(memory[0x000b01u] == (u8)accepted[index]);
        assert(memory[0x000c83u] == 0xbfu);
    }

    ResetState();
    ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0u, 0x0810ul));
    AssertInvalidField();

    ResetState();
    memory[0x000c83u] = 0u;
    ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0x82u, 0x0930ul));
    assert((memory[0x000c83u] & 0x40u) != 0u);
    assert(memory[0x000c34u] == 0u);

    ResetState();
    ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0x84u, 0x0990ul));
    assert(memory[0x000c34u] == 2u);

    ResetState();
    ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0x82u, 0x0940ul));
    assert(memory[0x000c34u] == 1u);
}

static void TestValidationFailuresAndMode10DescriptorQuirk(void)
{
    ResetState();
    controller_data[4] = 0x04u;
    controller_data[5] = 0u;
    ProcessModeSelectParameterList(6u, 6u);
    AssertInvalidField();
    assert(validate_calls == 0u);

    ResetState();
    controller_data[4] = 0x01u;
    controller_data[5] = 7u;
    ProcessModeSelectParameterList(6u, 6u);
    AssertInvalidField();

    ResetState();
    controller_data[4] = 0x02u;
    controller_data[5] = 1u;
    ProcessModeSelectParameterList(6u, 6u);
    AssertInvalidField();

    ResetState();
    controller_data[4] = 0x02u;
    controller_data[5] = 0u;
    rejected_page = 0x02u;
    ProcessModeSelectParameterList(6u, 6u);
    AssertInvalidField();
    assert(validate_calls == 1u && validated_pages[0] == 0x02u);

    ResetState();
    WriteBe16(&controller_data[6], 8u);
    controller_data[8] = 0x82u;
    controller_data[9] = 0u;
    ProcessModeSelectParameterList(10u, 16u);
    AssertInvalidField();
    assert(validate_calls == 0u);
}

static u16 AppendPage(u16 cursor, u8 page_code, u8 payload_length)
{
    u8 index;

    controller_data[cursor++] = page_code;
    controller_data[cursor++] = payload_length;
    for(index = 0u; index < payload_length; ++index)
        controller_data[cursor++] = (u8)(page_code + index);
    return cursor;
}

static void TestAllPageDispatchAndFinalization(void)
{
    static const u8 pages[] = { 0x01u, 0x02u, 0x03u, 0x05u, 0x07u, 0x08u, 0x0du, 0x0eu, 0x1au, 0x1du };
    static const fw_addr_t destinations[] = { 0x00e623ul, 0x00e62ful, 0x2bc6eful, 0x00e63ful, 0x00e673ul, 0x00e67bul, 0x00e687ul, 0x00e68ful, 0x00e69ful };
    static const u8 counts[] = { 0x0cu, 0x10u, 0x08u, 0x34u, 0x08u, 0x0cu, 0x08u, 0x10u, 0x0cu };
    static const u8 applied[] = { 0x01u, 0x02u, 0x03u, 0x05u, 0x07u, 0x0du, 0x0eu, 0x1au };
    u16 cursor = 4u;
    unsigned index;

    ResetState();
    cursor = AppendPage(cursor, 0x01u, 6u);
    for(index = 1u; index < sizeof(pages) / sizeof(pages[0]); ++index)
        cursor = AppendPage(cursor, pages[index], 0u);
    memory[0x000c86u] = 0xffu;
    WriteBe32Memory(0x0012c1ul, 1000ul);
    WriteBe32Memory(0x00157aul, 2000ul);
    ProcessModeSelectParameterList(6u, cursor);

    assert(sense_calls == 0u && no_sense_calls == 1u);
    assert(validate_calls == 10u && length_calls == 9u);
    for(index = 0u; index < 10u; ++index)
        assert(validated_pages[index] == pages[index]);
    for(index = 0u; index < 9u; ++index)
        assert(length_pages[index] == pages[index + 1u]);
    assert(runtime_copy_calls == 9u && apply_calls == 8u);
    for(index = 0u; index < 9u; ++index)
    {
        assert(runtime_destinations[index] == destinations[index]);
        assert(runtime_counts[index] == counts[index]);
    }
    for(index = 0u; index < 8u; ++index)
        assert(applied_pages[index] == applied[index]);
    assert((memory[0x000c86u] & 1u) == 0u);
    assert(ReadBe32Memory(0x0012c5ul) == 850ul);
    assert(ReadBe32Memory(0x00157eul) == 1850ul);
}

static void TestBlockSizeChangeRestartGate(void)
{
    ResetState();
    memory[0x000c85u] = 2u;
    ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0u, 0x0930ul));
    assert(reset_calls == 1u && initialize_calls == 1u);
    assert(initialized_entry == 0x00f84a00ul);
    assert(dispatch_calls == 1u && yield_calls == 1u);

    ResetState();
    memory[0x000b00u] = 0x09u;
    memory[0x000b01u] = 0x30u;
    memory[0x000c85u] = 2u;
    ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0u, 0x0930ul));
    assert(reset_calls == 0u);

    ResetState();
    memory[0x000c85u] = 2u;
    memory[0x000faeu] = 1u;
    ProcessModeSelectParameterList(6u, BuildModeSelect6BlockDescriptor(0u, 0x0930ul));
    assert(reset_calls == 0u);
}

int main(void)
{
    TestTransferAndHeaderExits();
    TestBlockDescriptorSizesAndPublication();
    TestValidationFailuresAndMode10DescriptorQuirk();
    TestAllPageDispatchAndFinalization();
    TestBlockSizeChangeRestartGate();
    return 0;
}
