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
    /* --- enemy counter-window (optional) --------------------------------- */
    COMBAT_STATE_PLAYER_COUNTER,       /* player is attempting a counter-spell  */
    /* --- enemy turn ------------------------------------------------------ */
    COMBAT_STATE_ENEMY_PLAYING,        /* enemy is playing its notes            */
    COMBAT_STATE_ENEMY_EFFECT,         /* enemy spell effect is active          */
    /* --- hit / impact feedback ------------------------------------------ */
    COMBAT_STATE_PLAYER_HIT,           /* player receives damage – show flash   */
    COMBAT_STATE_ENEMY_HIT             /* enemy receives damage – show flash    */
} CombatState;
extern CombatState combat_state; // Current combat state

typedef struct {
    CombatState state;        // current combat phase                      
    u16         frameInState; // ++ every frame, reset on state change     
    /* active pattern (either side) -------------------------------------- */
    u16         activePattern;
    u16         effectTimer;  // frames since effect started               
    bool     patternReversed; // true = effect was reflected / reversed    
    /* note–playing phase ------------------------------------------------ */
    u16         noteTimer;    // frames since current note started         
    u8          playerNotes;  // how many notes the player has played (0-4)
    u8          enemyNotes;   // how many notes the enemy has played  (0-4)
    /* attacker id (relevant only during enemy phases) ------------------- */
    u8          activeEnemy;  // ENEMY_NONE = none                         
} CombatContext;
extern CombatContext combatContext; // Combat context

#endif