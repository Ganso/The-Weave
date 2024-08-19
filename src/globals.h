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
#include "../res/res_interface.h"
#include "../res/res_sound.h"
#include "../res/res_geesebumps.h"

u16 tile_ind; // Tiles index
u16 random_seed; // Random number generator seed

// Game libraries
#include "entity.h"
#include "texts.h"
#include "init.h"
#include "act_1.h"
#include "dialogs.h"
#include "texts.h"
#include "characters.h"
#include "controller.h"
#include "interface.h"
#include "patterns.h"
#include "enemies.h"
#include "combat.h"
#include "background.h"
#include "intro.h"

// Global functions
void wait_seconds(int sec); // Wait for N seconds
void next_frame(void); // Wait for next frame and do each-frame actions

#endif
