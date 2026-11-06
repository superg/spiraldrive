#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveType2DetectedLayoutConvertMsf(fw_addr_t address);
extern u32 ObserveType2DetectedLayoutMultiply(u32 left, u32 right);
extern u8 ObserveType2DetectedLayoutProbeRecord(s32 frame_position);
extern void ObserveType2DetectedLayoutReset(void);
extern void ObserveType2DetectedLayoutConfigureRing(u8 geometry_mode);
extern void ObserveType2DetectedLayoutInitializePrimary(void);
extern void ObserveType2DetectedLayoutInitializeTables(void);
extern void ObserveType2DetectedLayoutInitializeMip(void);
extern void ObserveType2DetectedLayoutInitializeRuntime(void);
extern u8 ObserveType2DetectedLayoutProbeTdi(u8 fixed_offset);
extern u8 ObserveType2DetectedLayoutLocate(void);
extern u8 ObserveType2DetectedLayoutScan(u8 fallback_scan);
#define CONVERT_TYPE2_DETECTED_MSF(address) ObserveType2DetectedLayoutConvertMsf(address)
#define MULTIPLY_TYPE2_DETECTED(left, right) ObserveType2DetectedLayoutMultiply((left), (right))
#define PROBE_TYPE2_DETECTED_RECORD(position) ObserveType2DetectedLayoutProbeRecord(position)
#define RESET_TYPE2_DETECTED_STATE() ObserveType2DetectedLayoutReset()
#define CONFIGURE_TYPE2_DETECTED_RING(mode) ObserveType2DetectedLayoutConfigureRing(mode)
#define INITIALIZE_TYPE2_DETECTED_PRIMARY() ObserveType2DetectedLayoutInitializePrimary()
#define INITIALIZE_TYPE2_DETECTED_TABLES() ObserveType2DetectedLayoutInitializeTables()
#define INITIALIZE_TYPE2_DETECTED_MIP() ObserveType2DetectedLayoutInitializeMip()
#define INITIALIZE_TYPE2_DETECTED_RUNTIME() ObserveType2DetectedLayoutInitializeRuntime()
#define PROBE_TYPE2_DETECTED_TDI(fixed) ObserveType2DetectedLayoutProbeTdi(fixed)
#define LOCATE_TYPE2_DETECTED_WINDOW() ObserveType2DetectedLayoutLocate()
#define SCAN_TYPE2_DETECTED_REGIONS(fallback) ObserveType2DetectedLayoutScan(fallback)
#else
#define CONVERT_TYPE2_DETECTED_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define MULTIPLY_TYPE2_DETECTED(left, right) MultiplyU32Low((left), (right))
#define PROBE_TYPE2_DETECTED_RECORD(position) ProbeControllerRecordAtFrameAndMapStatus(position)
#define RESET_TYPE2_DETECTED_STATE() ResetType2ControllerRecordDetectionState()
#define CONFIGURE_TYPE2_DETECTED_RING(mode) ConfigureMainDataRingGeometry(mode)
#define INITIALIZE_TYPE2_DETECTED_PRIMARY() InitializePrimaryType2ControllerRecordDescriptorsAndBounds()
#define INITIALIZE_TYPE2_DETECTED_TABLES() InitializeType2ControllerRecordDescriptorTablesAndBounds()
#define INITIALIZE_TYPE2_DETECTED_MIP() InitializeType2ControllerRecordStateFromMipPayload()
#define INITIALIZE_TYPE2_DETECTED_RUNTIME() InitializeType2ControllerRecordRuntimeStateFromWorkingTrack()
#define PROBE_TYPE2_DETECTED_TDI(fixed) ProbeType2TdiRecordNearTrackStart(fixed)
#define LOCATE_TYPE2_DETECTED_WINDOW() LocateAndAdjustType2ControllerRecordWindow()
#define SCAN_TYPE2_DETECTED_REGIONS(fallback) ScanControllerRecordRegionGridAndCachePages(fallback)
#endif
#endif
