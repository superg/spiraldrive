; Native inherited-register boundary whose input cannot be named in C251.
$CASE
NAME RETURN_BOUNDARIES

PUBLIC CaptureInheritedWR6ForMeasurementDuration?

RETURN_BOUNDARIES SEGMENT ECODE
RSEG RETURN_BOUNDARIES

; Addressless first-operation capture for RAM:f97d5a's unmatched path.
; A C251 u16 result is already returned in WR6, so RET preserves the input.
CaptureInheritedWR6ForMeasurementDuration?:
    ERET

END
