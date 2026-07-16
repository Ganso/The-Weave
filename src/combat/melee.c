// melee.c — combate físico (sin hechizos): ver melee.h para el contrato.
//
// Cómo convive con el motor:
//  - combat_state queda en COMBAT_NO: update_combat() no hace nada y los
//    enemigos nunca cantan patrones (spell_enemy_try_launch solo corre en IDLE).
//  - La locomoción de los enemigos la dirige este módulo poniéndolos en
//    STATE_WALKING (update_enemy_animations no toca la animación en ese estado).
//  - El daño al jugador reutiliza hit_player(); el "daño" al jabalí NO usa
//    hit_enemy() (mataría con 2 golpes): aquí un golpe = STATE_HIT (flash de
//    ANIM_HURT vía update_enemy_animations) + huida, y gana el contador global.
//  - El golpe del jugador reutiliza STATE_PLAYING_NOTE, que ya mapea a
//    ANIM_ACTION en update_character_animations y bloquea el movimiento
//    mientras dura (controller.c solo mueve en IDLE/WALKING).

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "audio/audio.h"
#include "res_all.h"
#include "combat/melee.h"

// Estados de cada enemigo durante el combate físico
typedef enum {
    MB_GONE,      // liberado o slot vacío
    MB_WAIT,      // aún no ha entrado (entradas escalonadas)
    MB_CHASE,     // persigue al jugador
    MB_BITE,      // mordiendo (ANIM_ACTION; el daño cae a mitad del ciclo)
    MB_HURT,      // golpeado (STATE_HIT: lo gestiona update_enemy_animations)
    MB_RETREAT,   // corre de vuelta a su punto de entrada y regresa
    MB_LEAVE      // huida final: corre a la derecha y se libera
} MeleeBoarState;

// Parámetros (ajustables con playtest)
#define MELEE_CHASE_SPEED    FASTFIX32_FROM_INT(2)          // px/frame persiguiendo
#define MELEE_CHASE_SPEED_Y  FASTFIX32_FROM_INT(1)
#define MELEE_RUN_SPEED      (FASTFIX32_FROM_INT(5) / 2)    // px/frame huyendo
#define MELEE_ENTER_STAGGER  40    // frames entre la entrada de un enemigo y el siguiente
#define MELEE_BITE_RANGE_X   34    // alcance del mordisco (entre centros de pies)
#define MELEE_BITE_RANGE_Y   8
#define MELEE_BITE_TIME      48    // ciclo del mordisco (6 frames x timer 8 del sprite)
#define MELEE_BITE_HIT_AT    24    // frame del ciclo en el que muerde de verdad
#define MELEE_ATTACK_TIME    20    // duración del golpe de Linus (5 frames x timer 4)
#define MELEE_ATTACK_HIT_AT  8     // frame del golpe en el que impacta
#define MELEE_ATTACK_RANGE_X 44    // alcance del golpe hacia delante
#define MELEE_ATTACK_RANGE_Y 12
#define MELEE_ATTACK_CD      30    // frames mínimos entre golpes
#define MELEE_COMPANION_GAP  44    // distancia a la que se coloca el acompañante
#define MELEE_EXIT_MARGIN    56    // px de pies fuera de pantalla al huir
#define MELEE_PAUSE_CHANCE   0x7F  // pausa aleatoria: ~1/128 por frame de persecución
#define MELEE_PAUSE_CD       (SCREEN_FPS * 2)  // frames sin re-pausar tras una pausa

static u8  mb_state[MAX_ENEMIES];
static u16 mb_timer[MAX_ENEMIES];
static s16 mb_exit_x[MAX_ENEMIES];  // x de pies destino de la huida en curso (lado más cercano)
static u16 mb_pause[MAX_ENEMIES];   // pausa aleatoria en curso (frames quieto)
static u16 mb_pause_cd[MAX_ENEMIES];// cooldown entre pausas aleatorias
static u16 melee_hits;              // golpes conectados (u16: legible por el driver MCP)
static u16 attack_timer;            // frames restantes del golpe en curso (0 = ninguno)
static u16 attack_cd;
static u16 prev_joy;
static bool melee_thunder_wins;     // true: solo el TRUENO ahuyenta (jabalíes de vuelta, sin antorcha)
static bool melee_thunder_seen;     // el trueno del jugador está sonando

