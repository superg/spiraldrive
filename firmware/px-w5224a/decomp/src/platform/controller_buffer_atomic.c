#include "controller_buffer_atomic.h"
#include "firmware_memory.h"
#include "memory.h"

extern u8 AcquireThirtyNineFrameControllerRecordIndexRange(u16 first_index, u16 last_index, u8 operation_mode);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_buffer_atomic_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static fw_addr_t ReadControllerRecordIndexStateTableAddress(void)
{
    return ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAFF) << 24) | ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB00) << 16) | ((fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB01) << 8)
         | (fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB02);
}

static u8 ReadControllerRecordIndexState(u16 index)
{
    fw_addr_t table_address = ReadControllerRecordIndexStateTableAddress();

    return FirmwareRead8(FirmwareAddressIncrementLow16(table_address, index));
}

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_buffer_atomic_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fbf39a */
u8 PLEXTOR_NEAR_FUNCTION TestControllerBufferBankStridePattern(u8 starting_pattern)
{
    fw_addr_t base = READ_RESERVED_CONTROLLER_BUFFER_BASE();
    u8 bank_count = (u8)((FW_HUGE_UNKNOWN_1B0000 - base) >> 15);
    fw_addr_t address = base;
    u8 bank_index;

    for(bank_index = 0u; bank_index < bank_count; ++bank_index)
    {
        WRITE_BANK_STRIDE_BYTE(address, (u8)(starting_pattern + bank_index));
        address += 0x8000ul;
    }

    address = base;
    for(bank_index = 0u; bank_index < bank_count; ++bank_index)
    {
        if(READ_BANK_STRIDE_BYTE(address) != (u8)(starting_pattern + bank_index))
            return 0u;
        address += 0x8000ul;
    }
    return 1u;
}

/* RAM:fa286a */
void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter)
{
    u8 controller_state;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0xe0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0xc3u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)controller_buffer_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(controller_buffer_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(controller_buffer_address >> 16));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018029, command_parameter);

    controller_state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAB) | 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAB, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x80u);

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_14DA) == 0u)
    {
    }

    controller_state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAB) & 0xbfu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAB, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
}

/* RAM:fa2904 */
u8 AcquireZeroStateControllerRecordIndexRuns(u16 last_index, u8 operation_mode)
{
    u16 index = 0u;
    u16 run_start = 0u;
    u16 run_end = 0u;
    u8 run_active = 0u;

    do
    {
        if(run_active == 0u && ReadControllerRecordIndexState(index) == 0u)
        {
            run_start = index;
            run_active = 1u;
        }

        if(run_active != 0u)
        {
            if(ReadControllerRecordIndexState(index) != 0u)
            {
                run_end = (u16)(index - 1u);
                run_active = 0u;
            }
            else if(index == last_index)
            {
                run_end = index;
                run_active = 0u;
            }

            if(run_active == 0u && AcquireThirtyNineFrameControllerRecordIndexRange(run_start, run_end, operation_mode) != 0u)
                return 1u;
        }

        index++;
    } while(index <= last_index);

    return 0u;
}
