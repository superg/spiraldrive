typedef unsigned char u8;
typedef unsigned long fw_addr_t;

extern const u8 RelocatableFirmwareData[4];

fw_addr_t RelocatableFirmwareDataAddress(void)
{
    return (fw_addr_t)&RelocatableFirmwareData[3];
}

const u8 RelocatableFirmwareData[4] = { 0x12u, 0x34u, 0x56u, 0x78u };
