cmake_minimum_required(VERSION 3.24)
project(gplextor LANGUAGES C)

find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
include(CTest)
if(BUILD_TESTING)
  add_executable(startup_progress_diagnostics_tests
    tests/startup_progress_diagnostics_tests.c src/boot/startup_progress_diagnostics.c)
  target_include_directories(startup_progress_diagnostics_tests PRIVATE src)
  target_compile_definitions(startup_progress_diagnostics_tests PRIVATE
    PLEXTOR_HOST_TEST=1 PLEXTOR_LED_STARTUP_DIAGNOSTICS=1)
  add_test(NAME startup_progress_diagnostics_tests COMMAND startup_progress_diagnostics_tests)
endif()
option(PLEXTOR_BUILD_FIRMWARE "Enable the Keil firmware build" ON)
option(PLEXTOR_FIRMWARE_DIAGNOSTIC_LAYOUT
  "Use a non-flashable expanded layout to diagnose linker fixups" OFF)
option(PLEXTOR_LED_BOOT_DIAGNOSTICS
  "Build a flashable boot-checkpoint image using the two P3 status outputs" OFF)
option(PLEXTOR_LED_STARTUP_DIAGNOSTICS
  "Report startup progress after 8192 Timer0 ticks, then halt (diagnostic only)" OFF)
option(PLEXTOR_LED_FOREGROUND_DIAGNOSTICS
  "Emit startup checkpoint frames directly without timers or XDATA (diagnostic only)" OFF)
option(PLEXTOR_LED_HOST_GATE_DIAGNOSTICS
  "Snapshot interrupt/host state at media entry, then halt (diagnostic only)" OFF)
set(PLEXTOR_HOST_GATE_SNAPSHOT "flags" CACHE STRING
  "Host diagnostic: flags, packet, zero, interval, route, delay, or phases")
set_property(CACHE PLEXTOR_HOST_GATE_SNAPSHOT PROPERTY STRINGS flags packet zero interval route delay phases)
if(NOT PLEXTOR_HOST_GATE_SNAPSHOT MATCHES "^(flags|packet|zero|interval|route|delay|phases)$")
  message(FATAL_ERROR "PLEXTOR_HOST_GATE_SNAPSHOT must be flags, packet, zero, interval, route, delay or phases")
endif()
if((PLEXTOR_LED_BOOT_DIAGNOSTICS AND PLEXTOR_LED_STARTUP_DIAGNOSTICS) OR
   (PLEXTOR_LED_FOREGROUND_DIAGNOSTICS AND
    (PLEXTOR_LED_BOOT_DIAGNOSTICS OR PLEXTOR_LED_STARTUP_DIAGNOSTICS)) OR
   (PLEXTOR_LED_HOST_GATE_DIAGNOSTICS AND
    (PLEXTOR_LED_BOOT_DIAGNOSTICS OR PLEXTOR_LED_STARTUP_DIAGNOSTICS OR
     PLEXTOR_LED_FOREGROUND_DIAGNOSTICS)))
  message(FATAL_ERROR "Select only one LED diagnostic mode")
endif()
set(FIRMWARE_REFERENCE "${PROJECT_SOURCE_DIR}/data/W5224A104/MONT_104.bin"
  CACHE FILEPATH "Immutable firmware 1.04 used only for comparison")

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  file(GLOB_RECURSE ALL_SOURCE_FILES
    "src/*.c"
    "src/*.h"
    "tests/*.c"
    "tests/*.h"
    "tools/*.c"
    "tools/*.h")

  add_custom_target(format COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES})
  add_custom_target(check-format
    COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCE_FILES})
else()
  message(WARNING
    "clang-format not found, source code formatting targets will not be available")
endif()

