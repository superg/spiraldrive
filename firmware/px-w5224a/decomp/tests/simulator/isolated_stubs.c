/* Link-only dependencies for isolated leaf-function simulator images. */
typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

static volatile u32 stub_address;
static volatile u16 stub_value;

u8 CaptureR3ForFourByteBitConsensusState(void)
{
    return 0;
}

u16 ReadBigEndianU16(u32 address)
{
    stub_address = address;
    return 0;
}

void WriteBigEndianU16(u32 address, u16 value)
{
    stub_address = address;
    stub_value = value;
}
