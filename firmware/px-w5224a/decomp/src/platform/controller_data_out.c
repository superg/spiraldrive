#include "controller_data_out.h"
#include "../boot/led_boot_diagnostics.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_data_out_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fddf0b */
void ExecuteControllerDataOutTransfer(fw_addr_t destination_address, u32 transfer_length)
{
    u8 timer_sample;
    u8 timeout_counter = 0u;
    u8 yield_counter = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 8u);
    CONFIGURE_DATA_OUT_STATUS();

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0xc0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)transfer_length);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(transfer_length >> 8));

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)destination_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(destination_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(destination_address >> 16));
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805E, 0x87u);

    timer_sample = READ_DATA_OUT_TIMER();
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 8u)
    {
        if(timer_sample == READ_DATA_OUT_TIMER())
        {
            u8 previous_timeout_count;

            timer_sample++;
            if((s8)timer_sample > 9)
                timer_sample = 0u;
            previous_timeout_count = timeout_counter;
            timeout_counter++;
            if(previous_timeout_count >= 10u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 4u);
                SET_DATA_OUT_SENSE(4u, 0x1b00u);
            }
        }

        if(yield_counter++ > 50u)
        {
            YIELD_DATA_OUT();
            yield_counter = 0u;
        }
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
}

/* RAM:fdddad */
void ExecuteControllerDataInTransfer(fw_addr_t source_address, u32 transfer_length)
{
    u8 timer_sample;
    u8 timeout_counter = 0u;
    u8 yield_counter = 0u;

    LED_DATA_IN_BEGIN();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 8u);
    CONFIGURE_DATA_IN_STATUS();

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    if((transfer_length & 1ul) != 0ul)
        transfer_length++;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, (u8)transfer_length);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, (u8)(transfer_length >> 8));

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018010, (u8)source_address);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018011, (u8)(source_address >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018012, (u8)(source_address >> 16));
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805E, 0x97u);
    LED_INQUIRY_MARK(3u);

    timer_sample = READ_DATA_IN_TIMER();
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 8u)
    {
        if(timer_sample == READ_DATA_IN_TIMER())
        {
            u8 previous_timeout_count;

            timer_sample++;
            if((s8)timer_sample > 9)
                timer_sample = 0u;
            previous_timeout_count = timeout_counter;
            timeout_counter++;
            if(previous_timeout_count >= 10u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 4u);
                SET_DATA_IN_SENSE(4u, 0x1b00u);
            }
        }

        {
            u8 abort_requested = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u);

            if(abort_requested == 0u)
            {
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x08u) != 0u)
                    abort_requested = 1u;
                else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x10u) != 0u)
                    abort_requested = 1u;
            }
            if(abort_requested != 0u && FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) != 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 4u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xefu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xf7u));
            }
        }

        if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xefu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 4u);
        }

        if(yield_counter++ > 50u)
        {
            YIELD_DATA_IN();
            yield_counter = 0u;
        }
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
    LED_DATA_IN_EXIT();
}
