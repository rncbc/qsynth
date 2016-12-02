#ifndef CONFIG_H
#define CONFIG_H

/* Define to the build version of this package. */
#cmakedefine CONFIG_BUILD_VERSION "@CONFIG_BUILD_VERSION@"

/* Default installation prefix. */
#cmakedefine CONFIG_PREFIX "@CONFIG_PREFIX@"

/* Define if debugging is enabled. */
#cmakedefine CONFIG_DEBUG @CONFIG_DEBUG@

/* Define if round is available. */
#cmakedefine CONFIG_ROUND @CONFIG_ROUND@

/* Define if fluid_synth_set_bank_offset is available. */
#cmakedefine CONFIG_FLUID_BANK_OFFSET @CONFIG_FLUID_BANK_OFFSET@

/* Define if fluid_synth_get_channel_info is available. */
#cmakedefine CONFIG_FLUID_CHANNEL_INFO @CONFIG_FLUID_CHANNEL_INFO@

/* Define if fluid_synth_system_reset is available. */
#cmakedefine CONFIG_FLUID_RESET @CONFIG_FLUID_RESET@

/* Define if new_fluid_server is available. */
#cmakedefine CONFIG_FLUID_SERVER @CONFIG_FLUID_SERVER@

/* Define if fluid_synth_unset_program is available. */
#cmakedefine CONFIG_FLUID_UNSET_PROGRAM @CONFIG_FLUID_UNSET_PROGRAM@

/* Define if fluid_version_str is available. */
#cmakedefine CONFIG_FLUID_VERSION_STR @CONFIG_FLUID_VERSION_STR@

/* Define if system tray is enabled. */
#cmakedefine CONFIG_SYSTEM_TRAY @CONFIG_SYSTEM_TRAY@

/* Define if X11 Unique/Single instance is enabled. */
#cmakedefine CONFIG_XUNIQUE @CONFIG_XUNIQUE@

/* Define if gradient eye-candy is enabled. */
#cmakedefine CONFIG_GRADIENT @CONFIG_GRADIENT@

/* Define if debugger stack-trace is enabled. */
#cmakedefine CONFIG_STACKTRACE @CONFIG_STACKTRACE@

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H @HAVE_SIGNAL_H@

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT "@PACKAGE_BUGREPORT@"

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "@PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "@PACKAGE_STRING@"

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME "@PACKAGE_TARNAME@"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "@PACKAGE_VERSION@"

#endif /* CONFIG_H */
