#ifndef PLEXTOR_SCSI_VENDOR_H
#define PLEXTOR_SCSI_VENDOR_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION CopyControllerMeasurementSeries0123ToVendorF2Response(void);
void CopyControllerMeasurementSeries456ToActiveWorkspace(void);
void PLEXTOR_NEAR_FUNCTION BuildVendorF2Mode1ControllerMeasurementResponse(void);
void SelectVendorF5Mode1BufferWindow(fw_addr_t source_output, fw_addr_t length_output);
void SelectVendorF5Mode6BufferWindow(fw_addr_t source_output, fw_addr_t length_output);
void SelectVendorF5Mode7BufferWindow(fw_addr_t source_output, fw_addr_t length_output);
void SelectVendorF5Mode5BufferWindow(fw_addr_t source_output, fw_addr_t length_output);
void HandleScsiOpcodeF7(void);
void HandleScsiOpcodeF8(void);
void HandleScsiOpcodeDE(void);

#endif
