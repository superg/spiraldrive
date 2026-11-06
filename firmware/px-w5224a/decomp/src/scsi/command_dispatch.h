#ifndef PLEXTOR_SCSI_COMMAND_DISPATCH_H
#define PLEXTOR_SCSI_COMMAND_DISPATCH_H

#include "../platform/mcs251.h"

u8 IsSupportedScsiOpcode(u8 opcode);
u8 ScsiCommandRequiresMedia(u8 opcode);
void DispatchScsiOpcode(u8 opcode);
void DispatchCurrentScsiOpcode(void);

#endif
