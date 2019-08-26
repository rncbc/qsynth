#ifndef CONFIG_H
#define CONFIG_H

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "@PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "@PACKAGE_STRING@"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "@PACKAGE_VERSION@"

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT "@PACKAGE_BUGREPORT@"

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME "@PACKAGE_TARNAME@"

/* Define to the version of this package. */
#cmakedefine CONFIG_VERSION "@CONFIG_VERSION@"

/* Define to the build version of this package. */
#cmakedefine CONFIG_BUILD_VERSION "@CONFIG_BUILD_VERSION@"

/* Default installation prefix. */
#cmakedefine CONFIG_PREFIX "@CONFIG_PREFIX@"

/* Define to target installation dirs. */
#cmakedefine CONFIG_BINDIR "@CONFIG_BINDIR@"
#cmakedefine CONFIG_LIBDIR "@CONFIG_LIBDIR@"
#cmakedefine CONFIG_DATADIR "@CONFIG_DATADIR@"
#cmakedefine CONFIG_MANDIR "@CONFIG_MANDIR@"

/* Define if debugging is enabled. */
#cmakedefine CONFIG_DEBUG @CONFIG_DEBUG@

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H @HAVE_SIGNAL_H@

/* Define if lroundf is available. */
#cmakedefine CONFIG_ROUND @CONFIG_ROUND@

/* Define if X11 Unique/Single instance is enabled. */
#cmakedefine CONFIG_XUNIQUE @CONFIG_XUNIQUE@

/* Define if gradient eye-candy is enabled. */
#cmakedefine CONFIG_GRADIENT @CONFIG_GRADIENT@

/* Define if debugger stack-trace is enabled. */
#cmakedefine CONFIG_STACKTRACE @CONFIG_STACKTRACE@

/* Define if FluidSynth server support is enabled. */
#cmakedefine CONFIG_FLUID_SERVER @CONFIG_FLUID_SERVER@

/* Define if FluidSynth system reset support is enabled. */
#cmakedefine CONFIG_FLUID_SYSTEM_RESET @CONFIG_FLUID_SYSTEM_RESET@

/* Define if FluidSynth bank offset support is enabled. */
#cmakedefine CONFIG_FLUID_BANK_OFFSET @CONFIG_FLUID_BANK_OFFSET@

/* Define if FluidSynth channel info is enabled. */
#cmakedefine CONFIG_FLUID_CHANNEL_INFO @CONFIG_FLUID_CHANNEL_INFO@

/* Define if FluidSynth MIDI router is enabled. */
#cmakedefine CONFIG_FLUID_MIDI_ROUTER @CONFIG_FLUID_MIDI_ROUTER@

/* Define if FluidSynth unset program support is enabled. */
#cmakedefine CONFIG_FLUID_UNSET_PROGRAM @CONFIG_UNSET_PROGRAM@

/* Define if FluidSynth version string support is enabled. */
#cmakedefine CONFIG_FLUID_VERSION_STR @CONFIG_FLUID_VERSION_STR@

/* Define if fluid_settings_dupstr function is available. */
#cmakedefine CONFIG_FLUID_SETTINGS_DUPSTR @CONFIG_FLUID_SETTINGS_DUPSTR@

/* Define if fluid_preset_get_banknum function is available. */
#cmakedefine CONFIG_FLUID_PRESET_GET_BANKNUM @CONFIG_FLUID_PRESET_GET_BANKNUM@

/* Define if fluid_preset_get_num function is available. */
#cmakedefine CONFIG_FLUID_PRESET_GET_NUM @CONFIG_FLUID_PRESET_GET_NUM@

/* Define if fluid_preset_get_name function is available. */
#cmakedefine CONFIG_FLUID_PRESET_GET_NAME @CONFIG_FLUID_PRESET_GET_NAME@

/* Define if fluid_preset_get_sfont function is available. */
#cmakedefine CONFIG_FLUID_PRESET_GET_SFONT @CONFIG_FLUID_PRESET_GET_SFONT@

/* Define if fluid_sfont_get_id function is available. */
#cmakedefine CONFIG_FLUID_SFONT_GET_ID @CONFIG_FLUID_SFONT_GET_ID@

/* Define if fluid_sfont_get_name function is available. */
#cmakedefine CONFIG_FLUID_SFONT_GET_NAME @CONFIG_FLUID_SFONT_GET_NAME@

/* Define if fluid_sfont_iteration_start function is available. */
#cmakedefine CONFIG_FLUID_SFONT_ITERATION_START @CONFIG_FLUID_SFONT_ITERATION_START@

/* Define if fluid_sfont_iteration_next function is available. */
#cmakedefine CONFIG_FLUID_SFONT_ITERATION_NEXT @CONFIG_FLUID_SFONT_ITERATION_NEXT@

/* Define if fluid_synth_get_chorus_speed function is available. */
#cmakedefine CONFIG_FLUID_SYNTH_GET_CHORUS_SPEED @CONFIG_FLUID_SYNTH_GET_CHORUS_SPEED@

/* Define if fluid_synth_get_chorus_depth function is available. */
#cmakedefine CONFIG_FLUID_SYNTH_GET_CHORUS_DEPTH @CONFIG_FLUID_SYNTH_GET_CHORUS_DEPTH@

/* Define if fluid_settings_getnum_default function is available. */
#cmakedefine CONFIG_FLUID_SETTINGS_GETNUM_DEFAULT @CONFIG_FLUID_SETTINGS_GETNUM_DEFAULT@

/* Define if fluid_settings_getint_default function is available. */
#cmakedefine CONFIG_FLUID_SETTINGS_GETINT_DEFAULT @CONFIG_FLUID_SETTINGS_GETINT_DEFAULT@

/* Define if fluid_settings_getstr_default function is available. */
#cmakedefine CONFIG_FLUID_SETTINGS_GETSTR_DEFAULT @CONFIG_FLUID_SETTINGS_GETSTR_DEFAULT@

/* Define if fluid_settings_foreach function is available. */
#cmakedefine CONFIG_FLUID_SETTINGS_FOREACH @CONFIG_FLUID_SETTINGS_FOREACH@

/* Define if fluid_settings_foreach_option function is available. */
#cmakedefine CONFIG_FLUID_SETTINGS_FOREACH_OPTION @CONFIG_FLUID_SETTINGS_FOREACH_OPTION@

/* Define if new_fluid_server function is available. */
#cmakedefine CONFIG_NEW_FLUID_SERVER @CONFIG_NEW_FLUID_SERVER@

/* Define if system tray is enabled. */
#cmakedefine CONFIG_SYSTEM_TRAY @CONFIG_SYSTEM_TRAY@

#endif /* CONFIG_H */
