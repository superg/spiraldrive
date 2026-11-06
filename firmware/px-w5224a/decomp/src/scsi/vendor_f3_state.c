#include "vendor_f3_state.h"
#include "../platform/controller_parameters.h"
#include "../platform/controller_state_profile.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f3_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/vendor_f3_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f8b71f */
void SetVendorF3StateWordsAndFlag(u16 value)
{
    SetVendorF3StateFlag();
    FirmwareWriteNearU16(FW_NEAR_UNKNOWN_FACF, value);
    FirmwareWriteNearU16(FW_NEAR_UNKNOWN_F96E, value);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F971, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) | 0x02u));
}

/* RAM:fbf1c4 */
void PLEXTOR_NEAR_FUNCTION ExecuteVendorF3IndexedDriveCycleSequence(u8 maximum_cycle_index)
{
    u8 cycle_index = 0u;

    if(maximum_cycle_index == 0u)
        return;

    for(;;)
    {
        WRITE_VENDOR_F3_CYCLE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);

        if(cycle_index != 0u)
        {
            APPLY_VENDOR_F3_CYCLE_PROFILE(0u, 0x0b54u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 2u);
            while((s8)READ_VENDOR_F3_CYCLE_COUNTDOWN() > 0)
                ;
            APPLY_VENDOR_F3_CYCLE_PROFILE(1u, 0x0b54u);
        }

        if(cycle_index == 0u || (cycle_index >= 10u && cycle_index % 10u == 0u))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 30u);
            while((s8)READ_VENDOR_F3_CYCLE_COUNTDOWN() > 0)
                ;

            CONFIGURE_VENDOR_F3_CYCLE_DRIVE(3u, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 120u);
            while(READ_VENDOR_F3_CYCLE_INPUT_BIT2() == 0u)
            {
                if((s8)READ_VENDOR_F3_CYCLE_COUNTDOWN() < 0)
                {
                    WRITE_VENDOR_F3_CYCLE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
                    break;
                }
            }

            WRITE_VENDOR_F3_CYCLE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 5u);
            while((s8)READ_VENDOR_F3_CYCLE_COUNTDOWN() >= 0)
                ;

            CONFIGURE_VENDOR_F3_CYCLE_DRIVE(4u, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 6u);
            while((s8)READ_VENDOR_F3_CYCLE_COUNTDOWN() >= 0)
                ;
        }

        WRITE_VENDOR_F3_CYCLE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 2u);
        while((s8)READ_VENDOR_F3_CYCLE_COUNTDOWN() > 0)
            ;

        if(cycle_index == 0xffu || cycle_index == maximum_cycle_index)
            break;
        ++cycle_index;
    }

    WRITE_VENDOR_F3_CYCLE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    WRITE_VENDOR_F3_CYCLE_PORT(FW_CONTROLLER_UNKNOWN_01809A, 0u);
}
