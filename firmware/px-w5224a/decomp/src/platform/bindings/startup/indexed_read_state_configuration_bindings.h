#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveIndexedReadConvertMsf(fw_addr_t tuple_address);
extern s32 ObserveIndexedReadDivide(s32 dividend, s32 divisor);
extern u16 ObserveIndexedReadMapSeconds(u16 frame_seconds);
extern void ObserveIndexedReadConfigurationByte(fw_addr_t offset, u8 *destination, u16 count);
extern void ObserveIndexedReadInitializeSubsystem(void);
extern void ObserveIndexedReadAdvanceStateMachine(void);
extern void ObserveIndexedReadResetPseudoRandom(u16 seed);
extern void ObserveIndexedReadWriteBit(u16 bit_address, u8 value);
#define INDEXED_READ_CONVERT(address) ObserveIndexedReadConvertMsf(address)
#define INDEXED_READ_DIVIDE(dividend, divisor) ObserveIndexedReadDivide((dividend), (divisor))
#define INDEXED_READ_MAP(seconds) ObserveIndexedReadMapSeconds(seconds)
#define INDEXED_READ_CONFIGURATION(offset, destination, count) ObserveIndexedReadConfigurationByte((offset), (destination), (count))
#define INDEXED_READ_INITIALIZE_SUBSYSTEM() ObserveIndexedReadInitializeSubsystem()
#define INDEXED_READ_ADVANCE() ObserveIndexedReadAdvanceStateMachine()
#define INDEXED_READ_RESET(seed) ObserveIndexedReadResetPseudoRandom(seed)
#define INDEXED_READ_WRITE_BIT(address, value) ObserveIndexedReadWriteBit((address), (value))
#else
#include "platform/arithmetic.h"
#include "platform/encoding.h"
#include "platform/indexed_state_subsystem.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
u8 bdata IndexedReadBitByte26 _at_ 0x26;
sbit IndexedReadBit0037 = IndexedReadBitByte26 ^ 7;
sbit IndexedReadP3Bit0 = 0xb0 ^ 0;
sbit IndexedReadP3Bit1 = 0xb0 ^ 1;
#define INDEXED_READ_CONVERT(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define INDEXED_READ_DIVIDE(dividend, divisor) DivideS32Quotient((dividend), (divisor))
#define INDEXED_READ_MAP(seconds) MapFrameSecondsToRadialModelValue(seconds)
#define INDEXED_READ_CONFIGURATION(offset, destination, count) ReadDriveConfigurationBytes((offset), (fw_addr_t)(destination), (count))
#define INDEXED_READ_INITIALIZE_SUBSYSTEM() InitializeIndexedStateSubsystem()
#define INDEXED_READ_ADVANCE() AdvanceMediaReadConfigurationStateMachine()
#define INDEXED_READ_RESET(seed) ResetPseudoRandomGenerator(seed)
#define INDEXED_READ_WRITE_BIT(address, value)                                                                                                                                                         \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if((address) == 0x0037u)                                                                                                                                                                       \
            IndexedReadBit0037 = (value);                                                                                                                                                              \
        else if((address) == 0x0580u)                                                                                                                                                                  \
            IndexedReadP3Bit0 = (value);                                                                                                                                                               \
        else                                                                                                                                                                                           \
            IndexedReadP3Bit1 = (value);                                                                                                                                                               \
    } while(0)
#endif
#endif
