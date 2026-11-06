#include "read_toc_dispatch.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"
#include "read_formatted_toc_response.h"
#include "read_full_toc_response.h"
#include "read_toc_cd_text_acquisition.h"
#include "read_toc_format1_multisession.h"
#include "read_toc_format3_pma.h"
#include "read_toc_format4_atip.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_toc_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f99a34 */
void HandleReadTocPmaAtipCommand(void)
{
    u8 format;
    u8 control;

    if(VALIDATE_READ_TOC_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DD3)) == 0u)
    {
        UNSUPPORTED_READ_TOC();
        return;
    }

    format = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) & 0x0fu);
    control = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13) & 0xc0u);
    switch(format)
    {
    case 0u:
        if(control == 0u)
            READ_FORMATTED_TOC();
        else if(control == 0x40u)
            READ_TOC_FORMAT1();
        else if(control == 0x80u)
            READ_FULL_TOC(1u);
        else
            UNSUPPORTED_READ_TOC();
        break;
    case 1u:
        READ_TOC_FORMAT1();
        break;
    case 2u:
        READ_FULL_TOC(0u);
        break;
    case 3u:
        READ_TOC_FORMAT3();
        break;
    case 4u:
        READ_TOC_FORMAT4();
        break;
    case 5u:
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_toc_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        break;
    default:
        UNSUPPORTED_READ_TOC();
        break;
    }
}
