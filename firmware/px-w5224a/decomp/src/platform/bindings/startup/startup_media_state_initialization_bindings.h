#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartupMediaWriteBit(u8 bit_address, u8 value);
extern void ObserveStartupMediaRebuildRadialTable(void);
extern void ObserveStartupMediaClearStateFlags(void);
extern void ObserveStartupMediaWriteE8E9(u8 value);
extern void ObserveStartupMediaClearSpeedReduction(void);
extern void ObserveStartupMediaInitializeDiscControl(void);
extern u8 ObserveStartupMediaIndexedState(u8 state_code);
extern void ObserveStartupMediaCapturePosition(void);
#define STARTUP_MEDIA_WRITE_BIT(address, value) ObserveStartupMediaWriteBit((address), (value))
#define STARTUP_MEDIA_REBUILD_RADIAL() ObserveStartupMediaRebuildRadialTable()
#define STARTUP_MEDIA_CLEAR_STATE_FLAGS() ObserveStartupMediaClearStateFlags()
#define STARTUP_MEDIA_WRITE_E8E9(value) ObserveStartupMediaWriteE8E9(value)
#define STARTUP_MEDIA_CLEAR_SPEED_REDUCTION() ObserveStartupMediaClearSpeedReduction()
#define STARTUP_MEDIA_INITIALIZE_DISC_CONTROL() ObserveStartupMediaInitializeDiscControl()
#define STARTUP_MEDIA_INDEXED_STATE(value) ObserveStartupMediaIndexedState(value)
#define STARTUP_MEDIA_CAPTURE_POSITION() ObserveStartupMediaCapturePosition()
#else
#include "platform/controller_parameters.h"
#include "platform/leaf_workers.h"
#include "platform/media_state_updates.h"
#include "platform/radial_position_table.h"
#include "platform/status_mapping.h"
u8 bdata StartupMediaBitByte25 _at_ 0x25;
u8 bdata StartupMediaBitByte26 _at_ 0x26;
u8 bdata StartupMediaBitByte27 _at_ 0x27;
sbit StartupMediaBit0028 = StartupMediaBitByte25 ^ 0;
sbit StartupMediaBit0031 = StartupMediaBitByte26 ^ 1;
sbit StartupMediaBit0036 = StartupMediaBitByte26 ^ 6;
sbit StartupMediaBit0037 = StartupMediaBitByte26 ^ 7;
sbit StartupMediaBit0038 = StartupMediaBitByte27 ^ 0;
sbit StartupMediaBit003A = StartupMediaBitByte27 ^ 2;
#define STARTUP_MEDIA_WRITE_BIT(address, value)                                                                                                                                                        \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if((address) == 0x28u)                                                                                                                                                                         \
            StartupMediaBit0028 = (value);                                                                                                                                                             \
        else if((address) == 0x31u)                                                                                                                                                                    \
            StartupMediaBit0031 = (value);                                                                                                                                                             \
        else if((address) == 0x36u)                                                                                                                                                                    \
            StartupMediaBit0036 = (value);                                                                                                                                                             \
        else if((address) == 0x37u)                                                                                                                                                                    \
            StartupMediaBit0037 = (value);                                                                                                                                                             \
        else if((address) == 0x38u)                                                                                                                                                                    \
            StartupMediaBit0038 = (value);                                                                                                                                                             \
        else                                                                                                                                                                                           \
            StartupMediaBit003A = (value);                                                                                                                                                             \
    } while(0)
#define STARTUP_MEDIA_REBUILD_RADIAL() RebuildRadialPositionTableAndThresholds()
#define STARTUP_MEDIA_CLEAR_STATE_FLAGS() ClearMediaStateFlags7Through12()
#define STARTUP_MEDIA_WRITE_E8E9(value) WriteControllerParametersE8E9ForValue(value)
#define STARTUP_MEDIA_CLEAR_SPEED_REDUCTION() ClearTemporaryWriteSpeedReductionState()
#define STARTUP_MEDIA_INITIALIZE_DISC_CONTROL() InitializeDiscControlAndCalibrationState()
#define STARTUP_MEDIA_INDEXED_STATE(value) IsIndexedDriveStateActive(value)
#define STARTUP_MEDIA_CAPTURE_POSITION() ClearPositionSnapshotFlagsAndCaptureCurrentFrame()
#endif
#endif
