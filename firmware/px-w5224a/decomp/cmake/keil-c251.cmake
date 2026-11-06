# C251 uses OMF251 and its own locator, not CMake's MSVC/GNU compiler rules.
# Keep the generator independent of the compiler via explicit build commands.
set(KEIL_C251_ROOT "C:/Keil_v5/C251" CACHE PATH "Keil C251 installation root")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES KEIL_C251_ROOT)
foreach(tool IN ITEMS C251 A251 L251 OH251)
  string(TOLOWER "${tool}" tool_lower)
  find_program(KEIL_${tool}
    NAMES ${tool}.exe ${tool}.EXE ${tool} ${tool_lower}.exe ${tool_lower}
    PATHS "${KEIL_C251_ROOT}/BIN" NO_DEFAULT_PATH NO_CACHE)
  if(NOT KEIL_${tool})
    message(FATAL_ERROR "Missing ${tool} in ${KEIL_C251_ROOT}/BIN. Set KEIL_C251_ROOT to your C251 installation.")
  endif()
endforeach()

set(PLEXTOR_C251_OPTIONS
  MODSRC "ROM(HUGE)" LARGE REGPARMS PARM251 "OPTIMIZE(5,SIZE)" CODE)
set(PLEXTOR_A251_OPTIONS MODSRC)
if(PLEXTOR_FIRMWARE_DIAGNOSTIC_LAYOUT)
  # Deliberately non-flashable: isolate genuine fixups from physical overflow.
  set(PLEXTOR_LINKER_CLASSES
    "CLASSES(CODE(0xE10000-0xE100FF),ECODE(0xF80050-0xFFFFFF),HCONST(0xE00000-0xE0FFFF),XDATA(0x010000-0x017FFF))")
elseif(PLEXTOR_LED_STARTUP_DIAGNOSTICS)
  # The diagnostic needs 256 more code bytes. Borrow from unused HCONST
  # capacity within the main-flash region; keep vectors, fixed data, monitor,
  # CODE helpers and XDATA at their ordinary boundaries.
  set(PLEXTOR_LINKER_CLASSES
    "CLASSES(CODE(0xFEFF00-0xFEFFFF),ECODE(0xF80050-0xFEB5FF),HCONST(0xFEB600-0xFEFEFF),XDATA(0x010000-0x017FFF))")
else()
  set(PLEXTOR_LINKER_CLASSES
    "CLASSES(CODE(0xFEFF00-0xFEFFFF),ECODE(0xF80050-0xFEB4FF),HCONST(0xFEB500-0xFEFEFF),XDATA(0x010000-0x017FFF))")
endif()
set(PLEXTOR_L251_OPTIONS
  "${PLEXTOR_LINKER_CLASSES}"
  "SEGMENTS(RESET_VECTOR_VENEER(0xF80050),CONTROLLER_EVENT_VECTOR_VENEER(0xF80054),HOST_INTERFACE_VECTOR_VENEER(0xF8007F),CONTROLLER_PACKET_VECTOR_VENEER(0xF800CB),TIMER0_VECTOR_VENEER(0xF800F6),SFR_E1_BIT7_VECTOR_VENEER(0xF80123),?PR?COROUTINE_INITIALIZATION(0xFDCEA9))"
  NOOVERLAY
  "DISABLEWARNING(30)")
set(PLEXTOR_OH251_OPTIONS H386 "RANGE(0xF80000-0xFFFFFF)")

function(keil_step name tool output)
  cmake_parse_arguments(STEP "" "" "ARGS;DEPENDS;BYPRODUCTS" ${ARGN})
  if((tool STREQUAL "C251" OR tool STREQUAL "A251") AND
      output MATCHES "\\.obj$")
    set_property(GLOBAL APPEND PROPERTY PLEXTOR_KEIL_FIRMWARE_OBJECTS
      "${output}")
  endif()
  if(tool STREQUAL "C251")
    list(APPEND STEP_ARGS "INCDIR(${PROJECT_SOURCE_DIR}/src)")
    list(APPEND STEP_DEPENDS
      "${PROJECT_SOURCE_DIR}/src/platform/mcs251.h"
      "${PROJECT_SOURCE_DIR}/src/platform/firmware_memory.h"
      "${PROJECT_SOURCE_DIR}/src/platform/firmware_rom_data.h")
    list(APPEND STEP_DEPENDS
      "${PROJECT_SOURCE_DIR}/src/boot/startup_progress_diagnostics.h")
  endif()
  if(tool STREQUAL "L251")
    list(GET STEP_ARGS 0 link_input_placeholder)
    if(NOT link_input_placeholder STREQUAL "@REGISTERED_OBJECTS@")
      message(FATAL_ERROR
        "L251 input must be @REGISTERED_OBJECTS@; object lists are generated")
    endif()
    get_property(link_objects GLOBAL PROPERTY PLEXTOR_KEIL_FIRMWARE_OBJECTS)
    if(NOT link_objects)
      message(FATAL_ERROR "No C251/A251 objects registered for L251")
    endif()
    set(unique_link_objects ${link_objects})
    list(REMOVE_DUPLICATES unique_link_objects)
    list(LENGTH link_objects link_object_count)
    list(LENGTH unique_link_objects unique_link_object_count)
    if(NOT link_object_count EQUAL unique_link_object_count)
      message(FATAL_ERROR "Duplicate C251/A251 output registered for L251")
    endif()
    list(JOIN unique_link_objects "," link_inputs)
    list(JOIN unique_link_objects "\n" link_manifest)
    file(GENERATE OUTPUT "${FW_DIR}/link_manifest.txt"
      CONTENT "${link_manifest}\n")
    message(STATUS
      "Keil firmware link manifest: ${unique_link_object_count} unique objects")
    list(REMOVE_AT STEP_ARGS 0)
    foreach(link_object IN LISTS unique_link_objects)
      list(APPEND STEP_DEPENDS "${FW_DIR}/${link_object}")
    endforeach()
    list(INSERT STEP_ARGS 0 "${link_inputs}")
  endif()
  add_custom_command(
    OUTPUT "${FW_DIR}/${output}"
    BYPRODUCTS "${FW_DIR}/${name}.log" "${FW_DIR}/${name}.json" ${STEP_BYPRODUCTS}
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
      --tool "${KEIL_${tool}}" --root "${KEIL_C251_ROOT}"
      --name "${name}" --output "${output}" -- ${STEP_ARGS}
    WORKING_DIRECTORY "${FW_DIR}"
    DEPENDS ${STEP_DEPENDS} "${PROJECT_SOURCE_DIR}/tools/keil_driver.py"
      "${KEIL_${tool}}"
    COMMENT "Keil ${tool}: ${name}"
    VERBATIM)
endfunction()
