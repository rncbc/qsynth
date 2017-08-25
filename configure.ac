# Process this file with autoconf to produce a configure script.
AC_INIT(Qsynth, 0.4.5, rncbc@rncbc.org, qsynth)

AC_CONFIG_SRCDIR(src/qsynth.cpp)
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_FILES(Makefile qsynth.spec src/src.pri)

# Build version string.
AC_CACHE_VAL([ac_cv_build_version], [
   ac_cv_build_version=$(git describe --tags --dirty --abbrev=6 2>/dev/null)
   if test -n "$ac_cv_build_version"; then
      ac_cv_build_version=$(echo $ac_cv_build_version | sed 's/^[[^_]]*[[_v]]//i')
      ac_cv_build_version=$(echo $ac_cv_build_version | sed 's/-g/git./')
      ac_cv_build_version=$(echo $ac_cv_build_version | sed 's/[[_|-]]\+/./g')
      ac_cv_build_version_extra=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
      if test "x$ac_cv_build_version_extra" != "xmaster"; then
         ac_cv_build_version="$ac_cv_build_version [[$ac_cv_build_version_extra]]"
      fi
   else
      ac_cv_build_version=$PACKAGE_VERSION
   fi
])
ac_build_version="$ac_cv_build_version"
AC_DEFINE_UNQUOTED(CONFIG_BUILD_VERSION, ["$ac_build_version"], [Build version string.])

# Sanitized version string.
AC_CACHE_VAL([ac_cv_version], [
   ac_cv_version=$(echo $PACKAGE_VERSION | sed -r 's/^([[0-9|\.]]+).*$/\1/')
])
ac_version="$ac_cv_version"
AC_DEFINE_UNQUOTED(CONFIG_VERSION, ["$ac_version"], [Version string.])
AC_SUBST(ac_version)

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
if test "x$prefix" = "xNONE"; then
   prefix=$ac_default_prefix
fi
if test "x$exec_prefix" = "xNONE"; then
   exec_prefix=$prefix
fi
eval ac_prefix=$prefix
AC_SUBST(ac_prefix)
AC_DEFINE_UNQUOTED(CONFIG_PREFIX, ["$ac_prefix"], [Default installation prefix.])

# Set default installation directories.
eval ac_bindir=$bindir
AC_SUBST(ac_bindir)
AC_DEFINE_UNQUOTED(CONFIG_BINDIR, ["$ac_bindir"], [Default executable binary path.])

eval ac_libdir=$libdir
AC_SUBST(ac_libdir)
AC_DEFINE_UNQUOTED(CONFIG_LIBDIR, ["$ac_libdir"], [Default object library path.])

eval datarootdir=$datarootdir
eval ac_datadir=$datadir
AC_SUBST(ac_datadir)
AC_DEFINE_UNQUOTED(CONFIG_DATADIR, ["$ac_datadir"], [Default arch-idependent data path.])

eval ac_mandir=$mandir
AC_SUBST(ac_mandir)
AC_DEFINE_UNQUOTED(CONFIG_MANDIR, ["$ac_mandir"], [Default man page path.])


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


# Enable Qt4/5 availability.
AC_ARG_ENABLE(qt4,
  AC_HELP_STRING([--enable-qt4], [enable Qt4 build (default=no)]),
  [ac_qt4="$enableval"],
  [ac_qt4="no"])

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

# Enable X11 unique/single instance.
AC_ARG_ENABLE(xunique,
  AC_HELP_STRING([--enable-xunique], [enable X11 unique/single instance (default=yes)]),
  [ac_xunique="$enableval"],
  [ac_xunique="yes"])

# Enable debugger stack-trace option (assumes --enable-debug).
AC_ARG_ENABLE(stacktrace,
  AC_HELP_STRING([--enable-stacktrace], [enable debugger stack-trace (default=no)]),
  [ac_stacktrace="$enableval"])


# Standard installation base dirs.
ac_with_paths=""

# Set for alternate Qt4/5 installation dir.
AC_ARG_WITH(qt4,
  AC_HELP_STRING([--with-qt4=PATH], [use alternate Qt4 install path]),
  [ac_qt4_path="$withval"], [ac_qt4_path="no"])

AC_ARG_WITH(qt5,
  AC_HELP_STRING([--with-qt5=PATH], [use alternate Qt5 install path]),
  [ac_qt5_path="$withval"], [ac_qt5_path="no"])

if test "x$ac_qt4_path" != "xno"; then
   ac_with_paths="$ac_with_paths $ac_qt4_path"
   ac_qt4="yes"
fi

if test "x$ac_qt5_path" != "xno"; then
   ac_with_paths="$ac_with_paths $ac_qt5_path"
   ac_qt4="no"
fi

# Set for alternate fluidsynth installation dir.
AC_ARG_WITH(fluidsynth,
  AC_HELP_STRING([--with-fluidsynth=PATH], [use alternate fluidsynth install path]),
  [ac_with_paths="$ac_with_paths $withval"])


# Honor user specified flags.
ac_cflags=$CFLAGS
ac_ldflags=$LDFLAGS


