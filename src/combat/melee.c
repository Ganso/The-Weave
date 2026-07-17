// melee.c — director del combate de CONTACTO (la manada de jabalíes).
// Contrato y configuración en melee.h; funcionamiento detallado en docs/combat.md.
//
// Convivencia con el resto del motor (resumen; el detalle está en combat.md):
//  - combat_state pertenece al director de PATRONES (combat.c). Aquí se
//    normaliza a COMBAT_NO en cada frame: un cast del jugador lo deja en
//    IDLE al terminar si ve enemigos activos, y ese residuo disparaba
//    diálogos de tutorial y corrompía los reintentos.
//  - La locomoción de los enemigos es nuestra: van en STATE_WALKING (estado
//    que update_enemy_animations no toca) y sus animaciones se fijan aquí.
//  - El daño al jugador reutiliza hit_player() (vida + derrota); el "daño"
//    al jabalí NO usa hit_enemy() (moriría): un impacto = flash de STATE_HIT
//    + huida al borde más cercano + vuelta a la carga.
//  - El golpe físico reutiliza STATE_PLAYING_NOTE → ANIM_ACTION (bloquea el
//    movimiento del jugador mientras dura, como tocar una nota).

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "audio/audio.h"
#include "res_all.h"
#include "combat/melee.h"

// ---------------------------------------------------------------------------
// Parámetros de ajuste (playtest)
// ---------------------------------------------------------------------------
#define CHASE_SPEED_X    FASTFIX32_FROM_INT(2)        // persecución (px/frame)
#define CHASE_SPEED_Y    FASTFIX32_FROM_INT(1)
#define RUN_SPEED        (FASTFIX32_FROM_INT(5) / 2)  // huida (px/frame)
#define ENTER_STAGGER    40    // frames entre la entrada de un enemigo y el siguiente
#define WANDER_RANGE     16    // desvío vertical aleatorio de la persecución (±px)
#define BITE_RANGE_X     34    // alcance del mordisco (entre centros de pies)
#define BITE_RANGE_Y     8
#define BITE_TIME        48    // ciclo de la anim de mordisco (6 frames x timer 8)
#define BITE_HIT_AT      24    // frame del ciclo en el que muerde de verdad
#define PAUSE_CHANCE     0x7F  // pausa aleatoria: ~1/128 por frame de persecución
#define PAUSE_COOLDOWN   (SCREEN_FPS * 2)
#define ATTACK_TIME      20    // duración del golpe físico (5 frames x timer 4)
#define ATTACK_HIT_AT    8     // frame del golpe en el que impacta
#define ATTACK_RANGE_X   44    // alcance del golpe hacia delante
#define ATTACK_RANGE_Y   12
#define ATTACK_COOLDOWN  30    // frames mínimos entre golpes
#define COMPANION_GAP    44    // distancia a la que se recoloca el acompañante
#define EXIT_MARGIN      56    // px de pies fuera de pantalla al huir

// ---------------------------------------------------------------------------
// Estado del combate
// ---------------------------------------------------------------------------

// Estados de cada enemigo (FSM: ver el diagrama en docs/combat.md)
typedef enum {
    MB_GONE,      // liberado o slot vacío
    MB_WAIT,      // aún no ha entrado (entradas escalonadas)
    MB_CHASE,     // persigue al jugador (con pausas y desvío aleatorios)
    MB_BITE,      // mordiendo (ANIM_ACTION; el daño cae a mitad del ciclo)
    MB_HURT,      // impactado (flash de STATE_HIT; lo cuenta update_enemy_animations)
    MB_RETREAT,   // corre al borde más cercano y vuelve a la carga
    MB_LEAVE      // huida final: corre al borde más cercano y se libera
} BoarState;

static u8  mb_state[MAX_ENEMIES];
static u16 mb_timer[MAX_ENEMIES];     // temporizador del estado (entrada, mordisco)
static s16 mb_exit_x[MAX_ENEMIES];    // x de pies destino de la huida en curso
static s8  mb_wander[MAX_ENEMIES];    // desvío vertical aleatorio del objetivo
static u16 mb_wander_t[MAX_ENEMIES];  // frames hasta re-sortear el desvío
static u16 mb_pause[MAX_ENEMIES];     // pausa aleatoria en curso (frames quieto)
static u16 mb_pause_cd[MAX_ENEMIES];  // cooldown entre pausas

static MeleeConfig cfg;               // configuración del combate en curso
static u16 melee_hits;                // impactos conectados (legible por el driver MCP)
static u16 attack_timer;              // frames restantes del golpe físico (0 = ninguno)
static u16 attack_cd;
static u16 prev_joy;
static bool thunder_was_active;       // flanco de subida del cast de TRUENO

// ---------------------------------------------------------------------------
// Utilidades
// ---------------------------------------------------------------------------
static s16 chr_feet_x(u16 nchar)  { return FASTFIX32_TO_INT(obj_character[nchar].x) + obj_character[nchar].x_size / 2; }
static s16 chr_feet_y(u16 nchar)  { return FASTFIX32_TO_INT(obj_character[nchar].y) + obj_character[nchar].y_size; }
static s16 enemy_feet_x(u16 e)    { return FASTFIX32_TO_INT(obj_enemy[e].obj_character.x) + obj_enemy[e].obj_character.x_size / 2; }
static s16 enemy_feet_y(u16 e)    { return FASTFIX32_TO_INT(obj_enemy[e].obj_character.y) + obj_enemy[e].obj_character.y_size; }

// Borde de pantalla más cercano al enemigo, sea cual sea
static s16 nearest_exit_x(u16 e)
{
    return (enemy_feet_x(e) < SCREEN_WIDTH / 2) ? -EXIT_MARGIN
                                                : SCREEN_WIDTH + EXIT_MARGIN;
}

// Un impacto sobre el enemigo: flash de daño y a huir (sin restar HP: los
// jabalíes no mueren, escarmientan)
static void scare_enemy(u16 e)
{
    obj_enemy[e].obj_character.state = STATE_HIT;   // el flash lo lleva update_enemy_animations
    obj_enemy[e].modeTimer = ENEMY_HURT_DURATION;
    mb_state[e] = MB_HURT;
}

// ---------------------------------------------------------------------------
// El acompañante (Clio): deja de seguir y espera quieto durante el combate
// ---------------------------------------------------------------------------
static bool stage_companion(void)
{
    u16 c = cfg.companion;
    if (c >= MAX_CHR || !obj_character[c].active) return false;

    bool was_following = obj_character[c].follows_character;
    obj_character[c].follows_character = false;

    s16 px = FASTFIX32_TO_INT(obj_character[active_character].x);
    if (cfg.reposition_companion &&
        FASTFIX32_TO_INT(obj_character[c].x) > px - (COMPANION_GAP / 2))
        move_character(c, px - COMPANION_GAP, chr_feet_y(active_character));

    look_left(c, false);   // mirando a la derecha
    obj_character[c].state = STATE_IDLE;
    anim_character(c, ANIM_IDLE);
    return was_following;
}

static void unstage_companion(bool was_following)
{
    u16 c = cfg.companion;
    if (c < MAX_CHR && obj_character[c].active && was_following)
        obj_character[c].follows_character = true;
}

// ---------------------------------------------------------------------------
// Movimiento: un paso hacia (tx, ty) de pies, con deslizamiento por ejes
// ---------------------------------------------------------------------------
// solid=false ignora la colisión con personajes (las huidas corren "por
// detrás" y así nadie se queda encajonado). Devuelve false si está bloqueado
// del todo o ya ha llegado.
static bool step_towards(u16 e, s16 tx, s16 ty, fastfix32 sp_x, fastfix32 sp_y, u8 anim, bool solid)
{
    Entity *en = &obj_enemy[e].obj_character;
    s16 dx = tx - enemy_feet_x(e);
    s16 dy = ty - enemy_feet_y(e);

    fastfix32 sx = 0, sy = 0;
    if (dx > 0) sx = (sp_x > FASTFIX32_FROM_INT(dx) ? FASTFIX32_FROM_INT(dx) : sp_x);
    else if (dx < 0) sx = -(sp_x > FASTFIX32_FROM_INT(-dx) ? FASTFIX32_FROM_INT(-dx) : sp_x);
    if (dy > 0) sy = (sp_y > FASTFIX32_FROM_INT(dy) ? FASTFIX32_FROM_INT(dy) : sp_y);
    else if (dy < 0) sy = -(sp_y > FASTFIX32_FROM_INT(-dy) ? FASTFIX32_FROM_INT(-dy) : sp_y);

    if (sx == 0 && sy == 0) return false;   // ya está en el destino

    // Paso completo y, si choca, deslizamiento por un solo eje (ambos sentidos)
    fastfix32 dodge = sy ? sy : sp_y;
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
    return false;   // bloqueado del todo
}

// ---------------------------------------------------------------------------
// Armas del jugador
// ---------------------------------------------------------------------------

// Arma TRUENO: en cuanto el patrón suena entero (el cast arranca), la
// descarga espanta a TODA la manada y cuenta como un impacto del combate
static void update_weapon_thunder(void)
{
    bool active = (spell_active_id(SPELL_SLOT_PLAYER) == SPELL_THUNDER);
    bool launched = active && !thunder_was_active;   // flanco de subida
    thunder_was_active = active;
    if (!launched) return;

    dprintf(2, "Melee: trueno (%d/%d)", melee_hits + 1, cfg.hits_to_win);
    melee_hits++;
    play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
    for (u16 e = 0; e < MAX_ENEMIES; e++)
        if (mb_state[e] == MB_CHASE || mb_state[e] == MB_BITE)
            scare_enemy(e);
}

// Arma GOLPE (A): busca el enemigo más cercano delante del jugador y a su altura
static u16 find_strike_target(void)
{
    bool facing_left = obj_character[active_character].flipH;
    s16 px = chr_feet_x(active_character);
    s16 py = chr_feet_y(active_character);
    u16 best = MAX_ENEMIES;
    s16 best_dist = 0x7FFF;

    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (mb_state[e] != MB_CHASE && mb_state[e] != MB_BITE && mb_state[e] != MB_RETREAT)
            continue;
        s16 dx = enemy_feet_x(e) - px;
        s16 dy = enemy_feet_y(e) - py;
        s16 fwd = facing_left ? -dx : dx;           // distancia hacia delante
        if (fwd < 0 || fwd > ATTACK_RANGE_X) continue;
        if (dy < -ATTACK_RANGE_Y || dy > ATTACK_RANGE_Y) continue;
        if (fwd < best_dist) { best_dist = fwd; best = e; }
    }
    return best;
}

static void update_weapon_strike(void)
{
    u16 joy = JOY_readJoypad(JOY_ALL);
    u16 pressed = joy & ~prev_joy;
    prev_joy = joy;

    if (attack_cd) attack_cd--;

    if (attack_timer) {
        attack_timer--;

        // Momento del impacto (si no nos han mordido antes)
        if (attack_timer == ATTACK_TIME - ATTACK_HIT_AT &&
            obj_character[active_character].state == STATE_PLAYING_NOTE) {
            u16 e = find_strike_target();
            if (e < MAX_ENEMIES) {
                dprintf(2, "Melee: golpe a %d (%d/%d)", e, melee_hits + 1, cfg.hits_to_win);
                melee_hits++;
                play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
                scare_enemy(e);
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
        SPR_setAnimAndFrame(spr_chr[active_character], ANIM_ACTION, 0);
        attack_timer = ATTACK_TIME;
        attack_cd = ATTACK_COOLDOWN;
    }
}

// ---------------------------------------------------------------------------
// FSM de cada enemigo, un tick por frame
// ---------------------------------------------------------------------------
static void update_enemy_fsm(u16 e)
{
    Entity *en = &obj_enemy[e].obj_character;

    if (mb_state[e] == MB_GONE) return;
    if (!en->active) { mb_state[e] = MB_GONE; return; }

    // Con el combate ganado, todo el mundo pasa a la huida final (el que esté
    // en pleno flash de daño la empieza al terminarlo)
    if (melee_hits >= cfg.hits_to_win &&
        mb_state[e] != MB_LEAVE && mb_state[e] != MB_HURT) {
        mb_state[e] = MB_LEAVE;
        mb_exit_x[e] = nearest_exit_x(e);
        en->state = STATE_WALKING;
        en->animation = ANIM_RUN;
    }

    s16 px = chr_feet_x(active_character);
    s16 py = chr_feet_y(active_character);

    switch (mb_state[e]) {

    case MB_WAIT:   // entrada escalonada
        if (mb_timer[e]) { mb_timer[e]--; break; }
        mb_state[e] = MB_CHASE;
        en->state = STATE_WALKING;   // locomoción dirigida por este módulo
        break;

    case MB_CHASE: {
        // Desvío vertical aleatorio del objetivo: variedad de movimiento y
        // menos atascos contra Clio o entre jabalíes
        if (mb_wander_t[e]) mb_wander_t[e]--;
        else {
            mb_wander[e] = (s8)(random() % (WANDER_RANGE * 2 + 1)) - WANDER_RANGE;
            mb_wander_t[e] = SCREEN_FPS + (random() & 63);
        }
        s16 wy = py + mb_wander[e];
        if (wy < (s16)y_limit_min) wy = y_limit_min;
        if (wy > (s16)y_limit_max) wy = y_limit_max;

        // ¿A distancia de mordisco (medida contra el jugador real)?
        s16 dx = px - enemy_feet_x(e);
        s16 dy = py - enemy_feet_y(e);
        if (abs(dx) <= BITE_RANGE_X && abs(dy) <= BITE_RANGE_Y) {
            mb_state[e] = MB_BITE;
            mb_timer[e] = BITE_TIME;
            en->flipH = (dx < 0);
            en->animation = ANIM_ACTION;
            SPR_setAnimAndFrame(spr_enemy[e], ANIM_ACTION, 0);
            update_enemy(e);
            break;
        }

        // Pausa aleatoria: se queda quieto ~1 s y sigue (cooldown por enemigo)
        if (mb_pause[e]) {
            mb_pause[e]--;
            en->animation = ANIM_IDLE;
            update_enemy(e);
            break;
        }
        if (mb_pause_cd[e]) {
            mb_pause_cd[e]--;
        } else if ((random() & PAUSE_CHANCE) == 0) {
            mb_pause[e] = SCREEN_FPS - 16 + (random() & 31);   // ~0,7–1,3 s
            mb_pause_cd[e] = PAUSE_COOLDOWN;
            en->animation = ANIM_IDLE;
            update_enemy(e);
            break;
        }

        if (!step_towards(e, px, wy, CHASE_SPEED_X, CHASE_SPEED_Y, ANIM_WALK, true)) {
            mb_wander_t[e] = 0;         // bloqueado: re-sortear el desvío ya
            en->animation = ANIM_IDLE;
            update_enemy(e);
        }
        break;
    }

    case MB_BITE:
        if (mb_timer[e]) mb_timer[e]--;
        if (mb_timer[e] == BITE_TIME - BITE_HIT_AT) {
            s16 dx = px - enemy_feet_x(e);
            s16 dy = py - enemy_feet_y(e);
            if (abs(dx) <= BITE_RANGE_X && abs(dy) <= BITE_RANGE_Y) {
                dprintf(2, "Melee: mordisco de %d", e);
                hit_player(1);
            }
        }
        if (mb_timer[e] == 0) {
            mb_state[e] = MB_CHASE;
            en->animation = ANIM_WALK;
        }
        break;

    case MB_HURT:
        // update_enemy_animations lleva el flash y devuelve el estado a IDLE
        if (en->state != STATE_HIT) {
            mb_state[e] = MB_RETREAT;
            mb_exit_x[e] = nearest_exit_x(e);
            en->state = STATE_WALKING;
            en->animation = ANIM_RUN;
        }
        break;

    case MB_RETREAT:
        if (!step_towards(e, mb_exit_x[e], enemy_feet_y(e), RUN_SPEED, RUN_SPEED, ANIM_RUN, false)) {
            mb_state[e] = MB_CHASE;     // llegó fuera de pantalla: vuelve a la carga
            en->animation = ANIM_WALK;
        }
        break;

    case MB_LEAVE:
        if (!step_towards(e, mb_exit_x[e], enemy_feet_y(e), RUN_SPEED, RUN_SPEED, ANIM_RUN, false)) {
            release_enemy(e);           // llegó fuera de pantalla: se va del todo
            mb_state[e] = MB_GONE;
        }
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// Bucle principal (bloqueante)
// ---------------------------------------------------------------------------
void melee_combat_run(const MeleeConfig *config)
{
    cfg = *config;
    dprintf(2, "Melee: inicio (%d impactos, arma=%s)", cfg.hits_to_win,
            cfg.weapon_is_thunder ? "trueno" : "golpe");

    bool comp_was_following = stage_companion();

    // Tomar el control de los enemigos ya spawneados
    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (obj_enemy[e].obj_character.active) {
            mb_state[e] = MB_WAIT;
            mb_timer[e] = e * ENTER_STAGGER;
            mb_wander[e] = 0;
            mb_wander_t[e] = (random() & 31);
            mb_pause[e] = 0;
            mb_pause_cd[e] = SCREEN_FPS;   // sin pausas nada más entrar
        } else {
            mb_state[e] = MB_GONE;
        }
    }

    melee_hits = 0;
    attack_timer = 0;
    attack_cd = 15;                      // margen para no golpear con el A del diálogo
    prev_joy = JOY_readJoypad(JOY_ALL);
    thunder_was_active = false;
    player_hitpoints = player_max_hitpoints;   // la vida se reinicia en cada combate
    player_defeated = false;
    combat_state = COMBAT_NO;

    bool old_scroll = player_scroll_active;
    player_scroll_active = false;        // arena fija durante el combate

    bool running = true;
    while (running && !player_defeated) {
        next_frame(true);

        // El motor de patrones deja combat_state en IDLE tras un cast del
        // jugador si ve enemigos activos; aquí el director es melee, así que
        // ese residuo se normaliza (evita tutoriales y estados corruptos)
        if (combat_state == COMBAT_STATE_IDLE) combat_state = COMBAT_NO;

        if (cfg.weapon_is_thunder) update_weapon_thunder();
        else update_weapon_strike();

        running = false;
        for (u16 e = 0; e < MAX_ENEMIES; e++) {
            update_enemy_fsm(e);
            if (mb_state[e] != MB_GONE) running = true;
        }
    }

    // Derrota: liberar a los que queden (la escena decide el reintento)
    if (player_defeated) {
        for (u16 e = 0; e < MAX_ENEMIES; e++) {
            if (mb_state[e] != MB_GONE && obj_enemy[e].obj_character.active)
                release_enemy(e);
            mb_state[e] = MB_GONE;
        }
    }

    // Dejar el motor limpio: jugador disponible, sin hechizos ni notas a
    // medias y sin estados de combate residuales
    if (obj_character[active_character].state == STATE_PLAYING_NOTE)
        obj_character[active_character].state = STATE_IDLE;
    if (spell_slot_active(SPELL_SLOT_PLAYER))
        spell_cancel(SPELL_SLOT_PLAYER);
    reset_note_queue();
    combat_state = COMBAT_NO;
    player_scroll_active = old_scroll;
    unstage_companion(comp_was_following);

    dprintf(2, "Melee: fin (%d impactos, %s)", melee_hits,
            player_defeated ? "derrota" : "victoria");
}