static s16 chr_feet_x(u16 nchar)  { return FASTFIX32_TO_INT(obj_character[nchar].x) + obj_character[nchar].x_size / 2; }
static s16 chr_feet_y(u16 nchar)  { return FASTFIX32_TO_INT(obj_character[nchar].y) + obj_character[nchar].y_size; }
static s16 enemy_feet_x(u16 e)    { return FASTFIX32_TO_INT(obj_enemy[e].obj_character.x) + obj_enemy[e].obj_character.x_size / 2; }
static s16 enemy_feet_y(u16 e)    { return FASTFIX32_TO_INT(obj_enemy[e].obj_character.y) + obj_enemy[e].obj_character.y_size; }

// El acompañante se queda inmóvil detrás (a la izquierda) del jugador mirando
// a la derecha; si no estaba detrás, se recoloca andando (bloqueante).
// Devuelve el valor previo de follows_character para restaurarlo al terminar.
static bool melee_stage_companion(u16 companion)
{
    if (companion >= MAX_CHR || !obj_character[companion].active) return false;

    bool was_following = obj_character[companion].follows_character;
    obj_character[companion].follows_character = false;
    s16 px = FASTFIX32_TO_INT(obj_character[active_character].x);
    if (FASTFIX32_TO_INT(obj_character[companion].x) > px - (MELEE_COMPANION_GAP / 2))
        move_character(companion, px - MELEE_COMPANION_GAP, chr_feet_y(active_character));
    look_left(companion, false);   // mirando a la derecha
    obj_character[companion].state = STATE_IDLE;
    anim_character(companion, ANIM_IDLE);
    return was_following;
}

// Lado de huida: hacia el borde de pantalla más cercano, sea cual sea
static s16 melee_nearest_exit_x(u16 e)
{
    if (enemy_feet_x(e) < SCREEN_WIDTH / 2) return -MELEE_EXIT_MARGIN;
    return SCREEN_WIDTH + MELEE_EXIT_MARGIN;
}

// Mueve un enemigo un paso hacia (tx, ty) de pies, con la animación pedida.
// solid=false ignora la colisión con personajes (huidas: corren "por detrás").
// Si el paso completo choca, intenta deslizarse por un eje (rodear a Clio).
// Devuelve true si se ha movido (false = bloqueado del todo o ya llegó).
static bool melee_step_towards(u16 e, s16 tx, s16 ty, fastfix32 speed_x, fastfix32 speed_y, u8 anim, bool solid)
{
    Entity *en = &obj_enemy[e].obj_character;
    s16 dx = tx - enemy_feet_x(e);
    s16 dy = ty - enemy_feet_y(e);

    fastfix32 sx = 0, sy = 0;
    if (dx > 0) sx = (speed_x > FASTFIX32_FROM_INT(dx) ? FASTFIX32_FROM_INT(dx) : speed_x);
    else if (dx < 0) sx = -(speed_x > FASTFIX32_FROM_INT(-dx) ? FASTFIX32_FROM_INT(-dx) : speed_x);
    if (dy > 0) sy = (speed_y > FASTFIX32_FROM_INT(dy) ? FASTFIX32_FROM_INT(dy) : speed_y);
    else if (dy < 0) sy = -(speed_y > FASTFIX32_FROM_INT(-dy) ? FASTFIX32_FROM_INT(-dy) : speed_y);

    if (sx == 0 && sy == 0) return false;           // ya está en el destino

    // Paso completo, y si choca, deslizamiento por un solo eje
    fastfix32 dodge = sy ? sy : speed_y;   // esquiva vertical (en ambos sentidos)
    const fastfix32 tries[4][2] = { {sx, sy}, {sx, 0}, {0, dodge}, {0, -dodge} };
    for (u16 t = 0; t < 4; t++) {
        fastfix32 nx = en->x + tries[t][0];
        fastfix32 ny = en->y + tries[t][1];
        if (nx == en->x && ny == en->y) continue;
        if (solid &&
            detect_enemy_char_collision(e, FASTFIX32_TO_INT(nx), FASTFIX32_TO_INT(ny)) != CHR_NONE)
            continue;
        en->x = nx;
        en->y = ny;
        if (dx != 0) en->flipH = (dx < 0);
        en->animation = anim;
        update_enemy(e);
        return true;
    }
    return false;                                   // bloqueado del todo
}

