project(QSYNTH)

cmake_minimum_required(VERSION 2.6)

set (VERSION "0.5.4")

set (PACKAGE_NAME "Qsynth")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_BUGREPORT "rncbc@rncbc.org")
set (PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set (PACKAGE_TARNAME "qsynth")

set (CONFIG_BUILD_VERSION "${PACKAGE_VERSION}")

if (CMAKE_BUILD_TYPE)
  set (CONFIG_BUILD_TYPE CMAKE_BUILD_TYPE)
else ()
  set (CONFIG_BUILD_TYPE "release")
endif ()

set (CONFIG_DEBUG)
if (CONFIG_BUILD_TYPE MATCHES "debug")
  set(CONFIG_DEBUG 1)
endif ()
set (CONFIG_PREFIX "${CMAKE_INSTALL_PREFIX}")

# X11 unique/single instance.# gradient eye-candy.
option (CONFIG_XUNIQUE "Define if X11 unique/single instance is enabled." 1)
# gradient eye-candy.
option (CONFIG_GRADIENT "Define if gradient eye-candy is enabled." 1)
# debugging stack-trace.
option (CONFIG_STACKTRACE "Define if debugger stack-trace is enabled." 0)
# system-tray icon.
option (CONFIG_SYSTEM_TRAY "Define if system tray is enabled." 1)

# fluid_synth_get_channel_info (DEPRECATED).
option (CONFIG_FLUID_CHANNEL_INFO "Define if FluidSynth_channel_info is enabled." 0)
# fluid_synth_set_midi_router (DEPRECATED).
option (CONFIG_FLUID_MIDI_ROUTER "Define if FluidSynth MIDI router support is enabled." 0)
# FluidSynth server support (DEPRECATED).
option (CONFIG_FLUID_SERVER "Define if FluidSynth server support is enabled." 1)


# Check for Qt
if (UNIX AND NOT APPLE)
  find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets X11Extras)
else ()
  find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
endif ()
find_package (Qt5LinguistTools)

include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckLibraryExists)

# Checks for libraries.
if (WIN32)
  check_function_exists ( lroundf CONFIG_ROUND )
else ()
  find_library ( MATH_LIBRARY m )
  # Check for round math function.
  if (MATH_LIBRARY)
    check_function_exists ( lroundf CONFIG_ROUND )
  else ()
    message (FATAL_ERROR "math library not found")
  endif ()
endif ()

# Check for fluidsynth library.
find_library ( FLUIDSYNTH_LIBRARY fluidsynth )

# Checks for header files.
if (UNIX AND NOT APPLE)
  check_include_files ("fcntl.h;unistd.h;signal.h" HAVE_SIGNAL_H)
  if (CONFIG_XUNIQUE)
    find_library ( X11_LIBRARY X11 )
  endif ()
endif ()

# Check for fluidsynth headers.
find_path (FLUIDSYNTH_INCLUDEDIR NAMES fluidsynth.h)
set (CMAKE_REQUIRED_INCLUDES ${FLUIDSYNTH_INCLUDEDIR})
check_include_file ("fluidsynth.h" HAVE_FLUIDSYNTH_H)

