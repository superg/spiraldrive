#include "c4_sense.h"
#include "c4_handler.h"



void spiraldrive_report_c4_failure(u8 sense_key, u16 sense_code) reentrant
{
    /* C4 has no decoded command LBA, including when a stock helper supplies the sense */
    call_stock_no_sense();
    firmware_sense_key = sense_key;
    firmware_sense_asc = (u8)(sense_code >> 8);
    firmware_sense_ascq = (u8)sense_code;
}
