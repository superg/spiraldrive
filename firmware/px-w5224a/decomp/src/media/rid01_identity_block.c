#include "rid01_identity_block.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/rid01_identity_block_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fe30b5 */
void PLEXTOR_NEAR_FUNCTION BuildRid01RecorderIdentificationBlockAtControllerAddress(u32 controller_buffer_address)
{
    u8 configuration[5];
    u8 patch[3];
    u8 configuration_status;
    u8 index;

    COPY_RID01_FIRMWARE_BYTES(controller_buffer_address, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9269), 0x60u);
    COPY_RID01_FIRMWARE_BYTES(controller_buffer_address + 0x40ul, FW_NEAR_UNKNOWN_DCD5, 0x10u);

    patch[0] = 0u;
    patch[1] = 0u;
    patch[2] = 0u;
    READ_RID01_CONFIGURATION(0x1aul, configuration, 5u);
    for(index = 0u; index < 5u; ++index)
    {
        u8 nibble = DecodeAsciiHexNibbleOrZero(configuration[index]);
        u8 pair_index = (u8)(index >> 1);

        if((index & 1u) == 0u)
            patch[pair_index] = (u8)(patch[pair_index] | nibble);
        else
            patch[pair_index + 1u] = (u8)(nibble << 4);
    }

    READ_RID01_CONFIGURATION(0x41ul, &configuration_status, 1u);
    if(configuration_status == 0u)
    {
        patch[0] = (u8)(configuration[0] >> 2);
        patch[1] = (u8)((configuration[0] << 6) | (DecodeAsciiHexNibbleOrZero(configuration[1]) << 4) | DecodeAsciiHexNibbleOrZero(configuration[2]));
        patch[2] = (u8)((DecodeAsciiHexNibbleOrZero(configuration[3]) << 4) | DecodeAsciiHexNibbleOrZero(configuration[4]));
    }

    COPY_RID01_PATCH_BYTES(controller_buffer_address + 0x18ul, patch, 3u);
}
