#ifndef PLEXTOR_RAW_READ_Q_RECORD_SECTOR_ACQUISITION_H
#define PLEXTOR_RAW_READ_Q_RECORD_SECTOR_ACQUISITION_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireRawSectorsFromQRecordTable(u32 expected_sector_count, u16 q_record_count);

#endif
