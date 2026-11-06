set(PLEXTOR_SIMULATOR_OUTPUT "${PROJECT_SOURCE_DIR}/out/simulator")
set(PLEXTOR_SIMULATOR_IMAGES "${PLEXTOR_SIMULATOR_OUTPUT}/images")
set(PLEXTOR_SIMULATOR_REPORT "${PLEXTOR_SIMULATOR_OUTPUT}/report")
set(PLEXTOR_SIMULATOR_RUNS "${PLEXTOR_SIMULATOR_OUTPUT}/runs")
set(PLEXTOR_SIMULATOR_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/simulator.py")
set(PLEXTOR_SIMULATOR_METADATA
  "${CMAKE_CURRENT_SOURCE_DIR}/metadata/W5224A104.json")
set(PLEXTOR_SIMULATOR_INVENTORY
  "${PLEXTOR_SIMULATOR_OUTPUT}/catalog/inventory.json")
set(PLEXTOR_GENERATED_CASES_DIR "${PLEXTOR_SIMULATOR_OUTPUT}/cases")
set(PLEXTOR_GENERATED_CASES "${PLEXTOR_GENERATED_CASES_DIR}/auto.json")
set(PLEXTOR_GENERATED_CASE_OVERRIDES
  "${CMAKE_CURRENT_SOURCE_DIR}/cases/generated-overrides.json")
set(PLEXTOR_MONITOR_CAPTURE "" CACHE FILEPATH
  "Optional 64-KiB RAM:ff0000 runtime-monitor capture")

add_custom_command(OUTPUT "${PLEXTOR_SIMULATOR_INVENTORY}"
  COMMAND "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}" inventory
    --metadata "${PLEXTOR_SIMULATOR_METADATA}"
    --output "${PLEXTOR_SIMULATOR_INVENTORY}"
  DEPENDS "${PLEXTOR_SIMULATOR_RUNNER}" "${PLEXTOR_SIMULATOR_METADATA}"
  VERBATIM)
add_custom_target(emulator-inventory DEPENDS "${PLEXTOR_SIMULATOR_INVENTORY}")

add_custom_command(OUTPUT "${PLEXTOR_GENERATED_CASES}"
  COMMAND "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}" generate-cases
    --metadata "${PLEXTOR_SIMULATOR_METADATA}"
    --overrides "${PLEXTOR_GENERATED_CASE_OVERRIDES}"
    --output "${PLEXTOR_GENERATED_CASES}"
  DEPENDS "${PLEXTOR_SIMULATOR_RUNNER}" "${PLEXTOR_SIMULATOR_METADATA}"
    "${PLEXTOR_GENERATED_CASE_OVERRIDES}"
  VERBATIM)
add_custom_target(emulator-generated-cases DEPENDS "${PLEXTOR_GENERATED_CASES}")

set(PLEXTOR_SIMULATOR_IMAGE_OUTPUTS
  "${PLEXTOR_SIMULATOR_IMAGES}/stock.hex"
  "${PLEXTOR_SIMULATOR_IMAGES}/images.json")
set(PLEXTOR_SIMULATOR_IMAGE_DEPENDS)
set(PLEXTOR_SIMULATOR_PREPARE_MONITOR_ARGS)
set(PLEXTOR_SIMULATOR_EXECUTION_MONITOR_ARGS)
if(PLEXTOR_MONITOR_CAPTURE)
  list(APPEND PLEXTOR_SIMULATOR_IMAGE_OUTPUTS
    "${PLEXTOR_SIMULATOR_IMAGES}/monitor.hex")
  list(APPEND PLEXTOR_SIMULATOR_IMAGE_DEPENDS "${PLEXTOR_MONITOR_CAPTURE}")
  list(APPEND PLEXTOR_SIMULATOR_PREPARE_MONITOR_ARGS
    --monitor "${PLEXTOR_MONITOR_CAPTURE}")
  list(APPEND PLEXTOR_SIMULATOR_EXECUTION_MONITOR_ARGS
    --monitor-hex "${PLEXTOR_SIMULATOR_IMAGES}/monitor.hex")
endif()

add_custom_command(
  OUTPUT ${PLEXTOR_SIMULATOR_IMAGE_OUTPUTS}
  COMMAND "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}" prepare
    --stock "${FIRMWARE_REFERENCE}"
    ${PLEXTOR_SIMULATOR_PREPARE_MONITOR_ARGS}
    --metadata "${PLEXTOR_SIMULATOR_METADATA}"
    --output "${PLEXTOR_SIMULATOR_IMAGES}"
  DEPENDS "${PLEXTOR_SIMULATOR_RUNNER}" "${FIRMWARE_REFERENCE}"
    "${PLEXTOR_SIMULATOR_METADATA}"
    ${PLEXTOR_SIMULATOR_IMAGE_DEPENDS}
  VERBATIM)
add_custom_target(emulator-images DEPENDS ${PLEXTOR_SIMULATOR_IMAGE_OUTPUTS})

set(PLEXTOR_SIMULATOR_MONITOR_REPORT_ARGS)
set(PLEXTOR_SIMULATOR_MONITOR_REPORT_DEPENDS)
if(PLEXTOR_MONITOR_CAPTURE AND PLEXTOR_BUILD_FIRMWARE)
  list(APPEND PLEXTOR_SIMULATOR_MONITOR_REPORT_ARGS
    --cases "${CMAKE_CURRENT_SOURCE_DIR}/monitor-cases"
    --results "${PLEXTOR_SIMULATOR_RUNS}/monitor/results.json")
  list(APPEND PLEXTOR_SIMULATOR_MONITOR_REPORT_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/monitor-cases/integration.json")
endif()

add_custom_target(emulator-report
  COMMAND "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}" report
    --source-root "${PROJECT_SOURCE_DIR}/src"
    --cases "${CMAKE_CURRENT_SOURCE_DIR}/cases"
    --cases "${PLEXTOR_GENERATED_CASES}"
    ${PLEXTOR_SIMULATOR_MONITOR_REPORT_ARGS}
    --metadata "${PLEXTOR_SIMULATOR_METADATA}"
    --results "${PLEXTOR_SIMULATOR_RUNS}/results.json"
    --results "${PLEXTOR_SIMULATOR_RUNS}/pilot/results.json"
    --results "${PLEXTOR_SIMULATOR_RUNS}/generated/results.json"
    --output "${PLEXTOR_SIMULATOR_REPORT}"
  DEPENDS "${PLEXTOR_SIMULATOR_RUNNER}"
    "${PLEXTOR_SIMULATOR_METADATA}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cases/cases.json"
    ${PLEXTOR_SIMULATOR_MONITOR_REPORT_DEPENDS}
    emulator-generated-cases
  VERBATIM)

if(BUILD_TESTING)
  add_test(NAME simulator_harness_tests
    COMMAND "${Python3_EXECUTABLE}" -m unittest
      tests.simulator.test_simulator -v)
  set_tests_properties(simulator_harness_tests PROPERTIES
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    ENVIRONMENT "PYTHONDONTWRITEBYTECODE=1")
endif()

if(PLEXTOR_BUILD_FIRMWARE)
  find_program(KEIL_UVISION_EXECUTABLE
    NAMES UV4.exe UV4
    PATHS "${KEIL_C251_ROOT}/../UV4"
    NO_DEFAULT_PATH)
  set(PLEXTOR_UVISION_SIMULATOR_COMMAND
    "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}")
  set(PLEXTOR_UVISION_LAUNCHER_DEPENDS)
  if(WIN32)
    set(PLEXTOR_WINDOWS_PRIVATE_DESKTOP_LAUNCHER
      "${PROJECT_SOURCE_DIR}/tools/run_on_private_desktop.py")
    set(PLEXTOR_UVISION_SIMULATOR_COMMAND
      "${Python3_EXECUTABLE}" "${PLEXTOR_WINDOWS_PRIVATE_DESKTOP_LAUNCHER}"
      --watch-executable UV4.exe --watchdog-seconds 90
      -- "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}")
    list(APPEND PLEXTOR_UVISION_LAUNCHER_DEPENDS
      "${PLEXTOR_WINDOWS_PRIVATE_DESKTOP_LAUNCHER}")
  endif()
  set(PLEXTOR_GHIDRA_HOME "" CACHE PATH
    "Ghidra installation containing support/analyzeHeadless")

  add_custom_target(emulator-catalog
    COMMAND "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}"
      validate-metadata --metadata "${PLEXTOR_SIMULATOR_METADATA}"
      --stock "${FIRMWARE_REFERENCE}"
    DEPENDS "${PLEXTOR_SIMULATOR_RUNNER}" "${PLEXTOR_SIMULATOR_METADATA}"
      "${FIRMWARE_REFERENCE}"
    VERBATIM)

  set(PLEXTOR_STOCK_APPLICATION_DIR "${PLEXTOR_SIMULATOR_OUTPUT}/stock")
  add_custom_command(OUTPUT
      "${PLEXTOR_STOCK_APPLICATION_DIR}/stock_firmware.abs"
      "${PLEXTOR_STOCK_APPLICATION_DIR}/firmware_memory.abs"
    COMMAND "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}" build-stock
      --metadata "${PLEXTOR_SIMULATOR_METADATA}"
      --stock "${FIRMWARE_REFERENCE}"
      --keil-driver "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
      --keil-root "${KEIL_C251_ROOT}"
      --a251 "${KEIL_A251}"
      --l251 "${KEIL_L251}"
      --output "${PLEXTOR_STOCK_APPLICATION_DIR}"
    DEPENDS "${PLEXTOR_SIMULATOR_RUNNER}" "${PLEXTOR_SIMULATOR_METADATA}"
      "${FIRMWARE_REFERENCE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
    USES_TERMINAL VERBATIM)
  add_custom_target(emulator-stock-application
    DEPENDS "${PLEXTOR_STOCK_APPLICATION_DIR}/stock_firmware.abs"
      "${PLEXTOR_STOCK_APPLICATION_DIR}/firmware_memory.abs")

  if(PLEXTOR_MONITOR_CAPTURE)
    add_custom_command(OUTPUT "${PLEXTOR_SIMULATOR_IMAGES}/monitor.abs"
      COMMAND "${Python3_EXECUTABLE}" "${PLEXTOR_SIMULATOR_RUNNER}" build-monitor
        --monitor "${PLEXTOR_MONITOR_CAPTURE}"
        --keil-driver "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
        --keil-root "${KEIL_C251_ROOT}"
        --a251 "${KEIL_A251}"
        --l251 "${KEIL_L251}"
        --output "${PLEXTOR_SIMULATOR_IMAGES}"
      DEPENDS "${PLEXTOR_SIMULATOR_RUNNER}" "${PLEXTOR_MONITOR_CAPTURE}"
        "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
      VERBATIM)
    add_custom_target(emulator-monitor-application
      DEPENDS "${PLEXTOR_SIMULATOR_IMAGES}/monitor.abs")
  endif()

  set(PLEXTOR_SMOKE_DIR "${PLEXTOR_SIMULATOR_OUTPUT}/smoke")
  file(MAKE_DIRECTORY "${PLEXTOR_SMOKE_DIR}")
  add_custom_command(OUTPUT "${PLEXTOR_SMOKE_DIR}/smoke.obj"
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
      --tool "${KEIL_C251}" --root "${KEIL_C251_ROOT}"
      --name smoke_compile --output smoke.obj --
      "${CMAKE_CURRENT_SOURCE_DIR}/smoke.c" ${PLEXTOR_C251_OPTIONS}
      "OBJECT(smoke.obj)" "PRINT(smoke.lst)"
    WORKING_DIRECTORY "${PLEXTOR_SMOKE_DIR}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/smoke.c"
      "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
    VERBATIM)
  add_custom_command(OUTPUT "${PLEXTOR_SMOKE_DIR}/smoke.abs"
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
      --tool "${KEIL_L251}" --root "${KEIL_C251_ROOT}"
      --name smoke_link --output smoke.abs --
      smoke.obj TO smoke.abs "${PLEXTOR_LINKER_CLASSES}" NOOVERLAY
      "PRINT(smoke.map)"
    WORKING_DIRECTORY "${PLEXTOR_SMOKE_DIR}"
    DEPENDS "${PLEXTOR_SMOKE_DIR}/smoke.obj"
      "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
    VERBATIM)

  if(KEIL_UVISION_EXECUTABLE)
    add_custom_target(emulator-smoke
      COMMAND ${PLEXTOR_UVISION_SIMULATOR_COMMAND} smoke
        --uv4 "${KEIL_UVISION_EXECUTABLE}"
        --base-project "${KEIL_C251_ROOT}/Examples/Blinky/BLINKY.uvproj"
        --application "${PLEXTOR_SMOKE_DIR}/smoke.abs"
        --memory-application
          "${PLEXTOR_STOCK_APPLICATION_DIR}/firmware_memory.abs"
        --output "${PLEXTOR_SMOKE_DIR}/run"
      DEPENDS "${PLEXTOR_SMOKE_DIR}/smoke.abs" emulator-stock-application
        "${PLEXTOR_SIMULATOR_RUNNER}" ${PLEXTOR_UVISION_LAUNCHER_DEPENDS}
      USES_TERMINAL VERBATIM)
  else()
    add_custom_target(emulator-smoke
      COMMAND "${CMAKE_COMMAND}" -E echo
        "UV4.exe not found; set KEIL_UVISION_EXECUTABLE"
      COMMAND "${CMAKE_COMMAND}" -E false VERBATIM)
  endif()

  if(EXISTS "${PLEXTOR_SIMULATOR_METADATA}")
    if(KEIL_UVISION_EXECUTABLE)
      set(PLEXTOR_PILOT_DIR "${PLEXTOR_SIMULATOR_OUTPUT}/pilot")
      file(MAKE_DIRECTORY "${PLEXTOR_PILOT_DIR}")
      add_custom_command(OUTPUT "${PLEXTOR_PILOT_DIR}/arithmetic.obj"
        COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
          --tool "${KEIL_C251}" --root "${KEIL_C251_ROOT}"
          --name pilot_arithmetic_compile --output arithmetic.obj --
          "${PROJECT_SOURCE_DIR}/src/platform/arithmetic.c"
          ${PLEXTOR_C251_OPTIONS} "INCDIR(${PROJECT_SOURCE_DIR}/src)"
          "OBJECT(arithmetic.obj)" "PRINT(arithmetic.lst)"
        WORKING_DIRECTORY "${PLEXTOR_PILOT_DIR}"
        DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/arithmetic.c"
          "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
        VERBATIM)
      add_custom_command(OUTPUT "${PLEXTOR_PILOT_DIR}/isolated_stubs.obj"
        COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
          --tool "${KEIL_C251}" --root "${KEIL_C251_ROOT}"
          --name pilot_stubs_compile --output isolated_stubs.obj --
          "${CMAKE_CURRENT_SOURCE_DIR}/isolated_stubs.c"
          ${PLEXTOR_C251_OPTIONS} "INCDIR(${PROJECT_SOURCE_DIR}/src)"
          "OBJECT(isolated_stubs.obj)" "PRINT(isolated_stubs.lst)"
        WORKING_DIRECTORY "${PLEXTOR_PILOT_DIR}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/isolated_stubs.c"
          "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
        VERBATIM)
      add_custom_command(OUTPUT "${PLEXTOR_PILOT_DIR}/firmware_ram_window.obj"
        COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
          --tool "${KEIL_A251}" --root "${KEIL_C251_ROOT}"
          --name pilot_ram_window_assemble --output firmware_ram_window.obj --
          "${PROJECT_SOURCE_DIR}/src/platform/firmware_ram_window.a51"
          MODSRC "OBJECT(firmware_ram_window.obj)" "PRINT(firmware_ram_window.lst)"
        WORKING_DIRECTORY "${PLEXTOR_PILOT_DIR}"
        DEPENDS "${PROJECT_SOURCE_DIR}/src/platform/firmware_ram_window.a51"
          "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
        VERBATIM)
      add_custom_command(OUTPUT "${PLEXTOR_PILOT_DIR}/arithmetic.abs"
        COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
          --tool "${KEIL_L251}" --root "${KEIL_C251_ROOT}"
          --name pilot_link --output arithmetic.abs --
          "arithmetic.obj,isolated_stubs.obj,firmware_ram_window.obj"
          TO arithmetic.abs "${PLEXTOR_LINKER_CLASSES}" NOOVERLAY
          "PRINT(arithmetic.map)"
        WORKING_DIRECTORY "${PLEXTOR_PILOT_DIR}"
        DEPENDS "${PLEXTOR_PILOT_DIR}/arithmetic.obj"
          "${PLEXTOR_PILOT_DIR}/isolated_stubs.obj"
          "${PLEXTOR_PILOT_DIR}/firmware_ram_window.obj"
          "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
        VERBATIM)
      add_custom_target(emulator-pilot
        COMMAND ${PLEXTOR_UVISION_SIMULATOR_COMMAND}
          differential
          --uv4 "${KEIL_UVISION_EXECUTABLE}"
          --base-project "${KEIL_C251_ROOT}/Examples/Blinky/BLINKY.uvproj"
          --application "${PLEXTOR_PILOT_DIR}/arithmetic.abs"
          --keil-driver "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
          --keil-root "${KEIL_C251_ROOT}"
          --a251 "${KEIL_A251}"
          --l251 "${KEIL_L251}"
          --stock "${FIRMWARE_REFERENCE}"
          --stock-application
            "${PLEXTOR_STOCK_APPLICATION_DIR}/stock_firmware.abs"
          --stock-memory-application
            "${PLEXTOR_STOCK_APPLICATION_DIR}/firmware_memory.abs"
          --no-reconstructed-memory-application
          --map "${PLEXTOR_PILOT_DIR}/arithmetic.map"
          --listings "${PLEXTOR_PILOT_DIR}"
          --metadata "${PLEXTOR_SIMULATOR_METADATA}"
          --cases "${CMAKE_CURRENT_SOURCE_DIR}/cases/cases.json"
          --output "${PLEXTOR_SIMULATOR_RUNS}/pilot"
        DEPENDS "${PLEXTOR_PILOT_DIR}/arithmetic.abs" emulator-catalog
          emulator-stock-application
          "${PLEXTOR_SIMULATOR_RUNNER}"
          ${PLEXTOR_UVISION_LAUNCHER_DEPENDS}
          "${CMAKE_CURRENT_SOURCE_DIR}/cases/cases.json"
        USES_TERMINAL VERBATIM)

      add_custom_target(emulator-differential
        COMMAND ${PLEXTOR_UVISION_SIMULATOR_COMMAND}
          differential
          --uv4 "${KEIL_UVISION_EXECUTABLE}"
          --base-project "${KEIL_C251_ROOT}/Examples/Blinky/BLINKY.uvproj"
          --application "${FW_DIR}/main.abs"
          --keil-driver "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
          --keil-root "${KEIL_C251_ROOT}"
          --a251 "${KEIL_A251}"
          --l251 "${KEIL_L251}"
          --stock "${FIRMWARE_REFERENCE}"
          --stock-application
            "${PLEXTOR_STOCK_APPLICATION_DIR}/stock_firmware.abs"
          --stock-memory-application
            "${PLEXTOR_STOCK_APPLICATION_DIR}/firmware_memory.abs"
          --reconstructed-image "${FW_DIR}/MONT_reconstructed.bin"
          ${PLEXTOR_SIMULATOR_EXECUTION_MONITOR_ARGS}
          --map "${FW_DIR}/main.map"
          --listings "${FW_DIR}"
          --metadata "${PLEXTOR_SIMULATOR_METADATA}"
          --cases "${CMAKE_CURRENT_SOURCE_DIR}/cases"
          --output "${PLEXTOR_SIMULATOR_RUNS}"
        DEPENDS firmware emulator-images emulator-catalog
          emulator-stock-application
          "${PLEXTOR_SIMULATOR_RUNNER}"
          ${PLEXTOR_UVISION_LAUNCHER_DEPENDS}
          "${CMAKE_CURRENT_SOURCE_DIR}/cases/cases.json"
          "${CMAKE_CURRENT_SOURCE_DIR}/cases/feasibility.json"
          "${CMAKE_CURRENT_SOURCE_DIR}/cases/integration.json"
        USES_TERMINAL VERBATIM)

      if(PLEXTOR_MONITOR_CAPTURE)
        add_custom_target(emulator-monitor-differential
          COMMAND ${PLEXTOR_UVISION_SIMULATOR_COMMAND}
            differential
            --uv4 "${KEIL_UVISION_EXECUTABLE}"
            --base-project "${KEIL_C251_ROOT}/Examples/Blinky/BLINKY.uvproj"
            --application "${FW_DIR}/main.abs"
            --keil-driver "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
            --keil-root "${KEIL_C251_ROOT}"
            --a251 "${KEIL_A251}"
            --l251 "${KEIL_L251}"
            --stock "${FIRMWARE_REFERENCE}"
            --stock-application
              "${PLEXTOR_STOCK_APPLICATION_DIR}/stock_firmware.abs"
            --stock-memory-application
              "${PLEXTOR_STOCK_APPLICATION_DIR}/firmware_memory.abs"
            --reconstructed-image "${FW_DIR}/MONT_reconstructed.bin"
            --monitor-hex "${PLEXTOR_SIMULATOR_IMAGES}/monitor.abs"
            --map "${FW_DIR}/main.map"
            --listings "${FW_DIR}"
            --metadata "${PLEXTOR_SIMULATOR_METADATA}"
            --cases "${CMAKE_CURRENT_SOURCE_DIR}/monitor-cases"
            --output "${PLEXTOR_SIMULATOR_RUNS}/monitor"
          DEPENDS firmware emulator-monitor-application emulator-catalog
            emulator-stock-application "${PLEXTOR_SIMULATOR_RUNNER}"
            ${PLEXTOR_UVISION_LAUNCHER_DEPENDS}
            "${CMAKE_CURRENT_SOURCE_DIR}/monitor-cases/integration.json"
          USES_TERMINAL VERBATIM)
      endif()

      add_custom_target(emulator-generated-differential
        COMMAND ${PLEXTOR_UVISION_SIMULATOR_COMMAND}
          differential
          --uv4 "${KEIL_UVISION_EXECUTABLE}"
          --base-project "${KEIL_C251_ROOT}/Examples/Blinky/BLINKY.uvproj"
          --application "${FW_DIR}/main.abs"
          --keil-driver "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
          --keil-root "${KEIL_C251_ROOT}"
          --a251 "${KEIL_A251}"
          --l251 "${KEIL_L251}"
          --stock "${FIRMWARE_REFERENCE}"
          --stock-application
            "${PLEXTOR_STOCK_APPLICATION_DIR}/stock_firmware.abs"
          --stock-memory-application
            "${PLEXTOR_STOCK_APPLICATION_DIR}/firmware_memory.abs"
          --reconstructed-image "${FW_DIR}/MONT_reconstructed.bin"
          ${PLEXTOR_SIMULATOR_EXECUTION_MONITOR_ARGS}
          --map "${FW_DIR}/main.map"
          --listings "${FW_DIR}"
          --metadata "${PLEXTOR_SIMULATOR_METADATA}"
          --cases "${PLEXTOR_GENERATED_CASES}"
          --output "${PLEXTOR_SIMULATOR_RUNS}/generated"
          --resume
        DEPENDS firmware emulator-images emulator-stock-application
          emulator-generated-cases emulator-catalog
          "${PLEXTOR_SIMULATOR_RUNNER}"
          ${PLEXTOR_UVISION_LAUNCHER_DEPENDS}
        USES_TERMINAL VERBATIM)
    endif()
  else()
    add_custom_target(emulator-differential
      COMMAND "${CMAKE_COMMAND}" -E echo
        "Committed simulator metadata is missing"
      COMMAND "${CMAKE_COMMAND}" -E false VERBATIM)
  endif()

  if(PLEXTOR_GHIDRA_HOME AND
      EXISTS "${PLEXTOR_GHIDRA_HOME}/support/analyzeHeadless")
    set(PLEXTOR_REFRESHED_METADATA
      "${PLEXTOR_SIMULATOR_OUTPUT}/catalog/ghidra-refresh.json")
    add_custom_target(emulator-refresh-metadata
      COMMAND "${PLEXTOR_GHIDRA_HOME}/support/analyzeHeadless"
        "${PROJECT_SOURCE_DIR}/ghidra" gplextor/validated
        -process MONT_104.bin -readOnly -noanalysis
        -scriptPath "${CMAKE_CURRENT_SOURCE_DIR}"
        -postScript ExportSimulatorCatalog.java "${PLEXTOR_REFRESHED_METADATA}"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ExportSimulatorCatalog.java"
      USES_TERMINAL VERBATIM)
  else()
    add_custom_target(emulator-refresh-metadata
      COMMAND "${CMAKE_COMMAND}" -E echo
        "Ghidra is optional; set PLEXTOR_GHIDRA_HOME only to refresh metadata"
      COMMAND "${CMAKE_COMMAND}" -E false VERBATIM)
  endif()
endif()
