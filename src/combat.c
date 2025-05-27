#include "globals.h"

CombatState combat_state; // Current combat state
CombatContext combatContext; // Combat context

// Try to counter an enemy spell
bool tryCounterSpell(void)
{
    if (!combatContext.patternReversed ||
        combat_state != COMBAT_STATE_ENEMY_EFFECT) return false;

    EnemyPattern* ep = // Search active pattern
        &enemyPatterns[combatContext.activeEnemy][0]; // slot 0 = thunder

    if (!ep->counterable || !ep->onCounter) return false;

    // Cancela el efecto enemigo
    ep->onCounter(combatContext.activeEnemy);
    combatContext.effectTimer = 0;
    hit_enemy(combatContext.activeEnemy, 1); // -1 HP to the enemy
    return true;
}

// Start combat phase
void combatInit(void)
{
    combat_state = COMBAT_STATE_ENEMY_PLAYING;
    combatContext.effectTimer = 0;
    combatContext.patternReversed = false;
    combatContext.playerNotes = 0;

    // Initialize every active enemy's patterns
    for (u8 id = 0; id < MAX_ENEMIES; id++)
        if (obj_enemy[id].obj_character.active)   
            initEnemyPatterns(id);

    show_or_hide_interface(true);
}

// Finish combat phase
void combatFinish(void)
{
    // Reset combat context
    combat_state = COMBAT_NO;
}

// Hit an enemy
void hit_enemy(u8 enemyId, u8 damage)
{
    if (enemyId >= MAX_ENEMIES || !obj_enemy[enemyId].obj_character.active) return;

    play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));  // feedback
    combat_state              = COMBAT_STATE_ENEMY_HIT;
    
    // Reduce enemy HP
    obj_enemy[enemyId].hitpoints -= damage;
    if (obj_enemy[enemyId].hitpoints <= 0) {
        obj_enemy[enemyId].hitpoints = 0; // Prevent negative HP
        // PENDING: Handle enemy defeat (remove from combat)
    }
}

// Hit the player
void hit_player(u8 damage)
{
    play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));  // feedback
    combat_state = COMBAT_STATE_PLAYER_HIT;

    // PENDING: Handle player damage
}