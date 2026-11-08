$case
NAME STOCK_CALLS
STOCK_YIELD0 EQU 0FEE529H
PUBLIC call_stock_read_event?
PUBLIC call_restore_controller_d6?
PUBLIC call_initialize_controller_operating_state?
PUBLIC call_stock_response_data_in?
PUBLIC call_stock_no_sense?
PUBLIC call_stock_yield0?
PUBLIC call_raw_range_update_progress?
PUBLIC call_raw_range_reset_queue?
PUBLIC call_raw_range_initialize_coroutine1?
PUBLIC call_raw_range_initialize_coroutine2?
PUBLIC call_raw_range_poll_host_transfer?
PUBLIC call_prepare_raw_batch?
PUBLIC call_count_raw_batch?
PUBLIC call_configure_ata_data_transfer_status?
PUBLIC call_alternate_raw_read_yield2?
PUBLIC call_stock_initialize_raw_read_speed?
PUBLIC call_alternate_step_build_descriptor?
PUBLIC call_clear_controller_buffer?
PUBLIC call_alternate_step_enqueue_descriptor?
PUBLIC call_alternate_helper_initialize_cursors?
PUBLIC call_c4_finalize_controller_capture?
PUBLIC call_acq_update_measurement?
PUBLIC call_acq_reset_eight_channel?
PUBLIC call_acq_strobe_port?
PUBLIC call_acq_set_operating_enable?
PUBLIC call_acq_select_operating_code?
PUBLIC call_acq_yield1?
PUBLIC call_acq_configure_alternate?
PUBLIC call_acq_apply_operating_code?
PUBLIC call_acq_reset_ports?
PUBLIC call_acq_prepare_command_0e?
PUBLIC call_acq_service_eight_channel?
PUBLIC call_acq_service_command_0e?
PUBLIC call_acq_finalize_eight_channel?
PUBLIC call_position_set_p1_bits?
PUBLIC call_position_reset_controller_ports?
PUBLIC call_position_select_operating_code?
PUBLIC call_select_operating_code_for_position?
PUBLIC call_position_ramp_controller_word?
PUBLIC call_position_map_zero?
PUBLIC call_position_map_signed?
PUBLIC call_position_configure_ports?
PUBLIC call_position_write_port?
PUBLIC call_move_stock_write_port?
PUBLIC call_move_stock_read_port?
PUBLIC call_move_stock_word_pair?
PUBLIC call_move_stock_clear_d1_bit6?
PUBLIC call_move_stock_write_d2_d3?
PUBLIC call_move_stock_apply_fixed_table?
PUBLIC call_move_stock_apply_fixed_tables?
PUBLIC call_move_stock_program_profile?
PUBLIC call_move_stock_write_config_d1_d2_d3?
PUBLIC call_move_stock_apply_d1_bits?
PUBLIC call_move_stock_set_channel?
PUBLIC call_move_stock_strobe_port?
PUBLIC call_move_stock_clear_channel86?
PUBLIC call_move_stock_stop?
PUBLIC call_position_apply_channels1_through3?
STOCK_CALLS SEGMENT ECODE
RSEG STOCK_CALLS
call_restore_controller_d6?: EJMP 0FC96B7H
call_initialize_controller_operating_state?: EJMP 0FB0CE4H
call_stock_read_event?:
EJMP 0F90D22H
call_stock_response_data_in?:
EJMP 0FDDD42H
call_stock_no_sense?:
EJMP 0FAFF7AH
call_stock_yield0?:
EJMP STOCK_YIELD0
; reuse stock queue and host-transfer routines through their original ABIs
call_raw_range_update_progress?:
EJMP 0FC6721H
call_raw_range_reset_queue?:
EJMP 0F927BDH
call_raw_range_initialize_coroutine1?:
MOV DR0,DR4
EJMP 0FEE4C1H
call_raw_range_initialize_coroutine2?:
MOV DR0,DR4
EJMP 0FEE4D5H
call_raw_range_poll_host_transfer?:
LCALL 0FD6FE0H
ERET
call_prepare_raw_batch?: EJMP 0FC683CH
call_count_raw_batch?: EJMP 0FC65F4H
call_configure_ata_data_transfer_status?: EJMP 0FDAFEBH
call_alternate_raw_read_yield2?:
EJMP 0FEE53FH
call_stock_initialize_raw_read_speed?:
EJMP 0FBF67CH
; The producer and its three near-returning helpers are C. Preserve the
; throttle helper's inherited DR12 across stock calls and coroutine yields.
call_alternate_step_build_descriptor?:
EJMP 0FE0000H
; C251 passes buffer_address in DR4 and byte_count in DR0, matching stock
call_clear_controller_buffer?:
CLR A
EJMP 0FACBD6H
call_alternate_step_enqueue_descriptor?:
MOV WR6,#012A2H
EJMP 0FEEFDBH
call_alternate_helper_initialize_cursors?:
EJMP 0F92955H
call_c4_finalize_controller_capture?:
EJMP 0FA8909H
; controller setup leaves retain their stock far-return ABI
call_acq_update_measurement?: EJMP 0FCEA2DH
call_acq_reset_eight_channel?: EJMP 0FA832DH
call_acq_strobe_port?:
MOV WR2,WR6
MOV WR0,#01H
EJMP 0F8A934H
call_acq_set_operating_enable?: EJMP 0F8B2CAH
call_acq_select_operating_code?: EJMP 0F8BDE2H
call_acq_yield1?: EJMP 0FEE534H
call_acq_configure_alternate?: EJMP 0F8C6EFH
call_acq_apply_operating_code?: EJMP 0F8C3D4H
call_acq_reset_ports?: EJMP 0F8E8C1H
call_acq_prepare_command_0e?: EJMP 0FA8891H
call_acq_service_eight_channel?: EJMP 0FA844CH
call_acq_service_command_0e?: EJMP 0FA89D7H
call_acq_finalize_eight_channel?: EJMP 0FA861DH
call_position_set_p1_bits?:
EJMP 0FD06CDH
call_position_reset_controller_ports?:
EJMP 0F8E82BH
call_position_select_operating_code?:
EJMP 0FC8B2AH
call_select_operating_code_for_position?:
EJMP 0FC8E1AH
call_position_ramp_controller_word?:
EJMP 0F8C7CEH
call_position_map_zero?:
SUB DR4,DR4
EJMP 0FCECDCH
call_position_map_signed?:
EJMP 0FCECDCH
call_position_configure_ports?:
EJMP 0F8F20CH
call_position_write_port?:
EJMP call_move_stock_write_port?
; Only entry/return-size adaptation for unchanged 1.04 stock routines.
call_move_stock_write_port?:
; C251 passes the 16-bit address in WR6. Stock expects 0x01:WR2 in DR0.
MOV WR2,WR6
MOV WR0,#01H
EJMP 0F8A7DCH
call_move_stock_read_port?:
MOV WR2,WR6
MOV WR0,#01H
EJMP 0F8A803H
call_move_stock_word_pair?:
; C251 uses WR6,WR4; this stock routine uses WR4,WR6.
PUSH WR2
MOV WR2,WR4
MOV WR4,WR6
MOV WR6,WR2
ECALL 0F8A828H
POP WR2
ERET
call_move_stock_clear_d1_bit6?:
EJMP 0FD0156H
call_move_stock_write_d2_d3?:
EJMP 0FCA66DH
call_move_stock_apply_fixed_table?:
LCALL 0FD0C88H
ERET
call_move_stock_apply_fixed_tables?:
LCALL 0FD0CA8H
ERET
call_move_stock_program_profile?:
LCALL 0FD0D1CH
ERET
call_move_stock_write_config_d1_d2_d3?:
EJMP 0FCA6B4H
call_move_stock_apply_d1_bits?:
EJMP 0FD0125H
call_move_stock_set_channel?:
EJMP 0F8B147H
call_move_stock_strobe_port?:
MOV WR2,WR6
MOV WR0,#01H
EJMP 0F8A934H
call_move_stock_clear_channel86?:
EJMP 0FD00F5H
call_position_apply_channels1_through3?: EJMP 0F8B265H
call_move_stock_stop?: EJMP 0F8D936H
END
