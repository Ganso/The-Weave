// combat.c — el director de combate. Contrato y doctrina en combat.h;
// funcionamiento detallado en docs/combat.md.
//
// Estructura del archivo:
//   1. Vida del jugador y daño (hit_enemy / hit_player) — común a ambos roles
//   2. El encuentro: config, acompañante, start/tick/end/run
//   3. El motor de patrones por frame (update_combat) y set_idle

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"

CombatState combat_state; // FSM de turnos del motor de PATRONES (ver combat.h)

u16 player_max_hitpoints = 5;   // vida inicial de cada combate
u16 player_hitpoints = 5;       // vida restante del combate en curso
bool player_defeated;           // el último combate acabó con la vida a 0

// Estado del encuentro en curso
static CombatConfig cfg;            // configuración consumida al arrancar
static CombatConfig pending_cfg;    // la deja el hook con combat_configure()
static bool pending_set;
static bool encounter_active;       // hay un encuentro entre start y el fin
static bool comp_was_following;     // follows_character previo del acompañante
static bool old_scroll;             // player_scroll_active previo al encuentro

#define COMPANION_GAP 44   // distancia a la que se recoloca el acompañante

// --------------------------------
// LOCAL HELPERS
// --------------------------------

// Returns TRUE if at least one enemy entity is still active
static bool any_enemy_active(void)
{
    for (u8 i = 0; i < MAX_ENEMIES; ++i)
        if (obj_enemy[i].obj_character.active)
            return true;
    return false;
}

// ---------------------------------------------------------------------------
// 1. Daño (común a ambos roles)
// ---------------------------------------------------------------------------

// Hit an enemy
void hit_enemy(u8 enemyId, u8 damage)
{
    if (enemyId >= MAX_ENEMIES || !obj_enemy[enemyId].obj_character.active) return; // If enemy does not exist or is inactive, do nothing
    if (obj_enemy[enemyId].obj_character.state == STATE_HIT) return; // If enemy is already hit, do nothing

    dprintf(2, "Hit enemy %d for %d damage", enemyId, damage);

    if (HACK_ENEMIES_ONE_HP) damage = obj_enemy[enemyId].hitpoints; // Dev hack (core/hack.h)

    // Reduce enemy HP (B25: compare first — hitpoints is u16 and would underflow if damage > HP)
    if (damage >= obj_enemy[enemyId].hitpoints) { // If enemy is defeated
        // TODO: Better enemy defeat handling (diferido)
        dprintf(2, "Enemy %d defeated", enemyId);
        obj_enemy[enemyId].hitpoints = 0; // Marks the enemy as dying
        SPR_setVisibility(spr_int_life_counter, HIDDEN); // Hide life counter sprite
        anim_enemy(enemyId, ANIM_HURT); // Death animation (via entity field so update_enemy keeps it)
        play_sample(snd_effect_magic_disappear, sizeof(snd_effect_magic_disappear)); // Play hit sound
        // B4: no blocking wait here. STATE_HIT pauses AI/pattern launches; the release
        // happens in update_enemy_animations when the death animation finishes.
        obj_enemy[enemyId].obj_character.state = STATE_HIT;
        obj_enemy[enemyId].modeTimer = ENEMY_HURT_DURATION;
    }
    else {
        obj_enemy[enemyId].hitpoints -= damage;
        dprintf(2, "Enemy %d hit for %d damage, remaining HP: %d", enemyId, damage, obj_enemy[enemyId].hitpoints);
        SPR_setAnim(spr_enemy[enemyId], ANIM_HURT); // Show hurt animation
        play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
        obj_enemy[enemyId].obj_character.state = STATE_HIT; // Set enemy state to HURT
        obj_enemy[enemyId].modeTimer = ENEMY_HURT_DURATION; // Set a timer for the hurt state
        dprintf(2, "Enemy %d state set to HURT", enemyId);
    }
}

// Hit the player
void hit_player(u8 damage)
{
    if (HACK_PLAYER_INVULNERABLE) return; // Dev hack (core/hack.h)

    // Ignore if the hero is already hurt
    if (obj_character[active_character].state == STATE_HIT) return;

    dprintf(2,"Player hit for %d damage", damage);

    // Restar vida; a 0 queda marcada la derrota (el tick del encuentro la recoge)
    if (damage >= player_hitpoints) {
        player_hitpoints = 0;
        player_defeated = true;
        dprintf(2,"Player defeated");
    } else {
        player_hitpoints -= damage;
    }

    // Flash + sound
    play_sample(snd_player_hurt, sizeof(snd_player_hurt));
    anim_character(active_character, ANIM_HURT);

    // Enter HURT state with fixed timer
    obj_character[active_character].state     = STATE_HIT;
    obj_character[active_character].modeTimer = PLAYER_HURT_DURATION;

    // Block any new player pattern during the stun
    notes_set_lock(PLAYER_HURT_DURATION);
}

// ---------------------------------------------------------------------------
// 2. El encuentro
// ---------------------------------------------------------------------------

void combat_configure(const CombatConfig *config)
{
    pending_cfg = *config;
    pending_set = true;
}

void combat_config_clear(void)
{
    pending_set = false;
}

bool combat_ranged_present(void)
{
    if (!encounter_active) return false;
    for (u8 i = 0; i < MAX_ENEMIES; ++i)
        if (obj_enemy[i].obj_character.active &&
            obj_enemy[i].class.role == ENEMY_ROLE_RANGED)
            return true;
    return false;
}

