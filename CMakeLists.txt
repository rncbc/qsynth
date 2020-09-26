project(QSYNTH)

cmake_minimum_required(VERSION 3.1)

set (VERSION "0.9.0")

set (CONFIG_VERSION ${VERSION})
execute_process (
  COMMAND git describe --tags --dirty --abbrev=6
  OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
  RESULT_VARIABLE GIT_DESCRIBE_RESULT
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (GIT_DESCRIBE_RESULT EQUAL 0)
  set (VERSION "${GIT_DESCRIBE_OUTPUT}")
  string (REGEX REPLACE "^[^0-9]+" "" VERSION "${VERSION}")
  string (REGEX REPLACE "-g"   "git." VERSION "${VERSION}")
  string (REGEX REPLACE "[_|-]+"  "." VERSION "${VERSION}")
  execute_process (
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_REVPARSE_OUTPUT
    RESULT_VARIABLE GIT_REVPARSE_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (GIT_REVPARSE_RESULT EQUAL 0 AND NOT GIT_REVPARSE_OUTPUT STREQUAL "master")
    set (VERSION "${VERSION} [${GIT_REVPARSE_OUTPUT}]")
  endif ()
endif ()

set (PACKAGE_NAME "Qsynth")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_BUGREPORT "rncbc@rncbc.org")
set (PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set (PACKAGE_TARNAME "qsynth")

set (CONFIG_BUILD_VERSION "${PACKAGE_VERSION}")

if (CMAKE_BUILD_TYPE)
  set (CONFIG_BUILD_TYPE ${CMAKE_BUILD_TYPE})
else ()
  set (CONFIG_BUILD_TYPE "release")
endif ()

set (CONFIG_DEBUG 0)
if (CONFIG_BUILD_TYPE MATCHES "debug")
  set (CONFIG_DEBUG 1)
endif ()

set (CONFIG_PREFIX "${CMAKE_INSTALL_PREFIX}")

include (GNUInstallDirs)
set (CONFIG_BINDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_BINDIR}")
set (CONFIG_LIBDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set (CONFIG_DATADIR "${CONFIG_PREFIX}/${CMAKE_INSTALL_DATADIR}")
set (CONFIG_MANDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_MANDIR}")


# Enable system tray argument option.
option (CONFIG_SYSTEM_TRAY "Enable system tray (default=yes)" 1)

# Enable fluid_synth_get_channel_info function (DEPRECATED).
option (CONFIG_FLUID_CHANNEL_INFO "Enable FluidSynth channel info support (DEPRECATED) (default=no)" 0)
  
# Enable fluid_synth_set_midi_router function (DEPRECATED).
option (CONFIG_FLUID_MIDI_ROUTER "Enable FluidSynth MIDI router support (DEPRECATED) (default=no)" 0)

# Enable new_fluid_server function.
option (CONFIG_FLUID_SERVER "Enable FluidSynth server (default=yes)" 1)

# Enable unique/single instance.
option (CONFIG_XUNIQUE "Enable unique/single instance (default=yes)" 1)

# Enable gradient eye-candy.
option (CONFIG_GRADIENT "Enable gradient eye-candy (default=yes)" 1)

# Enable debugger stack-trace option (assumes --enable-debug).
option (CONFIG_STACKTRACE "Enable debugger stack-trace (default=no)" 0)


# Fix for new CMAKE_REQUIRED_LIBRARIES policy.
if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW)
endif ()

# Check for Qt
find_package (Qt5 REQUIRED COMPONENTS Core Gui Widgets)

if (CONFIG_XUNIQUE)
  find_package (Qt5 REQUIRED COMPONENTS Network)
endif ()

find_package (Qt5LinguistTools)

include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckIncludeFileCXX)
include (CheckFunctionExists)
include (CheckLibraryExists)

# Checks for libraries.
if (WIN32)
  check_function_exists (lroundf CONFIG_ROUND)
else ()
  find_library (MATH_LIBRARY m)
  # Check for round math function.
  if (MATH_LIBRARY)
    set (CMAKE_REQUIRED_LIBRARIES "${MATH_LIBRARY};${CMAKE_REQUIRED_LIBRARIES}")
    check_function_exists (lroundf CONFIG_ROUND)
  else ()
    message (FATAL_ERROR "*** math library not found.")
  endif ()
endif ()


# Checks for header files.
if (UNIX AND NOT APPLE)
  check_include_files ("fcntl.h;unistd.h;signal.h" HAVE_SIGNAL_H)
endif ()


# Find package modules
find_package (PkgConfig REQUIRED)

# Check for FLUIDSYNTH library.
pkg_check_modules (FLUIDSYNTH REQUIRED fluidsynth)
if (FLUIDSYNTH_FOUND)
  set (CONFIG_FLUIDSYNTH 1)
  include_directories (${FLUIDSYNTH_INCLUDE_DIRS})
  link_directories (${FLUIDSYNTH_LIBRARY_DIRS})
# link_libraries (${FLUIDSYNTH_LIBRARIES})
  set (CMAKE_REQUIRED_LIBRARIES "${FLUIDSYNTH_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
  # Check for new_fluid_server function.
  if (CONFIG_FLUID_SERVER)
    check_function_exists (new_fluid_server CONFIG_FLUID_SERVER)
  endif ()
  # Check for fluid_synth_system_reset function.
  check_function_exists (fluid_synth_system_reset CONFIG_FLUID_SYSTEM_RESET)
  # Check for fluid_synth_set_bank_offset function.
  check_function_exists (fluid_synth_set_bank_offset CONFIG_FLUID_BANK_OFFSET)
  # Check for fluid_synth_get_channel_info function.
  if (CONFIG_FLUID_CHANNEL_INFO)
    check_function_exists (fluid_synth_get_channel_info CONFIG_FLUID_CHANNEL_INFO)
  endif()
  # Check for fluid_synth_set_midi_router function.
  if (CONFIG_FLUID_MIDI_ROUTER)
    check_function_exists (fluid_synth_set_midi_router CONFIG_FLUID_MIDI_ROUTER)
  endif()
  # Check for fluid_synth_unset_program function.
  check_function_exists (fluid_synth_unset_program CONFIG_FLUID_UNSET_PROGRAM)
  # Check for fluid_version_str function.
  check_function_exists (fluid_version_str CONFIG_FLUID_VERSION_STR)
  # Check for fluid_settings_dupstr function.
  check_function_exists (fluid_settings_dupstr CONFIG_FLUID_SETTINGS_DUPSTR)
  # Check for fluid_preset_get_banknum function.
  check_function_exists (fluid_preset_get_banknum CONFIG_FLUID_PRESET_GET_BANKNUM)
  # Check for fluid_preset_get_num function.
  check_function_exists (fluid_preset_get_num CONFIG_FLUID_PRESET_GET_NUM)
  # Check for fluid_preset_get_name function.
  check_function_exists (fluid_preset_get_name CONFIG_FLUID_PRESET_GET_NAME)
  # Check for fluid_preset_get_sfont function.
  check_function_exists (fluid_preset_get_sfont CONFIG_FLUID_PRESET_GET_SFONT)
  # Check for fluid_sfont_get_id function.
  check_function_exists (fluid_sfont_get_id CONFIG_FLUID_SFONT_GET_ID)
  # Check for fluid_sfont_get_name function.
  check_function_exists (fluid_sfont_get_name CONFIG_FLUID_SFONT_GET_NAME)
  # Check for fluid_sfont_iteration_start function.
  check_function_exists (fluid_sfont_iteration_start CONFIG_FLUID_SFONT_ITERATION_START)
  # Check for fluid_sfont_iteration_next function.
  check_function_exists (fluid_sfont_iteration_next CONFIG_FLUID_SFONT_ITERATION_NEXT)
  # Check for fluid_synth_get_chorus_speed function.
  check_function_exists (fluid_synth_get_chorus_speed CONFIG_FLUID_SYNTH_GET_CHORUS_SPEED)
  # Check for fluid_synth_get_chorus_depth function.
  check_function_exists (fluid_synth_get_chorus_depth CONFIG_FLUID_SYNTH_GET_CHORUS_DEPTH)
  # Check for FluidSynth API V2 (>= 2.0.0) specifics...
  if (CONFIG_FLUID_SFONT_ITERATION_START AND CONFIG_FLUID_SFONT_ITERATION_NEXT)
    # Check for fluid_settings_getnum_default function.
    check_function_exists (fluid_settings_getnum_default CONFIG_FLUID_SETTINGS_GETNUM_DEFAULT)
    # Check for fluid_settings_getint_default function.
    check_function_exists (fluid_settings_getint_default CONFIG_FLUID_SETTINGS_GETINT_DEFAULT)
    # Check for fluid_settings_getstr_default function.
    check_function_exists (fluid_settings_getstr_default CONFIG_FLUID_SETTINGS_GETSTR_DEFAULT)
    # Check for fluid_settings_foreach function.
    check_function_exists (fluid_settings_foreach CONFIG_FLUID_SETTINGS_FOREACH)
    # Check for fluid_settings_foreach_option function.
    check_function_exists (fluid_settings_foreach_option CONFIG_FLUID_SETTINGS_FOREACH_OPTION)
    # Check for new_fluid_server function.
    check_function_exists (new_fluid_server CONFIG_NEW_FLUID_SERVER)
  endif ()
else ()
  message (FATAL_ERROR "*** FLUIDSYNTH library not found.")
  set (CONFIG_FLUIDSYNTH 0)
endif ()


add_subdirectory (src)

configure_file (qsynth.spec.in qsynth.spec IMMEDIATE @ONLY)

install (FILES qsynth.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install (FILES qsynth.fr.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/fr/man1 RENAME qsynth.1)

# Configuration status
macro (SHOW_OPTION text value)
  if (${value})
    message ("${text}: yes")
  else ()
    message ("${text}: no")
  endif ()
endmacro ()


message   ("\n  ${PACKAGE_NAME} ${PACKAGE_VERSION}")
message   ("\n  Build target . . . . . . . . . . . . . . . . . . .: ${CONFIG_BUILD_TYPE}\n")
show_option ("  FluidSynth library support . . . . . . . . . . . ." CONFIG_FLUIDSYNTH)
show_option ("  FluidSynth server support  . . . . . . . . . . . ." CONFIG_FLUID_SERVER)
show_option ("  FluidSynth system reset support . .  . . . . . . ." CONFIG_FLUID_SYSTEM_RESET)
show_option ("  FluidSynth bank offset support . . . . . . . . . ." CONFIG_FLUID_BANK_OFFSET)
show_option ("  FluidSynth channel info support (DEPRECATED) . . ." CONFIG_FLUID_CHANNEL_INFO)
show_option ("  FluidSynth MIDI router support  (DEPRECATED) . . ." CONFIG_FLUID_MIDI_ROUTER)
show_option ("  FluidSynth unset program support . . . . . . . . ." CONFIG_FLUID_UNSET_PROGRAM)
show_option ("  FluidSynth version string support  . . . . . . . ." CONFIG_FLUID_VERSION_STR)
message     ("")
show_option ("  System tray icon support . . . . . . . . . . . . ." CONFIG_SYSTEM_TRAY)
message     ("")
show_option ("  Unique/Single instance support . . . . . . . . . ." CONFIG_XUNIQUE)
show_option ("  Gradient eye-candy . . . . . . . . . . . . . . . ." CONFIG_GRADIENT)
show_option ("  Debugger stack-trace (gdb) . . . . . . . . . . . ." CONFIG_STACKTRACE)
message   ("\n  Install prefix . . . . . . . . . . . . . . . . . .: ${CMAKE_INSTALL_PREFIX}")
message   ("\nNow type 'make', followed by 'make install' as root.\n")
