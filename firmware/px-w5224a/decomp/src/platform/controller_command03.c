#include "controller_command03.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_command03_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f8d96b */
void RunControllerCommand03AndUpdateWord03AF(void)
{
    u8 saved_channel2 = FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E);
    u16 saved_word0339;
    u16 saved_word035d;
    u16 word03af = 0u;

    RESET_CHANNEL(2u);
    saved_word0339 = READ_WORD(0x0339u);
    SUBMIT_WORD(0u, 0x0339u);
    saved_word035d = READ_WORD(0x035du);
    SUBMIT_WORD(0u, 0x035du);
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180A2, 0x92u);
    WAIT_TIMER(2000u, 0u);
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C5, 7u);
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 3u);

    if(WAIT_500_AND_YIELD() != 0u)
    {
        u16 status_word;

        WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        status_word = (u16)READ_PORT(FW_CONTROLLER_UNKNOWN_0180C7) << 8;
        word03af = (u16)(0u - status_word);
    }
    else
    {
        WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        WAIT_WITHOUT_TIMEOUT();
    }

    SUBMIT_WORD(word03af, 0x03afu);
    SUBMIT_WORD(saved_word0339, 0x0339u);
    SET_CHANNEL(1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98D));
    SUBMIT_WORD(saved_word035d, 0x035du);
    SET_CHANNEL(2u, saved_channel2);
}
