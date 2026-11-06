#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveBufferSetupReadBit(u8 bit_address);
extern void ObserveBufferSetupWriteBit(u8 bit_address, u8 value);
extern void ObserveBufferSetupLoadCurrentDescriptor(void);
extern void ObserveBufferSetupLoadLookaheadDescriptor(void);
extern void ObserveBufferSetupIncrementMsf(u16 address);
extern u8 ObserveBufferSetupBinaryToBcd(u8 value);
#define SETUP_READ_BIT(address) ObserveBufferSetupReadBit(address)
#define SETUP_WRITE_BIT(address, value) ObserveBufferSetupWriteBit((address), (value))
#define SETUP_LOAD_CURRENT() ObserveBufferSetupLoadCurrentDescriptor()
#define SETUP_LOAD_LOOKAHEAD() ObserveBufferSetupLoadLookaheadDescriptor()
#define SETUP_INCREMENT_MSF(address) ObserveBufferSetupIncrementMsf(address)
#define SETUP_BINARY_TO_BCD(value) ObserveBufferSetupBinaryToBcd(value)
#else
#include "platform/encoding.h"
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0028ForBufferSetup(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit002BForBufferSetup(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0030ForBufferSetup(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0032ForBufferSetup(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0035ForBufferSetup(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0036ForBufferSetup(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit002BForBufferSetup(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit002BForBufferSetup(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0032ForBufferSetup(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0032ForBufferSetup(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0030ForBufferSetup(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0033ForBufferSetup(void);
extern void PLEXTOR_NEAR_FUNCTION LoadControllerBufferSetupFromCurrentRawDescriptor(void);
extern void LoadControllerBufferLookaheadFromNextRawDescriptor(void);

static u8 ReadBufferSetupBit(u8 address)
{
    if(address == 0x28u)
        return ReadBit0028ForBufferSetup();
    if(address == 0x2bu)
        return ReadBit002BForBufferSetup();
    if(address == 0x30u)
        return ReadBit0030ForBufferSetup();
    if(address == 0x32u)
        return ReadBit0032ForBufferSetup();
    if(address == 0x35u)
        return ReadBit0035ForBufferSetup();
    return ReadBit0036ForBufferSetup();
}

static void WriteBufferSetupBit(u8 address, u8 value)
{
    if(address == 0x2bu)
        if(value != 0u)
            SetBit002BForBufferSetup();
        else
            ClearBit002BForBufferSetup();
    else if(address == 0x32u)
        if(value != 0u)
            SetBit0032ForBufferSetup();
        else
            ClearBit0032ForBufferSetup();
    else if(address == 0x30u)
        ClearBit0030ForBufferSetup();
    else if(address == 0x33u)
        ClearBit0033ForBufferSetup();
    else
        FirmwareWriteBit0043(0u);
}

#define SETUP_READ_BIT(address) ReadBufferSetupBit(address)
#define SETUP_WRITE_BIT(address, value) WriteBufferSetupBit((address), (value))
#define SETUP_LOAD_CURRENT() LoadControllerBufferSetupFromCurrentRawDescriptor()
#define SETUP_LOAD_LOOKAHEAD() LoadControllerBufferLookaheadFromNextRawDescriptor()
#define SETUP_INCREMENT_MSF(address) IncrementPackedBcdMsf(address)
#define SETUP_BINARY_TO_BCD(value) BinaryByteToPackedBcd(value)
#endif
#endif
