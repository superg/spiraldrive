#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveModeSenseResponseSense(u8 sense_key, u16 sense_code);
extern void ObserveModeSenseResponseUnsupported(void);
extern void ObserveModeSenseResponseNoSense(void);
extern u8 ObserveModeSenseResponsePageLength(u8 page_code);
extern u32 ObserveModeSenseResponseCopy(fw_addr_t source_address, u32 response_address, u32 byte_count);
extern u32 ObserveModeSenseResponseAppend(u32 response_address, u8 page_code, u8 page_control);
extern u8 ObserveModeSenseResponseDataIn(u32 transfer_length);
#define MODE_SENSE_SET_SENSE(key, code) ObserveModeSenseResponseSense((key), (code))
#define MODE_SENSE_UNSUPPORTED() ObserveModeSenseResponseUnsupported()
#define MODE_SENSE_NO_SENSE() ObserveModeSenseResponseNoSense()
#define MODE_SENSE_PAYLOAD_LENGTH(page) ObserveModeSenseResponsePageLength(page)
#define MODE_SENSE_COPY(source, response, count) ObserveModeSenseResponseCopy((source), (response), (count))
#define MODE_SENSE_APPEND(response, page, control) ObserveModeSenseResponseAppend((response), (page), (control))
#define MODE_SENSE_DATA_IN(length) ObserveModeSenseResponseDataIn(length)
#else
#define MODE_SENSE_SET_SENSE(key, code) SetSenseData((key), (code))
#define MODE_SENSE_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define MODE_SENSE_NO_SENSE() SetNoSenseData()
#define MODE_SENSE_PAYLOAD_LENGTH(page) GetModePagePayloadLength(page)
#define MODE_SENSE_COPY(source, response, count) CopyToControllerBufferWindow((source), (response), (count))
#define MODE_SENSE_APPEND(response, page, control) AppendModeSensePageToControllerBuffer((response), (page), (control))
#define MODE_SENSE_DATA_IN(length) ExecuteControllerDataInAndMapStatus(length)
#endif
#endif
