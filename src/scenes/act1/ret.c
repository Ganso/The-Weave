// act1/ret.c — hooks del regreso hacia la nave (escena 6 del guión).
// La secuencia está en return.scene: vuelta nocturna con antorcha, segunda
// emboscada de jabalíes y los espectros del Caos (combate por hechizos).

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"
#include "scenes/act1/ret.h"

void act1_return_start(void)    // Linus saca la antorcha; sopla el viento
{
    linus_has_torch = true;
    reinit_character_sprite(CHR_linus);
    play_sample(snd_ambient_wind, sizeof(snd_ambient_wind));
}

void act1_return_boars(void)    // Segunda emboscada: 3 jabalíes (con antorcha)
{
    PAL_setPalette(PAL3, boar_sprite.palette->data, DMA);
    static const s16 spawn[3][2] = {    // {x esquina, y de pies}
        { SCREEN_WIDTH + 12, 146 },
        { -60,               158 },
        { SCREEN_WIDTH + 40, 170 },
    };
    for (u16 i = 0; i < 3; i++) {
        init_enemy(i, ENEMY_CLS_BOAR);
        move_enemy_instant(i, FASTFIX32_FROM_INT(spawn[i][0]), FASTFIX32_FROM_INT(spawn[i][1]));
        show_enemy(i, true);
    }
    player_max_hitpoints = 5;
    melee_combat_run(4, CHR_clio);
}

void act1_return_ghosts(void)    // Espectros del Caos (combate por hechizos)
{
    show_or_hide_interface(false);

    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA);

    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    init_enemy(1, ENEMY_CLS_WEAVERGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy_instant(1, FASTFIX32_FROM_INT(-20), FASTFIX32_FROM_INT(156));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(136));
    move_enemy(1, FASTFIX32_FROM_INT(20), FASTFIX32_FROM_INT(156));

    player_max_hitpoints = 5;
}

void act1_return_torch_out(void)    // El viento apaga la antorcha: oscuridad
{
    play_sample(snd_ambient_wind, sizeof(snd_ambient_wind));
    linus_has_torch = false;
    reinit_character_sprite(CHR_linus);
    // El patrón queda inscrito (PLACEHOLDER: el guión inscribe Oscuridad con
    // Luz como reverso; mientras no exista SPELL_DARK se inscribe LUZ, que ya
    // es invertible en el motor)
    activate_spell(SPELL_LIGHT);
}

// Espera interactiva a que el jugador cante LUZ (la antorcha no se
// reenciende hasta que el patrón suena entero)
void act1_return_wait_light(void)
{
    bool seen = false;
    while (true) {
        next_frame(true);
        if (spell_active_id(SPELL_SLOT_PLAYER) == SPELL_LIGHT) seen = true;
        else if (seen) return;   // el efecto ha terminado
    }
}

void act1_return_torch_relight(void)    // La Luz cantada reenciende la antorcha
{
    linus_has_torch = true;
    reinit_character_sprite(CHR_linus);
}

void act1_return_end(void)    // Se apaga la antorcha antes de la escena final
{
    linus_has_torch = false;
}
