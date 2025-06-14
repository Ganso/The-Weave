#include "globals.h"

CombatState combat_state; // Current combat state
CombatContext combatContext; // Combat context

// --------------------------------
// LOCAL HELPERS
// --------------------------------

// Returns TRUE if pattern slot pslot for enemy eid can be launched
static inline bool enemy_pattern_ready(u8 eid, u8 pslot)
{
    EnemyPattern *p = &enemyPatterns[eid][pslot];
    dprintf(3, "Checking enemy pattern %d:%d: enabled=%d, rechargeFrames=%d",
            eid, pslot, p->enabled, p->rechargeFrames);
    return p->enabled && (p->rechargeFrames == 0);
}

// Decrements cooldowns and launches the first ready pattern.
// Returns TRUE when a launch happened this frame.
static bool enemy_choose_and_launch(void)
{
    dprintf(3, "Checking enemy patterns to launch");

    // Suspend launches while the player is recovering
    if (obj_character[active_character].state == STATE_HIT)
        return FALSE;

    // If any enemy is hit, we cannot launch a patterns -------------------
    for (u8 e = 0; e < MAX_ENEMIES; ++e)
        if (obj_enemy[e].obj_character.active &&
            obj_enemy[e].obj_character.state == STATE_HIT)
            return false; 

    // Tick all cooldowns ------------------------------------------------
    for (u8 e = 0; e < MAX_ENEMIES; ++e)
        if (obj_enemy[e].obj_character.active)
            for (u8 p = 0; p < MAX_PATTERN_ENEMY; ++p)
                if (enemyPatterns[e][p].rechargeFrames)
                    --enemyPatterns[e][p].rechargeFrames;

    // Launch first available pattern -----------------------------------
    for (u8 e = 0; e < MAX_ENEMIES; ++e)
        if (obj_enemy[e].obj_character.active)
            for (u8 p = 0; p < MAX_PATTERN_ENEMY; ++p)
                if (enemy_pattern_ready(e, p))
                {
                    dprintf(2, "Enemy %d launching pattern %d", e, p);
                    launch_enemy_pattern(e, p);          // switches combat_state
                    return TRUE;
                }

    return FALSE;   // nobody ready yet
}

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

// Try to counter an enemy spell
bool try_counter_spell(void)
{
    if (!combatContext.patternReversed ||
        combat_state != COMBAT_STATE_ENEMY_EFFECT) return false;

    EnemyPattern* ep = // Search active pattern
        &enemyPatterns[combatContext.activeEnemy][0]; // slot 0 = thunder

    if (!ep->counterable || !ep->onCounter) return false;

    // Cancel enemy effect
    ep->onCounter(combatContext.activeEnemy);
    combatContext.effectTimer = 0;
    return true;
}

// Start combat phase
void combat_init(void)
{
    dprintf(2,"Starting combat phase");

    combat_state = COMBAT_STATE_IDLE; // Set initial state;
    combatContext.effectTimer = 0;
    combatContext.patternReversed = false;
    combatContext.playerNotes = 0;
    combatContext.enemyNoteIndex  = 0;
    combatContext.enemyNoteTimer  = 0;

    player_scroll_active = false; // Disable player scroll during combat

    // Initialize every active enemy's patterns
    for (u8 id = 0; id < MAX_ENEMIES; id++)
        if (obj_enemy[id].obj_character.active)   
            init_enemy_patterns(id);

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

    // Reduce enemy HP
    obj_enemy[enemyId].hitpoints -= damage;
    if (obj_enemy[enemyId].hitpoints <= 0) { // If enemy is defeated
        // TODO: Better enemy defeat handling
        dprintf(2, "Enemy %d defeated", enemyId);
        SPR_setVisibility(spr_int_life_counter, HIDDEN); // Hide life counter sprite
        SPR_setAnim(spr_enemy[enemyId], ANIM_HURT); // Show hurt animation
        play_sample(snd_effect_magic_disappear, sizeof(snd_effect_magic_disappear)); // Play hit sound
        while (!SPR_isAnimationDone(spr_enemy[enemyId])) {
            SPR_update(); // Update sprite animations
            SYS_doVBlankProcess(); // Wait for animation to finish
        }
        release_enemy(enemyId); // Enemy defeated
    }
    else {
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
    combatContext.patternLockTimer = PLAYER_HURT_DURATION;
}


// Update combat state.  Call every frame while combat_state != COMBAT_STATE_NO
// Call once per frame when combat_state != COMBAT_STATE_NO
void update_combat(void)
{
    // --- A) Count down global pattern lock --------------------------
    if (combatContext.patternLockTimer)
        --combatContext.patternLockTimer;

    // --- B) Always advance the enemy pattern (notes or flash) -------
    if (combatContext.activeEnemy != ENEMY_NONE)
        update_enemy_pattern(combatContext.activeEnemy);

    // --- C) FSM ------------------------------------------------------
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
       enemy_choose_and_launch(); // Otherwise let enemies try to launch a pattern
       break;

    case COMBAT_STATE_PLAYER_PLAYING:  break;       // input handled elsewhere

    case COMBAT_STATE_PLAYER_EFFECT:
        if (update_player_pattern())                   // pattern finished
            combat_state = COMBAT_STATE_IDLE;
        break;

    case COMBAT_STATE_ENEMY_PLAYING:  break;        // logic runs in B

    case COMBAT_STATE_ENEMY_EFFECT:   break;        // idem

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
