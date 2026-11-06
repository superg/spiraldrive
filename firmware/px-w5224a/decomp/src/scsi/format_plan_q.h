#ifndef PLEXTOR_SCSI_FORMAT_PLAN_Q_H
#define PLEXTOR_SCSI_FORMAT_PLAN_Q_H

#include "platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION BuildFormatPlanPrimaryStagedRawQRecordSequence(void);
void PLEXTOR_NEAR_FUNCTION BuildFormatPlanSecondaryStagedRawQRecordSequence(void) PLEXTOR_REENTRANT;

#endif
