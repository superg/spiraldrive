#ifndef PLEXTOR_TRACK_STATE_H
#define PLEXTOR_TRACK_STATE_H

#include "mcs251.h"

void PLEXTOR_NEAR_FUNCTION ClearControllerBufferWordsC119AndC517(void);
void ClearWorkingTrackInformation(void);
void InitializeWorkingTrackFromTrackTableTail(void);
void PLEXTOR_NEAR_FUNCTION BuildTrackTablePrimaryStagedRawQRecordSequence(void);
void CollectNonzeroPmaAdr3TupleBytes(fw_addr_t output_address);
u8 PLEXTOR_NEAR_FUNCTION IsPmaQRecordStateComplete(void);
void BuildPmaAdr3Adr5RawQRecordSequence(fw_addr_t destination_address, fw_addr_t record_count_address);
u8 CountPmaAdr3Adr5RawQRecords(void);
void PLEXTOR_NEAR_FUNCTION BuildTrackTableSecondaryStagedRawQRecordSequence(void) PLEXTOR_REENTRANT;
void ClearRawReadPipelineStatusState(void);
void PLEXTOR_NEAR_FUNCTION ResetType2ControllerRecordDetectionState(void);
void InitializeWorkingTrackFlagsFromSelectedTrackMode(void);
void DeriveWorkingTrackStartFromStatusClassAndDataBlockType(void);
u8 DeriveWorkingTrackStartFrameAndReadPlan(void);
void SelectWorkingTrackReadPlanCodeByStatusClass(void);
u8 SelectWorkingTrackReadPlanCode(void);
void BuildWorkingTrackAdr1RawQRecord(void);
void UpdateWorkingTrackDataBlockTypeFields(u8 inherited_r7, u8 inherited_b);
u8 InitializeWorkingTrackFromTrackTableHighEntryFromRegisterInputs(u8 inherited_r7, u8 inherited_b);
u8 LoadTrackInformationAgainstSnapshotFromInheritedB(u8 track_number, u8 inherited_b);
u8 LoadTrackInformationByNumberFromInheritedB(u8 track_number, u8 inherited_b);
void UpdateTrackTableFromWorkingTrack(void);

#endif
