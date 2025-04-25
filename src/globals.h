#ifndef _GLOBALS_H_
#define _GLOBALS_H_


#define DEBUG_ON

// Create version using compile date
#define GAMEVERSION ({ static char version[20]; sprintf(version, "v%c%c%c%c%c%c%c", (__DATE__[4] == ' ' ? '0' : __DATE__[4]), __DATE__[5], __DATE__[0], __DATE__[1], __DATE__[2], __DATE__[9], __DATE__[10]); version; })

// Sega Genesis
#include <genesis.h>

// Debug
#ifdef DEBUG_ON
#include "KDebug.h"
#include "tools.h"
#include "timer.h"
#include "test_config.h"      // Test configuration
#endif

// Resources
#include "../res/resources.h"
#include "../res/res_backgrounds.h"
#include "../res/res_characters.h"
#include "../res/res_faces.h"
#include "../res/res_enemies.h"
#include "../res/res_items.h"
#include "../res/res_interface.h"
#include "../res/res_sound.h"
#include "../res/res_geesebumps.h"
#include "../res/res_intro.h"

extern u16 tile_ind; // Tiles index
extern u16 frame_counter; // Number of frames counter (random number generator, and frameskip counter in some functions)

// Auxiliary game libraries
#include "entity.h" // Every object in the game that has a sprite you can show, move...
#include "patterns.h" // Pattern structure and definitions
#include "statemachine.h" // State machine library
#include "characters.h" // Characters that can talk, or you can control
#include "character_patterns.h" // Paterns the player can play and use
#include "enemies_patterns.h" // Patterns the enemies can play and use
#include "enemies.h" // Enemies you fight
#include "items.h" // Items in the scenery
#include "texts.h" // Text strings (English / Spanish)
#include "dialogs.h" // Dialog and text related functions
#include "controller.h" // Controller related functions
#include "interface.h" // Game interface
#include "combat.h" // Combat related functions (including patterns enemy can use)
#include "background.h" // Background and scenery related objects and functions
#include "collisions.h" // Distance and collisions related functions
#include "sound.h" // Music and SFX

#ifdef DEBUG_ON
#include "test_statemachine.h" // State machine tests
#endif

// Main game libraries
#include "init.h" // Initialization functions
#include "intro.h" // Game intro
#include "geesebumps.h" // Geesebumps logo
#include "act_1.h" // Act 1

// Global definitions (NTSC)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
extern u8 SCREEN_FPS;

// Game state globals
extern u8 current_act;
extern u8 current_scene;

// Global functions
void wait_seconds(int sec); // Wait for N seconds
void next_frame(bool interactive); // Wait for next frame and do each-frame actions, including interactive actions if selected
u16 calc_ticks(u16 milliseconds); // Translate millisecons to ticks

#endif
