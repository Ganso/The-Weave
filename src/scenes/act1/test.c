// act1/test.c — hooks de la escena de PRUEBA del motor (la secuencia está en test.scene).
// Reutiliza íntegramente recursos existentes: nivel del pasillo, sprites del
// corridor y el WeaverGhost del bosque. Nada nuevo en res/.

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/act1/test.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "actors/enemies.h"
#include "actors/items.h"
#include "world/background.h"
#include "spells/spell.h"
#include "spells/notes.h"
#include "interface/interface.h"
#include "res_backgrounds.h"
#include "res_items.h"
#include "res_enemies.h"

void act1_test_setup(void)    // Nivel del pasillo + decorado mínimo + personajes + hechizos
{
    new_level(NULL, NULL, &historians_corridor_front_tile, &historians_corridor_front_map, historians_corridor_pal, 800, BG_SCRL_USER_LEFT, 0);
    set_limits(0,140,275,168);

    // Decorado reutilizado del corridor: lámparas y la "puerta del puzzle"
    init_item(0, &item_corridor_lamp_sprite, PAL0, 176, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(1, &item_corridor_lamp_sprite, PAL0, 464, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(2, &item_corridor_door_bottom_sprite, PAL0, 96, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND); // la puerta del puzzle

    init_character(CHR_linus);
    init_character(CHR_clio);        // para probar show/hide desde el DSL
    active_character = CHR_linus;

    // Vara + hechizos del test (fire incluido: primera vez casteable por el jugador)
    player_has_rod = true;
    spell_enable(SPELL_THUNDER);
    spell_enable(SPELL_HIDE);
    spell_enable(SPELL_FIRE);
}

void act1_test_ghost(void)    // Un WeaverGhost para el combate de prueba (patrón de forest)
{
    show_or_hide_interface(false);

    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA); // Enemy palette

    // Parar a todo el mundo antes de que aparezca
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(140));
}
