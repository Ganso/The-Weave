#ifndef _COMBAT_H_
#define _COMBAT_H_

typedef enum
{
    COMBAT_NO,                          /* no combat active - no enemies */
    COMBAT_STATE_IDLE,                 /* combat active, everybody waiting  */
    /* --- player turn ----------------------------------------------------- */
    /* NOTE: If no combat is active, but player is casting a Spell ... */
    /* we can use these states with no active enemies */
    COMBAT_STATE_PLAYER_PLAYING,       /* player is playing invocation notes    */
    COMBAT_STATE_PLAYER_EFFECT,        /* player spell effect is active         */
    /* --- enemy turn ------------------------------------------------------ */
    COMBAT_STATE_ENEMY_PLAYING,        /* enemy is playing its notes            */
    COMBAT_STATE_ENEMY_EFFECT         /* enemy spell effect is active          */
} CombatState;
extern CombatState combat_state; // Current combat state

typedef struct {
    u16         frameInState; // ++ every frame, reset on state change     
    /* active pattern (either side) -------------------------------------- */
    u16         activePattern;
    u16         effectTimer;  // frames since effect started               
    bool     patternReversed; // true = effect was reflected / reversed    
    /* note–playing phase ------------------------------------------------ */
    u16         noteTimer;    // frames since current note started         
    u8          playerNotes;  // how many notes the player has played (0-4)
    u8  enemyNoteIndex;       // 0-3 while the foe is playing its notes
    u16 enemyNoteTimer;       // frames since last enemy note
    u16 patternLockTimer; // frames until the next pattern can be played
    /* attacker id (relevant only during enemy phases) ------------------- */
    u8          activeEnemy;  // ENEMY_NONE = none
} CombatContext;
extern CombatContext combatContext; // Combat context


// Timings
#define ENEMY_HURT_DURATION   SCREEN_FPS  // Duration of the hurt animation in frames (enemies)
#define PLAYER_HURT_DURATION   SCREEN_FPS/2 // Duration of the hurt animation in frames (player)

// Combat functions

bool tryCounterSpell(void); // Try to counter an enemy spell
void combatInit(void); // Start combat phase
void combatFinish(void); // Finish combat phase
void hit_enemy(u8 enemyId, u8 damage); // Hit an enemy
void hit_player(u8 damage); // Hit the player
void update_combat(void); // Update combat state, 
void setIdle(void); // Set combat state to idle or none, depending on the context

#endif