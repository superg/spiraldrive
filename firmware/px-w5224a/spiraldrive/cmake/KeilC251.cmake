# Reusable Keil C251 support (CMake 3.21 or newer).
#
# Set KEIL_C251_ROOT before including this file. keil_c251_find_tools() searches
# only <root>/BIN and returns absolute KEIL_C251_{C251,A251,L251,OH251} paths and
# KEIL_C251_ROOT to its caller. Every call searches again, without cached tools.
# keil_c251_add_target() calls discovery automatically.
#
# keil_c251_add_target(<target> [ALL] [WARNINGS_AS_ERRORS]
#   [OUTPUT_NAME <basename>] [OUTPUT_DIRECTORY <directory>]
#   SOURCES <ordered .c/.a51 files...> [HEADERS <dependencies...>]
#   [INCLUDE_DIRECTORIES <directories...>] [SEGMENTS <name(address)...>]
#   [COMPILE_OPTIONS <C251 arguments...>] [ASSEMBLE_OPTIONS <A251 arguments...>]
#   [LINK_OPTIONS <L251 arguments...>] [HEX_OPTIONS <OH251 arguments...>])
#
# Source/header/include paths are relative to the caller's source directory;
# output paths are relative to its binary directory. The default output name is
# the target name; the default directory is <caller binary>/keil/<target>.
# Include directories apply to both C251 and A251. Tool options have no defaults.
# The module owns OBJECT, PRINT, TO, and HEXFILE output arguments. SEGMENTS entries
# are joined into one linker directive. Headers trigger the entire batch build.
# Warnings are allowed unless WARNINGS_AS_ERRORS is specified; errors always fail.
#
# Absolute output paths are available as target properties KEIL_C251_HEX_FILE,
# KEIL_C251_MAP_FILE, and KEIL_C251_ABS_FILE. Consumers in another CMake directory
# must depend on the target as well as any output files they consume.
#
# Example:
#   set(KEIL_C251_ROOT "C:/Keil_v5/C251" CACHE PATH "Keil C251 installation")
#   include("cmake/KeilC251.cmake")
#   keil_c251_add_target("payload" ALL OUTPUT_NAME "application" SOURCES "main.c"
#     COMPILE_OPTIONS "MODSRC" "ROM(HUGE)" "LARGE" LINK_OPTIONS "CLASSES(ECODE(0x10000-0x1FFFF))" "NOOVERLAY"
#     HEX_OPTIONS "H386" "RANGE(0x10000-0x1FFFF)")
#   get_target_property(payload_hex "payload" "KEIL_C251_HEX_FILE")

include_guard(GLOBAL)

