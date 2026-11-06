#ifndef PLEXTOR_COMPILER_STACK_TRANSFORMS_H
#define PLEXTOR_COMPILER_STACK_TRANSFORMS_H

#include "mcs251.h"

typedef struct
{
    u32 dr0;
    u32 dr4;
    u32 dr8;
    u32 dr12;
    u32 dr16;
    u32 dr20;
    u32 dr24;
    u16 sp;
    u8 *stack;
} CompilerRegisterStackState;

void PLEXTOR_NEAR_FUNCTION SaveDataRegistersAndDuplicateReturnAddress(CompilerRegisterStackState *state);
void PLEXTOR_NEAR_FUNCTION RestoreDataRegistersAndRelocateReturnAddress(CompilerRegisterStackState *state);

#endif
