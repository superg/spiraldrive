#ifndef PLEXTOR_WRITE_RECORD_PREPARE_PREDICATE_H
#define PLEXTOR_WRITE_RECORD_PREPARE_PREDICATE_H

#include "../platform/mcs251.h"

u8 DoesWriteStartRequireControllerRecordPreparation(s32 start_address, u8 alternate_policy);

#endif
