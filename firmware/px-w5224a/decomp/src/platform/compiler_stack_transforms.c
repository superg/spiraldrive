#include "compiler_stack_transforms.h"

static u16 ReadCompilerStackU16(const CompilerRegisterStackState *state, u16 address)
{
    return (u16)(((u16)state->stack[address] << 8) | state->stack[(u16)(address + 1u)]);
}

static void WriteCompilerStackU16(CompilerRegisterStackState *state, u16 address, u16 value)
{
    state->stack[address] = (u8)(value >> 8);
    state->stack[(u16)(address + 1u)] = (u8)value;
}

static void PushCompilerStackU32(CompilerRegisterStackState *state, u32 value)
{
    state->stack[(u16)(state->sp + 1u)] = (u8)(value >> 24);
    state->stack[(u16)(state->sp + 2u)] = (u8)(value >> 16);
    state->stack[(u16)(state->sp + 3u)] = (u8)(value >> 8);
    state->stack[(u16)(state->sp + 4u)] = (u8)value;
    state->sp = (u16)(state->sp + 4u);
}

static u32 PopCompilerStackU32(CompilerRegisterStackState *state)
{
    u16 address = (u16)(state->sp - 3u);
    u32 value = (u32)state->stack[address] << 24;

    value |= (u32)state->stack[(u16)(address + 1u)] << 16;
    value |= (u32)state->stack[(u16)(address + 2u)] << 8;
    value |= state->stack[(u16)(address + 3u)];
    state->sp = (u16)(state->sp - 4u);
    return value;
}

/* RAM:fee571 */
/* The explicit state replaces the private DR0..DR24/SPX entry contract. */
void PLEXTOR_NEAR_FUNCTION SaveDataRegistersAndDuplicateReturnAddress(CompilerRegisterStackState *state)
{
    u16 word24;
    u16 word26;

    PushCompilerStackU32(state, state->dr0);
    PushCompilerStackU32(state, state->dr4);
    PushCompilerStackU32(state, state->dr8);
    PushCompilerStackU32(state, state->dr12);
    PushCompilerStackU32(state, state->dr16);
    PushCompilerStackU32(state, state->dr20);
    PushCompilerStackU32(state, state->dr24);
    word24 = ReadCompilerStackU16(state, (u16)(state->sp - 0x21u));
    word26 = ReadCompilerStackU16(state, (u16)(state->sp - 0x23u));
    state->dr24 = ((u32)word24 << 16) | word26;
    PushCompilerStackU32(state, state->dr24);
}

/* RAM:fee58a */
/* This is the inverse private stack/register transform with explicit state. */
void PLEXTOR_NEAR_FUNCTION RestoreDataRegistersAndRelocateReturnAddress(CompilerRegisterStackState *state)
{
    u16 word24 = ReadCompilerStackU16(state, state->sp);
    u16 word26 = ReadCompilerStackU16(state, (u16)(state->sp + 2u));

    state->sp = (u16)(state->sp - 4u);
    WriteCompilerStackU16(state, (u16)(state->sp - 0x1cu), word24);
    WriteCompilerStackU16(state, (u16)(state->sp - 0x1eu), word26);
    state->dr24 = PopCompilerStackU32(state);
    state->dr20 = PopCompilerStackU32(state);
    state->dr16 = PopCompilerStackU32(state);
    state->dr12 = PopCompilerStackU32(state);
    state->dr8 = PopCompilerStackU32(state);
    state->dr4 = PopCompilerStackU32(state);
    state->dr0 = PopCompilerStackU32(state);
}
