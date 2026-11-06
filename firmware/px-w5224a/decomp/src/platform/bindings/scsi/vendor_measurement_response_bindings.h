#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveVendorF2Mode1Response(void);
extern void ObserveVendorF2Mode2Or6Response(void);
extern void ObserveVendorF2Series0123Copy(void);
extern void ObserveVendorF2SharedResponse(void);
extern void ObserveVendorF2AlternateResponse(void);
#define BuildVendorF2Mode1Response() ObserveVendorF2Mode1Response()
#define BuildVendorF2Mode2Or6Response() ObserveVendorF2Mode2Or6Response()
#define CopyVendorF2Series0123() ObserveVendorF2Series0123Copy()
#define BuildVendorF2SharedResponse() ObserveVendorF2SharedResponse()
#define BuildVendorF2AlternateResponse() ObserveVendorF2AlternateResponse()
#else
extern void PLEXTOR_NEAR_FUNCTION BuildVendorF2SharedMeasurementResponseFields(void);
#define BuildVendorF2Mode1Response() BuildVendorF2Mode1ControllerMeasurementResponse()
#define BuildVendorF2Mode2Or6Response() BuildVendorF2Mode2Or6MeasurementResponse()
#define CopyVendorF2Series0123() CopyControllerMeasurementSeries0123ToVendorF2Response()
#define BuildVendorF2SharedResponse() BuildVendorF2SharedMeasurementResponseFields()
#define BuildVendorF2AlternateResponse() FUN_RAM_fbe11f()
#endif
#endif
