#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawSectorTransferBit000D(void);
extern u16 ObserveRawSectorTransferGetDescriptor(void);
extern void ObserveRawSectorTransferSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveRawSectorTransferSetInformation(u32 information);
extern void ObserveRawSectorTransferNoSense(void);
extern void ObserveRawSectorTransferConfigureAta(void);
extern void ObserveRawSectorTransferNormalize(u32 descriptor_handle, u8 header_type);
extern u32 ObserveRawSectorTransferPrepareSize(u16 block_size);
extern void ObserveRawSectorTransferStart(u32 transfer_value);
extern void ObserveRawSectorTransferYield(void);
#define RAW_SECTOR_TRANSFER_BIT000D() ObserveRawSectorTransferBit000D()
#define RAW_SECTOR_TRANSFER_GET_DESCRIPTOR() ObserveRawSectorTransferGetDescriptor()
#define RAW_SECTOR_TRANSFER_SET_SENSE(key, code) ObserveRawSectorTransferSetSense((key), (code))
#define RAW_SECTOR_TRANSFER_SET_INFORMATION(value) ObserveRawSectorTransferSetInformation(value)
#define RAW_SECTOR_TRANSFER_NO_SENSE() ObserveRawSectorTransferNoSense()
#define RAW_SECTOR_TRANSFER_CONFIGURE_ATA() ObserveRawSectorTransferConfigureAta()
#define RAW_SECTOR_TRANSFER_NORMALIZE(address, type) ObserveRawSectorTransferNormalize((address), (type))
#define RAW_SECTOR_TRANSFER_PREPARE_SIZE(size) ObserveRawSectorTransferPrepareSize(size)
#define RAW_SECTOR_TRANSFER_START(value) ObserveRawSectorTransferStart(value)
#define RAW_SECTOR_TRANSFER_YIELD() ObserveRawSectorTransferYield()
#else
#include "media/raw_read_header_normalization.h"
#include "platform/controller_parameters.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine0(void);
u8 bdata RawSectorTransferBitByte21 _at_ 0x21;
sbit RawSectorTransferBit000D = RawSectorTransferBitByte21 ^ 5;
#define RAW_SECTOR_TRANSFER_BIT000D() ((u8)RawSectorTransferBit000D)
#define RAW_SECTOR_TRANSFER_GET_DESCRIPTOR() GetRawReadSecondaryCursorDescriptorAddress()
#define RAW_SECTOR_TRANSFER_SET_SENSE(key, code) SetSenseData((key), (code))
#define RAW_SECTOR_TRANSFER_SET_INFORMATION(value) SetSenseInformation(value)
#define RAW_SECTOR_TRANSFER_NO_SENSE() SetNoSenseData()
#define RAW_SECTOR_TRANSFER_CONFIGURE_ATA() ConfigureAtaDataTransferStatus()
#define RAW_SECTOR_TRANSFER_NORMALIZE(address, type) CopyAndNormalizeRawReadHeaderToLinkedBuffer((address), (type))
#define RAW_SECTOR_TRANSFER_PREPARE_SIZE(size) FUN_RAM_fde9cf(size)
#define RAW_SECTOR_TRANSFER_START(value) StartRawReadHostTransferFromStackAddress(value)
#define RAW_SECTOR_TRANSFER_YIELD() YieldCoroutine0()
#endif
#endif
