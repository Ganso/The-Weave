/*
 * src/combat/combat.h — Combate por hechizos
 * --------------------------------------------
 * FSM global (combat_state): IDLE → PLAYER/ENEMY_PLAYING (notas) →
 * PLAYER/ENEMY_EFFECT (efecto). Durante ENEMY_EFFECT el jugador puede tocar
 * el patrón invertido → counter. El estado de los hechizos vive en el motor
 * (spells/spell.c, dos slots); combat_state sigue siendo el director.
 *
 * Relación con otros subsistemas:
 *   spells/spell.c  — motor de hechizos: update_combat llama spell_update()
 *                     cada frame y spell_enemy_try_launch() en IDLE
 *   spells/notes.c  — input de notas del jugador (validación → cast)
 *   actors/enemies  — enemigos activos, HP, animación de muerte (diferida, B4)
 *   interface/      — HUD de notas y contador de vida
 *   core/frame.c    — update_combat() se llama cada frame interactivo
 *
 * hit_enemy/hit_player aplican daño y estados HURT; la liberación del enemigo
 * derrotado la hace update_enemy_animations cuando expira modeTimer (B4).
 */
#ifndef _COMBAT_H_
#define _COMBAT_H_

#include <genesis.h>

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

// (El contexto de hechizos/notas vive ahora en el motor: spells/spell.c y spells/notes.c)

// Timings
#define ENEMY_HURT_DURATION   SCREEN_FPS  // Duration of the hurt animation in frames (enemies)
#define PLAYER_HURT_DURATION   SCREEN_FPS/2 // Duration of the hurt animation in frames (player)

// Combat functions

void combat_init(void); // Start combat phase
void combat_finish(void); // Finish combat phase
void hit_enemy(u8 enemyId, u8 damage); // Hit an enemy
void hit_player(u8 damage); // Hit the player
void update_combat(void); // Update combat state, 
void set_idle(void); // Set combat state to idle or none, depending on the context

#endif
