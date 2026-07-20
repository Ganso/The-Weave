// weapons.c — armas del jugador y reglas prefab. Contrato en weapons.h.
//
// El golpe físico reutiliza STATE_PLAYING_NOTE → ANIM_ACTION (bloquea el
// movimiento del jugador mientras dura, como tocar una nota). El impacto se
// resuelve contra los enemigos de contacto (contact_find_in_front).

#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "audio/audio.h"
#include "res_all.h"
#include "combat/weapons.h"
#include "combat/contact.h"

// ---------------------------------------------------------------------------
// Parámetros de ajuste del golpe (playtest)
// ---------------------------------------------------------------------------
#define ATTACK_TIME      20    // duración del golpe (5 frames x timer 4)
#define ATTACK_HIT_AT    8     // frame del golpe en el que impacta
#define ATTACK_RANGE_X   44    // alcance del golpe hacia delante
#define ATTACK_RANGE_Y   12
#define ATTACK_COOLDOWN  30    // frames mínimos entre golpes

static bool strike_on;          // el golpe con A está disponible en este encuentro
static u16  attack_timer;       // frames restantes del golpe en curso (0 = ninguno)
static u16  attack_cd;
static u16  prev_joy;
static bool thunder_was_active; // flanco de subida del cast de TRUENO (regla prefab)

void weapons_reset(bool strike_enabled)
{
    strike_on = strike_enabled;
    attack_timer = 0;
    attack_cd = 15;                      // margen para no golpear con el A del diálogo
    prev_joy = JOY_readJoypad(JOY_ALL);
    thunder_was_active = false;
}

// ---------------------------------------------------------------------------
// Golpe físico con A
// ---------------------------------------------------------------------------
void weapons_tick(void)
{
    if (!strike_on) return;

    u16 joy = JOY_readJoypad(JOY_ALL);
    u16 pressed = joy & ~prev_joy;
    prev_joy = joy;

    if (attack_cd) attack_cd--;

    if (attack_timer) {
        attack_timer--;

        // Momento del impacto (si no nos han golpeado antes)
        if (attack_timer == ATTACK_TIME - ATTACK_HIT_AT &&
            obj_character[active_character].state == STATE_PLAYING_NOTE) {
            u16 e = contact_find_in_front(ATTACK_RANGE_X, ATTACK_RANGE_Y);
            if (e < MAX_ENEMIES) {
                dprintf(2, "Weapons: golpe a %d", e);
                contact_count_hit();
                play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
                contact_scare_enemy(e);
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
// Reglas prefab de encuentro
// ---------------------------------------------------------------------------

// El TRUENO ahuyenta a la manada: en cuanto el cast arranca (flanco de
// subida), espanta a todos los de contacto y cuenta un impacto
void combat_rule_thunder_scares(void)
{
    bool active = (spell_active_id(SPELL_SLOT_PLAYER) == SPELL_THUNDER);
    bool launched = active && !thunder_was_active;
    thunder_was_active = active;
    if (!launched) return;

    dprintf(2, "Regla: trueno ahuyenta (impacto %d)", contact_hits + 1);
    contact_count_hit();
    play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
    contact_scare_all();
}
