#ifndef PLEXTOR_CONTROLLER_BUFFER_TRANSFER_H
#define PLEXTOR_CONTROLLER_BUFFER_TRANSFER_H

#include "mcs251.h"

void TransferControllerBufferSectorsAndWait(u32 first_controller_address, u32 second_controller_address, u8 sector_count);
void TransferControllerBufferWordsAndWait(u32 first_controller_address, u32 second_controller_address, u32 controller_word_count);
void CopyControllerBufferRecordPageToScratch(void);
void StartWriteControllerBufferTransfer(u8 block_count, u32 controller_buffer_address);

#endif
