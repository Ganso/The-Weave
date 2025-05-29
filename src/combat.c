#include "globals.h"

CombatState combat_state; // Current combat state
CombatContext combatContext; // Combat context

// --------------------------------
// LOCAL HELPERS
// --------------------------------

// Returns TRUE if pattern slot pslot for enemy eid can be launched
static inline bool enemyPatternReady(u8 eid, u8 pslot)
{
    EnemyPattern *p = &enemyPatterns[eid][pslot];
    dprintf(3, "Checking enemy pattern %d:%d: enabled=%d, rechargeFrames=%d",
            eid, pslot, p->enabled, p->rechargeFrames);
    return p->enabled && (p->rechargeFrames == 0);
}

// Decrements cooldowns and launches the first ready pattern.
// Returns TRUE when a launch happened this frame.
static bool enemyChooseAndLaunch(void)
{
    dprintf(2, "Checking enemy patterns to launch");

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
                if (enemyPatternReady(e, p))
                {
                    dprintf(2, "Enemy %d launching pattern %d", e, p);
                    launchEnemyPattern(e, p);          // switches combat_state
                    return TRUE;
                }

    return FALSE;   // nobody ready yet
}


// --------------------------------
// Combat functions
// --------------------------------

// Try to counter an enemy spell
bool tryCounterSpell(void)
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
void combatInit(void)
{
    dprintf(2,"Starting combat phase");

    combat_state = COMBAT_STATE_IDLE; // Set initial state;
    combatContext.effectTimer = 0;
    combatContext.patternReversed = false;
    combatContext.playerNotes = 0;
    combatContext.enemyNoteIndex  = 0;
    combatContext.enemyNoteTimer  = 0;

    // Initialize every active enemy's patterns
    for (u8 id = 0; id < MAX_ENEMIES; id++)
        if (obj_enemy[id].obj_character.active)   
            initEnemyPatterns(id);

    show_or_hide_interface(true);
}

// Finish combat phase
void combatFinish(void)
{
    dprintf(2,"Finishing combat phase");

    // Reset combat context
    combat_state = COMBAT_NO;
}

// Hit an enemy
void hit_enemy(u8 enemyId, u8 damage)
{
    if (enemyId >= MAX_ENEMIES || !obj_enemy[enemyId].obj_character.active) return;
  
    // Reduce enemy HP
    obj_enemy[enemyId].hitpoints -= damage;
    if (obj_enemy[enemyId].hitpoints <= 0) {
        // TODO: Show enemy defeated animation
        dprintf(2, "Enemy %d defeated", enemyId);
        release_enemy(enemyId); // Enemy defeated
    }
    else {
        dprintf(2, "Enemy %d hit for %d damage, remaining HP: %d", enemyId, damage, obj_enemy[enemyId].hitpoints);
        SPR_setAnim(spr_enemy[enemyId], ANIM_HURT); // Show hurt animation
        play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));
        obj_enemy[enemyId].obj_character.state = STATE_HIT; // Set enemy state to HURT
    }
}

// Hit the player
void hit_player(u8 damage)
{
    dprintf(2, "Player hit for %d damage", damage);
    obj_character[active_character].state = STATE_HIT; // Set player state to HURT
    play_sample(snd_player_hurt, sizeof(snd_player_hurt));  // feedback
}

// Update combat state.  Call every frame while combat_state != COMBAT_STATE_NO
void update_combat(void)
{
    dprintf(2,"Checking combat state: %d", combat_state);

    // -----------------------------------------------------------------
    //  A) ALWAYS advance the active enemy pattern.
    //     • Durante ENEMY_PLAYING cuenta sus notas.
    //     • Durante ENEMY_EFFECT gestiona el flash / daño / timeout.
    // -----------------------------------------------------------------
    if (combatContext.activeEnemy != ENEMY_NONE)
    {
        updateEnemyPattern(combatContext.activeEnemy);
    }

    // Count down the global lock between patterns
    if (combatContext.patternLockTimer)
        --combatContext.patternLockTimer;

    // -----------------------------------------------------------------
    //  B) Global finite-state machine
    // -----------------------------------------------------------------
    switch (combat_state)
    {
    // ------------------------------------------------------------------ IDLE
    case COMBAT_STATE_IDLE:
        // If the hero sprite is still in its HIT pose, wait.
        if (obj_character[active_character].state == STATE_HIT)
            break;

        // Enemy AI: try to cast a new pattern (sets ENEMY_PLAYING if ready).
        enemyChooseAndLaunch();
        break;

    // --------------------------------------------------------- PLAYER lanes
    case COMBAT_STATE_PLAYER_PLAYING:
        // Note input is handled elsewhere; nothing to update here.
        break;

    case COMBAT_STATE_PLAYER_EFFECT:
        // When the player spell animation/effect finishes, return to IDLE.
        if (updatePlayerPattern())
            combat_state = COMBAT_STATE_IDLE;
        break;

    // ---------------------------------------------------------- ENEMY lanes
    case COMBAT_STATE_ENEMY_PLAYING:
        // Nothing extra: enemy notes advance in updateEnemyPattern() above.
        break;

    case COMBAT_STATE_ENEMY_EFFECT:
        // All timing & end-of-effect logic also live in updateEnemyPattern().
        break;

    default: break;           // COMBAT_STATE_NO or undefined
    }
}