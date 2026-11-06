#include "controller_parameters.h"
#include "firmware_memory.h"

/* RAM:f8e3d8 */
u16 DrainFourSfrD8OrD9Bytes(u8 select_sfr_d9)
{
    u8 count;
    u16 sum = 0u;

    FirmwareWriteDirectSfr8(E1, 0u);
    FirmwareWriteDirectSfr8(D1, (u8)(FirmwareReadDirectSfr8(D1) | 0x02u));
    FirmwareWriteDirectSfr8(E1, (u8)(FirmwareReadDirectSfr8(E1) | 0x80u));
    if(select_sfr_d9 == 0u)
    {
        for(count = 0u; count != 4u; ++count)
        {
            while((FirmwareReadDirectSfr8(F1) & 0x01u) == 0u)
            {
            }
            sum = (u16)(sum + FirmwareReadDirectSfr8(D8) + 0x78u);
        }
    }
    else
    {
        for(count = 0u; count != 4u; ++count)
        {
            while((FirmwareReadDirectSfr8(F1) & 0x02u) == 0u)
            {
            }
            sum = (u16)(sum + FirmwareReadDirectSfr8(D9));
        }
    }
    FirmwareWriteDirectSfr8(E1, (u8)(FirmwareReadDirectSfr8(E1) & 0x7fu));
    FirmwareWriteDirectSfr8(D1, (u8)(FirmwareReadDirectSfr8(D1) & 0xfdu));
    return sum;
}
