#ifndef PLEXTOR_SCSI_GET_CONFIGURATION_FEATURES_H
#define PLEXTOR_SCSI_GET_CONFIGURATION_FEATURES_H

#include "../platform/mcs251.h"

u16 PLEXTOR_NEAR_FUNCTION BuildGetConfigurationFeatureResponse(u8 request_type, u16 starting_feature_number, u16 initial_response_length);

#endif
