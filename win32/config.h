
#define PACKAGE_NAME    "Qsynth"
#define PACKAGE_VERSION "0.3.1.13"

#define CONFIG_PREFIX   "."
#define CONFIG_DEBUG    1

#define CONFIG_SYSTEM_TRAY

#if !defined(__MINGW32__)
#undef  CONFIG_FLUID_SERVER
#undef  CONFIG_FLUID_RESET
#undef  CONFIG_FLUID_BANK_OFFSET
#undef  CONFIG_ROUND
#endif
