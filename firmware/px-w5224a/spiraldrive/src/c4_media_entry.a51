$CASE
NAME C4_MEDIA_ENTRY

EXTRN ECODE (spiraldrive_run_c4_media_coroutine1??)
PUBLIC spiraldrive_c4_media_coroutine1_entry?

C4_MEDIA_ENTRY SEGMENT ECODE
RSEG C4_MEDIA_ENTRY

; A coroutine entry has no caller. C handles C4 state 2 and returns only
; when another media state needs the original scheduler's other branches.
spiraldrive_c4_media_coroutine1_entry?:
ECALL spiraldrive_run_c4_media_coroutine1??
MOV R15,#0FFH
EJMP 0F85D98H

END
