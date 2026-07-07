// act1/test.c — hooks de la escena de PRUEBA del motor (la secuencia está en test.scene).
// Reutiliza recursos existentes: fondo de bosque + el WeaverGhost. Nada nuevo en res/.

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/act1/test.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "actors/enemies.h"
#include "world/background.h"
#include "spells/spell.h"
#include "spells/notes.h"
#include "interface/interface.h"
#include "res_backgrounds.h"
#include "res_enemies.h"

void act1_test_setup(void)    // Fondo de bosque + personajes visibles + vara y hechizos del test
{
    // Misma config que act1_forest: anchura real del mapa (1440) y scroll por el
    // jugador (USER_RIGHT). El AUTO_RIGHT con anchura 320 derivaba fuera del tilemap
    // y provocaba una lectura no mapeada (cuelgue "unmapped read").
    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map, forest_pal, 1440, BG_SCRL_USER_RIGHT, 3);
    set_limits(0,134,275,172);

    init_character(CHR_linus);
    init_character(CHR_clio);        // para probar show/hide desde el DSL
    active_character = CHR_linus;

    // Colocar y MOSTRAR a Linus (init_character crea el sprite oculto)
    move_character_instant(CHR_linus, 140, 154);
    show_character(CHR_linus, true);
    move_character_instant(CHR_clio, 70, 154); // Clio empieza oculta; el DSL la muestra/oculta

    // Vara + hechizos del test (fire y luz: casteables por el jugador solo aquí)
    player_has_rod = true;
    spell_enable(SPELL_THUNDER);
    spell_enable(SPELL_HIDE);
    spell_enable(SPELL_FIRE);
    spell_enable(SPELL_LIGHT);   // LUZ: solo existe habilitado aquí (fases + invertible)
}

void act1_test_ghost2(void)    // Oleada 2: fantasma de TEST con dos hechizos (thunder + mordisco)
{
    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA);

    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_TESTGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(140));
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
