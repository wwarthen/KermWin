#define APP_TITLE       "Kermit DEC Emulator"

#define VER_MAJOR       0
#define VER_MINOR       85
#define VER_PATCH       0
#define VER_BUILD       8

#define VER_DESC        "0.85"
#define BLD_DESC        "8"

#ifdef _WIN32
#define BUILD_OS        "WIN32"
#else
#define BUILD_OS        "WIN16"
#endif

#define REL_TYPE        "Prerelease"
#define BUILD_DATE      __DATE__

#ifdef DEBUG
#define BUILD_TYPE      "Debug"
#else
#define BUILD_TYPE
#endif

#define COPYRIGHT       "Copyright © 1990-98 Wayne Warthen"

#define CONTACT         "Internet: WWarthen@WWarthen.com\n" \
                        "CompuServe: 73457,2401"
