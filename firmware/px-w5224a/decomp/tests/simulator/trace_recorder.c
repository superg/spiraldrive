typedef unsigned char u8;
typedef unsigned int u16;

u16 edata PlextorTraceDependencyId _at_ 0x45d0;
u8 edata PlextorTraceArgumentCount _at_ 0x45d2;
u8 edata PlextorTraceArguments[16] _at_ 0x45d4;
u16 xdata PlextorTraceCount _at_ 0x2000;
u8 xdata PlextorTraceOverflow _at_ 0x2002;
u8 xdata PlextorTraceBuffer[20460] _at_ 0x2004;

void PlextorTraceRecord(void)
{
    u16 offset;
    u8 index;
    u8 xdata *record;

    if(PlextorTraceCount >= 1023u)
    {
        PlextorTraceOverflow = 1u;
        return;
    }
    offset = PlextorTraceCount * 20u;
    record = PlextorTraceBuffer + offset;
    record[0] = (u8)(PlextorTraceDependencyId >> 8);
    record[1] = (u8)PlextorTraceDependencyId;
    record[2] = PlextorTraceArgumentCount;
    record[3] = 0u;
    for(index = 0u; index < 16u; ++index)
        record[4u + index] = PlextorTraceArguments[index];
    ++PlextorTraceCount;
}
