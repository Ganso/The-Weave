#include <genesis.h>
#include "core/config.h"
#include "core/hack.h"
#include "combat/combat.h"
#include "spells/spell.h"
#include "spells/notes.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "actors/enemies.h"
#include "core/frame.h"
#include "world/background.h"
#include "interface/interface.h"
#include "audio/sound.h"
#include "res_sound.h"

CombatState combat_state; // Current combat state

// --------------------------------
// LOCAL HELPERS
// --------------------------------

// Returns TRUE if at least one enemy entity is still active
static bool any_enemy_active(void)
{
    for (u8 i = 0; i < MAX_ENEMIES; ++i)
        if (obj_enemy[i].obj_character.active)
            return true;                    // Found a live enemy → keep combat
    return false;                           // No enemies left → combat can end
}

// --------------------------------
// Combat functions
// --------------------------------

// Start combat phase
void combat_init(void)
{
    dprintf(2,"Starting combat phase");

    combat_state = COMBAT_STATE_IDLE; // Set initial state;
    spell_engine_reset();

    player_scroll_active = false; // Disable player scroll during combat

    // Initialize every active enemy's spell recharges
    for (u8 id = 0; id < MAX_ENEMIES; id++)
        if (obj_enemy[id].obj_character.active)
            init_enemy_spells(id);

    show_or_hide_interface(true);
}

// Finish combat phase
void combat_finish(void)
{
    dprintf(2,"Finishing combat phase");

    // Reset combat context
    combat_state = COMBAT_NO;

    player_scroll_active = true; // Enable player scroll after combat
}

// Hit an enemy
void hit_enemy(u8 enemyId, u8 damage)
{
    if (enemyId >= MAX_ENEMIES || !obj_enemy[enemyId].obj_character.active) return; // If enemy does not exist or is inactive, do nothing
    if (obj_enemy[enemyId].obj_character.state == STATE_HIT) return; // If enemy is already hit, do nothing

    dprintf(2, "Hit enemy %d for %d damage", enemyId, damage);

    if (HACK_ENEMIES_ONE_HP) damage = obj_enemy[enemyId].hitpoints; // Dev hack (core/hack.h)

    // Reduce enemy HP (B25: compare first — hitpoints is u16 and would underflow if damage > HP)
    if (damage >= obj_enemy[enemyId].hitpoints) { // If enemy is defeated
        // TODO: Better enemy defeat handling (pospuesto a Fase 4, decisión refactorizar.md §15)
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

    // Flash + sound
    play_sample(snd_player_hurt, sizeof(snd_player_hurt));
    anim_character(active_character, ANIM_HURT);

    // Enter HURT state with fixed timer
    obj_character[active_character].state     = STATE_HIT;
    obj_character[active_character].modeTimer = PLAYER_HURT_DURATION;

    // Block any new player pattern during the stun
    notes_set_lock(PLAYER_HURT_DURATION);
}


// Update combat state.  Call every frame while combat_state != COMBAT_STATE_NO
// Call once per frame when combat_state != COMBAT_STATE_NO
void update_combat(void)
{
    // --- A) El motor avanza ambos slots (y el lock de input) ----------
    spell_update();

    // --- B) FSM -------------------------------------------------------
    switch (combat_state)
    {
    case COMBAT_NO:
        break; // No combat active, nothing to do

    case COMBAT_STATE_IDLE:
        if (obj_character[active_character].state == STATE_HIT) break; //  If the player was just hit, stay here one frame
        if (!any_enemy_active()) { // If no enemy is alive any more, leave the combat loop
            set_idle();
            break;
        }
        spell_enemy_try_launch(); // Otherwise let enemies try to launch a spell
        break;

    case COMBAT_STATE_PLAYER_PLAYING:  break;   // input handled elsewhere (notes.c)
    case COMBAT_STATE_PLAYER_EFFECT:   break;   // el motor gestiona el efecto y el fin
    case COMBAT_STATE_ENEMY_PLAYING:   break;   // idem (cadencia de notas del enemigo)
    case COMBAT_STATE_ENEMY_EFFECT:    break;   // idem

    default: break;
    }
}

// Set combat state to idle or none, depending on the context
void set_idle(void) {
    // Check if there's active enemies
    bool hasActiveEnemies = false;
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        if (obj_enemy[i].obj_character.active) {
            hasActiveEnemies = true;
            break;
        }
    }
    if (hasActiveEnemies) {
        combat_state = COMBAT_STATE_IDLE; // Set to idle if there are active enemies
    } else {
        combat_state = COMBAT_NO; // No combat active
    }

    // Set player state to idle
    obj_character[active_character].state = STATE_IDLE;

    dprintf(2, "All is quiet. Combat state set to %d", combat_state);
}