// Golpe del jugador: busca el enemigo más cercano DELANTE (según flipH) y a su
// altura. Devuelve MAX_ENEMIES si no hay ninguno al alcance.
static u16 melee_find_attack_target(void)
{
    bool facing_left = obj_character[active_character].flipH;
    s16 px = chr_feet_x(active_character);
    s16 py = chr_feet_y(active_character);
    u16 best = MAX_ENEMIES;
    s16 best_dist = 0x7FFF;

    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (mb_state[e] == MB_GONE || mb_state[e] == MB_WAIT ||
            mb_state[e] == MB_HURT || mb_state[e] == MB_LEAVE) continue;

        s16 dx = enemy_feet_x(e) - px;
        s16 dy = enemy_feet_y(e) - py;
        s16 fwd = facing_left ? -dx : dx;           // distancia hacia delante
        if (fwd < 0 || fwd > MELEE_ATTACK_RANGE_X) continue;
        if (dy < -MELEE_ATTACK_RANGE_Y || dy > MELEE_ATTACK_RANGE_Y) continue;
        if (fwd < best_dist) { best_dist = fwd; best = e; }
    }
    return best;
}

// Modo trueno: cuando el TRUENO del jugador termina de sonar, ahuyenta a
// TODOS los jabalíes en pantalla y cuenta como un "golpe" del combate
static void melee_update_thunder(void)
{
    bool active = (spell_active_id(SPELL_SLOT_PLAYER) == SPELL_THUNDER);
    if (active) { melee_thunder_seen = true; return; }
    if (!melee_thunder_seen) return;
    melee_thunder_seen = false;      // el efecto acaba de terminar: espantada general

    dprintf(2, "Melee: trueno completado (%d)", melee_hits + 1);
    melee_hits++;
    play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (mb_state[e] == MB_CHASE || mb_state[e] == MB_BITE) {
            obj_enemy[e].obj_character.state = STATE_HIT;   // flash de susto
            obj_enemy[e].modeTimer = ENEMY_HURT_DURATION;
            mb_state[e] = MB_HURT;
        }
    }
}

// Input del golpe + resolución del impacto
static void melee_update_player_attack(void)
{
    u16 joy = JOY_readJoypad(JOY_ALL);
    u16 pressed = joy & ~prev_joy;
    prev_joy = joy;

    if (attack_cd) attack_cd--;

    if (attack_timer) {
        attack_timer--;

        // Momento del impacto (si no nos han mordido antes)
        if (attack_timer == MELEE_ATTACK_TIME - MELEE_ATTACK_HIT_AT &&
            obj_character[active_character].state == STATE_PLAYING_NOTE) {
            u16 e = melee_find_attack_target();
            if (e < MAX_ENEMIES) {
                dprintf(2, "Melee: player hit enemy %d (%d hits)", e, melee_hits + 1);
                melee_hits++;
                play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
                obj_enemy[e].obj_character.state = STATE_HIT;   // flash de daño (update_enemy_animations)
                obj_enemy[e].modeTimer = ENEMY_HURT_DURATION;
                mb_state[e] = MB_HURT;
            }
        }

        // Fin del golpe: soltar el estado para recuperar el control
        if (attack_timer == 0 &&
            obj_character[active_character].state == STATE_PLAYING_NOTE)
            obj_character[active_character].state = STATE_IDLE;
        return;
    }

    // Nuevo golpe: botón A, sin cooldown y con el jugador disponible
    if ((pressed & BUTTON_A) && attack_cd == 0 &&
        (obj_character[active_character].state == STATE_IDLE ||
         obj_character[active_character].state == STATE_WALKING)) {
        obj_character[active_character].state = STATE_PLAYING_NOTE;  // → ANIM_ACTION
        obj_character[active_character].animation = ANIM_ACTION;
        SPR_setAnimAndFrame(spr_chr[active_character], ANIM_ACTION, 0); // reiniciar el golpe
        attack_timer = MELEE_ATTACK_TIME;
        attack_cd = MELEE_ATTACK_CD;
    }
}

