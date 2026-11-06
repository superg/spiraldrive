#ifndef PLEXTOR_SCSI_VENDOR_EC_RECORD_LIST_H
#define PLEXTOR_SCSI_VENDOR_EC_RECORD_LIST_H

#include "../platform/mcs251.h"

u16 PLEXTOR_NEAR_FUNCTION BuildVendorEcRecordListResponse(u8 include_all_records, u8 use_alternate_table_format);

#endif
