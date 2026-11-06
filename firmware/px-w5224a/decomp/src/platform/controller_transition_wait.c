#include "controller_transition_wait.h"
#include "controller_mapping.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_transition_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadTransitionWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteTransitionWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static s16 AddSignedWordWrap(s16 value, u16 increment)
{
    return (s16)(u16)((u16)value + increment);
}

/* RAM:fd05d6 */
void WaitForControllerMeasurementWithinToleranceOrExpiry(u8 wait_for_initial_advance, u8 lower_bound_only)
{
    u16 high_word_count = 0u;
    s16 initial_countdown = (s16)ReadTransitionWord(FW_NEAR_UNKNOWN_1200);
    s16 timed_offset = 300;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 2u)
        timed_offset = 200;

    if(wait_for_initial_advance != 0u)
    {
        s16 current_countdown;

        while(initial_countdown < AddSignedWordWrap((s16)ReadTransitionWord(FW_NEAR_UNKNOWN_1200), 4u))
            MEASUREMENT_WAIT_YIELD();

        while(MEASUREMENT_WITHIN_TOLERANCE(10u, lower_bound_only) != 1u)
        {
            s16 lower_limit;
            s16 upper_limit;

            current_countdown = (s16)ReadTransitionWord(FW_NEAR_UNKNOWN_1200);
            if(current_countdown < 0)
                break;

            if(initial_countdown > AddSignedWordWrap(current_countdown, 30u))
            {
                if((s16)MEASUREMENT_READ_WORD(0x0253u) >= (s16)0x4000)
                {
                    high_word_count = (u16)(high_word_count + 1u);
                    if(high_word_count >= 40u)
                        MEASUREMENT_SET_P1_BITS();
                }
            }

            lower_limit = AddSignedWordWrap(current_countdown, (u16)timed_offset);
            upper_limit = AddSignedWordWrap(lower_limit, 50u);
            if(initial_countdown > lower_limit && initial_countdown < upper_limit)
                MEASUREMENT_SET_P1_BITS();

            MEASUREMENT_WAIT_YIELD();
        }
    }

    MEASUREMENT_SET_P1_BITS();
    MEASUREMENT_SUBMIT_WORD(0x7fffu, 0x0369u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        MEASUREMENT_STROBE_PORT(FW_CONTROLLER_UNKNOWN_018006, 0xc0u);
    MEASUREMENT_SET_CHANNEL(3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98F));
    MEASUREMENT_SET_P1_BITS();
}

/* RAM:fc8c63 */
void PLEXTOR_NEAR_FUNCTION WaitForOperatingCodeTransitionSettling(void)
{
    u32 inherited_dr12 = CAPTURE_INHERITED_DR12();
    u8 current_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 direction_allows_p0_count = 1u;
    u8 p0_high_count = 0u;
    u8 p1_low_count = 0u;
    u8 current_class;
    u8 applied_class;
    u16 applied_offset;
    u16 initial_countdown;
    fw_addr_t table_address;

    if(current_code < FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D))
    {
        SET_P1_TIMER_INPUTS();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) != 1u)
            direction_allows_p0_count = 0u;
    }

    current_class = MapOperatingCodeToTransitionClass(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    applied_class = MapOperatingCodeToTransitionClass(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D));
    applied_offset = (u16)(((u16)applied_class << 3) + ((u16)applied_class << 2));
    table_address = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF952C) + (fw_addr_t)applied_offset + (fw_addr_t)current_class * 2u;
    WriteTransitionWord(FW_NEAR_UNKNOWN_1200, ReadTransitionWord(table_address));
    initial_countdown = ReadTransitionWord(FW_NEAR_UNKNOWN_1200);

    while((s16)ReadTransitionWord(FW_NEAR_UNKNOWN_1200) > 0)
    {
        s16 halfway_countdown = (s16)(initial_countdown >> 1);

        if(halfway_countdown > (s16)ReadTransitionWord(FW_NEAR_UNKNOWN_1200))
        {
            if(READ_P0_BIT7() != 0u && direction_allows_p0_count == 1u)
                p0_high_count = (u8)(p0_high_count + 1u);
            else
                p0_high_count = 0u;
        }
        if(READ_P1_CEX0() == 0u)
            p1_low_count = (u8)(p1_low_count + 1u);
        else
            p1_low_count = 0u;
        if(p0_high_count >= 100u || p1_low_count >= 20u)
            break;
        YIELD_COROUTINE1();
    }

    FINALIZE_P1_TIMER_INPUTS();
    RESTORE_INHERITED_DR12(inherited_dr12);
}