function(keil_c251_find_tools)
  if(NOT KEIL_C251_ROOT)
    message(FATAL_ERROR "Set KEIL_C251_ROOT to the Keil C251 installation directory")
  endif()
  get_filename_component(root "${KEIL_C251_ROOT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  foreach(name IN ITEMS "C251" "A251" "L251" "OH251")
    set(program "program-NOTFOUND")
    find_program(program NAMES "${name}.exe" "${name}" PATHS "${root}/BIN" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH NO_CACHE)
    if(NOT program)
      message(FATAL_ERROR "Keil C251: cannot find ${name} in ${root}/BIN (KEIL_C251_ROOT)")
    endif()
    set("KEIL_C251_${name}" "${program}" PARENT_SCOPE)
  endforeach()
  set(KEIL_C251_ROOT "${root}" PARENT_SCOPE)
endfunction()

# Bracket arguments preserve lists, backslashes, quotes, and literal ${...} text.
function(keil_c251_append_setting content_variable name value)
  set(delimiter "=")
  string(FIND "${value}" "]${delimiter}]" closing)
  while(NOT closing EQUAL -1)
    string(APPEND delimiter "=")
    string(FIND "${value}" "]${delimiter}]" closing)
  endwhile()
  set("${content_variable}" "${${content_variable}}set(${name} [${delimiter}[${value}]${delimiter}])\n" PARENT_SCOPE)
endfunction()

function(keil_c251_add_target target)
  cmake_parse_arguments(PARSE_ARGV 1 "keil" "ALL;WARNINGS_AS_ERRORS" "OUTPUT_NAME;OUTPUT_DIRECTORY"
    "SOURCES;HEADERS;INCLUDE_DIRECTORIES;SEGMENTS;COMPILE_OPTIONS;ASSEMBLE_OPTIONS;LINK_OPTIONS;HEX_OPTIONS")
  if(keil_UNPARSED_ARGUMENTS OR keil_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "keil_c251_add_target(${target}): invalid arguments: ${keil_UNPARSED_ARGUMENTS}; missing values: ${keil_KEYWORDS_MISSING_VALUES}")
  endif()
  if(NOT keil_SOURCES)
    message(FATAL_ERROR "keil_c251_add_target(${target}) requires SOURCES")
  endif()
  if(NOT DEFINED keil_OUTPUT_NAME)
    set(keil_OUTPUT_NAME "${target}")
  endif()
  if(keil_OUTPUT_NAME MATCHES "[/\\\\;]" OR keil_OUTPUT_NAME STREQUAL "." OR keil_OUTPUT_NAME STREQUAL "..")
    message(FATAL_ERROR "Keil OUTPUT_NAME must be a basename: ${keil_OUTPUT_NAME}")
  endif()
  if(NOT DEFINED keil_OUTPUT_DIRECTORY)
    set(keil_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/keil/${target}")
  endif()
  get_filename_component(keil_OUTPUT_DIRECTORY "${keil_OUTPUT_DIRECTORY}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  foreach(kind IN ITEMS "SOURCES" "HEADERS" "INCLUDE_DIRECTORIES")
    set(paths)
    foreach(path IN LISTS "keil_${kind}")
      get_filename_component(path "${path}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
      list(APPEND paths "${path}")
    endforeach()
    set("keil_${kind}" "${paths}")
  endforeach()

  set(stems)
  set(byproducts "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.abs"
    "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.link.log" "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.hex.log")
  foreach(source IN LISTS keil_SOURCES)
    get_filename_component(stem "${source}" NAME_WE)
    get_filename_component(extension "${source}" EXT)
    string(TOLOWER "${extension}" extension)
    if(extension STREQUAL ".c")
      set(step "compile")
    elseif(extension STREQUAL ".a51")
      set(step "assemble")
    else()
      message(FATAL_ERROR "Unsupported Keil source: ${source}")
    endif()
    string(TOLOWER "${stem}" key)
    if(key IN_LIST stems)
      message(FATAL_ERROR "Duplicate Keil source stem '${stem}' would overwrite object/listing files in target ${target}")
    endif()
    list(APPEND stems "${key}")
    list(APPEND byproducts "${keil_OUTPUT_DIRECTORY}/${stem}.obj" "${keil_OUTPUT_DIRECTORY}/${stem}.lst" "${keil_OUTPUT_DIRECTORY}/${stem}.${step}.log")
  endforeach()

  keil_c251_find_tools()
  set(settings "# Generated Keil build settings for ${target}.\n")
  foreach(name IN ITEMS "ROOT" "C251" "A251" "L251" "OH251")
    keil_c251_append_setting("settings" "keil_${name}" "${KEIL_C251_${name}}")
  endforeach()
  foreach(name IN ITEMS "OUTPUT_NAME" "OUTPUT_DIRECTORY" "SOURCES" "INCLUDE_DIRECTORIES" "SEGMENTS" "COMPILE_OPTIONS" "ASSEMBLE_OPTIONS" "LINK_OPTIONS" "HEX_OPTIONS" "WARNINGS_AS_ERRORS")
    keil_c251_append_setting("settings" "keil_${name}" "${keil_${name}}")
  endforeach()
  set(config "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}-keil.cmake")
  # Substitute only the settings variable, without interpreting its contents.
  file(CONFIGURE OUTPUT "${config}" CONTENT "@settings@" @ONLY)
  set(hex "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.hex")
  set(map "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.map")
  add_custom_command(OUTPUT "${hex}" "${map}" BYPRODUCTS ${byproducts}
    COMMAND "${CMAKE_COMMAND}" "-DKEIL_C251_BUILD_CONFIG=${config}" "-P" "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"
    DEPENDS ${keil_SOURCES} ${keil_HEADERS} "${config}" "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" "${KEIL_C251_C251}" "${KEIL_C251_A251}" "${KEIL_C251_L251}" "${KEIL_C251_OH251}"
    COMMENT "Building Keil C251 target ${target}" VERBATIM)
  set(all)
  if(keil_ALL)
    set(all "ALL")
  endif()
  add_custom_target("${target}" ${all} DEPENDS "${hex}" "${map}" SOURCES ${keil_SOURCES} ${keil_HEADERS})
  set_target_properties("${target}" PROPERTIES "KEIL_C251_HEX_FILE" "${hex}" "KEIL_C251_MAP_FILE" "${map}" "KEIL_C251_ABS_FILE" "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.abs")
endfunction()

function(keil_c251_run tool output log_file arguments)
  file(REMOVE "${keil_OUTPUT_DIRECTORY}/${output}")
  execute_process(COMMAND "${tool}" ${arguments} WORKING_DIRECTORY "${keil_OUTPUT_DIRECTORY}" RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
  set(log "${stdout}${stderr}")
  file(WRITE "${keil_OUTPUT_DIRECTORY}/${log_file}" "${log}")
  message("${log}")
  string(TOUPPER "${log}" diagnostics)
  # Keil exit statuses: 0 = success, 1 = warnings, 2 = errors, 3 = fatal errors.
  # https://www.keil.com/support/docs/613.htm
  if((NOT result STREQUAL "0" AND NOT result STREQUAL "1") OR diagnostics MATCHES "\\*\\*\\*[ ]*(FATAL[ -]?)?ERROR"
      OR diagnostics MATCHES "[1-9][0-9]*[ ]+ERROR\\(S\\)")
    message(FATAL_ERROR "Keil step failed: ${output} (see ${keil_OUTPUT_DIRECTORY}/${log_file})")
  endif()
  if(keil_WARNINGS_AS_ERRORS AND (result STREQUAL "1" OR diagnostics MATCHES "\\*\\*\\*[ ]*WARNING" OR diagnostics MATCHES "[1-9][0-9]*[ ]+WARNING\\(S\\)"))
    message(FATAL_ERROR "Keil warnings treated as errors: ${output} (see ${keil_OUTPUT_DIRECTORY}/${log_file})")
  endif()
  if(NOT EXISTS "${keil_OUTPUT_DIRECTORY}/${output}")
    message(FATAL_ERROR "Keil step did not produce ${output}")
  endif()
endfunction()

function(keil_c251_build)
  file(MAKE_DIRECTORY "${keil_OUTPUT_DIRECTORY}")
  file(REMOVE "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.hex" "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.map" "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.abs")
  set(ENV{C251INC} "${keil_ROOT}/INC")
  set(ENV{C251LIB} "${keil_ROOT}/LIB")
  set(include_option)
  if(keil_INCLUDE_DIRECTORIES)
    string(JOIN ";" include_directories ${keil_INCLUDE_DIRECTORIES})
    # Preserve INCDIR's semicolon-separated paths as one command argument.
    string(REPLACE ";" "\\;" include_option "INCDIR(${include_directories})")
  endif()
  set(objects)
  foreach(source IN LISTS keil_SOURCES)
    get_filename_component(stem "${source}" NAME_WE)
    get_filename_component(extension "${source}" EXT)
    string(TOLOWER "${extension}" extension)
    if(extension STREQUAL ".c")
      set(tool "${keil_C251}")
      set(options "${keil_COMPILE_OPTIONS}")
      set(step "compile")
    else()
      set(tool "${keil_A251}")
      set(options "${keil_ASSEMBLE_OPTIONS}")
      set(step "assemble")
    endif()
    set(arguments "${options}")
    list(PREPEND arguments "${source}")
    if(include_option)
      list(APPEND arguments "${include_option}")
    endif()
    list(APPEND arguments "OBJECT(${stem}.obj)" "PRINT(${stem}.lst)")
    keil_c251_run("${tool}" "${stem}.obj" "${stem}.${step}.log" "${arguments}")
    list(APPEND objects "${stem}.obj")
  endforeach()
  string(JOIN "," object_files ${objects})
  set(arguments "${keil_LINK_OPTIONS}")
  list(PREPEND arguments "${object_files}" "TO" "${keil_OUTPUT_NAME}.abs")
  if(keil_SEGMENTS)
    string(JOIN "," segments ${keil_SEGMENTS})
    list(APPEND arguments "SEGMENTS(${segments})")
  endif()
  list(APPEND arguments "PRINT(${keil_OUTPUT_NAME}.map)")
  keil_c251_run("${keil_L251}" "${keil_OUTPUT_NAME}.abs" "${keil_OUTPUT_NAME}.link.log" "${arguments}")
  if(NOT EXISTS "${keil_OUTPUT_DIRECTORY}/${keil_OUTPUT_NAME}.map")
    message(FATAL_ERROR "Keil linker did not produce ${keil_OUTPUT_NAME}.map")
  endif()
  set(arguments "${keil_HEX_OPTIONS}")
  list(PREPEND arguments "${keil_OUTPUT_NAME}.abs" "HEXFILE(${keil_OUTPUT_NAME}.hex)")
  keil_c251_run("${keil_OH251}" "${keil_OUTPUT_NAME}.hex" "${keil_OUTPUT_NAME}.hex.log" "${arguments}")
endfunction()

if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
  if(NOT KEIL_C251_BUILD_CONFIG)
    message(FATAL_ERROR "KeilC251.cmake script mode requires KEIL_C251_BUILD_CONFIG")
  endif()
  include("${KEIL_C251_BUILD_CONFIG}")
  keil_c251_build()
endif()
