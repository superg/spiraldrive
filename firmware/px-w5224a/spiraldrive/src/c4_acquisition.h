#ifndef SPIRALDRIVE_C4_ACQUISITION_H
#define SPIRALDRIVE_C4_ACQUISITION_H

#include "firmware_memory_map.h"



void spiraldrive_process_c4_acquisition(void) reentrant;

void call_acq_update_measurement(void);
void call_acq_reset_eight_channel(void);
void call_acq_strobe_port(u16 port, u8 value);
void call_acq_set_operating_enable(u8 enabled);
void call_acq_select_operating_code(void);
void call_acq_yield1(void);
void call_acq_configure_alternate(void);
void call_acq_apply_operating_code(void);
void call_acq_reset_ports(void);
void call_acq_prepare_command_0e(void);
void call_acq_service_eight_channel(void);
void call_acq_service_command_0e(void);
void call_acq_finalize_eight_channel(void);

void call_position_set_p1_bits(void);
void call_position_reset_controller_ports(void);
void call_position_select_operating_code(u8 requested);
void call_position_ramp_controller_word(void);
u16 call_position_map_zero(void);
u16 call_position_map_signed(s32 frame_seconds);
void call_position_apply_channels1_through3(void);
void call_position_configure_ports(void);
void call_position_write_port(u16 port, u8 value);

#endif
