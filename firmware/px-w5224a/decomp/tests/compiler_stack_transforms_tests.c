#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/compiler_stack_transforms.h"

static u32 ReadU32(const u8 *bytes, u16 address)
{
    return ((u32)bytes[address] << 24) | ((u32)bytes[(u16)(address + 1u)] << 16) | ((u32)bytes[(u16)(address + 2u)] << 8) | bytes[(u16)(address + 3u)];
}

static void WriteU32(u8 *bytes, u16 address, u32 value)
{
    bytes[address] = (u8)(value >> 24);
    bytes[(u16)(address + 1u)] = (u8)(value >> 16);
    bytes[(u16)(address + 2u)] = (u8)(value >> 8);
    bytes[(u16)(address + 3u)] = (u8)value;
}

static void TestSaveTransform(void)
{
    u8 stack[256];
    CompilerRegisterStackState state;

    memset(stack, 0, sizeof(stack));
    memset(&state, 0, sizeof(state));
    state.dr0 = 0x00112233ul;
    state.dr4 = 0x44556677ul;
    state.dr8 = 0x8899aabbul;
    state.dr12 = 0xccddeefful;
    state.dr16 = 0x10203040ul;
    state.dr20 = 0x50607080ul;
    state.dr24 = 0x90a0b0c0ul;
    state.sp = 0x40u;
    state.stack = stack;
    stack[0x39u] = 0xdeu;
    stack[0x3au] = 0xadu;
    stack[0x3bu] = 0xbeu;
    stack[0x3cu] = 0xefu;

    SaveDataRegistersAndDuplicateReturnAddress(&state);

    assert(state.sp == 0x60u);
    assert(ReadU32(stack, 0x41u) == 0x00112233ul);
    assert(ReadU32(stack, 0x45u) == 0x44556677ul);
    assert(ReadU32(stack, 0x49u) == 0x8899aabbul);
    assert(ReadU32(stack, 0x4du) == 0xccddeefful);
    assert(ReadU32(stack, 0x51u) == 0x10203040ul);
    assert(ReadU32(stack, 0x55u) == 0x50607080ul);
    assert(ReadU32(stack, 0x59u) == 0x90a0b0c0ul);
    assert(state.dr24 == 0xbeefdeadul);
    assert(ReadU32(stack, 0x5du) == 0xbeefdeadul);
}

static void TestRestoreTransform(void)
{
    u8 stack[256];
    CompilerRegisterStackState state;

    memset(stack, 0, sizeof(stack));
    memset(&state, 0xff, sizeof(state));
    state.stack = stack;
    state.sp = 0x80u;
    stack[0x80u] = 0x12u;
    stack[0x81u] = 0x34u;
    stack[0x82u] = 0x56u;
    stack[0x83u] = 0x78u;
    WriteU32(stack, 0x61u, 0x00112233ul);
    WriteU32(stack, 0x65u, 0x44556677ul);
    WriteU32(stack, 0x69u, 0x8899aabbul);
    WriteU32(stack, 0x6du, 0xccddeefful);
    WriteU32(stack, 0x71u, 0x10203040ul);
    WriteU32(stack, 0x75u, 0x50607080ul);
    WriteU32(stack, 0x79u, 0x90a0b0c0ul);

    RestoreDataRegistersAndRelocateReturnAddress(&state);

    assert(state.sp == 0x60u);
    assert(state.dr0 == 0x34112233ul);
    assert(state.dr4 == 0x44556677ul);
    assert(state.dr8 == 0x8899aabbul);
    assert(state.dr12 == 0xccddeefful);
    assert(state.dr16 == 0x10203040ul);
    assert(state.dr20 == 0x50607080ul);
    assert(state.dr24 == 0x90a0b0c0ul);
    assert(stack[0x5eu] == 0x56u && stack[0x5fu] == 0x78u);
    assert(stack[0x60u] == 0x12u && stack[0x61u] == 0x34u);
}

int main(void)
{
    TestSaveTransform();
    TestRestoreTransform();
    puts("compiler stack transform tests passed");
    return 0;
}
