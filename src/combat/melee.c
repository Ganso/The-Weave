// melee.c — director del combate de CONTACTO (transitorio, hito 1 del refactor).
// La lógica vive ya en contact.c (FSM de la manada) y weapons.c (golpe con A y
// regla del trueno); aquí solo queda la composición y la limpieza. El hito 2
// sustituye este módulo por el director único de combat.c (docs/combat.md §8).

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "audio/audio.h"
#include "res_all.h"
#include "combat/melee.h"

#define COMPANION_GAP 44   // distancia a la que se recoloca el acompañante

// ---------------------------------------------------------------------------
// El acompañante (Clio): deja de seguir y espera quieto durante el combate
// ---------------------------------------------------------------------------
static bool stage_companion(const MeleeConfig *cfg)
{
    u16 c = cfg->companion;
    if (c >= MAX_CHR || !obj_character[c].active) return false;

    bool was_following = obj_character[c].follows_character;
    obj_character[c].follows_character = false;

    s16 px = FASTFIX32_TO_INT(obj_character[active_character].x);
    if (cfg->reposition_companion &&
        FASTFIX32_TO_INT(obj_character[c].x) > px - (COMPANION_GAP / 2))
        move_character(c, px - COMPANION_GAP,
                       FASTFIX32_TO_INT(obj_character[active_character].y) +
                       obj_character[active_character].y_size);

    look_left(c, false);   // mirando a la derecha
    obj_character[c].state = STATE_IDLE;
    anim_character(c, ANIM_IDLE);
    return was_following;
}

static void unstage_companion(const MeleeConfig *cfg, bool was_following)
{
    u16 c = cfg->companion;
    if (c < MAX_CHR && obj_character[c].active && was_following)
        obj_character[c].follows_character = true;
}

// ---------------------------------------------------------------------------
// Bucle principal (bloqueante)
// ---------------------------------------------------------------------------
void melee_combat_run(const MeleeConfig *config)
{
    dprintf(2, "Melee: inicio (%d impactos, arma=%s)", config->hits_to_win,
            config->weapon_is_thunder ? "trueno" : "golpe");

    bool comp_was_following = stage_companion(config);

    contact_reset(config->hits_to_win);
    weapons_reset(!config->weapon_is_thunder);   // el golpe solo si no es modo trueno

    player_hitpoints = player_max_hitpoints;     // la vida se reinicia en cada combate
    player_defeated = false;
    combat_state = COMBAT_NO;

    bool old_scroll = player_scroll_active;
    player_scroll_active = false;                // arena fija durante el combate

    while (!contact_all_gone() && !player_defeated) {
        next_frame(true);

        // El motor de patrones deja combat_state en IDLE tras un cast del
        // jugador si ve enemigos activos; aquí el director es el melee, así
        // que ese residuo se normaliza (evita tutoriales y estados corruptos)
        if (combat_state == COMBAT_STATE_IDLE) combat_state = COMBAT_NO;

        if (config->weapon_is_thunder) combat_rule_thunder_scares();
        else weapons_tick();
        contact_tick();
    }

    // Derrota: liberar a los que queden (la escena decide el reintento)
    if (player_defeated) contact_release_all();

    // Dejar el motor limpio: jugador disponible, sin hechizos ni notas a
    // medias y sin estados de combate residuales
    if (obj_character[active_character].state == STATE_PLAYING_NOTE)
        obj_character[active_character].state = STATE_IDLE;
    if (spell_slot_active(SPELL_SLOT_PLAYER))
        spell_cancel(SPELL_SLOT_PLAYER);
    reset_note_queue();
    combat_state = COMBAT_NO;
    player_scroll_active = old_scroll;
    unstage_companion(config, comp_was_following);

    dprintf(2, "Melee: fin (%d impactos, %s)", contact_hits,
            player_defeated ? "derrota" : "victoria");
}