// El acompañante deja de seguir y espera quieto durante el combate
static void stage_companion(void)
{
    u16 c = cfg.companion;
    comp_was_following = false;
    if (c >= MAX_CHR || c == active_character || !obj_character[c].active) return;

    comp_was_following = obj_character[c].follows_character;
    obj_character[c].follows_character = false;

    s16 px = FASTFIX32_TO_INT(obj_character[active_character].x);
    if (cfg.reposition_companion &&
        FASTFIX32_TO_INT(obj_character[c].x) > px - (COMPANION_GAP / 2))
        move_character(c, px - COMPANION_GAP,
                       FASTFIX32_TO_INT(obj_character[active_character].y) +
                       obj_character[active_character].y_size);

    look_left(c, false);
    obj_character[c].state = STATE_IDLE;
    anim_character(c, ANIM_IDLE);
}

static void unstage_companion(void)
{
    u16 c = cfg.companion;
    if (c < MAX_CHR && c != active_character &&
        obj_character[c].active && comp_was_following)
        obj_character[c].follows_character = true;
}

void combat_start(void)
{
    // Consumir la config pendiente; por defecto: solo patrones, sin acompañante
    cfg = pending_set ? pending_cfg : (CombatConfig){ .companion = CHR_NONE };
    pending_set = false;
    encounter_active = true;

    dprintf(2,"Combat: inicio (strike=%d, hits_to_win=%d)", cfg.weapon_strike, cfg.hits_to_win);

    // Vida y motor de hechizos limpios
    player_hitpoints = player_max_hitpoints;
    player_defeated = false;
    spell_engine_reset();

    // Recargas de hechizos de los enemigos (los de contacto no tienen: no-op)
    for (u8 id = 0; id < MAX_ENEMIES; id++)
        if (obj_enemy[id].obj_character.active)
            init_enemy_spells(id);

    // Subsistemas del encuentro
    contact_reset(cfg.hits_to_win);
    weapons_reset(cfg.weapon_strike);
    stage_companion();

    // Arena fija durante el combate (se restaura el valor previo al salir)
    old_scroll = player_scroll_active;
    player_scroll_active = false;

    if (player_patterns_enabled) show_or_hide_interface(true);

    combat_state = combat_ranged_present() ? COMBAT_STATE_IDLE : COMBAT_NO;

    if (cfg.onStart) cfg.onStart();
}

void combat_tick(void)
{
    if (!encounter_active) return;

    weapons_tick();                  // el golpe con A (no-op si no está habilitado)
    if (cfg.onTick) cfg.onTick();    // reglas propias del encuentro
    contact_tick();                  // FSM de los enemigos de contacto

    // ¿Fin del encuentro?
    if (player_defeated) { encounter_active = false; return; }
    bool won = cfg.isWon ? cfg.isWon() : !any_enemy_active();
    if (won) encounter_active = false;
}

bool combat_running(void)
{
    return encounter_active;
}

void combat_end(void)
{
    dprintf(2,"Combat: fin (%s)", player_defeated ? "derrota" : "victoria");

    if (cfg.onEnd) cfg.onEnd();

    // Los enemigos que queden se liberan (en derrota siempre; con una
    // condición de victoria propia también podrían quedar vivos)
    for (u8 i = 0; i < MAX_ENEMIES; i++)
        if (obj_enemy[i].obj_character.active)
            release_enemy(i);        // libera también su slot de hechizo
    contact_release_all();

    // Dejar el motor limpio: jugador disponible, sin hechizos ni notas a medias
    if (obj_character[active_character].state == STATE_PLAYING_NOTE ||
        obj_character[active_character].state == STATE_PATTERN_EFFECT)
        obj_character[active_character].state = STATE_IDLE;
    if (spell_slot_active(SPELL_SLOT_PLAYER))
        spell_cancel(SPELL_SLOT_PLAYER);
    reset_note_queue();

    combat_state = COMBAT_NO;
    player_scroll_active = old_scroll;
    unstage_companion();
    encounter_active = false;
}

// Vía bloqueante: la usa el op `combat` de la VM (y cualquier hook que quiera)
void combat_run(void)
{
    combat_start();
    while (combat_running())
        { next_frame(true); combat_tick(); }
    combat_end();
}

// ---------------------------------------------------------------------------
// 3. Motor de patrones por frame
// ---------------------------------------------------------------------------

// Update por frame del motor de PATRONES. La llama next_frame() SIEMPRE (los
// cast libres fuera de combate también viven aquí).
void update_combat(void)
{
    // --- A) El motor avanza ambos slots (y el lock de input) ----------
    spell_update();

    // --- B) FSM de turnos ----------------------------------------------
    switch (combat_state)
    {
    case COMBAT_NO:
        break; // sin patrones en juego

    case COMBAT_STATE_IDLE:
        if (obj_character[active_character].state == STATE_HIT) break; // recién golpeado: espera un frame
        if (!any_enemy_active()) { // no queda nadie: cerrar el lado de patrones
            set_idle();
            break;
        }
        spell_enemy_try_launch(); // dejar que los enemigos intenten lanzar
        break;

    case COMBAT_STATE_PLAYER_PLAYING:  break;   // input en notes.c
    case COMBAT_STATE_PLAYER_EFFECT:   break;   // el motor gestiona el efecto y el fin
    case COMBAT_STATE_ENEMY_PLAYING:   break;   // idem (cadencia de notas del enemigo)
    case COMBAT_STATE_ENEMY_EFFECT:    break;   // idem

    default: break;
    }
}

// Repone combat_state al estado de reposo que toque: IDLE si el encuentro
// tiene enemigos a distancia vivos (combate de patrones esperando), COMBAT_NO
// en cualquier otro caso (cast libre, combate solo-contacto, sin combate).
void set_idle(void)
{
    combat_state = combat_ranged_present() ? COMBAT_STATE_IDLE : COMBAT_NO;

    // Set player state to idle
    obj_character[active_character].state = STATE_IDLE;

    dprintf(2, "All is quiet. Combat state set to %d", combat_state);
}
