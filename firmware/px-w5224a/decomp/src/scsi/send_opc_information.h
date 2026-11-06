#ifndef PLEXTOR_SCSI_SEND_OPC_INFORMATION_H
#define PLEXTOR_SCSI_SEND_OPC_INFORMATION_H

#include "../platform/mcs251.h"

void HandleSendOpcInformationCommand(void);
u8 RunOpcForCurrentWriteSpeed(void);

#endif
