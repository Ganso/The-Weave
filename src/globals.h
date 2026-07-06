#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// TRANSITIONAL umbrella header (Fase 3 del refactor).
// Solo lo incluyen los módulos condenados: patterns.c, patterns/*.c y act_1.c,
// que las Fases 4 y 5 reescriben con includes explícitos.
// NO añadir nuevos consumidores. Desaparece al final de la Fase 5.

// Sega Genesis
#include <genesis.h>

// Configuración y debug (dprintf, GAMEVERSION, SCREEN_WIDTH/HEIGHT)
#include "core/config.h"
#include "core/hack.h"

// Resources
#include "resources.h"
#include "res_backgrounds.h"
#include "res_characters.h"
#include "res_faces.h"
#include "res_enemies.h"
#include "res_items.h"
#include "res_interface.h"
#include "res_sound.h"
#include "res_geesebumps.h"
#include "res_intro.h"
#include "res_dialogs.h"

// Núcleo
#include "core/frame.h"       // next_frame, wait_seconds, calc_ticks, frame_counter, SCREEN_FPS, current_act/scene
#include "core/init.h"        // initialize, new_level, end_level
#include "core/controller.h"  // joy_check, wait_for_followers

// Auxiliary game libraries
#include "actors/entity.h"
#include "combat/combat.h"
#include "actors/characters.h"
#include "actors/enemies.h"
#include "spells/spell.h"
#include "spells/notes.h"
#include "actors/items.h"
#include "narrative/texts.h"
#include "narrative/texts_data.h"
#include "narrative/encode.h"
#include "narrative/dialogs.h"
#include "interface/interface.h"
#include "world/background.h"
#include "actors/collisions.h"
#include "audio/sound.h"

// Main game libraries
#include "scenes/intro.h"
#include "scenes/geesebumps.h"
#include "act_1.h"

#endif
