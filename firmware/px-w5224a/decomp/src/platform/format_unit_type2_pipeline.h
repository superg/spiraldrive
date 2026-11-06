#ifndef PLEXTOR_FORMAT_UNIT_TYPE2_PIPELINE_H
#define PLEXTOR_FORMAT_UNIT_TYPE2_PIPELINE_H

#include "mcs251.h"

u8 InitializeFormatUnitCode11Type2ControllerRecordPipelineState(u8 operation_mode);
u8 PrepareFormatUnitCode11Type2ControllerRecordPipeline(void);
void PrepareMediaWritePipelineState(void);

#endif
