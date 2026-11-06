#ifndef PLEXTOR_SCSI_SPC_H
#define PLEXTOR_SCSI_SPC_H

#include "../platform/mcs251.h"

/* Native entry: R7 is the starting product-ID byte index and B is the
 * prior-match flag; the result is returned in ACC through ERET.  This C value
 * form makes that register state explicit. */
u8 CompareInquiryProductIdentificationFromIndex(u8 starting_index, u8 prior_match);
u8 CompareFullInquiryProductIdentification(void);
u8 PLEXTOR_NEAR_FUNCTION AppendDriveSerialNumberFeaturePayload(fw_addr_t destination);
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceDescriptorFromFourU32Values(u16 response_length, u32 value0, u32 value1, u32 value2, u32 value3);
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceNominalSpeedDescriptor(u8 nominal_speed_code, u16 remaining_descriptor_count, u16 response_length);
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceWriteSpeedDescriptorsForSpeed(u8 speed_x, u16 current_response_length, u16 descriptor_limit);
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceDescriptorFromTableRecord(fw_addr_t performance_table_record_address, u16 response_length, u16 inherited_speed_ceiling);
u16 PLEXTOR_NEAR_FUNCTION AppendGetPerformanceDescriptorWithFlagsAndThreeU32Values(u8 flags, u16 response_length, u32 value0, u32 value1, u32 value2);
u8 PLEXTOR_NEAR_FUNCTION FindFirstTrackNumberForLastFullTocSession(void);
void HandleReadCapacity10Command(void);

#endif
