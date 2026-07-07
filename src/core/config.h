#ifndef _CONFIG_H_
#define _CONFIG_H_

// Global engine configuration: screen constants, version string and debug output.
// SCREEN_FPS is NOT here: it's a runtime variable (PAL/NTSC detection) in core/frame.h.

#include <genesis.h>
#include "core/hack.h"   // DEBUG_LEVEL y toggles de desarrollo

// Global definitions (NTSC)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224

// Create version using compile date
#define GAMEVERSION ({ static char version[20]; sprintf(version, "v%c%c%c%c%c%c%c", (__DATE__[4] == ' ' ? '0' : __DATE__[4]), __DATE__[5], __DATE__[0], __DATE__[1], __DATE__[2], __DATE__[9], __DATE__[10]); version; })

// Debug (DEBUG_LEVEL se configura en core/hack.h)
#define dprintf(level, ...) ((void)0)
#if DEBUG_LEVEL > 0
  #undef dprintf
  #include "kdebug.h"
  #include "tools.h"
  #include "timer.h"
  #define dprintf(level, ...) do { if ((level) <= DEBUG_LEVEL) kprintf(__VA_ARGS__); } while (0)
#endif

#endif
