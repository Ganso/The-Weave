#ifndef _CONSTANTS_SPELLS_H_
#define _CONSTANTS_SPELLS_H_

// Identificadores y constantes del sistema de hechizos.
// ESPACIO DE IDS UNIFICADO: jugador y enemigo comparten numeración (a diferencia
// del sistema antiguo, donde THUNDER==EN_THUNDER==0 y el solape causó bugs).

// Player spells (0 .. SPELL_PLAYER_COUNT-1; el HUD/pausa itera este rango)
#define SPELL_THUNDER      0
#define SPELL_HIDE         1
#define SPELL_OPEN         2
#define SPELL_SLEEP        3
#define SPELL_FIRE         4
#define SPELL_LIGHT        5
#define SPELL_HEAL         6
#define SPELL_PLAYER_COUNT 7

// Enemy spells
#define SPELL_EN_THUNDER   7
#define SPELL_EN_BITE      8

#define SPELL_COUNT        9
#define SPELL_NONE         254

// Notas musicales (botones A B C X Y Z)
#define NOTE_NONE 0
#define NOTE_MI   1
#define NOTE_FA   2
#define NOTE_SOL  3
#define NOTE_LA   4
#define NOTE_SI   5
#define NOTE_DO   6
#define MAX_SPELL_NOTES 4

// Zonas narrativas (para canUse de hechizos de puzzle; las fija la escena)
#define ZONE_NONE     0
#define ZONE_CAULDRON 1

// Timing and delays
#define MAX_PATTERN_WAIT_TIME 2000             // ms de espera de la siguiente nota antes de cancelar
#define MIN_TIME_BETWEEN_NOTES SCREEN_FPS/4    // Ticks mínimos entre notas (anti-repetición)
#define MIN_TIME_BETWEEN_PATTERNS SCREEN_FPS/4 // Ticks mínimos entre hechizos
#define ENEMY_FRAMES_PER_NOTE SCREEN_FPS/4     // Cadencia de notas del enemigo

// Palette entry helpers compartidos por los efectos (B19)
#define PAL_ENTRY(pal,col)  (((pal)<<4)|(col))
#define PAL0_COL4           PAL_ENTRY(0,4)

#endif
