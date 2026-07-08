// act1/hall.c — hook de setup del hall (la secuencia entera está en hall.scene)

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "res_all.h"
#include "scenes/act1/hall.h"

void act1_hall_setup(void)    // Nivel del hall + posiciones iniciales
{
    new_level(&historians_bg_tile, &historians_bg_map, &historians_front_tile, &historians_front_map, historians_pal, SCREEN_WIDTH, BG_SCRL_AUTO_RIGHT, 3);

    init_character(CHR_linus);
    init_character(CHR_clio);
    init_character(CHR_xander);

    move_character_instant(CHR_clio, 40, 174);
    move_character_instant(CHR_linus, 360, 164);
    look_left(CHR_clio, false);
    look_left(CHR_linus, true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
}
