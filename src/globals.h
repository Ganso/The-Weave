#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Debug settings
#define DEBUG_ON
#define DEBUG_STATE_MACHINES 1

// Version macro
#define GAMEVERSION ({ static char version[20]; sprintf(version, "v%c%c%c%c%c%c%c", (__DATE__[4] == ' ' ? '0' : __DATE__[4]), __DATE__[5], __DATE__[0], __DATE__[1], __DATE__[2], __DATE__[9], __DATE__[10]); version; })

// Core includes
#include <genesis.h>

// Constants and types
#include "game_constants.h"

// Debug tools
#ifdef DEBUG_ON
#include "KDebug.h"
#include "tools.h"
#include "timer.h"
#endif

// Core systems
#include "state_machine.h"    // State machine framework
#include "message_system.h"   // Message passing system

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

// Global variables
extern u16 tile_ind;        // Tiles index
extern u16 frame_counter;   // Frame counter for random numbers and frameskip

// Entity implementations
#include "entity.h"          // Base entity system
#include "characters.h"      // Character entities
#include "enemies.h"         // Enemy entities
#include "items.h"           // Item entities

// Combat system
#include "combat_states.h"   // Combat state machine
#include "enemy_pattern_states.h" // Enemy pattern state machine
#include "character_patterns.h" // Player patterns
#include "enemies_patterns.h" // Enemy patterns
#include "combat.h"          // Combat core

// Game systems
#include "texts.h"           // Text strings
#include "dialogs.h"         // Dialog system
#include "controller.h"      // Input handling
#include "interface.h"       // UI system
#include "background.h"      // Background handling
#include "collisions.h"      // Collision detection
#include "sound.h"           // Audio system

// Main game libraries
#include "init.h"            // Initialization functions
#include "intro.h"           // Game intro
#include "geesebumps.h"      // Geesebumps logo
#include "act_1.h"           // Act 1

// Screen constants
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
extern u8 SCREEN_FPS;

// Game state globals
extern u8 current_act;
extern u8 current_scene;

// Global functions
void wait_seconds(int sec);                    // Wait for N seconds
void next_frame(bool interactive);             // Wait for next frame and handle input
u16 calc_ticks(u16 milliseconds);             // Convert milliseconds to ticks

#endif // _GLOBALS_H_
