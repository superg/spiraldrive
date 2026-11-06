#ifndef PLEXTOR_HOST_COMMAND_DISPATCH_H
#define PLEXTOR_HOST_COMMAND_DISPATCH_H

#include "platform/mcs251.h"

void DispatchCurrentHostCommandAndPublishCompletion(void);
void ServiceCurrentHostCommandDuringActiveOperation(void);
void ReceiveAndDispatchHostPacketDuringActiveOperation(void);
void PLEXTOR_NEAR_FUNCTION ServicePendingHostCommandDuringMediaOperation(void);

#endif