if (FLUIDSYNTH_LIBRARY AND HAVE_FLUIDSYNTH_H)
  set(CMAKE_REQUIRED_LIBRARIES ${FLUIDSYNTH_LIBRARY})
  # Check for new_fluid_server function.
  if (CONFIG_FLUID_SERVER)
    check_function_exists ( new_fluid_server CONFIG_FLUID_SERVER )
  endif ()
  # Check for fluid_synth_system_reset function.
  check_function_exists ( fluid_synth_system_reset CONFIG_FLUID_SYSTEM_RESET )
  # Check for fluid_synth_set_bank_offset function.
  check_function_exists ( fluid_synth_set_bank_offset CONFIG_FLUID_BANK_OFFSET )
  # Check for fluid_synth_get_channel_info function.
  if (CONFIG_FLUID_CHANNEL_INFO)
    check_function_exists ( fluid_synth_get_channel_info CONFIG_FLUID_CHANNEL_INFO )
  endif()
  # Check for fluid_synth_set_midi_router function.
  if (CONFIG_FLUID_MIDI_ROUTER)
    check_function_exists ( fluid_synth_set_midi_router CONFIG_FLUID_MIDI_ROUTER )
  endif()
  # Check for fluid_synth_unset_program function.
  check_function_exists ( fluid_synth_unset_program CONFIG_FLUID_UNSET_PROGRAM )
  # Check for fluid_version_str function.
  check_function_exists ( fluid_version_str CONFIG_FLUID_VERSION_STR )
  # Check for fluid_settings_dupstr function.
  check_function_exists ( fluid_settings_dupstr CONFIG_FLUID_SETTINGS_DUPSTR )
  # Check for fluid_preset_get_banknum function.
  check_function_exists ( fluid_preset_get_banknum CONFIG_FLUID_PRESET_GET_BANKNUM )
  # Check for fluid_preset_get_num function.
  check_function_exists ( fluid_preset_get_num CONFIG_FLUID_PRESET_GET_NUM )
  # Check for fluid_preset_get_name function.
  check_function_exists ( fluid_preset_get_name CONFIG_FLUID_PRESET_GET_NAME )
  # Check for fluid_preset_get_sfont function.
  check_function_exists ( fluid_preset_get_sfont CONFIG_FLUID_PRESET_GET_SFONT )
  # Check for fluid_sfont_get_id function.
  check_function_exists ( fluid_sfont_get_id CONFIG_FLUID_SFONT_GET_ID )
  # Check for fluid_sfont_get_name function.
  check_function_exists ( fluid_sfont_get_name CONFIG_FLUID_SFONT_GET_NAME )
  # Check for fluid_sfont_iteration_start function.
  check_function_exists ( fluid_sfont_iteration_start CONFIG_FLUID_SFONT_ITERATION_START )
  # Check for fluid_sfont_iteration_next function.
  check_function_exists ( fluid_sfont_iteration_next CONFIG_FLUID_SFONT_ITERATION_NEXT )
  # Check for fluid_synth_get_chorus_speed function.
  check_function_exists ( fluid_synth_get_chorus_speed CONFIG_FLUID_SYNTH_GET_CHORUS_SPEED )
  # Check for fluid_synth_get_chorus_depth function.
  check_function_exists ( fluid_synth_get_chorus_depth CONFIG_FLUID_SYNTH_GET_CHORUS_DEPTH )
  # Check for FluidSynth API V2 (>= 2.0.0) specifics...
  if (CONFIG_FLUID_SFONT_ITERATION_START AND CONFIG_FLUID_SFONT_ITERATION_NEXT)
    # Check for fluid_settings_getnum_default function.
    check_function_exists ( fluid_settings_getnum_default CONFIG_FLUID_SETTINGS_GETNUM_DEFAULT )
    # Check for fluid_settings_getint_default function.
    check_function_exists ( fluid_settings_getint_default CONFIG_FLUID_SETTINGS_GETINT_DEFAULT )
    # Check for fluid_settings_getstr_default function.
    check_function_exists ( fluid_settings_getstr_default CONFIG_FLUID_SETTINGS_GETSTR_DEFAULT )
    # Check for fluid_settings_foreach function.
    check_function_exists ( fluid_settings_foreach CONFIG_FLUID_SETTINGS_FOREACH )
    # Check for fluid_settings_foreach_option function.
    check_function_exists ( fluid_settings_foreach_option CONFIG_FLUID_SETTINGS_FOREACH_OPTION )
    # Check for new_fluid_server function.
    check_function_exists ( new_fluid_server CONFIG_NEW_FLUID_SERVER )
  endif ()
else ()
  message (FATAL_ERROR "fluidsynth library not found")
endif ()

add_subdirectory (src)

configure_file (qsynth.spec.in qsynth.spec IMMEDIATE @ONLY)

# Configuration status
macro (SHOW_OPTION text value)
  if (${value})
    message( "${text}: yes" )
  else ()
    message( "${text}: no" )
  endif ()
endmacro ()

message ( "\n  ${PACKAGE_NAME} ${PACKAGE_VERSION}" )
message ( "\n  Build target . . . . . . . . . . . . . . . . . . .: ${CONFIG_BUILD_TYPE}\n" )
show_option ( "  FluidSynth library support . . . . . . . . . . . ." FLUIDSYNTH_LIBRARY )
show_option ( "  FluidSynth server support  . . . . . . . . . . . ." CONFIG_FLUID_SERVER )
show_option ( "  FluidSynth system reset support . .  . . . . . . ." CONFIG_FLUID_SYSTEM_RESET )
show_option ( "  FluidSynth bank offset support . . . . . . . . . ." CONFIG_FLUID_BANK_OFFSET )
show_option ( "  FluidSynth channel info support (DEPRECATED) . . ." CONFIG_FLUID_CHANNEL_INFO )
show_option ( "  FluidSynth MIDI router support  (DEPRECATED) . . ." CONFIG_FLUID_MIDI_ROUTER )
show_option ( "  FluidSynth unset program support . . . . . . . . ." CONFIG_FLUID_UNSET_PROGRAM )
show_option ( "  FluidSynth version string support  . . . . . . . ." CONFIG_FLUID_VERSION_STR )
message ( "\n" )
show_option ( "  System tray icon support . . . . . . . . . . . . ." CONFIG_SYSTEM_TRAY )
show_option ( "  X11 Unique/Single instance . . . . . . . . . . . ." CONFIG_XUNIQUE )
show_option ( "  Gradient eye-candy . . . . . . . . . . . . . . . ." CONFIG_GRADIENT )
show_option ( "  Debugger stack-trace (gdb) . . . . . . . . . . . ." CONFIG_STACKTRACE )
message ( "\n  Install prefix . . . . . . . . . . . . . . . . . .: ${CMAKE_INSTALL_PREFIX}" )
message ( "\nNow type 'make', followed by 'make install' as root.\n" )
