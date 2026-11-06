#include "raw_read_buffer_setup.h"
#include "arithmetic.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/raw_read_buffer_setup_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadRawReadBufferU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:faaca4 */
void ClearRawReadControllerBufferRegions(void)
{
    u32 ring_span;
    u32 buffer_base;
    u32 reserved_region_base;
    u8 use_alternate_main_span;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 1u);
    FILL_RAW_READ_BUFFER(ReadRawReadBufferU32(FW_NEAR_UNKNOWN_1572), 0x24c00ul, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);

    use_alternate_main_span = (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x10u);
    ring_span = MultiplyU32Low(ReadRawReadBufferU32(FW_NEAR_UNKNOWN_156A), 0x930ul);
    if(use_alternate_main_span != 0u)
    {
        FILL_RAW_READ_BUFFER(0x00016650ul, ring_span - 0x00016650ul, 0u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xefu));
    }
    else
    {
        FILL_RAW_READ_BUFFER(0u, ring_span, 0u);
    }

    ring_span = MultiplyU32Low(ReadRawReadBufferU32(FW_NEAR_UNKNOWN_156A), 0x126ul);
    buffer_base = ReadRawReadBufferU32(FW_NEAR_UNKNOWN_1562);
    FILL_RAW_READ_BUFFER(buffer_base, ring_span, 0u);
    ring_span = MultiplyU32Low(ReadRawReadBufferU32(FW_NEAR_UNKNOWN_156A), 0x60ul);
    buffer_base = ReadRawReadBufferU32(FW_NEAR_UNKNOWN_1566);
    FILL_RAW_READ_BUFFER(buffer_base, ring_span, 0u);
    ring_span = ReadRawReadBufferU32(FW_NEAR_UNKNOWN_156A) << 4;
    buffer_base = ReadRawReadBufferU32(FW_NEAR_UNKNOWN_156E);
    FILL_RAW_READ_BUFFER(buffer_base, ring_span, 0u);

    reserved_region_base = ReadRawReadBufferU32(FW_NEAR_UNKNOWN_155A);
    FILL_RAW_READ_BUFFER(ReadRawReadBufferU32(FW_NEAR_UNKNOWN_155A), FW_HUGE_UNKNOWN_1B0000 - reserved_region_base, 0u);
}