// FSM de un enemigo por frame
static void melee_update_enemy(u16 e, u8 hits_to_win)
{
    Entity *en = &obj_enemy[e].obj_character;

    if (mb_state[e] == MB_GONE) return;
    if (!en->active) { mb_state[e] = MB_GONE; return; }

    // Con el combate ganado, todo el mundo pasa a la huida final
    // (el golpeado termina su flash de daño antes de salir corriendo)
    if (melee_hits >= hits_to_win &&
        mb_state[e] != MB_LEAVE && mb_state[e] != MB_HURT) {
        mb_state[e] = MB_LEAVE;
        mb_exit_x[e] = melee_nearest_exit_x(e);
        en->state = STATE_WALKING;
        en->animation = ANIM_RUN;
    }

    s16 px = chr_feet_x(active_character);
    s16 py = chr_feet_y(active_character);

    switch (mb_state[e]) {

    case MB_WAIT:
        if (mb_timer[e]) { mb_timer[e]--; break; }
        mb_state[e] = MB_CHASE;
        en->state = STATE_WALKING;      // locomoción dirigida por melee
        break;

    case MB_CHASE: {
        s16 dx = px - enemy_feet_x(e);
        s16 dy = py - enemy_feet_y(e);
        if (abs(dx) <= MELEE_BITE_RANGE_X && abs(dy) <= MELEE_BITE_RANGE_Y) {
            mb_state[e] = MB_BITE;
            mb_timer[e] = MELEE_BITE_TIME;
            en->flipH = (dx < 0);
            en->animation = ANIM_ACTION;
            SPR_setAnimAndFrame(spr_enemy[e], ANIM_ACTION, 0);
            update_enemy(e);
            break;
        }

        // Pausa aleatoria al acercarse: se queda quieto ~1 s y sigue avanzando,
        // con un cooldown por enemigo para no encadenar pausas.
        if (mb_pause[e]) {
            mb_pause[e]--;
            en->animation = ANIM_IDLE;
            update_enemy(e);
            break;
        }
        if (mb_pause_cd[e]) {
            mb_pause_cd[e]--;
        } else if ((random() & MELEE_PAUSE_CHANCE) == 0) {
            mb_pause[e] = SCREEN_FPS - 16 + (random() & 31);   // ~0,7–1,3 s
            mb_pause_cd[e] = MELEE_PAUSE_CD;
            en->animation = ANIM_IDLE;
            update_enemy(e);
            break;
        }

        if (!melee_step_towards(e, px, py, MELEE_CHASE_SPEED, MELEE_CHASE_SPEED_Y, ANIM_WALK, true)) {
            en->animation = ANIM_IDLE;  // bloqueado: esperar sin patalear
            update_enemy(e);
        }
        break;
    }

    case MB_BITE:
        if (mb_timer[e]) mb_timer[e]--;
        if (mb_timer[e] == MELEE_BITE_TIME - MELEE_BITE_HIT_AT) {
            s16 dx = px - enemy_feet_x(e);
            s16 dy = py - enemy_feet_y(e);
            if (abs(dx) <= MELEE_BITE_RANGE_X && abs(dy) <= MELEE_BITE_RANGE_Y) {
                dprintf(2, "Melee: enemy %d bites player", e);
                hit_player(1);
            }
        }
        if (mb_timer[e] == 0) {
            mb_state[e] = MB_CHASE;
            en->animation = ANIM_WALK;
        }
        break;

    case MB_HURT:
        // update_enemy_animations gestiona el flash y devuelve el estado a IDLE
        if (en->state != STATE_HIT) {
            mb_state[e] = MB_RETREAT;
            mb_exit_x[e] = melee_nearest_exit_x(e);   // huye hacia el lado más cercano
            en->state = STATE_WALKING;
            en->animation = ANIM_RUN;
        }
        break;

    case MB_RETREAT:
        if (!melee_step_towards(e, mb_exit_x[e], enemy_feet_y(e),
                                MELEE_RUN_SPEED, MELEE_RUN_SPEED, ANIM_RUN, false)) {
            mb_state[e] = MB_CHASE;     // llegó fuera de pantalla: vuelve a por Linus
            en->animation = ANIM_WALK;
        }
        break;

    case MB_LEAVE:
        if (!melee_step_towards(e, mb_exit_x[e], enemy_feet_y(e),
                                MELEE_RUN_SPEED, MELEE_RUN_SPEED, ANIM_RUN, false)) {
            release_enemy(e);           // llegó fuera de pantalla: se va del todo
            mb_state[e] = MB_GONE;
        }
        break;

    default:
        break;
    }
}

