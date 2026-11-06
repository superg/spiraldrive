#ifndef PLEXTOR_DRIVE_CONFIGURATION_STATE_H
#define PLEXTOR_DRIVE_CONFIGURATION_STATE_H

#include "../platform/mcs251.h"

#if defined(PLEXTOR_HOST_TEST)
enum DriveConfigurationStateEvent
{
    DRIVE_CONFIGURATION_READ_WORD = 1,
    DRIVE_CONFIGURATION_VALIDATE_STARTUP_INPUTS,
    DRIVE_CONFIGURATION_DELAY
};
#endif

void InitializeDriveConfigurationState(void);

#endif
