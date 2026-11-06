#include <assert.h>
#include <string.h>
#include "media/atip_initialization.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u8 initialize_results[8];
static u8 acquire_results[8];
static u8 parse_results[8];
static u8 validate_results[8];
static unsigned initialize_count;
static unsigned initialize_index;
static unsigned acquire_count;
static unsigned acquire_index;
static unsigned parse_count;
static unsigned parse_index;
static unsigned validate_count;
static unsigned validate_index;
static unsigned reset_atip_calls;
static unsigned reset_drive_calls;
static unsigned reset_ports_calls;
static unsigned reset_hardware_calls;
static unsigned reset_operating_calls;
static unsigned process_indexed_calls;
static unsigned apply_calls;
static unsigned arm_calls;
static unsigned yield_calls;
static unsigned supplemental_calls;
static u8 indexed_result;
static u8 recoverable_initialize_failure;
static u8 make_second_initialize_nonrecordable;
static u8 expire_on_yield;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void ObserveAtipInitializationResetAtip(void)
{
    reset_atip_calls++;
}
void ObserveAtipInitializationResetDriveControl(void)
{
    reset_drive_calls++;
}
u8 ObserveAtipInitializationIsIndexed(u8 selector)
{
    assert(selector == 0xf7u);
    return indexed_result;
}
void ObserveAtipInitializationProcessIndexed(void)
{
    process_indexed_calls++;
}
u8 ObserveAtipInitializationInitializeController(void)
{
    u8 result;
    assert(initialize_index < initialize_count);
    result = initialize_results[initialize_index++];
    if(result == 0u && recoverable_initialize_failure != 0u)
        memory[0x001304u] |= 4u;
    if(initialize_index == 2u && make_second_initialize_nonrecordable != 0u)
        memory[0x001008u] = 0u;
    return result;
}
void ObserveAtipInitializationResetPorts(void)
{
    reset_ports_calls++;
}
void ObserveAtipInitializationResetHardware(void)
{
    reset_hardware_calls++;
}
void ObserveAtipInitializationResetOperatingState(void)
{
    reset_operating_calls++;
}
void ObserveAtipInitializationWriteController(u32 address, u8 value)
{
    assert(address == 0x018082ul && value == 0xb0u);
}
u8 ObserveAtipInitializationAcquirePosition(void)
{
    assert(acquire_index < acquire_count);
    return acquire_results[acquire_index++];
}
void ObserveAtipInitializationApplyOperatingCode(void)
{
    apply_calls++;
}
void ObserveAtipInitializationArmPacket(void)
{
    arm_calls++;
}
u8 ObserveAtipInitializationParsePacket(void)
{
    assert(parse_index < parse_count);
    return parse_results[parse_index++];
}
void ObserveAtipInitializationYield(void)
{
    yield_calls++;
    if(expire_on_yield != 0u)
        memory[0x001203u] = 0u;
}
u8 ObserveAtipInitializationValidatePacket(u8 mode)
{
    assert(mode == 1u && validate_index < validate_count);
    return validate_results[validate_index++];
}
void ObserveAtipInitializationAcquireSupplemental(void)
{
    supplemental_calls++;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(initialize_results, 0, sizeof(initialize_results));
    memset(acquire_results, 0, sizeof(acquire_results));
    memset(parse_results, 0, sizeof(parse_results));
    memset(validate_results, 0, sizeof(validate_results));
    initialize_count = initialize_index = 0u;
    acquire_count = acquire_index = 0u;
    parse_count = parse_index = 0u;
    validate_count = validate_index = 0u;
    reset_atip_calls = reset_drive_calls = 0u;
    reset_ports_calls = reset_hardware_calls = reset_operating_calls = 0u;
    process_indexed_calls = apply_calls = arm_calls = yield_calls = 0u;
    supplemental_calls = 0u;
    indexed_result = recoverable_initialize_failure = 0u;
    make_second_initialize_nonrecordable = expire_on_yield = 0u;
}

static void TestControllerFailureRoutes(void)
{
    ResetState();
    initialize_count = 1u;
    assert(InitializeControllerAndAcquireAtipState() == 2u);
    assert(reset_ports_calls == 1u && reset_hardware_calls == 1u && reset_operating_calls == 1u && memory[0x001300u] == 0u);

    ResetState();
    initialize_count = 4u;
    recoverable_initialize_failure = 1u;
    memory[0x000e41u] = 0x15u;
    assert(InitializeControllerAndAcquireAtipState() == 0x15u);
    assert(initialize_index == 4u && memory[0x00fab1u] == 0u && memory[0x00fab2u] == 5u);

    ResetState();
    initialize_count = 2u;
    recoverable_initialize_failure = 1u;
    memory[0xf7c01fu] = 1u;
    memory[0x000e41u] = 7u;
    assert(InitializeControllerAndAcquireAtipState() == 0x14u);
    assert(initialize_index == 2u && memory[0x00fab2u] == 3u);
}

static void TestNonrecordableAndIndexedSuccess(void)
{
    ResetState();
    initialize_results[0] = 1u;
    initialize_count = 1u;
    indexed_result = 1u;
    assert(InitializeControllerAndAcquireAtipState() == 0u);
    assert(process_indexed_calls == 1u);
    assert(memory[0x000e43u] == 4u && memory[0x000e4fu] == 4u && memory[0x000e44u] == 4u);
}

static void TestAtipSuccessAndSupplemental(void)
{
    ResetState();
    initialize_results[0] = 1u;
    initialize_count = 1u;
    acquire_results[0] = 1u;
    acquire_count = 1u;
    parse_results[0] = 0xfeu;
    parse_count = 1u;
    validate_results[0] = 1u;
    validate_count = 1u;
    memory[0x001008u] = 1u;
    memory[0x001306u] = 1u;
    assert(InitializeControllerAndAcquireAtipState() == 0u);
    assert(apply_calls == 1u && arm_calls == 1u && yield_calls == 1u);
    assert(supplemental_calls == 1u);
    assert(memory[0x01802bu] == 0x80u);
    assert((memory[0x001301u] & 2u) == 0u);
    assert(memory[0x000e3bu] == 0xffu && memory[0x000e3cu] == 0xffu && memory[0x000e3du] == 0xdcu && memory[0x000e3eu] == 0xd8u);
}

static void TestAcquisitionRetryAndTimeout(void)
{
    ResetState();
    initialize_results[0] = initialize_results[1] = 1u;
    initialize_count = 2u;
    acquire_count = 1u;
    memory[0x001008u] = 1u;
    make_second_initialize_nonrecordable = 1u;
    assert(InitializeControllerAndAcquireAtipState() == 0u);
    assert(acquire_index == 1u && (memory[0x001304u] & 1u) != 0u);

    ResetState();
    initialize_results[0] = 1u;
    initialize_count = 1u;
    acquire_results[0] = 1u;
    acquire_count = 1u;
    parse_results[0] = 0xfeu;
    parse_count = 1u;
    validate_count = 1u;
    expire_on_yield = 1u;
    memory[0x001008u] = 1u;
    assert(InitializeControllerAndAcquireAtipState() == 4u);
    assert(reset_drive_calls == 2u && memory[0x01802bu] == 0x80u);
}

int main(void)
{
    TestControllerFailureRoutes();
    TestNonrecordableAndIndexedSuccess();
    TestAtipSuccessAndSupplemental();
    TestAcquisitionRetryAndTimeout();
    return 0;
}
