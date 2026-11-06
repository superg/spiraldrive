#include "mode_sense_pages.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/mode_sense_pages_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fc2dcd */
u32 PLEXTOR_NEAR_FUNCTION AppendModeSensePageToControllerBuffer(u32 response_address, u8 page_code, u8 page_control) PLEXTOR_REENTRANT
{
    fw_addr_t source_address;
    u8 page_length;

    if(page_code == 0x3fu)
    {
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x01u, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x02u, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x03u, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x05u, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x07u, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x08u, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x0du, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x0eu, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x1au, page_control);
        response_address = AppendModeSensePageToControllerBuffer(response_address, 0x1du, page_control);
        return AppendModeSensePageToControllerBuffer(response_address, 0x2au, page_control);
    }

    page_length = (u8)(MODE_SENSE_PAGE_LENGTH(page_code) + 2u);
    source_address = SELECT_MODE_SENSE_PAGE_IMAGE(page_code, page_control);
    if(page_control == 0u && page_code == 0x2au)
        REFRESH_MODE_SENSE_PAGE_2A();
    if(page_control == 0u && page_code == 0x1au)
        REFRESH_MODE_SENSE_PAGE_1A();
    return COPY_MODE_SENSE_PAGE(source_address, response_address, (u32)page_length);
}
