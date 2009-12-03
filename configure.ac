# Process this file with autoconf to produce a configure script.
AC_INIT(Qsynth, 0.3.4.11, rncbc@rncbc.org, qsynth)

AC_CONFIG_SRCDIR(src/qsynthMainForm.ui)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES(Makefile qsynth.pro qsynth.spec qsynth.desktop)

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
ac_prefix=$prefix
if test "x$ac_prefix" = "xNONE"; then
    ac_prefix=$ac_default_prefix
fi
AC_SUBST(ac_prefix)
AC_DEFINE_UNQUOTED(CONFIG_PREFIX, ["$ac_prefix"], [Default installation prefix.])

# Enable debugging argument option.
AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug], [enable debugging (default=no)]),
  [ac_debug="$enableval"])

if test "x$ac_debug" = "xyes"; then
   AC_DEFINE(CONFIG_DEBUG, 1, [Define if debugging is enabled.])
   ac_stacktrace="yes"
   ac_debug="debug"
else
   ac_stacktrace="no"
   ac_debug="release"
fi
AC_SUBST(ac_debug)

# Enable system tray argument option.
AC_ARG_ENABLE(system_tray,
  AC_HELP_STRING([--enable-system-tray], [enable system tray (default=yes)]),
  [ac_system_tray="$enableval"],
  [ac_system_tray="yes"])

# Enable gradient eye-candy.
AC_ARG_ENABLE(gradient,
  AC_HELP_STRING([--enable-gradient], [enable gradient eye-candy (default=yes)]),
  [ac_gradient="$enableval"],
  [ac_gradient="yes"])

# Enable debugger stack-trace option (assumes --enable-debug).
AC_ARG_ENABLE(stacktrace,
  AC_HELP_STRING([--enable-stacktrace], [enable debugger stack-trace (default=no)]),
  [ac_stacktrace="$enableval"])


# Standard installation base dirs.
ac_with_paths="/usr /usr/local"

# Some a-la-debian alternatives...
for X in /usr/lib /usr/lib64 /usr/share; do
  for Y in qt qt4; do
    if test -d $X/$Y/bin; then
      ac_with_paths="$ac_with_paths $X/$Y"
    fi
  done
done