static void melee_run(u8 hits_to_win, u16 companion, bool thunder_wins)
{
    dprintf(2, "Melee combat: start (%d hits to win, thunder=%d)", hits_to_win, thunder_wins);

    melee_thunder_wins = thunder_wins;
    melee_thunder_seen = false;
    bool comp_was_following = melee_stage_companion(companion);

    // Tomar el control de los enemigos ya spawneados
    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (obj_enemy[e].obj_character.active) {
            mb_state[e] = MB_WAIT;
            mb_timer[e] = e * MELEE_ENTER_STAGGER;
            mb_pause[e] = 0;
            mb_pause_cd[e] = SCREEN_FPS;   // sin pausas aleatorias nada más entrar
        } else {
            mb_state[e] = MB_GONE;
        }
    }

    melee_hits = 0;
    attack_timer = 0;
    attack_cd = 15;                      // margen para no golpear con el A del diálogo
    prev_joy = JOY_readJoypad(JOY_ALL);
    player_hitpoints = player_max_hitpoints;   // la vida se reinicia en cada combate
    player_defeated = false;

    bool old_scroll = player_scroll_active;
    player_scroll_active = false;        // arena fija durante el combate

    bool running = true;
    while (running && !player_defeated) {
        next_frame(true);
        if (melee_thunder_wins) melee_update_thunder();   // el arma es el patrón
        else melee_update_player_attack();                // el arma es el golpe con A

        running = false;
        for (u16 e = 0; e < MAX_ENEMIES; e++) {
            melee_update_enemy(e, hits_to_win);
            if (mb_state[e] != MB_GONE) running = true;
        }
    }

    // Derrota: liberar a los que queden; la escena decide el reintento (op if_defeated)
    if (player_defeated) {
        for (u16 e = 0; e < MAX_ENEMIES; e++) {
            if (mb_state[e] != MB_GONE && obj_enemy[e].obj_character.active)
                release_enemy(e);
            mb_state[e] = MB_GONE;
        }
    }

    // Dejar al jugador limpio
    if (obj_character[active_character].state == STATE_PLAYING_NOTE)
        obj_character[active_character].state = STATE_IDLE;

    player_scroll_active = old_scroll;

    // El acompañante vuelve a seguir al jugador si lo hacía antes del combate
    if (companion < MAX_CHR && obj_character[companion].active && comp_was_following)
        obj_character[companion].follows_character = true;

    dprintf(2, "Melee combat: end (%d hits, %s)", melee_hits,
            player_defeated ? "derrota" : "victoria");
}

void melee_combat_run(u8 hits_to_win, u16 companion)
{
    melee_run(hits_to_win, companion, false);
}

void melee_combat_run_thunder(u8 casts_to_win, u16 companion)
{
    melee_run(casts_to_win, companion, true);
}