# Checks for programs.
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP

# Checks for languages.
AC_LANG_CPLUSPLUS

# Check for pkg-config.
PKG_PROG_PKG_CONFIG

# Check whether -std=c++11 support is necessary (4 < g++ version < 6).
AC_CACHE_CHECK([for g++ major version], [ac_cv_gxx_version_major], [
   ac_cv_gxx_version_major=$($CC -dumpversion)
   if test -n "$ac_cv_gxx_version_major"; then
      ac_cv_gxx_version_major=$(echo $ac_cv_gxx_version_major | cut -d'.' -f1);
   fi
   ac_gxx_version_major=$(($ac_cv_gxx_version_major + 0))
])
ac_gxx_version_major=$ac_cv_gxx_version_major
if test $ac_gxx_version_major -ge 4 -a $ac_gxx_version_major -lt 6; then
   CXXFLAGS="-std=c++11 $CXXFLAGS"
fi

# Check for proper flags.
ac_arch=$(uname -m)

# Check for install paths and alternatives...
ac_libdirs="lib"

if test "x$ac_arch" = "xx86_64"; then
   ac_libdirs="$ac_libdirs lib64"
fi

CFLAGS="-fPIC $CFLAGS"
CPPFLAGS="-fPIC $CPPFLAGS"

# Prepend alternate dependencies paths.
ac_path=$PATH

for X in $ac_with_paths; do
  if test -d $X/bin; then
    ac_path="$X/bin:$ac_path"
  fi
  if test -d $X/include; then
    CFLAGS="-I$X/include $CFLAGS "
    CPPFLAGS="-I$X/include $CPPFLAGS"
    ac_incpath="$X/include $ac_incpath"
  fi
  for Y in $ac_libdirs; do
    if test -d $X/$Y; then
       LIBS="-L$X/$Y $LIBS"
       ac_libs="-L$X/$Y $ac_libs"
    fi
  done
done

# A common error message:
ac_errmsg="not found in current PATH. Maybe QT development environment isn't available."

if test "x$ac_qt4" = "xyes"; then
   AC_PATH_PROG(ac_qmake, qmake-qt4, [no], $ac_path)
else
   AC_PATH_PROG(ac_qmake, qmake-qt5, [no], $ac_path)
fi
if test "x$ac_qmake" = "xno"; then
   AC_PATH_PROG(ac_cv_qmake, qmake, [no], $ac_path)
   ac_qmake=$ac_cv_qmake
fi
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi

# Check for proper Qt major version.
AC_CACHE_CHECK([for Qt major version], [ac_cv_qt_version_major], [
   ac_cv_qt_version_major=$($ac_qmake -query QT_VERSION | cut -d'.' -f1)
   ac_cv_qt_version_major=$(($ac_cv_qt_version_major + 0))
])
ac_qt_version_major=$ac_cv_qt_version_major
if test "x$ac_qt4" = "xyes"; then
   if test $ac_qt_version_major -ne 4; then
      AC_MSG_ERROR([qmake-qt4 $ac_errmsg (qt4-devel)])
   fi
else
   if test $ac_qt_version_major -ne 5; then
      AC_MSG_ERROR([qmake-qt5 $ac_errmsg (qt5-devel)])
   fi
fi

# Check for proper Qt install path.
AC_CACHE_CHECK([for Qt install path], [ac_cv_qt_install_path], [
   ac_cv_qt_install_path=$($ac_qmake -query QT_INSTALL_BINS)
])
ac_qt_install_path=$ac_cv_qt_install_path
if test -d $ac_qt_install_path; then
   ac_path="$ac_qt_install_path:$ac_path"
fi

# Check it again, now with updated PATH, just in case...
AC_PATH_PROG(ac_cv_qmake, qmake, [no], $ac_path)
ac_qmake=$ac_cv_qmake
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi

AC_SUBST(ac_qmake)

AC_CACHE_CHECK([for Qt install headers], [ac_cv_qt_install_headers], [
   ac_cv_qt_install_headers=$($ac_qmake -query QT_INSTALL_HEADERS)
])
ac_qt_install_headers=$ac_cv_qt_install_headers
if test -d $ac_qt_install_headers; then
   CFLAGS="-I$ac_qt_install_headers $CFLAGS "
   CPPFLAGS="-I$ac_qt_install_headers $CPPFLAGS"
   ac_incpath="$ac_qt_install_headers $ac_incpath"
fi

AC_CACHE_CHECK([for Qt install libraries], [ac_cv_qt_install_libs], [
   ac_cv_qt_install_libs=$($ac_qmake -query QT_INSTALL_LIBS)
])
ac_qt_install_libs=$ac_cv_qt_install_libs
if test -d $ac_qt_install_libs; then
   LIBS="-L$ac_qt_install_libs $LIBS"
   ac_libs="-L$ac_qt_install_libs $ac_libs"
fi

