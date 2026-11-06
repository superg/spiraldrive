#ifndef PLEXTOR_FIRMWARE_ENTRY_H
#define PLEXTOR_FIRMWARE_ENTRY_H

void InitializeAndEnterFirmwareMain(void);
void InitializeAndEnterFirmwareMainAfterStackReset(void);
void RestartFirmware(void);

#endif
