#include "controller_record_find.h"
#include "firmware_memory.h"

static u32 ReadRecordFindAddress(fw_addr_t address)
{
    u16 low_word = ((u16)FirmwareRead8(address + 2u) << 8) | FirmwareRead8(address + 3u);
    u16 high_word = ((u16)FirmwareRead8(address) << 8) | FirmwareRead8(address + 1u);

    return ((u32)high_word << 16) | low_word;
}

static u8 ReadRecordFindByte(fw_addr_t record_address, u8 offset)
{
    return FirmwareRead8(FirmwareAddressIncrement24(record_address, offset));
}

/* RAM:fc7766 */
u32 FindControllerBufferSixByteRecord(u32 address_key, u8 search_mode)
{
    u32 controller_address = ReadRecordFindAddress(FW_HUGE_UNKNOWN_2BA51C);
    fw_addr_t cpu_address = FirmwareAddressIncrement24(controller_address, FW_ADDRESS_BANK_STRIDE);

    do
    {
        u8 record0 = ReadRecordFindByte(cpu_address, 0u);
        u8 record3;
        u8 record4;
        u8 record5;
        u8 record_type;

        ReadRecordFindByte(cpu_address, 1u);
        ReadRecordFindByte(cpu_address, 2u);
        record3 = ReadRecordFindByte(cpu_address, 3u);
        record4 = ReadRecordFindByte(cpu_address, 4u);
        record5 = ReadRecordFindByte(cpu_address, 5u);
        record_type = (u8)(record0 >> 4);
        if(search_mode == 0x20u)
        {
            if(record_type == 2u)
                return controller_address;
        }
        else if(search_mode == 0x31u)
        {
            if(record_type == 3u)
                return controller_address;
        }
        else
        {
            u32 record_key = ((u32)(record3 & 0x0fu) << 16) | ((u32)record4 << 8) | record5;

            if(address_key == record_key && record_type == 2u)
                return controller_address;
        }
        cpu_address = FirmwareAddressIncrement24(cpu_address, 6u);
        controller_address += 6ul;
    } while(controller_address <= ReadRecordFindAddress(FW_HUGE_UNKNOWN_2BA520));

    return 0ul;
}