if(BUILD_TESTING)
  add_test(NAME firmware_image_tests COMMAND "${Python3_EXECUTABLE}"
    -m unittest discover -s "${PROJECT_SOURCE_DIR}/tests" -v)
  set_tests_properties(firmware_image_tests PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}")
  add_test(NAME scsi_dispatch_map_validation
    COMMAND "${Python3_EXECUTABLE}"
      "${PROJECT_SOURCE_DIR}/tools/verify_scsi_dispatch_map.py")
  set_tests_properties(scsi_dispatch_map_validation PROPERTIES
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  add_test(NAME firmware_rom_data_generation
    COMMAND "${Python3_EXECUTABLE}"
      "${PROJECT_SOURCE_DIR}/tools/generate_firmware_rom_data.py"
      --firmware "${FIRMWARE_REFERENCE}"
      --output "${CMAKE_CURRENT_BINARY_DIR}/generated/firmware_rom_data.c")

  add_library(reconstruction_foundation_host STATIC
    src/boot/image.c
    src/boot/reset_vector.c
    src/boot/controller_event_interrupt_entry.c
    src/boot/host_interface_interrupt_entry.c
    src/boot/controller_packet_interrupt_entry.c
    src/boot/timer0_interrupt_entry.c
    src/boot/sfr_e1_bit7_interrupt_entry.c
    src/scsi/vendor.c
    src/scsi/vendor_f5_response.c
    src/scsi/vendor_ed.c
    src/scsi/vendor_measurement_response.c
    src/scsi/controller_command_preparation.c
    src/scsi/format_unit_code11.c
    src/scsi/lba_mapping.c
    src/scsi/play_cd.c
    src/scsi/play_audio.c
    src/scsi/play_audio_track_index.c
    src/scsi/play_track_relative.c
    src/scsi/scsi_opcode_ba_cd.c
    src/scsi/playable_audio_track.c
    src/scsi/spc.c
    src/scsi/mode_page_initialization.c
    src/scsi/read_buffer_capacity.c
    src/scsi/read_buffer.c
    src/scsi/read_buffer_snapshot.c
    src/scsi/read_buffer_snapshot_restore.c
    src/scsi/opcode_f0_configuration_update.c
    src/scsi/read_cd_engine.c
    src/scsi/read_cd_track_range_match.c
    src/scsi/read_format_capacities.c
    src/scsi/write_buffer.c
    src/scsi/write_buffer_snapshot_validation.c
    src/scsi/read_disc_information.c
    src/scsi/get_configuration.c
    src/scsi/get_configuration_features.c
    src/scsi/read_formatted_toc_response.c
    src/scsi/read_full_toc_response.c
    src/scsi/read_master_cue_status.c
    src/scsi/read_master_cue_response.c
    src/scsi/read_subchannel_isrc_f1.c
    src/scsi/read_subchannel_identification_acquisition.c
    src/scsi/read_subchannel_current_position.c
    src/scsi/read_subchannel_command.c
    src/scsi/read_toc_dispatch.c
    src/scsi/read_toc_format1_multisession.c
    src/scsi/read_toc_format3_pma.c
    src/scsi/read_toc_cd_text_acquisition.c
    src/scsi/cd_text_pack_validation.c
    src/scsi/read_toc_format5_response.c
    src/scsi/read_header_acquisition.c
    src/scsi/read_cd_track_coverage.c
    src/scsi/repair_track.c
    src/scsi/reserve_track.c
    src/scsi/send_opc_information.c
    src/startup/debounced_inputs.c
    src/startup/drive_firmware_initialization.c
    src/startup/drive_configuration_state.c
    src/startup/disc_control_state_initialization.c
    src/scsi/read_verify_range.c
    src/scsi/read_verify_finalize.c
    src/scsi/write_and_verify.c
    src/scsi/raw_sector_host_transfer_poll.c
    src/scsi/read_track_information_poll.c
    src/scsi/inquiry_configuration_refresh.c
    src/scsi/command_dispatch.c
    src/scsi/command_admission.c
    src/scsi/active_media_operation_admission.c
    src/ata/task_file_command_dispatch.c
    src/ata/host_command_dispatch.c
    src/ata/identify_packet_device.c
    src/scsi/request_sense.c
    src/scsi/start_stop_unit.c
    src/scsi/format_unit.c
    src/scsi/format_unit_track_state.c
    src/scsi/format_plan_q.c
    src/scsi/indexed_state0f_coroutine.c
    src/scsi/synchronize_cache10.c
    src/scsi/close_track_session.c
    src/scsi/write_command_range.c
    src/scsi/vendor_f2_shared.c
    src/scsi/vendor_f3_controller_mode.c
    src/scsi/vendor_f3_measurement_response.c
    src/scsi/vendor_f5_media_strategy_response.c
    src/scsi/vendor_e9.c
    src/scsi/vendor_eb.c
    src/scsi/vendor_ec_record_list.c
    src/scsi/vendor_f3.c
    src/scsi/vendor_f5.c
    src/scsi/write_command_transfer.c
    src/scsi/write_transfer_range.c
    src/scsi/write_pipeline_prepare.c
    src/scsi/write_partial_range.c
    src/scsi/write_record_prepare_predicate.c
    src/scsi/get_performance.c
    src/scsi/set_cd_speed.c
    src/scsi/blank.c
    src/scsi/mode_select.c
    src/media/active_write_pipeline.c
    src/media/atip_initialization.c
    src/media/pma_acquisition.c
    src/scsi/mmc_write.c
    src/scsi/vendor_raw_read.c
    src/scsi/vendor_f3_state.c
    src/scsi/mode_sense_pages.c
    src/scsi/mode_page2a.c
    src/scsi/mode_sense_response.c
    src/media/decoder.c
    src/media/session_boundary.c
    src/media/atip_capacity.c
    src/media/write_command_raw_read_prefix.c
    src/media/write_command_raw_read_pipeline.c
    src/media/write_preservation_read.c
    src/media/raw_read_controller_transfer_batch.c
    src/media/synthetic_write_descriptor_batch.c
    src/media/controller_event_bit7_progress.c
    src/media/controller_q.c
    src/media/controller_measurement_dispatch.c
    src/media/disc_recognition.c
    src/media/read_acquisition_distance.c
    src/media/disc_read_position.c
    src/media/radial_step_scale_calibration.c
    src/media/read_acquisition_radial_distance.c
    src/media/operating_code_selection.c
    src/media/read_acquisition_request.c
    src/media/raw_read_descriptor_completion.c
    src/media/raw_read_header_normalization.c
    src/media/raw_sector_header_inspect.c
    src/media/raw_sector_descriptor_status.c
    src/media/raw_sector_transfer_setup.c
    src/media/raw_sector_transfer.c
    src/media/raw_read_runtime.c
    src/media/raw_read_speed_update.c
    src/media/raw_read_pattern_validation.c
    src/media/raw_read_pattern_acquisition.c
    src/media/blank_format_track_state.c
    src/media/type2_mip_geometry.c
    src/media/raw_read_frame_sequence.c
    src/media/raw_read_ring_alignment.c
    src/media/premium_raw_batch.c
    src/media/fixed_packet_padding.c
    src/media/raw_read_range_reset.c
    src/media/calibration_offset.c
    src/media/raw_read_preparation_range.c
    src/media/raw_read_acquisition_plan.c
    src/media/configured_raw_read_preparation_segments.c
    src/media/raw_read_range_five_frame_preroll.c
    src/media/raw_read_q_record_queue.c
    src/media/raw_read_q_record_table_build.c
    src/media/format_operation_raw_read_state.c
    src/media/raw_sector_sequence.c
    src/media/lead_in_raw_sector_descriptor_sequence.c
    src/media/lead_out_raw_sector_descriptor_sequence.c
    src/media/lead_in_out_descriptor_sequences.c
    src/media/raw_read_preparation_range_end.c
    src/media/raw_track_selection.c
    src/media/coroutine_state5.c
    src/media/state3_countdown.c
    src/media/state3_indexed_transitions.c
    src/media/state5_read_acquisition_sequence.c
    src/media/read_acquisition_state.c
    src/media/state_coroutine_dispatch.c
    src/media/write_pipeline_q.c
    src/media/write_pipeline_transfer_batch_poll.c
    src/media/write_command_transfer_pipeline.c
    src/media/raw_read_q_record_table_population.c
    src/media/synthetic_raw_read_q_record_table.c
    src/media/write_pipeline_finalize.c
    src/media/post_toc_controller_state.c
    src/media/pma_q_record_reset.c
    src/media/synthetic_raw_read_state.c
    src/media/synthetic_track_table.c
    src/media/close_track_sync_operation.c
    src/media/close_sync_raw_read_prepare.c
    src/media/read_transition_coroutine.c
    src/media/raw_read_position_probe.c
    src/media/raw_read_position_probe_step.c
    src/media/packed_subchannel_record_batch.c
    src/media/raw_sector_buffer_event.c
    src/media/raw_sector_completion.c
    src/media/primary_raw_read_producer_position.c
    src/media/parsed_q_raw_read_producer_position.c
    src/media/controller_read_event.c
    src/media/primary_raw_read_operating_code.c
    src/media/primary_raw_read_pipeline_step.c
    src/media/primary_raw_read_pipeline_coroutine.c
    src/media/raw_read_pipeline_reset.c
    src/media/raw_q_position_target_probe.c
    src/media/raw_payload_signature.c
    src/platform/arithmetic.c
    src/platform/integer_division.c
    src/platform/configuration_range_update.c
    src/platform/encoding.c
    src/platform/memory.c
    src/platform/timing.c
    src/platform/timing_wait.c
    src/platform/noop_handlers.c
    src/platform/coroutine_entries.c
    src/platform/coroutine_context_switch.c
    src/platform/compiler_stack_transforms.c
    src/platform/interrupt_control.c
    src/platform/floating_result_tails.c
    src/platform/floating_floor.c
    src/platform/float32_runtime_arithmetic.c
    src/platform/float64_runtime_helpers.c
    src/platform/host_interface_interrupt.c
    src/platform/controller_operating_code_interpolation_tables.c
    src/platform/floating_runtime_internals.c
    src/platform/floating_width_conversions.c
    src/platform/integer_float32_conversions.c
    src/platform/drive_control_reset.c
    src/platform/controller_buffer_atomic.c
    src/platform/controller_buffer_transfer.c
    src/platform/buffer_transfer_setup.c
    src/platform/controller_data_out.c
    src/platform/controller_command03.c
    src/platform/controller_command_c0_calibration.c
    src/platform/controller_sample_series.c
    src/platform/frame_radial_float.c
    src/platform/operating_code_measurement_bounds.c
    src/platform/operating_code14_measurement_bounds.c
    src/platform/controller_operating_adjustment_response.c
    src/platform/controller_record_boundaries.c
    src/platform/controller_state_profile.c
    src/platform/indexed_state_operation.c
    src/platform/radial_pickup_parameter_sequence.c
    src/platform/radial_pickup_restore.c
    src/platform/scaled_radial_pickup_move_sequence.c
    src/platform/controller_operating_interpolation.c
    src/platform/firmware_programmer_handoff.c
    src/platform/format_unit_type2_pipeline.c
    src/platform/read_acquisition_request_dispatch.c
    src/platform/raw_read_opc.c
    src/platform/timer_debounce.c
    src/platform/timer_phase0.c
    src/platform/timer_phase2.c
    src/platform/sfr_da_db_measurement.c
    src/platform/controller_mapping.c
    src/platform/controller_progress.c
    src/platform/controller_progress_transition.c
    src/platform/controller_operating_range.c
    src/platform/controller_operating_window.c
    src/platform/controller_transition_wait.c
    src/platform/media_state_updates.c
    src/platform/controller_calibration.c
    src/platform/controller_calibration_outcome.c
    src/platform/controller_parameters.c
    src/platform/controller_parameter_f4_calibration.c
    src/platform/controller_operating_code_configuration.c
    src/platform/controller_operating_code_ports.c
    src/platform/controller_threshold_configuration.c
    src/platform/controller_word0319_offset_calibration.c
    src/platform/controller_words0204_0319_calibration.c
    src/platform/controller_p1_bit4.c
    src/platform/pickup_move_outputs.c
    src/platform/controller_profiles.c
    src/platform/controller_profile_selector.c
    src/platform/coroutine_initialization.c
    src/platform/firmware_entry.c
    src/platform/power_control.c
    src/platform/read_measurement_mode1.c
    src/platform/read_acquisition_controller_init.c
    src/platform/read_acquisition_coroutine3.c
    src/platform/controller_startup_mode.c
    src/platform/controller_asic_initialization.c
    src/platform/runtime_drive_state.c
    src/platform/type2_mip_state.c
    src/platform/status_mapping.c
    src/platform/write_speed_reduction.c
    src/platform/controller_threshold_probe.c
    src/platform/controller_threshold_sampling.c
    src/platform/configuration_runtime_update.c
    src/platform/indexed_state_subsystem.c
    src/platform/radial_model.c
    src/platform/radial_refinement.c
    src/platform/disc_radial_calibration.c
    src/platform/controller_position_average.c
    src/platform/radial_position_table.c
    src/platform/controller_q_packet_process.c
    src/platform/controller_measurement_reference.c
    src/platform/controller_measurement_transition.c
    src/platform/pickup_move_controller_sequence.c
    src/platform/controller_event_dispatch.c
    src/platform/controller_sfr_drain.c
    src/platform/controller_word0251_sweep.c
    src/platform/decoded_position_wait.c
    src/platform/indexed_state_dispatch.c
    src/platform/write_controller_range.c
    src/platform/write_controller_range_build.c
    src/platform/controller_record_bank_scan.c
    src/platform/controller_record_find.c
    src/platform/controller_record_page_compaction.c
    src/platform/controller_record_region_scan.c
    src/platform/controller_parameter_calibration_search.c
    src/platform/controller_parameter_fc_calibration.c
    src/platform/controller_record_slot_preparation.c
    src/platform/media_state6_lead_in_reference.c
    src/platform/working_track_probe_state.c
    src/platform/timed_output_sequence.c
    src/platform/eight_channel_capture.c
    src/platform/toc_workspace.c
    src/platform/raw_read_buffer_setup.c
    src/platform/raw_read_capacity.c
    src/platform/raw_q_record_decode.c
    src/platform/raw_read_q_control.c
    src/platform/raw_sector_capture.c
    src/platform/raw_sector_descriptor_scratch.c
    src/platform/recovery_wait.c
    src/platform/read_measurement_wait.c
    src/platform/controller_measurement_capture.c
    src/platform/controller_d8d9_window.c
    src/startup/status_routing.c
    src/platform/controller_measurement_window.c
    src/platform/controller_measurement_series02.c
    src/platform/controller_measurement_mode6_series05.c
    src/platform/controller_measurement_series05.c
    src/platform/controller_measurement_series04_value.c
    src/platform/controller_measurement_mode6_adjustment.c
    src/platform/controller_measurement_mode2_adjustment.c
    src/platform/controller_measurement_mode6_transform.c
    src/platform/controller_measurement_transform.c
    src/platform/controller_measurement_interpolation.c
    src/platform/controller_measurement_series00_01.c
    src/platform/controller_measurement_profile_initialization.c
    src/platform/controller_measurement_sequence.c
    src/platform/controller_measurement_pass.c
    src/platform/controller_measurement_retry_sequence.c
    src/platform/controller_measurement_range.c
    src/platform/controller_packet_capture.c
    src/platform/controller_measurement_recovery.c
    src/platform/controller_state6_operating_path.c
    src/platform/controller_current_state_calibration.c
    src/platform/controller_calibration_path_validation.c
    src/platform/controller_operating_state_initialization.c
    src/platform/controller_operating_state_reset.c
    src/platform/controller_port80f8_calibration_attempt.c
    src/platform/controller_state6_sample_calibration.c
    src/platform/controller_state6_calibration_finalization.c
    src/platform/controller_event_bit2_progress.c
    src/platform/controller_buffer_setup.c
    src/platform/write_transfer_cursor_alignment.c
    src/platform/leaf_workers.c
    src/platform/media_interrupt_leaves_host.c
    src/platform/track_state.c)
  target_include_directories(reconstruction_foundation_host PUBLIC src)
  target_compile_definitions(reconstruction_foundation_host PRIVATE PLEXTOR_HOST_TEST=1)
  target_compile_features(reconstruction_foundation_host PRIVATE c_std_99)

  add_executable(reconstruction_foundation_tests tests/reconstruction_foundation_tests.c)
  target_link_libraries(reconstruction_foundation_tests PRIVATE reconstruction_foundation_host)
  target_compile_definitions(reconstruction_foundation_tests PRIVATE
    PLEXTOR_HOST_TEST=1 PLEXTOR_REVIEWED_TESTS=1)
  target_include_directories(reconstruction_foundation_tests PRIVATE src)
  target_compile_features(reconstruction_foundation_tests PRIVATE c_std_99)
  add_test(NAME reconstruction_foundation_tests COMMAND reconstruction_foundation_tests)
  add_test(NAME configuration_queue_count_tests
    COMMAND reconstruction_foundation_tests --configuration-queue-count)
  add_test(NAME controller_reset_addressing_tests
    COMMAND reconstruction_foundation_tests --controller-reset-addressing)
  add_test(NAME staged_checksum_tests
    COMMAND reconstruction_foundation_tests --staged-checksum)
  add_executable(signed_division_tests
    tests/signed_division_tests.c
    src/platform/integer_division.c)
  target_compile_definitions(signed_division_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(signed_division_tests PRIVATE src)
  target_compile_features(signed_division_tests PRIVATE c_std_99)
  add_test(NAME signed_division_tests COMMAND signed_division_tests)
  add_executable(reconstruction_reviewed_tests tests/reconstruction_foundation_tests.c)
  target_link_libraries(reconstruction_reviewed_tests PRIVATE reconstruction_foundation_host)
  target_compile_definitions(reconstruction_reviewed_tests PRIVATE
    PLEXTOR_HOST_TEST=1 PLEXTOR_REVIEWED_TESTS=1)
  target_compile_features(reconstruction_reviewed_tests PRIVATE c_std_99)
  add_test(NAME reconstruction_reviewed_tests
    COMMAND reconstruction_reviewed_tests)
  add_test(NAME debounced_p1_predicates_tests
    COMMAND reconstruction_reviewed_tests --debounced-p1)
  add_executable(inquiry_configuration_refresh_tests
    tests/inquiry_configuration_refresh_tests.c
    src/scsi/inquiry_configuration_refresh.c)
  target_compile_definitions(inquiry_configuration_refresh_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(inquiry_configuration_refresh_tests PRIVATE src)
  target_compile_features(inquiry_configuration_refresh_tests PRIVATE c_std_99)
  add_test(NAME inquiry_configuration_refresh_tests
    COMMAND inquiry_configuration_refresh_tests)
  add_executable(command_admission_tests
    tests/command_admission_tests.c
    src/scsi/command_admission.c)
  target_compile_definitions(command_admission_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(command_admission_tests PRIVATE src)
  target_compile_features(command_admission_tests PRIVATE c_std_99)
  add_test(NAME command_admission_tests COMMAND command_admission_tests)
  add_executable(active_media_operation_admission_tests
    tests/active_media_operation_admission_tests.c
    src/scsi/active_media_operation_admission.c)
  target_compile_definitions(active_media_operation_admission_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(active_media_operation_admission_tests PRIVATE src)
  target_compile_features(active_media_operation_admission_tests PRIVATE c_std_99)
  add_test(NAME active_media_operation_admission_tests
    COMMAND active_media_operation_admission_tests)
  add_executable(task_file_command_dispatch_tests
    tests/task_file_command_dispatch_tests.c
    src/ata/task_file_command_dispatch.c)
  target_compile_definitions(task_file_command_dispatch_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(task_file_command_dispatch_tests PRIVATE src)
  target_compile_features(task_file_command_dispatch_tests PRIVATE c_std_99)
  add_test(NAME task_file_command_dispatch_tests
    COMMAND task_file_command_dispatch_tests)
  add_executable(host_command_dispatch_tests
    tests/host_command_dispatch_tests.c
    src/ata/host_command_dispatch.c)
  target_compile_definitions(host_command_dispatch_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(host_command_dispatch_tests PRIVATE src)
  target_compile_features(host_command_dispatch_tests PRIVATE c_std_99)
  add_test(NAME host_command_dispatch_tests
    COMMAND host_command_dispatch_tests)
  add_executable(controller_packet_capture_tests
    tests/controller_packet_capture_tests.c
    src/platform/controller_packet_capture.c)
  target_compile_definitions(controller_packet_capture_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_packet_capture_tests PRIVATE src)
  target_compile_features(controller_packet_capture_tests PRIVATE c_std_99)
  add_test(NAME controller_packet_capture_tests
    COMMAND controller_packet_capture_tests)
  add_executable(controller_measurement_recovery_tests
    tests/controller_measurement_recovery_tests.c
    src/platform/controller_measurement_recovery.c)
  target_compile_definitions(controller_measurement_recovery_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_recovery_tests PRIVATE src)
  target_compile_features(controller_measurement_recovery_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_recovery_tests
    COMMAND controller_measurement_recovery_tests)
  add_executable(controller_state6_operating_path_tests
    tests/controller_state6_operating_path_tests.c
    src/platform/controller_state6_operating_path.c)
  target_compile_definitions(controller_state6_operating_path_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_state6_operating_path_tests PRIVATE src)
  target_compile_features(controller_state6_operating_path_tests PRIVATE c_std_99)
  add_test(NAME controller_state6_operating_path_tests
    COMMAND controller_state6_operating_path_tests)
  add_executable(controller_current_state_calibration_tests
    tests/controller_current_state_calibration_tests.c
    src/platform/controller_current_state_calibration.c)
  target_compile_definitions(controller_current_state_calibration_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_current_state_calibration_tests PRIVATE src)
  target_compile_features(controller_current_state_calibration_tests PRIVATE c_std_99)
  add_test(NAME controller_current_state_calibration_tests
    COMMAND controller_current_state_calibration_tests)
  add_executable(controller_calibration_path_validation_tests
    tests/controller_calibration_path_validation_tests.c
    src/platform/controller_calibration_path_validation.c)
  target_compile_definitions(controller_calibration_path_validation_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_calibration_path_validation_tests PRIVATE src)
  target_compile_features(controller_calibration_path_validation_tests PRIVATE c_std_99)
  add_test(NAME controller_calibration_path_validation_tests
    COMMAND controller_calibration_path_validation_tests)
  add_executable(controller_operating_state_initialization_tests
    tests/controller_operating_state_initialization_tests.c
    src/platform/controller_operating_state_initialization.c)
  target_compile_definitions(controller_operating_state_initialization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_operating_state_initialization_tests PRIVATE src)
  target_compile_features(controller_operating_state_initialization_tests PRIVATE c_std_99)
  add_test(NAME controller_operating_state_initialization_tests
    COMMAND controller_operating_state_initialization_tests)
  add_executable(controller_operating_state_reset_tests
    tests/controller_operating_state_reset_tests.c
    src/platform/controller_operating_state_reset.c)
  target_compile_definitions(controller_operating_state_reset_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_operating_state_reset_tests PRIVATE src)
  target_compile_features(controller_operating_state_reset_tests PRIVATE c_std_99)
  add_test(NAME controller_operating_state_reset_tests
    COMMAND controller_operating_state_reset_tests)
  add_executable(controller_port80f8_calibration_attempt_tests
    tests/controller_port80f8_calibration_attempt_tests.c
    src/platform/controller_port80f8_calibration_attempt.c)
  target_compile_definitions(controller_port80f8_calibration_attempt_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_port80f8_calibration_attempt_tests PRIVATE src)
  target_compile_features(controller_port80f8_calibration_attempt_tests PRIVATE c_std_99)
  add_test(NAME controller_port80f8_calibration_attempt_tests
    COMMAND controller_port80f8_calibration_attempt_tests)
  add_executable(controller_state6_sample_calibration_tests
    tests/controller_state6_sample_calibration_tests.c
    src/platform/controller_state6_sample_calibration.c)
  target_compile_definitions(controller_state6_sample_calibration_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_state6_sample_calibration_tests PRIVATE src)
  target_compile_features(controller_state6_sample_calibration_tests PRIVATE c_std_99)
  add_test(NAME controller_state6_sample_calibration_tests
    COMMAND controller_state6_sample_calibration_tests)
  add_executable(controller_state6_calibration_finalization_tests
    tests/controller_state6_calibration_finalization_tests.c
    src/platform/controller_state6_calibration_finalization.c)
  target_compile_definitions(controller_state6_calibration_finalization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_state6_calibration_finalization_tests PRIVATE src)
  target_compile_features(controller_state6_calibration_finalization_tests PRIVATE c_std_99)
  add_test(NAME controller_state6_calibration_finalization_tests
    COMMAND controller_state6_calibration_finalization_tests)
  add_executable(start_stop_unit_tests
    tests/start_stop_unit_tests.c
    src/scsi/start_stop_unit.c)
  target_compile_definitions(start_stop_unit_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(start_stop_unit_tests PRIVATE src)
  target_compile_features(start_stop_unit_tests PRIVATE c_std_99)
  add_test(NAME start_stop_unit_tests COMMAND start_stop_unit_tests)
  add_executable(format_unit_tests
    tests/format_unit_tests.c
    src/scsi/format_unit.c)
  target_compile_definitions(format_unit_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(format_unit_tests PRIVATE src)
  target_compile_features(format_unit_tests PRIVATE c_std_99)
  add_test(NAME format_unit_tests COMMAND format_unit_tests)
  add_executable(format_unit_track_state_tests
    tests/format_unit_track_state_tests.c
    src/scsi/format_unit_track_state.c)
  target_compile_definitions(format_unit_track_state_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(format_unit_track_state_tests PRIVATE src)
  target_compile_features(format_unit_track_state_tests PRIVATE c_std_99)
  add_test(NAME format_unit_track_state_tests COMMAND format_unit_track_state_tests)
  add_executable(format_plan_q_tests
    tests/format_plan_q_tests.c
    src/scsi/format_plan_q.c)
  target_compile_definitions(format_plan_q_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(format_plan_q_tests PRIVATE src)
  target_compile_features(format_plan_q_tests PRIVATE c_std_99)
  add_test(NAME format_plan_q_tests COMMAND format_plan_q_tests)
  add_executable(indexed_state0f_coroutine_tests
    tests/indexed_state0f_coroutine_tests.c
    src/scsi/indexed_state0f_coroutine.c)
  target_compile_definitions(indexed_state0f_coroutine_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(indexed_state0f_coroutine_tests PRIVATE src)
  target_compile_features(indexed_state0f_coroutine_tests PRIVATE c_std_99)
  add_test(NAME indexed_state0f_coroutine_tests COMMAND indexed_state0f_coroutine_tests)
  add_executable(synchronize_cache10_tests
    tests/synchronize_cache10_tests.c
    src/scsi/synchronize_cache10.c)
  target_compile_definitions(synchronize_cache10_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(synchronize_cache10_tests PRIVATE src)
  target_compile_features(synchronize_cache10_tests PRIVATE c_std_99)
  add_test(NAME synchronize_cache10_tests COMMAND synchronize_cache10_tests)
  add_executable(active_write_pipeline_tests
    tests/active_write_pipeline_tests.c
    src/media/active_write_pipeline.c)
  target_compile_definitions(active_write_pipeline_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(active_write_pipeline_tests PRIVATE src)
  target_compile_features(active_write_pipeline_tests PRIVATE c_std_99)
  add_test(NAME active_write_pipeline_tests COMMAND active_write_pipeline_tests)
  add_executable(close_track_session_tests
    tests/close_track_session_tests.c
    src/scsi/close_track_session.c)
  target_compile_definitions(close_track_session_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(close_track_session_tests PRIVATE src)
  target_compile_features(close_track_session_tests PRIVATE c_std_99)
  add_test(NAME close_track_session_tests COMMAND close_track_session_tests)
  add_executable(write_command_range_tests
    tests/write_command_range_tests.c
    src/scsi/write_command_range.c)
  target_compile_definitions(write_command_range_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_command_range_tests PRIVATE src)
  target_compile_features(write_command_range_tests PRIVATE c_std_99)
  add_test(NAME write_command_range_tests COMMAND write_command_range_tests)
  add_executable(vendor_f2_shared_tests
    tests/vendor_f2_shared_tests.c
    src/scsi/vendor_f2_shared.c)
  target_compile_definitions(vendor_f2_shared_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_f2_shared_tests PRIVATE src)
  target_compile_features(vendor_f2_shared_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(vendor_f2_shared_tests PRIVATE m)
  endif()
  add_test(NAME vendor_f2_shared_tests COMMAND vendor_f2_shared_tests)
  add_executable(vendor_f3_controller_mode_tests
    tests/vendor_f3_controller_mode_tests.c
    src/scsi/vendor_f3_controller_mode.c)
  target_compile_definitions(vendor_f3_controller_mode_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_f3_controller_mode_tests PRIVATE src)
  target_compile_features(vendor_f3_controller_mode_tests PRIVATE c_std_99)
  add_test(NAME vendor_f3_controller_mode_tests
    COMMAND vendor_f3_controller_mode_tests)
  add_executable(vendor_f3_measurement_response_tests
    tests/vendor_f3_measurement_response_tests.c
    src/scsi/vendor_f3_measurement_response.c)
  target_compile_definitions(vendor_f3_measurement_response_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_f3_measurement_response_tests PRIVATE src)
  target_compile_features(vendor_f3_measurement_response_tests PRIVATE c_std_99)
  add_test(NAME vendor_f3_measurement_response_tests
    COMMAND vendor_f3_measurement_response_tests)
  add_executable(vendor_f5_media_strategy_response_tests
    tests/vendor_f5_media_strategy_response_tests.c
    src/scsi/vendor_f5_media_strategy_response.c)
  target_compile_definitions(vendor_f5_media_strategy_response_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_f5_media_strategy_response_tests PRIVATE src)
  target_compile_features(vendor_f5_media_strategy_response_tests PRIVATE c_std_99)
  add_test(NAME vendor_f5_media_strategy_response_tests
    COMMAND vendor_f5_media_strategy_response_tests)
  add_executable(raw_read_speed_update_tests
    tests/raw_read_speed_update_tests.c
    src/media/raw_read_speed_update.c)
  target_compile_definitions(raw_read_speed_update_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_speed_update_tests PRIVATE src)
  target_compile_features(raw_read_speed_update_tests PRIVATE c_std_99)
  add_test(NAME raw_read_speed_update_tests
    COMMAND raw_read_speed_update_tests)
  add_executable(raw_read_pattern_validation_tests
    tests/raw_read_pattern_validation_tests.c
    src/media/raw_read_pattern_validation.c)
  target_compile_definitions(raw_read_pattern_validation_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_pattern_validation_tests PRIVATE src)
  target_compile_features(raw_read_pattern_validation_tests PRIVATE c_std_99)
  add_test(NAME raw_read_pattern_validation_tests
    COMMAND raw_read_pattern_validation_tests)
  add_executable(raw_read_pattern_acquisition_tests
    tests/raw_read_pattern_acquisition_tests.c
    src/media/raw_read_pattern_acquisition.c)
  target_compile_definitions(raw_read_pattern_acquisition_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_pattern_acquisition_tests PRIVATE src)
  target_compile_features(raw_read_pattern_acquisition_tests PRIVATE c_std_99)
  add_test(NAME raw_read_pattern_acquisition_tests
    COMMAND raw_read_pattern_acquisition_tests)
  add_executable(get_performance_tests
    tests/get_performance_tests.c
    src/scsi/get_performance.c)
  target_compile_definitions(get_performance_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(get_performance_tests PRIVATE src)
  target_compile_features(get_performance_tests PRIVATE c_std_99)
  add_test(NAME get_performance_tests COMMAND get_performance_tests)
  add_executable(set_cd_speed_tests
    tests/set_cd_speed_tests.c
    src/scsi/set_cd_speed.c)
  target_compile_definitions(set_cd_speed_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(set_cd_speed_tests PRIVATE src)
  target_compile_features(set_cd_speed_tests PRIVATE c_std_99)
  add_test(NAME set_cd_speed_tests COMMAND set_cd_speed_tests)
  add_executable(blank_tests tests/blank_tests.c src/scsi/blank.c)
  target_compile_definitions(blank_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(blank_tests PRIVATE src)
  target_compile_features(blank_tests PRIVATE c_std_99)
  add_test(NAME blank_tests COMMAND blank_tests)
  add_executable(mode_select_tests
    tests/mode_select_tests.c
    src/scsi/mode_select.c)
  target_compile_definitions(mode_select_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(mode_select_tests PRIVATE src)
  target_compile_features(mode_select_tests PRIVATE c_std_99)
  add_test(NAME mode_select_tests COMMAND mode_select_tests)
  add_executable(read_master_cue_response_tests
    tests/read_master_cue_response_tests.c
    src/scsi/read_master_cue_response.c)
  target_compile_definitions(read_master_cue_response_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_master_cue_response_tests PRIVATE src)
  target_compile_features(read_master_cue_response_tests PRIVATE c_std_99)
  add_test(NAME read_master_cue_response_tests
    COMMAND read_master_cue_response_tests)
  add_executable(raw_sector_transfer_tests
    tests/raw_sector_transfer_tests.c
    src/media/raw_sector_transfer.c)
  target_compile_definitions(raw_sector_transfer_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_sector_transfer_tests PRIVATE src)
  target_compile_features(raw_sector_transfer_tests PRIVATE c_std_99)
  add_test(NAME raw_sector_transfer_tests COMMAND raw_sector_transfer_tests)
  add_executable(write_controller_range_build_tests
    tests/write_controller_range_build_tests.c
    src/platform/write_controller_range_build.c)
  target_compile_definitions(write_controller_range_build_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(write_controller_range_build_tests PRIVATE src)
  target_compile_features(write_controller_range_build_tests PRIVATE c_std_99)
  add_test(NAME write_controller_range_build_tests
    COMMAND write_controller_range_build_tests)
  add_executable(controller_record_bank_scan_tests
    tests/controller_record_bank_scan_tests.c
    src/platform/controller_record_bank_scan.c)
  target_compile_definitions(controller_record_bank_scan_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_bank_scan_tests PRIVATE src)
  target_compile_features(controller_record_bank_scan_tests PRIVATE c_std_99)
  add_test(NAME controller_record_bank_scan_tests
    COMMAND controller_record_bank_scan_tests)
  add_executable(controller_d8d9_window_tests
    tests/controller_d8d9_window_tests.c
    src/platform/controller_d8d9_window.c)
  target_compile_definitions(controller_d8d9_window_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_d8d9_window_tests PRIVATE src)
  target_compile_features(controller_d8d9_window_tests PRIVATE c_std_99)
  add_test(NAME controller_d8d9_window_tests
    COMMAND controller_d8d9_window_tests)
  add_executable(startup_status_routing_tests
    tests/startup_status_routing_tests.c
    src/startup/status_routing.c)
  target_compile_definitions(startup_status_routing_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(startup_status_routing_tests PRIVATE src)
  target_compile_features(startup_status_routing_tests PRIVATE c_std_99)
  add_test(NAME startup_status_routing_tests
    COMMAND startup_status_routing_tests)
  add_executable(drive_control_state10_tests
    tests/drive_control_state10_tests.c
    src/startup/drive_control_state10.c)
  target_compile_definitions(drive_control_state10_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(drive_control_state10_tests PRIVATE src)
  target_compile_features(drive_control_state10_tests PRIVATE c_std_99)
  add_test(NAME drive_control_state10_tests
    COMMAND drive_control_state10_tests)
  add_executable(full_toc_track_span_tests
    tests/full_toc_track_span_tests.c
    src/scsi/full_toc_track_span.c)
  target_compile_definitions(full_toc_track_span_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(full_toc_track_span_tests PRIVATE src)
  target_compile_features(full_toc_track_span_tests PRIVATE c_std_99)
  add_test(NAME full_toc_track_span_tests
    COMMAND full_toc_track_span_tests)
  add_executable(read_track_information_size_blocks_tests
    tests/read_track_information_size_blocks_tests.c
    src/scsi/read_track_information_size_blocks.c)
  target_compile_definitions(read_track_information_size_blocks_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_track_information_size_blocks_tests PRIVATE src)
  target_compile_features(read_track_information_size_blocks_tests PRIVATE c_std_99)
  add_test(NAME read_track_information_size_blocks_tests
    COMMAND read_track_information_size_blocks_tests)
  add_executable(read_track_information_free_blocks_tests
    tests/read_track_information_free_blocks_tests.c
    src/scsi/read_track_information_free_blocks.c)
  target_compile_definitions(read_track_information_free_blocks_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_track_information_free_blocks_tests PRIVATE src)
  target_compile_features(read_track_information_free_blocks_tests PRIVATE c_std_99)
  add_test(NAME read_track_information_free_blocks_tests
    COMMAND read_track_information_free_blocks_tests)
  add_executable(read_disc_information_recordable_tests
    tests/read_disc_information_recordable_tests.c
    src/scsi/read_disc_information_recordable.c)
  target_compile_definitions(read_disc_information_recordable_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_disc_information_recordable_tests PRIVATE src)
  target_compile_features(read_disc_information_recordable_tests PRIVATE c_std_99)
  add_test(NAME read_disc_information_recordable_tests
    COMMAND read_disc_information_recordable_tests)
  add_executable(send_cue_sheet_tests
    tests/send_cue_sheet_tests.c
    src/scsi/send_cue_sheet.c)
  target_compile_definitions(send_cue_sheet_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(send_cue_sheet_tests PRIVATE src)
  target_compile_features(send_cue_sheet_tests PRIVATE c_std_99)
  add_test(NAME send_cue_sheet_tests COMMAND send_cue_sheet_tests)
  add_executable(toc_session_removal_tests
    tests/toc_session_removal_tests.c
    src/media/toc_session_removal.c)
  target_compile_definitions(toc_session_removal_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(toc_session_removal_tests PRIVATE src)
  target_compile_features(toc_session_removal_tests PRIVATE c_std_99)
  add_test(NAME toc_session_removal_tests COMMAND toc_session_removal_tests)
  add_executable(toc_state_rebuild_tests
    tests/toc_state_rebuild_tests.c
    src/media/toc_state_rebuild.c)
  target_compile_definitions(toc_state_rebuild_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(toc_state_rebuild_tests PRIVATE src)
  target_compile_features(toc_state_rebuild_tests PRIVATE c_std_99)
  add_test(NAME toc_state_rebuild_tests COMMAND toc_state_rebuild_tests)
  add_executable(toc_session_append_tests
    tests/toc_session_append_tests.c
    src/media/toc_session_append.c)
  target_compile_definitions(toc_session_append_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(toc_session_append_tests PRIVATE src)
  target_compile_features(toc_session_append_tests PRIVATE c_std_99)
  add_test(NAME toc_session_append_tests COMMAND toc_session_append_tests)
  add_executable(toc_state_parse_tests
    tests/toc_state_parse_tests.c
    src/media/toc_state_parse.c)
  target_compile_definitions(toc_state_parse_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(toc_state_parse_tests PRIVATE src)
  target_compile_features(toc_state_parse_tests PRIVATE c_std_99)
  add_test(NAME toc_state_parse_tests COMMAND toc_state_parse_tests)
  add_executable(read_acquisition_controller_config_tests
    tests/read_acquisition_controller_config_tests.c
    src/platform/read_acquisition_controller_config.c)
  target_compile_definitions(read_acquisition_controller_config_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_acquisition_controller_config_tests PRIVATE src)
  target_compile_features(read_acquisition_controller_config_tests PRIVATE c_std_99)
  add_test(NAME read_acquisition_controller_config_tests
    COMMAND read_acquisition_controller_config_tests)
  add_executable(write_controller_range_sequence_tests
    tests/write_controller_range_sequence_tests.c
    src/platform/write_controller_range_sequence.c)
  target_compile_definitions(write_controller_range_sequence_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(write_controller_range_sequence_tests PRIVATE src)
  target_compile_features(write_controller_range_sequence_tests PRIVATE c_std_99)
  add_test(NAME write_controller_range_sequence_tests
    COMMAND write_controller_range_sequence_tests)
  add_executable(raw_read_preparation_segments_clone_tests
    tests/raw_read_preparation_segments_clone_tests.c
    src/platform/raw_read_preparation_segments_clone.c)
  target_compile_definitions(raw_read_preparation_segments_clone_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_preparation_segments_clone_tests PRIVATE src)
  target_compile_features(raw_read_preparation_segments_clone_tests PRIVATE c_std_99)
  add_test(NAME raw_read_preparation_segments_clone_tests
    COMMAND raw_read_preparation_segments_clone_tests)
  add_executable(raw_read_preparation_range_end_tests
    tests/raw_read_preparation_range_end_tests.c
    src/media/raw_read_preparation_range_end.c)
  target_compile_definitions(raw_read_preparation_range_end_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_preparation_range_end_tests PRIVATE src)
  target_compile_features(raw_read_preparation_range_end_tests PRIVATE c_std_99)
  add_test(NAME raw_read_preparation_range_end_tests
    COMMAND raw_read_preparation_range_end_tests)
  add_executable(raw_read_acquisition_plan_tests
    tests/raw_read_acquisition_plan_tests.c
    src/media/raw_read_acquisition_plan.c)
  target_compile_definitions(raw_read_acquisition_plan_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_acquisition_plan_tests PRIVATE src)
  target_compile_features(raw_read_acquisition_plan_tests PRIVATE c_std_99)
  add_test(NAME raw_read_acquisition_plan_tests
    COMMAND raw_read_acquisition_plan_tests)
  add_executable(configured_raw_read_preparation_segments_tests
    tests/configured_raw_read_preparation_segments_tests.c
    src/media/configured_raw_read_preparation_segments.c)
  target_compile_definitions(configured_raw_read_preparation_segments_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(configured_raw_read_preparation_segments_tests
    PRIVATE src)
  target_compile_features(configured_raw_read_preparation_segments_tests
    PRIVATE c_std_99)
  add_test(NAME configured_raw_read_preparation_segments_tests
    COMMAND configured_raw_read_preparation_segments_tests)
  add_executable(write_pipeline_transfer_batch_poll_tests
    tests/write_pipeline_transfer_batch_poll_tests.c
    src/media/write_pipeline_transfer_batch_poll.c)
  target_compile_definitions(write_pipeline_transfer_batch_poll_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_pipeline_transfer_batch_poll_tests
    PRIVATE src)
  target_compile_features(write_pipeline_transfer_batch_poll_tests
    PRIVATE c_std_99)
  add_test(NAME write_pipeline_transfer_batch_poll_tests
    COMMAND write_pipeline_transfer_batch_poll_tests)
  add_executable(write_command_transfer_pipeline_tests
    tests/write_command_transfer_pipeline_tests.c
    src/media/write_command_transfer_pipeline.c)
  target_compile_definitions(write_command_transfer_pipeline_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_command_transfer_pipeline_tests PRIVATE src)
  target_compile_features(write_command_transfer_pipeline_tests PRIVATE c_std_99)
  add_test(NAME write_command_transfer_pipeline_tests
    COMMAND write_command_transfer_pipeline_tests)
  add_executable(raw_read_q_record_table_population_tests
    tests/raw_read_q_record_table_population_tests.c
    src/media/raw_read_q_record_table_population.c)
  target_compile_definitions(raw_read_q_record_table_population_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_q_record_table_population_tests
    PRIVATE src)
  target_compile_features(raw_read_q_record_table_population_tests
    PRIVATE c_std_99)
  add_test(NAME raw_read_q_record_table_population_tests
    COMMAND raw_read_q_record_table_population_tests)
  add_executable(synthetic_raw_read_q_record_table_tests
    tests/synthetic_raw_read_q_record_table_tests.c
    src/media/synthetic_raw_read_q_record_table.c)
  target_compile_definitions(synthetic_raw_read_q_record_table_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(synthetic_raw_read_q_record_table_tests
    PRIVATE src)
  target_compile_features(synthetic_raw_read_q_record_table_tests
    PRIVATE c_std_99)
  add_test(NAME synthetic_raw_read_q_record_table_tests
    COMMAND synthetic_raw_read_q_record_table_tests)
  add_executable(raw_read_q_record_table_build_tests
    tests/raw_read_q_record_table_build_tests.c
    src/media/raw_read_q_record_table_build.c)
  target_compile_definitions(raw_read_q_record_table_build_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_q_record_table_build_tests PRIVATE src)
  target_compile_features(raw_read_q_record_table_build_tests PRIVATE c_std_99)
  add_test(NAME raw_read_q_record_table_build_tests
    COMMAND raw_read_q_record_table_build_tests)
  add_executable(blank_format_raw_read_plan_tests
    tests/blank_format_raw_read_plan_tests.c
    src/media/blank_format_raw_read_plan.c)
  target_compile_definitions(blank_format_raw_read_plan_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(blank_format_raw_read_plan_tests PRIVATE src)
  target_compile_features(blank_format_raw_read_plan_tests PRIVATE c_std_99)
  add_test(NAME blank_format_raw_read_plan_tests
    COMMAND blank_format_raw_read_plan_tests)
  add_executable(raw_read_operation_mode_speed_tests
    tests/raw_read_operation_mode_speed_tests.c
    src/media/raw_read_operation_mode_speed.c)
  target_compile_definitions(raw_read_operation_mode_speed_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_operation_mode_speed_tests PRIVATE src)
  target_compile_features(raw_read_operation_mode_speed_tests PRIVATE c_std_99)
  add_test(NAME raw_read_operation_mode_speed_tests
    COMMAND raw_read_operation_mode_speed_tests)
  add_executable(drive_operation_result_coroutine_tests
    tests/drive_operation_result_coroutine_tests.c
    src/media/drive_operation_result_coroutine.c)
  target_compile_definitions(drive_operation_result_coroutine_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(drive_operation_result_coroutine_tests PRIVATE src)
  target_compile_features(drive_operation_result_coroutine_tests PRIVATE c_std_99)
  add_test(NAME drive_operation_result_coroutine_tests
    COMMAND drive_operation_result_coroutine_tests)
  add_executable(scsi_command_processor_coroutine_tests
    tests/scsi_command_processor_coroutine_tests.c
    src/scsi/command_processor_coroutine.c)
  target_compile_definitions(scsi_command_processor_coroutine_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(scsi_command_processor_coroutine_tests PRIVATE src)
  target_compile_features(scsi_command_processor_coroutine_tests PRIVATE c_std_99)
  add_test(NAME scsi_command_processor_coroutine_tests
    COMMAND scsi_command_processor_coroutine_tests)
  add_executable(host_interface_interrupt_tests
    tests/host_interface_interrupt_tests.c
    src/platform/host_interface_interrupt.c)
  target_compile_definitions(host_interface_interrupt_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(host_interface_interrupt_tests PRIVATE src)
  target_compile_features(host_interface_interrupt_tests PRIVATE c_std_99)
  add_test(NAME host_interface_interrupt_tests
    COMMAND host_interface_interrupt_tests)
  add_executable(vector_entries_tests
    tests/vector_entries_tests.c
    src/boot/reset_vector.c
    src/boot/controller_event_interrupt_entry.c
    src/boot/host_interface_interrupt_entry.c
    src/boot/controller_packet_interrupt_entry.c
    src/boot/timer0_interrupt_entry.c
    src/boot/sfr_e1_bit7_interrupt_entry.c)
  target_compile_definitions(vector_entries_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(vector_entries_tests PRIVATE src)
  target_compile_features(vector_entries_tests PRIVATE c_std_99)
  add_test(NAME vector_entries_tests COMMAND vector_entries_tests)
  add_executable(coroutine_context_switch_tests
    tests/coroutine_context_switch_tests.c
    src/platform/coroutine_context_switch.c)
  target_compile_definitions(coroutine_context_switch_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(coroutine_context_switch_tests PRIVATE src)
  target_compile_features(coroutine_context_switch_tests PRIVATE c_std_99)
  add_test(NAME coroutine_context_switch_tests
    COMMAND coroutine_context_switch_tests)
  add_executable(compiler_stack_transforms_tests
    tests/compiler_stack_transforms_tests.c
    src/platform/compiler_stack_transforms.c)
  target_compile_definitions(compiler_stack_transforms_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(compiler_stack_transforms_tests PRIVATE src)
  target_compile_features(compiler_stack_transforms_tests PRIVATE c_std_99)
  add_test(NAME compiler_stack_transforms_tests
    COMMAND compiler_stack_transforms_tests)
  add_executable(controller_operating_code_interpolation_tables_tests
    tests/controller_operating_code_interpolation_tables_tests.c
    src/platform/controller_operating_code_interpolation_tables.c)
  target_compile_definitions(
    controller_operating_code_interpolation_tables_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(
    controller_operating_code_interpolation_tables_tests PRIVATE src)
  target_compile_features(
    controller_operating_code_interpolation_tables_tests PRIVATE c_std_99)
  add_test(NAME controller_operating_code_interpolation_tables_tests
    COMMAND controller_operating_code_interpolation_tables_tests)
  add_executable(repeated_raw_read_range_windows_tests
    tests/repeated_raw_read_range_windows_tests.c
    src/media/repeated_raw_read_range_windows.c)
  target_compile_definitions(repeated_raw_read_range_windows_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(repeated_raw_read_range_windows_tests PRIVATE src)
  target_compile_features(repeated_raw_read_range_windows_tests PRIVATE c_std_99)
  add_test(NAME repeated_raw_read_range_windows_tests
    COMMAND repeated_raw_read_range_windows_tests)
  add_executable(raw_read_transfer_layout_tests
    tests/raw_read_transfer_layout_tests.c
    src/media/raw_read_transfer_layout.c)
  target_compile_definitions(raw_read_transfer_layout_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_transfer_layout_tests PRIVATE src)
  target_compile_features(raw_read_transfer_layout_tests PRIVATE c_std_99)
  add_test(NAME raw_read_transfer_layout_tests
    COMMAND raw_read_transfer_layout_tests)
  add_executable(raw_read_track_record_advance_tests
    tests/raw_read_track_record_advance_tests.c
    src/media/raw_read_track_record_advance.c)
  target_compile_definitions(raw_read_track_record_advance_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_track_record_advance_tests PRIVATE src)
  target_compile_features(raw_read_track_record_advance_tests PRIVATE c_std_99)
  add_test(NAME raw_read_track_record_advance_tests
    COMMAND raw_read_track_record_advance_tests)
  add_executable(working_track_raw_read_range_tests
    tests/working_track_raw_read_range_tests.c
    src/media/working_track_raw_read_range.c)
  target_compile_definitions(working_track_raw_read_range_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(working_track_raw_read_range_tests PRIVATE src)
  target_compile_features(working_track_raw_read_range_tests PRIVATE c_std_99)
  add_test(NAME working_track_raw_read_range_tests
    COMMAND working_track_raw_read_range_tests)
  add_executable(raw_read_frame_prefix_wait_tests
    tests/raw_read_frame_prefix_wait_tests.c
    src/media/raw_read_frame_prefix_wait.c)
  target_compile_definitions(raw_read_frame_prefix_wait_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_frame_prefix_wait_tests PRIVATE src)
  target_compile_features(raw_read_frame_prefix_wait_tests PRIVATE c_std_99)
  add_test(NAME raw_read_frame_prefix_wait_tests
    COMMAND raw_read_frame_prefix_wait_tests)
  add_executable(scaled_radial_pickup_move_sequence_tests
    tests/scaled_radial_pickup_move_sequence_tests.c
    src/platform/scaled_radial_pickup_move_sequence.c)
  target_compile_definitions(scaled_radial_pickup_move_sequence_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(scaled_radial_pickup_move_sequence_tests PRIVATE src)
  target_compile_features(scaled_radial_pickup_move_sequence_tests PRIVATE c_std_99)
  add_test(NAME scaled_radial_pickup_move_sequence_tests
    COMMAND scaled_radial_pickup_move_sequence_tests)
  add_executable(read_acquisition_state_tests
    tests/read_acquisition_state_tests.c
    src/media/read_acquisition_state.c)
  target_compile_definitions(read_acquisition_state_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_acquisition_state_tests PRIVATE src)
  target_compile_features(read_acquisition_state_tests PRIVATE c_std_99)
  add_test(NAME read_acquisition_state_tests
    COMMAND read_acquisition_state_tests)
  add_executable(natural_log_float32_tests
    tests/natural_log_float32_tests.c
    src/platform/arithmetic.c)
  target_compile_definitions(natural_log_float32_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(natural_log_float32_tests PRIVATE src)
  target_compile_features(natural_log_float32_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(natural_log_float32_tests PRIVATE m)
  endif()
  add_test(NAME natural_log_float32_tests
    COMMAND natural_log_float32_tests)
  add_executable(floating_floor_tests
    tests/floating_floor_tests.c
    src/platform/floating_floor.c)
  target_compile_definitions(floating_floor_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(floating_floor_tests PRIVATE src)
  target_compile_features(floating_floor_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(floating_floor_tests PRIVATE m)
  endif()
  add_test(NAME floating_floor_tests COMMAND floating_floor_tests)
  add_executable(float64_runtime_helpers_tests
    tests/float64_runtime_helpers_tests.c
    src/platform/float64_runtime_helpers.c)
  target_compile_definitions(float64_runtime_helpers_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(float64_runtime_helpers_tests PRIVATE src)
  target_compile_features(float64_runtime_helpers_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(float64_runtime_helpers_tests PRIVATE m)
  endif()
  add_test(NAME float64_runtime_helpers_tests
    COMMAND float64_runtime_helpers_tests)
  add_executable(float32_runtime_arithmetic_tests
    tests/float32_runtime_arithmetic_tests.c
    src/platform/float32_runtime_arithmetic.c)
  target_compile_definitions(float32_runtime_arithmetic_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(float32_runtime_arithmetic_tests PRIVATE src)
  target_compile_features(float32_runtime_arithmetic_tests PRIVATE c_std_99)
  add_test(NAME float32_runtime_arithmetic_tests
    COMMAND float32_runtime_arithmetic_tests)
  add_executable(floating_runtime_internals_tests
    tests/floating_runtime_internals_tests.c
    src/platform/floating_runtime_internals.c)
  target_compile_definitions(floating_runtime_internals_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(floating_runtime_internals_tests PRIVATE src)
  target_compile_features(floating_runtime_internals_tests PRIVATE c_std_99)
  add_test(NAME floating_runtime_internals_tests
    COMMAND floating_runtime_internals_tests)
  add_executable(floating_width_conversions_tests
    tests/floating_width_conversions_tests.c
    src/platform/floating_width_conversions.c)
  target_compile_definitions(floating_width_conversions_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(floating_width_conversions_tests PRIVATE src)
  target_compile_features(floating_width_conversions_tests PRIVATE c_std_99)
  add_test(NAME floating_width_conversions_tests
    COMMAND floating_width_conversions_tests)
  add_executable(integer_float32_conversions_tests
    tests/integer_float32_conversions_tests.c
    src/platform/integer_float32_conversions.c)
  target_compile_definitions(integer_float32_conversions_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(integer_float32_conversions_tests PRIVATE src)
  target_compile_features(integer_float32_conversions_tests PRIVATE c_std_99)
  add_test(NAME integer_float32_conversions_tests
    COMMAND integer_float32_conversions_tests)
  add_executable(close_sync_raw_read_prepare_tests
    tests/close_sync_raw_read_prepare_tests.c
    src/media/close_sync_raw_read_prepare.c)
  target_compile_definitions(close_sync_raw_read_prepare_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(close_sync_raw_read_prepare_tests PRIVATE src)
  target_compile_features(close_sync_raw_read_prepare_tests PRIVATE c_std_99)
  add_test(NAME close_sync_raw_read_prepare_tests
    COMMAND close_sync_raw_read_prepare_tests)
  add_executable(raw_sector_descriptor_scratch_tests
    tests/raw_sector_descriptor_scratch_tests.c
    src/platform/raw_sector_descriptor_scratch.c)
  target_compile_definitions(raw_sector_descriptor_scratch_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_sector_descriptor_scratch_tests PRIVATE src)
  target_compile_features(raw_sector_descriptor_scratch_tests PRIVATE c_std_99)
  add_test(NAME raw_sector_descriptor_scratch_tests
    COMMAND raw_sector_descriptor_scratch_tests)
  add_executable(format_unit_type2_pipeline_prepare_tests
    tests/format_unit_type2_pipeline_prepare_tests.c
    src/platform/format_unit_type2_pipeline.c)
  target_compile_definitions(format_unit_type2_pipeline_prepare_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(format_unit_type2_pipeline_prepare_tests PRIVATE src)
  target_compile_features(format_unit_type2_pipeline_prepare_tests PRIVATE c_std_99)
  add_test(NAME format_unit_type2_pipeline_prepare_tests
    COMMAND format_unit_type2_pipeline_prepare_tests)
  add_executable(read_acquisition_request_dispatch_tests
    tests/read_acquisition_request_dispatch_tests.c
    src/platform/read_acquisition_request_dispatch.c)
  target_compile_definitions(read_acquisition_request_dispatch_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_acquisition_request_dispatch_tests PRIVATE src)
  target_compile_features(read_acquisition_request_dispatch_tests PRIVATE c_std_99)
  add_test(NAME read_acquisition_request_dispatch_tests
    COMMAND read_acquisition_request_dispatch_tests)
  add_executable(raw_read_range_five_frame_preroll_tests
    tests/raw_read_range_five_frame_preroll_tests.c
    src/media/raw_read_range_five_frame_preroll.c)
  target_compile_definitions(raw_read_range_five_frame_preroll_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_range_five_frame_preroll_tests PRIVATE src)
  target_compile_features(raw_read_range_five_frame_preroll_tests PRIVATE c_std_99)
  add_test(NAME raw_read_range_five_frame_preroll_tests
    COMMAND raw_read_range_five_frame_preroll_tests)
  add_executable(raw_read_q_record_queue_tests
    tests/raw_read_q_record_queue_tests.c
    src/media/raw_read_q_record_queue.c)
  target_compile_definitions(raw_read_q_record_queue_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_q_record_queue_tests PRIVATE src)
  target_compile_features(raw_read_q_record_queue_tests PRIVATE c_std_99)
  add_test(NAME raw_read_q_record_queue_tests
    COMMAND raw_read_q_record_queue_tests)
  add_executable(format_operation_raw_read_state_tests
    tests/format_operation_raw_read_state_tests.c
    src/media/format_operation_raw_read_state.c)
  target_compile_definitions(format_operation_raw_read_state_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(format_operation_raw_read_state_tests PRIVATE src)
  target_compile_features(format_operation_raw_read_state_tests PRIVATE c_std_99)
  add_test(NAME format_operation_raw_read_state_tests
    COMMAND format_operation_raw_read_state_tests)
  add_executable(reserve_track_raw_read_pipeline_tests
    tests/reserve_track_raw_read_pipeline_tests.c
    src/media/reserve_track_raw_read_pipeline.c)
  target_compile_definitions(reserve_track_raw_read_pipeline_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(reserve_track_raw_read_pipeline_tests PRIVATE src)
  target_compile_features(reserve_track_raw_read_pipeline_tests PRIVATE c_std_99)
  add_test(NAME reserve_track_raw_read_pipeline_tests
    COMMAND reserve_track_raw_read_pipeline_tests)
  add_executable(current_mode_raw_read_pipeline_tests
    tests/current_mode_raw_read_pipeline_tests.c
    src/media/current_mode_raw_read_pipeline.c)
  target_compile_definitions(current_mode_raw_read_pipeline_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(current_mode_raw_read_pipeline_tests PRIVATE src)
  target_compile_features(current_mode_raw_read_pipeline_tests PRIVATE c_std_99)
  add_test(NAME current_mode_raw_read_pipeline_tests
    COMMAND current_mode_raw_read_pipeline_tests)
  add_executable(raw_read_transfer_batch_service_tests
    tests/raw_read_transfer_batch_service_tests.c
    src/media/raw_read_transfer_batch_service.c)
  target_compile_definitions(raw_read_transfer_batch_service_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_transfer_batch_service_tests PRIVATE src)
  target_compile_features(raw_read_transfer_batch_service_tests PRIVATE c_std_99)
  add_test(NAME raw_read_transfer_batch_service_tests
    COMMAND raw_read_transfer_batch_service_tests)
  add_executable(raw_read_toc_summary_tests
    tests/raw_read_toc_summary_tests.c
    src/media/raw_read_toc_summary.c)
  target_compile_definitions(raw_read_toc_summary_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_toc_summary_tests PRIVATE src)
  target_compile_features(raw_read_toc_summary_tests PRIVATE c_std_99)
  add_test(NAME raw_read_toc_summary_tests COMMAND raw_read_toc_summary_tests)
  add_executable(raw_read_interleaved_q_batch_tests
    tests/raw_read_interleaved_q_batch_tests.c
    src/media/raw_read_interleaved_q_batch.c)
  target_compile_definitions(raw_read_interleaved_q_batch_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_interleaved_q_batch_tests PRIVATE src)
  target_compile_features(raw_read_interleaved_q_batch_tests PRIVATE c_std_99)
  add_test(NAME raw_read_interleaved_q_batch_tests
    COMMAND raw_read_interleaved_q_batch_tests)
  add_executable(raw_read_q_record_sector_acquisition_tests
    tests/raw_read_q_record_sector_acquisition_tests.c
    src/media/raw_read_q_record_sector_acquisition.c)
  target_compile_definitions(raw_read_q_record_sector_acquisition_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_q_record_sector_acquisition_tests PRIVATE
    src)
  target_compile_features(raw_read_q_record_sector_acquisition_tests PRIVATE
    c_std_99)
  add_test(NAME raw_read_q_record_sector_acquisition_tests
    COMMAND raw_read_q_record_sector_acquisition_tests)
  add_executable(raw_sector_sequence_tests
    tests/raw_sector_sequence_tests.c
    src/media/raw_sector_sequence.c)
  target_compile_definitions(raw_sector_sequence_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_sector_sequence_tests PRIVATE src)
  target_compile_features(raw_sector_sequence_tests PRIVATE c_std_99)
  add_test(NAME raw_sector_sequence_tests COMMAND raw_sector_sequence_tests)
  add_executable(lead_in_out_descriptor_sequences_tests
    tests/lead_in_out_descriptor_sequences_tests.c
    src/media/lead_in_out_descriptor_sequences.c)
  target_compile_definitions(lead_in_out_descriptor_sequences_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(lead_in_out_descriptor_sequences_tests PRIVATE src)
  target_compile_features(lead_in_out_descriptor_sequences_tests PRIVATE c_std_99)
  add_test(NAME lead_in_out_descriptor_sequences_tests
    COMMAND lead_in_out_descriptor_sequences_tests)
  add_executable(lead_in_raw_sector_descriptor_sequence_tests
    tests/lead_in_raw_sector_descriptor_sequence_tests.c
    src/media/lead_in_raw_sector_descriptor_sequence.c)
  target_compile_definitions(lead_in_raw_sector_descriptor_sequence_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(lead_in_raw_sector_descriptor_sequence_tests
    PRIVATE src)
  target_compile_features(lead_in_raw_sector_descriptor_sequence_tests
    PRIVATE c_std_99)
  add_test(NAME lead_in_raw_sector_descriptor_sequence_tests
    COMMAND lead_in_raw_sector_descriptor_sequence_tests)
  add_executable(lead_out_raw_sector_descriptor_sequence_tests
    tests/lead_out_raw_sector_descriptor_sequence_tests.c
    src/media/lead_out_raw_sector_descriptor_sequence.c)
  target_compile_definitions(lead_out_raw_sector_descriptor_sequence_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(lead_out_raw_sector_descriptor_sequence_tests
    PRIVATE src)
  target_compile_features(lead_out_raw_sector_descriptor_sequence_tests
    PRIVATE c_std_99)
  add_test(NAME lead_out_raw_sector_descriptor_sequence_tests
    COMMAND lead_out_raw_sector_descriptor_sequence_tests)
  add_executable(controller_record_workspace_page_tests
    tests/controller_record_workspace_page_tests.c
    src/platform/controller_record_workspace_page.c)
  target_compile_definitions(controller_record_workspace_page_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_workspace_page_tests PRIVATE src)
  target_compile_features(controller_record_workspace_page_tests PRIVATE c_std_99)
  add_test(NAME controller_record_workspace_page_tests
    COMMAND controller_record_workspace_page_tests)
  add_executable(mip_controller_record_snapshot_tests
    tests/mip_controller_record_snapshot_tests.c
    src/platform/mip_controller_record_snapshot.c)
  target_compile_definitions(mip_controller_record_snapshot_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(mip_controller_record_snapshot_tests PRIVATE src)
  target_compile_features(mip_controller_record_snapshot_tests PRIVATE c_std_99)
  add_test(NAME mip_controller_record_snapshot_tests
    COMMAND mip_controller_record_snapshot_tests)
  add_executable(type2_leadin_toc_sequence_tests
    tests/type2_leadin_toc_sequence_tests.c
    src/platform/type2_leadin_toc_sequence.c)
  target_compile_definitions(type2_leadin_toc_sequence_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(type2_leadin_toc_sequence_tests PRIVATE src)
  target_compile_features(type2_leadin_toc_sequence_tests PRIVATE c_std_99)
  add_test(NAME type2_leadin_toc_sequence_tests
    COMMAND type2_leadin_toc_sequence_tests)
  add_executable(type2_controller_record_geometry_tests
    tests/type2_controller_record_geometry_tests.c
    src/platform/type2_controller_record_geometry.c)
  target_compile_definitions(type2_controller_record_geometry_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(type2_controller_record_geometry_tests PRIVATE src)
  target_compile_features(type2_controller_record_geometry_tests PRIVATE c_std_99)
  add_test(NAME type2_controller_record_geometry_tests
    COMMAND type2_controller_record_geometry_tests)
  add_executable(controller_record_page_reconciliation_tests
    tests/controller_record_page_reconciliation_tests.c
    src/platform/controller_record_page_reconciliation.c)
  target_compile_definitions(controller_record_page_reconciliation_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_page_reconciliation_tests PRIVATE src)
  target_compile_features(controller_record_page_reconciliation_tests PRIVATE c_std_99)
  add_test(NAME controller_record_page_reconciliation_tests
    COMMAND controller_record_page_reconciliation_tests)
  add_executable(controller_record_page_header_tests
    tests/controller_record_page_header_tests.c
    src/platform/controller_record_page_header.c)
  target_compile_definitions(controller_record_page_header_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_page_header_tests PRIVATE src)
  target_compile_features(controller_record_page_header_tests PRIVATE c_std_99)
  add_test(NAME controller_record_page_header_tests
    COMMAND controller_record_page_header_tests)
  add_executable(operating_code_transition_finalization_tests
    tests/operating_code_transition_finalization_tests.c
    src/platform/operating_code_transition_finalization.c)
  target_compile_definitions(operating_code_transition_finalization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(operating_code_transition_finalization_tests PRIVATE src)
  target_compile_features(operating_code_transition_finalization_tests PRIVATE c_std_99)
  add_test(NAME operating_code_transition_finalization_tests
    COMMAND operating_code_transition_finalization_tests)
  add_executable(type2_detected_layout_prepare_tests
    tests/type2_detected_layout_prepare_tests.c
    src/platform/type2_detected_layout_prepare.c)
  target_compile_definitions(type2_detected_layout_prepare_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(type2_detected_layout_prepare_tests PRIVATE src)
  target_compile_features(type2_detected_layout_prepare_tests PRIVATE c_std_99)
  add_test(NAME type2_detected_layout_prepare_tests
    COMMAND type2_detected_layout_prepare_tests)
  add_executable(rid01_identity_block_tests
    tests/rid01_identity_block_tests.c
    src/media/rid01_identity_block.c)
  target_compile_definitions(rid01_identity_block_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(rid01_identity_block_tests PRIVATE src)
  target_compile_features(rid01_identity_block_tests PRIVATE c_std_99)
  add_test(NAME rid01_identity_block_tests
    COMMAND rid01_identity_block_tests)
  add_executable(track_packet_lba_mapping_tests
    tests/track_packet_lba_mapping_tests.c
    src/scsi/track_packet_lba_mapping.c)
  target_compile_definitions(track_packet_lba_mapping_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(track_packet_lba_mapping_tests PRIVATE src)
  target_compile_features(track_packet_lba_mapping_tests PRIVATE c_std_99)
  add_test(NAME track_packet_lba_mapping_tests
    COMMAND track_packet_lba_mapping_tests)
  add_executable(synthetic_raw_read_pattern_tests
    tests/synthetic_raw_read_pattern_tests.c
    src/media/synthetic_raw_read_pattern.c)
  target_compile_definitions(synthetic_raw_read_pattern_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(synthetic_raw_read_pattern_tests PRIVATE src)
  target_compile_features(synthetic_raw_read_pattern_tests PRIVATE c_std_99)
  add_test(NAME synthetic_raw_read_pattern_tests
    COMMAND synthetic_raw_read_pattern_tests)
  add_executable(media_state6_calibration_threshold_tests
    tests/media_state6_calibration_threshold_tests.c
    src/platform/media_state6_calibration_threshold.c)
  target_compile_definitions(media_state6_calibration_threshold_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(media_state6_calibration_threshold_tests PRIVATE src)
  target_compile_features(media_state6_calibration_threshold_tests PRIVATE c_std_99)
  add_test(NAME media_state6_calibration_threshold_tests
    COMMAND media_state6_calibration_threshold_tests)
  add_executable(controller_position_sample_acquisition_tests
    tests/controller_position_sample_acquisition_tests.c
    src/platform/controller_position_sample_acquisition.c)
  target_compile_definitions(controller_position_sample_acquisition_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_position_sample_acquisition_tests PRIVATE src)
  target_compile_features(controller_position_sample_acquisition_tests PRIVATE c_std_99)
  add_test(NAME controller_position_sample_acquisition_tests
    COMMAND controller_position_sample_acquisition_tests)
  add_executable(controller_position_sample_evaluation_tests
    tests/controller_position_sample_evaluation_tests.c
    src/platform/controller_position_sample_evaluation.c)
  target_compile_definitions(controller_position_sample_evaluation_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_position_sample_evaluation_tests PRIVATE src)
  target_compile_features(controller_position_sample_evaluation_tests PRIVATE c_std_99)
  add_test(NAME controller_position_sample_evaluation_tests
    COMMAND controller_position_sample_evaluation_tests)
  add_executable(controller_state6_position_window_tests
    tests/controller_state6_position_window_tests.c
    src/platform/controller_state6_position_window.c)
  target_compile_definitions(controller_state6_position_window_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_state6_position_window_tests PRIVATE src)
  target_compile_features(controller_state6_position_window_tests PRIVATE c_std_99)
  add_test(NAME controller_state6_position_window_tests
    COMMAND controller_state6_position_window_tests)
  add_executable(controller_state6_calibration_positioning_tests
    tests/controller_state6_calibration_positioning_tests.c
    src/platform/controller_state6_calibration_positioning.c)
  target_compile_definitions(controller_state6_calibration_positioning_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_state6_calibration_positioning_tests PRIVATE src)
  target_compile_features(controller_state6_calibration_positioning_tests PRIVATE c_std_99)
  add_test(NAME controller_state6_calibration_positioning_tests
    COMMAND controller_state6_calibration_positioning_tests)
  add_executable(media_state6_controller_calibration_sequence_tests
    tests/media_state6_controller_calibration_sequence_tests.c
    src/platform/media_state6_controller_calibration_sequence.c)
  target_compile_definitions(media_state6_controller_calibration_sequence_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(media_state6_controller_calibration_sequence_tests PRIVATE src)
  target_compile_features(media_state6_controller_calibration_sequence_tests PRIVATE c_std_99)
  add_test(NAME media_state6_controller_calibration_sequence_tests
    COMMAND media_state6_controller_calibration_sequence_tests)
  add_executable(media_coroutine_state6_tests
    tests/media_coroutine_state6_tests.c
    src/platform/media_coroutine_state6.c)
  target_compile_definitions(media_coroutine_state6_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(media_coroutine_state6_tests PRIVATE src)
  target_compile_features(media_coroutine_state6_tests PRIVATE c_std_99)
  add_test(NAME media_coroutine_state6_tests COMMAND media_coroutine_state6_tests)
  add_executable(drive_state_machine_tests
    tests/drive_state_machine_tests.c
    src/startup/drive_state_machine.c)
  target_compile_definitions(drive_state_machine_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(drive_state_machine_tests PRIVATE src)
  target_compile_features(drive_state_machine_tests PRIVATE c_std_99)
  add_test(NAME drive_state_machine_tests COMMAND drive_state_machine_tests)
  add_executable(indexed_drive_state_transitions_tests
    tests/indexed_drive_state_transitions_tests.c
    src/startup/indexed_drive_state_transitions.c)
  target_compile_definitions(indexed_drive_state_transitions_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(indexed_drive_state_transitions_tests PRIVATE src)
  target_compile_features(indexed_drive_state_transitions_tests PRIVATE c_std_99)
  add_test(NAME indexed_drive_state_transitions_tests
    COMMAND indexed_drive_state_transitions_tests)
  add_executable(startup_status_coroutine1_tests
    tests/startup_status_coroutine1_tests.c
    src/startup/startup_status_coroutine1.c)
  target_compile_definitions(startup_status_coroutine1_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(startup_status_coroutine1_tests PRIVATE src)
  target_compile_features(startup_status_coroutine1_tests PRIVATE c_std_99)
  add_test(NAME startup_status_coroutine1_tests
    COMMAND startup_status_coroutine1_tests)
  add_executable(startup_media_state_initialization_tests
    tests/startup_media_state_initialization_tests.c
    src/startup/startup_media_state_initialization.c)
  target_compile_definitions(startup_media_state_initialization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(startup_media_state_initialization_tests PRIVATE src)
  target_compile_features(startup_media_state_initialization_tests PRIVATE c_std_99)
  add_test(NAME startup_media_state_initialization_tests
    COMMAND startup_media_state_initialization_tests)
  add_executable(indexed_read_state_configuration_tests
    tests/indexed_read_state_configuration_tests.c
    src/startup/indexed_read_state_configuration.c)
  target_compile_definitions(indexed_read_state_configuration_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(indexed_read_state_configuration_tests PRIVATE src)
  target_compile_features(indexed_read_state_configuration_tests PRIVATE c_std_99)
  add_test(NAME indexed_read_state_configuration_tests
    COMMAND indexed_read_state_configuration_tests)
  add_executable(controller_calibration_configuration_tests
    tests/controller_calibration_configuration_tests.c
    src/startup/controller_calibration_configuration.c)
  target_compile_definitions(controller_calibration_configuration_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_calibration_configuration_tests PRIVATE src)
  target_compile_features(controller_calibration_configuration_tests PRIVATE c_std_99)
  add_test(NAME controller_calibration_configuration_tests
    COMMAND controller_calibration_configuration_tests)
  add_executable(media_coroutine_state1_tests
    tests/media_coroutine_state1_tests.c
    src/startup/media_coroutine_state1.c)
  target_compile_definitions(media_coroutine_state1_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(media_coroutine_state1_tests PRIVATE src)
  target_compile_features(media_coroutine_state1_tests PRIVATE c_std_99)
  add_test(NAME media_coroutine_state1_tests
    COMMAND media_coroutine_state1_tests)
  add_executable(state5_position_window_tests
    tests/state5_position_window_tests.c
    src/media/state5_position_window.c)
  target_compile_definitions(state5_position_window_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(state5_position_window_tests PRIVATE src)
  target_compile_features(state5_position_window_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(state5_position_window_tests PRIVATE m)
  endif()
  add_test(NAME state5_position_window_tests
    COMMAND state5_position_window_tests)
  add_executable(state3_indexed_transitions_tests
    tests/state3_indexed_transitions_tests.c
    src/media/state3_indexed_transitions.c)
  target_compile_definitions(state3_indexed_transitions_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(state3_indexed_transitions_tests PRIVATE src)
  target_compile_features(state3_indexed_transitions_tests PRIVATE c_std_99)
  add_test(NAME state3_indexed_transitions_tests
    COMMAND state3_indexed_transitions_tests)
  add_executable(coroutine_state5_tests
    tests/coroutine_state5_tests.c
    src/media/coroutine_state5.c)
  target_compile_definitions(coroutine_state5_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(coroutine_state5_tests PRIVATE src)
  target_compile_features(coroutine_state5_tests PRIVATE c_std_99)
  add_test(NAME coroutine_state5_tests COMMAND coroutine_state5_tests)
  add_executable(state5_read_acquisition_sequence_tests
    tests/state5_read_acquisition_sequence_tests.c
    src/media/state5_read_acquisition_sequence.c)
  target_compile_definitions(state5_read_acquisition_sequence_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(state5_read_acquisition_sequence_tests PRIVATE src)
  target_compile_features(state5_read_acquisition_sequence_tests PRIVATE c_std_99)
  add_test(NAME state5_read_acquisition_sequence_tests
    COMMAND state5_read_acquisition_sequence_tests)
  add_executable(high_write_speed_calibration_flags_tests
    tests/high_write_speed_calibration_flags_tests.c
    src/platform/high_write_speed_calibration_flags.c)
  target_compile_definitions(high_write_speed_calibration_flags_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(high_write_speed_calibration_flags_tests PRIVATE src)
  target_compile_features(high_write_speed_calibration_flags_tests PRIVATE c_std_99)
  add_test(NAME high_write_speed_calibration_flags_tests
    COMMAND high_write_speed_calibration_flags_tests)
  add_executable(controller_record_page_update_tests
    tests/controller_record_page_update_tests.c
    src/platform/controller_record_page_update.c)
  target_compile_definitions(controller_record_page_update_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_page_update_tests PRIVATE src)
  target_compile_features(controller_record_page_update_tests PRIVATE c_std_99)
  add_test(NAME controller_record_page_update_tests
    COMMAND controller_record_page_update_tests)
  add_executable(format_unit_code11_probe_tests
    tests/format_unit_code11_probe_tests.c
    src/scsi/format_unit_code11_probe.c)
  target_compile_definitions(format_unit_code11_probe_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(format_unit_code11_probe_tests PRIVATE src)
  target_compile_features(format_unit_code11_probe_tests PRIVATE c_std_99)
  add_test(NAME format_unit_code11_probe_tests
    COMMAND format_unit_code11_probe_tests)
  add_executable(format_unit_code11_scan_tests
    tests/format_unit_code11_scan_tests.c
    src/scsi/format_unit_code11_scan.c)
  target_compile_definitions(format_unit_code11_scan_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(format_unit_code11_scan_tests PRIVATE src)
  target_compile_features(format_unit_code11_scan_tests PRIVATE c_std_99)
  add_test(NAME format_unit_code11_scan_tests
    COMMAND format_unit_code11_scan_tests)
  add_executable(format_unit_code11_start_tests
    tests/format_unit_code11_start_tests.c
    src/scsi/format_unit_code11_start.c)
  target_compile_definitions(format_unit_code11_start_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(format_unit_code11_start_tests PRIVATE src)
  target_compile_features(format_unit_code11_start_tests PRIVATE c_std_99)
  add_test(NAME format_unit_code11_start_tests
    COMMAND format_unit_code11_start_tests)
  add_executable(controller_record_type3_refresh_tests
    tests/controller_record_type3_refresh_tests.c
    src/platform/controller_record_type3_refresh.c)
  target_compile_definitions(controller_record_type3_refresh_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_type3_refresh_tests PRIVATE src)
  target_compile_features(controller_record_type3_refresh_tests PRIVATE c_std_99)
  add_test(NAME controller_record_type3_refresh_tests
    COMMAND controller_record_type3_refresh_tests)
  add_executable(controller_record_index_range_acquisition_tests
    tests/controller_record_index_range_acquisition_tests.c
    src/platform/controller_record_index_range_acquisition.c)
  target_compile_definitions(controller_record_index_range_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_index_range_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_index_range_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_index_range_acquisition_tests
    COMMAND controller_record_index_range_acquisition_tests)
  add_executable(controller_record_leadout_acquisition_tests
    tests/controller_record_leadout_acquisition_tests.c
    src/platform/controller_record_leadout_acquisition.c)
  target_compile_definitions(controller_record_leadout_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_leadout_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_leadout_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_leadout_acquisition_tests
    COMMAND controller_record_leadout_acquisition_tests)
  add_executable(controller_record_tdi_acquisition_tests
    tests/controller_record_tdi_acquisition_tests.c
    src/platform/controller_record_tdi_acquisition.c)
  target_compile_definitions(controller_record_tdi_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_tdi_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_tdi_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_tdi_acquisition_tests
    COMMAND controller_record_tdi_acquisition_tests)
  add_executable(controller_record_leadin_acquisition_tests
    tests/controller_record_leadin_acquisition_tests.c
    src/platform/controller_record_leadin_acquisition.c)
  target_compile_definitions(controller_record_leadin_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_leadin_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_leadin_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_leadin_acquisition_tests
    COMMAND controller_record_leadin_acquisition_tests)
  add_executable(controller_record_required_runs_tests
    tests/controller_record_required_runs_tests.c
    src/platform/controller_record_required_runs.c)
  target_compile_definitions(controller_record_required_runs_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_required_runs_tests
    PRIVATE src)
  target_compile_features(controller_record_required_runs_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_required_runs_tests
    COMMAND controller_record_required_runs_tests)
  add_executable(controller_record_secondary_slot_acquisition_tests
    tests/controller_record_secondary_slot_acquisition_tests.c
    src/platform/controller_record_secondary_slot_acquisition.c)
  target_compile_definitions(controller_record_secondary_slot_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_secondary_slot_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_secondary_slot_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_secondary_slot_acquisition_tests
    COMMAND controller_record_secondary_slot_acquisition_tests)
  add_executable(controller_record_primary_slot_acquisition_tests
    tests/controller_record_primary_slot_acquisition_tests.c
    src/platform/controller_record_primary_slot_acquisition.c)
  target_compile_definitions(controller_record_primary_slot_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_primary_slot_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_primary_slot_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_primary_slot_acquisition_tests
    COMMAND controller_record_primary_slot_acquisition_tests)
  add_executable(controller_record_window_acquisition_tests
    tests/controller_record_window_acquisition_tests.c
    src/platform/controller_record_window_acquisition.c)
  target_compile_definitions(controller_record_window_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_window_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_window_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_window_acquisition_tests
    COMMAND controller_record_window_acquisition_tests)
  add_executable(controller_record_base_window_acquisition_tests
    tests/controller_record_base_window_acquisition_tests.c
    src/platform/controller_record_base_window_acquisition.c)
  target_compile_definitions(controller_record_base_window_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_base_window_acquisition_tests
    PRIVATE src)
  target_compile_features(controller_record_base_window_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME controller_record_base_window_acquisition_tests
    COMMAND controller_record_base_window_acquisition_tests)
  add_executable(floating_split_tests
    tests/floating_split_tests.c
    src/platform/floating_split.c)
  target_compile_definitions(floating_split_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(floating_split_tests PRIVATE src)
  target_compile_features(floating_split_tests PRIVATE c_std_99)
  add_test(NAME floating_split_tests COMMAND floating_split_tests)
  add_executable(atip_media_strategy_tests
    tests/atip_media_strategy_tests.c
    src/media/atip_media_strategy.c)
  target_compile_definitions(atip_media_strategy_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(atip_media_strategy_tests PRIVATE src)
  target_compile_features(atip_media_strategy_tests PRIVATE c_std_99)
  add_test(NAME atip_media_strategy_tests COMMAND atip_media_strategy_tests)
  add_executable(controller_register_profile_application_tests
    tests/controller_register_profile_application_tests.c
    src/platform/controller_register_profile_application.c)
  target_compile_definitions(controller_register_profile_application_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_register_profile_application_tests
    PRIVATE src)
  target_compile_features(controller_register_profile_application_tests
    PRIVATE c_std_99)
  add_test(NAME controller_register_profile_application_tests
    COMMAND controller_register_profile_application_tests)
  add_executable(controller_profile_sample_interpolation_tests
    tests/controller_profile_sample_interpolation_tests.c
    src/platform/controller_profile_sample_interpolation.c)
  target_compile_definitions(controller_profile_sample_interpolation_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_profile_sample_interpolation_tests
    PRIVATE src)
  target_compile_features(controller_profile_sample_interpolation_tests
    PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(controller_profile_sample_interpolation_tests
      PRIVATE m)
  endif()
  add_test(NAME controller_profile_sample_interpolation_tests
    COMMAND controller_profile_sample_interpolation_tests)
  add_executable(controller_profile_sample_generation_tests
    tests/controller_profile_sample_generation_tests.c
    src/platform/controller_profile_sample_generation.c)
  target_compile_definitions(controller_profile_sample_generation_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_profile_sample_generation_tests
    PRIVATE src)
  target_compile_features(controller_profile_sample_generation_tests
    PRIVATE c_std_99)
  add_test(NAME controller_profile_sample_generation_tests
    COMMAND controller_profile_sample_generation_tests)
  add_executable(radial_model_profile_coordinate_tests
    tests/radial_model_profile_coordinate_tests.c
    src/platform/radial_model_profile_coordinate.c)
  target_compile_definitions(radial_model_profile_coordinate_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(radial_model_profile_coordinate_tests PRIVATE src)
  target_compile_features(radial_model_profile_coordinate_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(radial_model_profile_coordinate_tests PRIVATE m)
  endif()
  add_test(NAME radial_model_profile_coordinate_tests
    COMMAND radial_model_profile_coordinate_tests)
  add_executable(controller_operating_interpolation_grid_column_tests
    tests/controller_operating_interpolation_grid_column_tests.c
    src/platform/controller_operating_interpolation_grid_column.c)
  target_compile_definitions(controller_operating_interpolation_grid_column_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_operating_interpolation_grid_column_tests
    PRIVATE src)
  target_compile_features(controller_operating_interpolation_grid_column_tests
    PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(controller_operating_interpolation_grid_column_tests
      PRIVATE m)
  endif()
  add_test(NAME controller_operating_interpolation_grid_column_tests
    COMMAND controller_operating_interpolation_grid_column_tests)
  add_executable(controller_operating_interpolation_table_slice_tests
    tests/controller_operating_interpolation_table_slice_tests.c
    src/platform/controller_operating_interpolation_table_slice.c)
  target_compile_definitions(controller_operating_interpolation_table_slice_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_operating_interpolation_table_slice_tests
    PRIVATE src)
  target_compile_features(controller_operating_interpolation_table_slice_tests
    PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(controller_operating_interpolation_table_slice_tests
      PRIVATE m)
  endif()
  add_test(NAME controller_operating_interpolation_table_slice_tests
    COMMAND controller_operating_interpolation_table_slice_tests)
  add_executable(controller_register_profile_generation_tests
    tests/controller_register_profile_generation_tests.c
    src/platform/controller_register_profile_generation.c)
  target_compile_definitions(controller_register_profile_generation_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_register_profile_generation_tests
    PRIVATE src)
  target_compile_features(controller_register_profile_generation_tests
    PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(controller_register_profile_generation_tests PRIVATE m)
  endif()
  add_test(NAME controller_register_profile_generation_tests
    COMMAND controller_register_profile_generation_tests)
  add_executable(controller_profile_selector_tests
    tests/controller_profile_selector_tests.c
    src/platform/controller_profile_selector.c)
  target_compile_definitions(controller_profile_selector_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_profile_selector_tests PRIVATE src)
  target_compile_features(controller_profile_selector_tests PRIVATE c_std_99)
  add_test(NAME controller_profile_selector_tests
    COMMAND controller_profile_selector_tests)
  add_executable(controller_operating_code_ports_tests
    tests/controller_operating_code_ports_tests.c
    src/platform/controller_operating_code_ports.c)
  target_compile_definitions(controller_operating_code_ports_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_operating_code_ports_tests PRIVATE src)
  target_compile_features(controller_operating_code_ports_tests PRIVATE c_std_99)
  add_test(NAME controller_operating_code_ports_tests
    COMMAND controller_operating_code_ports_tests)
  add_executable(controller_threshold_configuration_tests
    tests/controller_threshold_configuration_tests.c
    src/platform/controller_threshold_configuration.c)
  target_compile_definitions(controller_threshold_configuration_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_threshold_configuration_tests PRIVATE
    src)
  target_compile_features(controller_threshold_configuration_tests PRIVATE
    c_std_99)
  add_test(NAME controller_threshold_configuration_tests
    COMMAND controller_threshold_configuration_tests)
  add_executable(controller_word0319_offset_calibration_tests
    tests/controller_word0319_offset_calibration_tests.c
    src/platform/controller_word0319_offset_calibration.c)
  target_compile_definitions(controller_word0319_offset_calibration_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_word0319_offset_calibration_tests
    PRIVATE src)
  target_compile_features(controller_word0319_offset_calibration_tests
    PRIVATE c_std_99)
  add_test(NAME controller_word0319_offset_calibration_tests
    COMMAND controller_word0319_offset_calibration_tests)
  add_executable(controller_words0204_0319_calibration_tests
    tests/controller_words0204_0319_calibration_tests.c
    src/platform/controller_words0204_0319_calibration.c)
  target_compile_definitions(controller_words0204_0319_calibration_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_words0204_0319_calibration_tests
    PRIVATE src)
  target_compile_features(controller_words0204_0319_calibration_tests
    PRIVATE c_std_99)
  add_test(NAME controller_words0204_0319_calibration_tests
    COMMAND controller_words0204_0319_calibration_tests)
  add_executable(controller_fixed_profile_generation_tests
    tests/controller_fixed_profile_generation_tests.c
    src/platform/controller_fixed_profile_generation.c)
  target_compile_definitions(controller_fixed_profile_generation_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_fixed_profile_generation_tests
    PRIVATE src)
  target_compile_features(controller_fixed_profile_generation_tests
    PRIVATE c_std_99)
  add_test(NAME controller_fixed_profile_generation_tests
    COMMAND controller_fixed_profile_generation_tests)
  add_executable(controller_current_profile_configuration_tests
    tests/controller_current_profile_configuration_tests.c
    src/platform/controller_current_profile_configuration.c)
  target_compile_definitions(controller_current_profile_configuration_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_current_profile_configuration_tests
    PRIVATE src)
  target_compile_features(controller_current_profile_configuration_tests
    PRIVATE c_std_99)
  add_test(NAME controller_current_profile_configuration_tests
    COMMAND controller_current_profile_configuration_tests)
  add_executable(get_event_status_notification_tests
    tests/get_event_status_notification_tests.c
    src/scsi/get_event_status_notification.c)
  target_compile_definitions(get_event_status_notification_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(get_event_status_notification_tests PRIVATE src)
  target_compile_features(get_event_status_notification_tests PRIVATE c_std_99)
  add_test(NAME get_event_status_notification_tests
    COMMAND get_event_status_notification_tests)
  add_executable(get_configuration_features_tests
    tests/get_configuration_features_tests.c
    src/scsi/get_configuration_features.c)
  target_compile_definitions(get_configuration_features_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(get_configuration_features_tests PRIVATE src)
  target_compile_features(get_configuration_features_tests PRIVATE c_std_99)
  add_test(NAME get_configuration_features_tests
    COMMAND get_configuration_features_tests)
  add_executable(read_toc_format3_pma_tests
    tests/read_toc_format3_pma_tests.c
    src/scsi/read_toc_format3_pma.c)
  target_compile_definitions(read_toc_format3_pma_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_toc_format3_pma_tests PRIVATE src)
  target_compile_features(read_toc_format3_pma_tests PRIVATE c_std_99)
  add_test(NAME read_toc_format3_pma_tests COMMAND read_toc_format3_pma_tests)
  add_executable(read_full_toc_response_tests
    tests/read_full_toc_response_tests.c
    src/scsi/read_full_toc_response.c)
  target_compile_definitions(read_full_toc_response_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_full_toc_response_tests PRIVATE src)
  target_compile_features(read_full_toc_response_tests PRIVATE c_std_99)
  add_test(NAME read_full_toc_response_tests COMMAND read_full_toc_response_tests)
  add_executable(read_formatted_toc_response_tests
    tests/read_formatted_toc_response_tests.c
    src/scsi/read_formatted_toc_response.c)
  target_compile_definitions(read_formatted_toc_response_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_formatted_toc_response_tests PRIVATE src)
  target_compile_features(read_formatted_toc_response_tests PRIVATE c_std_99)
  add_test(NAME read_formatted_toc_response_tests
    COMMAND read_formatted_toc_response_tests)
  add_executable(read_subchannel_isrc_f1_tests
    tests/read_subchannel_isrc_f1_tests.c
    src/scsi/read_subchannel_isrc_f1.c)
  target_compile_definitions(read_subchannel_isrc_f1_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_subchannel_isrc_f1_tests PRIVATE src)
  target_compile_features(read_subchannel_isrc_f1_tests PRIVATE c_std_99)
  add_test(NAME read_subchannel_isrc_f1_tests
    COMMAND read_subchannel_isrc_f1_tests)
  add_executable(read_subchannel_identification_acquisition_tests
    tests/read_subchannel_identification_acquisition_tests.c
    src/scsi/read_subchannel_identification_acquisition.c)
  target_compile_definitions(read_subchannel_identification_acquisition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_subchannel_identification_acquisition_tests
    PRIVATE src)
  target_compile_features(read_subchannel_identification_acquisition_tests
    PRIVATE c_std_99)
  add_test(NAME read_subchannel_identification_acquisition_tests
    COMMAND read_subchannel_identification_acquisition_tests)
  add_executable(read_subchannel_current_position_tests
    tests/read_subchannel_current_position_tests.c
    src/scsi/read_subchannel_current_position.c)
  target_compile_definitions(read_subchannel_current_position_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_subchannel_current_position_tests PRIVATE src)
  target_compile_features(read_subchannel_current_position_tests PRIVATE c_std_99)
  add_test(NAME read_subchannel_current_position_tests
    COMMAND read_subchannel_current_position_tests)
  add_executable(read_subchannel_command_tests tests/read_subchannel_command_tests.c
    src/scsi/read_subchannel_command.c)
  target_compile_definitions(read_subchannel_command_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_subchannel_command_tests PRIVATE src)
  target_compile_features(read_subchannel_command_tests PRIVATE c_std_99)
  add_test(NAME read_subchannel_command_tests COMMAND read_subchannel_command_tests)
  add_executable(controller_measurement_series02_tests
    tests/controller_measurement_series02_tests.c
    src/platform/controller_measurement_series02.c)
  target_compile_definitions(controller_measurement_series02_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_series02_tests PRIVATE src)
  target_compile_features(controller_measurement_series02_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_series02_tests COMMAND controller_measurement_series02_tests)
  add_executable(controller_measurement_mode6_series05_tests
    tests/controller_measurement_mode6_series05_tests.c
    src/platform/controller_measurement_mode6_series05.c)
  target_compile_definitions(controller_measurement_mode6_series05_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_mode6_series05_tests PRIVATE src)
  target_compile_features(controller_measurement_mode6_series05_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_mode6_series05_tests
    COMMAND controller_measurement_mode6_series05_tests)
  add_executable(controller_measurement_series05_tests
    tests/controller_measurement_series05_tests.c
    src/platform/controller_measurement_series05.c)
  target_compile_definitions(controller_measurement_series05_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_series05_tests PRIVATE src)
  target_compile_features(controller_measurement_series05_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_series05_tests
    COMMAND controller_measurement_series05_tests)
  add_executable(controller_measurement_series04_value_tests
    tests/controller_measurement_series04_value_tests.c
    src/platform/controller_measurement_series04_value.c)
  target_compile_definitions(controller_measurement_series04_value_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_series04_value_tests PRIVATE src)
  target_compile_features(controller_measurement_series04_value_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_series04_value_tests
    COMMAND controller_measurement_series04_value_tests)
  add_executable(controller_measurement_mode6_adjustment_tests
    tests/controller_measurement_mode6_adjustment_tests.c
    src/platform/controller_measurement_mode6_adjustment.c)
  target_compile_definitions(controller_measurement_mode6_adjustment_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_mode6_adjustment_tests PRIVATE src)
  target_compile_features(controller_measurement_mode6_adjustment_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(controller_measurement_mode6_adjustment_tests PRIVATE m)
  endif()
  add_test(NAME controller_measurement_mode6_adjustment_tests
    COMMAND controller_measurement_mode6_adjustment_tests)
  add_executable(controller_measurement_mode2_adjustment_tests
    tests/controller_measurement_mode2_adjustment_tests.c
    src/platform/controller_measurement_mode2_adjustment.c)
  target_compile_definitions(controller_measurement_mode2_adjustment_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_mode2_adjustment_tests PRIVATE src)
  target_compile_features(controller_measurement_mode2_adjustment_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_mode2_adjustment_tests
    COMMAND controller_measurement_mode2_adjustment_tests)
  add_executable(controller_measurement_mode6_transform_tests
    tests/controller_measurement_mode6_transform_tests.c
    src/platform/controller_measurement_mode6_transform.c)
  target_compile_definitions(controller_measurement_mode6_transform_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_mode6_transform_tests PRIVATE src)
  target_compile_features(controller_measurement_mode6_transform_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_mode6_transform_tests
    COMMAND controller_measurement_mode6_transform_tests)
  add_executable(controller_measurement_transform_tests
    tests/controller_measurement_transform_tests.c
    src/platform/controller_measurement_transform.c)
  target_compile_definitions(controller_measurement_transform_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_transform_tests PRIVATE src)
  target_compile_features(controller_measurement_transform_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_transform_tests
    COMMAND controller_measurement_transform_tests)
  add_executable(controller_measurement_interpolation_tests
    tests/controller_measurement_interpolation_tests.c
    src/platform/controller_measurement_interpolation.c)
  target_compile_definitions(controller_measurement_interpolation_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_interpolation_tests PRIVATE src)
  target_compile_features(controller_measurement_interpolation_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_interpolation_tests
    COMMAND controller_measurement_interpolation_tests)
  add_executable(controller_measurement_series00_01_tests
    tests/controller_measurement_series00_01_tests.c
    src/platform/controller_measurement_series00_01.c)
  target_compile_definitions(controller_measurement_series00_01_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_series00_01_tests PRIVATE src)
  target_compile_features(controller_measurement_series00_01_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_series00_01_tests
    COMMAND controller_measurement_series00_01_tests)
  add_executable(controller_measurement_profile_initialization_tests
    tests/controller_measurement_profile_initialization_tests.c
    src/platform/controller_measurement_profile_initialization.c)
  target_compile_definitions(controller_measurement_profile_initialization_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_profile_initialization_tests
    PRIVATE src)
  target_compile_features(controller_measurement_profile_initialization_tests
    PRIVATE c_std_99)
  add_test(NAME controller_measurement_profile_initialization_tests
    COMMAND controller_measurement_profile_initialization_tests)
  add_executable(controller_measurement_sequence_tests
    tests/controller_measurement_sequence_tests.c
    src/platform/controller_measurement_sequence.c)
  target_compile_definitions(controller_measurement_sequence_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_sequence_tests PRIVATE src)
  target_compile_features(controller_measurement_sequence_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_sequence_tests
    COMMAND controller_measurement_sequence_tests)
  add_executable(controller_measurement_pass_tests
    tests/controller_measurement_pass_tests.c
    src/platform/controller_measurement_pass.c)
  target_compile_definitions(controller_measurement_pass_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_pass_tests PRIVATE src)
  target_compile_features(controller_measurement_pass_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_pass_tests
    COMMAND controller_measurement_pass_tests)
  add_executable(controller_measurement_retry_sequence_tests
    tests/controller_measurement_retry_sequence_tests.c
    src/platform/controller_measurement_retry_sequence.c)
  target_compile_definitions(controller_measurement_retry_sequence_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_retry_sequence_tests
    PRIVATE src)
  target_compile_features(controller_measurement_retry_sequence_tests
    PRIVATE c_std_99)
  add_test(NAME controller_measurement_retry_sequence_tests
    COMMAND controller_measurement_retry_sequence_tests)
  add_executable(raw_read_position_probe_step_tests
    tests/raw_read_position_probe_step_tests.c
    src/media/raw_read_position_probe_step.c)
  target_compile_definitions(raw_read_position_probe_step_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_position_probe_step_tests PRIVATE src)
  target_compile_features(raw_read_position_probe_step_tests PRIVATE c_std_99)
  add_test(NAME raw_read_position_probe_step_tests
    COMMAND raw_read_position_probe_step_tests)
  add_executable(packed_subchannel_record_batch_tests
    tests/packed_subchannel_record_batch_tests.c
    src/media/packed_subchannel_record_batch.c)
  target_compile_definitions(packed_subchannel_record_batch_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(packed_subchannel_record_batch_tests PRIVATE src)
  target_compile_features(packed_subchannel_record_batch_tests PRIVATE c_std_99)
  add_test(NAME packed_subchannel_record_batch_tests
    COMMAND packed_subchannel_record_batch_tests)
  add_executable(raw_sector_buffer_event_tests
    tests/raw_sector_buffer_event_tests.c
    src/media/raw_sector_buffer_event.c)
  target_compile_definitions(raw_sector_buffer_event_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_sector_buffer_event_tests PRIVATE src)
  target_compile_features(raw_sector_buffer_event_tests PRIVATE c_std_99)
  add_test(NAME raw_sector_buffer_event_tests
    COMMAND raw_sector_buffer_event_tests)
  add_executable(raw_sector_completion_tests
    tests/raw_sector_completion_tests.c
    src/media/raw_sector_completion.c)
  target_compile_definitions(raw_sector_completion_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_sector_completion_tests PRIVATE src)
  target_compile_features(raw_sector_completion_tests PRIVATE c_std_99)
  add_test(NAME raw_sector_completion_tests
    COMMAND raw_sector_completion_tests)
  add_executable(primary_raw_read_producer_position_tests
    tests/primary_raw_read_producer_position_tests.c
    src/media/primary_raw_read_producer_position.c)
  target_compile_definitions(primary_raw_read_producer_position_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(primary_raw_read_producer_position_tests PRIVATE src)
  target_compile_features(primary_raw_read_producer_position_tests PRIVATE c_std_99)
  add_test(NAME primary_raw_read_producer_position_tests
    COMMAND primary_raw_read_producer_position_tests)
  add_executable(parsed_q_raw_read_producer_position_tests
    tests/parsed_q_raw_read_producer_position_tests.c
    src/media/parsed_q_raw_read_producer_position.c)
  target_compile_definitions(parsed_q_raw_read_producer_position_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(parsed_q_raw_read_producer_position_tests PRIVATE src)
  target_compile_features(parsed_q_raw_read_producer_position_tests PRIVATE c_std_99)
  add_test(NAME parsed_q_raw_read_producer_position_tests
    COMMAND parsed_q_raw_read_producer_position_tests)
  add_executable(controller_read_event_tests
    tests/controller_read_event_tests.c
    src/media/controller_read_event.c)
  target_compile_definitions(controller_read_event_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_read_event_tests PRIVATE src)
  target_compile_features(controller_read_event_tests PRIVATE c_std_99)
  add_test(NAME controller_read_event_tests COMMAND controller_read_event_tests)
  add_executable(primary_raw_read_operating_code_tests
    tests/primary_raw_read_operating_code_tests.c
    src/media/primary_raw_read_operating_code.c)
  target_compile_definitions(primary_raw_read_operating_code_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(primary_raw_read_operating_code_tests PRIVATE src)
  target_compile_features(primary_raw_read_operating_code_tests PRIVATE c_std_99)
  add_test(NAME primary_raw_read_operating_code_tests
    COMMAND primary_raw_read_operating_code_tests)
  add_executable(primary_raw_read_pipeline_step_tests
    tests/primary_raw_read_pipeline_step_tests.c
    src/media/primary_raw_read_pipeline_step.c)
  target_compile_definitions(primary_raw_read_pipeline_step_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(primary_raw_read_pipeline_step_tests PRIVATE src)
  target_compile_features(primary_raw_read_pipeline_step_tests PRIVATE c_std_99)
  add_test(NAME primary_raw_read_pipeline_step_tests
    COMMAND primary_raw_read_pipeline_step_tests)
  add_executable(primary_raw_read_pipeline_coroutine_tests
    tests/primary_raw_read_pipeline_coroutine_tests.c
    src/media/primary_raw_read_pipeline_coroutine.c)
  target_compile_definitions(primary_raw_read_pipeline_coroutine_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(primary_raw_read_pipeline_coroutine_tests PRIVATE src)
  target_compile_features(primary_raw_read_pipeline_coroutine_tests PRIVATE c_std_99)
  add_test(NAME primary_raw_read_pipeline_coroutine_tests
    COMMAND primary_raw_read_pipeline_coroutine_tests)
  add_executable(controller_threshold_probe_tests
    tests/controller_threshold_probe_tests.c
    src/platform/controller_threshold_probe.c)
  target_compile_definitions(controller_threshold_probe_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_threshold_probe_tests PRIVATE src)
  target_compile_features(controller_threshold_probe_tests PRIVATE c_std_99)
  add_test(NAME controller_threshold_probe_tests
    COMMAND controller_threshold_probe_tests)
  add_executable(controller_threshold_sampling_tests
    tests/controller_threshold_sampling_tests.c
    src/platform/controller_threshold_sampling.c)
  target_compile_definitions(controller_threshold_sampling_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_threshold_sampling_tests PRIVATE src)
  target_compile_features(controller_threshold_sampling_tests PRIVATE c_std_99)
  add_test(NAME controller_threshold_sampling_tests
    COMMAND controller_threshold_sampling_tests)
  add_executable(raw_read_pipeline_reset_tests
    tests/raw_read_pipeline_reset_tests.c
    src/media/raw_read_pipeline_reset.c)
  target_compile_definitions(raw_read_pipeline_reset_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_pipeline_reset_tests PRIVATE src)
  target_compile_features(raw_read_pipeline_reset_tests PRIVATE c_std_99)
  add_test(NAME raw_read_pipeline_reset_tests
    COMMAND raw_read_pipeline_reset_tests)
  add_executable(controller_record_find_tests
    tests/controller_record_find_tests.c
    src/platform/controller_record_find.c)
  target_compile_definitions(controller_record_find_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_find_tests PRIVATE src)
  target_compile_features(controller_record_find_tests PRIVATE c_std_99)
  add_test(NAME controller_record_find_tests
    COMMAND controller_record_find_tests)
  add_executable(controller_record_page_compaction_tests
    tests/controller_record_page_compaction_tests.c
    src/platform/controller_record_page_compaction.c)
  target_compile_definitions(controller_record_page_compaction_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_page_compaction_tests PRIVATE src)
  target_compile_features(controller_record_page_compaction_tests PRIVATE c_std_99)
  add_test(NAME controller_record_page_compaction_tests
    COMMAND controller_record_page_compaction_tests)
  add_executable(controller_record_region_scan_tests
    tests/controller_record_region_scan_tests.c
    src/platform/controller_record_region_scan.c)
  target_compile_definitions(controller_record_region_scan_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_record_region_scan_tests PRIVATE src)
  target_compile_features(controller_record_region_scan_tests PRIVATE c_std_99)
  add_test(NAME controller_record_region_scan_tests
    COMMAND controller_record_region_scan_tests)
  add_executable(disc_radial_calibration_tests
    tests/disc_radial_calibration_tests.c
    src/platform/disc_radial_calibration.c)
  target_compile_definitions(disc_radial_calibration_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(disc_radial_calibration_tests PRIVATE src)
  target_compile_features(disc_radial_calibration_tests PRIVATE c_std_99)
  add_test(NAME disc_radial_calibration_tests
    COMMAND disc_radial_calibration_tests)
  add_executable(controller_position_average_tests
    tests/controller_position_average_tests.c
    src/platform/controller_position_average.c)
  target_compile_definitions(controller_position_average_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_position_average_tests PRIVATE src)
  target_compile_features(controller_position_average_tests PRIVATE c_std_99)
  add_test(NAME controller_position_average_tests
    COMMAND controller_position_average_tests)
  add_executable(radial_position_table_tests
    tests/radial_position_table_tests.c
    src/platform/radial_position_table.c)
  target_compile_definitions(radial_position_table_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(radial_position_table_tests PRIVATE src)
  target_compile_features(radial_position_table_tests PRIVATE c_std_99)
  if(UNIX)
    target_link_libraries(radial_position_table_tests PRIVATE m)
  endif()
  add_test(NAME radial_position_table_tests
    COMMAND radial_position_table_tests)
  add_executable(controller_q_packet_process_tests
    tests/controller_q_packet_process_tests.c
    src/platform/controller_q_packet_process.c)
  target_compile_definitions(controller_q_packet_process_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_q_packet_process_tests PRIVATE src)
  target_compile_features(controller_q_packet_process_tests PRIVATE c_std_99)
  add_test(NAME controller_q_packet_process_tests
    COMMAND controller_q_packet_process_tests)
  add_executable(controller_measurement_transition_tests
    tests/controller_measurement_transition_tests.c
    src/platform/controller_measurement_transition.c)
  target_compile_definitions(controller_measurement_transition_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_transition_tests
    PRIVATE src)
  target_compile_features(controller_measurement_transition_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_transition_tests
    COMMAND controller_measurement_transition_tests)
  add_executable(controller_measurement_reference_tests
    tests/controller_measurement_reference_tests.c
    src/platform/controller_measurement_reference.c)
  target_compile_definitions(controller_measurement_reference_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_measurement_reference_tests
    PRIVATE src)
  target_compile_features(controller_measurement_reference_tests PRIVATE c_std_99)
  add_test(NAME controller_measurement_reference_tests
    COMMAND controller_measurement_reference_tests)
  add_executable(pickup_move_controller_sequence_tests
    tests/pickup_move_controller_sequence_tests.c
    src/platform/pickup_move_controller_sequence.c)
  target_compile_definitions(pickup_move_controller_sequence_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(pickup_move_controller_sequence_tests
    PRIVATE src)
  target_compile_features(pickup_move_controller_sequence_tests PRIVATE c_std_99)
  add_test(NAME pickup_move_controller_sequence_tests
    COMMAND pickup_move_controller_sequence_tests)
  add_executable(controller_event_dispatch_tests
    tests/controller_event_dispatch_tests.c
    src/platform/controller_event_dispatch.c)
  target_compile_definitions(controller_event_dispatch_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_event_dispatch_tests PRIVATE src)
  target_compile_features(controller_event_dispatch_tests PRIVATE c_std_99)
  add_test(NAME controller_event_dispatch_tests
    COMMAND controller_event_dispatch_tests)
  add_executable(controller_sfr_drain_tests
    tests/controller_sfr_drain_tests.c
    src/platform/controller_sfr_drain.c)
  target_compile_definitions(controller_sfr_drain_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_sfr_drain_tests PRIVATE src)
  target_compile_features(controller_sfr_drain_tests PRIVATE c_std_99)
  add_test(NAME controller_sfr_drain_tests COMMAND controller_sfr_drain_tests)
  add_executable(controller_word0251_sweep_tests
    tests/controller_word0251_sweep_tests.c
    src/platform/controller_word0251_sweep.c)
  target_compile_definitions(controller_word0251_sweep_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_word0251_sweep_tests PRIVATE src)
  target_compile_features(controller_word0251_sweep_tests PRIVATE c_std_99)
  add_test(NAME controller_word0251_sweep_tests
    COMMAND controller_word0251_sweep_tests)
  add_executable(timer_phase2_tests
    tests/timer_phase2_tests.c
    src/platform/timer_phase2.c)
  target_compile_definitions(timer_phase2_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(timer_phase2_tests PRIVATE src)
  target_compile_features(timer_phase2_tests PRIVATE c_std_99)
  add_test(NAME timer_phase2_tests COMMAND timer_phase2_tests)
  add_executable(timer_phase0_tests
    tests/timer_phase0_tests.c
    src/platform/timer_phase0.c)
  target_compile_definitions(timer_phase0_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(timer_phase0_tests PRIVATE src)
  target_compile_features(timer_phase0_tests PRIVATE c_std_99)
  add_test(NAME timer_phase0_tests COMMAND timer_phase0_tests)
  add_executable(sfr_da_db_measurement_tests
    tests/sfr_da_db_measurement_tests.c
    src/platform/sfr_da_db_measurement.c)
  target_compile_definitions(sfr_da_db_measurement_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(sfr_da_db_measurement_tests PRIVATE src)
  target_compile_features(sfr_da_db_measurement_tests PRIVATE c_std_99)
  add_test(NAME sfr_da_db_measurement_tests COMMAND sfr_da_db_measurement_tests)
  add_executable(read_format_capacities_tests
    tests/read_format_capacities_tests.c
    src/scsi/read_format_capacities.c)
  target_compile_definitions(read_format_capacities_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_format_capacities_tests PRIVATE src)
  target_compile_features(read_format_capacities_tests PRIVATE c_std_99)
  add_test(NAME read_format_capacities_tests
    COMMAND read_format_capacities_tests)
  add_executable(write_buffer_tests
    tests/write_buffer_tests.c
    src/scsi/write_buffer.c)
  target_compile_definitions(write_buffer_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_buffer_tests PRIVATE src)
  target_compile_features(write_buffer_tests PRIVATE c_std_99)
  add_test(NAME write_buffer_tests COMMAND write_buffer_tests)
  add_executable(write_buffer_snapshot_validation_tests
    tests/write_buffer_snapshot_validation_tests.c
    src/scsi/write_buffer_snapshot_validation.c)
  target_compile_definitions(write_buffer_snapshot_validation_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(write_buffer_snapshot_validation_tests PRIVATE src)
  target_compile_features(write_buffer_snapshot_validation_tests PRIVATE c_std_99)
  add_test(NAME write_buffer_snapshot_validation_tests
    COMMAND write_buffer_snapshot_validation_tests)
  add_executable(read_buffer_tests
    tests/read_buffer_tests.c
    src/scsi/read_buffer.c)
  target_compile_definitions(read_buffer_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_buffer_tests PRIVATE src)
  target_compile_features(read_buffer_tests PRIVATE c_std_99)
  add_test(NAME read_buffer_tests COMMAND read_buffer_tests)
  add_executable(read_buffer_snapshot_tests
    tests/read_buffer_snapshot_tests.c
    src/scsi/read_buffer_snapshot.c)
  target_compile_definitions(read_buffer_snapshot_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_buffer_snapshot_tests PRIVATE src)
  target_compile_features(read_buffer_snapshot_tests PRIVATE c_std_99)
  add_test(NAME read_buffer_snapshot_tests COMMAND read_buffer_snapshot_tests)
  add_executable(read_buffer_snapshot_restore_tests
    tests/read_buffer_snapshot_restore_tests.c
    src/scsi/read_buffer_snapshot_restore.c)
  target_compile_definitions(read_buffer_snapshot_restore_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_buffer_snapshot_restore_tests PRIVATE src)
  target_compile_features(read_buffer_snapshot_restore_tests PRIVATE c_std_99)
  add_test(NAME read_buffer_snapshot_restore_tests
    COMMAND read_buffer_snapshot_restore_tests)
  add_executable(opcode_f0_configuration_update_tests
    tests/opcode_f0_configuration_update_tests.c
    src/scsi/opcode_f0_configuration_update.c)
  target_compile_definitions(opcode_f0_configuration_update_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(opcode_f0_configuration_update_tests PRIVATE src)
  target_compile_features(opcode_f0_configuration_update_tests PRIVATE c_std_99)
  add_test(NAME opcode_f0_configuration_update_tests
    COMMAND opcode_f0_configuration_update_tests)
  add_executable(configuration_range_update_tests
    tests/configuration_range_update_tests.c
    src/platform/configuration_range_update.c)
  target_compile_definitions(configuration_range_update_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(configuration_range_update_tests PRIVATE src)
  target_compile_features(configuration_range_update_tests PRIVATE c_std_99)
  add_test(NAME configuration_range_update_tests
    COMMAND configuration_range_update_tests)
  add_executable(raw_sector_range_tests
    tests/raw_sector_range_tests.c
    src/scsi/raw_sector_range.c)
  target_compile_definitions(raw_sector_range_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_sector_range_tests PRIVATE src)
  target_compile_features(raw_sector_range_tests PRIVATE c_std_99)
  add_test(NAME raw_sector_range_tests COMMAND raw_sector_range_tests)
  add_executable(read_toc_cd_text_acquisition_tests
    tests/read_toc_cd_text_acquisition_tests.c
    src/scsi/read_toc_cd_text_acquisition.c)
  target_compile_definitions(read_toc_cd_text_acquisition_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_toc_cd_text_acquisition_tests PRIVATE src)
  target_compile_features(read_toc_cd_text_acquisition_tests PRIVATE c_std_99)
  add_test(NAME read_toc_cd_text_acquisition_tests
    COMMAND read_toc_cd_text_acquisition_tests)
  add_executable(cd_text_pack_validation_tests
    tests/cd_text_pack_validation_tests.c
    src/scsi/cd_text_pack_validation.c)
  target_compile_definitions(cd_text_pack_validation_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(cd_text_pack_validation_tests PRIVATE src)
  target_compile_features(cd_text_pack_validation_tests PRIVATE c_std_99)
  add_test(NAME cd_text_pack_validation_tests
    COMMAND cd_text_pack_validation_tests)
  add_executable(raw_sector_batched_host_transfer_tests
    tests/raw_sector_batched_host_transfer_tests.c
    src/scsi/raw_sector_batched_host_transfer.c)
  target_compile_definitions(raw_sector_batched_host_transfer_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_sector_batched_host_transfer_tests PRIVATE src)
  target_compile_features(raw_sector_batched_host_transfer_tests PRIVATE c_std_99)
  add_test(NAME raw_sector_batched_host_transfer_tests
    COMMAND raw_sector_batched_host_transfer_tests)
  add_executable(read_cd_engine_tests
    tests/read_cd_engine_tests.c
    src/scsi/read_cd_engine.c)
  target_compile_definitions(read_cd_engine_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_cd_engine_tests PRIVATE src)
  target_compile_features(read_cd_engine_tests PRIVATE c_std_99)
  add_test(NAME read_cd_engine_tests COMMAND read_cd_engine_tests)
  add_executable(read_cd_track_range_match_tests
    tests/read_cd_track_range_match_tests.c
    src/scsi/read_cd_track_range_match.c)
  target_compile_definitions(read_cd_track_range_match_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_cd_track_range_match_tests PRIVATE src)
  target_compile_features(read_cd_track_range_match_tests PRIVATE c_std_99)
  add_test(NAME read_cd_track_range_match_tests
    COMMAND read_cd_track_range_match_tests)
  add_executable(disc_read_position_tests
    tests/disc_read_position_tests.c
    src/media/disc_read_position.c)
  target_compile_definitions(disc_read_position_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(disc_read_position_tests PRIVATE src)
  target_compile_features(disc_read_position_tests PRIVATE c_std_99)
  add_test(NAME disc_read_position_tests
    COMMAND disc_read_position_tests)
  add_executable(controller_parameter_calibration_search_tests
    tests/controller_parameter_calibration_search_tests.c
    src/platform/controller_parameter_calibration_search.c)
  target_compile_definitions(controller_parameter_calibration_search_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_parameter_calibration_search_tests
    PRIVATE src)
  target_compile_features(controller_parameter_calibration_search_tests
    PRIVATE c_std_99)
  add_test(NAME controller_parameter_calibration_search_tests
    COMMAND controller_parameter_calibration_search_tests)
  add_executable(controller_parameter_fc_calibration_tests
    tests/controller_parameter_fc_calibration_tests.c
    src/platform/controller_parameter_fc_calibration.c)
  target_compile_definitions(controller_parameter_fc_calibration_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_parameter_fc_calibration_tests
    PRIVATE src)
  target_compile_features(controller_parameter_fc_calibration_tests
    PRIVATE c_std_99)
  add_test(NAME controller_parameter_fc_calibration_tests
    COMMAND controller_parameter_fc_calibration_tests)
  add_executable(vendor_e9_tests
    tests/vendor_e9_tests.c
    src/scsi/vendor_e9.c)
  target_compile_definitions(vendor_e9_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_e9_tests PRIVATE src)
  target_compile_features(vendor_e9_tests PRIVATE c_std_99)
  add_test(NAME vendor_e9_tests COMMAND vendor_e9_tests)
  add_executable(vendor_eb_tests
    tests/vendor_eb_tests.c
    src/scsi/vendor_eb.c)
  target_compile_definitions(vendor_eb_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_eb_tests PRIVATE src)
  target_compile_features(vendor_eb_tests PRIVATE c_std_99)
  add_test(NAME vendor_eb_tests COMMAND vendor_eb_tests)
  add_executable(vendor_ec_record_list_tests
    tests/vendor_ec_record_list_tests.c
    src/scsi/vendor_ec_record_list.c)
  target_compile_definitions(vendor_ec_record_list_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_ec_record_list_tests PRIVATE src)
  target_compile_features(vendor_ec_record_list_tests PRIVATE c_std_99)
  add_test(NAME vendor_ec_record_list_tests
    COMMAND vendor_ec_record_list_tests)
  add_executable(vendor_f3_tests
    tests/vendor_f3_tests.c
    src/scsi/vendor_f3.c)
  target_compile_definitions(vendor_f3_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_f3_tests PRIVATE src)
  target_compile_features(vendor_f3_tests PRIVATE c_std_99)
  add_test(NAME vendor_f3_tests COMMAND vendor_f3_tests)
  add_executable(vendor_f5_tests
    tests/vendor_f5_tests.c
    src/scsi/vendor_f5.c)
  target_compile_definitions(vendor_f5_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(vendor_f5_tests PRIVATE src)
  target_compile_features(vendor_f5_tests PRIVATE c_std_99)
  add_test(NAME vendor_f5_tests COMMAND vendor_f5_tests)
  add_executable(write_command_transfer_tests
    tests/write_command_transfer_tests.c
    src/scsi/write_command_transfer.c)
  target_compile_definitions(write_command_transfer_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_command_transfer_tests PRIVATE src)
  target_compile_features(write_command_transfer_tests PRIVATE c_std_99)
  add_test(NAME write_command_transfer_tests
    COMMAND write_command_transfer_tests)
  add_executable(write_transfer_range_tests
    tests/write_transfer_range_tests.c
    src/scsi/write_transfer_range.c)
  target_compile_definitions(write_transfer_range_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_transfer_range_tests PRIVATE src)
  target_compile_features(write_transfer_range_tests PRIVATE c_std_99)
  add_test(NAME write_transfer_range_tests COMMAND write_transfer_range_tests)
  add_executable(write_pipeline_prepare_tests
    tests/write_pipeline_prepare_tests.c
    src/scsi/write_pipeline_prepare.c)
  target_compile_definitions(write_pipeline_prepare_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_pipeline_prepare_tests PRIVATE src)
  target_compile_features(write_pipeline_prepare_tests PRIVATE c_std_99)
  add_test(NAME write_pipeline_prepare_tests
    COMMAND write_pipeline_prepare_tests)
  add_executable(write_partial_range_tests
    tests/write_partial_range_tests.c
    src/scsi/write_partial_range.c)
  target_compile_definitions(write_partial_range_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_partial_range_tests PRIVATE src)
  target_compile_features(write_partial_range_tests PRIVATE c_std_99)
  add_test(NAME write_partial_range_tests
    COMMAND write_partial_range_tests)
  add_executable(write_record_prepare_predicate_tests
    tests/write_record_prepare_predicate_tests.c
    src/scsi/write_record_prepare_predicate.c)
  target_compile_definitions(write_record_prepare_predicate_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(write_record_prepare_predicate_tests PRIVATE src)
  target_compile_features(write_record_prepare_predicate_tests PRIVATE c_std_99)
  add_test(NAME write_record_prepare_predicate_tests
    COMMAND write_record_prepare_predicate_tests)
  add_executable(type2_mip_geometry_tests
    tests/type2_mip_geometry_tests.c
    src/media/type2_mip_geometry.c)
  target_compile_definitions(type2_mip_geometry_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(type2_mip_geometry_tests PRIVATE src)
  target_compile_features(type2_mip_geometry_tests PRIVATE c_std_99)
  add_test(NAME type2_mip_geometry_tests COMMAND type2_mip_geometry_tests)
  add_executable(read_toc_format1_multisession_tests
    tests/read_toc_format1_multisession_tests.c
    src/scsi/read_toc_format1_multisession.c)
  target_compile_definitions(read_toc_format1_multisession_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_toc_format1_multisession_tests PRIVATE src)
  target_compile_features(read_toc_format1_multisession_tests PRIVATE c_std_99)
  add_test(NAME read_toc_format1_multisession_tests
    COMMAND read_toc_format1_multisession_tests)
  add_executable(read_toc_format4_atip_tests
    tests/read_toc_format4_atip_tests.c
    src/scsi/read_toc_format4_atip.c)
  target_compile_definitions(read_toc_format4_atip_tests
    PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(read_toc_format4_atip_tests PRIVATE src)
  target_compile_features(read_toc_format4_atip_tests PRIVATE c_std_99)
  add_test(NAME read_toc_format4_atip_tests COMMAND read_toc_format4_atip_tests)
  add_executable(blank_format_track_state_tests
    tests/blank_format_track_state_tests.c
    src/media/blank_format_track_state.c)
  target_compile_definitions(blank_format_track_state_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(blank_format_track_state_tests PRIVATE src)
  target_compile_features(blank_format_track_state_tests PRIVATE c_std_99)
  add_test(NAME blank_format_track_state_tests
    COMMAND blank_format_track_state_tests)
  add_executable(controller_event_bit2_progress_tests
    tests/controller_event_bit2_progress_tests.c
    src/platform/controller_event_bit2_progress.c)
  target_compile_definitions(controller_event_bit2_progress_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_event_bit2_progress_tests PRIVATE src)
  target_compile_features(controller_event_bit2_progress_tests PRIVATE c_std_99)
  add_test(NAME controller_event_bit2_progress_tests
    COMMAND controller_event_bit2_progress_tests)
  add_executable(controller_buffer_setup_tests
    tests/controller_buffer_setup_tests.c
    src/platform/controller_buffer_setup.c)
  target_compile_definitions(controller_buffer_setup_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_buffer_setup_tests PRIVATE src)
  target_compile_features(controller_buffer_setup_tests PRIVATE c_std_99)
  add_test(NAME controller_buffer_setup_tests
    COMMAND controller_buffer_setup_tests)
  add_executable(atip_initialization_tests
    tests/atip_initialization_tests.c
    src/media/atip_initialization.c)
  target_compile_definitions(atip_initialization_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(atip_initialization_tests PRIVATE src)
  target_compile_features(atip_initialization_tests PRIVATE c_std_99)
  add_test(NAME atip_initialization_tests COMMAND atip_initialization_tests)
  add_executable(pma_acquisition_tests
    tests/pma_acquisition_tests.c
    src/media/pma_acquisition.c)
  target_compile_definitions(pma_acquisition_tests PRIVATE PLEXTOR_HOST_TEST=1)
  target_include_directories(pma_acquisition_tests PRIVATE src)
  target_compile_features(pma_acquisition_tests PRIVATE c_std_99)
  add_test(NAME pma_acquisition_tests COMMAND pma_acquisition_tests)
  add_executable(identify_packet_device_tests
    tests/identify_packet_device_tests.c
    src/ata/identify_packet_device.c)
  target_compile_definitions(identify_packet_device_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(identify_packet_device_tests PRIVATE src)
  target_compile_features(identify_packet_device_tests PRIVATE c_std_99)
  add_test(NAME identify_packet_device_tests
    COMMAND identify_packet_device_tests)
  add_executable(controller_asic_initialization_tests
    tests/controller_asic_initialization_tests.c
    src/platform/controller_asic_initialization.c)
  target_compile_definitions(controller_asic_initialization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_asic_initialization_tests PRIVATE src)
  target_compile_features(controller_asic_initialization_tests PRIVATE c_std_99)
  add_test(NAME controller_asic_initialization_tests
    COMMAND controller_asic_initialization_tests)
  add_executable(drive_firmware_initialization_tests
    tests/drive_firmware_initialization_tests.c
    src/startup/drive_firmware_initialization.c)
  target_compile_definitions(drive_firmware_initialization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(drive_firmware_initialization_tests PRIVATE src)
  target_compile_features(drive_firmware_initialization_tests PRIVATE c_std_99)
  add_test(NAME drive_firmware_initialization_tests
    COMMAND drive_firmware_initialization_tests)
  add_executable(drive_configuration_state_tests
    tests/drive_configuration_state_tests.c
    src/startup/drive_configuration_state.c)
  target_compile_definitions(drive_configuration_state_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(drive_configuration_state_tests PRIVATE src)
  target_compile_features(drive_configuration_state_tests PRIVATE c_std_99)
  add_test(NAME drive_configuration_state_tests
    COMMAND drive_configuration_state_tests)
  add_executable(disc_control_state_initialization_tests
    tests/disc_control_state_initialization_tests.c
    src/startup/disc_control_state_initialization.c)
  target_compile_definitions(disc_control_state_initialization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(disc_control_state_initialization_tests PRIVATE src)
  target_compile_features(disc_control_state_initialization_tests PRIVATE c_std_99)
  add_test(NAME disc_control_state_initialization_tests
    COMMAND disc_control_state_initialization_tests)
  add_executable(operating_code14_measurement_bounds_tests
    tests/operating_code14_measurement_bounds_tests.c
    src/platform/operating_code14_measurement_bounds.c)
  target_compile_definitions(operating_code14_measurement_bounds_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(operating_code14_measurement_bounds_tests PRIVATE src)
  target_compile_features(operating_code14_measurement_bounds_tests PRIVATE c_std_99)
  if(NOT MSVC)
    target_link_libraries(operating_code14_measurement_bounds_tests PRIVATE m)
  endif()
  add_test(NAME operating_code14_measurement_bounds_tests
    COMMAND operating_code14_measurement_bounds_tests)
  add_executable(controller_operating_adjustment_response_tests
    tests/controller_operating_adjustment_response_tests.c
    src/platform/controller_operating_adjustment_response.c)
  target_compile_definitions(controller_operating_adjustment_response_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_operating_adjustment_response_tests PRIVATE src)
  target_compile_features(controller_operating_adjustment_response_tests PRIVATE c_std_99)
  add_test(NAME controller_operating_adjustment_response_tests
    COMMAND controller_operating_adjustment_response_tests)
  add_executable(read_acquisition_calibration_sequence_tests
    tests/read_acquisition_calibration_sequence_tests.c
    src/platform/read_acquisition_calibration_sequence.c)
  target_compile_definitions(read_acquisition_calibration_sequence_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(read_acquisition_calibration_sequence_tests PRIVATE src)
  target_compile_features(read_acquisition_calibration_sequence_tests PRIVATE c_std_99)
  add_test(NAME read_acquisition_calibration_sequence_tests
    COMMAND read_acquisition_calibration_sequence_tests)
  add_executable(controller_buffer_chunk_fill_tests
    tests/controller_buffer_chunk_fill_tests.c
    src/platform/controller_buffer_chunk_fill.c)
  target_compile_definitions(controller_buffer_chunk_fill_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(controller_buffer_chunk_fill_tests PRIVATE src)
  target_compile_features(controller_buffer_chunk_fill_tests PRIVATE c_std_99)
  add_test(NAME controller_buffer_chunk_fill_tests
    COMMAND controller_buffer_chunk_fill_tests)
  add_executable(raw_read_position_validation_tests
    tests/raw_read_position_validation_tests.c
    src/media/raw_read_position_validation.c)
  target_compile_definitions(raw_read_position_validation_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_position_validation_tests PRIVATE src)
  target_compile_features(raw_read_position_validation_tests PRIVATE c_std_99)
  add_test(NAME raw_read_position_validation_tests
    COMMAND raw_read_position_validation_tests)
  add_executable(raw_read_position_tracker_tests
    tests/raw_read_position_tracker_tests.c
    src/media/raw_read_position_tracker.c)
  target_compile_definitions(raw_read_position_tracker_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(raw_read_position_tracker_tests PRIVATE src)
  target_compile_features(raw_read_position_tracker_tests PRIVATE c_std_99)
  add_test(NAME raw_read_position_tracker_tests
    COMMAND raw_read_position_tracker_tests)
  add_executable(fixed_packet_overhead_tests
    tests/fixed_packet_overhead_tests.c
    src/media/fixed_packet_overhead.c)
  target_compile_definitions(fixed_packet_overhead_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(fixed_packet_overhead_tests PRIVATE src)
  target_compile_features(fixed_packet_overhead_tests PRIVATE c_std_99)
  add_test(NAME fixed_packet_overhead_tests
    COMMAND fixed_packet_overhead_tests)
  add_executable(primary_raw_read_capture_batch_tests
    tests/primary_raw_read_capture_batch_tests.c
    src/media/primary_raw_read_capture_batch.c)
  target_compile_definitions(primary_raw_read_capture_batch_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(primary_raw_read_capture_batch_tests PRIVATE src)
  target_compile_features(primary_raw_read_capture_batch_tests PRIVATE c_std_99)
  add_test(NAME primary_raw_read_capture_batch_tests
    COMMAND primary_raw_read_capture_batch_tests)
  add_executable(strategy_profile_initialization_tests
    tests/strategy_profile_initialization_tests.c
    src/media/strategy_profile_initialization.c)
  target_compile_definitions(strategy_profile_initialization_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(strategy_profile_initialization_tests PRIVATE src)
  target_compile_features(strategy_profile_initialization_tests PRIVATE c_std_99)
  add_test(NAME strategy_profile_initialization_tests
    COMMAND strategy_profile_initialization_tests)
  add_executable(send_diagnostic_tests
    tests/send_diagnostic_tests.c
    src/scsi/send_diagnostic.c)
  target_compile_definitions(send_diagnostic_tests PRIVATE
    PLEXTOR_HOST_TEST=1)
  target_include_directories(send_diagnostic_tests PRIVATE src)
  target_compile_features(send_diagnostic_tests PRIVATE c_std_99)
  add_test(NAME send_diagnostic_tests COMMAND send_diagnostic_tests)
  if(NOT MSVC)
    target_link_libraries(reconstruction_foundation_host PUBLIC m)
  endif()
endif()

if(PLEXTOR_BUILD_FIRMWARE)
  include(cmake/keil-c251.cmake)
  # Separate configuration directories also work with Visual Studio generators.
  set(FW_DIR "${PROJECT_BINARY_DIR}/firmware/$<CONFIG>")
  if(PLEXTOR_LED_STARTUP_DIAGNOSTICS)
    list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_STARTUP_DIAGNOSTICS)")
  endif()
  if(PLEXTOR_LED_FOREGROUND_DIAGNOSTICS)
    list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_FOREGROUND_DIAGNOSTICS)")
  endif()
  if(PLEXTOR_LED_HOST_GATE_DIAGNOSTICS)
    list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_HOST_GATE_DIAGNOSTICS)")
    if(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "phases")
      list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)")
      set(PLEXTOR_HOST_GATE_ASSEMBLY src/boot/startup_host_phase_diagnostics.a51)
    elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "delay")
      list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS)")
      set(PLEXTOR_HOST_GATE_ASSEMBLY src/boot/startup_host_delay_diagnostics.a51)
    elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "route")
      list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_HOST_ROUTE_DIAGNOSTICS)")
      set(PLEXTOR_HOST_GATE_ASSEMBLY src/boot/startup_host_route_diagnostics.a51)
    elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "interval")
      list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_HOST_INTERVAL_DIAGNOSTICS)")
      set(PLEXTOR_HOST_GATE_ASSEMBLY src/boot/startup_host_interval_diagnostics.a51)
    elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "zero")
      list(APPEND PLEXTOR_C251_OPTIONS "DEFINE(PLEXTOR_LED_HOST_ZERO_DIAGNOSTICS)")
      set(PLEXTOR_HOST_GATE_ASSEMBLY src/boot/startup_host_zero_diagnostics.a51)
    elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "packet")
      set(PLEXTOR_HOST_GATE_ASSEMBLY src/boot/startup_host_packet_diagnostics.a51)
    else()
      set(PLEXTOR_HOST_GATE_ASSEMBLY src/boot/startup_host_gate_diagnostics.a51)
    endif()
    keil_step(startup_host_gate_diagnostics A251 startup_host_gate_diagnostics.obj
      ARGS "${PROJECT_SOURCE_DIR}/${PLEXTOR_HOST_GATE_ASSEMBLY}"
        ${PLEXTOR_A251_OPTIONS}
        "OBJECT(startup_host_gate_diagnostics.obj)" "PRINT(startup_host_gate_diagnostics.lst)"
      DEPENDS "${PLEXTOR_HOST_GATE_ASSEMBLY}"
      BYPRODUCTS "${FW_DIR}/startup_host_gate_diagnostics.lst")
  endif()
  if(PLEXTOR_LED_FOREGROUND_DIAGNOSTICS OR PLEXTOR_LED_HOST_GATE_DIAGNOSTICS)
    keil_step(startup_foreground_diagnostics A251 startup_foreground_diagnostics.obj
      ARGS "${PROJECT_SOURCE_DIR}/src/boot/startup_foreground_diagnostics.a51"
        ${PLEXTOR_A251_OPTIONS}
        "OBJECT(startup_foreground_diagnostics.obj)" "PRINT(startup_foreground_diagnostics.lst)"
      DEPENDS src/boot/startup_foreground_diagnostics.a51
      BYPRODUCTS "${FW_DIR}/startup_foreground_diagnostics.lst")
  endif()
  file(GENERATE OUTPUT "${FW_DIR}/.configured" CONTENT "C251 reconstructed main firmware\n")
  add_custom_command(
    OUTPUT "${FW_DIR}/firmware_rom_data.c"
    COMMAND "${Python3_EXECUTABLE}"
      "${PROJECT_SOURCE_DIR}/tools/generate_firmware_rom_data.py"
      --firmware "${FIRMWARE_REFERENCE}"
      --output "${FW_DIR}/firmware_rom_data.c"
    DEPENDS "${FIRMWARE_REFERENCE}"
      "${PROJECT_SOURCE_DIR}/tools/generate_firmware_rom_data.py"
      "${PROJECT_SOURCE_DIR}/src/platform/firmware_rom_data.h"
    VERBATIM)
  keil_step(firmware_rom_data C251 firmware_rom_data.obj
    ARGS "${FW_DIR}/firmware_rom_data.c" ${PLEXTOR_C251_OPTIONS}
      "OBJECT(firmware_rom_data.obj)" "PRINT(firmware_rom_data.lst)"
    DEPENDS "${FW_DIR}/firmware_rom_data.c"
      src/platform/firmware_rom_data.h
    BYPRODUCTS "${FW_DIR}/firmware_rom_data.lst")
  keil_step(arithmetic_boundaries A251 arithmetic_boundaries.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/arithmetic_boundaries.a51" ${PLEXTOR_A251_OPTIONS}
      "OBJECT(arithmetic_boundaries.obj)" "PRINT(arithmetic_boundaries.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/arithmetic_boundaries.a51"
    BYPRODUCTS "${FW_DIR}/arithmetic_boundaries.lst")
  keil_step(return_boundaries A251 return_boundaries.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/return_boundaries.a51" ${PLEXTOR_A251_OPTIONS}
      "OBJECT(return_boundaries.obj)" "PRINT(return_boundaries.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/return_boundaries.a51"
    BYPRODUCTS "${FW_DIR}/return_boundaries.lst")
  keil_step(configuration_runtime A251 configuration_runtime.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/configuration_runtime.a51" ${PLEXTOR_A251_OPTIONS}
      "OBJECT(configuration_runtime.obj)" "PRINT(configuration_runtime.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/configuration_runtime.a51"
    BYPRODUCTS "${FW_DIR}/configuration_runtime.lst")
  keil_step(firmware_ram_window A251 firmware_ram_window.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/firmware_ram_window.a51"
      ${PLEXTOR_A251_OPTIONS}
      "OBJECT(firmware_ram_window.obj)" "PRINT(firmware_ram_window.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/firmware_ram_window.a51"
    BYPRODUCTS "${FW_DIR}/firmware_ram_window.lst")
  keil_step(vector_veneers A251 vector_veneers.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/boot/vector_veneers.a51"
      ${PLEXTOR_A251_OPTIONS}
      "OBJECT(vector_veneers.obj)" "PRINT(vector_veneers.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/boot/vector_veneers.a51"
    BYPRODUCTS "${FW_DIR}/vector_veneers.lst")
  function(keil_c_source name source)
    keil_step(${name} C251 ${name}.obj
      ARGS "${PROJECT_SOURCE_DIR}/${source}" ${PLEXTOR_C251_OPTIONS}
        "OBJECT(${name}.obj)" "PRINT(${name}.lst)"
      DEPENDS "${source}" src/platform/mcs251.h src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/${name}.lst")
  endfunction()
  function(keil_c_interrupt_source name source)
    keil_step(${name} C251 ${name}.obj
      ARGS "${PROJECT_SOURCE_DIR}/${source}" ${PLEXTOR_C251_OPTIONS}
        NOINTVECTOR "OBJECT(${name}.obj)" "PRINT(${name}.lst)"
      DEPENDS "${source}" src/platform/mcs251.h src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/${name}.lst")
  endfunction()
  keil_step(media_interrupt_leaves C251 media_interrupt_leaves.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/scsi/command_dispatch.c"
      ${PLEXTOR_C251_OPTIONS}
      "OBJECT(media_interrupt_leaves.obj)"
      "PRINT(media_interrupt_leaves.lst)"
    DEPENDS src/scsi/command_dispatch.c src/scsi/command_dispatch.h
      src/scsi/command_dispatch_map.inc
      src/scsi/command_media_requirement_map.inc src/platform/mcs251.h
      src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/media_interrupt_leaves.lst")
  keil_c_source(timing_boundaries src/platform/timing_boundaries.c)
  keil_c_source(raw_read_queue_boundaries
    src/platform/raw_read_queue_boundaries.c)
  keil_c_source(media_interrupt_boundaries
    src/platform/media_interrupt_boundaries.c)
  keil_c_source(bit_boundaries src/platform/bit_boundaries.c)
  keil_c_source(call_boundaries src/platform/call_boundaries.c)
  keil_c_interrupt_source(reset_vector src/boot/reset_vector.c)
  keil_c_interrupt_source(controller_event_interrupt_entry
    src/boot/controller_event_interrupt_entry.c)
  keil_c_interrupt_source(host_interface_interrupt_entry
    src/boot/host_interface_interrupt_entry.c)
  keil_c_interrupt_source(controller_packet_interrupt_entry
    src/boot/controller_packet_interrupt_entry.c)
  keil_c_interrupt_source(timer0_interrupt_entry
    src/boot/timer0_interrupt_entry.c)
  keil_c_interrupt_source(sfr_e1_bit7_interrupt_entry
    src/boot/sfr_e1_bit7_interrupt_entry.c)
  keil_step(coroutine_context_switch A251 coroutine_context_switch.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/coroutine_context_switch.a51"
      ${PLEXTOR_A251_OPTIONS}
      "OBJECT(coroutine_context_switch.obj)"
      "PRINT(coroutine_context_switch.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/coroutine_context_switch.a51"
    BYPRODUCTS "${FW_DIR}/coroutine_context_switch.lst")
  keil_step(compiler_stack_transforms A251 compiler_stack_transforms.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/compiler_stack_transforms.a51"
      ${PLEXTOR_A251_OPTIONS}
      "OBJECT(compiler_stack_transforms.obj)"
      "PRINT(compiler_stack_transforms.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/compiler_stack_transforms.a51"
    BYPRODUCTS "${FW_DIR}/compiler_stack_transforms.lst")
  keil_c_source(image src/boot/image.c)
  keil_c_source(scsi_vendor src/scsi/vendor.c)
  keil_c_source(scsi_vendor_f5_response src/scsi/vendor_f5_response.c)
  keil_c_source(scsi_vendor_ed src/scsi/vendor_ed.c)
  keil_c_source(scsi_vendor_measurement_response src/scsi/vendor_measurement_response.c)
  keil_c_source(scsi_controller_command_preparation src/scsi/controller_command_preparation.c)
  keil_c_source(scsi_format_unit_code11 src/scsi/format_unit_code11.c)
  keil_c_source(scsi_lba_mapping src/scsi/lba_mapping.c)
  keil_c_source(scsi_play_cd src/scsi/play_cd.c)
  keil_c_source(scsi_play_audio src/scsi/play_audio.c)
  keil_c_source(scsi_play_audio_track_index src/scsi/play_audio_track_index.c)
  keil_c_source(scsi_play_track_relative src/scsi/play_track_relative.c)
  keil_c_source(scsi_opcode_ba_cd src/scsi/scsi_opcode_ba_cd.c)
  keil_c_source(scsi_playable_audio_track src/scsi/playable_audio_track.c)
  keil_c_source(scsi_spc src/scsi/spc.c)
  keil_c_source(scsi_mode_page_initialization src/scsi/mode_page_initialization.c)
  keil_c_source(scsi_read_buffer_capacity src/scsi/read_buffer_capacity.c)
  keil_c_source(scsi_read_buffer src/scsi/read_buffer.c)
  keil_c_source(scsi_read_buffer_snapshot src/scsi/read_buffer_snapshot.c)
  keil_c_source(scsi_read_buffer_snapshot_restore
    src/scsi/read_buffer_snapshot_restore.c)
  keil_c_source(scsi_opcode_f0_configuration_update
    src/scsi/opcode_f0_configuration_update.c)
  keil_c_source(configuration_range_update
    src/platform/configuration_range_update.c)
  keil_c_source(scsi_raw_sector_range src/scsi/raw_sector_range.c)
  keil_c_source(scsi_raw_sector_batched_host_transfer
    src/scsi/raw_sector_batched_host_transfer.c)
  keil_c_source(scsi_read_cd_engine src/scsi/read_cd_engine.c)
  keil_c_source(scsi_read_cd_track_range_match
    src/scsi/read_cd_track_range_match.c)
  keil_c_source(scsi_read_format_capacities
    src/scsi/read_format_capacities.c)
  keil_c_source(scsi_write_buffer src/scsi/write_buffer.c)
  keil_c_source(scsi_write_buffer_snapshot_validation
    src/scsi/write_buffer_snapshot_validation.c)
  keil_c_source(scsi_read_disc_information src/scsi/read_disc_information.c)
  keil_c_source(scsi_read_disc_information_recordable
    src/scsi/read_disc_information_recordable.c)
  keil_c_source(scsi_send_cue_sheet src/scsi/send_cue_sheet.c)
  keil_c_source(media_toc_session_removal src/media/toc_session_removal.c)
  keil_c_source(media_toc_state_rebuild src/media/toc_state_rebuild.c)
  keil_c_source(media_toc_session_append src/media/toc_session_append.c)
  keil_c_source(media_toc_state_parse src/media/toc_state_parse.c)
  keil_c_source(read_acquisition_controller_config
    src/platform/read_acquisition_controller_config.c)
  keil_c_source(write_controller_range_sequence
    src/platform/write_controller_range_sequence.c)
  keil_c_source(raw_read_preparation_segments_clone
    src/platform/raw_read_preparation_segments_clone.c)
  keil_c_source(controller_record_workspace_page
    src/platform/controller_record_workspace_page.c)
  keil_c_source(mip_controller_record_snapshot
    src/platform/mip_controller_record_snapshot.c)
  keil_c_source(type2_leadin_toc_sequence
    src/platform/type2_leadin_toc_sequence.c)
  keil_c_source(type2_controller_record_geometry
    src/platform/type2_controller_record_geometry.c)
  keil_c_source(controller_record_page_reconciliation
    src/platform/controller_record_page_reconciliation.c)
  keil_c_source(controller_record_page_header
    src/platform/controller_record_page_header.c)
  keil_c_source(operating_code_transition_finalization
    src/platform/operating_code_transition_finalization.c)
  keil_c_source(type2_detected_layout_prepare
    src/platform/type2_detected_layout_prepare.c)
  keil_c_source(media_rid01_identity_block
    src/media/rid01_identity_block.c)
  keil_c_source(scsi_track_packet_lba_mapping
    src/scsi/track_packet_lba_mapping.c)
  keil_c_source(media_synthetic_raw_read_pattern
    src/media/synthetic_raw_read_pattern.c)
  keil_c_source(media_state6_calibration_threshold
    src/platform/media_state6_calibration_threshold.c)
  keil_c_source(controller_position_sample_acquisition
    src/platform/controller_position_sample_acquisition.c)
  keil_c_source(controller_position_sample_evaluation
    src/platform/controller_position_sample_evaluation.c)
  keil_c_source(controller_state6_position_window
    src/platform/controller_state6_position_window.c)
  keil_c_source(controller_state6_calibration_positioning
    src/platform/controller_state6_calibration_positioning.c)
  keil_c_source(media_state6_controller_calibration_sequence
    src/platform/media_state6_controller_calibration_sequence.c)
  keil_c_source(media_coroutine_state6
    src/platform/media_coroutine_state6.c)
  keil_c_source(drive_state_machine
    src/startup/drive_state_machine.c)
  keil_c_source(indexed_drive_state_transitions
    src/startup/indexed_drive_state_transitions.c)
  keil_c_source(startup_status_coroutine1
    src/startup/startup_status_coroutine1.c)
  keil_c_source(startup_media_state_initialization
    src/startup/startup_media_state_initialization.c)
  keil_c_source(indexed_read_state_configuration
    src/startup/indexed_read_state_configuration.c)
  keil_c_source(controller_calibration_configuration
    src/startup/controller_calibration_configuration.c)
  keil_c_source(media_coroutine_state1
    src/startup/media_coroutine_state1.c)
  keil_c_source(high_write_speed_calibration_flags
    src/platform/high_write_speed_calibration_flags.c)
  keil_c_source(controller_record_page_update
    src/platform/controller_record_page_update.c)
  keil_c_source(scsi_format_unit_code11_probe
    src/scsi/format_unit_code11_probe.c)
  keil_c_source(scsi_format_unit_code11_scan
    src/scsi/format_unit_code11_scan.c)
  keil_c_source(scsi_format_unit_code11_start
    src/scsi/format_unit_code11_start.c)
  keil_c_source(controller_record_type3_refresh
    src/platform/controller_record_type3_refresh.c)
  keil_c_source(controller_record_index_range_acquisition
    src/platform/controller_record_index_range_acquisition.c)
  keil_c_source(controller_record_leadout_acquisition
    src/platform/controller_record_leadout_acquisition.c)
  keil_c_source(controller_record_tdi_acquisition
    src/platform/controller_record_tdi_acquisition.c)
  keil_c_source(controller_record_leadin_acquisition
    src/platform/controller_record_leadin_acquisition.c)
  keil_c_source(controller_record_required_runs
    src/platform/controller_record_required_runs.c)
  keil_c_source(controller_record_secondary_slot_acquisition
    src/platform/controller_record_secondary_slot_acquisition.c)
  keil_c_source(controller_record_primary_slot_acquisition
    src/platform/controller_record_primary_slot_acquisition.c)
  keil_c_source(controller_record_window_acquisition
    src/platform/controller_record_window_acquisition.c)
  keil_c_source(controller_record_base_window_acquisition
    src/platform/controller_record_base_window_acquisition.c)
  keil_step(floating_split C251 floating_split.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/floating_split.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(floating_split.obj)" "PRINT(floating_split.lst)"
    DEPENDS src/platform/floating_split.c src/platform/floating_split.h
      src/platform/mcs251.h
    BYPRODUCTS "${FW_DIR}/floating_split.lst")
  keil_step(floating_floor C251 floating_floor.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/floating_floor.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(floating_floor.obj)" "PRINT(floating_floor.lst)"
    DEPENDS src/platform/floating_floor.c src/platform/floating_floor.h
      src/platform/mcs251.h
    BYPRODUCTS "${FW_DIR}/floating_floor.lst")
  keil_step(float64_runtime_helpers C251 float64_runtime_helpers.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/float64_runtime_helpers.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(float64_runtime_helpers.obj)"
      "PRINT(float64_runtime_helpers.lst)"
    DEPENDS src/platform/float64_runtime_helpers.c
      src/platform/float64_runtime_helpers.h src/platform/mcs251.h
    BYPRODUCTS "${FW_DIR}/float64_runtime_helpers.lst")
  keil_step(floating_runtime_internals C251 floating_runtime_internals.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/floating_runtime_internals.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(floating_runtime_internals.obj)"
      "PRINT(floating_runtime_internals.lst)"
    DEPENDS src/platform/floating_runtime_internals.c
      src/platform/floating_runtime_internals.h src/platform/mcs251.h
    BYPRODUCTS "${FW_DIR}/floating_runtime_internals.lst")
  keil_c_source(float32_runtime_arithmetic
    src/platform/float32_runtime_arithmetic.c)
  keil_step(floating_width_conversions C251 floating_width_conversions.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/floating_width_conversions.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(floating_width_conversions.obj)"
      "PRINT(floating_width_conversions.lst)"
    DEPENDS src/platform/floating_width_conversions.c
      src/platform/floating_width_conversions.h src/platform/mcs251.h
    BYPRODUCTS "${FW_DIR}/floating_width_conversions.lst")
  keil_c_source(integer_float32_conversions
    src/platform/integer_float32_conversions.c)
  keil_c_source(atip_media_strategy src/media/atip_media_strategy.c)
  keil_c_source(controller_register_profile_application
    src/platform/controller_register_profile_application.c)
  keil_step(controller_profile_sample_interpolation C251
    controller_profile_sample_interpolation.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_profile_sample_interpolation.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_profile_sample_interpolation.obj)"
      "PRINT(controller_profile_sample_interpolation.lst)"
    DEPENDS src/platform/controller_profile_sample_interpolation.c
      src/platform/controller_profile_sample_interpolation.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_profile_sample_interpolation.lst")
  keil_c_source(controller_profile_sample_generation
    src/platform/controller_profile_sample_generation.c)
  keil_step(radial_model_profile_coordinate C251
    radial_model_profile_coordinate.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/radial_model_profile_coordinate.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(radial_model_profile_coordinate.obj)"
      "PRINT(radial_model_profile_coordinate.lst)"
    DEPENDS src/platform/radial_model_profile_coordinate.c
      src/platform/radial_model_profile_coordinate.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/radial_model_profile_coordinate.lst")
  keil_step(controller_operating_interpolation_grid_column C251
    controller_operating_interpolation_grid_column.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_operating_interpolation_grid_column.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_operating_interpolation_grid_column.obj)"
      "PRINT(controller_operating_interpolation_grid_column.lst)"
    DEPENDS src/platform/controller_operating_interpolation_grid_column.c
      src/platform/controller_operating_interpolation_grid_column.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_operating_interpolation_grid_column.lst")
  keil_step(controller_operating_interpolation_table_slice C251
    controller_operating_interpolation_table_slice.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_operating_interpolation_table_slice.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_operating_interpolation_table_slice.obj)"
      "PRINT(controller_operating_interpolation_table_slice.lst)"
    DEPENDS src/platform/controller_operating_interpolation_table_slice.c
      src/platform/controller_operating_interpolation_table_slice.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_operating_interpolation_table_slice.lst")
  keil_step(controller_register_profile_generation C251
    controller_register_profile_generation.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_register_profile_generation.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_register_profile_generation.obj)"
      "PRINT(controller_register_profile_generation.lst)"
    DEPENDS src/platform/controller_register_profile_generation.c
      src/platform/controller_register_profile_generation.h
      src/platform/controller_profile_sample_generation.h
      src/platform/radial_model_profile_coordinate.h
      src/platform/controller_parameters.h src/platform/mcs251.h
      src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_register_profile_generation.lst")
  keil_c_source(controller_fixed_profile_generation
    src/platform/controller_fixed_profile_generation.c)
  keil_c_source(controller_current_profile_configuration
    src/platform/controller_current_profile_configuration.c)
  keil_c_source(scsi_get_configuration src/scsi/get_configuration.c)
  keil_c_source(scsi_get_configuration_features
    src/scsi/get_configuration_features.c)
  keil_c_source(scsi_read_full_toc_response src/scsi/read_full_toc_response.c)
  keil_c_source(scsi_get_event_status_notification
    src/scsi/get_event_status_notification.c)
  keil_c_source(scsi_read_master_cue_status src/scsi/read_master_cue_status.c)
  keil_c_source(scsi_read_master_cue_response
    src/scsi/read_master_cue_response.c)
  keil_c_source(scsi_read_toc_dispatch src/scsi/read_toc_dispatch.c)
  keil_c_source(scsi_read_subchannel_isrc_f1
    src/scsi/read_subchannel_isrc_f1.c)
  keil_c_source(scsi_read_subchannel_identification_acquisition
    src/scsi/read_subchannel_identification_acquisition.c)
  keil_c_source(scsi_read_subchannel_current_position
    src/scsi/read_subchannel_current_position.c)
  keil_c_source(scsi_read_subchannel_command src/scsi/read_subchannel_command.c)
  keil_c_source(controller_measurement_series02
    src/platform/controller_measurement_series02.c)
  keil_c_source(controller_measurement_mode6_series05
    src/platform/controller_measurement_mode6_series05.c)
  keil_step(controller_measurement_series05 C251
    controller_measurement_series05.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_measurement_series05.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_measurement_series05.obj)"
      "PRINT(controller_measurement_series05.lst)"
    DEPENDS src/platform/controller_measurement_series05.c
      src/platform/controller_measurement_series05.h src/platform/mcs251.h
      src/platform/firmware_memory.h
      src/platform/controller_measurement_capture.h
      src/platform/status_mapping.h
      src/media/controller_measurement_dispatch.h
    BYPRODUCTS "${FW_DIR}/controller_measurement_series05.lst")
  keil_step(controller_measurement_series04_value C251
    controller_measurement_series04_value.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_measurement_series04_value.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_measurement_series04_value.obj)"
      "PRINT(controller_measurement_series04_value.lst)"
    DEPENDS src/platform/controller_measurement_series04_value.c
      src/platform/controller_measurement_series04_value.h src/platform/mcs251.h
      src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_measurement_series04_value.lst")
  keil_step(controller_measurement_mode6_adjustment C251
    controller_measurement_mode6_adjustment.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_measurement_mode6_adjustment.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_measurement_mode6_adjustment.obj)"
      "PRINT(controller_measurement_mode6_adjustment.lst)"
    DEPENDS src/platform/controller_measurement_mode6_adjustment.c
      src/platform/controller_measurement_mode6_adjustment.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_measurement_mode6_adjustment.lst")
  keil_c_source(controller_measurement_mode2_adjustment
    src/platform/controller_measurement_mode2_adjustment.c)
  keil_step(controller_measurement_mode6_transform C251
    controller_measurement_mode6_transform.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_measurement_mode6_transform.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_measurement_mode6_transform.obj)"
      "PRINT(controller_measurement_mode6_transform.lst)"
    DEPENDS src/platform/controller_measurement_mode6_transform.c
      src/platform/controller_measurement_mode6_transform.h
      src/platform/controller_measurement_mode6_series05.h
      src/platform/controller_measurement_series04_value.h
      src/platform/controller_measurement_series05.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_measurement_mode6_transform.lst")
  keil_c_source(controller_measurement_transform
    src/platform/controller_measurement_transform.c)
  keil_c_source(controller_measurement_interpolation
    src/platform/controller_measurement_interpolation.c)
  keil_c_source(controller_measurement_series00_01
    src/platform/controller_measurement_series00_01.c)
  keil_step(controller_measurement_profile_initialization C251
    controller_measurement_profile_initialization.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_measurement_profile_initialization.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(controller_measurement_profile_initialization.obj)"
      "PRINT(controller_measurement_profile_initialization.lst)"
    DEPENDS src/platform/controller_measurement_profile_initialization.c
      src/platform/controller_measurement_profile_initialization.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_measurement_profile_initialization.lst")
  keil_c_source(controller_measurement_sequence
    src/platform/controller_measurement_sequence.c)
  keil_c_source(controller_measurement_pass
    src/platform/controller_measurement_pass.c)
  keil_c_source(controller_measurement_retry_sequence
    src/platform/controller_measurement_retry_sequence.c)
  keil_c_source(raw_read_position_probe_step
    src/media/raw_read_position_probe_step.c)
  keil_c_source(packed_subchannel_record_batch
    src/media/packed_subchannel_record_batch.c)
  keil_c_source(raw_sector_buffer_event
    src/media/raw_sector_buffer_event.c)
  keil_c_source(raw_sector_completion
    src/media/raw_sector_completion.c)
  keil_c_source(primary_raw_read_producer_position
    src/media/primary_raw_read_producer_position.c)
  keil_c_source(parsed_q_raw_read_producer_position
    src/media/parsed_q_raw_read_producer_position.c)
  keil_c_source(controller_read_event
    src/media/controller_read_event.c)
  keil_c_source(primary_raw_read_operating_code
    src/media/primary_raw_read_operating_code.c)
  keil_c_source(primary_raw_read_pipeline_step
    src/media/primary_raw_read_pipeline_step.c)
  keil_c_source(primary_raw_read_pipeline_coroutine
    src/media/primary_raw_read_pipeline_coroutine.c)
  keil_c_source(raw_read_pipeline_reset
    src/media/raw_read_pipeline_reset.c)
  keil_c_source(scsi_read_formatted_toc_response
    src/scsi/read_formatted_toc_response.c)
  keil_c_source(scsi_read_toc_format1_multisession
    src/scsi/read_toc_format1_multisession.c)
  keil_c_source(scsi_read_toc_format3_pma src/scsi/read_toc_format3_pma.c)
  keil_c_source(scsi_read_toc_format4_atip src/scsi/read_toc_format4_atip.c)
  keil_c_source(scsi_read_toc_cd_text_acquisition
    src/scsi/read_toc_cd_text_acquisition.c)
  keil_c_source(scsi_cd_text_pack_validation
    src/scsi/cd_text_pack_validation.c)
  keil_c_source(scsi_read_toc_format5_response src/scsi/read_toc_format5_response.c)
  keil_c_source(scsi_read_header_acquisition src/scsi/read_header_acquisition.c)
  keil_c_source(scsi_read_cd_track_coverage src/scsi/read_cd_track_coverage.c)
  keil_c_source(scsi_repair_track src/scsi/repair_track.c)
  keil_c_source(scsi_reserve_track src/scsi/reserve_track.c)
  keil_c_source(scsi_send_opc_information src/scsi/send_opc_information.c)
  keil_c_source(startup_debounced_inputs src/startup/debounced_inputs.c)
  keil_c_source(scsi_read_verify_range src/scsi/read_verify_range.c)
  keil_c_source(scsi_read_verify_finalize src/scsi/read_verify_finalize.c)
  keil_c_source(scsi_write_and_verify src/scsi/write_and_verify.c)
  keil_c_source(scsi_raw_sector_host_transfer_poll src/scsi/raw_sector_host_transfer_poll.c)
  keil_c_source(scsi_read_track_information_poll src/scsi/read_track_information_poll.c)
  keil_c_source(scsi_full_toc_track_span src/scsi/full_toc_track_span.c)
  keil_c_source(scsi_read_track_information_size_blocks
    src/scsi/read_track_information_size_blocks.c)
  keil_c_source(scsi_read_track_information_free_blocks
    src/scsi/read_track_information_free_blocks.c)
  keil_c_source(scsi_request_sense src/scsi/request_sense.c)
  keil_c_source(scsi_mmc_write src/scsi/mmc_write.c)
  keil_c_source(scsi_vendor_raw_read src/scsi/vendor_raw_read.c)
  keil_step(scsi_vendor_f3_state C251 scsi_vendor_f3_state.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/scsi/vendor_f3_state.c"
      ${PLEXTOR_C251_OPTIONS}
      "OBJECT(scsi_vendor_f3_state.obj)" "PRINT(scsi_vendor_f3_state.lst)"
    DEPENDS src/scsi/vendor_f3_state.c src/scsi/vendor_f3_state.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/scsi_vendor_f3_state.lst")
  keil_c_source(scsi_mode_sense_pages src/scsi/mode_sense_pages.c)
  keil_c_source(scsi_mode_page2a src/scsi/mode_page2a.c)
  keil_c_source(scsi_mode_sense_response src/scsi/mode_sense_response.c)
  keil_c_source(media_decoder src/media/decoder.c)
  keil_c_source(media_session_boundary src/media/session_boundary.c)
  keil_c_source(media_atip_capacity src/media/atip_capacity.c)
  keil_c_source(media_write_command_raw_read_prefix src/media/write_command_raw_read_prefix.c)
  keil_c_source(media_write_command_raw_read_pipeline src/media/write_command_raw_read_pipeline.c)
  keil_c_source(media_write_preservation_read src/media/write_preservation_read.c)
  keil_c_source(media_raw_read_controller_transfer_batch src/media/raw_read_controller_transfer_batch.c)
  keil_c_source(media_synthetic_write_descriptor_batch src/media/synthetic_write_descriptor_batch.c)
  keil_c_source(media_controller_event_bit7_progress src/media/controller_event_bit7_progress.c)
  keil_c_source(media_controller_q src/media/controller_q.c)
  keil_step(media_controller_measurement_dispatch C251
    media_controller_measurement_dispatch.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/media/controller_measurement_dispatch.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(media_controller_measurement_dispatch.obj)"
      "PRINT(media_controller_measurement_dispatch.lst)"
    DEPENDS src/media/controller_measurement_dispatch.c
      src/media/controller_measurement_dispatch.h src/platform/mcs251.h
      src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/media_controller_measurement_dispatch.lst")
  keil_c_source(media_disc_recognition src/media/disc_recognition.c)
  keil_c_source(media_read_acquisition_distance src/media/read_acquisition_distance.c)
  keil_c_source(media_disc_read_position src/media/disc_read_position.c)
  keil_c_source(media_radial_step_scale_calibration src/media/radial_step_scale_calibration.c)
  keil_c_source(media_read_acquisition_radial_distance src/media/read_acquisition_radial_distance.c)
  keil_c_source(media_operating_code_selection src/media/operating_code_selection.c)
  keil_c_source(media_read_acquisition_request src/media/read_acquisition_request.c)
  keil_c_source(media_raw_read_descriptor_completion src/media/raw_read_descriptor_completion.c)
  keil_c_source(media_raw_read_header_normalization src/media/raw_read_header_normalization.c)
  keil_c_source(media_raw_sector_header_inspect src/media/raw_sector_header_inspect.c)
  keil_c_source(media_raw_sector_descriptor_status src/media/raw_sector_descriptor_status.c)
  keil_c_source(media_raw_sector_transfer_setup src/media/raw_sector_transfer_setup.c)
  keil_c_source(media_raw_sector_transfer src/media/raw_sector_transfer.c)
  keil_c_source(media_raw_read_runtime src/media/raw_read_runtime.c)
  keil_c_source(media_raw_read_frame_sequence src/media/raw_read_frame_sequence.c)
  keil_c_source(media_raw_read_ring_alignment src/media/raw_read_ring_alignment.c)
  keil_c_source(media_premium_raw_batch src/media/premium_raw_batch.c)
  keil_c_source(media_fixed_packet_padding src/media/fixed_packet_padding.c)
  keil_c_source(media_raw_read_range_reset src/media/raw_read_range_reset.c)
  keil_c_source(media_calibration_offset src/media/calibration_offset.c)
  keil_c_source(media_raw_read_preparation_range src/media/raw_read_preparation_range.c)
  keil_c_source(media_raw_read_acquisition_plan
    src/media/raw_read_acquisition_plan.c)
  keil_c_source(media_raw_read_range_five_frame_preroll
    src/media/raw_read_range_five_frame_preroll.c)
  keil_c_source(media_raw_read_q_record_queue
    src/media/raw_read_q_record_queue.c)
  keil_c_source(media_raw_read_q_record_table_build
    src/media/raw_read_q_record_table_build.c)
  keil_c_source(media_format_operation_raw_read_state
    src/media/format_operation_raw_read_state.c)
  keil_c_source(media_reserve_track_raw_read_pipeline
    src/media/reserve_track_raw_read_pipeline.c)
  keil_c_source(media_current_mode_raw_read_pipeline
    src/media/current_mode_raw_read_pipeline.c)
  keil_c_source(media_raw_read_transfer_batch_service
    src/media/raw_read_transfer_batch_service.c)
  keil_c_source(media_raw_read_toc_summary
    src/media/raw_read_toc_summary.c)
  keil_c_source(media_raw_read_interleaved_q_batch
    src/media/raw_read_interleaved_q_batch.c)
  keil_c_source(media_raw_read_q_record_sector_acquisition
    src/media/raw_read_q_record_sector_acquisition.c)
  keil_c_source(media_blank_format_raw_read_plan
    src/media/blank_format_raw_read_plan.c)
  keil_c_source(media_raw_read_operation_mode_speed
    src/media/raw_read_operation_mode_speed.c)
  keil_c_source(media_drive_operation_result_coroutine
    src/media/drive_operation_result_coroutine.c)
  if(PLEXTOR_LED_BOOT_DIAGNOSTICS)
    keil_step(scsi_command_processor_coroutine C251
      scsi_command_processor_coroutine.obj
      ARGS "${PROJECT_SOURCE_DIR}/src/scsi/command_processor_coroutine.c"
        ${PLEXTOR_C251_OPTIONS} "DEFINE(PLEXTOR_LED_BOOT_DIAGNOSTICS)"
        "OBJECT(scsi_command_processor_coroutine.obj)"
        "PRINT(scsi_command_processor_coroutine.lst)"
      DEPENDS src/scsi/command_processor_coroutine.c
        src/boot/led_boot_diagnostics.h src/platform/mcs251.h
        src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/scsi_command_processor_coroutine.lst")
  else()
    keil_c_source(scsi_command_processor_coroutine
      src/scsi/command_processor_coroutine.c)
  endif()
  if(PLEXTOR_LED_BOOT_DIAGNOSTICS)
    keil_step(platform_host_interface_interrupt C251 platform_host_interface_interrupt.obj
      ARGS "${PROJECT_SOURCE_DIR}/src/platform/host_interface_interrupt.c"
        ${PLEXTOR_C251_OPTIONS} "DEFINE(PLEXTOR_LED_BOOT_DIAGNOSTICS)"
        "OBJECT(platform_host_interface_interrupt.obj)"
        "PRINT(platform_host_interface_interrupt.lst)"
      DEPENDS src/platform/host_interface_interrupt.c
        src/boot/led_boot_diagnostics.h src/platform/mcs251.h
        src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/platform_host_interface_interrupt.lst")
  else()
    keil_c_source(platform_host_interface_interrupt
      src/platform/host_interface_interrupt.c)
  endif()
  keil_c_source(platform_controller_operating_code_interpolation_tables
    src/platform/controller_operating_code_interpolation_tables.c
    FLOAT64)
  keil_c_source(media_repeated_raw_read_range_windows
    src/media/repeated_raw_read_range_windows.c)
  keil_c_source(media_raw_read_transfer_layout
    src/media/raw_read_transfer_layout.c)
  keil_c_source(media_raw_read_track_record_advance
    src/media/raw_read_track_record_advance.c)
  keil_c_source(media_working_track_raw_read_range
    src/media/working_track_raw_read_range.c)
  keil_c_source(media_raw_read_frame_prefix_wait
    src/media/raw_read_frame_prefix_wait.c)
  keil_c_source(media_raw_sector_sequence src/media/raw_sector_sequence.c)
  keil_c_source(media_lead_in_raw_sector_descriptor_sequence
    src/media/lead_in_raw_sector_descriptor_sequence.c)
  keil_c_source(media_lead_out_raw_sector_descriptor_sequence
    src/media/lead_out_raw_sector_descriptor_sequence.c)
  keil_c_source(media_lead_in_out_descriptor_sequences
    src/media/lead_in_out_descriptor_sequences.c)
  keil_c_source(media_raw_read_preparation_range_end
    src/media/raw_read_preparation_range_end.c)
  keil_c_source(media_raw_track_selection src/media/raw_track_selection.c)
  keil_c_source(media_state3_countdown src/media/state3_countdown.c)
  keil_c_source(media_coroutine_state5 src/media/coroutine_state5.c)
  keil_c_source(media_state3_indexed_transitions src/media/state3_indexed_transitions.c)
  keil_c_source(media_state5_read_acquisition_sequence
    src/media/state5_read_acquisition_sequence.c)
  keil_c_source(media_read_acquisition_state
    src/media/read_acquisition_state.c)
  keil_c_source(media_state_coroutine_dispatch src/media/state_coroutine_dispatch.c)
  keil_c_source(media_state5_position_window src/media/state5_position_window.c)
  keil_c_source(media_write_pipeline_q src/media/write_pipeline_q.c)
  keil_c_source(media_write_pipeline_transfer_batch_poll
    src/media/write_pipeline_transfer_batch_poll.c)
  keil_c_source(media_write_command_transfer_pipeline
    src/media/write_command_transfer_pipeline.c)
  keil_c_source(media_raw_read_q_record_table_population
    src/media/raw_read_q_record_table_population.c)
  keil_c_source(media_synthetic_raw_read_q_record_table
    src/media/synthetic_raw_read_q_record_table.c)
  keil_c_source(media_write_pipeline_finalize src/media/write_pipeline_finalize.c)
  keil_c_source(media_post_toc_controller_state src/media/post_toc_controller_state.c)
  keil_c_source(media_pma_q_record_reset src/media/pma_q_record_reset.c)
  keil_c_source(media_synthetic_raw_read_state src/media/synthetic_raw_read_state.c)
  keil_c_source(media_synthetic_track_table src/media/synthetic_track_table.c)
  keil_c_source(media_close_track_sync_operation src/media/close_track_sync_operation.c)
  keil_c_source(media_close_sync_raw_read_prepare
    src/media/close_sync_raw_read_prepare.c)
  keil_c_source(media_configured_raw_read_preparation_segments
    src/media/configured_raw_read_preparation_segments.c)
  keil_c_source(media_read_transition_coroutine src/media/read_transition_coroutine.c)
  keil_c_source(media_raw_read_position_probe src/media/raw_read_position_probe.c)
  keil_c_source(media_raw_read_position_validation src/media/raw_read_position_validation.c)
  keil_c_source(media_raw_read_position_tracker src/media/raw_read_position_tracker.c)
  keil_c_source(media_fixed_packet_overhead src/media/fixed_packet_overhead.c)
  keil_c_source(media_primary_raw_read_capture_batch src/media/primary_raw_read_capture_batch.c)
  keil_c_source(media_strategy_profile_initialization src/media/strategy_profile_initialization.c)
  keil_c_source(scsi_send_diagnostic src/scsi/send_diagnostic.c)
  keil_c_source(scsi_start_stop_unit src/scsi/start_stop_unit.c)
  keil_c_source(scsi_format_unit src/scsi/format_unit.c)
  keil_c_source(scsi_format_unit_track_state src/scsi/format_unit_track_state.c)
  keil_c_source(scsi_format_plan_q src/scsi/format_plan_q.c)
  keil_c_source(scsi_indexed_state0f_coroutine src/scsi/indexed_state0f_coroutine.c)
  keil_c_source(scsi_synchronize_cache10 src/scsi/synchronize_cache10.c)
  keil_c_source(scsi_close_track_session src/scsi/close_track_session.c)
  keil_c_source(scsi_write_command_range src/scsi/write_command_range.c)
  keil_step(scsi_vendor_f2_shared C251 scsi_vendor_f2_shared.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/scsi/vendor_f2_shared.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(scsi_vendor_f2_shared.obj)"
      "PRINT(scsi_vendor_f2_shared.lst)"
    DEPENDS src/scsi/vendor_f2_shared.c src/scsi/vendor_f2_shared.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/scsi_vendor_f2_shared.lst")
  keil_c_source(scsi_vendor_f3_controller_mode
    src/scsi/vendor_f3_controller_mode.c)
  keil_c_source(scsi_vendor_f3_measurement_response
    src/scsi/vendor_f3_measurement_response.c)
  keil_c_source(scsi_vendor_f5_media_strategy_response
    src/scsi/vendor_f5_media_strategy_response.c)
  keil_c_source(scsi_vendor_e9 src/scsi/vendor_e9.c)
  keil_c_source(scsi_vendor_eb src/scsi/vendor_eb.c)
  keil_c_source(scsi_vendor_ec_record_list
    src/scsi/vendor_ec_record_list.c)
  keil_c_source(scsi_vendor_f3 src/scsi/vendor_f3.c)
  keil_c_source(scsi_vendor_f5 src/scsi/vendor_f5.c)
  keil_c_source(scsi_write_command_transfer
    src/scsi/write_command_transfer.c)
  keil_c_source(scsi_write_transfer_range src/scsi/write_transfer_range.c)
  keil_c_source(scsi_write_pipeline_prepare src/scsi/write_pipeline_prepare.c)
  keil_c_source(scsi_write_partial_range src/scsi/write_partial_range.c)
  keil_c_source(scsi_write_record_prepare_predicate
    src/scsi/write_record_prepare_predicate.c)
  keil_c_source(media_raw_read_speed_update
    src/media/raw_read_speed_update.c)
  keil_c_source(media_raw_read_pattern_validation
    src/media/raw_read_pattern_validation.c)
  keil_c_source(media_raw_read_pattern_acquisition
    src/media/raw_read_pattern_acquisition.c)
  keil_c_source(media_blank_format_track_state
    src/media/blank_format_track_state.c)
  keil_c_source(media_type2_mip_geometry src/media/type2_mip_geometry.c)
  keil_c_source(scsi_get_performance src/scsi/get_performance.c)
  keil_c_source(scsi_set_cd_speed src/scsi/set_cd_speed.c)
  keil_c_source(scsi_blank src/scsi/blank.c)
  keil_c_source(scsi_mode_select src/scsi/mode_select.c)
  keil_c_source(media_active_write_pipeline src/media/active_write_pipeline.c)
  keil_c_source(media_raw_q_position_target_probe src/media/raw_q_position_target_probe.c)
  keil_c_source(media_raw_payload_signature src/media/raw_payload_signature.c)
  keil_c_source(arithmetic src/platform/arithmetic.c)
  keil_c_source(integer_division src/platform/integer_division.c)
  keil_c_source(encoding src/platform/encoding.c)
  keil_c_source(memory src/platform/memory.c)
  keil_c_source(timing src/platform/timing.c)
  keil_c_source(timing_worker src/platform/timing_target.c)
  keil_c_source(timing_wait src/platform/timing_wait.c)
  keil_c_source(noop_handlers src/platform/noop_handlers.c)
  keil_step(coroutine_entries C251 coroutine_entries.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/coroutine_entries.c"
      ${PLEXTOR_C251_OPTIONS}
      "OBJECT(coroutine_entries.obj)" "PRINT(coroutine_entries.lst)"
    DEPENDS src/platform/coroutine_entries.c src/platform/coroutine_entries.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/coroutine_entries.lst")
  keil_step(floating_result_tails C251 floating_result_tails.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/floating_result_tails.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(floating_result_tails.obj)" "PRINT(floating_result_tails.lst)"
    DEPENDS src/platform/floating_result_tails.c
      src/platform/floating_result_tails.h src/platform/mcs251.h
    BYPRODUCTS "${FW_DIR}/floating_result_tails.lst")
  keil_c_source(drive_control_reset src/platform/drive_control_reset.c)
  keil_step(controller_buffer_atomic C251 controller_buffer_atomic.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_buffer_atomic.c"
      ${PLEXTOR_C251_OPTIONS}
      "OBJECT(controller_buffer_atomic.obj)" "PRINT(controller_buffer_atomic.lst)"
    DEPENDS src/platform/controller_buffer_atomic.c
      src/platform/controller_buffer_atomic.h
      src/platform/bindings/platform/controller_buffer_atomic_bindings.h
      src/platform/mcs251.h
    BYPRODUCTS "${FW_DIR}/controller_buffer_atomic.lst")
  keil_c_source(controller_buffer_transfer src/platform/controller_buffer_transfer.c)
  keil_c_source(controller_buffer_chunk_fill src/platform/controller_buffer_chunk_fill.c)
  keil_c_source(buffer_transfer_setup src/platform/buffer_transfer_setup.c)
  if(PLEXTOR_LED_BOOT_DIAGNOSTICS)
    keil_step(controller_data_out C251 controller_data_out.obj
      ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_data_out.c"
        ${PLEXTOR_C251_OPTIONS} "DEFINE(PLEXTOR_LED_BOOT_DIAGNOSTICS)"
        "OBJECT(controller_data_out.obj)" "PRINT(controller_data_out.lst)"
      DEPENDS src/platform/controller_data_out.c
        src/platform/controller_data_out.h src/boot/led_boot_diagnostics.h
        src/platform/mcs251.h src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/controller_data_out.lst")
  else()
    keil_c_source(controller_data_out src/platform/controller_data_out.c)
  endif()
  keil_c_source(controller_command03 src/platform/controller_command03.c)
  keil_c_source(controller_command_c0_calibration src/platform/controller_command_c0_calibration.c)
  keil_c_source(controller_sample_series src/platform/controller_sample_series.c)
  keil_c_source(frame_radial_float src/platform/frame_radial_float.c)
  keil_step(operating_code_measurement_bounds C251
    operating_code_measurement_bounds.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/operating_code_measurement_bounds.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(operating_code_measurement_bounds.obj)"
      "PRINT(operating_code_measurement_bounds.lst)"
    DEPENDS src/platform/operating_code_measurement_bounds.c
      src/platform/operating_code_measurement_bounds.h src/platform/mcs251.h
      src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/operating_code_measurement_bounds.lst")
  keil_step(operating_code14_measurement_bounds C251
    operating_code14_measurement_bounds.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/operating_code14_measurement_bounds.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(operating_code14_measurement_bounds.obj)"
      "PRINT(operating_code14_measurement_bounds.lst)"
    DEPENDS src/platform/operating_code14_measurement_bounds.c
      src/platform/operating_code14_measurement_bounds.h src/platform/mcs251.h
      src/platform/firmware_memory.h
      src/platform/operating_code_measurement_bounds.h
      src/media/controller_measurement_dispatch.h
    BYPRODUCTS "${FW_DIR}/operating_code14_measurement_bounds.lst")
  keil_c_source(controller_record_boundaries src/platform/controller_record_boundaries.c)
  keil_c_source(controller_operating_adjustment_response src/platform/controller_operating_adjustment_response.c)
  keil_c_source(controller_state_profile src/platform/controller_state_profile.c)
  keil_c_source(indexed_state_operation src/platform/indexed_state_operation.c)
  keil_c_source(radial_pickup_parameter_sequence src/platform/radial_pickup_parameter_sequence.c)
  keil_c_source(radial_pickup_restore src/platform/radial_pickup_restore.c)
  keil_c_source(scaled_radial_pickup_move_sequence
    src/platform/scaled_radial_pickup_move_sequence.c)
  keil_c_source(controller_transition_wait src/platform/controller_transition_wait.c)
  keil_c_source(controller_operating_interpolation src/platform/controller_operating_interpolation.c)
  keil_c_source(firmware_programmer_handoff src/platform/firmware_programmer_handoff.c)
  keil_c_source(format_unit_type2_pipeline src/platform/format_unit_type2_pipeline.c)
  keil_c_source(read_acquisition_request_dispatch src/platform/read_acquisition_request_dispatch.c)
  keil_c_source(raw_read_opc src/platform/raw_read_opc.c)
  keil_step(interrupt_control A251 interrupt_control.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/interrupt_control.a51"
      ${PLEXTOR_A251_OPTIONS}
      "OBJECT(interrupt_control.obj)" "PRINT(interrupt_control.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/interrupt_control.a51"
    BYPRODUCTS "${FW_DIR}/interrupt_control.lst")
  keil_c_source(timer_debounce src/platform/timer_debounce.c)
  keil_c_source(timer_phase0 src/platform/timer_phase0.c)
  keil_c_source(timer_phase2 src/platform/timer_phase2.c)
  keil_c_source(sfr_da_db_measurement src/platform/sfr_da_db_measurement.c)
  keil_c_source(controller_mapping src/platform/controller_mapping.c)
  keil_c_source(controller_progress src/platform/controller_progress.c)
  keil_c_source(controller_progress_transition src/platform/controller_progress_transition.c)
  keil_c_source(controller_operating_range src/platform/controller_operating_range.c)
  keil_c_source(controller_operating_window src/platform/controller_operating_window.c)
  keil_c_source(media_state_updates src/platform/media_state_updates.c)
  keil_c_source(controller_calibration src/platform/controller_calibration.c)
  keil_c_source(controller_calibration_outcome src/platform/controller_calibration_outcome.c)
  keil_c_source(controller_parameters src/platform/controller_parameters.c)
  keil_c_source(controller_parameter_f4_calibration src/platform/controller_parameter_f4_calibration.c)
  keil_c_source(controller_operating_code_configuration src/platform/controller_operating_code_configuration.c)
  keil_c_source(controller_operating_code_ports
    src/platform/controller_operating_code_ports.c)
  keil_c_source(controller_threshold_configuration
    src/platform/controller_threshold_configuration.c)
  keil_c_source(controller_word0319_offset_calibration
    src/platform/controller_word0319_offset_calibration.c)
  keil_c_source(controller_words0204_0319_calibration
    src/platform/controller_words0204_0319_calibration.c)
  keil_step(controller_p1_bit4 C251 controller_p1_bit4.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/controller_p1_bit4.c"
      ${PLEXTOR_C251_OPTIONS}
      "OBJECT(controller_p1_bit4.obj)" "PRINT(controller_p1_bit4.lst)"
    DEPENDS src/platform/controller_p1_bit4.c src/platform/controller_parameters.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/controller_p1_bit4.lst")
  keil_c_source(pickup_move_outputs src/platform/pickup_move_outputs.c)
  keil_c_source(controller_profiles src/platform/controller_profiles.c)
  keil_c_source(controller_profile_selector
    src/platform/controller_profile_selector.c)
  if(PLEXTOR_LED_BOOT_DIAGNOSTICS)
    keil_step(coroutine_initialization C251 coroutine_initialization.obj
      ARGS "${PROJECT_SOURCE_DIR}/src/platform/coroutine_initialization.c"
        ${PLEXTOR_C251_OPTIONS} "DEFINE(PLEXTOR_LED_BOOT_DIAGNOSTICS)"
        "OBJECT(coroutine_initialization.obj)"
        "PRINT(coroutine_initialization.lst)"
      DEPENDS src/platform/coroutine_initialization.c
        src/boot/led_boot_diagnostics.h src/platform/mcs251.h
        src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/coroutine_initialization.lst")
  else()
    keil_c_source(coroutine_initialization src/platform/coroutine_initialization.c)
  endif()
  keil_c_source(firmware_entry src/platform/firmware_entry.c)
  keil_step(power_control A251 power_control.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/power_control.a51"
      ${PLEXTOR_A251_OPTIONS}
      "OBJECT(power_control.obj)" "PRINT(power_control.lst)"
    DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/power_control.a51"
    BYPRODUCTS "${FW_DIR}/power_control.lst")
  keil_c_source(read_measurement_mode1 src/platform/read_measurement_mode1.c)
  keil_c_source(read_acquisition_controller_init src/platform/read_acquisition_controller_init.c)
  keil_c_source(read_acquisition_coroutine3 src/platform/read_acquisition_coroutine3.c)
  keil_c_source(controller_startup_mode src/platform/controller_startup_mode.c)
  keil_c_source(controller_asic_initialization src/platform/controller_asic_initialization.c)
  keil_c_source(drive_firmware_initialization
    src/startup/drive_firmware_initialization.c)
  if(PLEXTOR_LED_BOOT_DIAGNOSTICS OR PLEXTOR_LED_STARTUP_DIAGNOSTICS)
    keil_c_source(led_boot_diagnostics src/boot/led_boot_diagnostics.c)
  endif()
  if(PLEXTOR_LED_STARTUP_DIAGNOSTICS)
    keil_c_source(startup_progress_diagnostics src/boot/startup_progress_diagnostics.c)
  endif()
  keil_c_source(drive_configuration_state src/startup/drive_configuration_state.c)
  keil_c_source(disc_control_state_initialization src/startup/disc_control_state_initialization.c)
  keil_c_source(runtime_drive_state src/platform/runtime_drive_state.c)
  keil_c_source(inquiry_configuration_refresh src/scsi/inquiry_configuration_refresh.c)
  keil_c_source(command_admission src/scsi/command_admission.c)
  keil_c_source(active_media_operation_admission src/scsi/active_media_operation_admission.c)
  keil_c_source(task_file_command_dispatch src/ata/task_file_command_dispatch.c)
  keil_c_source(host_command_dispatch src/ata/host_command_dispatch.c)
  keil_c_source(identify_packet_device src/ata/identify_packet_device.c)
  keil_c_source(type2_mip_state src/platform/type2_mip_state.c)
  if(PLEXTOR_LED_BOOT_DIAGNOSTICS)
    keil_step(status_mapping C251 status_mapping.obj
      ARGS "${PROJECT_SOURCE_DIR}/src/platform/status_mapping.c"
        ${PLEXTOR_C251_OPTIONS} "DEFINE(PLEXTOR_LED_BOOT_DIAGNOSTICS)"
        "OBJECT(status_mapping.obj)" "PRINT(status_mapping.lst)"
      DEPENDS src/platform/status_mapping.c src/platform/status_mapping.h
        src/boot/led_boot_diagnostics.h src/platform/mcs251.h
        src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/status_mapping.lst")
  else()
    keil_step(status_mapping C251 status_mapping.obj
      ARGS "${PROJECT_SOURCE_DIR}/src/platform/status_mapping.c"
        ${PLEXTOR_C251_OPTIONS}
        "OBJECT(status_mapping.obj)" "PRINT(status_mapping.lst)"
      DEPENDS src/platform/status_mapping.c src/platform/status_mapping.h
        src/platform/mcs251.h src/platform/firmware_memory.h
      BYPRODUCTS "${FW_DIR}/status_mapping.lst")
  endif()
  keil_c_source(write_speed_reduction src/platform/write_speed_reduction.c)
  keil_c_source(controller_threshold_probe src/platform/controller_threshold_probe.c)
  keil_c_source(controller_threshold_sampling src/platform/controller_threshold_sampling.c)
  keil_c_source(configuration_runtime_update src/platform/configuration_runtime_update.c)
  keil_c_source(indexed_state_subsystem src/platform/indexed_state_subsystem.c)
  keil_c_source(radial_model src/platform/radial_model.c)
  keil_step(radial_refinement C251 radial_refinement.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/radial_refinement.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(radial_refinement.obj)" "PRINT(radial_refinement.lst)"
    DEPENDS src/platform/radial_refinement.c src/platform/radial_refinement.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/radial_refinement.lst")
  keil_c_source(disc_radial_calibration
    src/platform/disc_radial_calibration.c)
  keil_c_source(controller_position_average
    src/platform/controller_position_average.c)
  keil_step(radial_position_table C251 radial_position_table.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/radial_position_table.c"
      ${PLEXTOR_C251_OPTIONS} FLOAT64
      "OBJECT(radial_position_table.obj)" "PRINT(radial_position_table.lst)"
    DEPENDS src/platform/radial_position_table.c
      src/platform/radial_position_table.h src/platform/mcs251.h
      src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/radial_position_table.lst")
  keil_c_source(controller_q_packet_process
    src/platform/controller_q_packet_process.c)
  keil_c_source(controller_measurement_reference
    src/platform/controller_measurement_reference.c)
  keil_c_source(controller_measurement_transition
    src/platform/controller_measurement_transition.c)
  keil_c_source(pickup_move_controller_sequence
    src/platform/pickup_move_controller_sequence.c)
  keil_c_source(controller_event_dispatch
    src/platform/controller_event_dispatch.c)
  keil_c_source(controller_sfr_drain src/platform/controller_sfr_drain.c)
  keil_c_source(controller_word0251_sweep
    src/platform/controller_word0251_sweep.c)
  keil_c_source(decoded_position_wait src/platform/decoded_position_wait.c)
  keil_c_source(indexed_state_dispatch src/platform/indexed_state_dispatch.c)
  keil_c_source(write_controller_range src/platform/write_controller_range.c)
  keil_c_source(write_controller_range_build
    src/platform/write_controller_range_build.c)
  keil_c_source(controller_record_bank_scan
    src/platform/controller_record_bank_scan.c)
  keil_c_source(controller_record_find
    src/platform/controller_record_find.c)
  keil_c_source(controller_record_page_compaction
    src/platform/controller_record_page_compaction.c)
  keil_c_source(controller_record_region_scan
    src/platform/controller_record_region_scan.c)
  keil_c_source(controller_parameter_calibration_search
    src/platform/controller_parameter_calibration_search.c)
  keil_c_source(controller_parameter_fc_calibration
    src/platform/controller_parameter_fc_calibration.c)
  keil_c_source(controller_record_slot_preparation src/platform/controller_record_slot_preparation.c)
  keil_c_source(media_state6_lead_in_reference src/platform/media_state6_lead_in_reference.c)
  keil_c_source(working_track_probe_state src/platform/working_track_probe_state.c)
  keil_c_source(timed_output_sequence src/platform/timed_output_sequence.c)
  keil_c_source(eight_channel_capture src/platform/eight_channel_capture.c)
  keil_c_source(toc_workspace src/platform/toc_workspace.c)
  keil_c_source(raw_read_buffer_setup src/platform/raw_read_buffer_setup.c)
  keil_c_source(raw_read_capacity src/platform/raw_read_capacity.c)
  keil_c_source(raw_q_record_decode src/platform/raw_q_record_decode.c)
  keil_c_source(raw_read_q_control src/platform/raw_read_q_control.c)
  keil_c_source(raw_sector_capture src/platform/raw_sector_capture.c)
  keil_c_source(raw_sector_descriptor_scratch
    src/platform/raw_sector_descriptor_scratch.c)
  keil_c_source(recovery_wait src/platform/recovery_wait.c)
  keil_c_source(read_measurement_wait src/platform/read_measurement_wait.c)
  keil_c_source(controller_measurement_capture src/platform/controller_measurement_capture.c)
  keil_c_source(controller_d8d9_window src/platform/controller_d8d9_window.c)
  keil_c_source(startup_status_routing src/startup/status_routing.c)
  keil_c_source(drive_control_state10 src/startup/drive_control_state10.c)
  keil_c_source(controller_measurement_window src/platform/controller_measurement_window.c)
  keil_c_source(controller_measurement_range src/platform/controller_measurement_range.c)
  keil_c_source(controller_packet_capture src/platform/controller_packet_capture.c)
  keil_c_source(controller_measurement_recovery src/platform/controller_measurement_recovery.c)
  keil_c_source(controller_state6_operating_path src/platform/controller_state6_operating_path.c)
  keil_c_source(controller_current_state_calibration src/platform/controller_current_state_calibration.c)
  keil_c_source(controller_calibration_path_validation src/platform/controller_calibration_path_validation.c)
  keil_c_source(controller_operating_state_initialization src/platform/controller_operating_state_initialization.c)
  keil_c_source(controller_operating_state_reset src/platform/controller_operating_state_reset.c)
  keil_c_source(controller_port80f8_calibration_attempt src/platform/controller_port80f8_calibration_attempt.c)
  keil_c_source(controller_state6_sample_calibration src/platform/controller_state6_sample_calibration.c)
  keil_c_source(controller_state6_calibration_finalization src/platform/controller_state6_calibration_finalization.c)
  keil_c_source(controller_event_bit2_progress src/platform/controller_event_bit2_progress.c)
  keil_c_source(controller_buffer_setup src/platform/controller_buffer_setup.c)
  keil_c_source(atip_initialization src/media/atip_initialization.c)
  keil_c_source(pma_acquisition src/media/pma_acquisition.c)
  keil_c_source(read_acquisition_calibration_sequence src/platform/read_acquisition_calibration_sequence.c)
  keil_c_source(write_transfer_cursor_alignment src/platform/write_transfer_cursor_alignment.c)
  keil_step(leaf_workers C251 leaf_workers.obj
    ARGS "${PROJECT_SOURCE_DIR}/src/platform/leaf_workers.c"
      ${PLEXTOR_C251_OPTIONS}
      "OBJECT(leaf_workers.obj)" "PRINT(leaf_workers.lst)"
    DEPENDS src/platform/leaf_workers.c src/platform/leaf_workers.h
      src/platform/mcs251.h src/platform/firmware_memory.h
    BYPRODUCTS "${FW_DIR}/leaf_workers.lst")
  keil_c_source(track_state src/platform/track_state.c)
  # keil_step assembles this manifest from every preceding C251/A251 object.
  keil_step(link L251 main.abs
    ARGS "@REGISTERED_OBJECTS@" TO main.abs ${PLEXTOR_L251_OPTIONS}
      "PRINT(main.map)"
    BYPRODUCTS "${FW_DIR}/main.map")
  add_custom_command(OUTPUT "${FW_DIR}/absolute-data-audit.stamp"
    COMMAND "${Python3_EXECUTABLE}"
      "${PROJECT_SOURCE_DIR}/tools/audit_keil_absolute_data.py"
      "${FW_DIR}/main.map" --stamp "${FW_DIR}/absolute-data-audit.stamp"
    DEPENDS "${FW_DIR}/main.abs" "${FW_DIR}/main.map"
      tools/audit_keil_absolute_data.py VERBATIM)
  keil_step(hex OH251 main.hex
    ARGS main.abs "HEXFILE(main.hex)" ${PLEXTOR_OH251_OPTIONS}
    DEPENDS "${FW_DIR}/main.abs" "${FW_DIR}/absolute-data-audit.stamp")

  add_custom_target(firmware_link
    DEPENDS "${FW_DIR}/main.hex" "${FW_DIR}/absolute-data-audit.stamp")
  if(PLEXTOR_FIRMWARE_DIAGNOSTIC_LAYOUT)
    # The diagnostic layout intentionally extends ECODE through RAM:ff0000,
    # so it is linkable but cannot be packed into a flashable image without
    # overwriting the recovery/monitor hole.
    add_custom_target(firmware ALL)
    add_dependencies(firmware firmware_link)
  else()
    if(PLEXTOR_LED_HOST_GATE_DIAGNOSTICS)
      if(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "phases")
        set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_host_phase_diag.bin")
      elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "delay")
        set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_host_delay_diag.bin")
      elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "route")
        set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_host_route_diag.bin")
      elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "interval")
        set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_host_interval_diag.bin")
      elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "zero")
        set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_host_zero_diag.bin")
      elseif(PLEXTOR_HOST_GATE_SNAPSHOT STREQUAL "packet")
        set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_host_packet_diag.bin")
      else()
        set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_host_gate_diag.bin")
      endif()
      set(PLEXTOR_FIRMWARE_VALIDATION "${FW_DIR}/host-gate-diagnostic-validation.json")
      set(PLEXTOR_FIRMWARE_UPDATE_PLAN "${FW_DIR}/host-gate-diagnostic-update-plan.json")
    elseif(PLEXTOR_LED_FOREGROUND_DIAGNOSTICS)
      set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_foreground_diag.bin")
      set(PLEXTOR_FIRMWARE_VALIDATION "${FW_DIR}/foreground-diagnostic-validation.json")
      set(PLEXTOR_FIRMWARE_UPDATE_PLAN "${FW_DIR}/foreground-diagnostic-update-plan.json")
    elseif(PLEXTOR_LED_STARTUP_DIAGNOSTICS)
      set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_startup_diag.bin")
      set(PLEXTOR_FIRMWARE_VALIDATION "${FW_DIR}/startup-diagnostic-validation.json")
      set(PLEXTOR_FIRMWARE_UPDATE_PLAN "${FW_DIR}/startup-diagnostic-update-plan.json")
    elseif(PLEXTOR_LED_BOOT_DIAGNOSTICS)
      set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed_led_diag.bin")
      set(PLEXTOR_FIRMWARE_VALIDATION "${FW_DIR}/led-diagnostic-validation.json")
      set(PLEXTOR_FIRMWARE_UPDATE_PLAN "${FW_DIR}/led-diagnostic-update-plan.json")
    else()
      set(PLEXTOR_PACKAGED_FIRMWARE "${FW_DIR}/MONT_reconstructed.bin")
      set(PLEXTOR_FIRMWARE_VALIDATION "${FW_DIR}/validation.json")
      set(PLEXTOR_FIRMWARE_UPDATE_PLAN "${FW_DIR}/firmware-update-plan.json")
    endif()
    add_custom_command(OUTPUT "${PLEXTOR_PACKAGED_FIRMWARE}"
      COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/firmware_image.py"
        pack "${FW_DIR}/main.hex" "${PLEXTOR_PACKAGED_FIRMWARE}"
        --code-only --fixed-data-from "${FIRMWARE_REFERENCE}"
      DEPENDS "${FW_DIR}/main.hex" "${FIRMWARE_REFERENCE}"
        tools/firmware_image.py VERBATIM)
    add_custom_command(OUTPUT "${PLEXTOR_FIRMWARE_VALIDATION}"
      COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/firmware_image.py"
        verify "${PLEXTOR_PACKAGED_FIRMWARE}"
        --fixed-data-from "${FIRMWARE_REFERENCE}"
        --report "${PLEXTOR_FIRMWARE_VALIDATION}"
      DEPENDS "${PLEXTOR_PACKAGED_FIRMWARE}"
        tools/firmware_image.py VERBATIM)
    add_custom_command(OUTPUT "${PLEXTOR_FIRMWARE_UPDATE_PLAN}"
      COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tools/firmware_update_plan.py"
        --image "${PLEXTOR_PACKAGED_FIRMWARE}"
        --output "${PLEXTOR_FIRMWARE_UPDATE_PLAN}"
        --quiet
      DEPENDS "${PLEXTOR_PACKAGED_FIRMWARE}"
        "${PROJECT_SOURCE_DIR}/data/W5224A104/PlexFirm.ini"
        "${PROJECT_SOURCE_DIR}/data/W5224A104/PXFirm3.exe"
        tools/firmware_image.py tools/firmware_update_plan.py VERBATIM)
    add_custom_target(firmware ALL
      DEPENDS "${PLEXTOR_PACKAGED_FIRMWARE}"
        "${PLEXTOR_FIRMWARE_VALIDATION}" "${PLEXTOR_FIRMWARE_UPDATE_PLAN}")
    add_dependencies(firmware firmware_link)
    if(BUILD_TESTING)
      add_test(NAME firmware_image_validation COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tools/firmware_image.py" verify
        "${PLEXTOR_PACKAGED_FIRMWARE}"
        --fixed-data-from "${FIRMWARE_REFERENCE}"
        --report "${FW_DIR}/test-validation.json")
    endif()
  endif()
endif()

add_custom_target(verify
  COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C "$<CONFIG>"
  USES_TERMINAL)
if(BUILD_TESTING)
  get_property(PLEXTOR_ROOT_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
  foreach(PLEXTOR_TARGET IN LISTS PLEXTOR_ROOT_TARGETS)
    get_target_property(PLEXTOR_TARGET_TYPE "${PLEXTOR_TARGET}" TYPE)
    if(PLEXTOR_TARGET_TYPE STREQUAL "EXECUTABLE")
      add_dependencies(verify "${PLEXTOR_TARGET}")
    endif()
  endforeach()
endif()
if(PLEXTOR_BUILD_FIRMWARE)
  add_dependencies(verify firmware)
endif()

add_subdirectory(tests/simulator)
