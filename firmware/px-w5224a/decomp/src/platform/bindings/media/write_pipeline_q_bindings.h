#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveInitializeRawReadSectorControlForWritePipeline(void);
extern void ObserveInitializeSubchannelQIdentificationForWritePipeline(void);
extern void ObserveInitializeRawReadTrackSelectionForWritePipeline(void);
extern void ObserveClearRawReadPipelineStatusForWritePipeline(void);
extern u8 ObserveIsIsrcAvailableForWritePipeline(void);
extern void ObserveLoadIsrcCharactersForWritePipeline(void);
extern void ObserveInitializeWriteRawReadTransferLayoutForWritePipeline(void);
#define INIT_SECTOR_CONTROL() ObserveInitializeRawReadSectorControlForWritePipeline()
#define INIT_SUBCHANNEL_IDENTIFICATION() ObserveInitializeSubchannelQIdentificationForWritePipeline()
#define INIT_TRACK_SELECTION() ObserveInitializeRawReadTrackSelectionForWritePipeline()
#define CLEAR_PIPELINE_STATUS() ObserveClearRawReadPipelineStatusForWritePipeline()
#define IS_ISRC_AVAILABLE() ObserveIsIsrcAvailableForWritePipeline()
#define LOAD_ISRC_CHARACTERS() ObserveLoadIsrcCharactersForWritePipeline()
#define INIT_WRITE_TRANSFER_LAYOUT() ObserveInitializeWriteRawReadTransferLayoutForWritePipeline()
#else
extern void InitializeRawReadSectorControlState(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeSubchannelQIdentificationState(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeRawReadTrackSelectionState(void);
extern void ClearRawReadPipelineStatusState(void);
extern u8 PLEXTOR_NEAR_FUNCTION IsIsrcAvailableForCurrentTrack(void);
extern void PLEXTOR_NEAR_FUNCTION LoadIsrcCharactersForCurrentTrack(void);
extern void InitializeWriteRawReadTransferLayout(void) PLEXTOR_REENTRANT;
#define INIT_SECTOR_CONTROL() InitializeRawReadSectorControlState()
#define INIT_SUBCHANNEL_IDENTIFICATION() InitializeSubchannelQIdentificationState()
#define INIT_TRACK_SELECTION() InitializeRawReadTrackSelectionState()
#define CLEAR_PIPELINE_STATUS() ClearRawReadPipelineStatusState()
#define IS_ISRC_AVAILABLE() IsIsrcAvailableForCurrentTrack()
#define LOAD_ISRC_CHARACTERS() LoadIsrcCharactersForCurrentTrack()
#define INIT_WRITE_TRANSFER_LAYOUT() InitializeWriteRawReadTransferLayout()
#endif
#endif
