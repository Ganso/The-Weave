#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Sega Genesis
#include <genesis.h>

// Debug
#include "KDebug.h"
#include "tools.h"
#include "timer.h"

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

u16 tile_ind; // Tiles index
u16 frame_counter; // Number of frames counter (random number generator, and frameskip counter in some functions)

// Game libraries
#include "entity.h" // Every object in the game that has a sprite you can show, move...
#include "characters.h" // Characters that can talk, or you can control
#include "enemies.h" // Enemies you fight
#include "items.h" // Items in the scenery
#include "texts.h" // Text strings (English / Spanish)
#include "dialogs.h" // Dialog and text related functions
#include "controller.h" // Controller related functions
#include "interface.h" // Game interface
#include "patterns.h" // Paterns the player can play and use
#include "combat.h" // Combat related functions (including patterns enemy can use)
#include "background.h" // Background and scenery related objects and functions

// Main game libraries
#include "init.h" // Initialization functions
#include "intro.h" // Game intro
#include "act_1.h" // Act 1

// Global definitions (NTSC)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define SCREEN_FPS 60

// Game state globals
u8 current_act;
u8 current_scene;

// Global functions
void wait_seconds(int sec); // Wait for N seconds
void next_frame(bool interactive); // Wait for next frame and do each-frame actions, including interactive actions if selected
u16 calc_ticks(u16 milliseconds); // Translate millisecons to ticks

#endif
