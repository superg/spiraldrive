#include "platform/firmware_map.h"

#ifndef PLEXTOR_ATA_BINDINGS_H
#define PLEXTOR_ATA_BINDINGS_H

#include "../mcs251.h"

/*
 * Host substitution is intentionally confined to binding headers.  Firmware
 * translation units call the semantic operations below without containing
 * build-mode branches or observer declarations.
 */
#if defined(PLEXTOR_HOST_TEST)
u8 ObserveHostCommandAtaDispatch(void);
void ObserveHostCommandScsiDispatch(u8 opcode);
void ObserveHostCommandRestart(void);
void ObserveHostCommandClearCompletion(void);
void ObserveHostCommandPrepareCompletion(void);
void ObserveHostCommandPublishCompletion(void);
u16 ObserveActiveHostScanControllerBanks(u8 force_bank_scan, u32 frame_count, s32 start_frame_position);
u8 ObserveActiveHostValidateScsiAdmission(void);
void ObservePendingHostSetSense(u8 sense_key, u16 sense_code);
void ObservePendingHostTrayStatus(void);

#define DispatchAtaForCurrentHostCommand() ObserveHostCommandAtaDispatch()
#define DispatchScsiForCurrentHostCommand(opcode) ObserveHostCommandScsiDispatch(opcode)
#define RestartAfterHostCommand() ObserveHostCommandRestart()
#define ClearCurrentHostCompletion() ObserveHostCommandClearCompletion()
#define PrepareCurrentHostCompletion() ObserveHostCommandPrepareCompletion()
#define PublishCurrentHostCompletion() ObserveHostCommandPublishCompletion()
#define ScanActiveHostControllerBanks(force_scan, frame_count, start_frame) ObserveActiveHostScanControllerBanks((force_scan), (frame_count), (start_frame))
#define ValidateActiveHostAdmission() ObserveActiveHostValidateScsiAdmission()
#define SetPendingHostSense(key, code) ObservePendingHostSetSense((key), (code))
#define UpdatePendingHostTrayStatus() ObservePendingHostTrayStatus()
#else
#include "../../ata/task_file_command_dispatch.h"
#include "../../scsi/command_admission.h"
#include "../../scsi/command_dispatch.h"
#include "../controller_record_bank_scan.h"
#include "../firmware_entry.h"
#include "../status_mapping.h"

#define DispatchAtaForCurrentHostCommand() HandleAtaTaskFileCommand()
#define DispatchScsiForCurrentHostCommand(opcode) DispatchCurrentScsiOpcode()
#define RestartAfterHostCommand() RestartFirmware()
#define ClearCurrentHostCompletion() ClearScsiCompletionBits0And1IfBit4Set()
#define PrepareCurrentHostCompletion() PrepareAtaCompletionStatusFromSense()
#define PublishCurrentHostCompletion() PublishAtaCommandCompletion()
#define ScanActiveHostControllerBanks(force_scan, frame_count, start_frame) ScanControllerRecordBanksForFrameRange((force_scan), (start_frame), (frame_count))
#define ValidateActiveHostAdmission() ValidateScsiCommandAdmission()
#define SetPendingHostSense(key, code) SetSenseData((key), (code))
#define UpdatePendingHostTrayStatus() SetTrayOpenOrNoSenseForCurrentCommand()
#endif

#endif
