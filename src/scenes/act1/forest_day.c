// act1/forest_day.c — hooks del bosque de día de la isla (escena 4b del
// guión: tras la playa, el interior; aquí atacan por primera vez los
// jabalíes y Clio canta Curación). La secuencia está en forest_day.scene.

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"
#include "scenes/act1/forest_day.h"

void act1_fday_start(void)    // Bosque del atardecer: Linus lleva la antorcha
{
    linus_has_torch = true;
    reinit_character_sprite(CHR_linus);
    play_sample(snd_ambient_wind, sizeof(snd_ambient_wind));
}

void act1_fday_boars(void)    // Primer combate físico: 3 jabalíes (Linus sin vara)
{
    PAL_setPalette(PAL_ENEMIES, boar_sprite.palette->data, DMA);
    static const s16 spawn[3][2] = {    // {x esquina, y de pies}
        { SCREEN_WIDTH + 12, 146 },
        { SCREEN_WIDTH + 40, 170 },
        { -60,               158 },
    };
    for (u16 i = 0; i < 3; i++) {
        init_enemy(i, ENEMY_CLS_BOAR);
        move_enemy_instant(i, FASTFIX32_FROM_INT(spawn[i][0]), FASTFIX32_FROM_INT(spawn[i][1]));
        show_enemy(i, true);
    }
    player_max_hitpoints = 5;
    combat_configure(&(CombatConfig){
        .weapon_strike = true,       // sin bastón: el arma es el golpe con A
        .hits_to_win = 4,            // 4 golpes los ahuyentan
        .companion = CHR_clio,       // Clio espera detrás
        .reposition_companion = true,
    });
    // El combate lo ejecuta el op `combat` de la escena, justo tras este hook
}

#define BITE_GAP     32   // a qué distancia de Clio se planta el jabalí
#define BITE_RUN_IN   3   // px/frame de la carrera de entrada
#define BITE_RUN_OUT  4   // px/frame de la huida

// Un jabalí rezagado entra corriendo por la IZQUIERDA, se planta junto a Clio,
// la muerde, y ella queda herida hasta que se cure (guión 4.3). La pose de
// herida se fija con set_character_anim para que aguante los diálogos que
// vienen después; act1_fday_heal la suelta.
void act1_fday_bite(void)
{
    idle_all_characters();               // que nadie se quede andando

    s16 clio_x    = FASTFIX32_TO_INT(obj_character[CHR_clio].x);
    s16 clio_feet = FASTFIX32_TO_INT(obj_character[CHR_clio].y) +
                    obj_character[CHR_clio].y_size;

    PAL_setPalette(PAL_ENEMIES, boar_sprite.palette->data, DMA);
    init_enemy(0, ENEMY_CLS_BOAR);
    move_enemy_instant(0, FASTFIX32_FROM_INT(-64), FASTFIX32_FROM_INT(clio_feet));
    show_enemy(0, true);

    // La locomoción la lleva este gancho: STATE_WALKING evita que
    // update_enemy_animations le cambie la animación de galope
    Entity *boar = &obj_enemy[0].obj_character;
    boar->state = STATE_WALKING;
    boar->flipH = false;                 // el arte mira a la DERECHA
    anim_enemy(0, ANIM_RUN);

    s16 stop_x = clio_x - BITE_GAP;
    while (FASTFIX32_TO_INT(boar->x) < stop_x) {
        boar->x += FASTFIX32_FROM_INT(BITE_RUN_IN);
        update_enemy(0);
        next_frame(false);
    }

    // El mordisco
    anim_enemy(0, ANIM_ACTION);
    play_sample(snd_player_hurt, sizeof(snd_player_hurt));
    wait_seconds(1);

    // Clio encaja el golpe UNA vez y se queda herida en el suelo
    look_left(CHR_clio, true);           // encaja el golpe mirando al jabalí
    anim_character(CHR_clio, ANIM_HURT);
    wait_seconds(1);
    look_left(CHR_clio, false);          // la pose de herida está dibujada mirando a la DERECHA
    set_character_anim(CHR_clio, ANIM_WOUNDED);

    // El jabalí se larga por donde vino
    boar->flipH = true;
    anim_enemy(0, ANIM_RUN);
    while (FASTFIX32_TO_INT(boar->x) > -64) {
        boar->x -= FASTFIX32_FROM_INT(BITE_RUN_OUT);
        update_enemy(0);
        next_frame(false);
    }
    release_enemy(0);
}

void act1_fday_heal(void)    // Clio canta el patrón de Curación (narrativo)
{
    obj_character[active_character].state = STATE_IDLE;
    anim_character(CHR_linus, ANIM_IDLE);
    look_left(CHR_clio, false);          // mirando a Linus (a su derecha)

    // Sale de la pose de herida para cantar (sigue fijada: el motor no la toca)
    set_character_anim(CHR_clio, ANIM_MAGIC);
    play_sample(snd_effect_magic_appear, sizeof(snd_effect_magic_appear));

    // Destello verde en el cielo del bosque mientras canta
    u16 old = PAL_getColor(4);           // color 4 = azul del cielo en forest_pal
    for (u16 i = 0; i < SCREEN_FPS * 2; i++) {
        PAL_setColor(4, ((i >> 3) & 1) ? RGB24_TO_VDPCOLOR(0x55cc77) : old);
        next_frame(false);
    }
    PAL_setColor(4, old);
    release_character_anim(CHR_clio);    // curada: el motor recupera el control

    // Linus reconoce el patrón y lo anota, como la nana del dormitorio
    // (jingle + notas en el HUD + entrada en el menú de pausa)
    activate_spell(SPELL_HEAL);
}
