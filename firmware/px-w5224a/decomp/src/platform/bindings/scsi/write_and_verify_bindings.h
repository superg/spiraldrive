#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSetWriteAndVerifyCommandActive(void);
extern void ObserveClearWriteAndVerifyCommandActive(void);
extern u8 ObserveSelectWriteAndVerifyTransferLengthForm(void);
extern void ObserveUnsupportedWriteAndVerifyCommand(void);
extern void ObservePrepareWriteAndVerifyCommandState(u32 start_address);
extern u8 ObserveStartOrContinueWriteAndVerifyCommandTransfer(u32 start_address, u32 block_count);
extern void ObserveSetWriteAndVerifyNoSense(void);
extern u8 ObserveExecuteWriteAndVerifyTransferBlockRange(u32 block_count);
extern void ObserveExecuteWriteAndVerifyReadback(void);
#define SET_COMMAND_ACTIVE() ObserveSetWriteAndVerifyCommandActive()
#define CLEAR_COMMAND_ACTIVE() ObserveClearWriteAndVerifyCommandActive()
#define SELECT_TRANSFER_LENGTH_FORM() ObserveSelectWriteAndVerifyTransferLengthForm()
#define HANDLE_UNSUPPORTED() ObserveUnsupportedWriteAndVerifyCommand()
#define PREPARE_WRITE_STATE(address) ObservePrepareWriteAndVerifyCommandState(address)
#define START_OR_CONTINUE(address, count) ObserveStartOrContinueWriteAndVerifyCommandTransfer((address), (count))
#define SET_NO_SENSE() ObserveSetWriteAndVerifyNoSense()
#define EXECUTE_BLOCK_RANGE(count) ObserveExecuteWriteAndVerifyTransferBlockRange(count)
#define EXECUTE_READBACK() ObserveExecuteWriteAndVerifyReadback()
#define WriteAndVerifyFunction HandleWriteAndVerify10CommandForTest
#else
extern void SetWriteAndVerifyCommandActiveBitAtomic(void);
extern void ClearWriteAndVerifyCommandActiveBitAtomic(void);
extern u8 IsWriteAndVerifyLogicalBlockBelow256(void);
extern void HandleUnsupportedScsiCommand(void);
extern u8 PrepareWriteCommandState(u32 start_address);
extern u8 StartOrContinueWriteCommandTransfer(u32 start_address, u32 block_count);
extern void SetNoSenseData(void);
extern u8 ExecuteWriteTransferBlockRange(u32 block_count);
extern void ExecuteWriteAndVerifyReadback(void);
#define SET_COMMAND_ACTIVE() SetWriteAndVerifyCommandActiveBitAtomic()
#define CLEAR_COMMAND_ACTIVE() ClearWriteAndVerifyCommandActiveBitAtomic()
#define SELECT_TRANSFER_LENGTH_FORM() IsWriteAndVerifyLogicalBlockBelow256()
#define HANDLE_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define PREPARE_WRITE_STATE(address) ((void)PrepareWriteCommandState(address))
#define START_OR_CONTINUE(address, count) StartOrContinueWriteCommandTransfer((address), (count))
#define SET_NO_SENSE() SetNoSenseData()
#define EXECUTE_BLOCK_RANGE(count) ExecuteWriteTransferBlockRange(count)
#define EXECUTE_READBACK() ExecuteWriteAndVerifyReadback()
#define WriteAndVerifyFunction HandleWriteAndVerify10Command
#endif
#endif