# Set for alternate Qt installation dir.
AC_ARG_WITH(qt,
  AC_HELP_STRING([--with-qt=PATH], [use alternate Qt install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate fluidsynth installation dir.
AC_ARG_WITH(fluidsynth,
  AC_HELP_STRING([--with-fluidsynth=PATH], [use alternate fluidsynth install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_GCC_TRADITIONAL

# Checks for languages.
AC_LANG_C
AC_LANG_CPLUSPLUS

# Prepend alternate dependencies paths.
ac_path=$PATH
for X in $ac_with_paths; do
  if test -d $X/bin; then
    ac_path="$X/bin:$ac_path"
  fi
  if test -x $X/qmake; then
    ac_path="$X:$ac_path"
  fi
  if test -d $X/include; then
    for Y in qt qt4; do
      if test -d $X/include/$Y; then
        CFLAGS="-I$X/include/$Y $CFLAGS"
        CPPFLAGS="-I$X/include/$Y $CPPFLAGS"
        ac_incpath="$X/include/$Y $ac_incpath"
      fi
    done
    CFLAGS="-I$X/include $CFLAGS"
    CPPFLAGS="-I$X/include $CPPFLAGS"
    ac_incpath="$X/include $ac_incpath"
  fi
  if test -d $X/lib64; then
    LIBS="-L$X/lib64 $LIBS"
    ac_libs="-L$X/lib64 $ac_libs"
  fi
  if test -d $X/lib; then
    LIBS="-L$X/lib $LIBS"
    ac_libs="-L$X/lib $ac_libs"
  fi
done

# Check for proper Qt version.
AC_CACHE_CHECK([for Qt library version >= 4.1],
  ac_cv_qtversion, [
  AC_TRY_LINK([#include "Qt/qglobal.h"], [
#if QT_VERSION < 0x040100
#error Qt library 4.1 or greater required.
#endif
    ], ac_cv_qtversion="yes", [
      echo "no; Qt 4.1 or greater is required"
      exit
    ])
])

# A common error message:
ac_errmsg="not found in current PATH. Maybe Qt development environment isn't available (qt-devel)."

# Check for Qt qmake utility.
AC_PATH_PROG(ac_qmake, qmake, [no], $ac_path)
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi
AC_SUBST(ac_qmake)

# Check for Qt moc utility.
AC_PATH_PROG(ac_moc, moc, [no], $ac_path)
if test "x$ac_moc" = "xno"; then
   AC_MSG_ERROR([moc $ac_errmsg])
fi
AC_SUBST(ac_moc)

# Check for Qt uic utility.
AC_PATH_PROG(ac_uic, uic, [no], $ac_path)
if test "x$ac_uic" = "xno"; then
   AC_MSG_ERROR([uic $ac_errmsg])
fi
AC_SUBST(ac_uic)

# Check for Qt lupdate utility.
AC_PATH_PROG(ac_lupdate, lupdate, [no], $ac_path)
if test "x$ac_lupdate" = "xno"; then
   AC_MSG_ERROR([lupdate $ac_errmsg])
fi
AC_SUBST(ac_lupdate)

# Check for Qt lrelease utility.
AC_PATH_PROG(ac_lrelease, lrelease, [no], $ac_path)
if test "x$ac_release" = "xno"; then
   AC_MSG_ERROR([lrelease $ac_errmsg])
fi
AC_SUBST(ac_lrelease)

# Checks for libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(X11, main)
AC_CHECK_LIB(Xext, main)

# Check for round math function.
AC_CHECK_LIB(m, lroundf, [ac_round="yes"], [ac_round="no"])
if test "x$ac_round" = "xyes"; then
   AC_DEFINE(CONFIG_ROUND, 1, [Define if round is available.])
fi

# Check for fluidsynth library.
AC_CHECK_LIB(fluidsynth, main, [ac_fluidsynth_lib="yes"], [ac_fluidsynth_lib="no"])
if test "x$ac_fluidsynth_lib" = "xno"; then
   AC_MSG_ERROR([fluidsynth library not found.])
else
   ac_libs="$ac_libs -lfluidsynth"
fi

AC_SUBST(ac_libs)


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h unistd.h signal.h)

# Check for fluidsynth headers.
AC_CHECK_HEADER(fluidsynth.h, [ac_fluidsynth_h="yes"], [ac_fluidsynth_h="no"])
if test "x$ac_fluidsynth_h" = "xno"; then
   AC_MSG_ERROR([fluidsynth headers not found.])
fi

# Check for gradient eye-candy.
if test "x$ac_gradient" = "xyes"; then
   AC_DEFINE(CONFIG_GRADIENT, 1, [Define if gradient eye-candy is enabled.])
fi

# Check for debugging stack-trace.
if test "x$ac_stacktrace" = "xyes"; then
   AC_DEFINE(CONFIG_STACKTRACE, 1, [Define if debugger stack-trace is enabled.])
fi

AC_SUBST(ac_incpath)


# Checks for typedefs, structures, and compiler characteristics.
# AC_C_CONST

# Checks for library functions.
AC_CHECK_FUNCS(system)

# Check for system-tray icon.
if test "x$ac_system_tray" = "xyes"; then
   AC_DEFINE(CONFIG_SYSTEM_TRAY, 1, [Define if system tray is enabled.])
fi

# Check for new_fluid_server function.
AC_CHECK_LIB(fluidsynth, new_fluid_server, [ac_fluid_server="yes"], [ac_fluid_server="no"])
if test "x$ac_fluid_server" = "xyes"; then
   AC_DEFINE(CONFIG_FLUID_SERVER, 1, [Define if new_fluid_server is available.])
fi

# Check for fluid_synth_system_reset function.
AC_CHECK_LIB(fluidsynth, fluid_synth_system_reset, [ac_fluid_reset="yes"], [ac_fluid_reset="no"])
if test "x$ac_fluid_reset" = "xyes"; then
   AC_DEFINE(CONFIG_FLUID_RESET, 1, [Define if fluid_synth_system_reset is available.])
fi

# Check for fluid_synth_set_bank_offset function.
AC_CHECK_LIB(fluidsynth, fluid_synth_set_bank_offset, [ac_fluid_bank_offset="yes"], [ac_fluid_bank_offset="no"])
if test "x$ac_fluid_bank_offset" = "xyes"; then
   AC_DEFINE(CONFIG_FLUID_BANK_OFFSET, 1, [Define if fluid_synth_set_bank_offset is available.])
fi

# Check for fluid_synth_get_channel_info function.
AC_CHECK_LIB(fluidsynth, fluid_synth_get_channel_info, [ac_fluid_channel_info="yes"], [ac_fluid_channel_info="no"])
if test "x$ac_fluid_channel_info" = "xyes"; then
   AC_DEFINE(CONFIG_FLUID_CHANNEL_INFO, 1, [Define if fluid_synth_get_channel_info is available.])
fi

# Check for fluid_synth_unset_program function.
AC_CHECK_LIB(fluidsynth, fluid_synth_unset_program, [ac_fluid_unset_program="yes"], [ac_fluid_unset_program="no"])
if test "x$ac_fluid_unset_program" = "xyes"; then
   AC_DEFINE(CONFIG_FLUID_UNSET_PROGRAM, 1, [Define if fluid_synth_unset_program is available.])
fi

# Check for round math function.
AC_CHECK_LIB(m, round, [ac_round="yes"], [ac_round="no"])
if test "x$ac_round" = "xyes"; then
   AC_DEFINE(CONFIG_ROUND, 1, [Define if round is available.])
fi

# Finally produce a configure header file and the makefiles.
AC_OUTPUT

# make clean > /dev/null 2>&1

# Output summary message

echo
echo "  $PACKAGE_NAME $PACKAGE_VERSION"
echo
echo "  Build target . . . . . . . . . . . . . . . . . . .: $ac_debug"
echo
echo "  FluisSynth library support . . . . . . . . . . . .: $ac_fluidsynth_lib"
echo "  FluisSynth server support  . . . . . . . . . . . .: $ac_fluid_server"
echo "  FluisSynth reset support . . . . . . . . . . . . .: $ac_fluid_reset"
echo "  FluisSynth bank offset support . . . . . . . . . .: $ac_fluid_bank_offset"
echo "  FluisSynth channel info support  . . . . . . . . .: $ac_fluid_channel_info"
echo "  FluisSynth unset program support . . . . . . . . .: $ac_fluid_unset_program"
echo "  System tray icon support . . . . . . . . . . . . .: $ac_system_tray"
echo
echo "  Gradient eye-candy . . . . . . . . . . . . . . . .: $ac_gradient"
echo "  Debugger stack-trace (gdb) . . . . . . . . . . . .: $ac_stacktrace"
echo
echo "  Install prefix . . . . . . . . . . . . . . . . . .: $ac_prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo
