project(QSYNTH)

cmake_minimum_required(VERSION 2.6)

set (VERSION "0.4.4")

set (PACKAGE_NAME "Qsynth")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_BUGREPORT "rncbc@rncbc.org")
set (PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set (PACKAGE_TARNAME "qsynth")

set (CONFIG_BUILD_VERSION "${PACKAGE_VERSION}")

set (CONFIG_DEBUG)
if (CMAKE_BUILD_TYPE MATCHES "debug")
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

# Check for Qt
set (QT_MIN_VERSION "5.1.0")
find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets X11Extras)
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
      set ( CMAKE_REQUIRED_LIBRARIES ${MATH_LIBRARY} )
      check_function_exists ( lroundf CONFIG_ROUND )
  else ()
      message (FATAL_ERROR "math library not found")
  endif ()
endif ()

if (CONFIG_XUNIQUE)
  find_library ( X11_LIBRARY X11 )
endif ()

# Check for fluidsynth library.
find_library ( FLUIDSYNTH_LIBRARY fluidsynth )

# Checks for header files.
check_include_files ("fcntl.h;unistd.h;signal.h" HAVE_SIGNAL_H)

# Check for fluidsynth headers.
find_path (FLUIDSYNTH_INCLUDEDIR NAMES fluidsynth.h)
set (CMAKE_REQUIRED_INCLUDES ${FLUIDSYNTH_INCLUDEDIR})
check_include_file ("fluidsynth.h" HAVE_FLUIDSYNTH_H)

if (FLUIDSYNTH_LIBRARY AND HAVE_FLUIDSYNTH_H)
    set(CMAKE_REQUIRED_LIBRARIES ${FLUIDSYNTH_LIBRARY})
    # new_fluid_server function.
    check_function_exists ( new_fluid_server CONFIG_FLUID_SERVER )
    # Check for fluid_synth_system_reset function.
    check_function_exists ( fluid_synth_system_reset CONFIG_FLUID_RESET )
    # Check for fluid_synth_set_bank_offset function.
    check_function_exists ( fluid_synth_set_bank_offset CONFIG_FLUID_BANK_OFFSET )
    # Check for fluid_synth_get_channel_info function.
    check_function_exists ( fluid_synth_get_channel_info CONFIG_FLUID_CHANNEL_INFO )
    # Check for fluid_synth_unset_program function.
    check_function_exists ( fluid_synth_unset_program CONFIG_FLUID_UNSET_PROGRAM )
    # Check for fluid_version_str function.
    check_function_exists ( fluid_version_str CONFIG_FLUID_VERSION_STR )
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
message ( "\n  Build target . . . . . . . . . . . . . . . . . . .: ${CMAKE_BUILD_TYPE}" )
show_option ( "\n  FluidSynth library support . . . . . . . . . . . ." FLUIDSYNTH_LIBRARY )
show_option ( "  FluidSynth server support  . . . . . . . . . . . ." CONFIG_FLUID_SERVER )
show_option ( "  FluidSynth reset support . . . . . . . . . . . . ." CONFIG_FLUID_RESET )
show_option ( "  FluidSynth bank offset support . . . . . . . . . ." CONFIG_FLUID_BANK_OFFSET )
show_option ( "  FluidSynth channel info support  . . . . . . . . ." CONFIG_FLUID_CHANNEL_INFO )
show_option ( "  FluidSynth unset program support . . . . . . . . ." CONFIG_FLUID_UNSET_PROGRAM )
show_option ( "  FluidSynth version string support  . . . . . . . ." CONFIG_FLUID_VERSION_STR )
show_option ( "  System tray icon support . . . . . . . . . . . . ." CONFIG_SYSTEM_TRAY )
show_option ( "\n  X11 Unique/Single instance . . . . . . . . . . . ." CONFIG_XUNIQUE )
show_option ( "  Gradient eye-candy . . . . . . . . . . . . . . . ." CONFIG_GRADIENT )
show_option ( "  Debugger stack-trace (gdb) . . . . . . . . . . . ." CONFIG_STACKTRACE )
message ( "\n  Install prefix . . . . . . . . . . . . . . . . . .: ${CMAKE_INSTALL_PREFIX}" )
message ( "\nNow type 'make', followed by 'make install' as root.\n" )
