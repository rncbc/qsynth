cmake_minimum_required (VERSION 3.15)

project (Qsynth
  VERSION 0.9.91
  DESCRIPTION "A fluidsynth Qt GUI Interface"
  HOMEPAGE_URL "https://qsynth.sourceforge.io"
  LANGUAGES C CXX)

set (PROJECT_TITLE "${PROJECT_NAME}")
string (TOLOWER "${PROJECT_TITLE}" PROJECT_NAME)

set (PROJECT_COPYRIGHT "Copyright (C) 2003-2024, rncbc aka Rui Nuno Capela. All rights reserved.")
set (PROJECT_DOMAIN    "rncbc.org")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

execute_process (
  COMMAND git describe --tags --dirty --abbrev=6
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
  RESULT_VARIABLE GIT_DESCRIBE_RESULT
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (GIT_DESCRIBE_RESULT EQUAL 0)
  set (GIT_VERSION "${GIT_DESCRIBE_OUTPUT}")
  string (REGEX REPLACE "^[^0-9]+" "" GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "^1_"      "" GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "^[_vV]+"  "" GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "-g"   "git." GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "[_|-]"   "." GIT_VERSION "${GIT_VERSION}")
  execute_process (
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVPARSE_OUTPUT
    RESULT_VARIABLE GIT_REVPARSE_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (GIT_REVPARSE_RESULT EQUAL 0 AND NOT GIT_REVPARSE_OUTPUT STREQUAL "main")
    set (GIT_VERSION "${GIT_VERSION} [${GIT_REVPARSE_OUTPUT}]")
  endif ()
  set (PROJECT_VERSION "${GIT_VERSION}")
endif ()


if (CMAKE_BUILD_TYPE MATCHES "Debug")
  set (CONFIG_DEBUG 1)
  set (CONFIG_BUILD_TYPE "debug")
else ()
  set (CONFIG_DEBUG 0)
  set (CONFIG_BUILD_TYPE "release")
  set (CMAKE_BUILD_TYPE "Release")
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

# Enable PipwWire support.
option (CONFIG_PIPEWIRE "Enable PipeWire support (default=yes)" 1)

# Enable unique/single instance.
option (CONFIG_XUNIQUE "Enable unique/single instance (default=yes)" 1)

# Enable gradient eye-candy.
option (CONFIG_GRADIENT "Enable gradient eye-candy (default=yes)" 1)

# Enable debugger stack-trace option (assumes --enable-debug).
option (CONFIG_STACKTRACE "Enable debugger stack-trace (default=no)" 0)

# Enable Wayland support option.
option (CONFIG_WAYLAND "Enable Wayland support (EXPERIMENTAL) (default=no)" 0)

# Enable Qt6 build preference.
option (CONFIG_QT6 "Enable Qt6 build (default=yes)" 1)


# Fix for new CMAKE_REQUIRED_LIBRARIES policy.
if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW)
endif ()

# Check for Qt...
if (CONFIG_QT6)
  find_package (Qt6 QUIET)
  if (NOT Qt6_FOUND)
    set (CONFIG_QT6 0)
  endif ()
endif ()

if (CONFIG_QT6)
  find_package (QT QUIET NAMES Qt6)
else ()
  find_package (QT QUIET NAMES Qt5)
endif ()

find_package (Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Svg)

if (CONFIG_XUNIQUE)
  find_package (Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
endif ()

find_package (Qt${QT_VERSION_MAJOR}LinguistTools)

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
include (FindPkgConfig)

# Check for FLUIDSYNTH library.
pkg_check_modules (FLUIDSYNTH REQUIRED IMPORTED_TARGET fluidsynth)
if (FLUIDSYNTH_FOUND)
  find_library(FLUIDSYNTH_LIBRARY NAMES ${FLUIDSYNTH_LIBRARIES} HINTS ${FLUIDSYNTH_LIBDIR})
endif ()
if (FLUIDSYNTH_LIBRARY)
  set (CONFIG_FLUIDSYNTH 1)
  set (CMAKE_REQUIRED_LIBRARIES "${FLUIDSYNTH_LIBRARY};${CMAKE_REQUIRED_LIBRARIES}")
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
    if (CONFIG_FLUID_SERVER)
      check_function_exists (new_fluid_server CONFIG_NEW_FLUID_SERVER)
    endif ()
    # Check for fluid_free function
    check_function_exists (fluid_free CONFIG_FLUID_FREE)
  endif ()
else ()
  message (FATAL_ERROR "*** FLUIDSYNTH library not found.")
  set (CONFIG_FLUIDSYNTH 0)
endif ()


if (CONFIG_PIPEWIRE)
  pkg_check_modules (PIPEWIRE IMPORTED_TARGET libpipewire-0.3)
  if (NOT PIPEWIRE_FOUND)
    set (CONFIG_PIPEWIRE 0)
  endif ()
endif ()


add_subdirectory (src)


# Configuration status
macro (SHOW_OPTION text value)
  if (${value})
    message ("${text}: yes")
  else ()
    message ("${text}: no")
  endif ()
endmacro ()

message   ("\n  ${PROJECT_TITLE} ${PROJECT_VERSION} (Qt ${QT_VERSION})")
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
show_option ("  PipeWire (pw-init) support . . . . . . . . . . . ." CONFIG_PIPEWIRE)
message     ("")
show_option ("  System tray icon support . . . . . . . . . . . . ." CONFIG_SYSTEM_TRAY)
message     ("")
show_option ("  Unique/Single instance support . . . . . . . . . ." CONFIG_XUNIQUE)
show_option ("  Gradient eye-candy . . . . . . . . . . . . . . . ." CONFIG_GRADIENT)
show_option ("  Debugger stack-trace (gdb) . . . . . . . . . . . ." CONFIG_STACKTRACE)
message   ("\n  Install prefix . . . . . . . . . . . . . . . . . .: ${CONFIG_PREFIX}\n")
