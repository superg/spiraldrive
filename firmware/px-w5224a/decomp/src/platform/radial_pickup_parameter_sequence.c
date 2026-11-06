#include "radial_pickup_parameter_sequence.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/radial_pickup_parameter_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRadialPickupParameterWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static fw_addr_t ReadRadialPickupParameterAddress(fw_addr_t address)
{
    fw_addr_t high = ReadRadialPickupParameterWord(address);

    return (high << 16) | ReadRadialPickupParameterWord(FirmwareAddressIncrementLow16(address, 2u));
}

/* RAM:fd8bc6 */
void PLEXTOR_NEAR_FUNCTION ProgramRadialPickupMoveParameterSequenceForMagnitude(u16 scaled_magnitude)
{
    u8 table_index;
    fw_addr_t record_address;
    u16 selector;

    if(scaled_magnitude < 100u)
        table_index = 0u;
    else if(scaled_magnitude < 250u)
        table_index = 1u;
    else if(scaled_magnitude < 500u)
        table_index = 2u;
    else if(scaled_magnitude < 1000u)
        table_index = 3u;
    else if(scaled_magnitude < 1500u)
        table_index = 4u;
    else
        table_index = 5u;

    record_address = ReadRadialPickupParameterAddress(FirmwareAddressIncrementLow16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF85A8), (u16)((u16)table_index << 2)));

    while((selector = ReadRadialPickupParameterWord(record_address)) != 0xffffu)
    {
        u16 value = ReadRadialPickupParameterWord(FirmwareAddressIncrementLow16(record_address, 2u));

        if(selector == 0xf000u)
        {
            u8 port_value;

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1130) == 1u)
                port_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1131) == 1u ? (u8)value : 0x0fu;
            else
                port_value = 9u;
            WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849E, port_value);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
                WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849E, 1u);
        }
        else
        {
            SUBMIT_CONTROLLER_WORD(value, selector);
        }
        record_address = FirmwareAddressIncrementLow16(record_address, 4u);
    }
}

/* RAM:fd89ad */
void PLEXTOR_NEAR_FUNCTION ProgramAndExecuteRadialPickupMove(u8 inward_direction, u16 scaled_magnitude)
{
    u8 index;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1002, 0x7fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1003, 0x81u);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018498, FirmwareReadNear8(FW_NEAR_UNKNOWN_1002));
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018499, FirmwareReadNear8(FW_NEAR_UNKNOWN_1003));
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018496, 0xc0u);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018497, 0x80u);

    for(index = 0u; index != 3u; ++index)
    {
        u8 value = (u8)(FirmwareRead8(FirmwareAddressIncrementLow16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8908), index)) << 4);

        value |= FirmwareRead8(FirmwareAddressIncrementLow16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF890B), index));
        WRITE_CONTROLLER_PORT(FirmwareAddressIncrementLow16(FW_CONTROLLER_UNKNOWN_01849A, index), value);
    }

    PROGRAM_PARAMETER_SEQUENCE(scaled_magnitude);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180EE, (u8)scaled_magnitude);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180EF, (u8)((scaled_magnitude >> 8) | (inward_direction != 0u ? 0x80u : 0u)));
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0xc0u);

    while((s8)FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) < 0)
    {
        YIELD_COROUTINE1();
        if((READ_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849D) & 1u) != 0u)
        {
            WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
            return;
        }
    }
}