# Finally, check for proper Qt4/5 version.
if test "x$ac_qt4" = "xyes"; then
   AC_CACHE_CHECK([for Qt library version >= 4.4],
      ac_cv_qtversion, [
      AC_TRY_COMPILE([#include "QtCore/qglobal.h"], [
         #if QT_VERSION < 0x040400 || QT_VERSION >= 0x050000
         #error Qt library 4.4 or greater required.
         #endif
      ], ac_cv_qtversion="yes", [
         echo "no; Qt 4.4 or greater is required"
         exit 1
      ])
   ])
else
   AC_CACHE_CHECK([for Qt library version >= 5.1],
      ac_cv_qtversion, [
      AC_TRY_COMPILE([#include "QtCore/qglobal.h"], [
         #if QT_VERSION < 0x050100 || QT_VERSION >= 0x060000
         #error Qt library 5.1 or greater required.
         #endif
      ], ac_cv_qtversion="yes", [
         echo "no; Qt 5.1 or greater is required"
         exit 1
      ])
   ])
fi

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
PKG_CHECK_MODULES([FLUIDSYNTH], [fluidsynth >= 0.80.0], [ac_fluidsynth_lib="yes"], [ac_fluidsynth_lib="no"])
if test "x$ac_fluidsynth_lib" = "xyes"; then
   AC_DEFINE(CONFIG_FLUIDSYNTH, 1, [Define if FLUIDSYNTH library is available.])
   ac_cflags="$ac_cflags $FLUIDSYNTH_CFLAGS"
   ac_libs="$ac_libs $FLUIDSYNTH_LIBS"
   CFLAGS="$CFLAGS $FLUIDSYNTH_CFLAGS"
   CPPFLAGS="$CPPFLAGS $FLUIDSYNTH_CFLAGS"
   LIBS="$LIBS $FLUIDSYNTH_LIBS"
else
   AC_MSG_ERROR([*** FLUIDSYNTH library not found.])
fi


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h unistd.h signal.h)

# Check for X11 unique/single instance.
if test "x$ac_xunique" = "xyes" -a "x$ac_qt4" = "xno"; then
   PKG_CHECK_MODULES([QT5X11EXTRAS], [Qt5X11Extras], [ac_xunique="yes"], [ac_xunique="no"])
fi
if test "x$ac_xunique" = "xyes"; then
   AC_DEFINE(CONFIG_XUNIQUE, 1, [Define if X11 unique/single instance is enabled.])
   if test "x$ac_qt4" = "xno"; then
      ac_qx11extras="x11extras"
   fi
   # Some recent distros (eg. fedora, debian) require this.
   if test "x$ac_cv_lib_X11_main" = "xyes"; then
      ac_libs="$ac_libs -lX11"
   fi
fi
AC_SUBST(ac_qx11extras)

# Check for gradient eye-candy.
if test "x$ac_gradient" = "xyes"; then
   AC_DEFINE(CONFIG_GRADIENT, 1, [Define if gradient eye-candy is enabled.])
fi

# Check for debugging stack-trace.
if test "x$ac_stacktrace" = "xyes"; then
   AC_DEFINE(CONFIG_STACKTRACE, 1, [Define if debugger stack-trace is enabled.])
fi

AC_SUBST(ac_cflags)
AC_SUBST(ac_incpath)
AC_SUBST(ac_ldflags)
AC_SUBST(ac_libs)


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

# Check for fluid_version_str function.
AC_CHECK_LIB(fluidsynth, fluid_version_str, [ac_fluid_version_str="yes"], [ac_fluid_version_str="no"])
if test "x$ac_fluid_version_str" = "xyes"; then
   AC_DEFINE(CONFIG_FLUID_VERSION_STR, 1, [Define if fluid_version_str is available.])
fi

# Finally produce a configure header file and the makefiles.
AC_OUTPUT

# make clean > /dev/null 2>&1

# Output summary message

echo
echo "  $PACKAGE_NAME $ac_build_version"
echo
echo "  Build target . . . . . . . . . . . . . . . . . . .: $ac_debug"
echo
echo "  FluidSynth library support . . . . . . . . . . . .: $ac_fluidsynth_lib"
echo "  FluidSynth server support  . . . . . . . . . . . .: $ac_fluid_server"
echo "  FluidSynth reset support . . . . . . . . . . . . .: $ac_fluid_reset"
echo "  FluidSynth bank offset support . . . . . . . . . .: $ac_fluid_bank_offset"
echo "  FluidSynth channel info support  . . . . . . . . .: $ac_fluid_channel_info"
echo "  FluidSynth unset program support . . . . . . . . .: $ac_fluid_unset_program"
echo "  FluidSynth version string support  . . . . . . . .: $ac_fluid_version_str"
echo "  System tray icon support . . . . . . . . . . . . .: $ac_system_tray"
echo
echo "  X11 Unique/Single instance . . . . . . . . . . . .: $ac_xunique"
echo "  Gradient eye-candy . . . . . . . . . . . . . . . .: $ac_gradient"
echo "  Debugger stack-trace (gdb) . . . . . . . . . . . .: $ac_stacktrace"
echo
echo "  Install prefix . . . . . . . . . . . . . . . . . .: $ac_prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo

