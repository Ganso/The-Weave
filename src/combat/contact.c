// contact.c — FSM de los enemigos de CONTACTO. Contrato en contact.h;
// funcionamiento detallado en docs/combat.md.
//
// Reparto con el resto del motor:
//  - La locomoción es nuestra: los enemigos van en STATE_WALKING (estado que
//    update_enemy_animations no toca) y sus animaciones se fijan aquí.
//  - El flash de daño sí es del motor: contact_scare_enemy pone STATE_HIT y
//    update_enemy_animations lo cuenta y devuelve a IDLE (esa transición nos
//    saca de CT_HURT).
//  - El daño al jugador reutiliza hit_player(); un impacto al enemigo NO usa
//    hit_enemy() (moriría): aquí se escarmienta y se huye.
//  - El ataque (alcance, ritmo del ciclo, frame de impacto, daño) se lee del
//    ContactProfile de la clase: la FSM es la misma para cualquier clase.

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "audio/audio.h"
#include "res_all.h"
#include "combat/contact.h"

// ---------------------------------------------------------------------------
// Parámetros de ajuste comunes (lo propio de cada clase va en su ContactProfile)
// ---------------------------------------------------------------------------
#define CHASE_SPEED_X    FASTFIX32_FROM_INT(2)        // persecución (px/frame)
#define CHASE_SPEED_Y    FASTFIX32_FROM_INT(1)
#define RUN_SPEED        (FASTFIX32_FROM_INT(5) / 2)  // huida (px/frame)
#define ENTER_STAGGER    40    // frames entre la entrada de un enemigo y el siguiente
#define WANDER_RANGE     16    // desvío vertical aleatorio de la persecución (±px)
#define PAUSE_CHANCE     0x7F  // pausa aleatoria: ~1/128 por frame de persecución
#define PAUSE_COOLDOWN   (SCREEN_FPS * 2)
#define EXIT_MARGIN      56    // px de pies fuera de pantalla al huir

// ---------------------------------------------------------------------------
// Estado
// ---------------------------------------------------------------------------

// Estados de cada enemigo (FSM: ver el diagrama en docs/combat.md)
typedef enum {
    CT_GONE,      // liberado, slot vacío o no es de contacto
    CT_WAIT,      // aún no ha entrado (entradas escalonadas)
    CT_CHASE,     // persigue al jugador (con pausas y desvío aleatorios)
    CT_ATTACK,    // atacando (ANIM_ACTION; el golpe cae en el frame hit_at del perfil)
    CT_HURT,      // impactado (flash de STATE_HIT; lo cuenta update_enemy_animations)
    CT_RETREAT,   // corre al borde más cercano y vuelve a la carga
    CT_LEAVE      // huida final: corre al borde más cercano y se libera
} ContactState;

static u8  ct_state[MAX_ENEMIES];
static u16 ct_timer[MAX_ENEMIES];     // temporizador del estado (entrada, ciclo de ataque)
static s16 ct_exit_x[MAX_ENEMIES];    // x de pies destino de la huida en curso
static s8  ct_wander[MAX_ENEMIES];    // desvío vertical aleatorio del objetivo
static u16 ct_wander_t[MAX_ENEMIES];  // frames hasta re-sortear el desvío
static u16 ct_pause[MAX_ENEMIES];     // pausa aleatoria en curso (frames quieto)
static u16 ct_pause_cd[MAX_ENEMIES];  // cooldown entre pausas

static u8 ct_hits_to_win;             // impactos que ahuyentan a toda la manada
u16 contact_hits;                     // impactos acumulados (observable por RAM)

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

// ---------------------------------------------------------------------------
// Impactos
// ---------------------------------------------------------------------------
void contact_count_hit(void)
{
    contact_hits++;
}

// Un impacto sobre el enemigo: flash de daño y a huir (sin restar HP: los de
// contacto no mueren, escarmientan)
void contact_scare_enemy(u16 e)
{
    if (ct_state[e] == CT_GONE) return;
    obj_enemy[e].obj_character.state = STATE_HIT;   // el flash lo lleva update_enemy_animations
    obj_enemy[e].modeTimer = ENEMY_HURT_DURATION;
    ct_state[e] = CT_HURT;
}

void contact_scare_all(void)
{
    for (u16 e = 0; e < MAX_ENEMIES; e++)
        if (ct_state[e] == CT_CHASE || ct_state[e] == CT_ATTACK)
            contact_scare_enemy(e);
}

// Objetivo del golpe físico: el enemigo más cercano DELANTE del jugador
// (según su flip) y a su altura. MAX_ENEMIES si no hay ninguno.
u16 contact_find_in_front(s16 range_x, s16 range_y)
{
    bool facing_left = obj_character[active_character].flipH;
    s16 px = chr_feet_x(active_character);
    s16 py = chr_feet_y(active_character);
    u16 best = MAX_ENEMIES;
    s16 best_dist = 0x7FFF;

    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (ct_state[e] != CT_CHASE && ct_state[e] != CT_ATTACK && ct_state[e] != CT_RETREAT)
            continue;
        s16 dx = enemy_feet_x(e) - px;
        s16 dy = enemy_feet_y(e) - py;
        s16 fwd = facing_left ? -dx : dx;           // distancia hacia delante
        if (fwd < 0 || fwd > range_x) continue;
        if (dy < -range_y || dy > range_y) continue;
        if (fwd < best_dist) { best_dist = fwd; best = e; }
    }
    return best;
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
// FSM de cada enemigo, un tick por frame
// ---------------------------------------------------------------------------
static void tick_enemy(u16 e)
{
    Entity *en = &obj_enemy[e].obj_character;
    const ContactProfile *p = obj_enemy[e].class.contact;

    if (ct_state[e] == CT_GONE) return;
    if (!en->active || p == NULL) { ct_state[e] = CT_GONE; return; }

    // Con la manada ahuyentada, todo el mundo pasa a la huida final (el que
    // esté en pleno flash de daño la empieza al terminarlo)
    if (ct_hits_to_win && contact_hits >= ct_hits_to_win &&
        ct_state[e] != CT_LEAVE && ct_state[e] != CT_HURT) {
        ct_state[e] = CT_LEAVE;
        ct_exit_x[e] = nearest_exit_x(e);
        en->state = STATE_WALKING;
        en->animation = ANIM_RUN;
    }

    s16 px = chr_feet_x(active_character);
    s16 py = chr_feet_y(active_character);

    switch (ct_state[e]) {

    case CT_WAIT:   // entrada escalonada
        if (ct_timer[e]) { ct_timer[e]--; break; }
        ct_state[e] = CT_CHASE;
        en->state = STATE_WALKING;   // locomoción dirigida por este módulo
        break;

    case CT_CHASE: {
        // Desvío vertical aleatorio del objetivo: variedad de movimiento y
        // menos atascos contra el acompañante o entre enemigos
        if (ct_wander_t[e]) ct_wander_t[e]--;
        else {
            ct_wander[e] = (s8)(random() % (WANDER_RANGE * 2 + 1)) - WANDER_RANGE;
            ct_wander_t[e] = SCREEN_FPS + (random() & 63);
        }
        s16 wy = py + ct_wander[e];
        if (wy < (s16)y_limit_min) wy = y_limit_min;
        if (wy > (s16)y_limit_max) wy = y_limit_max;

        // ¿A distancia de ataque (medida contra el jugador real)?
        s16 dx = px - enemy_feet_x(e);
        s16 dy = py - enemy_feet_y(e);
        if (abs(dx) <= (s16)p->range_x && abs(dy) <= (s16)p->range_y) {
            ct_state[e] = CT_ATTACK;
            ct_timer[e] = p->attack_time;
            en->flipH = (dx < 0);
            en->animation = ANIM_ACTION;
            SPR_setAnimAndFrame(spr_enemy[e], ANIM_ACTION, 0);
            update_enemy(e);
            break;
        }

        // Pausa aleatoria: se queda quieto ~1 s y sigue (cooldown por enemigo)
        if (ct_pause[e]) {
            ct_pause[e]--;
            en->animation = ANIM_IDLE;
            update_enemy(e);
            break;
        }
        if (ct_pause_cd[e]) {
            ct_pause_cd[e]--;
        } else if ((random() & PAUSE_CHANCE) == 0) {
            ct_pause[e] = SCREEN_FPS - 16 + (random() & 31);   // ~0,7–1,3 s
            ct_pause_cd[e] = PAUSE_COOLDOWN;
            en->animation = ANIM_IDLE;
            update_enemy(e);
            break;
        }

        if (!step_towards(e, px, wy, CHASE_SPEED_X, CHASE_SPEED_Y, ANIM_WALK, true)) {
            ct_wander_t[e] = 0;         // bloqueado: re-sortear el desvío ya
            en->animation = ANIM_IDLE;
            update_enemy(e);
        }
        break;
    }

    case CT_ATTACK:
        if (ct_timer[e]) ct_timer[e]--;
        if (ct_timer[e] == p->attack_time - p->hit_at) {
            s16 dx = px - enemy_feet_x(e);
            s16 dy = py - enemy_feet_y(e);
            if (abs(dx) <= (s16)p->range_x && abs(dy) <= (s16)p->range_y) {
                dprintf(2, "Contact: ataque de %d", e);
                hit_player(p->damage);
            }
        }
        if (ct_timer[e] == 0) {
            ct_state[e] = CT_CHASE;
            en->animation = ANIM_WALK;
        }
        break;

    case CT_HURT:
        // update_enemy_animations lleva el flash y devuelve el estado a IDLE
        if (en->state != STATE_HIT) {
            ct_state[e] = CT_RETREAT;
            ct_exit_x[e] = nearest_exit_x(e);
            en->state = STATE_WALKING;
            en->animation = ANIM_RUN;
        }
        break;

    case CT_RETREAT:
        if (!step_towards(e, ct_exit_x[e], enemy_feet_y(e), RUN_SPEED, RUN_SPEED, ANIM_RUN, false)) {
            ct_state[e] = CT_CHASE;     // llegó fuera de pantalla: vuelve a la carga
            en->animation = ANIM_WALK;
        }
        break;

    case CT_LEAVE:
        if (!step_towards(e, ct_exit_x[e], enemy_feet_y(e), RUN_SPEED, RUN_SPEED, ANIM_RUN, false)) {
            release_enemy(e);           // llegó fuera de pantalla: se va del todo
            ct_state[e] = CT_GONE;
        }
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// API del director
// ---------------------------------------------------------------------------
void contact_reset(u8 hits_to_win)
{
    ct_hits_to_win = hits_to_win;
    contact_hits = 0;

    // Tomar el control de los enemigos de CONTACTO ya spawneados
    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (obj_enemy[e].obj_character.active &&
            obj_enemy[e].class.role == ENEMY_ROLE_CONTACT &&
            obj_enemy[e].class.contact != NULL) {
            ct_state[e] = CT_WAIT;
            ct_timer[e] = e * ENTER_STAGGER;
            ct_wander[e] = 0;
            ct_wander_t[e] = (random() & 31);
            ct_pause[e] = 0;
            ct_pause_cd[e] = SCREEN_FPS;   // sin pausas nada más entrar
        } else {
            ct_state[e] = CT_GONE;
        }
    }
}

void contact_tick(void)
{
    for (u16 e = 0; e < MAX_ENEMIES; e++)
        tick_enemy(e);
}

bool contact_all_gone(void)
{
    for (u16 e = 0; e < MAX_ENEMIES; e++)
        if (ct_state[e] != CT_GONE) return false;
    return true;
}

void contact_release_all(void)
{
    for (u16 e = 0; e < MAX_ENEMIES; e++) {
        if (ct_state[e] != CT_GONE && obj_enemy[e].obj_character.active)
            release_enemy(e);
        ct_state[e] = CT_GONE;
    }
}
