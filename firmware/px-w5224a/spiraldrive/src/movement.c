#include "c4_handler.h"
#include "c4_sense.h"
#include "firmware_memory_map.h"
#include "firmware_timing.h"



/* adapters to stock PX-W5224A 1.04 controller helpers */
void call_move_stock_write_port(u16 port, u8 value);
u8 call_move_stock_read_port(u16 port);
void call_move_stock_word_pair(u16 value, u16 selector);
void call_move_stock_clear_d1_bit6(void);
void call_move_stock_write_d2_d3(void);
void call_move_stock_apply_fixed_table(void);
void call_move_stock_apply_fixed_tables(void);
void call_move_stock_program_profile(u8 inward, u16 magnitude);
void call_move_stock_write_config_d1_d2_d3(void);
void call_move_stock_apply_d1_bits(void);
void call_move_stock_set_channel(u8 channel, u8 value);
void call_move_stock_strobe_port(u16 port, u8 value);
void call_move_stock_clear_channel86(void);
void call_move_stock_stop(void);


/* one raw controller move, based on stock RAM:fd07de without scaling, chunking, or a target LBA */
u8 spiraldrive_move_pickup(u16 magnitude, u8 inward) reentrant
{
    u8 status;
    u8 inner_limit = 0;
    u8 timed_out = 0;

    if(inward != 0 && firmware_pickup_home_input == 0)
        return SPIRALDRIVE_SEEK_INNER_LIMIT;

    /* a stopped controller can report immediate completion without moving the pickup */
    if(firmware_controller_channel1_value == 0 || firmware_controller_channel2_value == 0)
    {
        spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_NOT_READY, SPIRALDRIVE_ASC_INITIALIZATION_REQUIRED);
        return SPIRALDRIVE_SEEK_NONE;
    }

    /* apply the saved tracking configuration before issuing the movement */
    call_move_stock_set_channel(1, firmware_controller_channel1_value);

    if(firmware_controller_operating_mode == 1)
        call_move_stock_strobe_port(0x8007, 0x48);
    call_move_stock_clear_channel86();

    /* prepare the controller after the previous movement finishes */
    firmware_move_completion_countdown = 100;
    while((firmware_controller_port_80f8 & 0x80) != 0)
    {
        if((signed char)firmware_move_completion_countdown < 0)
        {
            call_move_stock_stop();
            spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_ABORTED_COMMAND, SPIRALDRIVE_ASC_INSUFFICIENT_TIME);
            return SPIRALDRIVE_SEEK_NONE;
        }
        call_stock_yield0();
    }
    if(firmware_controller_operating_mode == 1 || (firmware_controller_operating_mode == 0 && firmware_format_state == 2))
        call_move_stock_clear_d1_bit6();
    call_move_stock_write_d2_d3();
    call_move_stock_word_pair(0x3fff, 0x033a);
    call_move_stock_write_port(0x80d8, 0x18);
    firmware_controller_channel_countdown = 2;
    call_move_stock_write_port(0x80a4, (u8)(firmware_controller_channel2_value | 0x30));

    /* 80ee holds magnitude bits 0..7; 80ef holds bits 8..14 and the inward flag in bit 7 */
    call_move_stock_write_port(0x80e0, magnitude == 1 ? 0x60 : 0x80);
    call_move_stock_write_port(0x80ee, (u8)magnitude);
    call_move_stock_write_port(0x80ef, (u8)((magnitude >> 8) | (inward != 0 ? 0x80 : 0)));
    call_move_stock_write_port(0x80da, 1);
    call_move_stock_write_port(0x80e3, magnitude >= 10 ? 0xaf : 0xac);
    call_move_stock_write_port(0x80e2, 0x52);

    /* always load the stock calibration tables instead of depending on cached setup */
    if(magnitude == 1)
        call_move_stock_apply_fixed_table();
    else
        call_move_stock_apply_fixed_tables();
    if(magnitude >= 10)
    {
        call_move_stock_program_profile(inward, magnitude);
        call_move_stock_write_port(0x80a5, 3);
    }
    call_move_stock_write_port(0x80a6, 0);
    firmware_move_completion_countdown = 100;
    firmware_controller_port_8498_shadow = 0x10;
    firmware_controller_port_8499_shadow = 0xf0;
    call_move_stock_write_port(0x8498, firmware_controller_port_8498_shadow);
    call_move_stock_write_port(0x8499, firmware_controller_port_8499_shadow);
    call_move_stock_write_port(0x809a, 0x80);
    call_move_stock_write_port(0x84a2, 0x80);
    call_move_stock_write_port(0x8496, 0xc5);
    call_move_stock_write_port(0x8497, 0x80);

    /* start one movement and retain stock completion servicing */
    call_move_stock_write_port(0x80e4, 1);
    for(;;)
    {
        /* retain sensor detection even if the pickup releases it during controller cleanup */
        if(inward != 0 && firmware_pickup_home_input == 0)
        {
            inner_limit = 1;
            call_move_stock_stop();
            break;
        }
        if((firmware_controller_port_80f8 & 0x80) == 0)
            break;
        if((firmware_controller_runtime_flags & 8) == 0)
            call_stock_yield0();
        if((signed char)firmware_move_completion_countdown < 0)
        {
            timed_out = 1;
            call_move_stock_stop();
            break;
        }
    }
    call_move_stock_write_config_d1_d2_d3();
    call_move_stock_write_port(0x80a5, 4);

    status = call_move_stock_read_port(0x80e3) & 0xc3;
    /* a failed inward jump can finish before the sled settles onto the sensor */
    if(inward != 0 && inner_limit == 0 && (status != 0 || timed_out != 0))
    {
        u8 start_tick = firmware_timer_ticks;
        while((u8)(firmware_timer_ticks - start_tick) < 25000 / FIRMWARE_TIMER_TICK_US)
        {
            if(firmware_pickup_home_input == 0)
            {
                inner_limit = 1;
                call_move_stock_stop();
                break;
            }
            call_stock_yield0();
        }
    }
    /* restore the controller without stock refocus/retry moves, which can wait indefinitely at the limit */
    call_move_stock_set_channel(2, firmware_controller_channel2_value);
    call_move_stock_write_port(0x80a6, firmware_controller_port_80a6_shadow);
    call_move_stock_write_port(0x80d8, 0);
    call_move_stock_word_pair(0, 0x033a);
    if(firmware_controller_operating_mode == 1)
        call_move_stock_apply_d1_bits();
    if(inward != 0 && firmware_pickup_home_input == 0)
        inner_limit = 1;
    if(status == 0 && inner_limit == 0 && timed_out == 0)
    {
        firmware_move_cached_distance = magnitude;
        firmware_move_cached_direction = inward;
        firmware_move_cached_class = firmware_operating_code >= 4;
        firmware_move_cached_mode = firmware_controller_operating_mode;
    }
    else
    {
        firmware_move_cached_distance = 0;
        firmware_move_cached_direction = 2;
        firmware_move_cached_class = 2;
        firmware_move_cached_mode = 3;
    }
    firmware_current_q_position_flags &= 0xfe;
    if(inner_limit != 0)
        return SPIRALDRIVE_SEEK_INNER_LIMIT;
    if(timed_out != 0)
        spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_ABORTED_COMMAND, SPIRALDRIVE_ASC_INSUFFICIENT_TIME);
    else if(status != 0)
        spiraldrive_report_c4_failure(SPIRALDRIVE_SENSE_HARDWARE_ERROR, SPIRALDRIVE_ASC_NO_SEEK_COMPLETE);
    return SPIRALDRIVE_SEEK_NONE;
}
